////////////////////////////////////////////////////////////////////////////////
// Module: Linear transformation (gain, offset and saturation)
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
  int unsigned CW = 32   // counter width
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
str_bus_if #(.DAT_T (logic signed [DW-1:0])) sti (.clk (clk), .rstn (rstn));
str_bus_if #(.DAT_T (logic signed [DW-1:0])) sto (.clk (clk), .rstn (rstn));

// calibration
real gain   = 1.0;
real offset = 0.1;

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
  for (int i=-8; i<8; i++) begin
    str_src.put(i);
  end
  repeat(16) @(posedge clk);
  repeat(4) @(posedge clk);

  // end simulation
  repeat(4) @(posedge clk);
  $finish();
end

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

str_src #(
  .DW  (DW)
) str_src (
  // system signals
  .clk      (clk ),
  .rstn     (rstn),
  // stream
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
  .DW  (DW)
) str_drn (
  // system signals
  .clk      (clk ),
  .rstn     (rstn),
  // stream
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
