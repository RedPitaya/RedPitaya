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
 * Module wrappes PS module (BD design from Vivado or EDK from PlanAhead).
 * There is also included simple AXI slave which serves as master for custom
 * system bus. With this simpler bus it is more easy for newbies to develop 
 * their own module communication with ARM.
 * 
 */




module red_pitaya_ps
(
   // PS peripherals
 `ifdef TOOL_AHEAD
   inout   [ 54-1: 0] processing_system7_0_MIO,
   input              processing_system7_0_PS_SRSTB_pin,
   input              processing_system7_0_PS_CLK_pin,
   input              processing_system7_0_PS_PORB_pin,
   inout              processing_system7_0_DDR_Clk,
   inout              processing_system7_0_DDR_Clk_n,
   inout              processing_system7_0_DDR_CKE,
   inout              processing_system7_0_DDR_CS_n,
   inout              processing_system7_0_DDR_RAS_n,
   inout              processing_system7_0_DDR_CAS_n,
   output             processing_system7_0_DDR_WEB_pin,
   inout   [  3-1: 0] processing_system7_0_DDR_BankAddr,
   inout   [ 15-1: 0] processing_system7_0_DDR_Addr,
   inout              processing_system7_0_DDR_ODT,
   inout              processing_system7_0_DDR_DRSTB,
   inout   [ 32-1: 0] processing_system7_0_DDR_DQ,
   inout   [  4-1: 0] processing_system7_0_DDR_DM,
   inout   [  4-1: 0] processing_system7_0_DDR_DQS,
   inout   [  4-1: 0] processing_system7_0_DDR_DQS_n,
   inout              processing_system7_0_DDR_VRN,
   inout              processing_system7_0_DDR_VRP,
 `endif
 `ifdef TOOL_VIVADO
   inout   [ 54-1: 0] FIXED_IO_mio       ,
   inout              FIXED_IO_ps_clk    ,
   inout              FIXED_IO_ps_porb   ,
   inout              FIXED_IO_ps_srstb  ,
   inout              FIXED_IO_ddr_vrn   ,
   inout              FIXED_IO_ddr_vrp   ,
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
 `endif


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

   // SPI master
   output             spi_ss_o           ,  // select slave 0
   output             spi_ss1_o          ,  // select slave 1
   output             spi_ss2_o          ,  // select slave 2
   output             spi_sclk_o         ,  // serial clock
   output             spi_mosi_o         ,  // master out slave in
   input              spi_miso_i         ,  // master in slave out
   // SPI slave
   input              spi_ss_i           ,  // slave selected
   input              spi_sclk_i         ,  // serial clock
   input              spi_mosi_i         ,  // master out slave in
   output             spi_miso_o            // master in slave out

);




wire [  4-1: 0] fclk_clk             ;
wire [  4-1: 0] fclk_rstn            ;

wire            gp0_maxi_arvalid     ;
wire            gp0_maxi_awvalid     ;
wire            gp0_maxi_bready      ;
wire            gp0_maxi_rready      ;
wire            gp0_maxi_wlast       ;
wire            gp0_maxi_wvalid      ;
wire [ 12-1: 0] gp0_maxi_arid        ;
wire [ 12-1: 0] gp0_maxi_awid        ;
wire [ 12-1: 0] gp0_maxi_wid         ;
wire [  2-1: 0] gp0_maxi_arburst     ;
wire [  2-1: 0] gp0_maxi_arlock      ;
wire [  3-1: 0] gp0_maxi_arsize      ;
wire [  2-1: 0] gp0_maxi_awburst     ;
wire [  2-1: 0] gp0_maxi_awlock      ;
wire [  3-1: 0] gp0_maxi_awsize      ;
wire [  3-1: 0] gp0_maxi_arprot      ;
wire [  3-1: 0] gp0_maxi_awprot      ;
wire [ 32-1: 0] gp0_maxi_araddr      ;
wire [ 32-1: 0] gp0_maxi_awaddr      ;
wire [ 32-1: 0] gp0_maxi_wdata       ;
wire [  4-1: 0] gp0_maxi_arcache     ;
wire [  4-1: 0] gp0_maxi_arlen       ;
wire [  4-1: 0] gp0_maxi_arqos       ;
wire [  4-1: 0] gp0_maxi_awcache     ;
wire [  4-1: 0] gp0_maxi_awlen       ;
wire [  4-1: 0] gp0_maxi_awqos       ;
wire [  4-1: 0] gp0_maxi_wstrb       ;
wire            gp0_maxi_aclk        ;
wire            gp0_maxi_arready     ;
wire            gp0_maxi_awready     ;
wire            gp0_maxi_bvalid      ;
wire            gp0_maxi_rlast       ;
wire            gp0_maxi_rvalid      ;
wire            gp0_maxi_wready      ;
wire [ 12-1: 0] gp0_maxi_bid         ;
wire [ 12-1: 0] gp0_maxi_rid         ;
wire [  2-1: 0] gp0_maxi_bresp       ;
wire [  2-1: 0] gp0_maxi_rresp       ;
wire [ 32-1: 0] gp0_maxi_rdata       ;
wire            gp0_maxi_arstn       ;



assign fclk_rstn_o    = fclk_rstn      ;
assign gp0_maxi_aclk  = fclk_clk_o[0]  ;

BUFG i_fclk0_buf  (.O(fclk_clk_o[0]), .I(fclk_clk[0]));
BUFG i_fclk1_buf  (.O(fclk_clk_o[1]), .I(fclk_clk[1]));
BUFG i_fclk2_buf  (.O(fclk_clk_o[2]), .I(fclk_clk[2]));
BUFG i_fclk3_buf  (.O(fclk_clk_o[3]), .I(fclk_clk[3]));

//---------------------------------------------------------------------------------
//
//  USING PLANAHEAD

`ifdef TOOL_AHEAD
 system_stub system_i
 (
 // MIO
  .processing_system7_0_MIO           (  processing_system7_0_MIO             ),
  .processing_system7_0_PS_SRSTB_pin  (  processing_system7_0_PS_SRSTB_pin    ),
  .processing_system7_0_PS_CLK_pin    (  processing_system7_0_PS_CLK_pin      ),
  .processing_system7_0_PS_PORB_pin   (  processing_system7_0_PS_PORB_pin     ),
  .processing_system7_0_DDR_Clk       (  processing_system7_0_DDR_Clk         ),
  .processing_system7_0_DDR_Clk_n     (  processing_system7_0_DDR_Clk_n       ),
  .processing_system7_0_DDR_CKE       (  processing_system7_0_DDR_CKE         ),
  .processing_system7_0_DDR_CS_n      (  processing_system7_0_DDR_CS_n        ),
  .processing_system7_0_DDR_RAS_n     (  processing_system7_0_DDR_RAS_n       ),
  .processing_system7_0_DDR_CAS_n     (  processing_system7_0_DDR_CAS_n       ),
  .processing_system7_0_DDR_WEB_pin   (  processing_system7_0_DDR_WEB_pin     ),
  .processing_system7_0_DDR_BankAddr  (  processing_system7_0_DDR_BankAddr    ),
  .processing_system7_0_DDR_Addr      (  processing_system7_0_DDR_Addr        ),
  .processing_system7_0_DDR_ODT       (  processing_system7_0_DDR_ODT         ),
  .processing_system7_0_DDR_DRSTB     (  processing_system7_0_DDR_DRSTB       ),
  .processing_system7_0_DDR_DQ        (  processing_system7_0_DDR_DQ          ),
  .processing_system7_0_DDR_DM        (  processing_system7_0_DDR_DM          ),
  .processing_system7_0_DDR_DQS       (  processing_system7_0_DDR_DQS         ),
  .processing_system7_0_DDR_DQS_n     (  processing_system7_0_DDR_DQS_n       ),
  .processing_system7_0_DDR_VRN       (  processing_system7_0_DDR_VRN         ),
  .processing_system7_0_DDR_VRP       (  processing_system7_0_DDR_VRP         ),

 // FCLKs
  .processing_system7_0_FCLK_CLK0_pin          (  fclk_clk[0]                 ),  // out
  .processing_system7_0_FCLK_CLK1_pin          (  fclk_clk[1]                 ),  // out
  .processing_system7_0_FCLK_CLK2_pin          (  fclk_clk[2]                 ),  // out
  .processing_system7_0_FCLK_CLK3_pin          (  fclk_clk[3]                 ),  // out
  .processing_system7_0_FCLK_RESET0_N_pin      (  fclk_rstn[0]                ),  // out
  .processing_system7_0_FCLK_RESET1_N_pin      (  fclk_rstn[1]                ),  // out
  .processing_system7_0_FCLK_RESET2_N_pin      (  fclk_rstn[2]                ),  // out
  .processing_system7_0_FCLK_RESET3_N_pin      (  fclk_rstn[3]                ),  // out

 // GP0
  .processing_system7_0_M_AXI_GP0_ARVALID_pin  (  gp0_maxi_arvalid            ),  // out
  .processing_system7_0_M_AXI_GP0_AWVALID_pin  (  gp0_maxi_awvalid            ),  // out
  .processing_system7_0_M_AXI_GP0_BREADY_pin   (  gp0_maxi_bready             ),  // out
  .processing_system7_0_M_AXI_GP0_RREADY_pin   (  gp0_maxi_rready             ),  // out
  .processing_system7_0_M_AXI_GP0_WLAST_pin    (  gp0_maxi_wlast              ),  // out
  .processing_system7_0_M_AXI_GP0_WVALID_pin   (  gp0_maxi_wvalid             ),  // out
  .processing_system7_0_M_AXI_GP0_ARID_pin     (  gp0_maxi_arid               ),  // out 12
  .processing_system7_0_M_AXI_GP0_AWID_pin     (  gp0_maxi_awid               ),  // out 12
  .processing_system7_0_M_AXI_GP0_WID_pin      (  gp0_maxi_wid                ),  // out 12
  .processing_system7_0_M_AXI_GP0_ARBURST_pin  (  gp0_maxi_arburst            ),  // out 2
  .processing_system7_0_M_AXI_GP0_ARLOCK_pin   (  gp0_maxi_arlock             ),  // out 2
  .processing_system7_0_M_AXI_GP0_ARSIZE_pin   (  gp0_maxi_arsize             ),  // out 3
  .processing_system7_0_M_AXI_GP0_AWBURST_pin  (  gp0_maxi_awburst            ),  // out 2
  .processing_system7_0_M_AXI_GP0_AWLOCK_pin   (  gp0_maxi_awlock             ),  // out 2
  .processing_system7_0_M_AXI_GP0_AWSIZE_pin   (  gp0_maxi_awsize             ),  // out 3
  .processing_system7_0_M_AXI_GP0_ARPROT_pin   (  gp0_maxi_arprot             ),  // out 3
  .processing_system7_0_M_AXI_GP0_AWPROT_pin   (  gp0_maxi_awprot             ),  // out 3
  .processing_system7_0_M_AXI_GP0_ARADDR_pin   (  gp0_maxi_araddr             ),  // out 32
  .processing_system7_0_M_AXI_GP0_AWADDR_pin   (  gp0_maxi_awaddr             ),  // out 32
  .processing_system7_0_M_AXI_GP0_WDATA_pin    (  gp0_maxi_wdata              ),  // out 32
  .processing_system7_0_M_AXI_GP0_ARCACHE_pin  (  gp0_maxi_arcache            ),  // out 4
  .processing_system7_0_M_AXI_GP0_ARLEN_pin    (  gp0_maxi_arlen              ),  // out 4
  .processing_system7_0_M_AXI_GP0_ARQOS_pin    (  gp0_maxi_arqos              ),  // out 4
  .processing_system7_0_M_AXI_GP0_AWCACHE_pin  (  gp0_maxi_awcache            ),  // out 4
  .processing_system7_0_M_AXI_GP0_AWLEN_pin    (  gp0_maxi_awlen              ),  // out 4
  .processing_system7_0_M_AXI_GP0_AWQOS_pin    (  gp0_maxi_awqos              ),  // out 4
  .processing_system7_0_M_AXI_GP0_WSTRB_pin    (  gp0_maxi_wstrb              ),  // out 4
  .processing_system7_0_M_AXI_GP0_ACLK_pin     (  gp0_maxi_aclk               ),  // in
  .processing_system7_0_M_AXI_GP0_ARREADY_pin  (  gp0_maxi_arready            ),  // in
  .processing_system7_0_M_AXI_GP0_AWREADY_pin  (  gp0_maxi_awready            ),  // in
  .processing_system7_0_M_AXI_GP0_BVALID_pin   (  gp0_maxi_bvalid             ),  // in
  .processing_system7_0_M_AXI_GP0_RLAST_pin    (  gp0_maxi_rlast              ),  // in
  .processing_system7_0_M_AXI_GP0_RVALID_pin   (  gp0_maxi_rvalid             ),  // in
  .processing_system7_0_M_AXI_GP0_WREADY_pin   (  gp0_maxi_wready             ),  // in
  .processing_system7_0_M_AXI_GP0_BID_pin      (  gp0_maxi_bid                ),  // in 12
  .processing_system7_0_M_AXI_GP0_RID_pin      (  gp0_maxi_rid                ),  // in 12
  .processing_system7_0_M_AXI_GP0_BRESP_pin    (  gp0_maxi_bresp              ),  // in 2
  .processing_system7_0_M_AXI_GP0_RRESP_pin    (  gp0_maxi_rresp              ),  // in 2
  .processing_system7_0_M_AXI_GP0_RDATA_pin    (  gp0_maxi_rdata              ),  // in 32
  .processing_system7_0_M_AXI_GP0_ARESETN_pin  (  gp0_maxi_arstn              ),  // out

 // SPI0
  .processing_system7_0_SPI0_SS_O_pin     (  spi_ss_o        ),  // out
  .processing_system7_0_SPI0_SS_I_pin     (  spi_ss_i        ),  // in
  .processing_system7_0_SPI0_SS1_O_pin    (  spi_ss1_o       ),  // out
  .processing_system7_0_SPI0_SS2_O_pin    (  spi_ss2_o       ),  // out
  .processing_system7_0_SPI0_SCLK_I_pin   (  spi_sclk_i      ),  // in
  .processing_system7_0_SPI0_SCLK_O_pin   (  spi_sclk_o      ),  // out
  .processing_system7_0_SPI0_MOSI_I_pin   (  spi_mosi_i      ),  // in
  .processing_system7_0_SPI0_MOSI_O_pin   (  spi_mosi_o      ),  // out
  .processing_system7_0_SPI0_MISO_I_pin   (  spi_miso_i      ),  // in
  .processing_system7_0_SPI0_MISO_O_pin   (  spi_miso_o      ),  // out
  .processing_system7_0_SPI0_SS_T_pin     (                  ),  // out
  .processing_system7_0_SPI0_MOSI_T_pin   (                  ),  // out
  .processing_system7_0_SPI0_SCLK_T_pin   (                  ),  // out
  .processing_system7_0_SPI0_MISO_T_pin   (                  )   // out
 );
`endif



//---------------------------------------------------------------------------------
//
//  USING VIVADO

`ifdef TOOL_VIVADO
 system_wrapper system_i
 (
 // MIO
  .FIXED_IO_mio       (  FIXED_IO_mio                ),
  .FIXED_IO_ps_clk    (  FIXED_IO_ps_clk             ),
  .FIXED_IO_ps_porb   (  FIXED_IO_ps_porb            ),
  .FIXED_IO_ps_srstb  (  FIXED_IO_ps_srstb           ),
  .FIXED_IO_ddr_vrn   (  FIXED_IO_ddr_vrn            ),
  .FIXED_IO_ddr_vrp   (  FIXED_IO_ddr_vrp            ),
  .DDR_addr           (  DDR_addr                    ),
  .DDR_ba             (  DDR_ba                      ),
  .DDR_cas_n          (  DDR_cas_n                   ),
  .DDR_ck_n           (  DDR_ck_n                    ),
  .DDR_ck_p           (  DDR_ck_p                    ),
  .DDR_cke            (  DDR_cke                     ),
  .DDR_cs_n           (  DDR_cs_n                    ),
  .DDR_dm             (  DDR_dm                      ),
  .DDR_dq             (  DDR_dq                      ),
  .DDR_dqs_n          (  DDR_dqs_n                   ),
  .DDR_dqs_p          (  DDR_dqs_p                   ),
  .DDR_odt            (  DDR_odt                     ),
  .DDR_ras_n          (  DDR_ras_n                   ),
  .DDR_reset_n        (  DDR_reset_n                 ),
  .DDR_we_n           (  DDR_we_n                    ),

 // FCLKs
  .FCLK_CLK0          (  fclk_clk[0]                 ),  // out
  .FCLK_CLK1          (  fclk_clk[1]                 ),  // out
  .FCLK_CLK2          (  fclk_clk[2]                 ),  // out
  .FCLK_CLK3          (  fclk_clk[3]                 ),  // out
  .FCLK_RESET0_N      (  fclk_rstn[0]                ),  // out
  .FCLK_RESET1_N      (  fclk_rstn[1]                ),  // out
  .FCLK_RESET2_N      (  fclk_rstn[2]                ),  // out
  .FCLK_RESET3_N      (  fclk_rstn[3]                ),  // out

 // GP0
  .M_AXI_GP0_arvalid  (  gp0_maxi_arvalid            ),  // out
  .M_AXI_GP0_awvalid  (  gp0_maxi_awvalid            ),  // out
  .M_AXI_GP0_bready   (  gp0_maxi_bready             ),  // out
  .M_AXI_GP0_rready   (  gp0_maxi_rready             ),  // out
  .M_AXI_GP0_wlast    (  gp0_maxi_wlast              ),  // out
  .M_AXI_GP0_wvalid   (  gp0_maxi_wvalid             ),  // out
  .M_AXI_GP0_arid     (  gp0_maxi_arid               ),  // out 12
  .M_AXI_GP0_awid     (  gp0_maxi_awid               ),  // out 12
  .M_AXI_GP0_wid      (  gp0_maxi_wid                ),  // out 12
  .M_AXI_GP0_arburst  (  gp0_maxi_arburst            ),  // out 2
  .M_AXI_GP0_arlock   (  gp0_maxi_arlock             ),  // out 2
  .M_AXI_GP0_arsize   (  gp0_maxi_arsize             ),  // out 3
  .M_AXI_GP0_awburst  (  gp0_maxi_awburst            ),  // out 2
  .M_AXI_GP0_awlock   (  gp0_maxi_awlock             ),  // out 2
  .M_AXI_GP0_awsize   (  gp0_maxi_awsize             ),  // out 3
  .M_AXI_GP0_arprot   (  gp0_maxi_arprot             ),  // out 3
  .M_AXI_GP0_awprot   (  gp0_maxi_awprot             ),  // out 3
  .M_AXI_GP0_araddr   (  gp0_maxi_araddr             ),  // out 32
  .M_AXI_GP0_awaddr   (  gp0_maxi_awaddr             ),  // out 32
  .M_AXI_GP0_wdata    (  gp0_maxi_wdata              ),  // out 32
  .M_AXI_GP0_arcache  (  gp0_maxi_arcache            ),  // out 4
  .M_AXI_GP0_arlen    (  gp0_maxi_arlen              ),  // out 4
  .M_AXI_GP0_arqos    (  gp0_maxi_arqos              ),  // out 4
  .M_AXI_GP0_awcache  (  gp0_maxi_awcache            ),  // out 4
  .M_AXI_GP0_awlen    (  gp0_maxi_awlen              ),  // out 4
  .M_AXI_GP0_awqos    (  gp0_maxi_awqos              ),  // out 4
  .M_AXI_GP0_wstrb    (  gp0_maxi_wstrb              ),  // out 4
  .M_AXI_GP0_arready  (  gp0_maxi_arready            ),  // in
  .M_AXI_GP0_awready  (  gp0_maxi_awready            ),  // in
  .M_AXI_GP0_bvalid   (  gp0_maxi_bvalid             ),  // in
  .M_AXI_GP0_rlast    (  gp0_maxi_rlast              ),  // in
  .M_AXI_GP0_rvalid   (  gp0_maxi_rvalid             ),  // in
  .M_AXI_GP0_wready   (  gp0_maxi_wready             ),  // in
  .M_AXI_GP0_bid      (  gp0_maxi_bid                ),  // in 12
  .M_AXI_GP0_rid      (  gp0_maxi_rid                ),  // in 12
  .M_AXI_GP0_bresp    (  gp0_maxi_bresp              ),  // in 2
  .M_AXI_GP0_rresp    (  gp0_maxi_rresp              ),  // in 2
  .M_AXI_GP0_rdata    (  gp0_maxi_rdata              ),  // in 32

 // SPI0
  .SPI0_SS_I          (  spi_ss_i                    ),  // in
  .SPI0_SS_O          (  spi_ss_o                    ),  // out
  .SPI0_SS1_O         (  spi_ss1_o                   ),  // out
  .SPI0_SS2_O         (  spi_ss2_o                   ),  // out
  .SPI0_SCLK_I        (  spi_sclk_i                  ),  // in
  .SPI0_SCLK_O        (  spi_sclk_o                  ),  // out
  .SPI0_MOSI_I        (  spi_mosi_i                  ),  // in
  .SPI0_MOSI_O        (  spi_mosi_o                  ),  // out
  .SPI0_MISO_I        (  spi_miso_i                  ),  // in
  .SPI0_MISO_O        (  spi_miso_o                  ),  // out
  .SPI0_SS_T          (                              ),  // out
  .SPI0_SCLK_T        (                              ),  // out
  .SPI0_MOSI_T        (                              ),  // out
  .SPI0_MISO_T        (                              )   // out
 );

 assign gp0_maxi_arstn = fclk_rstn[0] ;
`endif







//---------------------------------------------------------------------------------
//
//  SIMPLE AXI SLAVE

axi_slave #(
  .AXI_DW     (  32     ), // data width (8,16,...,1024)
  .AXI_AW     (  32     ), // address width
  .AXI_IW     (  12     )  // ID width
)
i_gp0_slave
(
 // global signals
  .axi_clk_i        (  gp0_maxi_aclk           ),  // global clock
  .axi_rstn_i       (  gp0_maxi_arstn          ),  // global reset

 // axi write address channel
  .axi_awid_i       (  gp0_maxi_awid           ),  // write address ID
  .axi_awaddr_i     (  gp0_maxi_awaddr         ),  // write address
  .axi_awlen_i      (  gp0_maxi_awlen          ),  // write burst length
  .axi_awsize_i     (  gp0_maxi_awsize         ),  // write burst size
  .axi_awburst_i    (  gp0_maxi_awburst        ),  // write burst type
  .axi_awlock_i     (  gp0_maxi_awlock         ),  // write lock type
  .axi_awcache_i    (  gp0_maxi_awcache        ),  // write cache type
  .axi_awprot_i     (  gp0_maxi_awprot         ),  // write protection type
  .axi_awvalid_i    (  gp0_maxi_awvalid        ),  // write address valid
  .axi_awready_o    (  gp0_maxi_awready        ),  // write ready

 // axi write data channel
  .axi_wid_i        (  gp0_maxi_wid            ),  // write data ID
  .axi_wdata_i      (  gp0_maxi_wdata          ),  // write data
  .axi_wstrb_i      (  gp0_maxi_wstrb          ),  // write strobes
  .axi_wlast_i      (  gp0_maxi_wlast          ),  // write last
  .axi_wvalid_i     (  gp0_maxi_wvalid         ),  // write valid
  .axi_wready_o     (  gp0_maxi_wready         ),  // write ready

 // axi write response channel
  .axi_bid_o        (  gp0_maxi_bid            ),  // write response ID
  .axi_bresp_o      (  gp0_maxi_bresp          ),  // write response
  .axi_bvalid_o     (  gp0_maxi_bvalid         ),  // write response valid
  .axi_bready_i     (  gp0_maxi_bready         ),  // write response ready

 // axi read address channel
  .axi_arid_i       (  gp0_maxi_arid           ),  // read address ID
  .axi_araddr_i     (  gp0_maxi_araddr         ),  // read address
  .axi_arlen_i      (  gp0_maxi_arlen          ),  // read burst length
  .axi_arsize_i     (  gp0_maxi_arsize         ),  // read burst size
  .axi_arburst_i    (  gp0_maxi_arburst        ),  // read burst type
  .axi_arlock_i     (  gp0_maxi_arlock         ),  // read lock type
  .axi_arcache_i    (  gp0_maxi_arcache        ),  // read cache type
  .axi_arprot_i     (  gp0_maxi_arprot         ),  // read protection type
  .axi_arvalid_i    (  gp0_maxi_arvalid        ),  // read address valid
  .axi_arready_o    (  gp0_maxi_arready        ),  // read address ready
    
 // axi read data channel
  .axi_rid_o        (  gp0_maxi_rid            ),  // read response ID
  .axi_rdata_o      (  gp0_maxi_rdata          ),  // read data
  .axi_rresp_o      (  gp0_maxi_rresp          ),  // read response
  .axi_rlast_o      (  gp0_maxi_rlast          ),  // read last
  .axi_rvalid_o     (  gp0_maxi_rvalid         ),  // read response valid
  .axi_rready_i     (  gp0_maxi_rready         ),  // read response ready

 // system read/write channel
  .sys_addr_o       (  sys_addr_o              ),  // system read/write address
  .sys_wdata_o      (  sys_wdata_o             ),  // system write data
  .sys_sel_o        (  sys_sel_o               ),  // system write byte select
  .sys_wen_o        (  sys_wen_o               ),  // system write enable
  .sys_ren_o        (  sys_ren_o               ),  // system read enable
  .sys_rdata_i      (  sys_rdata_i             ),  // system read data
  .sys_err_i        (  sys_err_i               ),  // system error indicator
  .sys_ack_i        (  sys_ack_i               )   // system acknowledge signal
);





assign sys_clk_o  = gp0_maxi_aclk   ;
assign sys_rstn_o = gp0_maxi_arstn  ;


endmodule
