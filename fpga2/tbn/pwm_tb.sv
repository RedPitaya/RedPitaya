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
  int unsigned CW = 4,  // counter width (resolution)
  int unsigned OW = 4,  // output  width
  bit [CW-1:0] CCE = 2**CW-1  // 100% value
);

// system signals
logic                  clk ;  // clock
logic                  rstn;  // reset

// configuration
logic                  ena;  // enable
logic         [CW-1:0] rng;  // range

// data stream
logic [OW-1:0][CW-1:0] str_dat;
logic                  str_vld;
logic                  str_rdy;

// signals to PWM/PDM IO
logic [OW-1:0]         pwm;
logic [OW-1:0]         pdm;

////////////////////////////////////////////////////////////////////////////////
// clock and test sequence
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial begin
  // TODO, this are now constants, but should be tested
  ena = 1'b1;
  rng = CCE;
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
    str_dat <= {OW{i[CW-1:0]}};
    @ (posedge clk);
    while (~str_rdy) @ (posedge clk);
  end
  // end simulation
  repeat(2**CW) @(posedge clk);
  repeat(4) @(posedge clk);
  $finish();
end

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

pwm #(
  .CW (CW),
  .OW (OW)
) dut_pwm (
  // system signals
  .clk      (clk ),
  .rstn     (rstn),
  // configuration
  .ena      (ena),
  .rng      (rng),
  // input stream
  .str_dat  (str_dat), 
  .str_vld  (str_vld), 
  .str_rdy  (str_rdy), 
  // PWM outputs
  .pwm      (pwm)
);

pdm #(
  .CW (CW),
  .OW (OW)
) dut_pdm (
  // system signals
  .clk      (clk ),
  .rstn     (rstn),
  // configuration
  .ena      (ena),
  .rng      (rng),
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
