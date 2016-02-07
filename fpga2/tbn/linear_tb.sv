////////////////////////////////////////////////////////////////////////////////
// Module: Linear transformation (gain, offset and saturation)
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module linear_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // stream parameters
  type DTI = logic signed [8-1:0], // data type for input
  type DTO = logic signed [8-1:0], // data type for output
  int unsigned DWI = $bits(DTI),   // data width for input
  int unsigned DWO = $bits(DTO),   // data width for output
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
axi4_stream_if #(.DAT_T (DTI)) sti (.ACLK (clk), .ARESETn (rstn));
axi4_stream_if #(.DAT_T (DTO)) sto (.ACLK (clk), .ARESETn (rstn));

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
    str_src.put(i, '1, 1'b0);
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

axi4_stream_src #(.DAT_T (DTI)) str_src (.str (sti));

linear #(
  .DTI (DTI),
  .DTO (DTO),
  .DWM (DWM),
  .DWS (DWS)
) linear (
  // stream input/output
  .sti      (sti    ),
  .sto      (sto    ),
  // configuration
  .cfg_mul  (cfg_mul),
  .cfg_sum  (cfg_sum)
);

axi4_stream_drn #(.DAT_T (DTO)) str_drn (.str (sto));

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("linear_tb.vcd");
  $dumpvars(0, linear_tb);
end

endmodule: linear_tb
