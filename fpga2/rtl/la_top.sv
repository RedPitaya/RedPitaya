////////////////////////////////////////////////////////////////////////////////
// Module: Logic Analyzer
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module la_top #(
  // stream parameters
  type DAT_T = logic [8-1:0],
  int unsigned DWI = 16,  // data width for input
  int unsigned DWO = 14,  // data width for output
  // decimation parameters
  int unsigned DWC = 17,  // data width for counter
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
str_bus_if #(.DAT_T (DAT_T)) stf (.clk (sti.clk), .rstn (sti.rstn));  // from filter
str_bus_if #(.DAT_T (DAT_T)) std (.clk (sti.clk), .rstn (sti.rstn));  // from decimator

// control
logic                  ctl_rst;  // synchronous clear
logic                  ctl_acq;  // start acquire run
// status
logic                  sts_acq;  // acquire status
// trigger
logic        [TWS-1:0] cfg_sel;  // trigger select
logic        [ 32-1:0] cfg_dly;  // delay value
logic        [ 32-1:0] sts_dly;  // delay counter
// trigger source configuration
DAT_T                  cfg_old_val;  // old     value
DAT_T                  cfg_old_msk;  // old     mask
DAT_T                  cfg_cur_val;  // current value
DAT_T                  cfg_cur_msk;  // current mask
// decimation configuration
logic        [DWC-1:0] cfg_dec;  // decimation factor

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
  // trigger
  cfg_sel <= '0;
  cfg_dly <= '0;
  // trigger detection
  cfg_old_val <= '0;
  cfg_old_msk <= '0;
  cfg_cur_val <= '0;
  cfg_cur_msk <= '0;
  // filter/dacimation
  cfg_dec <= '0;
end else begin
  if (bus.wen) begin
    // trigger
    if (bus.addr[6-1:0]==6'h08)   cfg_sel <= bus.wdata[TWS-1:0];
    if (bus.addr[6-1:0]==6'h0c)   cfg_dly <= bus.wdata[ 32-1:0];
    // trigger detection
    if (bus.addr[6-1:0]==6'h10)   cfg_old_val <= DAT_T'(bus.wdata);
    if (bus.addr[6-1:0]==6'h14)   cfg_old_msk <= DAT_T'(bus.wdata);
    if (bus.addr[6-1:0]==6'h18)   cfg_cur_val <= DAT_T'(bus.wdata);
    if (bus.addr[6-1:0]==6'h1c)   cfg_cur_msk <= DAT_T'(bus.wdata);
    // filter/dacimation
    if (bus.addr[6-1:0]==6'h28)   cfg_dec <= bus.wdata[DWC-1:0];
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
    // trigger
    6'h08 : bus.rdata <= {{32-TWS{1'b0}}, cfg_sel}; 
    6'h0c : bus.rdata <=                  cfg_dly ;
    // trigger detection
    6'h10 : bus.rdata <=                  cfg_old_val;
    6'h14 : bus.rdata <=                  cfg_old_msk;
    6'h18 : bus.rdata <=                  cfg_cur_val;
    6'h1c : bus.rdata <=                  cfg_cur_msk;
    // filter/decimation
    6'h28 : bus.rdata <= {{32-DWC{1'b0}}, cfg_dec};

    default:bus.rdata <=  32'h0                   ;
  endcase
end

////////////////////////////////////////////////////////////////////////////////
// Decimation
////////////////////////////////////////////////////////////////////////////////

str_dec #(
  .CW (DWC)
) dec (
  // control
  .ctl_rst  (ctl_rst),
  // configuration
  .cfg_dec  (cfg_dec),
  // streams
  .sti      (stf),
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

endmodule: la_top
