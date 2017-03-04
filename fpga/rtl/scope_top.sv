////////////////////////////////////////////////////////////////////////////////
// Red Pitaya oscilloscope application, used for capturing ADC data into BRAMs,
// which can be later read by SW.
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module scope_top #(
  // stream parameters
  int unsigned DN = 1,   // data number
  type DT = logic signed [16-1:0],
  // decimation parameters
  int unsigned DCW = 17,  // data width for counter
  int unsigned DSW =  4,  // data width for shifter
  // aquisition parameters
  int unsigned CW = 32,  // counter width
  // event parameters
  int unsigned EW =  4   // external trigger array  width
)(
  // streams
  axi4_stream_if.d       sti,      // input
  axi4_stream_if.s       sto,      // output
  // external events
  input  logic  [EW-1:0] evn_ext,
  // event sources
  output logic           evn_str,  // software start
  output logic           evn_stp,  // software stop
  output logic           evn_trg,  // software trigger
  output logic           evn_lvl,  // level/edge
  output logic           evn_lst,  // last
  // System bus
  sys_bus_if.s           bus
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// streams
axi4_stream_if #(.DT (DT)) stf (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from filter
axi4_stream_if #(.DT (DT)) std (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from decimator
axi4_stream_if #(.DT (DT)) ste (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from edge detection

// acquire regset

// control
logic           ctl_rst;
// control/status start
logic           ctl_str;
logic           sts_str;
// control/status stop
logic           ctl_stp;
logic           sts_stp;
// control/status trigger
logic           ctl_trg;
logic           sts_trg;
// configuration (mode)
logic           cfg_con;  // continuous
logic           cfg_aut;  // automatic
// configuration/status pre trigger
logic  [CW-1:0] cfg_pre;
logic  [CW-1:0] sts_pre;
// configuration/status post trigger
logic  [CW-1:0] cfg_pst;
logic  [CW-1:0] sts_pst;

// event select masks
logic  [EW-1:0] cfg_str;  // start
logic  [EW-1:0] cfg_stp;  // stop
logic  [EW-1:0] cfg_trg;  // trigger

// edge detection configuration
DT                     cfg_pos;  // positive level
DT                     cfg_neg;  // negative level
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
  // event masks
  cfg_trg <= '0;
  cfg_pre <= '0;
  cfg_pst <= '0;
  // edge detection
  cfg_pos <= '0;
  cfg_neg <= '0;
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
    // event masks
    if (bus.addr[BAW-1:0]=='h10)   cfg_str <= bus.wdata[EW-1:0];
    if (bus.addr[BAW-1:0]=='h14)   cfg_stp <= bus.wdata[EW-1:0];
    if (bus.addr[BAW-1:0]=='h18)   cfg_trg <= bus.wdata[EW-1:0];
    // trigger pre/post time
    if (bus.addr[BAW-1:0]=='h20)   cfg_pre <= bus.wdata[CW-1:0];
    if (bus.addr[BAW-1:0]=='h24)   cfg_pst <= bus.wdata[CW-1:0];
    // edge detection
    if (bus.addr[BAW-1:0]=='h30)   cfg_pos <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h34)   cfg_neg <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h38)   cfg_edg <= bus.wdata[      0];
    // dacimation
    if (bus.addr[BAW-1:0]=='h40)   cfg_dec <= bus.wdata[DCW-1:0];
    if (bus.addr[BAW-1:0]=='h44)   cfg_shr <= bus.wdata[DSW-1:0];
    if (bus.addr[BAW-1:0]=='h48)   cfg_avg <= bus.wdata[      0];
    // filter
    if (bus.addr[BAW-1:0]=='h4c)   cfg_byp <= bus.wdata[      0];
    if (bus.addr[BAW-1:0]=='h50)   cfg_faa <= bus.wdata[ 18-1:0];
    if (bus.addr[BAW-1:0]=='h54)   cfg_fbb <= bus.wdata[ 25-1:0];
    if (bus.addr[BAW-1:0]=='h58)   cfg_fkk <= bus.wdata[ 25-1:0];
    if (bus.addr[BAW-1:0]=='h5c)   cfg_fpp <= bus.wdata[ 25-1:0];
  end
end

// control signals
always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  ctl_rst <= 1'b0;
  evn_str <= 1'b0;
  evn_stp <= 1'b0;
  evn_trg <= 1'b0;
end else begin
  if (bus.wen & (bus.addr[BAW-1:0]=='h00)) begin
  ctl_rst <= bus.wdata[0];  // reset
  evn_str <= bus.wdata[1];  // start
  evn_stp <= bus.wdata[2];  // stop
  evn_trg <= bus.wdata[3];  // trigger
  end else begin
    ctl_rst <= 1'b0;
    evn_str <= 1'b0;
    evn_stp <= 1'b0;
    evn_trg <= 1'b0;
  end
end

// read access
always_ff @(posedge bus.clk)
begin
  casez (bus.addr[BAW-1:0])
    // acquire regset
    'h00 : bus.rdata <= {{32-  4{1'b0}}, sts_trg, sts_stp, sts_str, 1'b0};
    'h04 : bus.rdata <= {{32-  2{1'b0}}, cfg_aut, cfg_con};
    // event masks
    'h10 : bus.rdata <= {{32- EW{1'b0}}, cfg_str};
    'h14 : bus.rdata <= {{32- EW{1'b0}}, cfg_stp};
    'h18 : bus.rdata <= {{32- EW{1'b0}}, cfg_trg};
    // trigger pre/post time
    'h20 : bus.rdata <=              32'(cfg_pre);
    'h24 : bus.rdata <=              32'(cfg_pst);
    'h28 : bus.rdata <=              32'(sts_pre);
    'h2c : bus.rdata <=              32'(sts_pst);
    // edge detection
    'h30 : bus.rdata <=                  cfg_pos ;
    'h34 : bus.rdata <=                  cfg_neg ;
    'h38 : bus.rdata <=              32'(cfg_edg);
    // decimation
    'h40 : bus.rdata <= {{32-DCW{1'b0}}, cfg_dec};
    'h44 : bus.rdata <= {{32-DSW{1'b0}}, cfg_shr};
    'h48 : bus.rdata <= {{32-  1{1'b0}}, cfg_avg};
    // filter
    'h4c : bus.rdata <= {{32-  1{1'b0}}, cfg_byp};
    'h50 : bus.rdata <=                  cfg_faa ;
    'h54 : bus.rdata <=                  cfg_fbb ;
    'h58 : bus.rdata <=                  cfg_fkk ;
    'h5c : bus.rdata <=                  cfg_fpp ;
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

//localparam int unsigned DWI = $bits(DT);  // data width for input
//localparam int unsigned DWO = $bits(DT);  // data width for output

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
  .DT (DT)
) edge_i (
  // control
  .ctl_rst  (ctl_rst),
  // configuration
  .cfg_edg  (cfg_edg),
  .cfg_pos  (cfg_pos),
  .cfg_neg  (cfg_neg),
  // output triggers
  .sts_trg  (evn_edg),
  // stream monitor
  .sti      (std),
  .sto      (ste)
);

////////////////////////////////////////////////////////////////////////////////
// aquire and trigger status handler
////////////////////////////////////////////////////////////////////////////////

assign ctl_str = evn_ext & cfg_str;
assign ctl_stp = evn_ext & cfg_stp;
assign ctl_trg = evn_ext & cfg_trg;

acq #(
  .DN (DN),
  .DT (DT),
  .CW (CW)
) acq (
  // stream input/output
  .sti      (ste),
  .sto      (sto),
  // events
  .evn_lst  (evn_lst),
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
  // configuration (mode)
  .cfg_con  (cfg_con),
  .cfg_aut  (cfg_aut),
  // configuration/status pre trigger
  .cfg_pre  (cfg_pre),
  .sts_pre  (sts_pre),
  // configuration/status post trigger
  .cfg_pst  (cfg_pst),
  .sts_pst  (sts_pst)
);

endmodule: scope_top
