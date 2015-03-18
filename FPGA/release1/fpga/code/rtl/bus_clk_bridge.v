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




module bus_clk_bridge (
   // system bus
   input                 sys_clk_i     ,  //!< bus clock
   input                 sys_rstn_i    ,  //!< bus reset - active low
   input      [ 32-1: 0] sys_addr_i    ,  //!< bus address
   input      [ 32-1: 0] sys_wdata_i   ,  //!< bus write data
   input      [  4-1: 0] sys_sel_i     ,  //!< bus write byte select
   input                 sys_wen_i     ,  //!< bus write enable
   input                 sys_ren_i     ,  //!< bus read enable
   output reg [ 32-1: 0] sys_rdata_o   ,  //!< bus read data
   output reg            sys_err_o     ,  //!< bus error indicator
   output reg            sys_ack_o     ,  //!< bus acknowledge signal
   // Destination bus
   input                 clk_i         ,  //!< clock
   input                 rstn_i        ,  //!< reset - active low
   output reg [ 32-1: 0] addr_o        ,  //!< address
   output reg [ 32-1: 0] wdata_o       ,  //!< write data
   output reg            wen_o         ,  //!< write enable
   output reg            ren_o         ,  //!< read enable
   input      [ 32-1: 0] rdata_i       ,  //!< read data
   input                 err_i         ,  //!< error indicator
   input                 ack_i            //!< acknowledge signal
);

//---------------------------------------------------------------------------------
//  Synchronize signals between clock domains

reg            tmp_rd   ;
reg            tmp_wr   ;
reg  [32-1: 0] tmp_addr ;
reg  [32-1: 0] tmp_wdata;
reg  [32-1: 0] tmp_rdata;

// chnhronization and handshaking
reg            sys_do    ;
reg  [ 2-1: 0] sys_sync  ;
reg            sys_done  ;
reg            dst_do    ;
reg  [ 2-1: 0] dst_sync  ;
reg            dst_done  ;

always @(posedge sys_clk_i)
   if (sys_rstn_i == 1'b0) begin
      tmp_rd   <= 1'b0 ;
      tmp_wr   <= 1'b0 ;
      sys_do   <= 1'b0 ;
      sys_sync <= 2'h0 ;
      sys_done <= 1'b0 ;
   end else begin
      if ((sys_do == sys_done) && (sys_wen_i || sys_ren_i)) begin
         tmp_addr  <= sys_addr_i ;
         tmp_wdata <= sys_wdata_i;
         tmp_rd    <= sys_ren_i  ;
         tmp_wr    <= sys_wen_i  ;
         sys_do  <= !sys_do;
      end
      sys_sync <= {sys_sync[0], dst_done};
      sys_done <= sys_sync[1];
   end

always @(posedge clk_i)
   if (rstn_i == 1'b0) begin
      dst_do    <= 1'b0 ;
      dst_sync  <= 2'h0 ;
      dst_done  <= 1'b0 ;
   end else begin
      dst_sync <= {dst_sync[0], sys_do};
      dst_do   <= dst_sync[1];

      if (ack_i && (dst_do != dst_done))
         dst_done <= dst_do;
   end

always @(posedge clk_i)
if (rstn_i == 1'b0) begin
   ren_o <= 1'b0;
   wen_o <= 1'b0;
end else begin
   ren_o <= tmp_rd && (dst_sync[1]^dst_do);
   wen_o <= tmp_wr && (dst_sync[1]^dst_do);
end

always @(posedge clk_i)
if (dst_sync[1]^dst_do) begin
   addr_o  <= tmp_addr ;
   wdata_o <= tmp_wdata;
end

always @(posedge clk_i)
if (ack_i)  tmp_rdata <= rdata_i;

always @(posedge sys_clk_i)
if (sys_done ^ sys_sync[1])  sys_rdata_o <= tmp_rdata;

always @(posedge sys_clk_i)
if (sys_rstn_i == 1'b0) begin
   sys_err_o <= 1'b0;
   sys_ack_o <= 1'b0;
end else begin
   sys_err_o <= err_i                 ;
   sys_ack_o <= sys_done ^ sys_sync[1];
end

endmodule
