/**
 * @brief Red Pitaya analog module testbench.
 *
 * @Author Matej Oblak
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in Verilog hardware description language (HDL).
 * Please visit http://en.wikipedia.org/wiki/Verilog
 * for more details on the language used herein.
 */

/**
 * GENERAL DESCRIPTION:
 *
 * Red Pitaya analog module testbench.
 *
 * Simple testbench to simulate ADC and DAC interface. Used also to simulate 
 * PLL and interface to PWM circuit on board.  
 * 
 */

`timescale 1ns / 1ps

module red_pitaya_pll_tb #(
  // time periods
  realtime  TP = 8.0ns  // 125MHz
);

glbl glbl();

logic clk       ;
logic rstn      ;

logic adc_clk   ;  // ADC clock
logic dac_clk_1x;  // DAC clock 125MHz
logic dac_clk_2x;  // DAC clock 250MHz
logic dac_clk_2p;  // DAC clock 250MHz -45DGR
logic ser_clk   ;  // fast serial clock
logic pwm_clk   ;  // PWM clock

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial begin
  rstn = 1'b0;
  repeat(10) @(posedge clk);
  rstn = 1'b1;
  repeat(100) @(posedge clk);
  $finish();
end

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

red_pitaya_pll pll (
  // inputs
  .clk         (clk ),  // clock
  .rstn        (rstn),  // reset - active low
  // output clocks
  .clk_adc     (adc_clk   ),  // ADC clock
  .clk_dac_1x  (dac_clk_1x),  // DAC clock 125MHz
  .clk_dac_2x  (dac_clk_2x),  // DAC clock 250MHz
  .clk_dac_2p  (dac_clk_2p),  // DAC clock 250MHz -45DGR
  .clk_ser     (ser_clk   ),  // fast serial clock
  .clk_pwm     (pwm_clk   ),  // PWM clock
  // status outputs
  .pll_locked  (pll_locked)
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("red_pitaya_pll_tb.vcd");
  $dumpvars(0, red_pitaya_pll_tb);
end

endmodule: red_pitaya_pll_tb
