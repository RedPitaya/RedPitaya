/*
* Copyright (c) 2015 Instrumentation Technologies
* All Rights Reserved.
*
* $Id: $
*/


// synopsys translate_off
`timescale 1ns / 1ps
// synopsys translate_on

module axi_master #(
  parameter    DW    =  64      , // data width (8,16,...,1024)
  parameter    AW    =  32      , // address width
  parameter    ID    =   0      , // master ID
  parameter    IW    =   4      , // master ID width
  parameter    LW    =   4      , // length width
  parameter    SW    = DW >> 3    // strobe width - 1 bit for every data byte
)
(
   // global signals
   input                   axi_clk_i      , // global clock
   input                   axi_rstn_i     , // global reset

   // axi write address channel
   output     [   IW-1: 0] axi_awid_o     , // write address ID
   output reg [   AW-1: 0] axi_awaddr_o   , // write address
   output reg [    4-1: 0] axi_awlen_o    , // write burst length
   output     [    3-1: 0] axi_awsize_o   , // write burst size
   output reg [    2-1: 0] axi_awburst_o  , // write burst type
   output     [    2-1: 0] axi_awlock_o   , // write lock type
   output     [    4-1: 0] axi_awcache_o  , // write cache type
   output     [    3-1: 0] axi_awprot_o   , // write protection type
   output reg              axi_awvalid_o  , // write address valid
   input                   axi_awready_i  , // write ready

   // axi write data channel
   output     [   IW-1: 0] axi_wid_o      , // write data ID
   output reg [   DW-1: 0] axi_wdata_o    , // write data
   output reg [   SW-1: 0] axi_wstrb_o    , // write strobes
   output reg              axi_wlast_o    , // write last
   output reg              axi_wvalid_o   , // write valid
   input                   axi_wready_i   , // write ready

   // axi write response channel
   input      [   IW-1: 0] axi_bid_i      , // write response ID
   input      [    2-1: 0] axi_bresp_i    , // write response
   input                   axi_bvalid_i   , // write response valid
   output                  axi_bready_o   , // write response ready

   // axi read address channel
   output     [   IW-1: 0] axi_arid_o     , // read address ID
   output reg [   AW-1: 0] axi_araddr_o   , // read address
   output reg [    4-1: 0] axi_arlen_o    , // read burst length
   output     [    3-1: 0] axi_arsize_o   , // read burst size
   output reg [    2-1: 0] axi_arburst_o  , // read burst type
   output     [    2-1: 0] axi_arlock_o   , // read lock type
   output     [    4-1: 0] axi_arcache_o  , // read cache type
   output     [    3-1: 0] axi_arprot_o   , // read protection type
   output reg              axi_arvalid_o  , // read address valid
   input                   axi_arready_i  , // read address ready
    
   // axi read data channel
   input      [   IW-1: 0] axi_rid_i      , // read response ID
   input      [   DW-1: 0] axi_rdata_i    , // read data
   input      [    2-1: 0] axi_rresp_i    , // read response
   input                   axi_rlast_i    , // read last
   input                   axi_rvalid_i   , // read response valid
   output reg              axi_rready_o   , // read response ready
    
   // system write channel
   input      [   AW-1: 0] sys_waddr_i    , // system write address
   input      [   DW-1: 0] sys_wdata_i    , // system write data
   input      [   SW-1: 0] sys_wsel_i     , // system write byte select
   input                   sys_wvalid_i   , // system write data valid
   input      [    4-1: 0] sys_wlen_i     , // system write burst length
   input                   sys_wfixed_i   , // system write burst type (fixed / incremental)
   output reg              sys_werr_o     , // system write error
   output reg              sys_wrdy_o     , // system write ready

   // system read channel
   input      [   AW-1: 0] sys_raddr_i    , // system read address
   input                   sys_rvalid_i   , // system read address valid
   input      [   SW-1: 0] sys_rsel_i     , // system read byte select
   input      [    4-1: 0] sys_rlen_i     , // system read burst length
   input                   sys_rfixed_i   , // system read burst type (fixed / incremental)
   output reg [   DW-1: 0] sys_rdata_o    , // system read data
   output reg              sys_rrdy_o     , // system read data is ready
   output reg              sys_rerr_o       // system read error
);





//---------------------------------------------------------------------------------
//
// Write address channel

assign axi_awid_o    = ID ;
assign axi_awsize_o  = {2'b01,(DW==64)} ; // 4 or 8 byte transfer      ; // write burst size
assign axi_awlock_o  = 2'h0   ; // normal
assign axi_awcache_o = 4'h0   ; // non-cacheable
assign axi_awprot_o  = 3'b010 ; // data, non-secured, unprivileged

reg [    4-1: 0] wr_cnt           ;
reg [    4-1: 0] axi_awwr_pt      ;
reg [    4-1: 0] axi_awrd_pt      ;
reg [    4-1: 0] axi_awfill_lvl   ;
reg [ 5+AW-1: 0] axi_awfifo[15:0] ;  //synthesis attribute ram_style of axi_awfifo is "distributed";
reg              awdata_in_reg    ;

wire axi_wlast ;
wire axi_wpush ;

wire axi_awpop  = (!awdata_in_reg || axi_awready_i) && |axi_awfill_lvl ;
wire axi_awpush = sys_wvalid_i && sys_wrdy_o && !wr_cnt ;

always @ (posedge axi_clk_i)
begin
   if (!axi_rstn_i) begin
      axi_awwr_pt <= 4'h0 ;
   end
   else if (axi_awpush) begin
      axi_awfifo[axi_awwr_pt] <= {!sys_wfixed_i,sys_wlen_i,sys_waddr_i} ;
      axi_awwr_pt             <= axi_awwr_pt + 4'h1                     ;
   end
end

always @ (posedge axi_clk_i)
begin
   if (!axi_rstn_i) begin
      axi_awrd_pt   <= 'h0 ;
      axi_awburst_o <= 'h0 ;
   end
   else begin
      if (axi_awpop) begin
         {axi_awburst_o[0],axi_awlen_o,axi_awaddr_o} <= axi_awfifo[axi_awrd_pt] ;
         axi_awrd_pt                                 <= axi_awrd_pt + 4'h1      ;
      end
      else if (axi_awready_i && axi_awvalid_o) begin
         {axi_awburst_o[0],axi_awlen_o,axi_awaddr_o} <= {1 + 4 + AW{1'h0}} ;
      end
   end
end

always @(posedge axi_clk_i)
begin
    if (!axi_rstn_i) begin
       axi_awvalid_o  <= 1'b0 ;
       axi_awfill_lvl <= 4'h0 ;
       awdata_in_reg  <= 1'b0 ;
    end
    else begin
      if (axi_awpop)
         axi_awvalid_o <= 1'b1 ;
      else if (axi_awready_i && axi_awvalid_o)
         axi_awvalid_o <= 1'b0 ;

      if (axi_awpush && !axi_awpop)
         axi_awfill_lvl <= axi_awfill_lvl + 4'h1 ;
      else if (!axi_awpush && axi_awpop)
         axi_awfill_lvl <= axi_awfill_lvl - 4'h1 ;

      if (axi_awpop)
         awdata_in_reg <= 1'b1 ;
      else if (axi_awready_i && axi_awvalid_o)
         awdata_in_reg <= 1'b0 ;

    end
end





//---------------------------------------------------------------------------------
//
// Write data channel

assign axi_wid_o  = ID ;

reg [    4-1: 0] axi_wwr_pt      ;
reg [    4-1: 0] axi_wrd_pt      ;
reg [    4-1: 0] axi_wfill_lvl   ;
reg [SW + DW: 0] axi_wfifo[15:0] ;  //synthesis attribute ram_style of axi_wfifo is "distributed";
                                    // stores last data, data select and data signals
reg              wdata_in_reg    ;

wire axi_wpop = (!wdata_in_reg || axi_wready_i) && |axi_wfill_lvl  ;

assign axi_wpush = sys_wvalid_i && sys_wrdy_o                          ;
assign axi_wlast = ((!sys_wlen_i && sys_wvalid_i) || (wr_cnt == 4'h1)) ;

always @ (posedge axi_clk_i)
begin
   if (!axi_rstn_i) begin
      axi_wwr_pt <= 4'h0 ;
      axi_wrd_pt <= 4'h0 ;
   end
   else begin
      if (axi_wpush) begin
         axi_wfifo[axi_wwr_pt] <= {axi_wlast,sys_wsel_i,sys_wdata_i} ;
         axi_wwr_pt            <= axi_wwr_pt + 4'h1                  ;
      end

      if (axi_wpop) begin
         {axi_wlast_o,axi_wstrb_o,axi_wdata_o} <= axi_wfifo[axi_wrd_pt] ;
         axi_wrd_pt                            <= axi_wrd_pt + 4'h1     ;
      end
      else if (axi_wready_i && axi_wvalid_o) begin
          {axi_wlast_o,axi_wstrb_o,axi_wdata_o} <= {1 + SW + DW{1'h0}} ;
      end

   end
end


always @(posedge axi_clk_i)
begin
   if (!axi_rstn_i) begin
      axi_wvalid_o  <= 1'b0 ;
      sys_wrdy_o    <= 1'b0 ;
      axi_wfill_lvl <= 4'h0 ;
   end
   else begin
      if (axi_wpop)
         axi_wvalid_o <= 1'b1 ;
      else if (axi_wready_i && axi_wvalid_o)
         axi_wvalid_o <= 1'b0 ;

      if (axi_wpush && !axi_wpop)
         axi_wfill_lvl <= axi_wfill_lvl + 4'h1 ;
      else if (!axi_wpush && axi_wpop)
         axi_wfill_lvl <= axi_wfill_lvl - 4'h1 ;

      // stop pushing, when either data or address FIFO is almost full
      sys_wrdy_o <= ~&axi_wfill_lvl[3:1] && ~&axi_awfill_lvl[3:1]; 
   end
end

always @(posedge axi_clk_i)
begin
   if (!axi_rstn_i) begin
      wdata_in_reg <= 'h0 ;
      wr_cnt       <= 'h0 ;
   end
   else begin
      if (axi_wpop)
         wdata_in_reg <= 1'b1 ;
      else if (axi_wready_i && axi_wvalid_o)
         wdata_in_reg <= 1'b0 ;

      if (sys_wvalid_i && sys_wrdy_o && !wr_cnt)
         wr_cnt <= sys_wlen_i ;
      else if (axi_wpush && wr_cnt)
         wr_cnt <= wr_cnt - 4'h1 ;

   end
end







//---------------------------------------------------------------------------------
//
// Write response channel

assign axi_bready_o = 'h1 ;

always @(posedge axi_clk_i)
begin
   if (!axi_rstn_i)
      sys_werr_o <= 1'b0 ;
   else
      sys_werr_o <= axi_bvalid_i && (axi_bresp_i == 2'h2) ;
end





//---------------------------------------------------------------------------------
//
// Read address channel

assign axi_arid_o    = ID ;
assign axi_arsize_o  = {2'b01,(DW==64)} ; // 4 or 8 byte transfer 
assign axi_arlock_o  = 2'h0   ; // normal
assign axi_arcache_o = 4'h0   ; // non-cacheable
assign axi_arprot_o  = 3'b010 ; // data, non-secured, unprivileged

reg [4-1: 0] rd_cnt  ; // counts data received by system port
reg          nxt_burst_rdy;

// system bus provides next address only
// after the current read burst has finished
always @(posedge axi_clk_i)
begin
   if (!axi_rstn_i) begin
      nxt_burst_rdy <= 1'b1 ;
      rd_cnt        <= 4'h0 ;
   end
   else begin
      if (sys_rvalid_i)
         nxt_burst_rdy <= 1'b0 ;
      else if (!rd_cnt && sys_rrdy_o && sys_rvalid_i)
         nxt_burst_rdy <= 1'b1 ;

      if (sys_rvalid_i && nxt_burst_rdy)
         rd_cnt <= sys_rlen_i ;
      else if (sys_rvalid_i && sys_rrdy_o && rd_cnt)
         rd_cnt <= rd_cnt - 4'h1 ;
   end
end


always @(posedge axi_clk_i)
begin
   if (!axi_rstn_i) begin
      axi_arvalid_o  <= 'h0 ;
      axi_arburst_o  <= 'h0 ;
   end
   else begin
      if (sys_rvalid_i && nxt_burst_rdy) begin
         axi_arvalid_o  <= 'h1           ;
         axi_arlen_o    <= sys_rlen_i    ;
         axi_araddr_o   <= sys_raddr_i   ;
         axi_arburst_o  <= !sys_rfixed_i ;
      end
      else if (axi_arready_i) begin
         axi_arvalid_o  <= 'h0 ;
      end
   end
end





//---------------------------------------------------------------------------------
//
// Read data channel

reg [ 4-1: 0] axi_rwr_pt      ,
              axi_rrd_pt      ;
reg [ 4-1: 0] axi_rfill_lvl   ;
reg [DW-1: 0] axi_rfifo[15:0] ;  //synthesis attribute ram_style of axi_rfifo is "distributed";
reg           rdata_in_reg    ;

wire axi_rpush = axi_rvalid_i && axi_rready_o ;
wire axi_rpop  = (!rdata_in_reg || sys_rvalid_i) && |axi_rfill_lvl ;

always @(posedge axi_clk_i)
begin
   if (!axi_rstn_i) begin
      axi_rwr_pt <= 4'h0 ;
      axi_rrd_pt <= 4'h0 ;
   end
   else begin
      if (axi_rpush) begin
         axi_rfifo[axi_rwr_pt] <= axi_rdata_i         ;
         axi_rwr_pt            <= axi_rwr_pt + 4'h1   ;
      end
      if (axi_rpop) begin
         sys_rdata_o <= axi_rfifo[axi_rrd_pt]  ;
         axi_rrd_pt <= axi_rrd_pt + 4'h1       ;
      end
   end
end


always @(posedge axi_clk_i)
begin
   if (!axi_rstn_i) begin
      sys_rrdy_o <= 1'b0 ;
      sys_rerr_o <= 1'b0 ;
   end
   else begin
      if (axi_rpop)
         sys_rrdy_o <= 1'b1 ;
      else if (sys_rrdy_o && sys_rvalid_i)
         sys_rrdy_o <= 1'b0 ;

      sys_rerr_o <= axi_rvalid_i && (axi_rresp_i == 2'h2) ;
   end
end


always @(posedge axi_clk_i)
begin
   if (!axi_rstn_i) begin
      axi_rfill_lvl <= 4'h0 ;
      axi_rready_o  <= 1'h1 ;
   end
   else begin
      if (axi_rpush && !axi_rpop)
         axi_rfill_lvl <= axi_rfill_lvl + 4'h1 ;
      else if (!axi_rpush && axi_rpop)
         axi_rfill_lvl <= axi_rfill_lvl - 4'h1 ;

      axi_rready_o <= ~&axi_rfill_lvl[3:1] ;
   end
end


always @(posedge axi_clk_i)
begin
   if (!axi_rstn_i) begin
      rdata_in_reg <= 'h0 ;
   end
   else begin
      if (axi_rpop) begin
         rdata_in_reg <= 1'b1 ;
      end
      else if (sys_rrdy_o && sys_rvalid_i) begin
         rdata_in_reg <= 1'b0 ;
      end
   end
end










endmodule // amba_axi_master 
