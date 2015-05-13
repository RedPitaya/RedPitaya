/**
 * $Id: red_pitaya_analog.v 964 2014-01-24 12:58:17Z matej.oblak $
 *
 * @brief Red Pitaya analog module. Connects to ADC & DAC pins.
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
 * Interace module between fast ADC and DAC IC.  
 *
 *
 *                 /------------\      
 *   ADC DAT ----> | RAW -> 2's | ----> ADC DATA TO USER
 *                 \------------/
 *                       ^
 *                       |
 *                    /-----\
 *   ADC CLK -------> | PLL |
 *                    \-----/
 *                       |
 *                       ˇ
 *                 /------------\
 *   DAC DAT <---- | RAW <- 2's | <---- DAC DATA FROM USER
 *                 \------------/
 *
 *
 * ADC clock is used for main clock domain, from this double clock is made which
 * is used for driving DAC IC (using DDR transfer) and PWM counters.
 *
 * ADC interface gives unsigned number format with negative slope because 
 * input amplifier. This is transfomed into 2's complement wich is more usable in
 * digital world.
 *
 * For sending data to DAC values has to be first translated from 2's format to 
 * unsigned format, where negative output amplifier gain is taken into account.
 * Interface to DAC is DDR, positive edge used for CHA and negative for CHB.
 
 * PWM in created with counter running on 2xDAC clock. Each 16 cycles of PWM_FULL
 * counts new value is taken. Upper 8 bits are used for dac_pwm_vcnt which defines
 * PWM rate of output. This repeates 16x times, where lower 16bits of input data
 * defines if ration of dac_pwm_vcnt is one cycle more.
 * 
 */

module red_pitaya_analog (
  // ADC IC
  input    [ 16-1: 2] adc_dat_a_i        ,  // ADC IC CHA data connection
  input    [ 16-1: 2] adc_dat_b_i        ,  // ADC IC CHB data connection
  input               adc_clk_p_i        ,  // ADC IC clock P connection
  input               adc_clk_n_i        ,  // ADC IC clock N connection
  // DAC IC
  output   [ 14-1: 0] dac_dat_o          ,  // DAC IC combined data
  output              dac_wrt_o          ,  // DAC IC write enable
  output              dac_sel_o          ,  // DAC IC channel select
  output              dac_clk_o          ,  // DAC IC clock
  output              dac_rst_o          ,  // DAC IC reset
  // user interface
  output   [ 14-1: 0] adc_dat_a_o        ,  // ADC CHA data
  output   [ 14-1: 0] adc_dat_b_o        ,  // ADC CHB data
  output              adc_clk_o          ,  // ADC clock
  input               adc_rstn_i         ,  // ADC reset - active low
  output              ser_clk_o          ,  // fast serial clock

  input    [ 14-1: 0] dac_dat_a_i        ,  // DAC CHA data
  input    [ 14-1: 0] dac_dat_b_i        ,  // DAC CHB data
  // PWM
  output              pwm_clk            ,  // PWM clock
  output              pwm_rstn              // PWM reset - active low
);

//---------------------------------------------------------------------------------
//
//  ADC input registers

wire           adc_clk_in ;
wire           adc_clk    ;

IBUFDS i_clk ( .I(adc_clk_p_i), .IB(adc_clk_n_i), .O(adc_clk_in));  // differential clock input
BUFG i_adc_buf  (.O(adc_clk), .I(adc_clk_in)); // use global clock buffer


assign adc_clk_o   =  adc_clk ;

//---------------------------------------------------------------------------------
//
//  Fast DAC - DDR interface

wire  dac_clk_fb      ;
wire  dac_clk_fb_buf  ;
wire  dac_clk_out     ;
wire  dac_2clk_out    ;
wire  dac_clk         ;
wire  dac_2clk        ;
wire  dac_locked      ;
reg   dac_rst         ;
wire  ser_clk_out     ;
wire  dac_2ph_out     ;
wire  dac_2ph         ;

PLLE2_ADV #(
   .BANDWIDTH            ( "OPTIMIZED"   ),
   .COMPENSATION         ( "ZHOLD"       ),
   .DIVCLK_DIVIDE        (  1            ),
   .CLKFBOUT_MULT        (  8            ),
   .CLKFBOUT_PHASE       (  0.000        ),
   .CLKOUT0_DIVIDE       (  8            ),
   .CLKOUT0_PHASE        (  0.000        ),
   .CLKOUT0_DUTY_CYCLE   (  0.5          ),
   .CLKOUT1_DIVIDE       (  4            ),
   .CLKOUT1_PHASE        (  0.000        ),
   .CLKOUT1_DUTY_CYCLE   (  0.5          ),
   .CLKOUT2_DIVIDE       (  4            ),
   .CLKOUT2_PHASE        ( -45.000       ),
   .CLKOUT2_DUTY_CYCLE   (  0.5          ),
   .CLKOUT3_DIVIDE       (  4            ),  // 4->250MHz, 2->500MHz
   .CLKOUT3_PHASE        (  0.000        ),
   .CLKOUT3_DUTY_CYCLE   (  0.5          ),
   .CLKIN1_PERIOD        (  8.000        ),
   .REF_JITTER1          (  0.010        )
) i_dac_plle2 (
   // Output clocks
   .CLKFBOUT     (  dac_clk_fb     ),
   .CLKOUT0      (  dac_clk_out    ),
   .CLKOUT1      (  dac_2clk_out   ),
   .CLKOUT2      (  dac_2ph_out    ),
   .CLKOUT3      (  ser_clk_out    ),
   .CLKOUT4      (        ),
   .CLKOUT5      (        ),
   // Input clock control
   .CLKFBIN      (  dac_clk_fb_buf ),
   .CLKIN1       (  adc_clk        ),
   .CLKIN2       (  1'b0           ),
   // Tied to always select the primary input clock
   .CLKINSEL     (  1'b1           ),
   // Ports for dynamic reconfiguration
   .DADDR        (  7'h0           ),
   .DCLK         (  1'b0           ),
   .DEN          (  1'b0           ),
   .DI           (  16'h0          ),
   .DO           (        ),
   .DRDY         (        ),
   .DWE          (  1'b0           ),
   // Other control and status signals
   .LOCKED       (  dac_locked     ),
   .PWRDWN       (  1'b0           ),
   .RST          ( !adc_rstn_i     )
);

BUFG i_dacfb_buf   (.O(dac_clk_fb_buf), .I(dac_clk_fb));
BUFG i_dac1_buf    (.O(dac_clk),        .I(dac_clk_out));
BUFG i_dac2_buf    (.O(dac_2clk),       .I(dac_2clk_out));
BUFG i_dac2ph_buf  (.O(dac_2ph),        .I(dac_2ph_out));
BUFG i_ser_buf     (.O(ser_clk_o),      .I(ser_clk_out));

endmodule
