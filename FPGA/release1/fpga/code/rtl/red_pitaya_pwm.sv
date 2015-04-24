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

module red_pitaya_pwm #(
  int unsigned CCW = 24  // configuration counter width (resolution)
)(
  // system signals
  input  logic           clk ,  // clock
  input  logic           rstn,  // reset
  // configuration
  input  logic [CCW-1:0] cfg ,  // 
  // PWM outputs
  output logic           pwm_o ,  // PWM output - driving RC
  output logic           pwm_s    // PWM synchronization
);

localparam bit [8-1:0] PWM_FULL = 8'd156; // 100% value

reg  [ 4-1: 0] pwm_bcnt  ;
reg  [16-1: 0] pwm_b     ;
reg  [ 8-1: 0] pwm_vcnt, pwm_vcnt_r;
reg  [ 8-1: 0] pwm_v   , pwm_v_r   ;

always @(posedge clk)
if (~rstn) begin
   pwm_vcnt <=  8'h0 ;
   pwm_bcnt <=  4'h0 ;
   pwm_o    <=  1'b0 ;
end else begin
   pwm_vcnt   <= (pwm_vcnt == PWM_FULL) ? 8'h1 : (pwm_vcnt + 8'd1) ;
   pwm_vcnt_r <= pwm_vcnt;
   pwm_v_r    <= (pwm_v + pwm_b[0]) ; // add decimal bit to current value
   if (pwm_vcnt == PWM_FULL) begin
      pwm_bcnt <=  pwm_bcnt + 4'h1 ;
      pwm_v    <= (pwm_bcnt == 4'hF) ? cfg[24-1:16] : pwm_v ; // new value on 16*PWM_FULL
      pwm_b    <= (pwm_bcnt == 4'hF) ? cfg[16-1:0] : {1'b0,pwm_b[15:1]} ; // shift right
   end
   // make PWM duty cycle
   pwm_o <= (pwm_vcnt_r <= pwm_v_r) ;
end

assign pwm_s = (pwm_bcnt == 4'hF) && (pwm_vcnt == (PWM_FULL-1)) ; // latch one before

endmodule
