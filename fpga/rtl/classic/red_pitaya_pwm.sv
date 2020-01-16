/**
 * @brief Red Pitaya PWM module
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
  int unsigned CCW = 24,  // configuration counter width (resolution)
  bit  [8-1:0] FULL = 8'd156 // 100% value
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

reg  [ 4-1: 0] bcnt  ;
reg  [16-1: 0] b     ;
reg  [ 8-1: 0] vcnt, vcnt_r;
reg  [ 8-1: 0] v   , v_r   ;

always_ff @(posedge clk)
if (~rstn) begin
   vcnt <=  8'h0 ;
   bcnt <=  4'h0 ;
   pwm_o    <=  1'b0 ;
end else begin
   vcnt   <= (vcnt == FULL) ? 8'h1 : (vcnt + 8'd1) ;
   vcnt_r <= vcnt;
   v_r    <= (v + b[0]) ; // add decimal bit to current value
   if (vcnt == FULL) begin
      bcnt <=  bcnt + 4'h1 ;
      v    <= (bcnt == 4'hF) ? cfg[24-1:16] : v ; // new value on 16*FULL
      b    <= (bcnt == 4'hF) ? cfg[16-1:0] : {1'b0,b[15:1]} ; // shift right
   end
   // make PWM duty cycle
   pwm_o <= (vcnt_r <= v_r) ;
end

assign pwm_s = (bcnt == 4'hF) && (vcnt == (FULL-1)) ; // latch one before

endmodule: red_pitaya_pwm
