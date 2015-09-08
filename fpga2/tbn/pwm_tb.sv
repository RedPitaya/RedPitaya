////////////////////////////////////////////////////////////////////////////////
// PWM (pulse width modulation) and PDM (pulse density modulation)
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module pwm_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // PWM parameters
  int unsigned  CCW = 4,        // configuration counter width (resolution)
  bit [CCW-1:0] CCE = 2**CCW-1  // 100% value
);

// system signals
logic           clk ;
logic           rstn;

// input stream
logic [CCW-1:0] str_dat;
logic           str_vld;
logic           str_rdy;

// signals to PWM/PDM IO
logic           pwm;
logic           pdm;

////////////////////////////////////////////////////////////////////////////////
// clock and test sequence
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial begin
  // initialization
  rstn = 1'b0;
  str_dat = '0;
  str_vld = 1'b0;
  repeat(4) @(posedge clk);
  // start
  rstn = 1'b1;
  str_vld = 1'b1;
  // test sequence
  for (int unsigned i=0; i<=CCE; i++) begin
    str_dat <= i[CCW-1:0];
    @ (posedge clk);
    while (~str_rdy) @ (posedge clk);
  end
  // end simulation
  repeat(2**CCW) @(posedge clk);
  repeat(4) @(posedge clk);
  $finish();
end

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

pwm #(
  .CCW (CCW),
  .CCE (CCE)
) dut_pwm (
  // system signals
  .clk      (clk ),
  .rstn     (rstn),
  // input stream
  .str_dat  (str_dat), 
  .str_vld  (str_vld), 
  .str_rdy  (str_rdy), 
  // PWM outputs
  .pwm      (pwm)
);

pdm #(
  .CCW (CCW),
  .CCE (CCE)
) dut_pdm (
  // system signals
  .clk      (clk ),
  .rstn     (rstn),
  // input stream
  .str_dat  (str_dat), 
  .str_vld  (str_vld), 
  .str_rdy  (       ), 
  // PWM outputs
  .pdm      (pdm)
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("pwm_tb.vcd");
  $dumpvars(0, pwm_tb);
end

endmodule: pwm_tb
