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
   input                 adc_clk_i       ,  //!< ADC clock
   input                 adc_rstn_i      ,  //!< ADC reset - active low
   input      [ 14-1: 0] adc_dat_i       ,  //!< ADC data
   output     [ 14-1: 0] adc_dat_o       ,  //!< ADC data

   // configuration
   input      [ 18-1: 0] cfg_aa_i        ,  //!< config AA coefficient
   input      [ 25-1: 0] cfg_bb_i        ,  //!< config BB coefficient
   input      [ 25-1: 0] cfg_kk_i        ,  //!< config KK coefficient
   input      [ 25-1: 0] cfg_pp_i           //!< config PP coefficient
);

//---------------------------------------------------------------------------------
//  register configuration - timing improvements

reg  [ 18-1: 0] cfg_aa_r  ;
reg  [ 25-1: 0] cfg_bb_r  ;
reg  [ 25-1: 0] cfg_kk_r  ;
reg  [ 25-1: 0] cfg_pp_r  ;

always @(posedge adc_clk_i) begin
   cfg_aa_r <= cfg_aa_i ;
   cfg_bb_r <= cfg_bb_i ;
   cfg_kk_r <= cfg_kk_i ;
   cfg_pp_r <= cfg_pp_i ;
end

//---------------------------------------------------------------------------------
//  FIR

wire [ 39-1: 0] bb_mult   ;
wire [ 33-1: 0] r2_sum    ;
reg  [ 33-1: 0] r1_reg    ;
reg  [ 23-1: 0] r2_reg    ;
reg  [ 32-1: 0] r01_reg   ;
reg  [ 28-1: 0] r02_reg   ;


assign bb_mult = $signed(adc_dat_i) * $signed(cfg_bb_r);
assign r2_sum  = $signed(r01_reg) + $signed(r1_reg);

always @(posedge adc_clk_i) begin
   if (adc_rstn_i == 1'b0) begin
      r1_reg  <= 33'h0 ;
      r2_reg  <= 23'h0 ;
      r01_reg <= 32'h0 ;
      r02_reg <= 28'h0 ;
   end
   else begin
      r1_reg  <= $signed(r02_reg) - $signed(r01_reg) ;
      r2_reg  <= r2_sum[33-1:10];
      r01_reg <= {adc_dat_i,18'h0};
      r02_reg <= bb_mult[39-2:10];
   end
end

//---------------------------------------------------------------------------------
//  IIR 1

wire [ 41-1: 0] aa_mult   ;
wire [ 49-1: 0] r3_sum    ; //24 + 25
(* use_dsp48="yes" *) reg  [ 23-1: 0] r3_reg    ;


assign aa_mult = $signed(r3_reg) * $signed(cfg_aa_r);
assign r3_sum  = $signed({r2_reg,25'h0}) + $signed({r3_reg,25'h0}) - $signed(aa_mult[41-1:0]);

always @(posedge adc_clk_i) begin
   if (adc_rstn_i == 1'b0) begin
      r3_reg  <= 23'h0 ;
   end
   else begin
      r3_reg  <= r3_sum[49-2:25] ;
   end
end

//---------------------------------------------------------------------------------
//  IIR 2

wire [ 40-1: 0] pp_mult   ;
wire [ 16-1: 0] r4_sum    ;
reg  [ 15-1: 0] r4_reg    ;
reg  [ 15-1: 0] r3_shr    ;

assign pp_mult = $signed(r4_reg) * $signed(cfg_pp_r);
assign r4_sum  = $signed(r3_shr) + $signed(pp_mult[40-2:16]);

always @(posedge adc_clk_i) begin
   if (adc_rstn_i == 1'b0) begin
      r3_shr <= 15'h0 ;
      r4_reg <= 15'h0 ;
   end
   else begin
      r3_shr <= r3_reg[23-1:8] ;
      r4_reg <= r4_sum[16-2:0] ;
   end
end

//---------------------------------------------------------------------------------
//  Scaling

wire [ 40-1: 0] kk_mult   ;
reg  [ 15-1: 0] r4_reg_r  ;
reg  [ 15-1: 0] r4_reg_rr ;
reg  [ 14-1: 0] r5_reg    ;

assign kk_mult = $signed(r4_reg_rr) * $signed(cfg_kk_r);

always @(posedge adc_clk_i) begin
   if (adc_rstn_i == 1'b0) begin
      r4_reg_r  <= 15'h0 ;
      r4_reg_rr <= 15'h0 ;
      r5_reg    <= 14'h0 ;
   end
   else begin
      r4_reg_r  <= r4_reg   ;
      r4_reg_rr <= r4_reg_r ;

      if ($signed(kk_mult[40-2:24]) > $signed(14'h1FFF))
         r5_reg <= 14'h1FFF ;
      else if ($signed(kk_mult[40-2:24]) < $signed(14'h2000))
         r5_reg <= 14'h2000 ;
      else
         r5_reg <= kk_mult[24+14-1:24];
   end
end

assign adc_dat_o = r5_reg ;

endmodule
