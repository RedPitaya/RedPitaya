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
  input  logic  [24-1:0] gpio_i,
  output logic  [24-1:0] gpio_o,
  output logic  [24-1:0] gpio_t,
  // system read/write channel
  sys_bus_if.m           bus,
  // AXI masters
  input              axi1_clk_i   , axi0_clk_i   ,  // global clock
  input              axi1_rstn_i  , axi0_rstn_i  ,  // global reset
  input   [ 32-1: 0] axi1_waddr_i , axi0_waddr_i ,  // system write address
  input   [ 64-1: 0] axi1_wdata_i , axi0_wdata_i ,  // system write data
  input   [  8-1: 0] axi1_wsel_i  , axi0_wsel_i  ,  // system write byte select
  input              axi1_wvalid_i, axi0_wvalid_i,  // system write data valid
  input   [  4-1: 0] axi1_wlen_i  , axi0_wlen_i  ,  // system write burst length
  input              axi1_wfixed_i, axi0_wfixed_i,  // system write burst type (fixed / incremental)
  output             axi1_werr_o  , axi0_werr_o  ,  // system write error
  output             axi1_wrdy_o  , axi0_wrdy_o     // system write ready
);

//------------------------------------------------------------------------------
// AXI masters

wire            hp1_saxi_clk_i  , hp0_saxi_clk_i  ;
wire            hp1_saxi_rstn_i , hp0_saxi_rstn_i ;

wire            hp1_saxi_arready, hp0_saxi_arready;
wire            hp1_saxi_awready, hp0_saxi_awready;
wire            hp1_saxi_bvalid , hp0_saxi_bvalid ;
wire            hp1_saxi_rlast  , hp0_saxi_rlast  ;
wire            hp1_saxi_rvalid , hp0_saxi_rvalid ;
wire            hp1_saxi_wready , hp0_saxi_wready ;
wire [  2-1: 0] hp1_saxi_bresp  , hp0_saxi_bresp  ;
wire [  2-1: 0] hp1_saxi_rresp  , hp0_saxi_rresp  ;
wire [  6-1: 0] hp1_saxi_bid    , hp0_saxi_bid    ;
wire [  6-1: 0] hp1_saxi_rid    , hp0_saxi_rid    ;
wire [ 64-1: 0] hp1_saxi_rdata  , hp0_saxi_rdata  ;
wire            hp1_saxi_aclk   , hp0_saxi_aclk   ;
wire            hp1_saxi_arvalid, hp0_saxi_arvalid;
wire            hp1_saxi_awvalid, hp0_saxi_awvalid;
wire            hp1_saxi_bready , hp0_saxi_bready ;
wire            hp1_saxi_rready , hp0_saxi_rready ;
wire            hp1_saxi_wlast  , hp0_saxi_wlast  ;
wire            hp1_saxi_wvalid , hp0_saxi_wvalid ;
wire [  2-1: 0] hp1_saxi_arburst, hp0_saxi_arburst;
wire [  2-1: 0] hp1_saxi_arlock , hp0_saxi_arlock ;
wire [  3-1: 0] hp1_saxi_arsize , hp0_saxi_arsize ;
wire [  2-1: 0] hp1_saxi_awburst, hp0_saxi_awburst;
wire [  2-1: 0] hp1_saxi_awlock , hp0_saxi_awlock ;
wire [  3-1: 0] hp1_saxi_awsize , hp0_saxi_awsize ;
wire [  3-1: 0] hp1_saxi_arprot , hp0_saxi_arprot ;
wire [  3-1: 0] hp1_saxi_awprot , hp0_saxi_awprot ;
wire [ 32-1: 0] hp1_saxi_araddr , hp0_saxi_araddr ;
wire [ 32-1: 0] hp1_saxi_awaddr , hp0_saxi_awaddr ;
wire [  4-1: 0] hp1_saxi_arcache, hp0_saxi_arcache;
wire [  4-1: 0] hp1_saxi_arlen  , hp0_saxi_arlen  ;
wire [  4-1: 0] hp1_saxi_arqos  , hp0_saxi_arqos  ;
wire [  4-1: 0] hp1_saxi_awcache, hp0_saxi_awcache;
wire [  4-1: 0] hp1_saxi_awlen  , hp0_saxi_awlen  ;
wire [  4-1: 0] hp1_saxi_awqos  , hp0_saxi_awqos  ;
wire [  6-1: 0] hp1_saxi_arid   , hp0_saxi_arid   ;
wire [  6-1: 0] hp1_saxi_awid   , hp0_saxi_awid   ;
wire [  6-1: 0] hp1_saxi_wid    , hp0_saxi_wid    ;
wire [ 64-1: 0] hp1_saxi_wdata  , hp0_saxi_wdata  ;
wire [  8-1: 0] hp1_saxi_wstrb  , hp0_saxi_wstrb  ;

axi_master #(
  .DW   (  64    ), // data width (8,16,...,1024)
  .AW   (  32    ), // address width
  .ID   (   0    ), // master ID // TODO, it is not OK to have two masters with same ID
  .IW   (   6    ), // master ID width
  .LW   (   4    )  // length width
) axi_master [1:0] (
   // global signals
  .axi_clk_i      ({hp1_saxi_clk_i  , hp0_saxi_clk_i  }), // global clock
  .axi_rstn_i     ({hp1_saxi_rstn_i , hp0_saxi_rstn_i }), // global reset
   // axi write address channel
  .axi_awid_o     ({hp1_saxi_awid   , hp0_saxi_awid   }), // write address ID
  .axi_awaddr_o   ({hp1_saxi_awaddr , hp0_saxi_awaddr }), // write address
  .axi_awlen_o    ({hp1_saxi_awlen  , hp0_saxi_awlen  }), // write burst length
  .axi_awsize_o   ({hp1_saxi_awsize , hp0_saxi_awsize }), // write burst size
  .axi_awburst_o  ({hp1_saxi_awburst, hp0_saxi_awburst}), // write burst type
  .axi_awlock_o   ({hp1_saxi_awlock , hp0_saxi_awlock }), // write lock type
  .axi_awcache_o  ({hp1_saxi_awcache, hp0_saxi_awcache}), // write cache type
  .axi_awprot_o   ({hp1_saxi_awprot , hp0_saxi_awprot }), // write protection type
  .axi_awvalid_o  ({hp1_saxi_awvalid, hp0_saxi_awvalid}), // write address valid
  .axi_awready_i  ({hp1_saxi_awready, hp0_saxi_awready}), // write ready
   // axi write data channel
  .axi_wid_o      ({hp1_saxi_wid    , hp0_saxi_wid    }), // write data ID
  .axi_wdata_o    ({hp1_saxi_wdata  , hp0_saxi_wdata  }), // write data
  .axi_wstrb_o    ({hp1_saxi_wstrb  , hp0_saxi_wstrb  }), // write strobes
  .axi_wlast_o    ({hp1_saxi_wlast  , hp0_saxi_wlast  }), // write last
  .axi_wvalid_o   ({hp1_saxi_wvalid , hp0_saxi_wvalid }), // write valid
  .axi_wready_i   ({hp1_saxi_wready , hp0_saxi_wready }), // write ready
   // axi write response channel
  .axi_bid_i      ({hp1_saxi_bid    , hp0_saxi_bid    }), // write response ID
  .axi_bresp_i    ({hp1_saxi_bresp  , hp0_saxi_bresp  }), // write response
  .axi_bvalid_i   ({hp1_saxi_bvalid , hp0_saxi_bvalid }), // write response valid
  .axi_bready_o   ({hp1_saxi_bready , hp0_saxi_bready }), // write response ready
   // axi read address channel
  .axi_arid_o     ({hp1_saxi_arid   , hp0_saxi_arid   }), // read address ID
  .axi_araddr_o   ({hp1_saxi_araddr , hp0_saxi_araddr }), // read address
  .axi_arlen_o    ({hp1_saxi_arlen  , hp0_saxi_arlen  }), // read burst length
  .axi_arsize_o   ({hp1_saxi_arsize , hp0_saxi_arsize }), // read burst size
  .axi_arburst_o  ({hp1_saxi_arburst, hp0_saxi_arburst}), // read burst type
  .axi_arlock_o   ({hp1_saxi_arlock , hp0_saxi_arlock }), // read lock type
  .axi_arcache_o  ({hp1_saxi_arcache, hp0_saxi_arcache}), // read cache type
  .axi_arprot_o   ({hp1_saxi_arprot , hp0_saxi_arprot }), // read protection type
  .axi_arvalid_o  ({hp1_saxi_arvalid, hp0_saxi_arvalid}), // read address valid
  .axi_arready_i  ({hp1_saxi_arready, hp0_saxi_arready}), // read address ready
   // axi read data channel
  .axi_rid_i      ({hp1_saxi_rid    , hp0_saxi_rid    }), // read response ID
  .axi_rdata_i    ({hp1_saxi_rdata  , hp0_saxi_rdata  }), // read data
  .axi_rresp_i    ({hp1_saxi_rresp  , hp0_saxi_rresp  }), // read response
  .axi_rlast_i    ({hp1_saxi_rlast  , hp0_saxi_rlast  }), // read last
  .axi_rvalid_i   ({hp1_saxi_rvalid , hp0_saxi_rvalid }), // read response valid
  .axi_rready_o   ({hp1_saxi_rready , hp0_saxi_rready }), // read response ready
   // system write channel
  .sys_waddr_i    ({axi1_waddr_i    , axi0_waddr_i    }), // system write address
  .sys_wdata_i    ({axi1_wdata_i    , axi0_wdata_i    }), // system write data
  .sys_wsel_i     ({axi1_wsel_i     , axi0_wsel_i     }), // system write byte select
  .sys_wvalid_i   ({axi1_wvalid_i   , axi0_wvalid_i   }), // system write data valid
  .sys_wlen_i     ({axi1_wlen_i     , axi0_wlen_i     }), // system write burst length
  .sys_wfixed_i   ({axi1_wfixed_i   , axi0_wfixed_i   }), // system write burst type (fixed / incremental)
  .sys_werr_o     ({axi1_werr_o     , axi0_werr_o     }), // system write error
  .sys_wrdy_o     ({axi1_wrdy_o     , axi0_wrdy_o     }), // system write ready
   // system read channel
  .sys_raddr_i    ({32'h0           , 32'h0           }), // system read address
  .sys_rvalid_i   ({ 1'b0           ,  1'b0           }), // system read address valid
  .sys_rsel_i     ({ 8'h0           ,  8'h0           }), // system read byte select
  .sys_rlen_i     ({ 4'h0           ,  4'h0           }), // system read burst length
  .sys_rfixed_i   ({ 1'b0           ,  1'b0           }), // system read burst type (fixed / incremental)
  .sys_rdata_o    (                                    ), // system read data
  .sys_rrdy_o     (                                    ), // system read data is ready
  .sys_rerr_o     (                                    )  // system read error
);

assign hp0_saxi_arqos  = 4'h0 ;
assign hp0_saxi_awqos  = 4'h0 ;
assign hp0_saxi_clk_i  = axi0_clk_i     ;
assign hp0_saxi_rstn_i = axi0_rstn_i    ;
assign hp0_saxi_aclk   = hp0_saxi_clk_i ;

assign hp1_saxi_arqos  = 4'h0 ;
assign hp1_saxi_awqos  = 4'h0 ;
assign hp1_saxi_clk_i  = axi1_clk_i     ;
assign hp1_saxi_rstn_i = axi1_rstn_i    ;
assign hp1_saxi_aclk   = hp1_saxi_clk_i ;

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
  .GPIO_I (gpio_i),
  .GPIO_O (gpio_o),
  .GPIO_T (gpio_t),
  // HP0                                  // HP1
  .S_AXI_HP0_arready (hp0_saxi_arready),  .S_AXI_HP1_arready (hp1_saxi_arready), // out
  .S_AXI_HP0_awready (hp0_saxi_awready),  .S_AXI_HP1_awready (hp1_saxi_awready), // out
  .S_AXI_HP0_bvalid  (hp0_saxi_bvalid ),  .S_AXI_HP1_bvalid  (hp1_saxi_bvalid ), // out
  .S_AXI_HP0_rlast   (hp0_saxi_rlast  ),  .S_AXI_HP1_rlast   (hp1_saxi_rlast  ), // out
  .S_AXI_HP0_rvalid  (hp0_saxi_rvalid ),  .S_AXI_HP1_rvalid  (hp1_saxi_rvalid ), // out
  .S_AXI_HP0_wready  (hp0_saxi_wready ),  .S_AXI_HP1_wready  (hp1_saxi_wready ), // out
  .S_AXI_HP0_bresp   (hp0_saxi_bresp  ),  .S_AXI_HP1_bresp   (hp1_saxi_bresp  ), // out 2
  .S_AXI_HP0_rresp   (hp0_saxi_rresp  ),  .S_AXI_HP1_rresp   (hp1_saxi_rresp  ), // out 2
  .S_AXI_HP0_bid     (hp0_saxi_bid    ),  .S_AXI_HP1_bid     (hp1_saxi_bid    ), // out 6
  .S_AXI_HP0_rid     (hp0_saxi_rid    ),  .S_AXI_HP1_rid     (hp1_saxi_rid    ), // out 6
  .S_AXI_HP0_rdata   (hp0_saxi_rdata  ),  .S_AXI_HP1_rdata   (hp1_saxi_rdata  ), // out 64
  .S_AXI_HP0_aclk    (hp0_saxi_aclk   ),  .S_AXI_HP1_aclk    (hp1_saxi_aclk   ), // in
  .S_AXI_HP0_arvalid (hp0_saxi_arvalid),  .S_AXI_HP1_arvalid (hp1_saxi_arvalid), // in
  .S_AXI_HP0_awvalid (hp0_saxi_awvalid),  .S_AXI_HP1_awvalid (hp1_saxi_awvalid), // in
  .S_AXI_HP0_bready  (hp0_saxi_bready ),  .S_AXI_HP1_bready  (hp1_saxi_bready ), // in
  .S_AXI_HP0_rready  (hp0_saxi_rready ),  .S_AXI_HP1_rready  (hp1_saxi_rready ), // in
  .S_AXI_HP0_wlast   (hp0_saxi_wlast  ),  .S_AXI_HP1_wlast   (hp1_saxi_wlast  ), // in
  .S_AXI_HP0_wvalid  (hp0_saxi_wvalid ),  .S_AXI_HP1_wvalid  (hp1_saxi_wvalid ), // in
  .S_AXI_HP0_arburst (hp0_saxi_arburst),  .S_AXI_HP1_arburst (hp1_saxi_arburst), // in 2
  .S_AXI_HP0_arlock  (hp0_saxi_arlock ),  .S_AXI_HP1_arlock  (hp1_saxi_arlock ), // in 2
  .S_AXI_HP0_arsize  (hp0_saxi_arsize ),  .S_AXI_HP1_arsize  (hp1_saxi_arsize ), // in 3
  .S_AXI_HP0_awburst (hp0_saxi_awburst),  .S_AXI_HP1_awburst (hp1_saxi_awburst), // in 2
  .S_AXI_HP0_awlock  (hp0_saxi_awlock ),  .S_AXI_HP1_awlock  (hp1_saxi_awlock ), // in 2
  .S_AXI_HP0_awsize  (hp0_saxi_awsize ),  .S_AXI_HP1_awsize  (hp1_saxi_awsize ), // in 3
  .S_AXI_HP0_arprot  (hp0_saxi_arprot ),  .S_AXI_HP1_arprot  (hp1_saxi_arprot ), // in 3
  .S_AXI_HP0_awprot  (hp0_saxi_awprot ),  .S_AXI_HP1_awprot  (hp1_saxi_awprot ), // in 3
  .S_AXI_HP0_araddr  (hp0_saxi_araddr ),  .S_AXI_HP1_araddr  (hp1_saxi_araddr ), // in 32
  .S_AXI_HP0_awaddr  (hp0_saxi_awaddr ),  .S_AXI_HP1_awaddr  (hp1_saxi_awaddr ), // in 32
  .S_AXI_HP0_arcache (hp0_saxi_arcache),  .S_AXI_HP1_arcache (hp1_saxi_arcache), // in 4
  .S_AXI_HP0_arlen   (hp0_saxi_arlen  ),  .S_AXI_HP1_arlen   (hp1_saxi_arlen  ), // in 4
  .S_AXI_HP0_arqos   (hp0_saxi_arqos  ),  .S_AXI_HP1_arqos   (hp1_saxi_arqos  ), // in 4
  .S_AXI_HP0_awcache (hp0_saxi_awcache),  .S_AXI_HP1_awcache (hp1_saxi_awcache), // in 4
  .S_AXI_HP0_awlen   (hp0_saxi_awlen  ),  .S_AXI_HP1_awlen   (hp1_saxi_awlen  ), // in 4
  .S_AXI_HP0_awqos   (hp0_saxi_awqos  ),  .S_AXI_HP1_awqos   (hp1_saxi_awqos  ), // in 4
  .S_AXI_HP0_arid    (hp0_saxi_arid   ),  .S_AXI_HP1_arid    (hp1_saxi_arid   ), // in 6
  .S_AXI_HP0_awid    (hp0_saxi_awid   ),  .S_AXI_HP1_awid    (hp1_saxi_awid   ), // in 6
  .S_AXI_HP0_wid     (hp0_saxi_wid    ),  .S_AXI_HP1_wid     (hp1_saxi_wid    ), // in 6
  .S_AXI_HP0_wdata   (hp0_saxi_wdata  ),  .S_AXI_HP1_wdata   (hp1_saxi_wdata  ), // in 64
  .S_AXI_HP0_wstrb   (hp0_saxi_wstrb  ),  .S_AXI_HP1_wstrb   (hp1_saxi_wstrb  )  // in 8
);

// since the PS GP0 port is AXI3 and the local bus is AXI4
assign axi_gp.AWREGION = '0;
assign axi_gp.ARREGION = '0;

endmodule
