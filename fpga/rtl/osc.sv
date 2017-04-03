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
  type DTL = logic,
  type DTT = evn_pkg::evt_t,
  type DTE = evn_pkg::evd_t
)(
  // streams
  axi4_stream_if.d      sti,  // input
  axi4_stream_if.s      sto,  // output
  // events input/output
  input  DTE            evi,  // input
  output evn_pkg::evs_t evo,  // output
  // reset output
  output logic          ctl_rst,
  // interrupt
  output logic          irq,
  // system bus
  sys_bus_if.s          bus
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// streams
axi4_stream_if #(.DT (DT)) stf (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from filter
axi4_stream_if #(.DT (DT)) std (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from decimator
axi4_stream_if #(.DT (DT)) ste (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from edge detection

// event select masks
DTL             cfg_rst;  // reset
DTL             cfg_str;  // start
DTL             cfg_stp;  // stop
DTT             cfg_trg;  // trigger

// interrupt enable/status/clear
logic   [2-1:0] irq_ena;  // enable
logic   [2-1:0] irq_sts;  // status

// control
//logic           ctl_rst;
// control/status start
logic           ctl_str;
logic           sts_str;
// control/status stop
logic           ctl_stp;
logic           sts_stp;
// control/status trigger
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
DT              cfg_neg;  // negative level
DT              cfg_pos;  // positive level
logic           cfg_edg;  // edge (0-pos, 1-neg)
logic  [CW-1:0] cfg_hld;  // hold off time

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
  // interrupt enable
  irq_ena <= '0;
  // event masks
  cfg_rst <= '0;
  cfg_str <= '0;
  cfg_stp <= '0;
  cfg_trg <= '0;
  // configuration
  cfg_pre <= '0;
  cfg_pst <= '0;
  // edge detection
  cfg_neg <= '0;
  cfg_pos <= '0;
  cfg_edg <= '0;
  cfg_hld <= '0;
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
    // interrupt enable (status/clear are elsewhere)
    if (bus.addr[BAW-1:0]=='h08)  irq_ena <= bus.wdata[  2-1:0];
    // event masks
    if (bus.addr[BAW-1:0]=='h10)  cfg_rst <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h14)  cfg_str <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h18)  cfg_stp <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h1c)  cfg_trg <= bus.wdata;
    // trigger pre/post time
    if (bus.addr[BAW-1:0]=='h20)  cfg_pre <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h24)  cfg_pst <= bus.wdata;
    // edge detection
    if (bus.addr[BAW-1:0]=='h30)  cfg_neg <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h34)  cfg_pos <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h38)  cfg_edg <= bus.wdata[      0];
    if (bus.addr[BAW-1:0]=='h3c)  cfg_hld <= bus.wdata[ CW-1:0];
    // dacimation/filter
    if (bus.addr[BAW-1:0]=='h40)  cfg_dec <= bus.wdata[DCW-1:0];
    if (bus.addr[BAW-1:0]=='h44)  cfg_shr <= bus.wdata[DSW-1:0];
    if (bus.addr[BAW-1:0]=='h48)  cfg_avg <= bus.wdata[      0];
    if (bus.addr[BAW-1:0]=='h4c)  cfg_byp <= bus.wdata[      0];
    if (bus.addr[BAW-1:0]=='h50)  cfg_faa <= bus.wdata[ 18-1:0];
    if (bus.addr[BAW-1:0]=='h54)  cfg_fbb <= bus.wdata[ 25-1:0];
    if (bus.addr[BAW-1:0]=='h58)  cfg_fkk <= bus.wdata[ 25-1:0];
    if (bus.addr[BAW-1:0]=='h5c)  cfg_fpp <= bus.wdata[ 25-1:0];
  end
end

// control signals
always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  evo.rst <= 1'b0;
  evo.str <= 1'b0;
  evo.stp <= 1'b0;
  evo.swt <= 1'b0;
end else begin
  if (bus.wen & (bus.addr[BAW-1:0]=='h00)) begin
    evo.rst <= bus.wdata[0];  // reset
    evo.str <= bus.wdata[1];  // start
    evo.stp <= bus.wdata[2];  // stop
    evo.swt <= bus.wdata[3];  // trigger
  end else begin
    evo.rst <= 1'b0;
    evo.str <= 1'b0;
    evo.stp <= 1'b0;
    evo.swt <= 1'b0;
  end
end

// read access
always_ff @(posedge bus.clk)
casez (bus.addr[BAW-1:0])
  // control
  'h00: bus.rdata <= {sts_trg, sts_stp, sts_str, 1'b0};
  // interrupts enable/status/clear
  'h08: bus.rdata <= irq_ena;
  'h0c: bus.rdata <= irq_sts;
  // event masks
  'h10: bus.rdata <= cfg_rst;
  'h14: bus.rdata <= cfg_str;
  'h18: bus.rdata <= cfg_stp;
  'h1c: bus.rdata <= cfg_trg;
  // trigger pre/post time
  'h20: bus.rdata <=              32'(cfg_pre);
  'h24: bus.rdata <=              32'(cfg_pst);
  'h28: bus.rdata <=    {sts_pro, 31'(sts_pre)};
  'h2c: bus.rdata <=    {sts_pso, 31'(sts_pst)};
  // edge detection
  'h30: bus.rdata <=                  cfg_neg ;
  'h34: bus.rdata <=                  cfg_pos ;
  'h38: bus.rdata <=              32'(cfg_edg);
  'h3c: bus.rdata <=              32'(cfg_hld);
  // decimation/filter
  'h40: bus.rdata <= {{32-DCW{1'b0}}, cfg_dec};
  'h44: bus.rdata <= {{32-DSW{1'b0}}, cfg_shr};
  'h48: bus.rdata <= {{32-  1{1'b0}}, cfg_avg};
  'h4c: bus.rdata <= {{32-  1{1'b0}}, cfg_byp};
  'h50: bus.rdata <=                  cfg_faa ;
  'h54: bus.rdata <=                  cfg_fbb ;
  'h58: bus.rdata <=                  cfg_fkk ;
  'h5c: bus.rdata <=                  cfg_fpp ;
  // default is 'x for better optimization
  default: bus.rdata <= 'x;
endcase

// interrupt status/clear
always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  irq_sts <= '0;
end else begin
  if (ctl_rst) begin
    irq_sts <= '0;
  end else if (bus.wen & (bus.addr[BAW-1:0]=='h0c)) begin
    // interrupt clear
    irq_sts <= irq_sts & ~bus.wdata[3-1:0];
  end else begin
    // interrupt set
    irq_sts <= irq_sts | {evo.lst, evo.trg} & irq_ena;
  end
end

// interrupt output
always_ff @(posedge bus.clk)
if (~bus.rstn)  irq <= '0;
else            irq <= |irq_sts;

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

//localparam int unsigned DWI = $bits(DT);  // data width for input
//localparam int unsigned DWO = $bits(DT);  // data width for output

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
  .ctl_rst  (ctl_rst)
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
  .DT (DT),
  .CW (CW)
) edge_i (
  // control
  .ctl_rst  (ctl_rst),
  // configuration
  .cfg_edg  (cfg_edg),
  .cfg_neg  (cfg_neg),
  .cfg_pos  (cfg_pos),
  .cfg_hld  (cfg_hld),
  // output triggers
  .sts_trg  (evo.trg),
  // stream monitor
  .sti      (std),
  .sto      (ste)
);

////////////////////////////////////////////////////////////////////////////////
// aquire and trigger status handler
////////////////////////////////////////////////////////////////////////////////

assign ctl_rst = |(evi.rst & cfg_rst);
assign ctl_str = |(evi.str & cfg_str);
assign ctl_stp = |(evi.stp & cfg_stp);
assign ctl_trg = |(evi.trg & cfg_trg);

acq #(
  .DN (DN),
  .DT (DT),
  .CW (CW)
) acq (
  // stream input/output
  .sti      (ste),
  .sto      (sto),
  // control
  .ctl_rst  (ctl_rst),
  // control/status start
  .ctl_str  (ctl_str),
  .sts_str  (sts_str),
  // control/status stop
  .ctl_stp  (ctl_stp),
  .sts_stp  (sts_stp),
  // control/status trigger
  .ctl_trg  (ctl_trg),
  .sts_trg  (sts_trg),
  // events
  .evn_lst  (evo.lst),
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
