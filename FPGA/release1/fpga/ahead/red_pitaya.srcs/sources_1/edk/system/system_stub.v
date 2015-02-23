//-----------------------------------------------------------------------------
// system_stub.v
//-----------------------------------------------------------------------------

module system_stub
  (
    processing_system7_0_MIO,
    processing_system7_0_PS_SRSTB_pin,
    processing_system7_0_PS_CLK_pin,
    processing_system7_0_PS_PORB_pin,
    processing_system7_0_DDR_Clk,
    processing_system7_0_DDR_Clk_n,
    processing_system7_0_DDR_CKE,
    processing_system7_0_DDR_CS_n,
    processing_system7_0_DDR_RAS_n,
    processing_system7_0_DDR_CAS_n,
    processing_system7_0_DDR_WEB_pin,
    processing_system7_0_DDR_BankAddr,
    processing_system7_0_DDR_Addr,
    processing_system7_0_DDR_ODT,
    processing_system7_0_DDR_DRSTB,
    processing_system7_0_DDR_DQ,
    processing_system7_0_DDR_DM,
    processing_system7_0_DDR_DQS,
    processing_system7_0_DDR_DQS_n,
    processing_system7_0_DDR_VRN,
    processing_system7_0_DDR_VRP,
    processing_system7_0_FCLK_CLK1_pin,
    processing_system7_0_FCLK_CLK2_pin,
    processing_system7_0_FCLK_CLK3_pin,
    processing_system7_0_FCLK_CLK0_pin,
    processing_system7_0_M_AXI_GP0_ARESETN_pin,
    processing_system7_0_FCLK_RESET1_N_pin,
    processing_system7_0_FCLK_RESET0_N_pin,
    processing_system7_0_FCLK_RESET3_N_pin,
    processing_system7_0_FCLK_RESET2_N_pin,
    processing_system7_0_M_AXI_GP0_ARVALID_pin,
    processing_system7_0_M_AXI_GP0_AWVALID_pin,
    processing_system7_0_M_AXI_GP0_BREADY_pin,
    processing_system7_0_M_AXI_GP0_RREADY_pin,
    processing_system7_0_M_AXI_GP0_WLAST_pin,
    processing_system7_0_M_AXI_GP0_WVALID_pin,
    processing_system7_0_M_AXI_GP0_ARID_pin,
    processing_system7_0_M_AXI_GP0_AWID_pin,
    processing_system7_0_M_AXI_GP0_WID_pin,
    processing_system7_0_M_AXI_GP0_ARBURST_pin,
    processing_system7_0_M_AXI_GP0_ARLOCK_pin,
    processing_system7_0_M_AXI_GP0_ARSIZE_pin,
    processing_system7_0_M_AXI_GP0_AWBURST_pin,
    processing_system7_0_M_AXI_GP0_AWLOCK_pin,
    processing_system7_0_M_AXI_GP0_AWSIZE_pin,
    processing_system7_0_M_AXI_GP0_ARPROT_pin,
    processing_system7_0_M_AXI_GP0_AWPROT_pin,
    processing_system7_0_M_AXI_GP0_ARADDR_pin,
    processing_system7_0_M_AXI_GP0_AWADDR_pin,
    processing_system7_0_M_AXI_GP0_WDATA_pin,
    processing_system7_0_M_AXI_GP0_ARCACHE_pin,
    processing_system7_0_M_AXI_GP0_ARLEN_pin,
    processing_system7_0_M_AXI_GP0_ARQOS_pin,
    processing_system7_0_M_AXI_GP0_AWCACHE_pin,
    processing_system7_0_M_AXI_GP0_AWLEN_pin,
    processing_system7_0_M_AXI_GP0_AWQOS_pin,
    processing_system7_0_M_AXI_GP0_WSTRB_pin,
    processing_system7_0_M_AXI_GP0_ACLK_pin,
    processing_system7_0_M_AXI_GP0_ARREADY_pin,
    processing_system7_0_M_AXI_GP0_AWREADY_pin,
    processing_system7_0_M_AXI_GP0_BVALID_pin,
    processing_system7_0_M_AXI_GP0_RLAST_pin,
    processing_system7_0_M_AXI_GP0_RVALID_pin,
    processing_system7_0_M_AXI_GP0_WREADY_pin,
    processing_system7_0_M_AXI_GP0_BID_pin,
    processing_system7_0_M_AXI_GP0_RID_pin,
    processing_system7_0_M_AXI_GP0_BRESP_pin,
    processing_system7_0_M_AXI_GP0_RRESP_pin,
    processing_system7_0_M_AXI_GP0_RDATA_pin,
    processing_system7_0_SPI0_MOSI_I_pin,
    processing_system7_0_SPI0_MOSI_T_pin,
    processing_system7_0_SPI0_MOSI_O_pin,
    processing_system7_0_SPI0_MISO_I_pin,
    processing_system7_0_SPI0_MISO_O_pin,
    processing_system7_0_SPI0_SS_O_pin,
    processing_system7_0_SPI0_SS1_O_pin,
    processing_system7_0_SPI0_SS2_O_pin,
    processing_system7_0_SPI0_SS_T_pin,
    processing_system7_0_SPI0_SCLK_I_pin,
    processing_system7_0_SPI0_SS_I_pin,
    processing_system7_0_SPI0_SCLK_T_pin,
    processing_system7_0_SPI0_MISO_T_pin,
    processing_system7_0_SPI0_SCLK_O_pin
  );
  inout [53:0] processing_system7_0_MIO;
  input processing_system7_0_PS_SRSTB_pin;
  input processing_system7_0_PS_CLK_pin;
  input processing_system7_0_PS_PORB_pin;
  inout processing_system7_0_DDR_Clk;
  inout processing_system7_0_DDR_Clk_n;
  inout processing_system7_0_DDR_CKE;
  inout processing_system7_0_DDR_CS_n;
  inout processing_system7_0_DDR_RAS_n;
  inout processing_system7_0_DDR_CAS_n;
  output processing_system7_0_DDR_WEB_pin;
  inout [2:0] processing_system7_0_DDR_BankAddr;
  inout [14:0] processing_system7_0_DDR_Addr;
  inout processing_system7_0_DDR_ODT;
  inout processing_system7_0_DDR_DRSTB;
  inout [31:0] processing_system7_0_DDR_DQ;
  inout [3:0] processing_system7_0_DDR_DM;
  inout [3:0] processing_system7_0_DDR_DQS;
  inout [3:0] processing_system7_0_DDR_DQS_n;
  inout processing_system7_0_DDR_VRN;
  inout processing_system7_0_DDR_VRP;
  output processing_system7_0_FCLK_CLK1_pin;
  output processing_system7_0_FCLK_CLK2_pin;
  output processing_system7_0_FCLK_CLK3_pin;
  output processing_system7_0_FCLK_CLK0_pin;
  output processing_system7_0_M_AXI_GP0_ARESETN_pin;
  output processing_system7_0_FCLK_RESET1_N_pin;
  output processing_system7_0_FCLK_RESET0_N_pin;
  output processing_system7_0_FCLK_RESET3_N_pin;
  output processing_system7_0_FCLK_RESET2_N_pin;
  output processing_system7_0_M_AXI_GP0_ARVALID_pin;
  output processing_system7_0_M_AXI_GP0_AWVALID_pin;
  output processing_system7_0_M_AXI_GP0_BREADY_pin;
  output processing_system7_0_M_AXI_GP0_RREADY_pin;
  output processing_system7_0_M_AXI_GP0_WLAST_pin;
  output processing_system7_0_M_AXI_GP0_WVALID_pin;
  output [11:0] processing_system7_0_M_AXI_GP0_ARID_pin;
  output [11:0] processing_system7_0_M_AXI_GP0_AWID_pin;
  output [11:0] processing_system7_0_M_AXI_GP0_WID_pin;
  output [1:0] processing_system7_0_M_AXI_GP0_ARBURST_pin;
  output [1:0] processing_system7_0_M_AXI_GP0_ARLOCK_pin;
  output [2:0] processing_system7_0_M_AXI_GP0_ARSIZE_pin;
  output [1:0] processing_system7_0_M_AXI_GP0_AWBURST_pin;
  output [1:0] processing_system7_0_M_AXI_GP0_AWLOCK_pin;
  output [2:0] processing_system7_0_M_AXI_GP0_AWSIZE_pin;
  output [2:0] processing_system7_0_M_AXI_GP0_ARPROT_pin;
  output [2:0] processing_system7_0_M_AXI_GP0_AWPROT_pin;
  output [31:0] processing_system7_0_M_AXI_GP0_ARADDR_pin;
  output [31:0] processing_system7_0_M_AXI_GP0_AWADDR_pin;
  output [31:0] processing_system7_0_M_AXI_GP0_WDATA_pin;
  output [3:0] processing_system7_0_M_AXI_GP0_ARCACHE_pin;
  output [3:0] processing_system7_0_M_AXI_GP0_ARLEN_pin;
  output [3:0] processing_system7_0_M_AXI_GP0_ARQOS_pin;
  output [3:0] processing_system7_0_M_AXI_GP0_AWCACHE_pin;
  output [3:0] processing_system7_0_M_AXI_GP0_AWLEN_pin;
  output [3:0] processing_system7_0_M_AXI_GP0_AWQOS_pin;
  output [3:0] processing_system7_0_M_AXI_GP0_WSTRB_pin;
  input processing_system7_0_M_AXI_GP0_ACLK_pin;
  input processing_system7_0_M_AXI_GP0_ARREADY_pin;
  input processing_system7_0_M_AXI_GP0_AWREADY_pin;
  input processing_system7_0_M_AXI_GP0_BVALID_pin;
  input processing_system7_0_M_AXI_GP0_RLAST_pin;
  input processing_system7_0_M_AXI_GP0_RVALID_pin;
  input processing_system7_0_M_AXI_GP0_WREADY_pin;
  input [11:0] processing_system7_0_M_AXI_GP0_BID_pin;
  input [11:0] processing_system7_0_M_AXI_GP0_RID_pin;
  input [1:0] processing_system7_0_M_AXI_GP0_BRESP_pin;
  input [1:0] processing_system7_0_M_AXI_GP0_RRESP_pin;
  input [31:0] processing_system7_0_M_AXI_GP0_RDATA_pin;
  input processing_system7_0_SPI0_MOSI_I_pin;
  output processing_system7_0_SPI0_MOSI_T_pin;
  output processing_system7_0_SPI0_MOSI_O_pin;
  input processing_system7_0_SPI0_MISO_I_pin;
  output processing_system7_0_SPI0_MISO_O_pin;
  output processing_system7_0_SPI0_SS_O_pin;
  output processing_system7_0_SPI0_SS1_O_pin;
  output processing_system7_0_SPI0_SS2_O_pin;
  output processing_system7_0_SPI0_SS_T_pin;
  input processing_system7_0_SPI0_SCLK_I_pin;
  input processing_system7_0_SPI0_SS_I_pin;
  output processing_system7_0_SPI0_SCLK_T_pin;
  output processing_system7_0_SPI0_MISO_T_pin;
  output processing_system7_0_SPI0_SCLK_O_pin;

  (* BOX_TYPE = "user_black_box" *)
  system
    system_i (
      .processing_system7_0_MIO ( processing_system7_0_MIO ),
      .processing_system7_0_PS_SRSTB_pin ( processing_system7_0_PS_SRSTB_pin ),
      .processing_system7_0_PS_CLK_pin ( processing_system7_0_PS_CLK_pin ),
      .processing_system7_0_PS_PORB_pin ( processing_system7_0_PS_PORB_pin ),
      .processing_system7_0_DDR_Clk ( processing_system7_0_DDR_Clk ),
      .processing_system7_0_DDR_Clk_n ( processing_system7_0_DDR_Clk_n ),
      .processing_system7_0_DDR_CKE ( processing_system7_0_DDR_CKE ),
      .processing_system7_0_DDR_CS_n ( processing_system7_0_DDR_CS_n ),
      .processing_system7_0_DDR_RAS_n ( processing_system7_0_DDR_RAS_n ),
      .processing_system7_0_DDR_CAS_n ( processing_system7_0_DDR_CAS_n ),
      .processing_system7_0_DDR_WEB_pin ( processing_system7_0_DDR_WEB_pin ),
      .processing_system7_0_DDR_BankAddr ( processing_system7_0_DDR_BankAddr ),
      .processing_system7_0_DDR_Addr ( processing_system7_0_DDR_Addr ),
      .processing_system7_0_DDR_ODT ( processing_system7_0_DDR_ODT ),
      .processing_system7_0_DDR_DRSTB ( processing_system7_0_DDR_DRSTB ),
      .processing_system7_0_DDR_DQ ( processing_system7_0_DDR_DQ ),
      .processing_system7_0_DDR_DM ( processing_system7_0_DDR_DM ),
      .processing_system7_0_DDR_DQS ( processing_system7_0_DDR_DQS ),
      .processing_system7_0_DDR_DQS_n ( processing_system7_0_DDR_DQS_n ),
      .processing_system7_0_DDR_VRN ( processing_system7_0_DDR_VRN ),
      .processing_system7_0_DDR_VRP ( processing_system7_0_DDR_VRP ),
      .processing_system7_0_FCLK_CLK1_pin ( processing_system7_0_FCLK_CLK1_pin ),
      .processing_system7_0_FCLK_CLK2_pin ( processing_system7_0_FCLK_CLK2_pin ),
      .processing_system7_0_FCLK_CLK3_pin ( processing_system7_0_FCLK_CLK3_pin ),
      .processing_system7_0_FCLK_CLK0_pin ( processing_system7_0_FCLK_CLK0_pin ),
      .processing_system7_0_M_AXI_GP0_ARESETN_pin ( processing_system7_0_M_AXI_GP0_ARESETN_pin ),
      .processing_system7_0_FCLK_RESET1_N_pin ( processing_system7_0_FCLK_RESET1_N_pin ),
      .processing_system7_0_FCLK_RESET0_N_pin ( processing_system7_0_FCLK_RESET0_N_pin ),
      .processing_system7_0_FCLK_RESET3_N_pin ( processing_system7_0_FCLK_RESET3_N_pin ),
      .processing_system7_0_FCLK_RESET2_N_pin ( processing_system7_0_FCLK_RESET2_N_pin ),
      .processing_system7_0_M_AXI_GP0_ARVALID_pin ( processing_system7_0_M_AXI_GP0_ARVALID_pin ),
      .processing_system7_0_M_AXI_GP0_AWVALID_pin ( processing_system7_0_M_AXI_GP0_AWVALID_pin ),
      .processing_system7_0_M_AXI_GP0_BREADY_pin ( processing_system7_0_M_AXI_GP0_BREADY_pin ),
      .processing_system7_0_M_AXI_GP0_RREADY_pin ( processing_system7_0_M_AXI_GP0_RREADY_pin ),
      .processing_system7_0_M_AXI_GP0_WLAST_pin ( processing_system7_0_M_AXI_GP0_WLAST_pin ),
      .processing_system7_0_M_AXI_GP0_WVALID_pin ( processing_system7_0_M_AXI_GP0_WVALID_pin ),
      .processing_system7_0_M_AXI_GP0_ARID_pin ( processing_system7_0_M_AXI_GP0_ARID_pin ),
      .processing_system7_0_M_AXI_GP0_AWID_pin ( processing_system7_0_M_AXI_GP0_AWID_pin ),
      .processing_system7_0_M_AXI_GP0_WID_pin ( processing_system7_0_M_AXI_GP0_WID_pin ),
      .processing_system7_0_M_AXI_GP0_ARBURST_pin ( processing_system7_0_M_AXI_GP0_ARBURST_pin ),
      .processing_system7_0_M_AXI_GP0_ARLOCK_pin ( processing_system7_0_M_AXI_GP0_ARLOCK_pin ),
      .processing_system7_0_M_AXI_GP0_ARSIZE_pin ( processing_system7_0_M_AXI_GP0_ARSIZE_pin ),
      .processing_system7_0_M_AXI_GP0_AWBURST_pin ( processing_system7_0_M_AXI_GP0_AWBURST_pin ),
      .processing_system7_0_M_AXI_GP0_AWLOCK_pin ( processing_system7_0_M_AXI_GP0_AWLOCK_pin ),
      .processing_system7_0_M_AXI_GP0_AWSIZE_pin ( processing_system7_0_M_AXI_GP0_AWSIZE_pin ),
      .processing_system7_0_M_AXI_GP0_ARPROT_pin ( processing_system7_0_M_AXI_GP0_ARPROT_pin ),
      .processing_system7_0_M_AXI_GP0_AWPROT_pin ( processing_system7_0_M_AXI_GP0_AWPROT_pin ),
      .processing_system7_0_M_AXI_GP0_ARADDR_pin ( processing_system7_0_M_AXI_GP0_ARADDR_pin ),
      .processing_system7_0_M_AXI_GP0_AWADDR_pin ( processing_system7_0_M_AXI_GP0_AWADDR_pin ),
      .processing_system7_0_M_AXI_GP0_WDATA_pin ( processing_system7_0_M_AXI_GP0_WDATA_pin ),
      .processing_system7_0_M_AXI_GP0_ARCACHE_pin ( processing_system7_0_M_AXI_GP0_ARCACHE_pin ),
      .processing_system7_0_M_AXI_GP0_ARLEN_pin ( processing_system7_0_M_AXI_GP0_ARLEN_pin ),
      .processing_system7_0_M_AXI_GP0_ARQOS_pin ( processing_system7_0_M_AXI_GP0_ARQOS_pin ),
      .processing_system7_0_M_AXI_GP0_AWCACHE_pin ( processing_system7_0_M_AXI_GP0_AWCACHE_pin ),
      .processing_system7_0_M_AXI_GP0_AWLEN_pin ( processing_system7_0_M_AXI_GP0_AWLEN_pin ),
      .processing_system7_0_M_AXI_GP0_AWQOS_pin ( processing_system7_0_M_AXI_GP0_AWQOS_pin ),
      .processing_system7_0_M_AXI_GP0_WSTRB_pin ( processing_system7_0_M_AXI_GP0_WSTRB_pin ),
      .processing_system7_0_M_AXI_GP0_ACLK_pin ( processing_system7_0_M_AXI_GP0_ACLK_pin ),
      .processing_system7_0_M_AXI_GP0_ARREADY_pin ( processing_system7_0_M_AXI_GP0_ARREADY_pin ),
      .processing_system7_0_M_AXI_GP0_AWREADY_pin ( processing_system7_0_M_AXI_GP0_AWREADY_pin ),
      .processing_system7_0_M_AXI_GP0_BVALID_pin ( processing_system7_0_M_AXI_GP0_BVALID_pin ),
      .processing_system7_0_M_AXI_GP0_RLAST_pin ( processing_system7_0_M_AXI_GP0_RLAST_pin ),
      .processing_system7_0_M_AXI_GP0_RVALID_pin ( processing_system7_0_M_AXI_GP0_RVALID_pin ),
      .processing_system7_0_M_AXI_GP0_WREADY_pin ( processing_system7_0_M_AXI_GP0_WREADY_pin ),
      .processing_system7_0_M_AXI_GP0_BID_pin ( processing_system7_0_M_AXI_GP0_BID_pin ),
      .processing_system7_0_M_AXI_GP0_RID_pin ( processing_system7_0_M_AXI_GP0_RID_pin ),
      .processing_system7_0_M_AXI_GP0_BRESP_pin ( processing_system7_0_M_AXI_GP0_BRESP_pin ),
      .processing_system7_0_M_AXI_GP0_RRESP_pin ( processing_system7_0_M_AXI_GP0_RRESP_pin ),
      .processing_system7_0_M_AXI_GP0_RDATA_pin ( processing_system7_0_M_AXI_GP0_RDATA_pin ),
      .processing_system7_0_SPI0_MOSI_I_pin ( processing_system7_0_SPI0_MOSI_I_pin ),
      .processing_system7_0_SPI0_MOSI_T_pin ( processing_system7_0_SPI0_MOSI_T_pin ),
      .processing_system7_0_SPI0_MOSI_O_pin ( processing_system7_0_SPI0_MOSI_O_pin ),
      .processing_system7_0_SPI0_MISO_I_pin ( processing_system7_0_SPI0_MISO_I_pin ),
      .processing_system7_0_SPI0_MISO_O_pin ( processing_system7_0_SPI0_MISO_O_pin ),
      .processing_system7_0_SPI0_SS_O_pin ( processing_system7_0_SPI0_SS_O_pin ),
      .processing_system7_0_SPI0_SS1_O_pin ( processing_system7_0_SPI0_SS1_O_pin ),
      .processing_system7_0_SPI0_SS2_O_pin ( processing_system7_0_SPI0_SS2_O_pin ),
      .processing_system7_0_SPI0_SS_T_pin ( processing_system7_0_SPI0_SS_T_pin ),
      .processing_system7_0_SPI0_SCLK_I_pin ( processing_system7_0_SPI0_SCLK_I_pin ),
      .processing_system7_0_SPI0_SS_I_pin ( processing_system7_0_SPI0_SS_I_pin ),
      .processing_system7_0_SPI0_SCLK_T_pin ( processing_system7_0_SPI0_SCLK_T_pin ),
      .processing_system7_0_SPI0_MISO_T_pin ( processing_system7_0_SPI0_MISO_T_pin ),
      .processing_system7_0_SPI0_SCLK_O_pin ( processing_system7_0_SPI0_SCLK_O_pin )
    );

endmodule

