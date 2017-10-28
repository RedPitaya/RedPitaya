////////////////////////////////////////////////////////////////////////////////
// Module: Logic Analyzer
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module la #(
  // stream parameters
  int unsigned DN = 1,  // data number
  type DT = logic [8-1:0],
  // decimation parameters
  int unsigned DCW = 17,  // decimation counter width
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
axi4_stream_if #(.DN (DN), .DT (DT)) stn            (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from negator
axi4_stream_if #(.DN (DN), .DT (DT)) std            (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from decimator
axi4_stream_if #(.DN (DN), .DT (DT)) stt            (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from trigger
axi4_stream_if #(.DN (DN), .DT (DT)) sta_str        (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from acquire
axi4_stream_if #(.DN (DN), .DT (logic [8-1:0])) sta (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from acquire

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

// input configuration
DT              cfg_msk;  // bitwise mask
DT              cfg_pol;  // bitwise polarity

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
  cfg_msk <= '0;
  cfg_pol <= '0;
end else begin
  if (bus.wen) begin
    // event select
    if (bus.addr[BAW-1:0]=='h04)  cfg_evn <= bus.wdata;
    // triger mask
    if (bus.addr[BAW-1:0]=='h08)  cfg_trg <= bus.wdata;
    // trigger pre/post time
    if (bus.addr[BAW-1:0]=='h10)  cfg_pre <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h14)  cfg_pst <= bus.wdata;
    // trigger detection
    if (bus.addr[BAW-1:0]=='h20)  cfg_cmp_msk <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h24)  cfg_cmp_val <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h28)  cfg_edg_pos <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h2c)  cfg_edg_neg <= bus.wdata;
    // RLE
    if (bus.addr[BAW-1:0]=='h30)  cfg_rle <= bus.wdata[0];
    // bitwise input polarity
    if (bus.addr[BAW-1:0]=='h40)  cfg_msk <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h44)  cfg_pol <= bus.wdata;
    // dacimation
    if (bus.addr[BAW-1:0]=='h48)  cfg_dec <= bus.wdata;
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
  'h10: bus.rdata <=           32'(cfg_pre);
  'h14: bus.rdata <=           32'(cfg_pst);
  'h18: bus.rdata <= {sts_pro, 31'(sts_pre)};
  'h1c: bus.rdata <= {sts_pso, 31'(sts_pst)};
  // trigger detection
  'h20: bus.rdata <= cfg_cmp_msk;
  'h24: bus.rdata <= cfg_cmp_val;
  'h28: bus.rdata <= cfg_edg_pos;
  'h2c: bus.rdata <= cfg_edg_neg;
  // RLE configuration
  'h30: bus.rdata <= cfg_rle;
  // stream counter status
  'h38: bus.rdata <= sts_cur;
  'h3c: bus.rdata <= sts_lst;
  // bitwise input polarity
  'h40: bus.rdata <= cfg_msk;
  'h44: bus.rdata <= cfg_pol;
  // decimation
  'h48: bus.rdata <= cfg_dec;
  default: bus.rdata <= 'x;
endcase

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

assign stn.TDATA  = (std.TDATA & cfg_msk) ^ cfg_pol;
assign stn.TKEEP  =  std.TKEEP ;
assign stn.TLAST  =  std.TLAST ;
assign stn.TVALID =  std.TVALID;

assign std.TREADY = stn.TREADY;

////////////////////////////////////////////////////////////////////////////////
// Edge detection (trigger source)
////////////////////////////////////////////////////////////////////////////////

la_trg #(
  .DT (DT)
) la_trg (
  // control
  .ctl_rst  (evn.rst),
  // configuration
  .cfg_cmp_msk (cfg_cmp_msk),
  .cfg_cmp_val (cfg_cmp_val),
  .cfg_edg_pos (cfg_edg_pos),
  .cfg_edg_neg (cfg_edg_neg),
  // output triggers
  .sts_trg  (tro),
  // stream monitor
  .sti      (stn),
  .sto      (stt)
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
  .sti      (stt),
  .sto      (sta_str),
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
