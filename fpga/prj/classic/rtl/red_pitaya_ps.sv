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
  output logic [  4-1:0] fclk_clk_o         ,
  output logic [  4-1:0] fclk_rstn_o        ,
  // XADC
  input  logic  [ 5-1:0] vinp_i             ,  // voltages p
  input  logic  [ 5-1:0] vinn_i             ,  // voltages n
  // GPIO
  gpio_if.m              gpio,
  // system read/write channel
  sys_bus_if.m           bus
);

////////////////////////////////////////////////////////////////////////////////
// AXI SLAVE
////////////////////////////////////////////////////////////////////////////////

logic [4-1:0] fclk_clk ;
logic [4-1:0] fclk_rstn;

axi4_if #(.DW (32), .AW (32), .IW (12), .LW (4)) axi_gp (.ACLK (bus.clk), .ARESETn (bus.rstn));

axi4_slave #(
  .DW (32),
  .AW (32),
  .IW (12)
) axi_slave_gp0 (
  // AXI bus
  .axi       (axi_gp),
  // system read/write channel
  .bus       (bus)
);

////////////////////////////////////////////////////////////////////////////////
// PS STUB
////////////////////////////////////////////////////////////////////////////////

assign fclk_rstn_o = fclk_rstn;

BUFG fclk_buf [4-1:0] (.O(fclk_clk_o), .I(fclk_clk));

system system_i (
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
  .M_AXI_GP0_ACLK    (axi_gp.ACLK   ),
//  .M_AXI_GP0_ARESETn (axi_gp.ARESETn),
  .M_AXI_GP0_arvalid (axi_gp.ARVALID),
  .M_AXI_GP0_awvalid (axi_gp.AWVALID),
  .M_AXI_GP0_bready  (axi_gp.BREADY ),
  .M_AXI_GP0_rready  (axi_gp.RREADY ),
  .M_AXI_GP0_wlast   (axi_gp.WLAST  ),
  .M_AXI_GP0_wvalid  (axi_gp.WVALID ),
  .M_AXI_GP0_arid    (axi_gp.ARID   ),
  .M_AXI_GP0_awid    (axi_gp.AWID   ),
  .M_AXI_GP0_wid     (axi_gp.WID    ),
  .M_AXI_GP0_arburst (axi_gp.ARBURST),
  .M_AXI_GP0_arlock  (axi_gp.ARLOCK ),
  .M_AXI_GP0_arsize  (axi_gp.ARSIZE ),
  .M_AXI_GP0_awburst (axi_gp.AWBURST),
  .M_AXI_GP0_awlock  (axi_gp.AWLOCK ),
  .M_AXI_GP0_awsize  (axi_gp.AWSIZE ),
  .M_AXI_GP0_arprot  (axi_gp.ARPROT ),
  .M_AXI_GP0_awprot  (axi_gp.AWPROT ),
  .M_AXI_GP0_araddr  (axi_gp.ARADDR ),
  .M_AXI_GP0_awaddr  (axi_gp.AWADDR ),
  .M_AXI_GP0_wdata   (axi_gp.WDATA  ),
  .M_AXI_GP0_arcache (axi_gp.ARCACHE),
  .M_AXI_GP0_arlen   (axi_gp.ARLEN  ),
  .M_AXI_GP0_arqos   (axi_gp.ARQOS  ),
  .M_AXI_GP0_awcache (axi_gp.AWCACHE),
  .M_AXI_GP0_awlen   (axi_gp.AWLEN  ),
  .M_AXI_GP0_awqos   (axi_gp.AWQOS  ),
  .M_AXI_GP0_wstrb   (axi_gp.WSTRB  ),
  .M_AXI_GP0_arready (axi_gp.ARREADY),
  .M_AXI_GP0_awready (axi_gp.AWREADY),
  .M_AXI_GP0_bvalid  (axi_gp.BVALID ),
  .M_AXI_GP0_rlast   (axi_gp.RLAST  ),
  .M_AXI_GP0_rvalid  (axi_gp.RVALID ),
  .M_AXI_GP0_wready  (axi_gp.WREADY ),
  .M_AXI_GP0_bid     (axi_gp.BID    ),
  .M_AXI_GP0_rid     (axi_gp.RID    ),
  .M_AXI_GP0_bresp   (axi_gp.BRESP  ),
  .M_AXI_GP0_rresp   (axi_gp.RRESP  ),
  .M_AXI_GP0_rdata   (axi_gp.RDATA  ),
  // GPIO
  .GPIO_tri_i (gpio.i),
  .GPIO_tri_o (gpio.o),
  .GPIO_tri_t (gpio.t),
  // SPI
  .SPI0_io0_i (1'b0),
  .SPI0_io0_o (),
  .SPI0_io0_t (),
  .SPI0_io1_i (1'b0),
  .SPI0_io1_o (),
  .SPI0_io1_t (),
  .SPI0_sck_i (1'b0),
  .SPI0_sck_o (),
  .SPI0_sck_t (),
  .SPI0_ss1_o (),
  .SPI0_ss2_o (),
  .SPI0_ss_i  (1'b0),
  .SPI0_ss_o  (),
  .SPI0_ss_t  ()
);

// since the PS GP0 port is AXI3 and the local bus is AXI4
assign axi_gp.AWREGION = '0;
assign axi_gp.ARREGION = '0;

endmodule
