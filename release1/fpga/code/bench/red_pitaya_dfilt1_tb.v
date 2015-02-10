/**
 * $Id: red_pitaya_dfilt1_tb.v 964 2014-01-24 12:58:17Z matej.oblak $
 *
 * @brief Red Pitaya equalization filter testbench.
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
 * Testbench for Red Pitaya equalization filter.
 *
 * Testing values are read from file (dfilt1_sim_values.txt) and used as filter
 * input. Results are in similar saved to file (easier to analyze). 
 * 
 */



`timescale 1ns / 1ps

module red_pitaya_dfilt1_tb(
);



reg  [ 14-1: 0] adc_read    ;
reg  [ 14-1: 0] adc_in      ;
wire [ 14-1: 0] adc_out     ;
reg             adc_clk     ;
reg             adc_rstn    ;

reg  [ 18-1: 0] cfg_aa      ;
reg  [ 25-1: 0] cfg_bb      ;
reg  [ 25-1: 0] cfg_kk      ;
reg  [ 25-1: 0] cfg_pp      ;


red_pitaya_dfilt1 i_filt1
(
   // ADC
  .adc_clk_i   ( adc_clk    ),
  .adc_rstn_i  ( adc_rstn   ),
  .adc_dat_i   ( adc_in     ),
  .adc_dat_o   ( adc_out    ),

   // configuration
  .cfg_aa_i    ( cfg_aa     ),
  .cfg_bb_i    ( cfg_bb     ),
  .cfg_kk_i    ( cfg_kk     ),
  .cfg_pp_i    ( cfg_pp     ) 
);







//---------------------------------------------------------------------------------
//
// signal generation


// ADC clock & reset
initial begin
   adc_clk  <= 1'b0  ;
   adc_rstn <= 1'b0  ;
   repeat(10) @(posedge adc_clk);
      adc_rstn <= 1'b1  ;
end

always begin
   #4  adc_clk <= !adc_clk ;
end




// ADC signal generation from file
integer fp1,fp2;
integer dummy;

initial begin

   fp1 = $fopen("../../../../code/bench/dfilt1_sim_values.txt", "r");
   fp2 = $fopen("../../../../code/bench/dfilt1_sim_output.txt", "w");
   adc_read <= 14'h0 ;
   adc_in   <= 14'h0 ;

   cfg_aa <= 18'd40724 ;     // 18'h0 ;
   cfg_bb <= 25'd341536 ;    // 25'h0 ;
   cfg_kk <= 25'd14260634 ;  // 25'hffffff ;
   cfg_pp <= 25'd9830 ;      // 25'h0 ;

   wait (adc_rstn)
   repeat(2) @(posedge adc_clk);

   // read signal file
   while ( !$feof(fp1) ) begin
      @(posedge adc_clk);
      dummy = $fscanf(fp1,"%d \n", adc_read);
   end

   repeat(2) @(posedge adc_clk);
   $fclose(fp1);
end




// Save register values into file
always @(posedge adc_clk) begin
   adc_in <= adc_read ; //additional register, some problems with Vivado simulator

   if(adc_rstn == 1'b1) begin //out of reset
      $fwrite(fp2, "%d %d %d %d %d %d %d \n", $signed(i_filt1.r01_reg), $signed(i_filt1.r02_reg), 
                                              $signed(i_filt1.r1_reg), $signed(i_filt1.r2_reg), 
                                              $signed(i_filt1.r3_reg), $signed(i_filt1.r4_reg), $signed(i_filt1.r5_reg) ) ;
   end
end








endmodule

