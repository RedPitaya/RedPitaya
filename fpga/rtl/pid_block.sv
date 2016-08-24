////////////////////////////////////////////////////////////////////////////////
// Module: PID controller
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

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

module pid_block #(
  // stream parameters
  int unsigned DWI = 14,
  int unsigned DWO = 14,
  // configuration parameters
  int unsigned PSR = 12,
  int unsigned ISR = 18,
  int unsigned DSR = 10
)(
  // system signals
  input  logic           clk ,  // clock
  input  logic           rstn,  // reset - active low
  // data
  input  logic signed [DWI-1:0] dat_i,  // input data
  output logic signed [DWO-1:0] dat_o,  // output data
  // configuration
  input  logic signed [DWI-1: 0] set_sp,  // set point
  input  logic signed [ 14-1: 0] set_kp,  // Kp
  input  logic signed [ 14-1: 0] set_ki,  // Ki
  input  logic signed [ 14-1: 0] set_kd,  // Kd
  // control
  input  logic                   int_rst  // integrator reset
);

////////////////////////////////////////////////////////////////////////////////
//  Set point error calculation
////////////////////////////////////////////////////////////////////////////////

logic signed [DWI+1-1:0] error;

always_ff @(posedge clk)
if (!rstn) begin
  error <= '0;
end else begin
  error <= set_sp - dat_i;
end

////////////////////////////////////////////////////////////////////////////////
//  Proportional part
////////////////////////////////////////////////////////////////////////////////

logic signed [29-PSR-1:0] kp_reg;
logic signed [29    -1:0] kp_mult;

always_ff @(posedge clk)
if (!rstn) begin
  kp_reg  <= '0;
end else begin
  kp_reg <= kp_mult[29-1:PSR];
end

assign kp_mult = error * set_kp;

////////////////////////////////////////////////////////////////////////////////
//  Integrator
////////////////////////////////////////////////////////////////////////////////

logic signed [    29-1: 0] ki_mult;
logic signed [    33-1: 0] int_sum;
logic signed [    32-1: 0] int_reg;
logic signed [32-ISR-1: 0] int_shr;

always_ff @(posedge clk)
if (!rstn) begin
  ki_mult  <= '0;
  int_reg  <= '0;
end else begin
  ki_mult <= error * set_ki;
  if (int_rst)
    int_reg <= '0;
  else
    // saturation
    int_reg <= ^int_sum[33-1:33-2] ? {int_sum[33-1], {32-1{~int_sum[33-1]}}}
                                   :  int_sum[32-1:0];
end

assign int_sum = ki_mult + int_reg;
assign int_shr = int_reg[32-1:ISR] ;

////////////////////////////////////////////////////////////////////////////////
//  Derivative
////////////////////////////////////////////////////////////////////////////////

logic signed [    29-1: 0] kd_mult ;
logic signed [29-DSR-1: 0] kd_reg  ;
logic signed [29-DSR-1: 0] kd_reg_r;
logic signed [29-DSR  : 0] kd_reg_s;

always_ff @(posedge clk)
if (!rstn) begin
  kd_reg   <= '0;
  kd_reg_r <= '0;
  kd_reg_s <= '0;
end else begin
  kd_reg   <= kd_mult[29-1:DSR];
  kd_reg_r <= kd_reg;
  kd_reg_s <= kd_reg - kd_reg_r;
end

assign kd_mult = error * set_kd;

////////////////////////////////////////////////////////////////////////////////
//  Sum together - saturate output
////////////////////////////////////////////////////////////////////////////////

logic signed [33-1:0] pid_sum; // biggest posible bit-width

assign pid_sum = kp_reg + int_shr + kd_reg_s;

always_ff @(posedge clk)
if (!rstn) begin
  dat_o <= '0;
end else begin
  if ({pid_sum[33-1],|pid_sum[32-2:13]} == 2'b01) //positive overflow
    dat_o <= 14'h1FFF ;
  else if ({pid_sum[33-1],&pid_sum[33-2:13]} == 2'b10) //negative overflow
    dat_o <= 14'h2000 ;
  else
    dat_o <= pid_sum[14-1:0] ;
end

endmodule: pid_block
