/**
 * $Id: red_pitaya_pid_tb.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya MIMO PID testbench.
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
 * Testbench for MIMO PID controller.
 *
 * Testing red_pitaya_pid module using only PID11 section. For system which it
 * controls simple model is used (acts as system of first order). 
 * 
 */






`timescale 1ns / 1ps

module red_pitaya_pid_tb(
);



reg              clk             ;
reg              rstn            ;
reg   [ 14-1: 0] dat_a_in        ;
reg   [ 14-1: 0] dat_b_in        ;
wire  [ 14-1: 0] dat_a_out       ;
wire  [ 14-1: 0] dat_b_out       ;

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



red_pitaya_pid i_pid
(
   // signals
  .clk_i           (  clk           ),  // clock
  .rstn_i          (  rstn          ),  // reset - active low
  .dat_a_i         (  dat_a_in      ),  // in 1
  .dat_b_i         (  dat_b_in      ),  // in 2
  .dat_a_o         (  dat_a_out     ),  // out 1
  .dat_b_o         (  dat_b_out     ),  // out 2
  
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
// Simple module - first order system
genvar k;
reg  [14-1: 0] uut_reg [0:21];
reg  [20-1: 0] uut_sum ;
integer        uut_scl ;
reg  [14-1: 0] uut_aaa ;
reg  [14-1: 0] uut_bbb ;
reg  [14-1: 0] uut_out ;
reg            uut_en  ;

generate
for (k=0; k<21; k=k+1) begin : delay
   always @(posedge clk)
      uut_reg[k+1] <= uut_reg[k] ;
end
endgenerate

always @(posedge clk) begin
   uut_reg[0] <= dat_a_out ;
   uut_sum <= $signed(dat_a_out)   + $signed(uut_reg[ 0]) + $signed(uut_reg[ 1]) + $signed(uut_reg[ 2]) + $signed(uut_reg[ 3])
            + $signed(uut_reg[ 4]) + $signed(uut_reg[ 5]) + $signed(uut_reg[ 6]) + $signed(uut_reg[ 7]) + $signed(uut_reg[ 8])
            + $signed(uut_reg[ 9]) + $signed(uut_reg[10]) + $signed(uut_reg[11]) + $signed(uut_reg[12]) + $signed(uut_reg[13])
            + $signed(uut_reg[14]) + $signed(uut_reg[15]) + $signed(uut_reg[16]) + $signed(uut_reg[17]) + $signed(uut_reg[18]) ;

   uut_scl <= $signed(uut_sum) / 20 ; // make simple avarage
   uut_aaa <= uut_scl ;
   uut_bbb <= uut_aaa ;
   uut_out <= uut_scl ;
   
   dat_a_in <= uut_en ? uut_out : 14'h0 ;
end



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




reg [8-1: 0] ch0_set;
reg [8-1: 0] ch1_set;

initial begin
   dat_b_in <= 14'h0 ;
   uut_en   <=  1'b0 ;

   wait (sys_rstn && rstn)
   repeat(10) @(posedge sys_clk);
   //PID settings
      i_bus.bus_write(32'h10, 32'd7000  );  // set point
      i_bus.bus_write(32'h14,-32'd3000  );  // Kp
      i_bus.bus_write(32'h18, 32'd1000  );  // Ki
      i_bus.bus_write(32'h1C, 32'd1000  );  // Kd


   repeat(100) @(posedge clk);
      uut_en <= 1'b1 ;

   //Int reset
   repeat(20) @(posedge clk);
      i_bus.bus_write(32'h00, 32'b1110  );  // int reset


   repeat(2000000) @(posedge clk);

end




endmodule

