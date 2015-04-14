/**
 * $Id: red_pitaya_hk_tb.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya house keeping testbench.
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
 * Testbench for house keeping module.
 *
 * Simple testbench to test register read and write. Testing expansion connector
 * and DNA macro.
 * 
 */




`timescale 1ns / 1ps

module red_pitaya_hk_tb(
);



wire  [  8-1: 0] led             ;
wire  [  8-1: 0] exp_dat         ;
PULLDOWN i_p0 (.O(exp_dat[0])) ;
PULLDOWN i_p1 (.O(exp_dat[1])) ;
PULLDOWN i_p2 (.O(exp_dat[2])) ;
PULLDOWN i_p3 (.O(exp_dat[3])) ;
PULLDOWN i_p4 (.O(exp_dat[4])) ;
PULLDOWN i_p5 (.O(exp_dat[5])) ;
PULLDOWN i_p6 (.O(exp_dat[6])) ;
PULLDOWN i_p7 (.O(exp_dat[7])) ;
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



red_pitaya_hk i_hk
(

  .clk_i           (  clk           ),  // clock
  .rstn_i          (  rstn          ),  // reset - active low

   // LED
  .led_o           (  led           ),  // LED output
   // Expansion connector
  .exp_p_io        (  exp_dat       ),
  .exp_n_io        (  exp_dat       ),


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
   repeat(600) @(posedge sys_clk);
      // LED & Expansion
      i_bus.bus_write(32'h30, 32'h3     ); // LED
      i_bus.bus_write(32'h10, 32'h33    ); // direction
      i_bus.bus_write(32'h18, 32'hFF    ); // value

      i_bus.bus_read(32'h4    );
      i_bus.bus_read(32'h8    );
      i_bus.bus_read(32'h24   );
      i_bus.bus_read(32'h30   );
   repeat(20000) @(posedge clk);

end




endmodule
