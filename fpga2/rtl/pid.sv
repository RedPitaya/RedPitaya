////////////////////////////////////////////////////////////////////////////////
// Module: MIMO PID controller
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

/**
 * GENERAL DESCRIPTION:
 *
 * Multiple input multiple output controller.
 *
 *
 *                 /-------\       /-----------\
 *   CHA -----+--> | PID11 | ----> | SUM & SAT | ---> CHA
 *            |    \-------/       \-----------/
 *            |                            ^
 *            |    /-------\               |
 *            ---> | PID21 | ----------    |
 *                 \-------/           |   |
 *                                     |   |
 *  INPUT                              |   |         OUTPUT
 *                                     |   |
 *                 /-------\           |   |
 *            ---> | PID12 | --------------
 *            |    \-------/           |    
 *            |                        ˇ
 *            |    /-------\       /-----------\
 *   CHB -----+--> | PID22 | ----> | SUM & SAT | ---> CHB
 *                 \-------/       \-----------/
 *
 *
 * MIMO controller is build from four equal submodules, each can have 
 * different settings.
 *
 * Each output is sum of two controllers with different input. That sum is also
 * saturated to protect from wrapping.
 * 
 */

module pid #(
  // data stream parameters
  int unsigned DWI = 14,  // data width input
  int unsigned DWO = 14,  // data width output
  // channel parameters
  int unsigned CNI = 2,   // channel number for inputs
  int unsigned CNO = 2    // channel number for outputs
)(
  // streams
  axi4_stream_if.d  sti [CNI-1:0],  // input
  axi4_stream_if.s  sto [CNO-1:0],  // output
  // system bus
  sys_bus_if.s      bus
);

localparam int unsigned PSR = 12;
localparam int unsigned ISR = 18;
localparam int unsigned DSR = 10;

typedef logic signed [DWI-1:0] dti_t;
typedef logic signed [DWO-1:0] dto_t;
typedef logic signed [ 14-1:0] cfg_t;

////////////////////////////////////////////////////////////////////////////////
// glue logic connecting streams to local signals
////////////////////////////////////////////////////////////////////////////////

dti_t [CNI-1:0] dat_i;  // input data
dto_t [CNO-1:0] dat_o;  // output data

generate
for (genvar i=0; i<CNI; i++) begin: for_dat_i
  assign dat_i[i] = sti[i].TDATA;
end: for_dat_i
endgenerate

generate
for (genvar i=0; i<CNI; i++) begin: for_dat_o
  assign sto[i].TDATA = dat_o[i];
end: for_dat_o
endgenerate

logic clk;
logic rstn;

assign clk  = sti[0].ACLK   ;
assign rstn = sti[0].ARESETn;

////////////////////////////////////////////////////////////////////////////////
//  PID block instances
////////////////////////////////////////////////////////////////////////////////

dto_t [CNO-1:0] [CNI-1:0] pid_out ;
cfg_t [CNO-1:0] [CNI-1:0] set_sp  ;
cfg_t [CNO-1:0] [CNI-1:0] set_kp  ;
cfg_t [CNO-1:0] [CNI-1:0] set_ki  ;
cfg_t [CNO-1:0] [CNI-1:0] set_kd  ;
logic [CNO-1:0] [CNI-1:0] set_irst;

pid_block #(
  .PSR (PSR),
  .ISR (ISR),
  .DSR (DSR)      
) pid [CNO-1:0][CNI-1:0] (
  // system signals
  .clk        (clk ),
  .rstn       (rstn),
  // data
  .dat_i      ({2{dat_i}}),
  .dat_o      (pid_out),
   // settings
  .set_sp     (set_sp  ),
  .set_kp     (set_kp  ),
  .set_ki     (set_ki  ),
  .set_kd     (set_kd  ),
  .int_rst    (set_irst) 
);

////////////////////////////////////////////////////////////////////////////////
//  Sum and saturation
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar o=0; o<CNO; o++) begin: for_cno

logic signed [DWO+1-1:0] out_sum;

always_comb
begin
  out_sum = '0;
  for (int unsigned i=0; i<CNI; i++) begin: for_cni
    out_sum += pid_out[o][i];
  end: for_cni
end

always_ff @(posedge clk)
if (!rstn) begin
   dat_o[o] <= '0;
end else begin
   if (out_sum[15-1:15-2]==2'b01) // postitive sat
      dat_o[o] <= 14'h1FFF ;
   else if (out_sum[15-1:15-2]==2'b10) // negative sat
      dat_o[o] <= 14'h2000 ;
   else
      dat_o[o] <= out_sum[14-1:0] ;
end

end: for_cno
endgenerate

////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////

localparam int unsigned CLI = $clog2(CNI);
localparam int unsigned CLO = $clog2(CNO);

// write access
always_ff @(posedge bus.clk)
if (!bus.rstn) begin
  set_irst <= '1;
end else begin
  if (bus.wen & (bus.addr[4+CLO+CLI]==1'b0)) begin
    set_irst <= bus.wdata[CNO*CNI-1:0];
  end
end

generate
for (genvar i=0; i<CNI; i++) begin: wr_cni
for (genvar o=0; o<CNO; o++) begin: wr_cno

always_ff @(posedge bus.clk)
if (!bus.rstn) begin
  set_sp [o][i] <= '0;
  set_kp [o][i] <= '0;
  set_ki [o][i] <= '0;
  set_kd [o][i] <= '0;
end else begin
  if (bus.wen & (bus.addr[4+CLO+CLI]==1'b1) & (bus.addr[4+CLO+CLI-1:4]=={o[CLO-1:0],i[CLI-1:0]})) begin
    if (bus.addr[3:0]==4'h0) set_sp [o][i] <= bus.wdata[14-1:0];
    if (bus.addr[3:0]==4'h4) set_kp [o][i] <= bus.wdata[14-1:0];
    if (bus.addr[3:0]==4'h8) set_ki [o][i] <= bus.wdata[14-1:0];
    if (bus.addr[3:0]==4'hC) set_kd [o][i] <= bus.wdata[14-1:0];
  end
end

end: wr_cno
end: wr_cni
endgenerate

// read access
logic sys_en;
assign sys_en = bus.wen | bus.ren;

always_ff @(posedge bus.clk)
if (!bus.rstn) begin
  bus.err <= 1'b0;
  bus.ack <= 1'b0;
end else begin
  bus.err <= 1'b0;
  bus.ack <= sys_en;
end

always_ff @(posedge bus.clk)
if (bus.ren) begin
  if (bus.addr[4+CLO+CLI]==1'b0) begin
    bus.rdata <= {{32-CNO*CNI{1'b0}}, set_irst};
  end else begin
    casez (bus.addr[3:0])
      4'b00?? : bus.rdata <= {{32-14{1'b0}}, set_sp [bus.addr[4+CLO+CLI-1:4+CLI]] [bus.addr[4+CLI-1:4]]};
      4'b01?? : bus.rdata <= {{32-14{1'b0}}, set_kp [bus.addr[4+CLO+CLI-1:4+CLI]] [bus.addr[4+CLI-1:4]]};
      4'b10?? : bus.rdata <= {{32-14{1'b0}}, set_ki [bus.addr[4+CLO+CLI-1:4+CLI]] [bus.addr[4+CLI-1:4]]};
      4'b11?? : bus.rdata <= {{32-14{1'b0}}, set_kd [bus.addr[4+CLO+CLI-1:4+CLI]] [bus.addr[4+CLI-1:4]]};
    endcase
  end
end

endmodule: pid
