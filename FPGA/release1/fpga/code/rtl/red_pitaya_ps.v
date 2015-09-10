/**
 * $Id: red_pitaya_ps.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya Processing System (PS) wrapper. Including simple AXI slave.
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
 * Wrapper of block design.  
 *
 *
 *
 *                   /-------\
 *   PS CLK -------> |       | <---------------------> SPI master & slave
 *   PS RST -------> |  PS   |
 *                   |       | ------------+---------> FCLK & reset 
 *                   |       |             |
 *   PS DDR <------> |  ARM  |   AXI   /-------\
 *   PS MIO <------> |       | <-----> |  AXI  | <---> system bus
 *                   \-------/         | SLAVE |
 *                                     \-------/
 *
 *
 *
 * This module wrappes the PS module (BD design from Vivado or EDK from PlanAhead).
 * There is also a simple AXI slave included which serves as a master for the custom
 * system bus. With this simpler bus it is more easy for newbies to develop 
 * their own module communication with the ARM.
 * 
 */

module red_pitaya_ps (
  // PS peripherals
  inout   [ 54-1: 0] FIXED_IO_mio       ,
  inout              FIXED_IO_ps_clk    ,
  inout              FIXED_IO_ps_porb   ,
  inout              FIXED_IO_ps_srstb  ,
  inout              FIXED_IO_ddr_vrn   ,
  inout              FIXED_IO_ddr_vrp   ,
  // DDR
  inout   [ 15-1: 0] DDR_addr           ,
  inout   [  3-1: 0] DDR_ba             ,
  inout              DDR_cas_n          ,
  inout              DDR_ck_n           ,
  inout              DDR_ck_p           ,
  inout              DDR_cke            ,
  inout              DDR_cs_n           ,
  inout   [  4-1: 0] DDR_dm             ,
  inout   [ 32-1: 0] DDR_dq             ,
  inout   [  4-1: 0] DDR_dqs_n          ,
  inout   [  4-1: 0] DDR_dqs_p          ,
  inout              DDR_odt            ,
  inout              DDR_ras_n          ,
  inout              DDR_reset_n        ,
  inout              DDR_we_n           ,

  output  [  4-1: 0] fclk_clk_o         ,
  output  [  4-1: 0] fclk_rstn_o        ,
  // system read/write channel
  output             sys_clk_o          ,  // system clock
  output             sys_rstn_o         ,  // system reset - active low
  output  [ 32-1: 0] sys_addr_o         ,  // system read/write address
  output  [ 32-1: 0] sys_wdata_o        ,  // system write data
  output  [  4-1: 0] sys_sel_o          ,  // system write byte select
  output             sys_wen_o          ,  // system write enable
  output             sys_ren_o          ,  // system read enable
  input   [ 32-1: 0] sys_rdata_i        ,  // system read data
  input              sys_err_i          ,  // system error indicator
  input              sys_ack_i          ,  // system acknowledge signal
  // 2 AXI masters
  input  [2* 1-1: 0] axi_clk_i          ,  // global clock
  input  [2* 1-1: 0] axi_rstn_i         ,  // global reset
  input  [2*32-1: 0] axi_waddr_i        ,  // system write address
  input  [2*64-1: 0] axi_wdata_i        ,  // system write data
  input  [2* 8-1: 0] axi_wsel_i         ,  // system write byte select
  input  [2* 1-1: 0] axi_wvalid_i       ,  // system write data valid
  input  [2* 4-1: 0] axi_wlen_i         ,  // system write burst length
  input  [2* 1-1: 0] axi_wfixed_i       ,  // system write burst type (fixed / incremental)
  output [2* 1-1: 0] axi_werr_o         ,  // system write error
  output [2* 1-1: 0] axi_wrdy_o            // system write ready
);

//------------------------------------------------------------------------------
// AXI masters

wire            hp_saxi_clk_i     [1:0] ;
wire            hp_saxi_rstn_i    [1:0] ;

wire            hp_saxi_arready   [1:0] ;
wire            hp_saxi_awready   [1:0] ;
wire            hp_saxi_bvalid    [1:0] ;
wire            hp_saxi_rlast     [1:0] ;
wire            hp_saxi_rvalid    [1:0] ;
wire            hp_saxi_wready    [1:0] ;
wire [  2-1: 0] hp_saxi_bresp     [1:0] ;
wire [  2-1: 0] hp_saxi_rresp     [1:0] ;
wire [  6-1: 0] hp_saxi_bid       [1:0] ;
wire [  6-1: 0] hp_saxi_rid       [1:0] ;
wire [ 64-1: 0] hp_saxi_rdata     [1:0] ;
wire            hp_saxi_aclk      [1:0] ;
wire            hp_saxi_arvalid   [1:0] ;
wire            hp_saxi_awvalid   [1:0] ;
wire            hp_saxi_bready    [1:0] ;
wire            hp_saxi_rready    [1:0] ;
wire            hp_saxi_wlast     [1:0] ;
wire            hp_saxi_wvalid    [1:0] ;
wire [  2-1: 0] hp_saxi_arburst   [1:0] ;
wire [  2-1: 0] hp_saxi_arlock    [1:0] ;
wire [  3-1: 0] hp_saxi_arsize    [1:0] ;
wire [  2-1: 0] hp_saxi_awburst   [1:0] ;
wire [  2-1: 0] hp_saxi_awlock    [1:0] ;
wire [  3-1: 0] hp_saxi_awsize    [1:0] ;
wire [  3-1: 0] hp_saxi_arprot    [1:0] ;
wire [  3-1: 0] hp_saxi_awprot    [1:0] ;
wire [ 32-1: 0] hp_saxi_araddr    [1:0] ;
wire [ 32-1: 0] hp_saxi_awaddr    [1:0] ;
wire [  4-1: 0] hp_saxi_arcache   [1:0] ;
wire [  4-1: 0] hp_saxi_arlen     [1:0] ;
wire [  4-1: 0] hp_saxi_arqos     [1:0] ;
wire [  4-1: 0] hp_saxi_awcache   [1:0] ;
wire [  4-1: 0] hp_saxi_awlen     [1:0] ;
wire [  4-1: 0] hp_saxi_awqos     [1:0] ;
wire [  6-1: 0] hp_saxi_arid      [1:0] ;
wire [  6-1: 0] hp_saxi_awid      [1:0] ;
wire [  6-1: 0] hp_saxi_wid       [1:0] ;
wire [ 64-1: 0] hp_saxi_wdata     [1:0] ;
wire [  8-1: 0] hp_saxi_wstrb     [1:0] ;

genvar   i;
generate
    for (i=0; i<2; i=i+1) begin : axi_master_gen
axi_master #(
  .DW   (  64    ), // data width (8,16,...,1024)
  .AW   (  32    ), // address width
  .ID   (   i    ), // master ID
  .IW   (   6    ), // master ID width
  .LW   (   4    )  // length width
) axi_master (
   // global signals
  .axi_clk_i      (hp_saxi_clk_i  [i] ), // global clock
  .axi_rstn_i     (hp_saxi_rstn_i [i] ), // global reset
   // axi write address channel
  .axi_awid_o     (hp_saxi_awid   [i] ), // write address ID
  .axi_awaddr_o   (hp_saxi_awaddr [i] ), // write address
  .axi_awlen_o    (hp_saxi_awlen  [i] ), // write burst length
  .axi_awsize_o   (hp_saxi_awsize [i] ), // write burst size
  .axi_awburst_o  (hp_saxi_awburst[i] ), // write burst type
  .axi_awlock_o   (hp_saxi_awlock [i] ), // write lock type
  .axi_awcache_o  (hp_saxi_awcache[i] ), // write cache type
  .axi_awprot_o   (hp_saxi_awprot [i] ), // write protection type
  .axi_awvalid_o  (hp_saxi_awvalid[i] ), // write address valid
  .axi_awready_i  (hp_saxi_awready[i] ), // write ready
   // axi write data channel
  .axi_wid_o      (hp_saxi_wid    [i] ), // write data ID
  .axi_wdata_o    (hp_saxi_wdata  [i] ), // write data
  .axi_wstrb_o    (hp_saxi_wstrb  [i] ), // write strobes
  .axi_wlast_o    (hp_saxi_wlast  [i] ), // write last
  .axi_wvalid_o   (hp_saxi_wvalid [i] ), // write valid
  .axi_wready_i   (hp_saxi_wready [i] ), // write ready
   // axi write response channel
  .axi_bid_i      (hp_saxi_bid    [i] ), // write response ID
  .axi_bresp_i    (hp_saxi_bresp  [i] ), // write response
  .axi_bvalid_i   (hp_saxi_bvalid [i] ), // write response valid
  .axi_bready_o   (hp_saxi_bready [i] ), // write response ready
   // axi read address channel
  .axi_arid_o     (hp_saxi_arid   [i] ), // read address ID
  .axi_araddr_o   (hp_saxi_araddr [i] ), // read address
  .axi_arlen_o    (hp_saxi_arlen  [i] ), // read burst length
  .axi_arsize_o   (hp_saxi_arsize [i] ), // read burst size
  .axi_arburst_o  (hp_saxi_arburst[i] ), // read burst type
  .axi_arlock_o   (hp_saxi_arlock [i] ), // read lock type
  .axi_arcache_o  (hp_saxi_arcache[i] ), // read cache type
  .axi_arprot_o   (hp_saxi_arprot [i] ), // read protection type
  .axi_arvalid_o  (hp_saxi_arvalid[i] ), // read address valid
  .axi_arready_i  (hp_saxi_arready[i] ), // read address ready
   // axi read data channel
  .axi_rid_i      (hp_saxi_rid    [i] ), // read response ID
  .axi_rdata_i    (hp_saxi_rdata  [i] ), // read data
  .axi_rresp_i    (hp_saxi_rresp  [i] ), // read response
  .axi_rlast_i    (hp_saxi_rlast  [i] ), // read last
  .axi_rvalid_i   (hp_saxi_rvalid [i] ), // read response valid
  .axi_rready_o   (hp_saxi_rready [i] ), // read response ready
   // system write channel
  .sys_waddr_i    (axi_waddr_i    [(i+1)*32-1: i*32] ), // system write address
  .sys_wdata_i    (axi_wdata_i    [(i+1)*64-1: i*64] ), // system write data
  .sys_wsel_i     (axi_wsel_i     [(i+1)* 8-1: i* 8] ), // system write byte select
  .sys_wvalid_i   (axi_wvalid_i                  [i] ), // system write data valid
  .sys_wlen_i     (axi_wlen_i     [(i+1)* 4-1: i* 4] ), // system write burst length
  .sys_wfixed_i   (axi_wfixed_i                  [i] ), // system write burst type (fixed / incremental)
  .sys_werr_o     (axi_werr_o                    [i] ), // system write error
  .sys_wrdy_o     (axi_wrdy_o                    [i] ), // system write ready
   // system read channel
  .sys_raddr_i    (32'h0              ), // system read address
  .sys_rvalid_i   ( 1'b0              ), // system read address valid
  .sys_rsel_i     ( 8'h0              ), // system read byte select
  .sys_rlen_i     ( 4'h0              ), // system read burst length
  .sys_rfixed_i   ( 1'b0              ), // system read burst type (fixed / incremental)
  .sys_rdata_o    (                   ), // system read data
  .sys_rrdy_o     (                   ), // system read data is ready
  .sys_rerr_o     (                   )  // system read error
);
    end
endgenerate

assign hp_saxi_arqos [0] = 4'h0 ;
assign hp_saxi_awqos [0] = 4'h0 ;
assign hp_saxi_clk_i [0] = axi_clk_i    [0] ;
assign hp_saxi_rstn_i[0] = axi_rstn_i   [0] ;
assign hp_saxi_aclk  [0] = hp_saxi_clk_i[0] ;

assign hp_saxi_arqos [1] = 4'h0 ;
assign hp_saxi_awqos [1] = 4'h0 ;
assign hp_saxi_clk_i [1] = axi_clk_i    [1] ;
assign hp_saxi_rstn_i[1] = axi_rstn_i   [1] ;
assign hp_saxi_aclk  [1] = hp_saxi_clk_i[1] ;

//------------------------------------------------------------------------------
// AXI SLAVE

wire [  4-1: 0] fclk_clk             ;
wire [  4-1: 0] fclk_rstn            ;

wire            gp_maxi_arvalid[0:0] ;
wire            gp_maxi_awvalid[0:0] ;
wire            gp_maxi_bready [0:0] ;
wire            gp_maxi_rready [0:0] ;
wire            gp_maxi_wlast  [0:0] ;
wire            gp_maxi_wvalid [0:0] ;
wire [ 12-1: 0] gp_maxi_arid   [0:0] ;
wire [ 12-1: 0] gp_maxi_awid   [0:0] ;
wire [ 12-1: 0] gp_maxi_wid    [0:0] ;
wire [  2-1: 0] gp_maxi_arburst[0:0] ;
wire [  2-1: 0] gp_maxi_arlock [0:0] ;
wire [  3-1: 0] gp_maxi_arsize [0:0] ;
wire [  2-1: 0] gp_maxi_awburst[0:0] ;
wire [  2-1: 0] gp_maxi_awlock [0:0] ;
wire [  3-1: 0] gp_maxi_awsize [0:0] ;
wire [  3-1: 0] gp_maxi_arprot [0:0] ;
wire [  3-1: 0] gp_maxi_awprot [0:0] ;
wire [ 32-1: 0] gp_maxi_araddr [0:0] ;
wire [ 32-1: 0] gp_maxi_awaddr [0:0] ;
wire [ 32-1: 0] gp_maxi_wdata  [0:0] ;
wire [  4-1: 0] gp_maxi_arcache[0:0] ;
wire [  4-1: 0] gp_maxi_arlen  [0:0] ;
wire [  4-1: 0] gp_maxi_arqos  [0:0] ;
wire [  4-1: 0] gp_maxi_awcache[0:0] ;
wire [  4-1: 0] gp_maxi_awlen  [0:0] ;
wire [  4-1: 0] gp_maxi_awqos  [0:0] ;
wire [  4-1: 0] gp_maxi_wstrb  [0:0] ;
wire            gp_maxi_aclk   [0:0] ;
wire            gp_maxi_arready[0:0] ;
wire            gp_maxi_awready[0:0] ;
wire            gp_maxi_bvalid [0:0] ;
wire            gp_maxi_rlast  [0:0] ;
wire            gp_maxi_rvalid [0:0] ;
wire            gp_maxi_wready [0:0] ;
wire [ 12-1: 0] gp_maxi_bid    [0:0] ;
wire [ 12-1: 0] gp_maxi_rid    [0:0] ;
wire [  2-1: 0] gp_maxi_bresp  [0:0] ;
wire [  2-1: 0] gp_maxi_rresp  [0:0] ;
wire [ 32-1: 0] gp_maxi_rdata  [0:0] ;
reg             gp_maxi_arstn  [0:0] ;

axi_slave #(
  .AXI_DW     (  32     ), // data width (8,16,...,1024)
  .AXI_AW     (  32     ), // address width
  .AXI_IW     (  12     )  // ID width
) axi_slave_gp0 (
  // global signals
  .axi_clk_i        (  gp_maxi_aclk   [0] ),  // global clock
  .axi_rstn_i       (  gp_maxi_arstn  [0] ),  // global reset
  // axi write address channel
  .axi_awid_i       (  gp_maxi_awid   [0] ),  // write address ID
  .axi_awaddr_i     (  gp_maxi_awaddr [0] ),  // write address
  .axi_awlen_i      (  gp_maxi_awlen  [0] ),  // write burst length
  .axi_awsize_i     (  gp_maxi_awsize [0] ),  // write burst size
  .axi_awburst_i    (  gp_maxi_awburst[0] ),  // write burst type
  .axi_awlock_i     (  gp_maxi_awlock [0] ),  // write lock type
  .axi_awcache_i    (  gp_maxi_awcache[0] ),  // write cache type
  .axi_awprot_i     (  gp_maxi_awprot [0] ),  // write protection type
  .axi_awvalid_i    (  gp_maxi_awvalid[0] ),  // write address valid
  .axi_awready_o    (  gp_maxi_awready[0] ),  // write ready
  // axi write data channel
  .axi_wid_i        (  gp_maxi_wid    [0] ),  // write data ID
  .axi_wdata_i      (  gp_maxi_wdata  [0] ),  // write data
  .axi_wstrb_i      (  gp_maxi_wstrb  [0] ),  // write strobes
  .axi_wlast_i      (  gp_maxi_wlast  [0] ),  // write last
  .axi_wvalid_i     (  gp_maxi_wvalid [0] ),  // write valid
  .axi_wready_o     (  gp_maxi_wready [0] ),  // write ready
  // axi write response channel
  .axi_bid_o        (  gp_maxi_bid    [0] ),  // write response ID
  .axi_bresp_o      (  gp_maxi_bresp  [0] ),  // write response
  .axi_bvalid_o     (  gp_maxi_bvalid [0] ),  // write response valid
  .axi_bready_i     (  gp_maxi_bready [0] ),  // write response ready
  // axi read address channel
  .axi_arid_i       (  gp_maxi_arid   [0] ),  // read address ID
  .axi_araddr_i     (  gp_maxi_araddr [0] ),  // read address
  .axi_arlen_i      (  gp_maxi_arlen  [0] ),  // read burst length
  .axi_arsize_i     (  gp_maxi_arsize [0] ),  // read burst size
  .axi_arburst_i    (  gp_maxi_arburst[0] ),  // read burst type
  .axi_arlock_i     (  gp_maxi_arlock [0] ),  // read lock type
  .axi_arcache_i    (  gp_maxi_arcache[0] ),  // read cache type
  .axi_arprot_i     (  gp_maxi_arprot [0] ),  // read protection type
  .axi_arvalid_i    (  gp_maxi_arvalid[0] ),  // read address valid
  .axi_arready_o    (  gp_maxi_arready[0] ),  // read address ready
  // axi read data channel
  .axi_rid_o        (  gp_maxi_rid    [0] ),  // read response ID
  .axi_rdata_o      (  gp_maxi_rdata  [0] ),  // read data
  .axi_rresp_o      (  gp_maxi_rresp  [0] ),  // read response
  .axi_rlast_o      (  gp_maxi_rlast  [0] ),  // read last
  .axi_rvalid_o     (  gp_maxi_rvalid [0] ),  // read response valid
  .axi_rready_i     (  gp_maxi_rready [0] ),  // read response ready
  // system read/write channel
  .sys_addr_o       (  sys_addr_o         ),  // system read/write address
  .sys_wdata_o      (  sys_wdata_o        ),  // system write data
  .sys_sel_o        (  sys_sel_o          ),  // system write byte select
  .sys_wen_o        (  sys_wen_o          ),  // system write enable
  .sys_ren_o        (  sys_ren_o          ),  // system read enable
  .sys_rdata_i      (  sys_rdata_i        ),  // system read data
  .sys_err_i        (  sys_err_i          ),  // system error indicator
  .sys_ack_i        (  sys_ack_i          )   // system acknowledge signal
);

assign sys_clk_o  = gp_maxi_aclk [0] ;
assign sys_rstn_o = gp_maxi_arstn[0] ;

assign gp_maxi_aclk[0] =  axi_clk_i[0] ;

always @(posedge axi_clk_i[0])
gp_maxi_arstn[0] <= fclk_rstn[0];



//------------------------------------------------------------------------------
// PS STUB

assign fclk_rstn_o = fclk_rstn;

BUFG i_fclk0_buf  (.O(fclk_clk_o[0]), .I(fclk_clk[0]));
BUFG i_fclk1_buf  (.O(fclk_clk_o[1]), .I(fclk_clk[1]));
BUFG i_fclk2_buf  (.O(fclk_clk_o[2]), .I(fclk_clk[2]));
BUFG i_fclk3_buf  (.O(fclk_clk_o[3]), .I(fclk_clk[3]));

system_wrapper system_i (
  // MIO
  .FIXED_IO_mio      (FIXED_IO_mio       ),
  .FIXED_IO_ps_clk   (FIXED_IO_ps_clk    ),
  .FIXED_IO_ps_porb  (FIXED_IO_ps_porb   ),
  .FIXED_IO_ps_srstb (FIXED_IO_ps_srstb  ),
  .FIXED_IO_ddr_vrn  (FIXED_IO_ddr_vrn   ),
  .FIXED_IO_ddr_vrp  (FIXED_IO_ddr_vrp   ),
  // DDR
  .DDR_addr          (DDR_addr           ),
  .DDR_ba            (DDR_ba             ),
  .DDR_cas_n         (DDR_cas_n          ),
  .DDR_ck_n          (DDR_ck_n           ),
  .DDR_ck_p          (DDR_ck_p           ),
  .DDR_cke           (DDR_cke            ),
  .DDR_cs_n          (DDR_cs_n           ),
  .DDR_dm            (DDR_dm             ),
  .DDR_dq            (DDR_dq             ),
  .DDR_dqs_n         (DDR_dqs_n          ),
  .DDR_dqs_p         (DDR_dqs_p          ),
  .DDR_odt           (DDR_odt            ),
  .DDR_ras_n         (DDR_ras_n          ),
  .DDR_reset_n       (DDR_reset_n        ),
  .DDR_we_n          (DDR_we_n           ),
  // FCLKs
  .FCLK_CLK0         (fclk_clk[0]        ),
  .FCLK_CLK1         (fclk_clk[1]        ),
  .FCLK_CLK2         (fclk_clk[2]        ),
  .FCLK_CLK3         (fclk_clk[3]        ),
  .FCLK_RESET0_N     (fclk_rstn[0]       ),
  .FCLK_RESET1_N     (fclk_rstn[1]       ),
  .FCLK_RESET2_N     (fclk_rstn[2]       ),
  .FCLK_RESET3_N     (fclk_rstn[3]       ),
  // GP0
  .M_AXI_GP0_ACLK    (axi_clk_i      [0:0] ),
  .M_AXI_GP0_arvalid (gp_maxi_arvalid[0] ),  // out
  .M_AXI_GP0_awvalid (gp_maxi_awvalid[0] ),  // out
  .M_AXI_GP0_bready  (gp_maxi_bready [0] ),  // out
  .M_AXI_GP0_rready  (gp_maxi_rready [0] ),  // out
  .M_AXI_GP0_wlast   (gp_maxi_wlast  [0] ),  // out
  .M_AXI_GP0_wvalid  (gp_maxi_wvalid [0] ),  // out
  .M_AXI_GP0_arid    (gp_maxi_arid   [0] ),  // out 12
  .M_AXI_GP0_awid    (gp_maxi_awid   [0] ),  // out 12
  .M_AXI_GP0_wid     (gp_maxi_wid    [0] ),  // out 12
  .M_AXI_GP0_arburst (gp_maxi_arburst[0] ),  // out 2
  .M_AXI_GP0_arlock  (gp_maxi_arlock [0] ),  // out 2
  .M_AXI_GP0_arsize  (gp_maxi_arsize [0] ),  // out 3
  .M_AXI_GP0_awburst (gp_maxi_awburst[0] ),  // out 2
  .M_AXI_GP0_awlock  (gp_maxi_awlock [0] ),  // out 2
  .M_AXI_GP0_awsize  (gp_maxi_awsize [0] ),  // out 3
  .M_AXI_GP0_arprot  (gp_maxi_arprot [0] ),  // out 3
  .M_AXI_GP0_awprot  (gp_maxi_awprot [0] ),  // out 3
  .M_AXI_GP0_araddr  (gp_maxi_araddr [0] ),  // out 32
  .M_AXI_GP0_awaddr  (gp_maxi_awaddr [0] ),  // out 32
  .M_AXI_GP0_wdata   (gp_maxi_wdata  [0] ),  // out 32
  .M_AXI_GP0_arcache (gp_maxi_arcache[0] ),  // out 4
  .M_AXI_GP0_arlen   (gp_maxi_arlen  [0] ),  // out 4
  .M_AXI_GP0_arqos   (gp_maxi_arqos  [0] ),  // out 4
  .M_AXI_GP0_awcache (gp_maxi_awcache[0] ),  // out 4
  .M_AXI_GP0_awlen   (gp_maxi_awlen  [0] ),  // out 4
  .M_AXI_GP0_awqos   (gp_maxi_awqos  [0] ),  // out 4
  .M_AXI_GP0_wstrb   (gp_maxi_wstrb  [0] ),  // out 4
  .M_AXI_GP0_arready (gp_maxi_arready[0] ),  // in
  .M_AXI_GP0_awready (gp_maxi_awready[0] ),  // in
  .M_AXI_GP0_bvalid  (gp_maxi_bvalid [0] ),  // in
  .M_AXI_GP0_rlast   (gp_maxi_rlast  [0] ),  // in
  .M_AXI_GP0_rvalid  (gp_maxi_rvalid [0] ),  // in
  .M_AXI_GP0_wready  (gp_maxi_wready [0] ),  // in
  .M_AXI_GP0_bid     (gp_maxi_bid    [0] ),  // in 12
  .M_AXI_GP0_rid     (gp_maxi_rid    [0] ),  // in 12
  .M_AXI_GP0_bresp   (gp_maxi_bresp  [0] ),  // in 2
  .M_AXI_GP0_rresp   (gp_maxi_rresp  [0] ),  // in 2
  .M_AXI_GP0_rdata   (gp_maxi_rdata  [0] ),  // in 32
  // HP0                                     // HP1
  .S_AXI_HP0_arready (hp_saxi_arready[0] ),  .S_AXI_HP1_arready (hp_saxi_arready[1] ), // out
  .S_AXI_HP0_awready (hp_saxi_awready[0] ),  .S_AXI_HP1_awready (hp_saxi_awready[1] ), // out
  .S_AXI_HP0_bvalid  (hp_saxi_bvalid [0] ),  .S_AXI_HP1_bvalid  (hp_saxi_bvalid [1] ), // out
  .S_AXI_HP0_rlast   (hp_saxi_rlast  [0] ),  .S_AXI_HP1_rlast   (hp_saxi_rlast  [1] ), // out
  .S_AXI_HP0_rvalid  (hp_saxi_rvalid [0] ),  .S_AXI_HP1_rvalid  (hp_saxi_rvalid [1] ), // out
  .S_AXI_HP0_wready  (hp_saxi_wready [0] ),  .S_AXI_HP1_wready  (hp_saxi_wready [1] ), // out
  .S_AXI_HP0_bresp   (hp_saxi_bresp  [0] ),  .S_AXI_HP1_bresp   (hp_saxi_bresp  [1] ), // out 2
  .S_AXI_HP0_rresp   (hp_saxi_rresp  [0] ),  .S_AXI_HP1_rresp   (hp_saxi_rresp  [1] ), // out 2
  .S_AXI_HP0_bid     (hp_saxi_bid    [0] ),  .S_AXI_HP1_bid     (hp_saxi_bid    [1] ), // out 6
  .S_AXI_HP0_rid     (hp_saxi_rid    [0] ),  .S_AXI_HP1_rid     (hp_saxi_rid    [1] ), // out 6
  .S_AXI_HP0_rdata   (hp_saxi_rdata  [0] ),  .S_AXI_HP1_rdata   (hp_saxi_rdata  [1] ), // out 64
  .S_AXI_HP0_aclk    (hp_saxi_aclk   [0] ),  .S_AXI_HP1_aclk    (hp_saxi_aclk   [1] ), // in
  .S_AXI_HP0_arvalid (hp_saxi_arvalid[0] ),  .S_AXI_HP1_arvalid (hp_saxi_arvalid[1] ), // in
  .S_AXI_HP0_awvalid (hp_saxi_awvalid[0] ),  .S_AXI_HP1_awvalid (hp_saxi_awvalid[1] ), // in
  .S_AXI_HP0_bready  (hp_saxi_bready [0] ),  .S_AXI_HP1_bready  (hp_saxi_bready [1] ), // in
  .S_AXI_HP0_rready  (hp_saxi_rready [0] ),  .S_AXI_HP1_rready  (hp_saxi_rready [1] ), // in
  .S_AXI_HP0_wlast   (hp_saxi_wlast  [0] ),  .S_AXI_HP1_wlast   (hp_saxi_wlast  [1] ), // in
  .S_AXI_HP0_wvalid  (hp_saxi_wvalid [0] ),  .S_AXI_HP1_wvalid  (hp_saxi_wvalid [1] ), // in
  .S_AXI_HP0_arburst (hp_saxi_arburst[0] ),  .S_AXI_HP1_arburst (hp_saxi_arburst[1] ), // in 2
  .S_AXI_HP0_arlock  (hp_saxi_arlock [0] ),  .S_AXI_HP1_arlock  (hp_saxi_arlock [1] ), // in 2
  .S_AXI_HP0_arsize  (hp_saxi_arsize [0] ),  .S_AXI_HP1_arsize  (hp_saxi_arsize [1] ), // in 3
  .S_AXI_HP0_awburst (hp_saxi_awburst[0] ),  .S_AXI_HP1_awburst (hp_saxi_awburst[1] ), // in 2
  .S_AXI_HP0_awlock  (hp_saxi_awlock [0] ),  .S_AXI_HP1_awlock  (hp_saxi_awlock [1] ), // in 2
  .S_AXI_HP0_awsize  (hp_saxi_awsize [0] ),  .S_AXI_HP1_awsize  (hp_saxi_awsize [1] ), // in 3
  .S_AXI_HP0_arprot  (hp_saxi_arprot [0] ),  .S_AXI_HP1_arprot  (hp_saxi_arprot [1] ), // in 3
  .S_AXI_HP0_awprot  (hp_saxi_awprot [0] ),  .S_AXI_HP1_awprot  (hp_saxi_awprot [1] ), // in 3
  .S_AXI_HP0_araddr  (hp_saxi_araddr [0] ),  .S_AXI_HP1_araddr  (hp_saxi_araddr [1] ), // in 32
  .S_AXI_HP0_awaddr  (hp_saxi_awaddr [0] ),  .S_AXI_HP1_awaddr  (hp_saxi_awaddr [1] ), // in 32
  .S_AXI_HP0_arcache (hp_saxi_arcache[0] ),  .S_AXI_HP1_arcache (hp_saxi_arcache[1] ), // in 4
  .S_AXI_HP0_arlen   (hp_saxi_arlen  [0] ),  .S_AXI_HP1_arlen   (hp_saxi_arlen  [1] ), // in 4
  .S_AXI_HP0_arqos   (hp_saxi_arqos  [0] ),  .S_AXI_HP1_arqos   (hp_saxi_arqos  [1] ), // in 4
  .S_AXI_HP0_awcache (hp_saxi_awcache[0] ),  .S_AXI_HP1_awcache (hp_saxi_awcache[1] ), // in 4
  .S_AXI_HP0_awlen   (hp_saxi_awlen  [0] ),  .S_AXI_HP1_awlen   (hp_saxi_awlen  [1] ), // in 4
  .S_AXI_HP0_awqos   (hp_saxi_awqos  [0] ),  .S_AXI_HP1_awqos   (hp_saxi_awqos  [1] ), // in 4
  .S_AXI_HP0_arid    (hp_saxi_arid   [0] ),  .S_AXI_HP1_arid    (hp_saxi_arid   [1] ), // in 6
  .S_AXI_HP0_awid    (hp_saxi_awid   [0] ),  .S_AXI_HP1_awid    (hp_saxi_awid   [1] ), // in 6
  .S_AXI_HP0_wid     (hp_saxi_wid    [0] ),  .S_AXI_HP1_wid     (hp_saxi_wid    [1] ), // in 6
  .S_AXI_HP0_wdata   (hp_saxi_wdata  [0] ),  .S_AXI_HP1_wdata   (hp_saxi_wdata  [1] ), // in 64
  .S_AXI_HP0_wstrb   (hp_saxi_wstrb  [0] ),  .S_AXI_HP1_wstrb   (hp_saxi_wstrb  [1] )  // in 8
);

endmodule
