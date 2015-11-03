////////////////////////////////////////////////////////////////////////////////
// Red Pitaya arbitrary signal generator (ASG).
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

/**
 * GENERAL DESCRIPTION:
 *
 * Arbitrary signal generator takes data stored in buffer and sends them to DAC.
 *
 *                /-----\         /--------\
 *   SW --------> | BUF | ------> | kx + o | ---> DAC CHB
 *                \-----/         \--------/ 
 *
 * Buffers are filed with SW. It also sets finite state machine which take control
 * over read pointer. All registers regarding reading from buffer has additional 
 * 16 bits used as decimal points. In this way we can make better ratio betwen 
 * clock cycle and frequency of output signal. 
 *
 * Finite state machine can be set for one time sequence or continously wrapping.
 * Starting trigger can come from outside, notification trigger used to synchronize
 * with other applications (scope) is also available. Both channels are independant.
 */

module asg_top #(
  // data parameters
  int unsigned DWO = 14,  // RAM data width
  // buffer parameters
  int unsigned CWM = 14,  // counter width magnitude (fixed point integer)
  int unsigned CWF = 16,  // counter width fraction  (fixed point fraction)
  // trigger parameters
  int unsigned TWA =  4,          // external trigger array  width
  int unsigned TWS = $clog2(TWA)  // external trigger select width
)(
  input  logic                  clk ,  // DAC clock
  input  logic                  rstn,  // DAC reset - active low
  // stream output
  output logic signed [DWO-1:0] sto_dat,  // data
  output logic                  sto_vld,  // valid
  input  logic                  sto_rdy,  // ready
  // triggers
  input  logic        [TWA-1:0] trg_ext,  // external input
  output logic                  trg_swo,  // output from software
  output logic                  trg_out,  // output from engine
  // System bus
  input  logic        [  4-1:0] sys_sel  ,  // bus write byte select
  input  logic                  sys_wen  ,  // bus write enable
  input  logic                  sys_ren  ,  // bus read enable
  input  logic        [ 32-1:0] sys_addr ,  // bus address
  input  logic        [ 32-1:0] sys_wdata,  // bus write data
  output logic        [ 32-1:0] sys_rdata,  // bus read data
  output logic                  sys_err  ,  // bus error indicator
  output logic                  sys_ack     // bus acknowledge signal
);

////////////////////////////////////////////////////////////////////////////////
// read/write access to buffer
////////////////////////////////////////////////////////////////////////////////

logic                  bus_we   ;
logic        [CWM-1:0] bus_addr ;
logic signed [DWO-1:0] bus_wdata;
logic signed [DWO-1:0] bus_rdata;
logic          [3-1:0] bus_dly ;
logic                  bus_ack ;

always @(posedge clk)
begin
  bus_we    <= sys_wen && (sys_addr[19:CWM+2] == 'h1);
  bus_addr  <= sys_addr[CWM+1:2];
  bus_wdata <= sys_addr[CWM+1:2];
end

always_ff @(posedge clk)
if (rstn == 1'b0) begin
   bus_dly   <= '0;
   bus_ack   <= '0;
end else begin
   bus_dly <= {bus_dly[3-2:0], sys_ren};
   bus_ack <=  bus_dly[3-1] || sys_wen ;
end

////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////

logic               ctl_rst ;
// configuration
logic     [TWS-1:0] cfg_tsel; // trigger select
logic               cfg_wrap;
logic [CWM+CWF-1:0] cfg_size;
logic [CWM+CWF-1:0] cfg_step;
logic [CWM+CWF-1:0] cfg_offs;
logic     [ 16-1:0] cfg_ncyc;
logic     [ 16-1:0] cfg_rnum;
logic     [ 32-1:0] cfg_rdly;

always_ff @(posedge clk)
if (rstn == 1'b0) begin
   cfg_tsel  <= '0;
   cfg_wrap  <= '0;
   cfg_size  <= '0;
   cfg_offs  <= '0;
   cfg_step  <= '0;
   cfg_ncyc  <= '0;
   cfg_rnum  <= '0;
   cfg_rdly  <= '0;
end else begin
   if (sys_wen) begin
      if (sys_addr[19:0]==20'h04)  cfg_tsel <= sys_wdata[    TWS-1:0];
      if (sys_addr[19:0]==20'h08)  cfg_size <= sys_wdata[CWM+CWF-1:0];
      if (sys_addr[19:0]==20'h0C)  cfg_offs <= sys_wdata[CWM+CWF-1:0];
      if (sys_addr[19:0]==20'h10)  cfg_step <= sys_wdata[CWM+CWF-1:0];
      if (sys_addr[19:0]==20'h18)  cfg_ncyc <= sys_wdata[     16-1:0];
      if (sys_addr[19:0]==20'h1C)  cfg_rnum <= sys_wdata[     16-1:0];
      if (sys_addr[19:0]==20'h20)  cfg_rdly <= sys_wdata[     32-1:0];
   end
end

wire sys_en;
assign sys_en = sys_wen | sys_ren;

always_ff @(posedge clk)
if (~rstn) begin
   sys_err <= 1'b0;
   sys_ack <= 1'b0;
end else begin
   sys_err <= 1'b0;
   casez (sys_addr[19:0])
     20'h00004 : begin sys_ack <= sys_en;          sys_rdata <= {{32-1  -TWS{1'b0}}, cfg_wrap
                                                                                   , cfg_tsel}; end
     20'h00008 : begin sys_ack <= sys_en;          sys_rdata <= {{32-CWM-CWF{1'b0}}, cfg_size}; end
     20'h0000C : begin sys_ack <= sys_en;          sys_rdata <= {{32-CWM-CWF{1'b0}}, cfg_offs}; end
     20'h00010 : begin sys_ack <= sys_en;          sys_rdata <= {{32-CWM-CWF{1'b0}}, cfg_step}; end
     20'h00018 : begin sys_ack <= sys_en;          sys_rdata <= {{32-     16{1'b0}}, cfg_ncyc}; end
     20'h0001C : begin sys_ack <= sys_en;          sys_rdata <= {{32-     16{1'b0}}, cfg_rnum}; end
     20'h00020 : begin sys_ack <= sys_en;          sys_rdata <= cfg_rdly                      ; end

     20'h1zzzz : begin sys_ack <= bus_ack;         sys_rdata <= {{32-    DWO{1'b0}}, bus_rdata}; end

       default : begin sys_ack <= sys_en;          sys_rdata <=  32'h0                       ; end
   endcase
end

assign ctl_rst = sys_wen & (sys_addr[19:0]==20'h00) & sys_wdata[0];
assign trg_swo = sys_wen & (sys_addr[19:0]==20'h00) & sys_wdata[1];

////////////////////////////////////////////////////////////////////////////////
// trigger multiplexer
////////////////////////////////////////////////////////////////////////////////

logic trg_mux;
assign trg_mux = trg_ext [cfg_tsel];

////////////////////////////////////////////////////////////////////////////////
// generator core instance 
////////////////////////////////////////////////////////////////////////////////

asg #(
  .DWO (DWO),
  .CWM (CWM),
  .CWF (CWF)
) asg (
  // system signals
  .clk       (clk      ),
  .rstn      (rstn     ),
  // DAC
  .sto_dat   (sto_dat  ),
  .sto_vld   (sto_vld  ),
  .sto_rdy   (sto_rdy  ),
  // trigger
  .trg_i     (trg_mux  ),
  .trg_o     (trg_out  ),
  // CPU buffer access
  .bus_ena   (bus_ena  ),
  .bus_wen   (bus_wen  ),
  .bus_addr  (bus_addr ),
  .bus_wdata (bus_wdata),
  .bus_rdata (bus_rdata),
  // configuration
  .cfg_size  (cfg_size ),
  .cfg_step  (cfg_step ),
  .cfg_offs  (cfg_offs ),
  .cfg_ncyc  (cfg_ncyc ),
  .cfg_rnum  (cfg_rnum ),
  .cfg_rdly  (cfg_rdly ),
  // control
  .ctl_rst   (ctl_rst  ),
  .cfg_wrap  (cfg_wrap )
);



endmodule: asg_top
