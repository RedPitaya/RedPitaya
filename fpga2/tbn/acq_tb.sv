////////////////////////////////////////////////////////////////////////////////
// Module: Acquire
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module acq_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // parameters
  int unsigned TW = 64,  // time width
  int unsigned CW = 32,  // counter width
  // data bus type
  type DAT_T = logic signed [14-1:0]
);

// system signals
logic          clk ;  // clock
logic          rstn;  // reset - active low

// current time stamp
logic [TW-1:0] cts;
// control
logic          ctl_rst;
// configuration (mode)
logic          cfg_con;  // continuous
logic          cfg_aut;  // automatic
// configuration/status pre trigger
logic [CW-1:0] cfg_pre;
logic [CW-1:0] sts_pre;
// configuration/status post trigger
logic [CW-1:0] cfg_pst;
logic [CW-1:0] sts_pst;
// control/status/timestamp acquire
logic          ctl_acq;  // acquire start
logic          sts_acq;
logic [TW-1:0] cts_acq;
// control/status/timestamp trigger
logic          ctl_trg;
logic          sts_trg;
logic [TW-1:0] cts_trg;
// control/status/timestamp stop
logic          ctl_stp;  // acquire stop
logic [TW-1:0] cts_stp;

// stream input/output
str_bus_if #(.DAT_T (DAT_T)) sti (.clk (clk), .rstn (rstn));
str_bus_if #(.DAT_T (DAT_T)) sto (.clk (clk), .rstn (rstn));

////////////////////////////////////////////////////////////////////////////////
// clock and time stamp
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial                cts  = 0;
always @ (posedge clk) cts <= cts + 1;

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

initial begin
  // for now initialize configuration to an idle value
  ctl_rst = 1'b0;
  cfg_con = 1'b0;
  cfg_aut = 1'b0;
  cfg_pre = 0;
  cfg_pst = 0;
  ctl_acq = 1'b0;
  ctl_trg = 1'b0;
  ctl_stp = 1'b0;

  // initialization
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  // start
  rstn = 1'b1;
  repeat(4) @(posedge clk);

  // send data into stream
  src_inc (-8, 8);
  #0; wait (!str_src.buf_siz);
  repeat(1) @(posedge clk);

  // activate acquire
  ctl_acq = 1'b1;
  repeat(1) @(posedge clk);
  ctl_acq = 1'b0;

  // send data into stream
  src_inc (-8, 8);
  #0; wait (!str_src.buf_siz);
  repeat(3) @(posedge clk);
  // check data from stream
  drn_inc (-8, 8);

  // end simulation
  repeat(4) @(posedge clk);
  $finish();
end

// source incremental sequence
task src_inc (
  int from,
  int to
);
  for (int i=from; i<=to; i++) begin
    str_src.put(i, i==to);
  end
endtask

// drain check incremental sequence
task drn_inc (
  int from,
  int to
);
  DAT_T        dat;
  logic        lst;
  int unsigned tmg;
  for (int i=from; i<=to; i++) begin
    str_drn.get(dat, lst, tmg);
      $display ("data %d is %x/%b", i, dat, lst);
    if ((dat !== DAT_T'(i)) || (lst !== 1'(i==to))) begin
      $display ("data %d is %x/%b, should be %x/%b", i, dat, lst, DAT_T'(i), 1'(i==to));
    end
  end
endtask

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

str_src #(.DAT_T (DAT_T)) str_src (.str (sti));

acq #(
  .TW (TW),
  .CW (CW)
) acq (
  // stream input/output
  .sti      (sti),
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
  .ctl_acq  (ctl_acq),  // acquire start
  .sts_acq  (sts_acq),
  .cts_acq  (cts_acq),
  // control/status/timestamp trigger
  .ctl_trg  (ctl_trg),
  .sts_trg  (sts_trg),
  .cts_trg  (cts_trg),
  // control/status/timestamp stop
  .ctl_stp  (ctl_stp),  // acquire stop
  .cts_stp  (cts_stp)
);

str_drn #(.DAT_T (DAT_T)) str_drn (.str (sto));

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("acq_tb.vcd");
  $dumpvars(0, acq_tb);
end

endmodule: acq_tb
