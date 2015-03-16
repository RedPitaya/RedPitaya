/**
 * $Id: bus_clk_bridge.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya system bus clock crossing bridge.
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
 * This is now just a dummy pass through.
 * 
 */

module bus_clk_bridge (
   // system bus
   input                 sys_clk_i     ,  //!< bus clock
   input                 sys_rstn_i    ,  //!< bus reset - active low
   input      [ 32-1: 0] sys_addr_i    ,  //!< bus address
   input      [ 32-1: 0] sys_wdata_i   ,  //!< bus write data
   input      [  4-1: 0] sys_sel_i     ,  //!< bus write byte select
   input                 sys_wen_i     ,  //!< bus write enable
   input                 sys_ren_i     ,  //!< bus read enable
   output     [ 32-1: 0] sys_rdata_o   ,  //!< bus read data
   output                sys_err_o     ,  //!< bus error indicator
   output                sys_ack_o     ,  //!< bus acknowledge signal
   // Destination bus
   input                 clk_i         ,  //!< clock
   input                 rstn_i        ,  //!< reset - active low
   output     [ 32-1: 0] addr_o        ,  //!< address
   output     [ 32-1: 0] wdata_o       ,  //!< write data

   output                wen_o         ,  //!< write enable
   output                ren_o         ,  //!< read enable
   input      [ 32-1: 0] rdata_i       ,  //!< read data
   input                 err_i         ,  //!< error indicator
   input                 ack_i            //!< acknowledge signal
);

assign addr_o  = sys_addr_i ;
assign wdata_o = sys_wdata_i;
assign wen_o   = sys_wen_i;
assign ren_o   = sys_ren_i;
assign sys_rdata_o = rdata_i;
assign sys_err_o   = err_i  ;
assign sys_ack_o   = ack_i  ;

endmodule
