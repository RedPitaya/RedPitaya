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

module red_pitaya_pid #(
  // data stream parameters
  int unsigned DWI = 14,  // data width input
  int unsigned DWO = 14,  // data width output
  // channel parameters
  int unsigned CNI = 2,   // channel number for inputs
  int unsigned CNO = 2    // channel number for outputs
)(
  // system signals
  input  logic                           clk ,  // processing clock
  input  logic                           rstn,  // processing reset - active low
  // signals
  input  logic signed [CNI-1:0][DWI-1:0] dat_i,  // input data
  output logic signed [CNO-1:0][DWO-1:0] dat_o,  // output data
  // system bus
  input  logic [32-1:0] sys_addr ,  // bus address
  input  logic [32-1:0] sys_wdata,  // bus write data
  input  logic [ 4-1:0] sys_sel  ,  // bus write byte select
  input  logic          sys_wen  ,  // bus write enable
  input  logic          sys_ren  ,  // bus read enable
  output logic [32-1:0] sys_rdata,  // bus read data
  output logic          sys_err  ,  // bus error indicator
  output logic          sys_ack     // bus acknowledge signal
);

localparam int unsigned PSR = 12;
localparam int unsigned ISR = 18;
localparam int unsigned DSR = 10;

////////////////////////////////////////////////////////////////////////////////
//  PID block instances
////////////////////////////////////////////////////////////////////////////////

logic signed [4-1:0] [14-1:0] pid_out ;
logic        [4-1:0] [14-1:0] set_sp  ;
logic        [4-1:0] [14-1:0] set_kp  ;
logic        [4-1:0] [14-1:0] set_ki  ;
logic        [4-1:0] [14-1:0] set_kd  ;
logic        [4-1:0]          set_irst;

red_pitaya_pid_block #(
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
logic signed [DWO  -1:0] out_sat;

assign out_sum = pid_out[o][1] + pid_out[o][1];

always @(posedge clk)
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
always_ff @(posedge clk)
if (!rstn) begin
  set_irst <= '1;
end else begin
  if (sys_wen & (sys_addr[4+CLO+CLI]==1'b0)) begin
    set_irst <= sys_wdata[CNO*CNI-1:0];
  end
end

generate
for (genvar i=0; i<CNI; i++) begin: wr_cni
for (genvar o=0; o<CNO; o++) begin: wr_cno

always_ff @(posedge clk)
if (!rstn) begin
  set_sp [o][i] <= '0;
  set_kp [o][i] <= '0;
  set_ki [o][i] <= '0;
  set_kd [o][i] <= '0;
end else begin
  if (sys_wen & (sys_addr[4+CLO+CLI]==1'b1) & (sys_addr[4+CLO+CLI-1:4]=={o[CLO-1:0],i[CLI-1:0]})) begin
    if (sys_addr[3:0]==4'h0) set_sp [o][i] <= sys_wdata[14-1:0];
    if (sys_addr[3:0]==4'h4) set_kp [o][i] <= sys_wdata[14-1:0];
    if (sys_addr[3:0]==4'h8) set_ki [o][i] <= sys_wdata[14-1:0];
    if (sys_addr[3:0]==4'hC) set_kd [o][i] <= sys_wdata[14-1:0];
  end
end

end: wr_cno
end: wr_cni
endgenerate

// read access
logic sys_en;
assign sys_en = sys_wen | sys_ren;

always @(posedge clk)
if (!rstn) begin
  sys_err <= 1'b0;
  sys_ack <= 1'b0;
end else begin
  sys_err <= 1'b0;
  sys_ack <= sys_en;
end

always @(posedge clk)
if (sys_ren) begin
  if (sys_addr[4+CLO+CLI]==1'b0) begin
    sys_rdata <= {{32-CNO*CNI{1'b0}}, set_irst};
  end else begin
    casez (sys_addr[3:0])
      4'b00?? : sys_rdata <= {{32-14{1'b0}}, set_sp [sys_addr[4+CLO+CLI-1:4+CLI]] [sys_addr[4+CLI-1:4]]};
      4'b01?? : sys_rdata <= {{32-14{1'b0}}, set_kp [sys_addr[4+CLO+CLI-1:4+CLI]] [sys_addr[4+CLI-1:4]]};
      4'b10?? : sys_rdata <= {{32-14{1'b0}}, set_ki [sys_addr[4+CLO+CLI-1:4+CLI]] [sys_addr[4+CLI-1:4]]};
      4'b11?? : sys_rdata <= {{32-14{1'b0}}, set_kd [sys_addr[4+CLO+CLI-1:4+CLI]] [sys_addr[4+CLI-1:4]]};
    endcase
  end
end

endmodule: red_pitaya_pid
