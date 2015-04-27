/**
 * $Id: axi_master_model.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya AXI master model.
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
 * AXI master model used for simulation.
 *
 * In order to develop AXI interface some model is good to have. It has two
 * tasks, one for write second for read. At the time only single transfer 
 * is supported.
 * Since model is approximation of real system this has to be further upgraded.
 * 
 */

`timescale 1ns / 1ps

module axi_master_model #(
    parameter AW    = 32 ,
    parameter DW    = 32 ,
    parameter IW    =  4 ,
    parameter LW    =  4 
)(
   // AXI signals
   input                   aclk_i             ,
   input                   arstn_i            ,
   // AXI Write Address Channel Signals
   output reg [   IW-1: 0] awid_o             ,
   output reg [   LW-1: 0] awlen_o            ,
   output reg [    3-1: 0] awsize_o           ,
   output reg [    2-1: 0] awburst_o          ,
   output reg [    4-1: 0] awcache_o          ,
   output reg [   AW-1: 0] awaddr_o           ,
   output reg [    3-1: 0] awprot_o           ,
   output reg              awvalid_o          ,
   input                   awready_i          ,
   output reg [    2-1: 0] awlock_o           ,
   // AXI Write Data Channel Signals
   output reg [   DW-1: 0] wdata_o            ,
   output reg [ DW/8-1: 0] wstrb_o            ,
   output reg              wlast_o            ,
   output reg              wvalid_o           ,
   input                   wready_i           ,
   // AXI Write Response Channel Signals
   input      [   IW-1: 0] bid_i              ,
   input      [    2-1: 0] bresp_i            ,
   input                   bvalid_i           ,
   output reg              bready_o           ,
   // AXI Read Address Channel Signals
   output reg [   IW-1: 0] arid_o             ,
   output reg [   LW-1: 0] arlen_o            ,
   output reg [    3-1: 0] arsize_o           ,
   output reg [    2-1: 0] arburst_o          ,
   output reg [    3-1: 0] arprot_o           ,
   output reg [    4-1: 0] arcache_o          ,
   output reg              arvalid_o          ,
   output reg [   AW-1: 0] araddr_o           ,
   output reg [    2-1: 0] arlock_o           ,
   input                   arready_i          ,
   // AXI Read Data Channel Sigals
   input      [   IW-1: 0] rid_i              ,
   input      [   DW-1: 0] rdata_i            ,
   input      [    2-1: 0] rresp_i            ,
   input                   rvalid_i           ,
   input                   rlast_i            ,
   output reg              rready_o            
);

reg     wr_idle        ;
reg     rd_idle        ;

//---------------------------------------------------------------------------------
//
// Write single data task

initial
begin
   awid_o    <=   'h0   ;
   awlen_o   <=   'h0   ;
   awsize_o  <=  3'h2   ;
   awburst_o <=  2'h0   ;
   awcache_o <=  4'h0   ;
   awaddr_o  <=   'h0   ;
   awprot_o  <=  3'h0   ;
   awvalid_o <=  1'b0   ;
   awlock_o  <=  2'h0   ;
   wdata_o   <=   'h0   ;
   wstrb_o   <=   'h0   ;
   wlast_o   <=  1'b0   ;
   wvalid_o  <=  1'b0   ;
   bready_o  <=  1'b1   ;

   arid_o    <=   'h0   ;
   arlen_o   <=   'h0   ;
   arsize_o  <=  3'h2   ;
   arburst_o <=  2'h0   ;
   arprot_o  <=  3'h0   ;
   arcache_o <=  4'h0   ;
   arvalid_o <=  1'b0   ;
   araddr_o  <=   'h0   ;
   arlock_o  <=  2'h0   ;
   rready_o  <=  1'b1   ;
end

task wr_single ;
   input   [ AW-1: 0] adr_i       ;
   input   [ DW-1: 0] dat_i       ;
   input   [ IW-1: 0] id_i        ;
   input   [  3-1: 0] size_i      ;
   input   [  2-1: 0] lock_i      ;
   input   [  3-1: 0] prot_i      ;
   output  [  2-1: 0] resp_o      ;

   reg     [  2-1: 0] dat_resp    ;
   reg                in_use      ;
   reg     [ IW-1: 0] id          ;

begin:main

   if (in_use === 1'b1)
   begin
      $display("%m re-entered @ %t", $time) ;
      in_use = 1'b0   ;
      disable main    ;
   end

   if (arstn_i !== 1'b1)
   begin
      $display("%m called during axi reset @ %t", $time) ;
      disable main    ;
   end

   if ((size_i * 8) > DW)
   begin
      $display("%m size parameter out of range @ %t", $time) ;
      disable main    ;
   end

   in_use      <= 1'b1   ;
   wr_idle     <= 1'b0   ;
   id          <= id_i   ;

   fork
    begin //addres
      @(posedge aclk_i) ;
      awaddr_o <= adr_i  ;
      awvalid_o <= 1'b1  ;
      awid_o <= id_i     ;
      awlen_o <= 0       ;
      awsize_o <= size_i ;
      awburst_o <= 2'h0  ;
      awprot_o <= prot_i ; //3'b010 ;
      awcache_o <= 4'h0  ;
      awlock_o <= lock_i ;

      @(posedge aclk_i) ;
      while(awready_i === 1'b0)
         @(posedge aclk_i) ;

      awvalid_o <= 1'b0 ;
      bready_o  <= 1'b1 ;
    end
    begin // data
      @(posedge aclk_i) ;
      wvalid_o <= 1'b1 ;
      wdata_o <= dat_i ;
      wvalid_o <= 1'b1 ;
      wlast_o <= 1'b1  ;
      wstrb_o <= strb(adr_i[2:0],size_i[1:0]) ;

      @(posedge aclk_i) ;
      while(wready_i === 1'b0)
         @(posedge aclk_i) ;
      wvalid_o <= 1'b0 ;
      wlast_o  <= 1'b0 ;
    end
   join

   @(posedge aclk_i) ;
   while (bvalid_i === 1'b0) begin
      @(posedge aclk_i) ;
   end
   if(bid_i !== id)
      $display("%m Received ID is not correct! @ %t", $time) ;
   if(bresp_i != 0)
      $display("%m Received ERROR response! @ %t", $time) ;
   bready_o  <= 1'b0 ;

   in_use    <= 1'b0 ;
   wr_idle   <= 1'b1 ;
   @(posedge aclk_i) ;

   resp_o <= bresp_i ;
end
endtask: wr_single

//---------------------------------------------------------------------------------
//
// Read single data task

task rd_single ;
   input   [ AW-1: 0] adr_i       ;
   input   [ IW-1: 0] id_i        ;
   input   [  3-1: 0] size_i      ;
   input   [  2-1: 0] lock_i      ;
   input   [  3-1: 0] prot_i      ;
   output  [ DW-1: 0] dat_o       ;  
   output  [  2-1: 0] resp_o      ;
  

   reg     [  2-1: 0] dat_resp    ;
   reg                in_use      ;
   reg     [ IW-1: 0] id          ;

begin:main

   if (in_use === 1'b1)
   begin
      $display("%m re-entered @ %t", $time) ;
      in_use = 1'b0   ;
      disable main    ;
   end

   if (arstn_i !== 1'b1)
   begin
      $display("%m called during axi reset @ %t", $time) ;
      disable main    ;
   end

   if ((size_i * 8) > DW)
   begin
      $display("%m size parameter out of range @ %t", $time) ;
      disable main    ;
   end

   in_use      <= 1'b1   ;
   rd_idle     <= 1'b0   ;
   id          <= id_i   ;

   @(posedge aclk_i) ;
   araddr_o <= adr_i  ;
   arvalid_o <= 1'b1  ;
   arid_o <= id_i     ;
   arlen_o <= 0       ;
   arsize_o <= size_i ;
   arburst_o <= 2'h0  ;
   arprot_o <= prot_i ; //3'b010 ;
   arcache_o <= 4'h0  ;
   arlock_o <= lock_i ;

   @(posedge aclk_i) ;
   while(arready_i === 1'b0)
      @(posedge aclk_i) ;

   arvalid_o <= 1'b0 ;
   rready_o  <= 1'b1 ;

   while ((rvalid_i === 1'b0) && (rlast_i === 1'b0))
      @(posedge aclk_i) ;

   rready_o  <= 1'b0 ;

   if(rid_i !== id)
      $display("%m Received ID is not correct! @ %t", $time) ;
   if(rresp_i !== 0)
      $display("%m Received ERROR response! @ %t", $time) ;

   if( (rresp_i === 0) && (rid_i === id) ) begin
      $display("%m Received data  = 0x%h @ %t", rdata_i, $time) ;
      dat_o <= rdata_i ;
   end

   in_use    <= 1'b0 ;
   rd_idle   <= 1'b1 ;
   @(posedge aclk_i) ;

   resp_o <= rresp_i ;
end
endtask: rd_single


function [8-1: 0] strb ;
    input [3-1: 0] addr ;
    input [2-1: 0] size ;
begin
    case ({addr,size})
      //Byte
      5'b00000 : strb = 8'b00000001 ;
      5'b00100 : strb = 8'b00000010 ;
      5'b01000 : strb = 8'b00000100 ;
      5'b01100 : strb = 8'b00001000 ;
      5'b10000 : strb = 8'b00010000 ;
      5'b10100 : strb = 8'b00100000 ;
      5'b11000 : strb = 8'b01000000 ;
      5'b11100 : strb = 8'b10000000 ;
      //Halfword
      5'b00001 : strb = 8'b00000011 ;
      5'b01001 : strb = 8'b00001100 ;
      5'b10001 : strb = 8'b00110000 ;
      5'b11001 : strb = 8'b11000000 ;
      //Word
      5'b00010 : strb = 8'b00001111 ;
      5'b10010 : strb = 8'b11110000 ;
      //Double word
      5'b00011 : strb = 8'b11111111 ;
       default : strb = 8'b11111111 ;
    endcase
end
endfunction: strb

endmodule: axi_master_model
