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
 * Clock domain bridge for system bus.
 *
 *
 *                  /------\
 *   SYSTEM         |      |         PROCESSING
 *   BUS    <-----> | SYNC | <----->    BUS
 *                  |      |
 *                  \------/
 *
 *
 * System bus runs on one clock domain while processing runs on separate. To 
 * simplify transition of writing and reading data this bridge was created.
 * 
 */




module bus_clk_bridge
(
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
   output reg [ 32-1: 0] addr_o        ,  //!< address
   output reg [ 32-1: 0] wdata_o       ,  //!< write data
   output                wen_o         ,  //!< write enable
   output                ren_o         ,  //!< read enable
   input      [ 32-1: 0] rdata_i       ,  //!< read data
   input                 err_i         ,  //!< error indicator
   input                 ack_i            //!< acknowledge signal
);



//---------------------------------------------------------------------------------
//  Synchronize signals between clock domains

reg            sys_rd    ;
reg            sys_wr    ;
reg            sys_do    ;
reg  [ 2-1: 0] sys_sync  ;
reg            sys_done  ;
reg            dst_do    ;
reg  [ 2-1: 0] dst_sync  ;
reg            dst_done  ;

always @(posedge sys_clk_i) begin
   if (sys_rstn_i == 1'b0) begin
      sys_rd   <= 1'b0 ;
      sys_wr   <= 1'b0 ;
      sys_do   <= 1'b0 ;
      sys_sync <= 2'h0 ;
      sys_done <= 1'b0 ;
   end 
   else begin

      if ((sys_do == sys_done) && (sys_wen_i || sys_ren_i)) begin
         addr_o  <= sys_addr_i    ;
         wdata_o <= sys_wdata_i   ;
         sys_rd  <= sys_ren_i     ;
         sys_wr  <= sys_wen_i     ;
         sys_do  <= !sys_do       ;
      end

      sys_sync <= {sys_sync[0], dst_done};
      sys_done <= sys_sync[1];
   end
end


always @(posedge clk_i) begin
   if (rstn_i == 1'b0) begin
      dst_do    <= 1'b0 ;
      dst_sync  <= 2'h0 ;
      dst_done  <= 1'b0 ;
   end
   else begin
      dst_sync <= {dst_sync[0], sys_do};
      dst_do   <= dst_sync[1];

      if (ack_i && (dst_do != dst_done))
         dst_done <= dst_do;
   end
end

assign ren_o = sys_rd && (dst_sync[1]^dst_do);
assign wen_o = sys_wr && (dst_sync[1]^dst_do);


assign sys_rdata_o = rdata_i                ;
assign sys_err_o   = err_i                  ;
assign sys_ack_o   = sys_done ^ sys_sync[1] ;



endmodule
