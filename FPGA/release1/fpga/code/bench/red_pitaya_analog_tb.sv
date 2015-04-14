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

module red_pitaya_analog_tb(
);



// DAC
wire  [ 14-1: 0] dac_dat_o       ;
wire             dac_wrt_o       ;
wire             dac_sel_o       ;
wire             dac_clk_o       ;
wire             dac_rst_o       ;
wire  [  4-1: 0] dac_pwm_o       ;


reg   [  4-1: 0] fclk            ;
reg   [  4-1: 0] frstn           ;
wire             adc_clk         ;
reg              adc_rst         ;
wire             ser_clk         ;
reg   [ 14-1: 0] adc_a           ;
reg   [ 14-1: 0] adc_b           ;
reg   [ 14-1: 0] dac_a           ;
reg   [ 14-1: 0] dac_b           ;
reg   [ 24-1: 0] dac_pwm_a       ;
reg   [ 24-1: 0] dac_pwm_b       ;
reg   [ 24-1: 0] dac_pwm_c       ;
reg   [ 24-1: 0] dac_pwm_d       ;


red_pitaya_analog i_analog
(
  // ADC
  .adc_dat_a_i        (  adc_a            ),  // CH 1
  .adc_dat_b_i        (  adc_b            ),  // CH 2
  .adc_clk_p_i        (  fclk[0]          ),  // data clock
  .adc_clk_n_i        ( !fclk[0]          ),  // data clock
  
  // DAC
  .dac_dat_o          (  dac_dat_o        ),  // combined data
  .dac_wrt_o          (  dac_wrt_o        ),  // write enable
  .dac_sel_o          (  dac_sel_o        ),  // channel select
  .dac_clk_o          (  dac_clk_o        ),  // clock
  .dac_rst_o          (  dac_rst_o        ),  // reset
  
  // PWM DAC
  .dac_pwm_o          (  dac_pwm_o        ),  // serial PWM DAC
  
  
  // user interface
  .adc_dat_a_o        (             ),  // ADC CH1
  .adc_dat_b_o        (             ),  // ADC CH2
  .adc_clk_o          (  adc_clk          ),  // ADC received clock
  .adc_rst_i          (  adc_rst          ),  // ADC reset - active low
  .ser_clk_o          (  ser_clk          ),  // fast serial clock

  .dac_dat_a_i        (  dac_a            ),  // DAC CH1
  .dac_dat_b_i        (  dac_b            ),  // DAC CH2

  .dac_pwm_a_i        (  dac_pwm_a        ),  // slow DAC CH1
  .dac_pwm_b_i        (  dac_pwm_b        ),  // slow DAC CH2
  .dac_pwm_c_i        (  dac_pwm_c        ),  // slow DAC CH3
  .dac_pwm_d_i        (  dac_pwm_d        ),  // slow DAC CH4
  .dac_pwm_sync_o     (  dac_pwm_sync     )
);

  








//---------------------------------------------------------------------------------
//
// signal generation


always @(posedge adc_clk) begin
   adc_rst <= frstn[0] ;
end


initial begin
   fclk  = 4'h2  ;
   frstn = 4'h0  ;
   fork
   begin
      repeat(10) @(posedge fclk[0]);
      frstn[0] = 1'b1  ;
   end
   begin
      repeat(21) @(posedge fclk[1]);
      frstn[1] = 1'b1  ;
   end
   begin
      repeat(10) @(posedge fclk[2]);
      frstn[2] = 1'b1  ;
   end
   begin
      repeat(10) @(posedge fclk[3]);
      frstn[3] = 1'b1  ;
   end
   join
end


always begin
   #4 fclk[0] <= !fclk[0] ;
end
always begin
   #2 fclk[1] <= !fclk[1] ;
end
always begin
   #10 fclk[2] <= !fclk[2] ;
end
always begin
   #10 fclk[3] <= !fclk[3] ;
end




initial begin
   adc_a <= 14'h3FFF  ;
   adc_b <= 14'h0000  ;
   #500;
   adc_a <= 14'h2000  ;
   adc_b <= 14'h1FFF  ;
   #500;
   adc_a <= 14'h1000  ;
   adc_b <= 14'h2000  ;
end


initial begin
   #500;
   dac_a <=  14'd500   ;
   dac_b <= -14'd500   ;
   #500;
   dac_a <=  14'd5000  ;
   dac_b <= -14'd5000  ;
   #500;
   dac_a <= -14'd7000  ;
   dac_b <=  14'd7000  ;
end



initial begin
   wait (adc_rst)
   repeat(10) @(posedge adc_clk);

   #10;
   wait (dac_pwm_sync)
      @(posedge adc_clk);
   dac_pwm_a <= {8'd0, 16'h0001};
   dac_pwm_b <= {8'd0, 16'h8000};
   dac_pwm_c <= {8'd0, 16'h0000};
   dac_pwm_d <= {8'd0, 16'h0000};

   #150000;
   wait (dac_pwm_sync)
      @(posedge adc_clk);
   dac_pwm_a <= {8'd0, 16'h8001};
   dac_pwm_b <= {8'd1, 16'h8001};
   dac_pwm_c <= {8'd0, 16'hFFFF};
   dac_pwm_d <= {8'd1, 16'h0000};

   #150000;
   wait (dac_pwm_sync)
      @(posedge adc_clk);
   dac_pwm_a <= {8'd0,   16'h8001};
   dac_pwm_b <= {8'd0,   16'h8181};
   dac_pwm_c <= {8'd155, 16'hFFFF};
   dac_pwm_d <= {8'd156, 16'h0000};

end





endmodule
