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
  int unsigned CW = 32   // counter width
)(
  // stream input/output
  axi4_stream_if.d  sti,
  axi4_stream_if.s  sto,
  // events
  output logic          evn_lst,  // last
  // control
  input  logic          ctl_rst,
  // configuration (mode)
  input  logic          cfg_con,  // continuous
  input  logic          cfg_aut,  // automatic
  // configuration/status pre trigger
  input  logic [CW-1:0] cfg_pre,
  output logic [CW-1:0] sts_pre,
  // configuration/status post trigger
  input  logic [CW-1:0] cfg_pst,
  output logic [CW-1:0] sts_pst,
  // control/status start
  input  logic          ctl_str,
  output logic          sts_str,
  // control/status stop
  input  logic          ctl_stp,
  output logic          sts_stp,
  // control/status trigger
  input  logic          ctl_trg,
  output logic          sts_trg
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

logic ena_pre;
logic sts_acq;
logic sts_lst;
logic trg;

logic [CW-1:0] nxt_pre;
logic [CW-1:0] nxt_pst;

logic end_pre;
logic end_pst;

////////////////////////////////////////////////////////////////////////////////
// aquire and trigger status handler
////////////////////////////////////////////////////////////////////////////////

// start/stop status
assign sts_str =  sts_acq;
assign sts_stp = ~sts_acq;

// stop event
assign sts_lst = sts_acq & (ctl_stp
               | (sts_trg & end_pst & ~cfg_con)
               | (sti.transf & sti.TLAST) );

always @(posedge sti.ACLK)
if (~sti.ARESETn) begin
  evn_lst <= 1'b0;
end else begin
  // TODO: make sure it is a proper pulse
  evn_lst <= sts_lst;
end

assign trg = ctl_trg & sts_acq & ena_pre & ~sts_trg;

always @(posedge sti.ACLK)
if (~sti.ARESETn) begin
  // status pre/post trigger
  ena_pre <= 1'b0;
  sts_pre <= '0;
  sts_pst <= '0;
  sts_acq <= 1'b0;
  sts_trg <= 1'b0;
end else begin
  if (ctl_rst) begin
    // status pre/post trigger
    ena_pre <= 1'b0;
    sts_pre <= '0;
    sts_pst <= '0;
    sts_acq <= 1'b0;
    sts_trg <= 1'b0;
  end else begin
    // acquire stop/start
    if (sts_lst) begin
      sts_acq <= 1'b0;
    end else if (ctl_str) begin
      sts_acq <= 1'b1;
      sts_trg <= cfg_aut;
      ena_pre <= ~|cfg_pre;
      sts_pre <= '0;
      sts_pst <= '0;
    end
    // pre counter trigger enable
    if (end_pre)
      ena_pre <= 1'b1;
    // trigger
    if (trg)
      sts_trg <= 1'b1;
    // pre and post trigger counters
    if (sts_acq & sti.transf) begin
      if (~sts_trg | trg)  sts_pre <= nxt_pre; // TODO: add out of range
      if ( sts_trg | trg)  sts_pst <= nxt_pst; // TODO: add out of range
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

// align data with control signals
axi4_stream_if #(.DN (DN), .DT (DT)) sto_algn (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  
axi4_stream_reg align_reg(.sti (sti), .sto (sto_algn));

assign sto_algn.TREADY = sto.TREADY | ~sto.TVALID;

// output valid
always @(posedge sti.ACLK)
if (~sti.ARESETn) begin
  sto.TVALID <= 1'b0;
end else begin
  sto.TVALID <= sts_acq & sto_algn.TVALID;
end

// output data
always @(posedge sti.ACLK)
if (sts_acq & sto_algn.transf) begin
  sto.TDATA <= sto_algn.TDATA;
  sto.TKEEP <= sto_algn.TKEEP; // TODO
  sto.TLAST <= sto_algn.TLAST | sts_lst;
end    

endmodule: acq
