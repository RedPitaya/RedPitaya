/**
 * $Id: red_pitaya_ams_tb.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya analog mixed signal testbench.
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
 * Testbench for analog mixed signal module .
 *
 * This bench tests functionality of XADC, values which are used as analog 
 * inputs are defined in xadc_sim_values.txt.
 * 
 */



`timescale 1ns / 1ps

module red_pitaya_ams_tb(
);

reg              clk             ;
reg              rstn            ;

reg              sys_clk         ;
reg              sys_rstn        ;
wire  [ 32-1: 0] sys_addr        ;
wire  [ 32-1: 0] sys_wdata       ;
wire  [  4-1: 0] sys_sel         ;
wire             sys_wen         ;
wire             sys_ren         ;
wire  [ 32-1: 0] sys_rdata       ;
wire             sys_err         ;
wire             sys_ack         ;


sys_bus_model i_bus
(
  .sys_clk_i      (  sys_clk      ),
  .sys_rstn_i     (  sys_rstn     ),
  .sys_addr_o     (  sys_addr     ),
  .sys_wdata_o    (  sys_wdata    ),
  .sys_sel_o      (  sys_sel      ),
  .sys_wen_o      (  sys_wen      ),
  .sys_ren_o      (  sys_ren      ),
  .sys_rdata_i    (  sys_rdata    ),
  .sys_err_i      (  sys_err      ),
  .sys_ack_i      (  sys_ack      ) 
);



red_pitaya_ams i_ams
(
  .clk_i           (  clk           ),  // clock
  .rstn_i          (  rstn          ),  // reset - active low

  .vinp_i          (     ),  // voltages p
  .vinn_i          (     ),  // voltages n

  .dac_a_o         (     ),
  .dac_b_o         (     ),
  .dac_c_o         (     ),
  .dac_d_o         (     ),

   // System bus
  .sys_clk_i       (  sys_clk       ),  // clock
  .sys_rstn_i      (  sys_rstn      ),  // reset - active low
  .sys_addr_i      (  sys_addr      ),  // address
  .sys_wdata_i     (  sys_wdata     ),  // write data
  .sys_sel_i       (  sys_sel       ),  // write byte select
  .sys_wen_i       (  sys_wen       ),  // write enable
  .sys_ren_i       (  sys_ren       ),  // read enable
  .sys_rdata_o     (  sys_rdata     ),  // read data
  .sys_err_o       (  sys_err       ),  // error indicator
  .sys_ack_o       (  sys_ack       )   // acknowledge signal
);





//---------------------------------------------------------------------------------
//
// signal generation

initial begin
   sys_clk  <= 1'b0 ;
   sys_rstn <= 1'b0 ;
   repeat(10) @(posedge sys_clk);
      sys_rstn <= 1'b1  ;
end

always begin
   #5  sys_clk <= !sys_clk ;
end



initial begin
   clk  <= 1'b0  ;
   rstn <= 1'b0  ;
   repeat(10) @(posedge clk);
      rstn <= 1'b1  ;
end

always begin
   #4  clk <= !clk ;
end




initial begin
   wait (sys_rstn && rstn)
   repeat(10) @(posedge sys_clk);

   i_bus.bus_write(32'h20,32'h10000);  // test write

   repeat(20000) @(posedge clk);

   i_bus.bus_read(32'h30);
   i_bus.bus_read(32'h34);
   i_bus.bus_read(32'h38);
   i_bus.bus_read(32'h20);
   i_bus.bus_read(32'h24);

end





endmodule
