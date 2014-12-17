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
 *                       |
 *                       ˇ
 *                   /-------\
 *   DAC PWM <------ |  PWM  | <------- SLOW DAC DATA FROM USER
 *                   \-------/
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




module red_pitaya_analog
(
  // ADC IC
  input    [ 16-1: 2] adc_dat_a_i        ,  //!< ADC IC CHA data connection
  input    [ 16-1: 2] adc_dat_b_i        ,  //!< ADC IC CHB data connection
  input               adc_clk_p_i        ,  //!< ADC IC clock P connection
  input               adc_clk_n_i        ,  //!< ADC IC clock N connection
  
  // DAC IC
  output   [ 14-1: 0] dac_dat_o          ,  //!< DAC IC combined data
  output              dac_wrt_o          ,  //!< DAC IC write enable
  output              dac_sel_o          ,  //!< DAC IC channel select
  output              dac_clk_o          ,  //!< DAC IC clock
  output              dac_rst_o          ,  //!< DAC IC reset
  
  // PWM DAC
  output   [  4-1: 0] dac_pwm_o          ,  //!< DAC PWM - driving RC
  
  
  // user interface
  output   [ 14-1: 0] adc_dat_a_o        ,  //!< ADC CHA data
  output   [ 14-1: 0] adc_dat_b_o        ,  //!< ADC CHB data
  output              adc_clk_o          ,  //!< ADC clock
  input               adc_rst_i          ,  //!< ADC reset - active low
  output              ser_clk_o          ,  //!< fast serial clock

  input    [ 14-1: 0] dac_dat_a_i        ,  //!< DAC CHA data
  input    [ 14-1: 0] dac_dat_b_i        ,  //!< DAC CHB data

  input    [ 24-1: 0] dac_pwm_a_i        ,  //!< DAC PWM CHA
  input    [ 24-1: 0] dac_pwm_b_i        ,  //!< DAC PWM CHB
  input    [ 24-1: 0] dac_pwm_c_i        ,  //!< DAC PWM CHC
  input    [ 24-1: 0] dac_pwm_d_i        ,  //!< DAC PWM CHD
  output              dac_pwm_sync_o        //!< DAC PWM sync
);


//---------------------------------------------------------------------------------
//
//  ADC input registers

reg  [14-1: 0] adc_dat_a  ;
reg  [14-1: 0] adc_dat_b  ;
wire           adc_clk_in ;
wire           adc_clk    ;

IBUFDS i_clk ( .I(adc_clk_p_i), .IB(adc_clk_n_i), .O(adc_clk_in));  // differential clock input
BUFG i_adc_buf  (.O(adc_clk), .I(adc_clk_in)); // use global clock buffer

always @(posedge adc_clk) begin
   adc_dat_a <= adc_dat_a_i[16-1:2]; // lowest 2 bits reserved for 16bit ADC
   adc_dat_b <= adc_dat_b_i[16-1:2];
end
    
assign adc_dat_a_o = {adc_dat_a[14-1], ~adc_dat_a[14-2:0]}; // transform into 2's complement (negative slope)
assign adc_dat_b_o = {adc_dat_b[14-1], ~adc_dat_b[14-2:0]};
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

PLLE2_ADV
#(
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
)
i_dac_plle2
(
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
   .RST          ( !adc_rst_i      )
);

BUFG i_dacfb_buf   (.O(dac_clk_fb_buf), .I(dac_clk_fb));
BUFG i_dac1_buf    (.O(dac_clk),        .I(dac_clk_out));
BUFG i_dac2_buf    (.O(dac_2clk),       .I(dac_2clk_out));
BUFG i_dac2ph_buf  (.O(dac_2ph),        .I(dac_2ph_out));
BUFG i_ser_buf     (.O(ser_clk_o),      .I(ser_clk_out));




reg  [14-1: 0] dac_dat_a  ;
reg  [14-1: 0] dac_dat_b  ;

// output registers + signed to unsigned (also to negative slope)
always @(posedge dac_clk) begin
   dac_dat_a <= {dac_dat_a_i[14-1], ~dac_dat_a_i[14-2:0]};
   dac_dat_b <= {dac_dat_b_i[14-1], ~dac_dat_b_i[14-2:0]};
   dac_rst   <= !dac_locked;
end


ODDR i_dac_clk ( .Q(dac_clk_o), .D1(1'b0), .D2(1'b1), .C(dac_2ph),  .CE(1'b1), .R(dac_rst), .S(1'b0) );
ODDR i_dac_wrt ( .Q(dac_wrt_o), .D1(1'b0), .D2(1'b1), .C(dac_2clk), .CE(1'b1), .R(dac_rst), .S(1'b0) );
ODDR i_dac_sel ( .Q(dac_sel_o), .D1(1'b1), .D2(1'b0), .C(dac_clk ), .CE(1'b1), .R(dac_rst), .S(1'b0) );
ODDR i_dac_rst ( .Q(dac_rst_o), .D1(dac_rst), .D2(dac_rst), .C(dac_clk ), .CE(1'b1), .R(1'b0), .S(1'b0) );

ODDR i_dac_0  ( .Q(dac_dat_o[ 0]), .D1(dac_dat_b[ 0]), .D2(dac_dat_a[ 0]), .C(dac_clk), .CE(1'b1), .R(dac_rst), .S(1'b0) );
ODDR i_dac_1  ( .Q(dac_dat_o[ 1]), .D1(dac_dat_b[ 1]), .D2(dac_dat_a[ 1]), .C(dac_clk), .CE(1'b1), .R(dac_rst), .S(1'b0) );
ODDR i_dac_2  ( .Q(dac_dat_o[ 2]), .D1(dac_dat_b[ 2]), .D2(dac_dat_a[ 2]), .C(dac_clk), .CE(1'b1), .R(dac_rst), .S(1'b0) );
ODDR i_dac_3  ( .Q(dac_dat_o[ 3]), .D1(dac_dat_b[ 3]), .D2(dac_dat_a[ 3]), .C(dac_clk), .CE(1'b1), .R(dac_rst), .S(1'b0) );
ODDR i_dac_4  ( .Q(dac_dat_o[ 4]), .D1(dac_dat_b[ 4]), .D2(dac_dat_a[ 4]), .C(dac_clk), .CE(1'b1), .R(dac_rst), .S(1'b0) );
ODDR i_dac_5  ( .Q(dac_dat_o[ 5]), .D1(dac_dat_b[ 5]), .D2(dac_dat_a[ 5]), .C(dac_clk), .CE(1'b1), .R(dac_rst), .S(1'b0) );
ODDR i_dac_6  ( .Q(dac_dat_o[ 6]), .D1(dac_dat_b[ 6]), .D2(dac_dat_a[ 6]), .C(dac_clk), .CE(1'b1), .R(dac_rst), .S(1'b0) );
ODDR i_dac_7  ( .Q(dac_dat_o[ 7]), .D1(dac_dat_b[ 7]), .D2(dac_dat_a[ 7]), .C(dac_clk), .CE(1'b1), .R(dac_rst), .S(1'b0) );
ODDR i_dac_8  ( .Q(dac_dat_o[ 8]), .D1(dac_dat_b[ 8]), .D2(dac_dat_a[ 8]), .C(dac_clk), .CE(1'b1), .R(dac_rst), .S(1'b0) );
ODDR i_dac_9  ( .Q(dac_dat_o[ 9]), .D1(dac_dat_b[ 9]), .D2(dac_dat_a[ 9]), .C(dac_clk), .CE(1'b1), .R(dac_rst), .S(1'b0) );
ODDR i_dac_10 ( .Q(dac_dat_o[10]), .D1(dac_dat_b[10]), .D2(dac_dat_a[10]), .C(dac_clk), .CE(1'b1), .R(dac_rst), .S(1'b0) );
ODDR i_dac_11 ( .Q(dac_dat_o[11]), .D1(dac_dat_b[11]), .D2(dac_dat_a[11]), .C(dac_clk), .CE(1'b1), .R(dac_rst), .S(1'b0) );
ODDR i_dac_12 ( .Q(dac_dat_o[12]), .D1(dac_dat_b[12]), .D2(dac_dat_a[12]), .C(dac_clk), .CE(1'b1), .R(dac_rst), .S(1'b0) );
ODDR i_dac_13 ( .Q(dac_dat_o[13]), .D1(dac_dat_b[13]), .D2(dac_dat_a[13]), .C(dac_clk), .CE(1'b1), .R(dac_rst), .S(1'b0) );










//---------------------------------------------------------------------------------
//
//  Slow DAC - PWM

localparam PWM_FULL = 8'd156 ; // 100% value

reg  [ 4-1: 0] dac_pwm_bcnt   ;
reg  [16-1: 0] dac_pwm_ba     ;
reg  [16-1: 0] dac_pwm_bb     ;
reg  [16-1: 0] dac_pwm_bc     ;
reg  [16-1: 0] dac_pwm_bd     ;
reg  [ 8-1: 0] dac_pwm_vcnt   ;
reg  [ 8-1: 0] dac_pwm_vcnt_r ;
reg  [ 8-1: 0] dac_pwm_va     ;
reg  [ 8-1: 0] dac_pwm_vb     ;
reg  [ 8-1: 0] dac_pwm_vc     ;
reg  [ 8-1: 0] dac_pwm_vd     ;
reg  [ 8-1: 0] dac_pwm_va_r   ;
reg  [ 8-1: 0] dac_pwm_vb_r   ;
reg  [ 8-1: 0] dac_pwm_vc_r   ;
reg  [ 8-1: 0] dac_pwm_vd_r   ;
reg  [ 4-1: 0] dac_pwm        ;
reg  [ 4-1: 0] dac_pwm_r      ;

always @(posedge dac_2clk) begin
   if (dac_rst == 1'b1) begin
      dac_pwm_vcnt <=  8'h0 ;
      dac_pwm_bcnt <=  4'h0 ;
      dac_pwm_r    <=  4'h0 ;
   end
   else begin
      dac_pwm_vcnt <= (dac_pwm_vcnt == PWM_FULL) ? 8'h1 : (dac_pwm_vcnt + 8'd1) ;

      // additional register to improve timing
      dac_pwm_vcnt_r <= dac_pwm_vcnt;
      dac_pwm_va_r   <= (dac_pwm_va + dac_pwm_ba[0]) ; // add decimal bit to current value
      dac_pwm_vb_r   <= (dac_pwm_vb + dac_pwm_bb[0]) ;
      dac_pwm_vc_r   <= (dac_pwm_vc + dac_pwm_bc[0]) ;
      dac_pwm_vd_r   <= (dac_pwm_vd + dac_pwm_bd[0]) ;

      // make PWM duty cycle
      dac_pwm_r[0] <= (dac_pwm_vcnt_r <= dac_pwm_va_r) ;
      dac_pwm_r[1] <= (dac_pwm_vcnt_r <= dac_pwm_vb_r) ;
      dac_pwm_r[2] <= (dac_pwm_vcnt_r <= dac_pwm_vc_r) ;
      dac_pwm_r[3] <= (dac_pwm_vcnt_r <= dac_pwm_vd_r) ;


      if (dac_pwm_vcnt == PWM_FULL) begin
         dac_pwm_bcnt <= dac_pwm_bcnt + 4'h1 ;

         dac_pwm_va <= (dac_pwm_bcnt == 4'hF) ? dac_pwm_a_i[24-1:16] : dac_pwm_va ; // new value on 16*PWM_FULL
         dac_pwm_vb <= (dac_pwm_bcnt == 4'hF) ? dac_pwm_b_i[24-1:16] : dac_pwm_vb ;
         dac_pwm_vc <= (dac_pwm_bcnt == 4'hF) ? dac_pwm_c_i[24-1:16] : dac_pwm_vc ;
         dac_pwm_vd <= (dac_pwm_bcnt == 4'hF) ? dac_pwm_d_i[24-1:16] : dac_pwm_vd ;

         dac_pwm_ba <= (dac_pwm_bcnt == 4'hF) ? dac_pwm_a_i[16-1:0] : {1'b0,dac_pwm_ba[15:1]} ; // shift right
         dac_pwm_bb <= (dac_pwm_bcnt == 4'hF) ? dac_pwm_b_i[16-1:0] : {1'b0,dac_pwm_bb[15:1]} ; // new value on 16*PWM_FULL
         dac_pwm_bc <= (dac_pwm_bcnt == 4'hF) ? dac_pwm_c_i[16-1:0] : {1'b0,dac_pwm_bc[15:1]} ;
         dac_pwm_bd <= (dac_pwm_bcnt == 4'hF) ? dac_pwm_d_i[16-1:0] : {1'b0,dac_pwm_bd[15:1]} ;
      end

      dac_pwm <= dac_pwm_r ; // improve timing
   end
end

assign dac_pwm_o      = dac_pwm ;
assign dac_pwm_sync_o = (dac_pwm_bcnt == 4'hF) && (dac_pwm_vcnt == (PWM_FULL-1)) ; // latch one before













endmodule

