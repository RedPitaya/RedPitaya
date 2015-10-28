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

// stream input
logic signed [DWI-1:0] sti_dat;
logic                  sti_vld;
logic                  sti_rdy;

// stream output
logic signed [DWO-1:0] sto_dat;
logic                  sto_vld;
logic                  sto_rdy;

// configuration
logic signed [DWM-1:0] cfg_mul;
logic signed [DWS-1:0] cfg_sum;

////////////////////////////////////////////////////////////////////////////////
// clock and test sequence
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial begin
  // initialization
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  // start
  rstn = 1'b1;
  // end simulation
  repeat(4) @(posedge clk);
  $finish();
end

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

linear #(
  .DWI (DWI),
  .DWO (DWO),
  .DWM (DWM),
  .DWS (DWS)
) linear (
  // system signals
  .clk      (clk    ),
  .rstn     (rstn   ),
  // input stream
  .sti_dat  (sti_dat),
  .sti_vld  (sti_vld),
  .sti_rdy  (sti_rdy),
  // output stream
  .sto_dat  (sto_dat),
  .sto_vld  (sto_vld),
  .sto_rdy  (sto_rdy),
  // configuration
  .cfg_mul  (cfg_mul),
  .cfg_sum  (cfg_sum)
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("linear_tb.vcd");
  $dumpvars(0, linear_tb);
end

endmodule: linear_tb
