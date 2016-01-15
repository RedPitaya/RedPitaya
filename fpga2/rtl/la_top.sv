////////////////////////////////////////////////////////////////////////////////
// Module: Logic Analyzer
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module la_top #(
  // stream parameters
  type DAT_T = logic [8-1:0],
  // decimation parameters
  int unsigned DCW = 17,  // decimation counter width
  // aquisition parameters
  int unsigned CW = 17,  // counter width
  // trigger parameters
  int unsigned TN =  4,  // trigger number
  // timestamp parameters
  int unsigned TW = 64   // timestamp width
)(
  // streams
  str_bus_if.d           sti,      // input
  str_bus_if.s           sto,      // output
  // triggers
  input  logic  [TN-1:0] trg_ext,  // external input
  output logic           trg_swo,  // output from software
  output logic           trg_out,  // output from edge detection
  // System bus
  sys_bus_if.s           bus
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// streams
str_bus_if #(.DAT_T (DAT_T)) std (.clk (sti.clk), .rstn (sti.rstn));  // from decimator

// acquire regset

// current time stamp
logic  [TW-1:0] cts;
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
logic           ctl_trg;
logic           sts_trg;
logic  [TW-1:0] cts_trg;
// control/status/timestamp stop
logic           ctl_stp;  // acquire stop
logic  [TW-1:0] cts_stp;

// trigger
logic  [TN-1:0] cfg_trg;  // trigger select

// trigger source configuration
DAT_T           cfg_old_val;  // old     value
DAT_T           cfg_old_msk;  // old     mask
DAT_T           cfg_cur_val;  // current value
DAT_T           cfg_cur_msk;  // current mask

// decimation configuration
logic [DCW-1:0] cfg_dec;  // decimation factor

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

  // trigger detection
  cfg_old_val <= '0;
  cfg_old_msk <= '0;
  cfg_cur_val <= '0;
  cfg_cur_msk <= '0;

  // filter/dacimation
  cfg_dec <= '0;
end else begin
  if (bus.wen) begin
    // acquire regset
    if (bus.addr[BAW-1:0]=='h04)   cfg_con <= bus.wdata[0];
    if (bus.addr[BAW-1:0]=='h04)   cfg_aut <= bus.wdata[1];
    if (bus.addr[BAW-1:0]=='h08)   cfg_trg <= bus.wdata[TN-1:0];
    if (bus.addr[BAW-1:0]=='h10)   cfg_pre <= bus.wdata[CW-1:0];
    if (bus.addr[BAW-1:0]=='h14)   cfg_pst <= bus.wdata[CW-1:0];

    // trigger detection
    if (bus.addr[BAW-1:0]=='h40)   cfg_old_val <= DAT_T'(bus.wdata);
    if (bus.addr[BAW-1:0]=='h44)   cfg_old_msk <= DAT_T'(bus.wdata);
    if (bus.addr[BAW-1:0]=='h48)   cfg_cur_val <= DAT_T'(bus.wdata);
    if (bus.addr[BAW-1:0]=='h4c)   cfg_cur_msk <= DAT_T'(bus.wdata);

    // dacimation
    if (bus.addr[BAW-1:0]=='h50)   cfg_dec <= bus.wdata[DCW-1:0];
  end
end

// control signals
assign trg_swo = bus.wen & (bus.addr[BAW-1:0]=='h00) & bus.wdata[3];  // trigger
assign ctl_acq = bus.wen & (bus.addr[BAW-1:0]=='h00) & bus.wdata[1];  // acquire start
assign ctl_stp = bus.wen & (bus.addr[BAW-1:0]=='h00) & bus.wdata[2];  // acquire stop
assign ctl_rst = bus.wen & (bus.addr[BAW-1:0]=='h00) & bus.wdata[0];  // reset

// read access
always_ff @(posedge bus.clk)
begin
  casez (bus.addr[BAW-1:0])
    // acquire regset
    'h00 : bus.rdata <= {{32-  4{1'b0}}, sts_trg, ~sts_acq, sts_acq, 1'b0};
    'h04 : bus.rdata <= {{32-  2{1'b0}}, cfg_aut, cfg_con};
    'h08 : bus.rdata <= {{32- TN{1'b0}}, cfg_trg};
    'h10 : bus.rdata <= {{32- CW{1'b0}}, cfg_pre};
    'h14 : bus.rdata <= {{32- CW{1'b0}}, cfg_pst};
    'h18 : bus.rdata <= {{32- CW{1'b0}}, sts_pre};
    'h1c : bus.rdata <= {{32- CW{1'b0}}, sts_pst};
    'h20 : bus.rdata <=              32'(cts_acq >>  0);
    'h24 : bus.rdata <=              32'(cts_acq >> 32);
    'h28 : bus.rdata <=              32'(cts_trg >>  0);
    'h2c : bus.rdata <=              32'(cts_trg >> 32);
    'h30 : bus.rdata <=              32'(cts_stp >>  0);
    'h34 : bus.rdata <=              32'(cts_stp >> 32);

    // trigger detection
    'h40 : bus.rdata <=                  cfg_old_val;
    'h44 : bus.rdata <=                  cfg_old_msk;
    'h48 : bus.rdata <=                  cfg_cur_val;
    'h4c : bus.rdata <=                  cfg_cur_msk;

    // decimation
    'h50 : bus.rdata <= {{32-DCW{1'b0}}, cfg_dec};

    default : bus.rdata <= '0;
  endcase
end

////////////////////////////////////////////////////////////////////////////////
// Decimation
////////////////////////////////////////////////////////////////////////////////

str_dec #(
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
// Edge detection (trigger source)
////////////////////////////////////////////////////////////////////////////////

la_trigger #(
  .DAT_T (DAT_T)
) trigger (
  // control
  .ctl_rst  (ctl_rst),
  // configuration
  .cfg_old_val (cfg_old_val),
  .cfg_old_msk (cfg_old_msk),
  .cfg_cur_val (cfg_cur_val),
  .cfg_cur_msk (cfg_cur_msk),
  // output triggers
  .sts_trg  (trg_out),
  // stream monitor
  .str      (std)
);

////////////////////////////////////////////////////////////////////////////////
// aquire and trigger status handler
////////////////////////////////////////////////////////////////////////////////

assign ctl_trg = |(trg_ext & cfg_trg);

acq #(
  .TW (TW),
  .CW (CW)
) acq (
  // stream input/output
  .sti      (std),
  .sto      (sto),
  // current time stamp
  .cts      (cts),
  // control
  .ctl_rst  (ctl_rst),
  // configuration (mode)
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
  .ctl_trg  (ctl_trg),
  .sts_trg  (sts_trg),
  .cts_trg  (cts_trg),
  // control/status/timestamp stop
  .ctl_stp  (ctl_stp),
  .cts_stp  (cts_stp)
);

endmodule: la_top
