////////////////////////////////////////////////////////////////////////////////
// Module: Linear transformation (gain, offset and saturation)
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module linear_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // PWM parameters
  int unsigned DWI = 14,   // data width for input
  int unsigned DWO = 14,   // data width for output
  int unsigned DWM = 16,   // data width for multiplier (gain)
  int unsigned DWS = DWO   // data width for summation (offset)
);

// system signals
logic                  clk ;  // clock
logic                  rstn;  // reset - active low

// configuration
logic signed [DWM-1:0] cfg_mul;
logic signed [DWS-1:0] cfg_sum;

// stream input/output
str_bus_if #(.DAT_T (logic signed [DWI-1:0])) sti (.clk (clk), .rstn (rstn));
str_bus_if #(.DAT_T (logic signed [DWO-1:0])) sto (.clk (clk), .rstn (rstn));

// calibration
real gain   = 1.0;
real offset = 0.1;

////////////////////////////////////////////////////////////////////////////////
// clock and test sequence
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial begin
  // for now initialize configuration to an idle value
  cfg_sum = (offset * 2**(DWS-1));
  cfg_mul = (gain   * 2**(DWM-2));

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
  .DW  (DWI)
) str_src (
  // system signals
  .clk      (clk ),
  .rstn     (rstn),
  // stream
  .str      (sti)
);

linear #(
  .DWI (DWI),
  .DWO (DWO),
  .DWM (DWM),
  .DWS (DWS)
) linear (
  // system signals
  .clk      (clk    ),
  .rstn     (rstn   ),
  // stream input/output
  .sti      (sti    ),
  .sto      (sto    ),
  // configuration
  .cfg_mul  (cfg_mul),
  .cfg_sum  (cfg_sum)
);

str_drn #(
  .DW  (DWI)
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
  $dumpfile("linear_tb.vcd");
  $dumpvars(0, linear_tb);
end

endmodule: linear_tb
