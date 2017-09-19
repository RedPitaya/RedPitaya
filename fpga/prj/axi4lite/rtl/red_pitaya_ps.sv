////////////////////////////////////////////////////////////////////////////////
// Red Pitaya Processing System (PS) wrapper
// Author: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module red_pitaya_ps (
  // PS peripherals
  inout  logic [54-1:0] FIXED_IO_mio     ,
  inout  logic          FIXED_IO_ps_clk  ,
  inout  logic          FIXED_IO_ps_porb ,
  inout  logic          FIXED_IO_ps_srstb,
  inout  logic          FIXED_IO_ddr_vrn ,
  inout  logic          FIXED_IO_ddr_vrp ,
  // DDR
  inout  logic [15-1:0] DDR_addr   ,
  inout  logic [ 3-1:0] DDR_ba     ,
  inout  logic          DDR_cas_n  ,
  inout  logic          DDR_ck_n   ,
  inout  logic          DDR_ck_p   ,
  inout  logic          DDR_cke    ,
  inout  logic          DDR_cs_n   ,
  inout  logic [ 4-1:0] DDR_dm     ,
  inout  logic [32-1:0] DDR_dq     ,
  inout  logic [ 4-1:0] DDR_dqs_n  ,
  inout  logic [ 4-1:0] DDR_dqs_p  ,
  inout  logic          DDR_odt    ,
  inout  logic          DDR_ras_n  ,
  inout  logic          DDR_reset_n,
  inout  logic          DDR_we_n   ,
  // system signals
  output logic  [4-1:0] fclk_clk_o ,
  output logic  [4-1:0] fclk_rstn_o,
  // XADC
  input  logic  [5-1:0] vinp_i,  // slow analog voltages p
  input  logic  [5-1:0] vinn_i,  // slow analog voltages n
  // GPIO
  gpio_if.m              gpio,
  // interrupt
  input logic            irq,
  // system read/write channel
  axi4_lite_if.m         bus
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

logic [4-1:0] fclk_clk ;
logic [4-1:0] fclk_rstn;

assign fclk_rstn_o = fclk_rstn;

BUFG fclk_buf [4-1:0] (.O(fclk_clk_o), .I(fclk_clk));

////////////////////////////////////////////////////////////////////////////////
// PS STUB
////////////////////////////////////////////////////////////////////////////////

system system (
  // MIO
  .FIXED_IO_mio      (FIXED_IO_mio     ),
  .FIXED_IO_ps_clk   (FIXED_IO_ps_clk  ),
  .FIXED_IO_ps_porb  (FIXED_IO_ps_porb ),
  .FIXED_IO_ps_srstb (FIXED_IO_ps_srstb),
  .FIXED_IO_ddr_vrn  (FIXED_IO_ddr_vrn ),
  .FIXED_IO_ddr_vrp  (FIXED_IO_ddr_vrp ),
  // DDR
  .DDR_addr          (DDR_addr   ),
  .DDR_ba            (DDR_ba     ),
  .DDR_cas_n         (DDR_cas_n  ),
  .DDR_ck_n          (DDR_ck_n   ),
  .DDR_ck_p          (DDR_ck_p   ),
  .DDR_cke           (DDR_cke    ),
  .DDR_cs_n          (DDR_cs_n   ),
  .DDR_dm            (DDR_dm     ),
  .DDR_dq            (DDR_dq     ),
  .DDR_dqs_n         (DDR_dqs_n  ),
  .DDR_dqs_p         (DDR_dqs_p  ),
  .DDR_odt           (DDR_odt    ),
  .DDR_ras_n         (DDR_ras_n  ),
  .DDR_reset_n       (DDR_reset_n),
  .DDR_we_n          (DDR_we_n   ),
  // FCLKs
  .FCLK_CLK0         (fclk_clk[0]),  // 125 MHz
  .FCLK_CLK1         (fclk_clk[1]),  // 142 MHz
  .FCLK_CLK2         (fclk_clk[2]),  // 166 MHz
  .FCLK_CLK3         (fclk_clk[3]),  // 200 MHz
  .FCLK_RESET0_N     (fclk_rstn[0]),
  .FCLK_RESET1_N     (fclk_rstn[1]),
  .FCLK_RESET2_N     (fclk_rstn[2]),
  .FCLK_RESET3_N     (fclk_rstn[3]),
  // XADC
//  .Vaux0_v_n (vinn_i[1]),  .Vaux0_v_p (vinp_i[1]),
//  .Vaux1_v_n (vinn_i[2]),  .Vaux1_v_p (vinp_i[2]),
//  .Vaux8_v_n (vinn_i[0]),  .Vaux8_v_p (vinp_i[0]),
//  .Vaux9_v_n (vinn_i[3]),  .Vaux9_v_p (vinp_i[3]),
//  .Vp_Vn_v_n (vinn_i[4]),  .Vp_Vn_v_p (vinp_i[4]),
  // PL clock and reset
  .PL_ACLK            (bus.ACLK   ),
  .PL_ARESETn         (bus.ARESETn),
  // GP0
  .M_AXI_GP0_araddr   (bus.ARADDR ),
  .M_AXI_GP0_arprot   (bus.ARPROT ),
  .M_AXI_GP0_arready  (bus.ARREADY),
  .M_AXI_GP0_arvalid  (bus.ARVALID),
  .M_AXI_GP0_awaddr   (bus.AWADDR ),
  .M_AXI_GP0_awprot   (bus.AWPROT ),
  .M_AXI_GP0_awready  (bus.AWREADY),
  .M_AXI_GP0_awvalid  (bus.AWVALID),
  .M_AXI_GP0_bready   (bus.BREADY ),
  .M_AXI_GP0_bresp    (bus.BRESP  ),
  .M_AXI_GP0_bvalid   (bus.BVALID ),
  .M_AXI_GP0_rdata    (bus.RDATA  ),
  .M_AXI_GP0_rready   (bus.RREADY ),
  .M_AXI_GP0_rresp    (bus.RRESP  ),
  .M_AXI_GP0_rvalid   (bus.RVALID ),
  .M_AXI_GP0_wdata    (bus.WDATA  ),
  .M_AXI_GP0_wready   (bus.WREADY ),
  .M_AXI_GP0_wstrb    (bus.WSTRB  ),
  .M_AXI_GP0_wvalid   (bus.WVALID ),
  // GPIO
  .GPIO_tri_i (gpio.i),
  .GPIO_tri_o (gpio.o),
  .GPIO_tri_t (gpio.t),
  // IRQ
  .IRQ        (irq)
);

endmodule: red_pitaya_ps
