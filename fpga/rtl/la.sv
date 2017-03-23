////////////////////////////////////////////////////////////////////////////////
// Module: Logic Analyzer
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module la #(
  // stream parameters
  int unsigned DN = 1,   // data number
  type DT = logic [8-1:0],
  // decimation parameters
  int unsigned DCW = 17,  // decimation counter width
  // aquisition parameters
  int unsigned CW  = 32-1,  // counter width
  // event parameters
  int unsigned EW  =  6   // external trigger array  width
)(
  // streams
  axi4_stream_if.d       sti,      // input
  axi4_stream_if.s       sto,      // output
  // external events
  input  logic  [EW-1:0] evn_ext,
  // event sources
  output logic           evn_rst,  // software reset
  output logic           evn_str,  // software start
  output logic           evn_stp,  // software stop
  output logic           evn_trg,  // software trigger
  output logic           evn_lvl,  // level/edge
  output logic           evn_lst,  // last
  // reset output
  output logic           ctl_rst,
  // interrupt
  output logic           irq,
  // system bus
  sys_bus_if.s           bus
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// streams
axi4_stream_if #(.DN (DN), .DT (DT)) stn            (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from negator
axi4_stream_if #(.DN (DN), .DT (DT)) std            (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from decimator
axi4_stream_if #(.DN (DN), .DT (DT)) stt            (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from trigger
axi4_stream_if #(.DN (DN), .DT (DT)) sta_str        (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from acquire
axi4_stream_if #(.DN (DN), .DT (logic [8-1:0])) sta (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from acquire

// acquire regset

// event select masks
logic  [EW-1:0] cfg_rst;  // reset
logic  [EW-1:0] cfg_str;  // start
logic  [EW-1:0] cfg_stp;  // stop
logic  [EW-1:0] cfg_trg;  // trigger

// interrupt enable/status/clear
logic   [4-1:0] irq_ena;  // enable
logic   [4-1:0] irq_sts;  // status
logic   [4-1:0] irq_clr;  // clear

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

// trigger source configuration
DT              cfg_cmp_msk;  // comparator mask
DT              cfg_cmp_val;  // comparator value
DT              cfg_edg_pos;  // edge positive
DT              cfg_edg_neg;  // edge negative

// decimation configuration
logic [DCW-1:0] cfg_dec;  // decimation factor

// RLE configuration
logic           cfg_rle;  // RLE enable

// stream counter staus
logic  [CW-1:0] sts_cur;  // current     counter status
logic  [CW-1:0] sts_lst;  // last packet counter status

// bitwise input polarity
DT              cfg_pol;

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
  // trigger detection
  cfg_cmp_msk <= '0;
  cfg_cmp_val <= '0;
  cfg_edg_pos <= '0;
  cfg_edg_neg <= '0;
  // filter/dacimation
  cfg_dec <= '0;
  // RLE
  cfg_rle <= 1'b0;
  // bitwise input polarity
  cfg_pol <= '0;
end else begin
  if (bus.wen) begin
    // interrupt enable (status/clear are elsewhere)
    if (bus.addr[BAW-1:0]=='h08)  irq_ena <= bus.wdata[  3-1:0];
    // event masks
    if (bus.addr[BAW-1:0]=='h10)  cfg_rst <= bus.wdata[ EW-1:0];
    if (bus.addr[BAW-1:0]=='h14)  cfg_str <= bus.wdata[ EW-1:0];
    if (bus.addr[BAW-1:0]=='h18)  cfg_stp <= bus.wdata[ EW-1:0];
    if (bus.addr[BAW-1:0]=='h1c)  cfg_trg <= bus.wdata[ EW-1:0];
    // trigger pre/post time
    if (bus.addr[BAW-1:0]=='h20)  cfg_pre <= bus.wdata[CW-1:0];
    if (bus.addr[BAW-1:0]=='h24)  cfg_pst <= bus.wdata[CW-1:0];
    // trigger detection
    if (bus.addr[BAW-1:0]=='h30)  cfg_cmp_msk <= DT'(bus.wdata);
    if (bus.addr[BAW-1:0]=='h34)  cfg_cmp_val <= DT'(bus.wdata);
    if (bus.addr[BAW-1:0]=='h38)  cfg_edg_pos <= DT'(bus.wdata);
    if (bus.addr[BAW-1:0]=='h3c)  cfg_edg_neg <= DT'(bus.wdata);
    // dacimation
    if (bus.addr[BAW-1:0]=='h40)  cfg_dec <= bus.wdata[DCW-1:0];
    // RLE
    if (bus.addr[BAW-1:0]=='h44)  cfg_rle <= bus.wdata[0];
    // bitwise input polarity
    if (bus.addr[BAW-1:0]=='h50)  cfg_pol <= DT'(bus.wdata);
  end
end

// control signals
always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  evn_rst <= 1'b0;
  evn_str <= 1'b0;
  evn_stp <= 1'b0;
  evn_trg <= 1'b0;
end else begin
  if (bus.wen & (bus.addr[BAW-1:0]=='h00)) begin
    evn_rst <= bus.wdata[0];  // reset
    evn_str <= bus.wdata[1];  // start
    evn_stp <= bus.wdata[2];  // stop
    evn_trg <= bus.wdata[3];  // trigger
  end else begin
    evn_rst <= 1'b0;
    evn_str <= 1'b0;
    evn_stp <= 1'b0;
    evn_trg <= 1'b0;
  end
end

// read access
always_ff @(posedge bus.clk)
casez (bus.addr[BAW-1:0])
  // control
  'h00: bus.rdata <= {{32-  4{1'b0}}, sts_trg, sts_stp, sts_str, 1'b0};
  // interrupts enable/status/clear
  'h08: bus.rdata <=                  irq_ena;
  'h0c: bus.rdata <=                  irq_sts;
  // event masks
  'h10: bus.rdata <= {{32- EW{1'b0}}, cfg_rst};
  'h14: bus.rdata <= {{32- EW{1'b0}}, cfg_str};
  'h18: bus.rdata <= {{32- EW{1'b0}}, cfg_stp};
  'h1c: bus.rdata <= {{32- EW{1'b0}}, cfg_trg};
  // trigger pre/post time
  'h20: bus.rdata <=              32'(cfg_pre);
  'h24: bus.rdata <=              32'(cfg_pst);
  'h28: bus.rdata <=    {sts_pro, 31'(sts_pre)};
  'h2c: bus.rdata <=    {sts_pso, 31'(sts_pst)};
  // trigger detection
  'h30: bus.rdata <=                  cfg_cmp_msk;
  'h34: bus.rdata <=                  cfg_cmp_val;
  'h38: bus.rdata <=                  cfg_edg_pos;
  'h3c: bus.rdata <=                  cfg_edg_neg;
  // decimation
  'h40: bus.rdata <= {{32-DCW{1'b0}}, cfg_dec};
  // RLE configuration
  'h44: bus.rdata <= {{32-  1{1'b0}}, cfg_rle};
  // stream counter status
  'h48: bus.rdata <=              32'(sts_cur);
  'h4c: bus.rdata <=              32'(sts_lst);
  // bitwise input polarity
  'h50: bus.rdata <=              32'(cfg_pol);
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
    irq_sts <= irq_sts | {ctl_trg, ctl_stp, ctl_str, ctl_rst};
  end
end

// interrupt output
always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  irq <= '0;
end else begin
  irq <= |(irq_sts & irq_ena);
end

////////////////////////////////////////////////////////////////////////////////
// Decimation
////////////////////////////////////////////////////////////////////////////////

str_dec #(
  .DN (DN),
  .CW (DCW)
) dec (
  // control
  .ctl_rst  (ctl_rst),
  // configuration
  .cfg_dec  (cfg_dec),
  // streams
  .sti      (sti),
  .sto      (std)
);

////////////////////////////////////////////////////////////////////////////////
// bitwise input polarity
////////////////////////////////////////////////////////////////////////////////

assign stn.TDATA  = std.TDATA ^ cfg_pol;
assign stn.TKEEP  = std.TKEEP ;
assign stn.TLAST  = std.TLAST ;
assign stn.TVALID = std.TVALID;

assign std.TREADY = stn.TREADY;

////////////////////////////////////////////////////////////////////////////////
// Edge detection (trigger source)
////////////////////////////////////////////////////////////////////////////////

la_trigger #(
  .DT (DT)
) trigger (
  // control
  .ctl_rst  (ctl_rst),
  // configuration
  .cfg_cmp_msk (cfg_cmp_msk),
  .cfg_cmp_val (cfg_cmp_val),
  .cfg_edg_pos (cfg_edg_pos),
  .cfg_edg_neg (cfg_edg_neg),
  // output triggers
  .sts_trg  (evn_lvl),
  // stream monitor
  .sti      (stn),
  .sto      (stt)
);

////////////////////////////////////////////////////////////////////////////////
// aquire and trigger status handler
////////////////////////////////////////////////////////////////////////////////

assign ctl_rst = |(evn_ext & cfg_rst);
assign ctl_str = |(evn_ext & cfg_str);
assign ctl_stp = |(evn_ext & cfg_stp);
assign ctl_trg = |(evn_ext & cfg_trg);

acq #(
  .DN (DN),
  .DT (DT),
  .CW (CW)
) acq (
  // stream input/output
  .sti      (stt),
  .sto      (sta_str),
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
  .evn_lst  (evn_lst),
  // configuration/status pre trigger
  .cfg_pre  (cfg_pre),
  .sts_pre  (sts_pre),
  .sts_pro  (sts_pro),
  // configuration/status post trigger
  .cfg_pst  (cfg_pst),
  .sts_pst  (sts_pst),
  .sts_pso  (sts_pso)
);

assign sta.TDATA  = sta_str.TDATA [0][8-1:0];
assign sta.TKEEP  = sta_str.TKEEP ;
assign sta.TLAST  = sta_str.TLAST ;
assign sta.TVALID = sta_str.TVALID;
assign sta_str.TREADY = sta.TREADY;

rle #(
  // counter properties
  .CW (8),
  // stream properties
  .DN (DN),
  .DTI (logic [  8-1:0]),
  .DTO (logic [8+8-1:0])
) rle (
  // input stream input/output
  .sti      (sta),
  .sto      (sto),
  // configuration
  .ctl_rst  (ctl_rst),
  .cfg_ena  (cfg_rle)
);

axi4_stream_cnt #(
  .DN (DN),
  .CW (CW)
) axi4_stream_cnt (
  // control
  .ctl_rst  (ctl_rst),
  // counter staus
  .sts_cur  (sts_cur),
  .sts_lst  (sts_lst),
  // stream monitor
  .str      (sto)
);

endmodule: la
