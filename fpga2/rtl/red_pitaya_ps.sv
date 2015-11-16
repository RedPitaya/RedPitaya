////////////////////////////////////////////////////////////////////////////////
// @brief Red Pitaya Processing System (PS) wrapper. Including simple AXI slave.
// @Author Matej Oblak
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

/**
 * GENERAL DESCRIPTION:
 *
 * Wrapper of block design.  
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
 * Module wrappes PS module (BD design from Vivado or EDK from PlanAhead).
 * There is also included simple AXI slave which serves as master for custom
 * system bus. With this simpler bus it is more easy for newbies to develop 
 * their own module communication with ARM.
 */

module red_pitaya_ps (
  // PS peripherals
  inout  logic [ 54-1:0] FIXED_IO_mio       ,
  inout  logic           FIXED_IO_ps_clk    ,
  inout  logic           FIXED_IO_ps_porb   ,
  inout  logic           FIXED_IO_ps_srstb  ,
  inout  logic           FIXED_IO_ddr_vrn   ,
  inout  logic           FIXED_IO_ddr_vrp   ,
  // DDR
  inout  logic [ 15-1:0] DDR_addr           ,
  inout  logic [  3-1:0] DDR_ba             ,
  inout  logic           DDR_cas_n          ,
  inout  logic           DDR_ck_n           ,
  inout  logic           DDR_ck_p           ,
  inout  logic           DDR_cke            ,
  inout  logic           DDR_cs_n           ,
  inout  logic [  4-1:0] DDR_dm             ,
  inout  logic [ 32-1:0] DDR_dq             ,
  inout  logic [  4-1:0] DDR_dqs_n          ,
  inout  logic [  4-1:0] DDR_dqs_p          ,
  inout  logic           DDR_odt            ,
  inout  logic           DDR_ras_n          ,
  inout  logic           DDR_reset_n        ,
  inout  logic           DDR_we_n           ,
  // system signals
  input  logic           clk                ,
  input  logic           rstn               ,
  output logic [  4-1:0] fclk_clk_o         ,
  output logic [  4-1:0] fclk_rstn_o        ,
  // XADC
  input  logic  [ 5-1:0] vinp_i             ,  // voltages p
  input  logic  [ 5-1:0] vinn_i             ,  // voltages n
  // system read/write channel
  output logic [ 32-1:0] sys_addr ,  // system read/write address
  output logic [ 32-1:0] sys_wdata,  // system write data
  output logic [  4-1:0] sys_sel  ,  // system write byte select
  output logic           sys_wen  ,  // system write enable
  output logic           sys_ren  ,  // system read enable
  input  logic [ 32-1:0] sys_rdata,  // system read data
  input  logic           sys_err  ,  // system error indicator
  input  logic           sys_ack  ,  // system acknowledge signal
  // AXI-4 stream
  input  logic [2*8-1:0] axi1_tdata , axi0_tdata ,  // stream data
  input  logic           axi1_tlast , axi0_tlast ,  // stream last
  input  logic           axi1_tvalid, axi0_tvalid,  // stream valid
  output logic           axi1_tready, axi0_tready   // stream ready
);

////////////////////////////////////////////////////////////////////////////////
// AXI SLAVE
////////////////////////////////////////////////////////////////////////////////

logic [  4-1: 0] fclk_clk             ;
logic [  4-1: 0] fclk_rstn            ;

logic            gp0_maxi_arvalid     ;
logic            gp0_maxi_awvalid     ;
logic            gp0_maxi_bready      ;
logic            gp0_maxi_rready      ;
logic            gp0_maxi_wlast       ;
logic            gp0_maxi_wvalid      ;
logic [ 12-1: 0] gp0_maxi_arid        ;
logic [ 12-1: 0] gp0_maxi_awid        ;
logic [ 12-1: 0] gp0_maxi_wid         ;
logic [  2-1: 0] gp0_maxi_arburst     ;
logic [  2-1: 0] gp0_maxi_arlock      ;
logic [  3-1: 0] gp0_maxi_arsize      ;
logic [  2-1: 0] gp0_maxi_awburst     ;
logic [  2-1: 0] gp0_maxi_awlock      ;
logic [  3-1: 0] gp0_maxi_awsize      ;
logic [  3-1: 0] gp0_maxi_arprot      ;
logic [  3-1: 0] gp0_maxi_awprot      ;
logic [ 32-1: 0] gp0_maxi_araddr      ;
logic [ 32-1: 0] gp0_maxi_awaddr      ;
logic [ 32-1: 0] gp0_maxi_wdata       ;
logic [  4-1: 0] gp0_maxi_arcache     ;
logic [  4-1: 0] gp0_maxi_arlen       ;
logic [  4-1: 0] gp0_maxi_arqos       ;
logic [  4-1: 0] gp0_maxi_awcache     ;
logic [  4-1: 0] gp0_maxi_awlen       ;
logic [  4-1: 0] gp0_maxi_awqos       ;
logic [  4-1: 0] gp0_maxi_wstrb       ;
logic            gp0_maxi_aclk        ;
logic            gp0_maxi_arready     ;
logic            gp0_maxi_awready     ;
logic            gp0_maxi_bvalid      ;
logic            gp0_maxi_rlast       ;
logic            gp0_maxi_rvalid      ;
logic            gp0_maxi_wready      ;
logic [ 12-1: 0] gp0_maxi_bid         ;
logic [ 12-1: 0] gp0_maxi_rid         ;
logic [  2-1: 0] gp0_maxi_bresp       ;
logic [  2-1: 0] gp0_maxi_rresp       ;
logic [ 32-1: 0] gp0_maxi_rdata       ;
logic            gp0_maxi_arstn       ;

axi_slave #(
  .AXI_DW     (  32     ), // data width (8,16,...,1024)
  .AXI_AW     (  32     ), // address width
  .AXI_IW     (  12     )  // ID width
) axi_slave_gp0 (
  // global signals
  .axi_clk_i        (gp0_maxi_aclk   ),  // global clock
  .axi_rstn_i       (gp0_maxi_arstn  ),  // global reset
  // axi write address channel
  .axi_awid_i       (gp0_maxi_awid   ),  // write address ID
  .axi_awaddr_i     (gp0_maxi_awaddr ),  // write address
  .axi_awlen_i      (gp0_maxi_awlen  ),  // write burst length
  .axi_awsize_i     (gp0_maxi_awsize ),  // write burst size
  .axi_awburst_i    (gp0_maxi_awburst),  // write burst type
  .axi_awlock_i     (gp0_maxi_awlock ),  // write lock type
  .axi_awcache_i    (gp0_maxi_awcache),  // write cache type
  .axi_awprot_i     (gp0_maxi_awprot ),  // write protection type
  .axi_awvalid_i    (gp0_maxi_awvalid),  // write address valid
  .axi_awready_o    (gp0_maxi_awready),  // write ready
  // axi write data channel
  .axi_wid_i        (gp0_maxi_wid    ),  // write data ID
  .axi_wdata_i      (gp0_maxi_wdata  ),  // write data
  .axi_wstrb_i      (gp0_maxi_wstrb  ),  // write strobes
  .axi_wlast_i      (gp0_maxi_wlast  ),  // write last
  .axi_wvalid_i     (gp0_maxi_wvalid ),  // write valid
  .axi_wready_o     (gp0_maxi_wready ),  // write ready
  // axi write response channel
  .axi_bid_o        (gp0_maxi_bid    ),  // write response ID
  .axi_bresp_o      (gp0_maxi_bresp  ),  // write response
  .axi_bvalid_o     (gp0_maxi_bvalid ),  // write response valid
  .axi_bready_i     (gp0_maxi_bready ),  // write response ready
  // axi read address channel
  .axi_arid_i       (gp0_maxi_arid   ),  // read address ID
  .axi_araddr_i     (gp0_maxi_araddr ),  // read address
  .axi_arlen_i      (gp0_maxi_arlen  ),  // read burst length
  .axi_arsize_i     (gp0_maxi_arsize ),  // read burst size
  .axi_arburst_i    (gp0_maxi_arburst),
  .axi_arlock_i     (gp0_maxi_arlock ),
  .axi_arcache_i    (gp0_maxi_arcache),
  .axi_arprot_i     (gp0_maxi_arprot ),
  .axi_arvalid_i    (gp0_maxi_arvalid),
  .axi_arready_o    (gp0_maxi_arready),
  // axi read data channel
  .axi_rid_o        (gp0_maxi_rid    ),
  .axi_rdata_o      (gp0_maxi_rdata  ),
  .axi_rresp_o      (gp0_maxi_rresp  ),
  .axi_rlast_o      (gp0_maxi_rlast  ),
  .axi_rvalid_o     (gp0_maxi_rvalid ),
  .axi_rready_i     (gp0_maxi_rready ),
  // system read/write channel
  .sys_addr_o       (sys_addr        ),
  .sys_wdata_o      (sys_wdata       ),
  .sys_sel_o        (sys_sel         ),
  .sys_wen_o        (sys_wen         ),
  .sys_ren_o        (sys_ren         ),
  .sys_rdata_i      (sys_rdata       ),
  .sys_err_i        (sys_err         ),
  .sys_ack_i        (sys_ack         )
);

assign gp0_maxi_aclk  = clk;
assign gp0_maxi_arstn = rstn;

////////////////////////////////////////////////////////////////////////////////
// PS STUB
////////////////////////////////////////////////////////////////////////////////

assign fclk_rstn_o = fclk_rstn;

BUFG i_fclk0_buf  (.O(fclk_clk_o[0]), .I(fclk_clk[0]));
BUFG i_fclk1_buf  (.O(fclk_clk_o[1]), .I(fclk_clk[1]));
BUFG i_fclk2_buf  (.O(fclk_clk_o[2]), .I(fclk_clk[2]));
BUFG i_fclk3_buf  (.O(fclk_clk_o[3]), .I(fclk_clk[3]));

system_wrapper system_i (
  // MIO
  .FIXED_IO_mio      (FIXED_IO_mio     ),
  .FIXED_IO_ps_clk   (FIXED_IO_ps_clk  ),
  .FIXED_IO_ps_porb  (FIXED_IO_ps_porb ),
  .FIXED_IO_ps_srstb (FIXED_IO_ps_srstb),
  .FIXED_IO_ddr_vrn  (FIXED_IO_ddr_vrn ),
  .FIXED_IO_ddr_vrp  (FIXED_IO_ddr_vrp ),
  // DDR
  .DDR_addr          (DDR_addr         ),
  .DDR_ba            (DDR_ba           ),
  .DDR_cas_n         (DDR_cas_n        ),
  .DDR_ck_n          (DDR_ck_n         ),
  .DDR_ck_p          (DDR_ck_p         ),
  .DDR_cke           (DDR_cke          ),
  .DDR_cs_n          (DDR_cs_n         ),
  .DDR_dm            (DDR_dm           ),
  .DDR_dq            (DDR_dq           ),
  .DDR_dqs_n         (DDR_dqs_n        ),
  .DDR_dqs_p         (DDR_dqs_p        ),
  .DDR_odt           (DDR_odt          ),
  .DDR_ras_n         (DDR_ras_n        ),
  .DDR_reset_n       (DDR_reset_n      ),
  .DDR_we_n          (DDR_we_n         ),
  // FCLKs
  .FCLK_CLK0         (fclk_clk[0]      ),
  .FCLK_CLK1         (fclk_clk[1]      ),
  .FCLK_CLK2         (fclk_clk[2]      ),
  .FCLK_CLK3         (fclk_clk[3]      ),
  .FCLK_RESET0_N     (fclk_rstn[0]     ),
  .FCLK_RESET1_N     (fclk_rstn[1]     ),
  .FCLK_RESET2_N     (fclk_rstn[2]     ),
  .FCLK_RESET3_N     (fclk_rstn[3]     ),
  // XADC
  .Vaux0_v_n (vinn_i[1]),  .Vaux0_v_p (vinp_i[1]),
  .Vaux1_v_n (vinn_i[2]),  .Vaux1_v_p (vinp_i[2]),
  .Vaux8_v_n (vinn_i[0]),  .Vaux8_v_p (vinp_i[0]),
  .Vaux9_v_n (vinn_i[3]),  .Vaux9_v_p (vinp_i[3]),
  .Vp_Vn_v_n (vinn_i[4]),  .Vp_Vn_v_p (vinp_i[4]),
  // GP0
  .M_AXI_GP0_ACLK    (gp0_maxi_aclk   ),  // in
  .M_AXI_GP0_arvalid (gp0_maxi_arvalid),  // out
  .M_AXI_GP0_awvalid (gp0_maxi_awvalid),  // out
  .M_AXI_GP0_bready  (gp0_maxi_bready ),  // out
  .M_AXI_GP0_rready  (gp0_maxi_rready ),  // out
  .M_AXI_GP0_wlast   (gp0_maxi_wlast  ),  // out
  .M_AXI_GP0_wvalid  (gp0_maxi_wvalid ),  // out
  .M_AXI_GP0_arid    (gp0_maxi_arid   ),  // out 12
  .M_AXI_GP0_awid    (gp0_maxi_awid   ),  // out 12
  .M_AXI_GP0_wid     (gp0_maxi_wid    ),  // out 12
  .M_AXI_GP0_arburst (gp0_maxi_arburst),  // out 2
  .M_AXI_GP0_arlock  (gp0_maxi_arlock ),  // out 2
  .M_AXI_GP0_arsize  (gp0_maxi_arsize ),  // out 3
  .M_AXI_GP0_awburst (gp0_maxi_awburst),  // out 2
  .M_AXI_GP0_awlock  (gp0_maxi_awlock ),  // out 2
  .M_AXI_GP0_awsize  (gp0_maxi_awsize ),  // out 3
  .M_AXI_GP0_arprot  (gp0_maxi_arprot ),  // out 3
  .M_AXI_GP0_awprot  (gp0_maxi_awprot ),  // out 3
  .M_AXI_GP0_araddr  (gp0_maxi_araddr ),  // out 32
  .M_AXI_GP0_awaddr  (gp0_maxi_awaddr ),  // out 32
  .M_AXI_GP0_wdata   (gp0_maxi_wdata  ),  // out 32
  .M_AXI_GP0_arcache (gp0_maxi_arcache),  // out 4
  .M_AXI_GP0_arlen   (gp0_maxi_arlen  ),  // out 4
  .M_AXI_GP0_arqos   (gp0_maxi_arqos  ),  // out 4
  .M_AXI_GP0_awcache (gp0_maxi_awcache),  // out 4
  .M_AXI_GP0_awlen   (gp0_maxi_awlen  ),  // out 4
  .M_AXI_GP0_awqos   (gp0_maxi_awqos  ),  // out 4
  .M_AXI_GP0_wstrb   (gp0_maxi_wstrb  ),  // out 4
  .M_AXI_GP0_arready (gp0_maxi_arready),  // in
  .M_AXI_GP0_awready (gp0_maxi_awready),  // in
  .M_AXI_GP0_bvalid  (gp0_maxi_bvalid ),  // in
  .M_AXI_GP0_rlast   (gp0_maxi_rlast  ),  // in
  .M_AXI_GP0_rvalid  (gp0_maxi_rvalid ),  // in
  .M_AXI_GP0_wready  (gp0_maxi_wready ),  // in
  .M_AXI_GP0_bid     (gp0_maxi_bid    ),  // in 12
  .M_AXI_GP0_rid     (gp0_maxi_rid    ),  // in 12
  .M_AXI_GP0_bresp   (gp0_maxi_bresp  ),  // in 2
  .M_AXI_GP0_rresp   (gp0_maxi_rresp  ),  // in 2
  .M_AXI_GP0_rdata   (gp0_maxi_rdata  ),  // in 32
  // AXI-4 streaming interfaces
  .S_AXI_STR1_aclk   (clk        ),  .S_AXI_STR0_aclk   (clk        ),
  .S_AXI_STR1_arstn  (rstn       ),  .S_AXI_STR0_arstn  (rstn       ),
  .S_AXI_STR1_tdata  (axi1_tdata ),  .S_AXI_STR0_tdata  (axi0_tdata ),
  .S_AXI_STR1_tkeep  ('1         ),  .S_AXI_STR0_tkeep  ('1         ),
  .S_AXI_STR1_tlast  (axi1_tlast ),  .S_AXI_STR0_tlast  (axi0_tlast ),
  .S_AXI_STR1_tready (axi1_tready),  .S_AXI_STR0_tready (axi0_tready),
  .S_AXI_STR1_tvalid (axi1_tvalid),  .S_AXI_STR0_tvalid (axi0_tvalid)
);

endmodule
