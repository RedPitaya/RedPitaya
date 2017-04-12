////////////////////////////////////////////////////////////////////////////////
// Module: acquire (start/trigger/stop on a data stream)
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module acq #(
  // data stream
  int unsigned DN = 1,   // data number
  type DT = logic [8-1:0],
  // timer/counter
  int unsigned CW = 32-1   // counter width
)(
  // stream input/output
  axi4_stream_if.d  sti,
  axi4_stream_if.s  sto,
  // events
  output logic          evn_lst,  // last
  // control
  input  logic          ctl_rst,
  // control/status start
  input  logic          ctl_str,
  output logic          sts_str,
  // control/status stop
  input  logic          ctl_stp,
  output logic          sts_stp,
  // control/status trigger
  input  logic          ctl_trg,
  output logic          sts_trg,
  // configuration/status/overflow pre trigger
  input  logic [CW-1:0] cfg_pre,
  output logic [CW-1:0] sts_pre,
  output logic          sts_pro,
  // configuration/status/overflow post trigger
  input  logic [CW-1:0] cfg_pst,
  output logic [CW-1:0] sts_pst,
  output logic          sts_pso
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

logic ena_pre;
logic sts_lst;
logic trg;

// incremented values of pre/post trigger counters
logic [CW-1:0] nxt_pre;
logic [CW-1:0] nxt_pst;
// combinatorial signal indicating in the next cycle
// pre/post trigger counters will reach their target number
logic end_pre;
logic end_pst;

////////////////////////////////////////////////////////////////////////////////
// aquire and trigger status handler
////////////////////////////////////////////////////////////////////////////////

// start/trigger/stop status
assign sts_stp = ~sts_str;

// stop event
assign sts_lst = sts_str & (ctl_stp
               | (sts_trg & end_pst)
               | (sti.transf & sti.TLAST) );

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn) begin
  evn_lst <= 1'b0;
end else begin
  // TODO: make sure it is a proper pulse
  evn_lst <= sts_lst;
end

assign trg = ctl_trg & sts_str & ena_pre & ~sts_trg;

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn) begin
  // status pre/post trigger
  ena_pre <= 1'b0;
  // status counter/overflow pre/post trigger
  sts_pre <= '0;
  sts_pst <= '0;
  sts_pro <= 1'b0;
  sts_pso <= 1'b0;
  // start/trigger/stop status
  sts_str <= 1'b0;
  sts_trg <= 1'b0;
end else begin
  if (ctl_rst) begin
    // status pre/post trigger
    ena_pre <= 1'b0;
    sts_pre <= '0;
    sts_pst <= '0;
    sts_pro <= 1'b0;
    sts_pso <= 1'b0;
    sts_str <= 1'b0;
    sts_trg <= 1'b0;
  end else begin
    // acquire stop/start
    if          (sts_lst) begin
      sts_str <= 1'b0;
    end else if (ctl_str) begin
      sts_str <= 1'b1;
      sts_trg <= ctl_trg;
      ena_pre <= ~|cfg_pre;
      sts_pre <= '0;
      sts_pst <= '0;
      sts_pro <= 1'b0;
      sts_pso <= 1'b0;
    end
    // pre counter trigger enable
    if (end_pre)
      ena_pre <= 1'b1;
    // trigger
    if (trg)
      sts_trg <= 1'b1;
    // pre and post trigger counters
    if (sts_str & sti.transf) begin
      if (~(sts_trg | trg))  begin sts_pre <= nxt_pre; sts_pro <= &sts_pre; end
      if ( (sts_trg | trg))  begin sts_pst <= nxt_pst; sts_pso <= &sts_pst; end
    end
  end
end

// next counter values
assign nxt_pre = sts_pre + 1;
assign nxt_pst = sts_pst + 1;

// counter ends
assign end_pre = (nxt_pre == cfg_pre);
assign end_pst = (nxt_pst == cfg_pst);

////////////////////////////////////////////////////////////////////////////////
// output stream
////////////////////////////////////////////////////////////////////////////////

assign sti.TREADY = sto.TREADY | ~sto.TVALID;

// output valid
always_ff @(posedge sti.ACLK)
if (~sti.ARESETn) begin
  sto.TVALID <= 1'b0;
end else begin
  sto.TVALID <= sts_str & sti.TVALID;
end

// output data
always_ff @(posedge sti.ACLK)
if (sts_str & sti.transf) begin
  sto.TDATA <= sti.TDATA;
  sto.TKEEP <= sti.TKEEP; // TODO
  sto.TLAST <= sti.TLAST | sts_lst;
end    

endmodule: acq
