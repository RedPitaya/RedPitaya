////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya oscilloscope.
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module osc #(
  // stream parameters
  int unsigned DN = 1,  // data number
  type DT = logic signed [16-1:0],
  // decimation parameters
  int unsigned DCW = 17,  // data width for counter
  int unsigned DSW =  4,  // data width for shifter
  // aquisition parameters
  int unsigned CW  = 32-1,  // counter width
  // event parameters
  int unsigned ER  = 0,   // event reset
  int unsigned EN  = 1,   // event number
  int unsigned EL  = $clog2(EN),
  // trigger parameters
  int unsigned TN  = 1    // trigger number
)(
  // streams
  axi4_stream_if.d               sti,  // input
  axi4_stream_if.s               sto,  // output
  // events input/output
  input  evn_pkg::evn_t [EN-1:0] evi,  // input
  output evn_pkg::evn_t          evo,  // output
  // triggers input/output
  input  logic          [TN-1:0] trg,  // input
  output logic                   tro,  // output
  // reset output
  output logic                   ctl_rst,
  // interrupt
  output logic                   irq,
  // system bus
  sys_bus_if.s                   bus
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// streams
axi4_stream_if #(.DT (DT)) stf (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from filter
axi4_stream_if #(.DT (DT)) std (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from decimator
axi4_stream_if #(.DT (DT)) ste (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from edge detection

// event select
logic  [EL-1:0] cfg_evn;
// trigger mask
logic  [TN-1:0] cfg_trg;

// software events
evn_pkg::evn_t  evn;  // multiplexed input
evn_pkg::evn_t  evs;  // status

// trigger
logic           ctl_trg;
logic           sts_trg;

// configuration/status/overflow pre trigger
logic  [CW-1:0] cfg_pre;
logic  [CW-1:0] sts_pre;
logic           sts_pro;
// configuration/status/overflow post trigger
logic  [CW-1:0] cfg_pst;
logic  [CW-1:0] sts_pst;
logic           sts_pso;

// edge detection configuration
DT              cfg_low;  // lower level
DT              cfg_upp;  // upper level
logic           cfg_edg;  // edge (0-pos, 1-neg)

// decimation configuration
logic           cfg_avg;  // averaging enable
logic [DCW-1:0] cfg_dec;  // decimation factor
logic [DSW-1:0] cfg_shr;  // shift right

// filter configuration
logic                  cfg_byp;  // bypass
logic signed [ 18-1:0] cfg_faa;  // AA coefficient
logic signed [ 25-1:0] cfg_fbb;  // BB coefficient
logic signed [ 25-1:0] cfg_fkk;  // KK coefficient
logic signed [ 25-1:0] cfg_fpp;  // PP coefficient

////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////

always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  bus.err <= 1'b0;
  bus.ack <= 1'b0;
end else begin
  bus.err <= 1'b0;
  bus.ack <= bus.wen | bus.ren;
end

localparam int unsigned BAW=7;

// write access
always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  // event select
  cfg_evn <= ER;
  // trigger mask
  cfg_trg <= '0;
  // configuration
  cfg_pre <= '0;
  cfg_pst <= '0;
  // edge detection
  cfg_low <= '0;
  cfg_upp <= '0;
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
    // event select
    if (bus.addr[BAW-1:0]=='h04)  cfg_evn <= bus.wdata;
    // triger mask
    if (bus.addr[BAW-1:0]=='h08)  cfg_trg <= bus.wdata;
    // trigger pre/post time
    if (bus.addr[BAW-1:0]=='h10)  cfg_pre <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h14)  cfg_pst <= bus.wdata;
    // edge detection
    if (bus.addr[BAW-1:0]=='h20)  cfg_low <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h24)  cfg_upp <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h28)  cfg_edg <= bus.wdata[      0];
    // dacimation/filter
    if (bus.addr[BAW-1:0]=='h30)  cfg_dec <= bus.wdata[DCW-1:0];
    if (bus.addr[BAW-1:0]=='h34)  cfg_shr <= bus.wdata[DSW-1:0];
    if (bus.addr[BAW-1:0]=='h38)  cfg_avg <= bus.wdata[      0];
    if (bus.addr[BAW-1:0]=='h3c)  cfg_byp <= bus.wdata[      0];
    if (bus.addr[BAW-1:0]=='h40)  cfg_faa <= bus.wdata[ 18-1:0];
    if (bus.addr[BAW-1:0]=='h44)  cfg_fbb <= bus.wdata[ 25-1:0];
    if (bus.addr[BAW-1:0]=='h48)  cfg_fkk <= bus.wdata[ 25-1:0];
    if (bus.addr[BAW-1:0]=='h4c)  cfg_fpp <= bus.wdata[ 25-1:0];
  end
end

// event outputs
always_ff @(posedge bus.clk)
if (~bus.rstn)  evo <= '0;
else            evo <= (bus.wen & (bus.addr[BAW-1:0]=='h00)) ? bus.wdata : '0;

// read access
always_ff @(posedge bus.clk)
casez (bus.addr[BAW-1:0])
  // control
  'h00: bus.rdata <= evs;
  // event select
  'h04: bus.rdata <= cfg_evn;
  // trigger mask
  'h08: bus.rdata <= cfg_trg;
  // trigger pre/post time
  'h10: bus.rdata <=              32'(cfg_pre);
  'h14: bus.rdata <=              32'(cfg_pst);
  'h18: bus.rdata <=    {sts_pro, 31'(sts_pre)};
  'h1c: bus.rdata <=    {sts_pso, 31'(sts_pst)};
  // edge detection
  'h20: bus.rdata <=                  cfg_low ;
  'h24: bus.rdata <=                  cfg_upp ;
  'h28: bus.rdata <=              32'(cfg_edg);
  // decimation/filter
  'h30: bus.rdata <= {{32-DCW{1'b0}}, cfg_dec};
  'h34: bus.rdata <= {{32-DSW{1'b0}}, cfg_shr};
  'h38: bus.rdata <= {{32-  1{1'b0}}, cfg_avg};
  'h3c: bus.rdata <= {{32-  1{1'b0}}, cfg_byp};
  'h40: bus.rdata <=                  cfg_faa ;
  'h44: bus.rdata <=                  cfg_fbb ;
  'h48: bus.rdata <=                  cfg_fkk ;
  'h4c: bus.rdata <=                  cfg_fpp ;
  // default is 'x for better optimization
  default: bus.rdata <= 'x;
endcase

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

// TODO: a proper CIC+FIR filter should be used instead
/*
scope_filter #(
  // stream parameters
  .DTI (DT),
  .DTO (DT)
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
  .ctl_rst  (evn.rst)
);
*/

red_pitaya_dfilt1 filter (
   // ADC
  .adc_clk_i   (tmp_sti.ACLK),
  .adc_rstn_i  (tmp_sti.ARESETn),
  .adc_dat_i   (tmp_sti.TDATA[0][16-1:2]),
  .adc_dat_o   (tmp_stf.TDATA[0][16-1:2]),
   // configuration
  .cfg_aa_i    (cfg_faa),
  .cfg_bb_i    (cfg_fbb),
  .cfg_kk_i    (cfg_fkk),
  .cfg_pp_i    (cfg_fpp)
);

assign tmp_sti.TREADY = 1'b1;

assign tmp_stf.TVALID = 1'b1;
assign tmp_stf.TLAST  = 1'b0;
assign tmp_stf.TKEEP  = '1;
assign tmp_stf.TDATA[0][1:0] = '0;

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
  .DTI (DT),
  .DTO (DT),
  // decimation parameters
  .DCW (17),
  .DSW ( 4)
) dec_avg (
  // control
  .ctl_rst  (evn.rst),
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

osc_trg #(
  // stream parameters
  .DT (DT)
) osc_trg (
  // control
  .ctl_rst  (evn.rst),
  // configuration
  .cfg_low  (cfg_low),
  .cfg_upp  (cfg_upp),
  .cfg_edg  (cfg_edg),
  // output triggers
  .sts_trg  (tro),
  // stream monitor
  .sti      (std),
  .sto      (ste)
);

////////////////////////////////////////////////////////////////////////////////
// aquire and trigger status handler
////////////////////////////////////////////////////////////////////////////////

always_ff @(posedge bus.clk)
if (~bus.rstn)  evn <= '0;
else            evn <= evi[cfg_evn];

assign ctl_rst = evn.rst;
assign evs.rst = 1'b0;

assign ctl_trg = evn.swt | |(trg & cfg_trg);

acq #(
  .DN (DN),
  .DT (DT),
  .CW (CW)
) acq (
  // stream input/output
  .sti      (ste),
  .sto      (sto),
  // control
  .ctl_rst  (evn.rst),
  // control/status start
  .ctl_str  (evn.str),
  .sts_str  (evs.str),
  // control/status stop
  .ctl_stp  (evn.stp),
  .sts_stp  (evs.stp),
  // control/status trigger
  .ctl_trg  (ctl_trg),
  .sts_trg  (evs.swt),
  // events
  .evn_lst  (irq),
  // configuration/status pre trigger
  .cfg_pre  (cfg_pre),
  .sts_pre  (sts_pre),
  .sts_pro  (sts_pro),
  // configuration/status post trigger
  .cfg_pst  (cfg_pst),
  .sts_pst  (sts_pst),
  .sts_pso  (sts_pso)
);

endmodule: osc
