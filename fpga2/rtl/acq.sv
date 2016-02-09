////////////////////////////////////////////////////////////////////////////////
// Red Pitaya oscilloscope application, used for capturing ADC data into BRAMs,
// which can be later read by SW.
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module acq #(
  int unsigned TN = 1,   // trigger number
  int unsigned TW = 32,  // time width
  int unsigned CW = 32   // counter width
)(
  // stream input/output
  str_bus_if.d  sti,
  str_bus_if.s  sto,
  // current time stamp
  input  logic [TW-1:0] cts,
  // interrupts
  output logic          irq_trg,  // trigger
  output logic          irq_stp,  // stop
  // control
  input  logic          ctl_rst,
  // configuration (mode)
  input  logic [TN-1:0] cfg_trg,  // trigger mask
  input  logic          cfg_con,  // continuous
  input  logic          cfg_aut,  // automatic
  // configuration/status pre trigger
  input  logic [CW-1:0] cfg_pre,
  output logic [CW-1:0] sts_pre,
  // configuration/status post trigger
  input  logic [CW-1:0] cfg_pst,
  output logic [CW-1:0] sts_pst,
  // control/status/timestamp acquire
  input  logic          ctl_acq,  // acquire start
  output logic          sts_acq,
  output logic [TW-1:0] cts_acq,
  // control/status/timestamp trigger
  input  logic [TN-1:0] ctl_trg,
  output logic          sts_trg,
  output logic [TW-1:0] cts_trg,
  // control/status/timestamp stop
  input  logic          ctl_stp,  // acquire stop
  output logic [TW-1:0] cts_stp
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

logic ena_pre;
logic sts_stp;
logic trg;

logic [CW-1:0] nxt_pre;
logic [CW-1:0] nxt_pst;

logic end_pre;
logic end_pst;

////////////////////////////////////////////////////////////////////////////////
// aquire and trigger status handler
////////////////////////////////////////////////////////////////////////////////

assign sts_stp = sts_acq & ( ctl_stp
               | (sts_trg & end_pst & ~cfg_con)
               | (sti.trn & sti.lst) );

assign trg = |(ctl_trg & cfg_trg)
           & (sts_acq & ena_pre & ~sts_trg);

always @(posedge sti.clk)
if (~sti.rstn) begin
  // status pre/post trigger
  ena_pre <= 1'b0;
  sts_pre <= '0;
  sts_pst <= '0;
  // control/status/timestamp acquire
  sts_acq <= 1'b0;
  cts_acq <= '0;
  // control/status/timestamp trigger
  sts_trg <= 1'b0;
  cts_trg <= '0;
  // control/status/timestamp stop
  cts_stp <= '0;
end else begin
  if (ctl_rst) begin
    // status pre/post trigger
    ena_pre <= 1'b0;
    sts_pre <= '0;
    sts_pst <= '0;
    // control/status/timestamp acquire
    sts_acq <= 1'b0;
    cts_acq <= '0;
    // control/status/timestamp trigger
    sts_trg <= 1'b0;
    cts_trg <= '0;
    // control/status/timestamp stop
    cts_stp <= '0;
  end else begin
    // acquire stop/start
    if (sts_stp) begin
      sts_acq <= 1'b0;
      cts_stp <= cts;
    end else if (ctl_acq) begin
      sts_acq <= 1'b1;
      cts_acq <= cts;
      sts_trg <= cfg_aut;
      ena_pre <= ~|cfg_pre;
      sts_pre <= '0;
      sts_pst <= '0;
    end
    // pre counter trigger enable
    if (end_pre)
      ena_pre <= 1'b1;
    // trigger
    if (trg) begin
      sts_trg <= 1'b1;
      cts_trg <= cts;
    end
    // pre and post trigger counters
    if (sts_acq & sti.trn) begin
      if (~sts_trg)  sts_pre <= nxt_pre; // TODO: add out of range
      if ( sts_trg)  sts_pst <= nxt_pst; // TODO: add out of range
    end
  end
end

// next counter values
assign nxt_pre = sts_pre + 1;
assign nxt_pst = sts_pst + 1;

// counter ends
assign end_pre = (nxt_pre == cfg_pre);
assign end_pst = (nxt_pst == cfg_pst);

// interrupts
assign irq_trg = trg;  // trigger
assign irq_stp = sts_stp;  // stop

////////////////////////////////////////////////////////////////////////////////
// output stream
////////////////////////////////////////////////////////////////////////////////

assign sti.rdy = sto.rdy | ~sto.vld;

// output valid
always @(posedge sti.clk)
if (~sti.rstn) begin
  sto.vld <= 1'b0;
  sto.lst <= 1'b0;
end else begin
  sto.vld <= sts_acq & sti.vld;
  sto.lst <= sts_acq & (sti.lst | end_pst);
end

// output data
always @(posedge sti.clk)
if (sts_acq) begin
  sto.dat <= sti.dat;
  sto.kep <= sti.kep; // TODO
end

endmodule: acq
