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

module red_pitaya_pid_block #(
  int unsigned PSR = 12,
  int unsigned ISR = 18,
  int unsigned DSR = 10
)(
  // system signals
  input  logic            clk ,  // clock
  input  logic            rstn,  // reset - active low
  // data
  input  logic [ 14-1: 0] dat_i,  // input data
  output logic [ 14-1: 0] dat_o,  // output data

  // settings
  input  logic [ 14-1: 0] set_sp_i,  // set point
  input  logic [ 14-1: 0] set_kp_i,  // Kp
  input  logic [ 14-1: 0] set_ki_i,  // Ki
  input  logic [ 14-1: 0] set_kd_i,  // Kd
  input  logic            int_rst_i  // integrator reset
);

////////////////////////////////////////////////////////////////////////////////
//  Set point error calculation
////////////////////////////////////////////////////////////////////////////////

logic [15-1:0] error;

always @(posedge clk)
if (rstn == 1'b0) begin
  error <= 15'h0 ;
end else begin
  error <= $signed(set_sp_i) - $signed(dat_i) ;
end

////////////////////////////////////////////////////////////////////////////////
//  Proportional part
////////////////////////////////////////////////////////////////////////////////

logic [29-PSR-1:0] kp_reg ;
logic [    29-1:0] kp_mult;

always @(posedge clk)
if (rstn == 1'b0) begin
  kp_reg  <= {29-PSR{1'b0}};
end else begin
  kp_reg <= kp_mult[29-1:PSR] ;
end

assign kp_mult = $signed(error) * $signed(set_kp_i);

////////////////////////////////////////////////////////////////////////////////
//  Integrator
////////////////////////////////////////////////////////////////////////////////

logic [    29-1: 0] ki_mult;
logic [    33-1: 0] int_sum;
logic [    32-1: 0] int_reg;
logic [32-ISR-1: 0] int_shr;

always @(posedge clk)
if (rstn == 1'b0) begin
  ki_mult  <= {29{1'b0}};
  int_reg  <= {32{1'b0}};
end else begin
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

assign int_sum = $signed(ki_mult) + $signed(int_reg) ;
assign int_shr = int_reg[32-1:ISR] ;

////////////////////////////////////////////////////////////////////////////////
//  Derivative
////////////////////////////////////////////////////////////////////////////////

logic [    29-1: 0] kd_mult ;
logic [29-DSR-1: 0] kd_reg  ;
logic [29-DSR-1: 0] kd_reg_r;
logic [29-DSR  : 0] kd_reg_s;

always @(posedge clk)
if (rstn == 1'b0) begin
  kd_reg   <= {29-DSR{1'b0}};
  kd_reg_r <= {29-DSR{1'b0}};
  kd_reg_s <= {29-DSR+1{1'b0}};
end else begin
  kd_reg   <= kd_mult[29-1:DSR] ;
  kd_reg_r <= kd_reg;
  kd_reg_s <= $signed(kd_reg) - $signed(kd_reg_r);
end

assign kd_mult = $signed(error) * $signed(set_kd_i) ;

////////////////////////////////////////////////////////////////////////////////
//  Sum together - saturate output
////////////////////////////////////////////////////////////////////////////////

wire  [   33-1: 0] pid_sum     ; // biggest posible bit-width
reg   [   14-1: 0] pid_out     ;

always @(posedge clk)
if (rstn == 1'b0) begin
  pid_out    <= 14'b0 ;
end else begin
  if ({pid_sum[33-1],|pid_sum[32-2:13]} == 2'b01) //positive overflow
    pid_out <= 14'h1FFF ;
  else if ({pid_sum[33-1],&pid_sum[33-2:13]} == 2'b10) //negative overflow
    pid_out <= 14'h2000 ;
  else
    pid_out <= pid_sum[14-1:0] ;
end

assign pid_sum = $signed(kp_reg) + $signed(int_shr) + $signed(kd_reg_s) ;


assign dat_o = pid_out ;

endmodule: red_pitaya_pid_block
