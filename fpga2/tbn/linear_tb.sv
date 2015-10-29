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
  .clk      (clk    ),
  .rstn     (rstn   ),
  // z stream signals
  .str_dat  (sti_dat),
  .str_vld  (sti_vld),
  .str_rdy  (sti_rdy)
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

str_drn #(
  .DW  (DWI)
) str_drn (
  // system signals
  .clk      (clk    ),
  .rstn     (rstn   ),
  // z stream signals
  .str_dat  (sto_dat),
  .str_vld  (sto_vld),
  .str_rdy  (sto_rdy)
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("linear_tb.vcd");
  $dumpvars(0, linear_tb);
end

endmodule: linear_tb
