`timescale 1 ps / 1 ps

module system #(
  int unsigned DN = 2
)(
  inout  logic [14:0] DDR_addr   ,
  inout  logic  [2:0] DDR_ba     ,
  inout  logic        DDR_cas_n  ,
  inout  logic        DDR_ck_n   ,
  inout  logic        DDR_ck_p   ,
  inout  logic        DDR_cke    ,
  inout  logic        DDR_cs_n   ,
  inout  logic  [3:0] DDR_dm     ,
  inout  logic [31:0] DDR_dq     ,
  inout  logic  [3:0] DDR_dqs_n  ,
  inout  logic  [3:0] DDR_dqs_p  ,
  inout  logic        DDR_odt    ,
  inout  logic        DDR_ras_n  ,
  inout  logic        DDR_reset_n,
  inout  logic        DDR_we_n   ,

  output logic        FCLK_CLK0,
  output logic        FCLK_CLK1,
  output logic        FCLK_CLK2,
  output logic        FCLK_CLK3,
  output logic        FCLK_RESET0_N,
  output logic        FCLK_RESET1_N,
  output logic        FCLK_RESET2_N,
  output logic        FCLK_RESET3_N,

  inout  logic        FIXED_IO_ddr_vrn ,
  inout  logic        FIXED_IO_ddr_vrp ,
  inout  logic [53:0] FIXED_IO_mio     ,
  inout  logic        FIXED_IO_ps_clk  ,
  inout  logic        FIXED_IO_ps_porb ,
  inout  logic        FIXED_IO_ps_srstb,

  input  logic        M_AXI_GP0_ACLK   ,
//input  logic        M_AXI_GP0_ARESETn,
  output logic [31:0] M_AXI_GP0_araddr ,
  output logic  [1:0] M_AXI_GP0_arburst,
  output logic  [3:0] M_AXI_GP0_arcache,
  output logic [11:0] M_AXI_GP0_arid   ,
  output logic  [3:0] M_AXI_GP0_arlen  ,
  output logic  [1:0] M_AXI_GP0_arlock ,
  output logic  [2:0] M_AXI_GP0_arprot ,
  output logic  [3:0] M_AXI_GP0_arqos  ,
  input  logic        M_AXI_GP0_arready,
  output logic  [2:0] M_AXI_GP0_arsize ,
  output logic        M_AXI_GP0_arvalid,
  output logic [31:0] M_AXI_GP0_awaddr ,
  output logic  [1:0] M_AXI_GP0_awburst,
  output logic  [3:0] M_AXI_GP0_awcache,
  output logic [11:0] M_AXI_GP0_awid   ,
  output logic  [3:0] M_AXI_GP0_awlen  ,
  output logic  [1:0] M_AXI_GP0_awlock ,
  output logic  [2:0] M_AXI_GP0_awprot ,
  output logic  [3:0] M_AXI_GP0_awqos  ,
  input  logic        M_AXI_GP0_awready,
  output logic  [2:0] M_AXI_GP0_awsize ,
  output logic        M_AXI_GP0_awvalid,
  input  logic [11:0] M_AXI_GP0_bid    ,
  output logic        M_AXI_GP0_bready ,
  input  logic  [1:0] M_AXI_GP0_bresp  ,
  input  logic        M_AXI_GP0_bvalid ,
  input  logic [31:0] M_AXI_GP0_rdata  ,
  input  logic [11:0] M_AXI_GP0_rid    ,
  input  logic        M_AXI_GP0_rlast  ,
  output logic        M_AXI_GP0_rready ,
  input  logic  [1:0] M_AXI_GP0_rresp  ,
  input  logic        M_AXI_GP0_rvalid ,
  output logic [31:0] M_AXI_GP0_wdata  ,
  output logic [11:0] M_AXI_GP0_wid    ,
  output logic        M_AXI_GP0_wlast  ,
  input  logic        M_AXI_GP0_wready ,
  output logic  [3:0] M_AXI_GP0_wstrb  ,
  output logic        M_AXI_GP0_wvalid ,

  input  logic        S_AXI_HP1_aclk   , S_AXI_HP0_aclk   ,
  input  logic [31:0] S_AXI_HP1_araddr , S_AXI_HP0_araddr ,
  input  logic  [1:0] S_AXI_HP1_arburst, S_AXI_HP0_arburst,
  input  logic  [3:0] S_AXI_HP1_arcache, S_AXI_HP0_arcache,
  input  logic  [5:0] S_AXI_HP1_arid   , S_AXI_HP0_arid   ,
  input  logic  [3:0] S_AXI_HP1_arlen  , S_AXI_HP0_arlen  ,
  input  logic  [1:0] S_AXI_HP1_arlock , S_AXI_HP0_arlock ,
  input  logic  [2:0] S_AXI_HP1_arprot , S_AXI_HP0_arprot ,
  input  logic  [3:0] S_AXI_HP1_arqos  , S_AXI_HP0_arqos  ,
  output logic        S_AXI_HP1_arready, S_AXI_HP0_arready,
  input  logic  [2:0] S_AXI_HP1_arsize , S_AXI_HP0_arsize ,
  input  logic        S_AXI_HP1_arvalid, S_AXI_HP0_arvalid,
  input  logic [31:0] S_AXI_HP1_awaddr , S_AXI_HP0_awaddr ,
  input  logic  [1:0] S_AXI_HP1_awburst, S_AXI_HP0_awburst,
  input  logic  [3:0] S_AXI_HP1_awcache, S_AXI_HP0_awcache,
  input  logic  [5:0] S_AXI_HP1_awid   , S_AXI_HP0_awid   ,
  input  logic  [3:0] S_AXI_HP1_awlen  , S_AXI_HP0_awlen  ,
  input  logic  [1:0] S_AXI_HP1_awlock , S_AXI_HP0_awlock ,
  input  logic  [2:0] S_AXI_HP1_awprot , S_AXI_HP0_awprot ,
  input  logic  [3:0] S_AXI_HP1_awqos  , S_AXI_HP0_awqos  ,
  output logic        S_AXI_HP1_awready, S_AXI_HP0_awready,
  input  logic  [2:0] S_AXI_HP1_awsize , S_AXI_HP0_awsize ,
  input  logic        S_AXI_HP1_awvalid, S_AXI_HP0_awvalid,
  output logic  [5:0] S_AXI_HP1_bid    , S_AXI_HP0_bid    ,
  input  logic        S_AXI_HP1_bready , S_AXI_HP0_bready ,
  output logic  [1:0] S_AXI_HP1_bresp  , S_AXI_HP0_bresp  ,
  output logic        S_AXI_HP1_bvalid , S_AXI_HP0_bvalid ,
  output logic [63:0] S_AXI_HP1_rdata  , S_AXI_HP0_rdata  ,
  output logic  [5:0] S_AXI_HP1_rid    , S_AXI_HP0_rid    ,
  output logic        S_AXI_HP1_rlast  , S_AXI_HP0_rlast  ,
  input  logic        S_AXI_HP1_rready , S_AXI_HP0_rready ,
  output logic  [1:0] S_AXI_HP1_rresp  , S_AXI_HP0_rresp  ,
  output logic        S_AXI_HP1_rvalid , S_AXI_HP0_rvalid ,
  input  logic [63:0] S_AXI_HP1_wdata  , S_AXI_HP0_wdata  ,
  input  logic  [5:0] S_AXI_HP1_wid    , S_AXI_HP0_wid    ,
  input  logic        S_AXI_HP1_wlast  , S_AXI_HP0_wlast  ,
  output logic        S_AXI_HP1_wready , S_AXI_HP0_wready ,
  input  logic  [7:0] S_AXI_HP1_wstrb  , S_AXI_HP0_wstrb  ,
  input  logic        S_AXI_HP1_wvalid , S_AXI_HP0_wvalid ,

  // IRQ
  input  logic        IRQ_GPIO,
  input  logic        IRQ_LG  ,
  input  logic        IRQ_LA  ,
  input  logic        IRQ_GEN0,
  input  logic        IRQ_GEN1,
  input  logic        IRQ_SCP0,
  input  logic        IRQ_SCP1,

  // XADC
  input  logic        Vaux0_v_n,
  input  logic        Vaux0_v_p,
  input  logic        Vaux1_v_n,
  input  logic        Vaux1_v_p,
  input  logic        Vaux8_v_n,
  input  logic        Vaux8_v_p,
  input  logic        Vaux9_v_n,
  input  logic        Vaux9_v_p,
  input  logic        Vp_Vn_v_n,
  input  logic        Vp_Vn_v_p,
  // GPIO
  input  logic [24-1:0] GPIO_tri_i,
  output logic [24-1:0] GPIO_tri_o,
  output logic [24-1:0] GPIO_tri_t,
  // SPI
  input  logic SPI0_io0_i,
  output logic SPI0_io0_o,
  output logic SPI0_io0_t,
  input  logic SPI0_io1_i,
  output logic SPI0_io1_o,
  output logic SPI0_io1_t,
  input  logic SPI0_sck_i,
  output logic SPI0_sck_o,
  output logic SPI0_sck_t,
  output logic SPI0_ss1_o,
  output logic SPI0_ss2_o,
  input  logic SPI0_ss_i ,
  output logic SPI0_ss_o ,
  output logic SPI0_ss_t
);

////////////////////////////////////////////////////////////////////////////////
// DDR memory
////////////////////////////////////////////////////////////////////////////////

// assign DDR_addr    = 'z;
// assign DDR_ba      = 'z;
// assign DDR_cas_n   = 'z;
// assign DDR_ck_n    = 'z;
// assign DDR_ck_p    = 'z;
// assign DDR_cke     = 'z;
// assign DDR_cs_n    = 'z;
// assign DDR_dm      = 'z;
// assign DDR_dq      = 'z;
// assign DDR_dqs_n   = 'z;
// assign DDR_dqs_p   = 'z;
// assign DDR_odt     = 'z;
// assign DDR_ras_n   = 'z;
// assign DDR_reset_n = 'z;
// assign DDR_we_n    = 'z;

////////////////////////////////////////////////////////////////////////////////
// clock and reset
////////////////////////////////////////////////////////////////////////////////

logic clk;
logic rstn;

initial         clk = 1'b1;
always #(8ns/2) clk = ~clk;

initial begin
  rstn = 1'b0;
  repeat (4) @ (posedge clk);
  rstn = 1'b1;
end

assign FCLK_CLK0 = clk;
assign FCLK_CLK1 = clk;
assign FCLK_CLK2 = clk;
assign FCLK_CLK3 = clk;
assign FCLK_RESET0_N = rstn;
assign FCLK_RESET1_N = rstn;
assign FCLK_RESET2_N = rstn;
assign FCLK_RESET3_N = rstn;

////////////////////////////////////////////////////////////////////////////////
// Fixed IO
////////////////////////////////////////////////////////////////////////////////

// assign FIXED_IO_ddr_vrn  = 'z;
// assign FIXED_IO_ddr_vrp  = 'z;
// assign FIXED_IO_mio      = 'z;
// assign FIXED_IO_ps_clk   = 'z;
// assign FIXED_IO_ps_porb  = 'z;
// assign FIXED_IO_ps_srstb = 'z;

////////////////////////////////////////////////////////////////////////////////
// AMBA AXI4 GP bus
////////////////////////////////////////////////////////////////////////////////

axi4_if #(.DW (32), .AW (32), .IW (12), .LW (4)) axi_gp (
  .ACLK    (M_AXI_GP0_ACLK   ),
//.ARESETn (M_AXI_GP0_ARESETn)
  .ARESETn (S_AXI_STR_RX0_arstn)
);

assign                  M_AXI_GP0_araddr  = axi_gp.ARADDR ;
assign                  M_AXI_GP0_arburst = axi_gp.ARBURST;
assign                  M_AXI_GP0_arcache = axi_gp.ARCACHE;
assign                  M_AXI_GP0_arid    = axi_gp.ARID   ;
assign                  M_AXI_GP0_arlen   = axi_gp.ARLEN  ;
assign                  M_AXI_GP0_arlock  = axi_gp.ARLOCK ;
assign                  M_AXI_GP0_arprot  = axi_gp.ARPROT ;
assign                  M_AXI_GP0_arqos   = axi_gp.ARQOS  ;
assign                  M_AXI_GP0_arsize  = axi_gp.ARSIZE ;
assign                  M_AXI_GP0_arvalid = axi_gp.ARVALID;
assign axi_gp.ARREADY = M_AXI_GP0_arready ;

assign                  M_AXI_GP0_awaddr  = axi_gp.AWADDR ;
assign                  M_AXI_GP0_awburst = axi_gp.AWBURST;
assign                  M_AXI_GP0_awcache = axi_gp.AWCACHE;
assign                  M_AXI_GP0_awid    = axi_gp.AWID   ;
assign                  M_AXI_GP0_awlen   = axi_gp.AWLEN  ;
assign                  M_AXI_GP0_awlock  = axi_gp.AWLOCK ;
assign                  M_AXI_GP0_awprot  = axi_gp.AWPROT ;
assign                  M_AXI_GP0_awqos   = axi_gp.AWQOS  ;
assign                  M_AXI_GP0_awsize  = axi_gp.AWSIZE ;
assign                  M_AXI_GP0_awvalid = axi_gp.AWVALID;
assign axi_gp.AWREADY = M_AXI_GP0_awready ;

assign axi_gp.BID     = M_AXI_GP0_bid     ;
assign axi_gp.BRESP   = M_AXI_GP0_bresp   ;
assign axi_gp.BVALID  = M_AXI_GP0_bvalid  ;
assign                  M_AXI_GP0_bready  = axi_gp.BREADY ;

assign axi_gp.RDATA   = M_AXI_GP0_rdata   ;
assign axi_gp.RID     = M_AXI_GP0_rid     ;
assign axi_gp.RLAST   = M_AXI_GP0_rlast   ;
assign axi_gp.RRESP   = M_AXI_GP0_rresp   ;
assign axi_gp.RVALID  = M_AXI_GP0_rvalid  ;
assign                  M_AXI_GP0_rready  = axi_gp.RREADY;

assign                  M_AXI_GP0_wdata   = axi_gp.WDATA ;
assign                  M_AXI_GP0_wid     = axi_gp.WID   ;
assign                  M_AXI_GP0_wlast   = axi_gp.WLAST ;
assign                  M_AXI_GP0_wstrb   = axi_gp.WSTRB ;
assign                  M_AXI_GP0_wvalid  = axi_gp.WVALID;
assign axi_gp.WREADY  = M_AXI_GP0_wready  ;

axi_bus_model #(.AW (32), .DW (32), .IW (12), .LW ( 4)) axi_bus_model (axi_gp);

////////////////////////////////////////////////////////////////////////////////
// AMBA AXI4 HP bus
////////////////////////////////////////////////////////////////////////////////

//     {S_AXI_HP1_aclk   , S_AXI_HP0_aclk   } = '0;
//     {S_AXI_HP1_araddr , S_AXI_HP0_araddr } = '0;
//     {S_AXI_HP1_arburst, S_AXI_HP0_arburst} = '0;
//     {S_AXI_HP1_arcache, S_AXI_HP0_arcache} = '0;
//     {S_AXI_HP1_arid   , S_AXI_HP0_arid   } = '0;
//     {S_AXI_HP1_arlen  , S_AXI_HP0_arlen  } = '0;
//     {S_AXI_HP1_arlock , S_AXI_HP0_arlock } = '0;
//     {S_AXI_HP1_arprot , S_AXI_HP0_arprot } = '0;
//     {S_AXI_HP1_arqos  , S_AXI_HP0_arqos  } = '0;
assign {S_AXI_HP1_arready, S_AXI_HP0_arready} = '0;
//     {S_AXI_HP1_arsize , S_AXI_HP0_arsize } = '0;
//     {S_AXI_HP1_arvalid, S_AXI_HP0_arvalid} = '0;
//     {S_AXI_HP1_awaddr , S_AXI_HP0_awaddr } = '0;
//     {S_AXI_HP1_awburst, S_AXI_HP0_awburst} = '0;
//     {S_AXI_HP1_awcache, S_AXI_HP0_awcache} = '0;
//     {S_AXI_HP1_awid   , S_AXI_HP0_awid   } = '0;
//     {S_AXI_HP1_awlen  , S_AXI_HP0_awlen  } = '0;
//     {S_AXI_HP1_awlock , S_AXI_HP0_awlock } = '0;
//     {S_AXI_HP1_awprot , S_AXI_HP0_awprot } = '0;
//     {S_AXI_HP1_awqos  , S_AXI_HP0_awqos  } = '0;
assign {S_AXI_HP1_awready, S_AXI_HP0_awready} = '0;
//     {S_AXI_HP1_awsize , S_AXI_HP0_awsize } = '0;
//     {S_AXI_HP1_awvalid, S_AXI_HP0_awvalid} = '0;
assign {S_AXI_HP1_bid    , S_AXI_HP0_bid    } = '0;
//     {S_AXI_HP1_bready , S_AXI_HP0_bready } = '0;
assign {S_AXI_HP1_bresp  , S_AXI_HP0_bresp  } = '0;
assign {S_AXI_HP1_bvalid , S_AXI_HP0_bvalid } = '0;
assign {S_AXI_HP1_rdata  , S_AXI_HP0_rdata  } = '0;
assign {S_AXI_HP1_rid    , S_AXI_HP0_rid    } = '0;
assign {S_AXI_HP1_rlast  , S_AXI_HP0_rlast  } = '0;
//     {S_AXI_HP1_rready , S_AXI_HP0_rready } = '0;
assign {S_AXI_HP1_rresp  , S_AXI_HP0_rresp  } = '0;
assign {S_AXI_HP1_rvalid , S_AXI_HP0_rvalid } = '0;
//     {S_AXI_HP1_wdata  , S_AXI_HP0_wdata  } = '0;
//     {S_AXI_HP1_wid    , S_AXI_HP0_wid    } = '0;
//     {S_AXI_HP1_wlast  , S_AXI_HP0_wlast  } = '0;
assign {S_AXI_HP1_wready , S_AXI_HP0_wready } = '0;
//     {S_AXI_HP1_wstrb  , S_AXI_HP0_wstrb  } = '0;
//     {S_AXI_HP1_wvalid , S_AXI_HP0_wvalid } = '0;

////////////////////////////////////////////////////////////////////////////////
// data streams
////////////////////////////////////////////////////////////////////////////////

assign {S_AXI_STR_RX3_tready, S_AXI_STR_RX2_tready, S_AXI_STR_RX1_tready, S_AXI_STR_RX0_tready} = '1;
assign {M_AXI_STR_TX3_tvalid, M_AXI_STR_TX2_tvalid, M_AXI_STR_TX1_tvalid, M_AXI_STR_TX0_tvalid} = '0;

////////////////////////////////////////////////////////////////////////////////
// analog inputs
////////////////////////////////////////////////////////////////////////////////

//  input  logic        Vaux0_v_n,
//  input  logic        Vaux0_v_p,
//  input  logic        Vaux1_v_n,
//  input  logic        Vaux1_v_p,
//  input  logic        Vaux8_v_n,
//  input  logic        Vaux8_v_p,
//  input  logic        Vaux9_v_n,
//  input  logic        Vaux9_v_p,
//  input  logic        Vp_Vn_v_n,
//  input  logic        Vp_Vn_v_p

////////////////////////////////////////////////////////////////////////////////
// GPIO
////////////////////////////////////////////////////////////////////////////////

// GPIO_tri_i
assign GPIO_tri_o = '0;
assign GPIO_tri_t = '1;  // output disabled

////////////////////////////////////////////////////////////////////////////////
// SPI
////////////////////////////////////////////////////////////////////////////////

//  input  logic SPI0_io0_i,
//  input  logic SPI0_io1_i,
//  input  logic SPI0_sck_i,
//  input  logic SPI0_ss_i ,

assign SPI0_io0_o = 1'b0;
assign SPI0_io0_t = 1'b1;
assign SPI0_io1_o = 1'b0;
assign SPI0_io1_t = 1'b1;
assign SPI0_sck_o = 1'b0;
assign SPI0_sck_t = 1'b1;
assign SPI0_ss1_o = 1'b0;
assign SPI0_ss2_o = 1'b0;
assign SPI0_ss_o  = 1'b0;
assign SPI0_ss_t  = 1'b1;

endmodule: system
