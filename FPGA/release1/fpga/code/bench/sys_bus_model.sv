/**
 * $Id: sys_bus_model.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya system bus model.
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
 * Simple model of Red Pitaya system bus.
 *
 * In order to simplify other testbenches this model was written. It has two
 * task, one for write other for read. Both serves as replacement for PS-AXI
 * control of Red Pitaya system bus.
 * 
 */

`timescale 1ns / 1ps

module sys_bus_model #(
  int unsigned AXI_DW = 32      , // data width (8,16,...,1024)
  int unsigned AXI_AW = 32      , // address width
  int unsigned AXI_SW = AXI_DW/8  // strobe width - 1 bit for every data byte
)(
  // system signals
  input  logic              sys_clk_i  ,  // system clock
  input  logic              sys_rstn_i ,  // system reset - active low
  // bus protocol signals
  output logic [AXI_AW-1:0] sys_addr_o ,  // system read/write address.
  output logic [AXI_DW-1:0] sys_wdata_o,  // system write data.
  output logic [AXI_SW-1:0] sys_sel_o  ,  // system write byte select.
  output logic              sys_wen_o  ,  // system write enable.
  output logic              sys_ren_o  ,  // system read enable.
  input  logic [AXI_DW-1:0] sys_rdata_i,  // system read data.
  input  logic              sys_err_i  ,  // system error indicator.
  input  logic              sys_ack_i     // system acknowledge signal.
);

initial begin
   sys_wen_o <= 1'b0 ;
   sys_ren_o <= 1'b0 ;
end

//---------------------------------------------------------------------------------

task bus_write (
  input  logic [32-1:0] addr,
  input  logic [32-1:0] wdata
);
  @(posedge sys_clk_i)
  sys_sel_o    <= '1;
  sys_wen_o    <= 1'b1  ;
  sys_addr_o   <= addr  ;
  sys_wdata_o  <= wdata ;
  @(posedge sys_clk_i);
  sys_wen_o    <= 1'b0  ;
  while (~sys_ack_i)
    @(posedge sys_clk_i);
endtask: bus_write

//---------------------------------------------------------------------------------

task bus_read (
  input  logic [32-1:0] addr,
  output logic [32-1:0] rdata
);
  @(posedge sys_clk_i)
  sys_ren_o    <= 1'b1  ;
  sys_addr_o   <= addr  ;
  @(posedge sys_clk_i);
  sys_ren_o    <= 1'b0  ;
  while (~sys_ack_i)
    @(posedge sys_clk_i);
  rdata <= sys_rdata_i ;
  @(posedge sys_clk_i);
  $display ("@%g Readed value at %h is %h", $time, addr[19:0], rdata);
endtask: bus_read

endmodule: sys_bus_model
