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
  // streams
  str_bus_if.d                  sti,      // input
  str_bus_if.s                  sto,      // output
  // triggers
  input  logic        [TWA-1:0] trg_ext,  // external input
  output logic                  trg_swo,  // output from software
  output logic                  trg_out,  // output from edge detection
  // System bus
  sys_bus_if.s                  bus
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////


// streams
str_bus_if #(.DAT_T (logic signed [DWI-1:0])) stf (.clk (sti.clk), .rstn (sti.rstn));  // from filter
str_bus_if #(.DAT_T (logic signed [DWI-1:0])) std (.clk (sti.clk), .rstn (sti.rstn));  // from decimator

// control
logic                  ctl_rst;  // synchronous clear
logic                  ctl_acq;  // start acquire run
// status
logic                  sts_acq;  // acquire status
// configuration
logic                  cfg_rng;  // range select (this one is only used by the firmware)
// trigger
logic signed [TWS-1:0] cfg_sel;  // trigger select
logic        [ 32-1:0] cfg_dly;  // delay value
logic        [ 32-1:0] sts_dly;  // delay counter
// edge detection configuration
logic signed [DWI-1:0] cfg_lvl;  // level
logic        [DWI-1:0] cfg_hst;  // hystheresis
// decimation configuration
logic                  cfg_avg;  // averaging enable
logic        [DWC-1:0] cfg_dec;  // decimation factor
logic        [DWS-1:0] cfg_shr;  // shift right
// filter configuration
logic                  cfg_byp;  // bypass
logic signed [ 18-1:0] cfg_faa;  // AA coefficient
logic signed [ 25-1:0] cfg_fbb;  // BB coefficient
logic signed [ 25-1:0] cfg_fkk;  // KK coefficient
logic signed [ 25-1:0] cfg_fpp;  // PP coefficient

// trigger
logic                  sts_trg;  // trigger status
logic                  trg_mux;  // multiplexed trigger signal


////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////

// control signals
wire sys_en;
assign sys_en = bus.wen | bus.ren;

always @(posedge bus.clk)
if (~bus.rstn) begin
  bus.err <= 1'b0;
  bus.ack <= 1'b0;
end else begin
  bus.err <= 1'b0;
  bus.ack <= sys_en;
end

// write access
always @(posedge bus.clk)
if (~bus.rstn) begin
  // control
  ctl_acq <= 1'b0;
  // configuration
  cfg_rng <= 1'b0;
  // trigger
  cfg_sel <= '0;
  cfg_dly <= '0;
  // edge detection
  cfg_lvl <= '0;
  cfg_hst <= '0;
  // filter/dacimation
  cfg_byp <= '0;
  cfg_avg <= '0;
  cfg_dec <= '0;
  cfg_shr <= '0;
  cfg_faa <= '0;
  cfg_fbb <= '0;
  cfg_fkk <= 25'hFFFFFF;
  cfg_fpp <= '0;
end else begin
  if (bus.wen) begin
    // configuration
    if (bus.addr[6-1:0]==6'h04)   cfg_rng <= bus.wdata[      0];
    // trigger
    if (bus.addr[6-1:0]==6'h08)   cfg_sel <= bus.wdata[TWS-1:0];
    if (bus.addr[6-1:0]==6'h0c)   cfg_dly <= bus.wdata[ 32-1:0];
    // edge detection
    if (bus.addr[6-1:0]==6'h10)   cfg_lvl <= bus.wdata[DWI-1:0];
    if (bus.addr[6-1:0]==6'h14)   cfg_hst <= bus.wdata[DWI-1:0];
    // filter/dacimation
    if (bus.addr[6-1:0]==6'h20)   cfg_byp <= bus.wdata[      0];
    if (bus.addr[6-1:0]==6'h24)   cfg_avg <= bus.wdata[      0];
    if (bus.addr[6-1:0]==6'h28)   cfg_dec <= bus.wdata[DWC-1:0];
    if (bus.addr[6-1:0]==6'h2c)   cfg_shr <= bus.wdata[DWS-1:0];
    if (bus.addr[6-1:0]==6'h30)   cfg_faa <= bus.wdata[ 18-1:0];
    if (bus.addr[6-1:0]==6'h34)   cfg_fbb <= bus.wdata[ 25-1:0];
    if (bus.addr[6-1:0]==6'h38)   cfg_fkk <= bus.wdata[ 25-1:0];
    if (bus.addr[6-1:0]==6'h3c)   cfg_fpp <= bus.wdata[ 25-1:0];
  end
end

// control signals
assign ctl_rst = bus.wen & (bus.addr[6-1:0]==6'h00) & bus.wdata[0];  // reset
assign trg_swo = bus.wen & (bus.addr[6-1:0]==6'h00) & bus.wdata[1];  // trigger
assign sts_run = bus.wen & (bus.addr[6-1:0]==6'h00) & bus.wdata[2];  // run acquire

// read access
always_ff @(posedge bus.clk)
begin
  casez (bus.addr[19:0])
    // control/status
    6'h00 : bus.rdata <= {{32-  3{1'b0}}, sts_acq,
                                          sts_trg, 1'b0};
    // configuration
    6'h04 : bus.rdata <= {{32-  1{1'b0}}, cfg_rng};
    // trigger
    6'h08 : bus.rdata <= {{32-TWS{1'b0}}, cfg_sel}; 
    6'h0c : bus.rdata <=                  cfg_dly ;
    // edge detection
    6'h10 : bus.rdata <=                  cfg_lvl ;
    6'h14 : bus.rdata <=                  cfg_hst ;
    // filter/decimation
    6'h20 : bus.rdata <= {{32-  1{1'b0}}, cfg_byp};
    6'h24 : bus.rdata <= {{32-  1{1'b0}}, cfg_avg};
    6'h28 : bus.rdata <= {{32-DWC{1'b0}}, cfg_dec};
    6'h2c : bus.rdata <= {{32-DWS{1'b0}}, cfg_shr};
    6'h30 : bus.rdata <=                  cfg_faa ;
    6'h34 : bus.rdata <=                  cfg_fbb ;
    6'h38 : bus.rdata <=                  cfg_fkk ;
    6'h3c : bus.rdata <=                  cfg_fpp ;

    default:bus.rdata <=  32'h0                   ;
  endcase
end

////////////////////////////////////////////////////////////////////////////////
// correction filter
////////////////////////////////////////////////////////////////////////////////

// streams
str_bus_if #(.DAT_T (logic signed [DWI-1:0])) tmp_sti (.clk (sti.clk), .rstn (rstn));  // tmp from input
str_bus_if #(.DAT_T (logic signed [DWI-1:0])) tmp_stf (.clk (sti.clk), .rstn (rstn));  // tmp from filter

assign tmp_sti.dat = cfg_byp ? '0      :     sti.dat;
assign tmp_sti.vld = cfg_byp ? '0      :     sti.vld;
assign     sti.rdy = cfg_byp ? stf.rdy : tmp_sti.rdy;

scope_filter #(
  // stream parameters
  .DWI (DWI),
  .DWO (DWO)
) filter (
  // input stream
  .sti      (tmp_sti),
  // output stream
  .sto      (tmp_stf),
  // configuration
  .cfg_aa   (cfg_faa),
  .cfg_bb   (cfg_fbb),
  .cfg_kk   (cfg_fkk),
  .cfg_pp   (cfg_fpp),
  // control
  .ctl_rst  (1'b0)
);

assign     stf.dat = cfg_byp ? sti.dat : tmp_stf.dat;
assign     stf.vld = cfg_byp ? sti.vld : tmp_stf.vld;
assign tmp_stf.rdy = cfg_byp ? '0      :     stf.rdy;

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
  // control
  .ctl_rst  (ctl_rst),
  // configuration
  .cfg_avg  (cfg_avg),
  .cfg_dec  (cfg_dec),
  .cfg_shr  (cfg_shr),
  // streams
  .sti      (stf),
  .sto      (std)
);

////////////////////////////////////////////////////////////////////////////////
// Edge detection (trigger source)
////////////////////////////////////////////////////////////////////////////////

scope_edge #(
  // stream parameters
  .DWI (DWI)
) edge_i (
  // control
  .ctl_rst  (ctl_rst),
  // configuration
  .cfg_edg  (cfg_edg),
  .cfg_lvl  (cfg_lvl),
  .cfg_hst  (cfg_hst),
  // output triggers
  .sts_trg  (trg_out),
  // stream monitor
  .str      (std)
);

////////////////////////////////////////////////////////////////////////////////
// aquire and trigger status handler
////////////////////////////////////////////////////////////////////////////////

assign trg_mux = trg_ext [cfg_sel];

acq #(.DW (DWO), .CW (32)) acq (
  // streams
  .sti      (std),
  .sto      (sto),
  // control
  .ctl_rst  (ctl_rst),
  // delay configuration/status
  .cfg_dly  (cfg_dly),
  .sts_dly  (sts_dly),
  // acquire control/status
  .ctl_acq  (ctl_acq),
  .sts_acq  (sts_acq),
  // trigger control/status
  .ctl_trg  (ctl_trg),
  .sts_trg  (sts_trg)
);

endmodule: scope_top
