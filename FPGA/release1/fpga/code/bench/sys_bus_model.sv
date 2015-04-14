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
  input  logic              clk  ,  // system clock
  input  logic              rstn ,  // system reset - active low
  // bus protocol signals
  output logic [AXI_AW-1:0] sys_addr ,  // system read/write address.
  output logic [AXI_DW-1:0] sys_wdata,  // system write data.
  output logic [AXI_SW-1:0] sys_sel  ,  // system write byte select.
  output logic              sys_wen  ,  // system write enable.
  output logic              sys_ren  ,  // system read enable.
  input  logic [AXI_DW-1:0] sys_rdata,  // system read data.
  input  logic              sys_err  ,  // system error indicator.
  input  logic              sys_ack     // system acknowledge signal.
);

initial begin
   sys_wen <= 1'b0 ;
   sys_ren <= 1'b0 ;
end

// bus write transfer
task transaction (
  input  logic          we,
  input  logic [32-1:0] addr,
  input  logic [32-1:0] wdata,
  output logic [32-1:0] rdata
);
  @(posedge clk)
  sys_sel    <= '1;
  sys_wen    <=  we   ;
  sys_ren    <= ~we   ;
  sys_addr   <= addr  ;
  sys_wdata  <= wdata ;
  @(posedge clk);
  sys_wen    <= 1'b0  ;
  sys_ren    <= 1'b0  ;
  while (~sys_ack & ~sys_err)
    @(posedge clk);
  rdata <= sys_rdata ;
  @(posedge clk);
endtask: transaction

// bus write transfer
task write (
  input  logic [32-1:0] addr,
  input  logic [32-1:0] wdata
);
  logic [32-1:0] rdata;
  transaction (.we (1'b1), .addr (addr), .wdata (wdata), .rdata (rdata));
endtask: write

// bus read transfer
task read (
  input  logic [32-1:0] addr,
  output logic [32-1:0] rdata
);
  transaction (.we (1'b0), .addr (addr), .wdata (32'hx), .rdata (rdata));
endtask: read

endmodule: sys_bus_model
