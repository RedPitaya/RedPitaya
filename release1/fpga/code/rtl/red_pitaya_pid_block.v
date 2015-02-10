/**
 * $Id: red_pitaya_pid_block.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya PID controller.
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
 * Proportional-integral-derivative (PID) controller.
 *
 *
 *        /---\         /---\      /-----------\
 *   IN --| - |----+--> | P | ---> | SUM & SAT | ---> OUT
 *        \---/    |    \---/      \-----------/
 *          ^      |                   ^  ^
 *          |      |    /---\          |  |
 *   set ----      +--> | I | ---------   |
 *   point         |    \---/             |
 *                 |                      |
 *                 |    /---\             |
 *                 ---> | D | ------------
 *                      \---/
 *
 *
 * Proportional-integral-derivative (PID) controller is made from three parts. 
 *
 * Error which is difference between set point and input signal is driven into
 * propotional, integral and derivative part. Each calculates its own value which
 * is then summed and saturated before given to output.
 *
 * Integral part has also separate input to reset integrator value to 0.
 * 
 */




module red_pitaya_pid_block #(
   parameter     PSR = 12         ,
   parameter     ISR = 18         ,
   parameter     DSR = 10          
)
(
   // data
   input                 clk_i           ,  // clock
   input                 rstn_i          ,  // reset - active low
   input      [ 14-1: 0] dat_i           ,  // input data
   output     [ 14-1: 0] dat_o           ,  // output data

   // settings
   input      [ 14-1: 0] set_sp_i        ,  // set point
   input      [ 14-1: 0] set_kp_i        ,  // Kp
   input      [ 14-1: 0] set_ki_i        ,  // Ki
   input      [ 14-1: 0] set_kd_i        ,  // Kd
   input                 int_rst_i          // integrator reset
);




//---------------------------------------------------------------------------------
//  Set point error calculation

reg  [ 15-1: 0] error        ;

always @(posedge clk_i) begin
   if (rstn_i == 1'b0) begin
      error <= 15'h0 ;
   end
   else begin
      error <= $signed(set_sp_i) - $signed(dat_i) ;
   end
end








//---------------------------------------------------------------------------------
//  Proportional part

reg   [29-PSR-1: 0] kp_reg        ;
wire  [    29-1: 0] kp_mult       ;

always @(posedge clk_i) begin
   if (rstn_i == 1'b0) begin
      kp_reg  <= {29-PSR{1'b0}};
   end
   else begin
      kp_reg <= kp_mult[29-1:PSR] ;
   end
end

assign kp_mult = $signed(error) * $signed(set_kp_i);








//---------------------------------------------------------------------------------
//  Integrator

reg   [    29-1: 0] ki_mult       ;
wire  [    33-1: 0] int_sum       ;
reg   [    32-1: 0] int_reg       ;
wire  [32-ISR-1: 0] int_shr       ;

always @(posedge clk_i) begin
   if (rstn_i == 1'b0) begin
      ki_mult  <= {29{1'b0}};
      int_reg  <= {32{1'b0}};
   end
   else begin
      ki_mult <= $signed(error) * $signed(set_ki_i) ;

      if (int_rst_i)
         int_reg <= 32'h0; // reset
      else if (int_sum[33-1:33-2] == 2'b01) // positive saturation
         int_reg <= 32'h7FFFFFFF; // max positive
      else if (int_sum[33-1:33-2] == 2'b10) // negative saturation
         int_reg <= 32'h80000000; // max negative
      else
         int_reg <= int_sum[32-1:0]; // use sum as it is
   end
end

assign int_sum = $signed(ki_mult) + $signed(int_reg) ;
assign int_shr = int_reg[32-1:ISR] ;






//---------------------------------------------------------------------------------
//  Derivative

wire  [    29-1: 0] kd_mult       ;
reg   [29-DSR-1: 0] kd_reg        ;
reg   [29-DSR-1: 0] kd_reg_r      ;
reg   [29-DSR  : 0] kd_reg_s      ;


always @(posedge clk_i) begin
   if (rstn_i == 1'b0) begin
      kd_reg   <= {29-DSR{1'b0}};
      kd_reg_r <= {29-DSR{1'b0}};
      kd_reg_s <= {29-DSR+1{1'b0}};
   end
   else begin
      kd_reg   <= kd_mult[29-1:DSR] ;
      kd_reg_r <= kd_reg;
      kd_reg_s <= $signed(kd_reg) - $signed(kd_reg_r);
   end
end

assign kd_mult = $signed(error) * $signed(set_kd_i) ;












//---------------------------------------------------------------------------------
//  Sum together - saturate output

wire  [   33-1: 0] pid_sum     ; // biggest posible bit-width
reg   [   14-1: 0] pid_out     ;

always @(posedge clk_i) begin
   if (rstn_i == 1'b0) begin
      pid_out    <= 14'b0 ;
   end
   else begin
      if ({pid_sum[33-1],|pid_sum[32-2:13]} == 2'b01) //positive overflow
         pid_out <= 14'h1FFF ;
      else if ({pid_sum[33-1],&pid_sum[33-2:13]} == 2'b10) //negative overflow
         pid_out <= 14'h2000 ;
      else
         pid_out <= pid_sum[14-1:0] ;
   end
end

assign pid_sum = $signed(kp_reg) + $signed(int_shr) + $signed(kd_reg_s) ;






assign dat_o = pid_out ;



endmodule

