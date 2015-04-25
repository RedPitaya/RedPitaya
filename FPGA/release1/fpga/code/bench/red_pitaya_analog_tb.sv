/**
 * $Id: red_pitaya_analog_tb.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
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

module red_pitaya_analog_tb #(
  // time periods
  realtime  TP = 8.0ns  // 125MHz
);

glbl glbl();

// signals to DAC IO
logic [ 14-1: 0] io_dac_dat      ;
logic            io_dac_wrt      ;
logic            io_dac_sel      ;
logic            io_dac_clk      ;
logic            io_dac_rst      ;

// signals from ADC IO
logic            io_adc_clk      ;
logic            io_adc_rstn     ;
logic [ 14-1: 0] io_adc_a        ;
logic [ 14-1: 0] io_adc_b        ;

logic            adc_clk         ;
logic            adc_rstn        ;
logic            ser_clk         ;

logic [ 14-1: 0] dac_a           ;
logic [ 14-1: 0] dac_b           ;

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

assign adc_rstn = io_adc_rstn;

initial begin
  io_adc_rstn = 1'b0;
  repeat(10) @(posedge io_adc_clk); io_adc_rstn = 1'b1;
end

initial        io_adc_clk = 1'h0;
always #(TP/2) io_adc_clk = ~io_adc_clk;

initial begin
  fork
    // ADC IO signal source
    begin
      io_adc_a <= 14'h3FFF;
      io_adc_b <= 14'h0000;
      #500;
      io_adc_a <= 14'h2000;
      io_adc_b <= 14'h1FFF;
      #500;
      io_adc_a <= 14'h1000;
      io_adc_b <= 14'h2000;
    end
    // DAC internal signal source
    begin
      #500;
      dac_a <=  14'd0500;
      dac_b <= -14'd0500;
      #500;
      dac_a <=  14'd5000;
      dac_b <= -14'd5000;
      #500;
      dac_a <= -14'd7000;
      dac_b <=  14'd7000;
    end
  join
  $finish();
end

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

red_pitaya_analog analog (
  // ADC
  .adc_dat_a_i        ( io_adc_a    ),  // CH 1
  .adc_dat_b_i        ( io_adc_b    ),  // CH 2
  .adc_clk_p_i        ( io_adc_clk  ),  // data clock
  .adc_clk_n_i        (!io_adc_clk  ),  // data clock
  // DAC
  .dac_dat_o          ( io_dac_dat  ),  // combined data
  .dac_wrt_o          ( io_dac_wrt  ),  // write enable
  .dac_sel_o          ( io_dac_sel  ),  // channel select
  .dac_clk_o          ( io_dac_clk  ),  // clock
  .dac_rst_o          ( io_dac_rst  ),  // reset
  // user interface
  .adc_dat_a_o        (             ),  // ADC CH1
  .adc_dat_b_o        (             ),  // ADC CH2
  .adc_clk_o          ( adc_clk     ),  // ADC received clock
  .adc_rstn_i         ( adc_rstn    ),  // ADC reset - active low
  .ser_clk_o          ( ser_clk     ),  // fast serial clock

  .dac_dat_a_i        ( dac_a       ),  // DAC CH1
  .dac_dat_b_i        ( dac_b       ),  // DAC CH2

  .pwm_clk            (),
  .pwm_rstn           ()
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("red_pitaya_analog_tb.vcd");
  $dumpvars(0, red_pitaya_analog_tb);
end

endmodule: red_pitaya_analog_tb
