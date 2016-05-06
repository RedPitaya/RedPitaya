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
  type DT = logic signed [14-1:0],
  // decimation parameters
  int unsigned DCW = 17,  // data width for counter
  int unsigned DSW =  4,  // data width for shifter
  // aquisition parameters
  int unsigned CW = 32,  // counter width
  // trigger parameters
  int unsigned TN =  4,  // external trigger array  width
  // timestamp parameters
  int unsigned TW = 64   // timestamp width
)(
  // streams
  axi4_stream_if.d       sti,      // input
  axi4_stream_if.s       sto,      // output
  // current time stamp
  input  logic  [TW-1:0] cts,
  // triggers
  input  logic  [TN-1:0] trg_ext,  // external input
  output logic           trg_swo,  // output from software
  output logic           trg_out,  // output from edge detection
  // interrupts
  output logic           irq_trg,  // trigger
  output logic           irq_stp,  // stop
  // System bus
  sys_bus_if.s           bus
);

localparam int unsigned DWI = $bits(DT);  // data width for input
localparam int unsigned DWO = $bits(DT);  // data width for output

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// streams
axi4_stream_if #(.DT (logic signed [DWI-1:0])) stf (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from filter
axi4_stream_if #(.DT (logic signed [DWI-1:0])) std (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from decimator

// acquire regset

// control
logic           ctl_rst;
// configuration (mode)
logic           cfg_con;  // continuous
logic           cfg_aut;  // automatic
// configuration/status pre trigger
logic  [CW-1:0] cfg_pre;
logic  [CW-1:0] sts_pre;
// configuration/status post trigger
logic  [CW-1:0] cfg_pst;
logic  [CW-1:0] sts_pst;
// control/status/timestamp acquire
logic           ctl_acq;  // acquire start
logic           sts_acq;
logic  [TW-1:0] cts_acq;
// control/status/timestamp trigger
logic           sts_trg;
logic  [TW-1:0] cts_trg;
// control/status/timestamp stop
logic           ctl_stp;  // acquire stop
logic  [TW-1:0] cts_stp;

// trigger
logic  [TN-1:0] cfg_trg;  // trigger select

// configuration
logic                  cfg_rng;  // range select (this one is only used by the firmware)

// edge detection configuration
logic signed [DWI-1:0] cfg_lvl;  // level
logic        [DWI-1:0] cfg_hst;  // hystheresis
logic                  cfg_edg;  // edge (0-pos, 1-neg)

// decimation configuration
logic                  cfg_avg;  // averaging enable
logic        [DCW-1:0] cfg_dec;  // decimation factor
logic        [DSW-1:0] cfg_shr;  // shift right

// filter configuration
logic                  cfg_byp;  // bypass
logic signed [ 18-1:0] cfg_faa;  // AA coefficient
logic signed [ 25-1:0] cfg_fbb;  // BB coefficient
logic signed [ 25-1:0] cfg_fkk;  // KK coefficient
logic signed [ 25-1:0] cfg_fpp;  // PP coefficient

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

localparam int unsigned BAW=7;

// write access
always @(posedge bus.clk)
if (~bus.rstn) begin
  // acquire regset
  cfg_con <= 1'b0;
  cfg_aut <= 1'b0;
  cfg_trg <= '0;
  cfg_pre <= '0;
  cfg_pst <= '0;

  // configuration
  cfg_rng <= '0;

  // edge detection
  cfg_lvl <= '0;
  cfg_hst <= '0;
  cfg_edg <= '0;

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
    // acquire regset
    if (bus.addr[BAW-1:0]=='h04)   cfg_con <= bus.wdata[     0];
    if (bus.addr[BAW-1:0]=='h04)   cfg_aut <= bus.wdata[     1];
    if (bus.addr[BAW-1:0]=='h08)   cfg_trg <= bus.wdata[TN-1:0];
    if (bus.addr[BAW-1:0]=='h10)   cfg_pre <= bus.wdata[CW-1:0];
    if (bus.addr[BAW-1:0]=='h14)   cfg_pst <= bus.wdata[CW-1:0];

    // configuration
    if (bus.addr[BAW-1:0]=='h40)   cfg_rng <= bus.wdata;

    // edge detection
    if (bus.addr[BAW-1:0]=='h50)   cfg_lvl <= bus.wdata[DWI-1:0];
    if (bus.addr[BAW-1:0]=='h54)   cfg_hst <= bus.wdata[DWI-1:0];
    if (bus.addr[BAW-1:0]=='h58)   cfg_edg <= bus.wdata[      0];

    // dacimation
    if (bus.addr[BAW-1:0]=='h60)   cfg_avg <= bus.wdata[      0];
    if (bus.addr[BAW-1:0]=='h64)   cfg_dec <= bus.wdata[DCW-1:0];
    if (bus.addr[BAW-1:0]=='h68)   cfg_shr <= bus.wdata[DSW-1:0];
    // filter
    if (bus.addr[BAW-1:0]=='h6c)   cfg_byp <= bus.wdata[      0];
    if (bus.addr[BAW-1:0]=='h70)   cfg_faa <= bus.wdata[ 18-1:0];
    if (bus.addr[BAW-1:0]=='h74)   cfg_fbb <= bus.wdata[ 25-1:0];
    if (bus.addr[BAW-1:0]=='h78)   cfg_fkk <= bus.wdata[ 25-1:0];
    if (bus.addr[BAW-1:0]=='h7c)   cfg_fpp <= bus.wdata[ 25-1:0];
  end
end

// control signals
assign ctl_stp = bus.wen & (bus.addr[BAW-1:0]=='h00) & bus.wdata[3];  // acquire stop
assign ctl_acq = bus.wen & (bus.addr[BAW-1:0]=='h00) & bus.wdata[2];  // acquire start
assign trg_swo = bus.wen & (bus.addr[BAW-1:0]=='h00) & bus.wdata[1];  // trigger
assign ctl_rst = bus.wen & (bus.addr[BAW-1:0]=='h00) & bus.wdata[0];  // reset

// read access
always_ff @(posedge bus.clk)
begin
  casez (bus.addr[BAW-1:0])
    // acquire regset
    'h00 : bus.rdata <= {{32-  4{1'b0}},~sts_acq, sts_acq, sts_trg, 1'b0};
    'h04 : bus.rdata <= {{32-  2{1'b0}}, cfg_aut, cfg_con};
    'h08 : bus.rdata <= {{32- TN{1'b0}}, cfg_trg};
    'h10 : bus.rdata <=              32'(cfg_pre);
    'h14 : bus.rdata <=              32'(cfg_pst);
    'h18 : bus.rdata <=              32'(sts_pre);
    'h1c : bus.rdata <=              32'(sts_pst);
    'h20 : bus.rdata <=              32'(cts_acq >>  0);
    'h24 : bus.rdata <=              32'(cts_acq >> 32);
    'h28 : bus.rdata <=              32'(cts_trg >>  0);
    'h2c : bus.rdata <=              32'(cts_trg >> 32);
    'h30 : bus.rdata <=              32'(cts_stp >>  0);
    'h34 : bus.rdata <=              32'(cts_stp >> 32);

    // configuration
    'h40 : bus.rdata <=                  cfg_rng ;

    // edge detection
    'h50 : bus.rdata <=                  cfg_lvl ;
    'h54 : bus.rdata <=                  cfg_hst ;
    'h58 : bus.rdata <=              32'(cfg_edg);

    // decimation
    'h60 : bus.rdata <= {{32-  1{1'b0}}, cfg_byp};
    'h64 : bus.rdata <= {{32-DCW{1'b0}}, cfg_dec};
    'h68 : bus.rdata <= {{32-DSW{1'b0}}, cfg_shr};
    // filter
    'h6c : bus.rdata <= {{32-  1{1'b0}}, cfg_avg};
    'h70 : bus.rdata <=                  cfg_faa ;
    'h74 : bus.rdata <=                  cfg_fbb ;
    'h78 : bus.rdata <=                  cfg_fkk ;
    'h7c : bus.rdata <=                  cfg_fpp ;

    default : bus.rdata <= '0;
  endcase
end

////////////////////////////////////////////////////////////////////////////////
// correction filter
////////////////////////////////////////////////////////////////////////////////

// streams
axi4_stream_if #(.DT (DT)) tmp_sti (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // tmp from input
axi4_stream_if #(.DT (DT)) tmp_stf (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // tmp from filter

assign tmp_sti.TDATA  = cfg_byp ? '0         :     sti.TDATA ;
assign tmp_sti.TLAST  = cfg_byp ? '0         :     sti.TLAST ;
assign tmp_sti.TKEEP  = cfg_byp ? '0         :     sti.TKEEP ;
assign tmp_sti.TVALID = cfg_byp ? '0         :     sti.TVALID;
assign     sti.TREADY = cfg_byp ? stf.TREADY : tmp_sti.TREADY;

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

assign     stf.TDATA  = cfg_byp ? sti.TDATA  : tmp_stf.TDATA ;
assign     stf.TLAST  = cfg_byp ? sti.TLAST  : tmp_stf.TLAST ;
assign     stf.TKEEP  = cfg_byp ? sti.TKEEP  : tmp_stf.TKEEP ;
assign     stf.TVALID = cfg_byp ? sti.TVALID : tmp_stf.TVALID;
assign tmp_stf.TREADY = cfg_byp ? '0         :     stf.TREADY;

////////////////////////////////////////////////////////////////////////////////
// Decimation
////////////////////////////////////////////////////////////////////////////////

scope_dec_avg #(
  // stream parameters
  .DWI (DWI),
  .DWO (DWO),
  // decimation parameters
  .DCW (17),
  .DSW ( 4)
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

acq #(
  .TN (TN),
  .TW (TW),
  .CW (CW)
) acq (
  // stream input/output
  .sti      (std),
  .sto      (sto),
  // current time stamp
  .cts      (cts),
  // interrupts
  .irq_trg  (irq_trg),
  .irq_stp  (irq_stp),
  // control
  .ctl_rst  (ctl_rst),
  // configuration (mode)
  .cfg_trg  (cfg_trg),
  .cfg_con  (cfg_con),
  .cfg_aut  (cfg_aut),
  // configuration/status pre trigger
  .cfg_pre  (cfg_pre),
  .sts_pre  (sts_pre),
  // configuration/status post trigger
  .cfg_pst  (cfg_pst),
  .sts_pst  (sts_pst),
  // control/status/timestamp acquire
  .ctl_acq  (ctl_acq),
  .sts_acq  (sts_acq),
  .cts_acq  (cts_acq),
  // control/status/timestamp trigger
  .ctl_trg  (trg_ext),
  .sts_trg  (sts_trg),
  .cts_trg  (cts_trg),
  // control/status/timestamp stop
  .ctl_stp  (ctl_stp),
  .cts_stp  (cts_stp)
);

endmodule: scope_top
