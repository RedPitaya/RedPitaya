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
  int unsigned DW = 14,  // data width
  int unsigned CW = 32,  // counter width
  // data bus type
  type DAT_T = logic signed [DW-1:0]
);

// system signals
logic          clk ;  // clock
logic          rstn;  // reset - active low

// control
logic          ctl_rst;
// current time stamp
logic [TW-1:0] ctl_cts;
logic [TW-1:0] sts_cts;
// delay configuration/status
logic [CW-1:0] cfg_dly;
logic [CW-1:0] sts_dly;
// acquire control/status
logic          ctl_acq;
logic          sts_acq;
// trigger control/status
logic          ctl_trg;
logic          sts_trg;

// stream input/output
str_bus_if #(.DAT_T (DAT_T)) sti (.clk (clk), .rstn (rstn));
str_bus_if #(.DAT_T (DAT_T)) sto (.clk (clk), .rstn (rstn));

////////////////////////////////////////////////////////////////////////////////
// clock and time stamp
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial                ctl_cts  = 0;
always @ (posedge clk) ctl_cts <= ctl_cts+1;

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

initial begin
  // for now initialize configuration to an idle value
  ctl_rst = 1'b0;
  cfg_dly = 0;
  ctl_acq = 1'b0;
  ctl_trg = 1'b0;

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

str_src #(
  .DAT_T (DAT_T)
) str_src (
  .str      (sti)
);

acq #(
  .TW (TW),
  .DW (DW),
  .CW (CW)
) acq (
  // stream input/output
  .sti      (sti),
  .sto      (sto),
  // control
  .ctl_rst  (ctl_rst),
  // current time stamp
  .ctl_cts  (ctl_cts),
  .sts_cts  (sts_cts),
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

str_drn #(
  .DAT_T (DAT_T)
) str_drn (
  .str      (sto)
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("acq_tb.vcd");
  $dumpvars(0, acq_tb);
end

endmodule: acq_tb
