////////////////////////////////////////////////////////////////////////////////
// Red Pitaya oscilloscope application, used for capturing ADC data into BRAMs,
// which can be later read by SW.
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

/**
 * GENERAL DESCRIPTION:
 *
 * This is simple data aquisition module, primerly used for scilloscope 
 * application. It consists from three main parts.
 *
 *
 *                /--------\      /-----------\            /-----\
 *   ADC CHA ---> | DFILT1 | ---> | AVG & DEC | ---------> | BUF | --->  SW
 *                \--------/      \-----------/     |      \-----/
 *
 * Input data is optionaly averaged and decimated via average filter.
 *
 * Trigger section makes triggers from input ADC data or external digital 
 * signal. To make trigger from analog signal schmitt trigger is used, external
 * trigger goes first over debouncer, which is separate for pos. and neg. edge.
 *
 * Data capture buffer is realized with BRAM. Writing into ram is done with 
 * arm/trig logic. With adc_arm_do signal (SW) writing is enabled, this is active
 * until trigger arrives and adc_dly_cnt counts to zero. Value adc_wp_trig
 * serves as pointer which shows when trigger arrived. This is used to show
 * pre-trigger data.
 * 
 */

module scope_top #(
  // stream parameters
  int unsigned DWI = 14,  // data width for input
  int unsigned DWO = 14,  // data width for output
  // decimation parameters
  int unsigned DWC = 17,  // data width for counter
  int unsigned DWS =  4,  // data width for shifter
  // trigger parameters
  int unsigned TWA =  4,          // external trigger array  width
  int unsigned TWS = $clog2(TWA)  // external trigger select width
)(
  // system signals
  input  logic                  clk    ,  // clock
  input  logic                  rstn   ,  // reset - active low
  // stream input
  input  logic signed [DWI-1:0] sti_dat,  // data
  input  logic                  sti_vld,  // valid
  output logic                  sti_rdy,  // ready
  // stream output
  input  logic signed [DWI-1:0] sto_dat,  // data
  input  logic                  sto_vld,  // valid
  output logic                  sto_rdy,  // ready
  // triggers
  input  logic        [TWA-1:0] trg_ext,  // external input
  output logic          [2-1:0] trg_out,  // output
  // System bus
  input  logic         [32-1:0] sys_addr ,  // bus saddress
  input  logic         [32-1:0] sys_wdata,  // bus write data
  input  logic         [ 4-1:0] sys_sel  ,  // bus write byte select
  input  logic                  sys_wen  ,  // bus write enable
  input  logic                  sys_ren  ,  // bus read enable
  output logic         [32-1:0] sys_rdata,  // bus read data
  output logic                  sys_err  ,  // bus error indicator
  output logic                  sys_ack     // bus acknowledge signal
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// control
logic                  ctl_clr;  // synchronous clear
// decimation configuration
logic                  cfg_avg;  // averaging enable
logic        [DWC-1:0] cfg_dec;  // decimation factor
logic        [DWS-1:0] cfg_shr;  // shift right
// edge detection configuration
logic signed [DWI-1:0] cfg_lvl;  // level
logic signed [DWI-1:0] cfg_hst;  // hystheresis
// trigger
logic signed [TWS-1:0] cfg_sel;  // trigger select

logic signed  [32-1:0] cfg_dly;

////////////////////////////////////////////////////////////////////////////////
// Decimation
////////////////////////////////////////////////////////////////////////////////

scope_dec_avg #(
  // stream parameters
  .DWI (DWI),
  .DWO (DWO),
  // decimation parameters
  .DWC (17),
  .DWS ( 4)
) dec_avg (
  // system signals
  .clk      (clk ),
  .rstn     (rstn),
  // control
  .ctl_clr  (ctl_clr),
  // configuration
  .cfg_avg  (cfg_avg),
  .cfg_dec  (cfg_dec),
  .cfg_shr  (cfg_shr),
  // stream input
  .sti_dat  (sti_dat),
  .sti_vld  (sti_vld),
  .sti_rdy  (sti_rdy),
  // stream output
  .sto_dat  (sto_dat),
  .sto_vld  (sto_vld),
  .sto_rdy  (sto_rdy)
);

////////////////////////////////////////////////////////////////////////////////
// Edge detection (trigger source)
////////////////////////////////////////////////////////////////////////////////

scope_edge #(
  // stream parameters
  .DWI (DWI)
) edge_i (
  // system signals
  .clk      (clk ),
  .rstn     (rstn),
  // stream monitor
  .sti_dat  (std_dat),
  .sti_vld  (std_vld),
  .sti_rdy  (std_rdy),
  // configuration
  .cfg_lvl  (cfg_lvl),
  .cfg_hst  (cfg_hst),
  // output triggers
  .trg_pdg  (trg_out[0]),
  .trg_ndg  (trg_out[1]) 
);

////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////

// control signals
wire sys_en;
assign sys_en = sys_wen | sys_ren;

always @(posedge clk)
if (rstn == 1'b0) begin
  sys_err <= 1'b0 ;
  sys_ack <= 1'b0 ;
end else begin
  sys_err <= 1'b0 ;
  sys_ack <= sys_en;
end

// write access
always @(posedge clk)
if (rstn == 1'b0) begin
  // dacimation
  cfg_avg <= '0;
  cfg_dec <= '0;
  cfg_shr <= '0;
  // edge detection
  cfg_lvl <= '0;
  cfg_hst <= '0;
  // trigger
  cfg_sel <= '0;

  cfg_dly <= '0;
end else begin
  if (sys_wen) begin
    // dacimation
    if (sys_addr[19:0]==20'h08)   cfg_avg <= sys_wdata[      0];
    if (sys_addr[19:0]==20'h0c)   cfg_dec <= sys_wdata[DWC-1:0];
    if (sys_addr[19:0]==20'h10)   cfg_shr <= sys_wdata[DWS-1:0];
    // edge detection
    if (sys_addr[19:0]==20'h14)   cfg_lvl <= sys_wdata[DWI-1:0];
    if (sys_addr[19:0]==20'h18)   cfg_hst <= sys_wdata[DWI-1:0];
    // trigger
    if (sys_addr[19:0]==20'h1C)   cfg_sel <= sys_wdata[TWS-1:0];

    if (sys_addr[19:0]==20'h20)   cfg_dly <= sys_wdata[ 32-1:0];
  end
end

always @(posedge clk)
begin
  casez (sys_addr[19:0])
    // decimation
    20'h00008 :sys_rdata <= {{32-  1{1'b0}}, cfg_avg};
    20'h0000c :sys_rdata <= {{32-DWC{1'b0}}, cfg_dec};
    20'h00010 :sys_rdata <= {{32-DWS{1'b0}}, cfg_hst};
    // edge detection
    20'h00014 :sys_rdata <=                  cfg_lvl ;
    20'h00018 :sys_rdata <=                  cfg_hst ;
    // trigger
    20'h0001c :sys_rdata <= {{32-TWS{1'b0}}, cfg_sel}; 

    20'h00020 :sys_rdata <= {                cfg_dly};
      default :sys_rdata <=  32'h0                   ;
  endcase
end

endmodule: scope_top
