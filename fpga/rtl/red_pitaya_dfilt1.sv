/**
 * $Id: red_pitaya_dfilt1.v 964 2014-01-24 12:58:17Z matej.oblak $
 *
 * @brief Red Pitaya equalization filter.
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
 * Filter to equalize input analog chain. 
 *
 */

module red_pitaya_dfilt1 (
   // ADC
   input                        adc_clk_i ,  // ADC clock
   input                        adc_rstn_i,  // ADC reset - active low
   input  logic signed [14-1:0] adc_dat_i ,  // ADC data
   output logic signed [14-1:0] adc_dat_o ,  // ADC data
   // configuration
   input  logic signed [18-1:0] cfg_aa_i,  // AA coefficient
   input  logic signed [25-1:0] cfg_bb_i,  // BB coefficient
   input  logic signed [25-1:0] cfg_kk_i,  // KK coefficient
   input  logic signed [25-1:0] cfg_pp_i   // PP coefficient
);

//---------------------------------------------------------------------------------
//  FIR

logic signed [39-1:0] bb_mult;
logic signed [33-1:0] r2_sum ;
logic signed [33-1:0] r1_reg ;
logic signed [23-1:0] r2_reg ;
logic signed [32-1:0] r01_reg;
logic signed [28-1:0] r02_reg;

assign bb_mult = adc_dat_i * cfg_bb_i;
assign r2_sum  = r01_reg + r1_reg;

always_ff @(posedge adc_clk_i)
if (~adc_rstn_i) begin
   r1_reg  <= '0;
   r2_reg  <= '0;
   r01_reg <= '0;
   r02_reg <= '0;
end else begin
   r1_reg  <= r02_reg - r01_reg;
   r2_reg  <= r2_sum >>> 10;
   r01_reg <= adc_dat_i <<< 18;
   r02_reg <= bb_mult >>> 10;
end

//---------------------------------------------------------------------------------
//  IIR 1

logic signed [41-1:0] aa_mult;
logic signed [49-1:0] r3_sum ; //24 + 25
(* use_dsp="yes" *) logic signed [23-1:0] r3_reg_dsp1;
(* use_dsp="yes" *) logic signed [23-1:0] r3_reg_dsp2;
logic signed [23-1:0] r3_reg_dsp3;

assign aa_mult = r3_reg_dsp1 * cfg_aa_i;
assign r3_sum  = (r2_reg <<< 25) + (r3_reg_dsp2 <<< 25) - aa_mult;

always_ff @(posedge adc_clk_i)
if (~adc_rstn_i) begin
   r3_reg_dsp1 <= '0;
   r3_reg_dsp2 <= '0;
   r3_reg_dsp3 <= '0;
end else begin
   r3_reg_dsp1 <= r3_sum >>> 25;
   r3_reg_dsp2 <= r3_sum >>> 25;
   r3_reg_dsp3 <= r3_sum >>> 33;
end

//---------------------------------------------------------------------------------
//  IIR 2

logic signed [40-1:0] pp_mult;
logic signed [16-1:0] r4_sum ;
logic signed [15-1:0] r4_reg ;
logic signed [15-1:0] r3_shr ;

assign pp_mult = r4_reg * cfg_pp_i;
assign r4_sum  = r3_shr + (pp_mult >>> 16);

always_ff @(posedge adc_clk_i)
if (~adc_rstn_i) begin
   r3_shr <= '0;
   r4_reg <= '0;
end else begin
   r3_shr <= r3_reg_dsp3;
   r4_reg <= r4_sum;
end

//---------------------------------------------------------------------------------
//  Scaling

logic signed [40-1:0] kk_mult  ;
logic signed [15-1:0] r4_reg_r ;
logic signed [15-1:0] r4_reg_rr;
logic signed [14-1:0] r5_reg   ;

assign kk_mult = r4_reg * cfg_kk_i;

always_ff @(posedge adc_clk_i)
if (adc_rstn_i == 1'b0) begin
   r5_reg    <= '0;
end else begin
   if      ((kk_mult >>> 24) > $signed(14'h1FFF))  r5_reg <= 14'h1FFF;
   else if ((kk_mult >>> 24) < $signed(14'h2000))  r5_reg <= 14'h2000;
   else                                            r5_reg <= kk_mult >>> 24;
end

assign adc_dat_o = r5_reg;

endmodule: red_pitaya_dfilt1
