//Copyright 1986-2015 Xilinx, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2015.2.1 (win64) Build 1302555 Wed Aug  5 13:06:02 MDT 2015
//Date        : Tue Nov 17 22:00:27 2015
//Host        : ULRICHHABEL0244 running 64-bit Service Pack 1  (build 7601)
//Command     : generate_target system.bd
//Design      : system
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

(* CORE_GENERATION_INFO = "system,IP_Integrator,{x_ipProduct=Vivado 2015.2.1,x_ipVendor=xilinx.com,x_ipLibrary=BlockDiagram,x_ipName=system,x_ipVersion=1.00.a,x_ipLanguage=VERILOG,numBlks=5,numReposBlks=5,numNonXlnxBlks=0,numHierBlks=0,maxHierDepth=0,synth_mode=Global}" *) (* HW_HANDOFF = "system.hwdef" *) 
module system
   (DDR_addr,
    DDR_ba,
    DDR_cas_n,
    DDR_ck_n,
    DDR_ck_p,
    DDR_cke,
    DDR_cs_n,
    DDR_dm,
    DDR_dq,
    DDR_dqs_n,
    DDR_dqs_p,
    DDR_odt,
    DDR_ras_n,
    DDR_reset_n,
    DDR_we_n,
    FCLK_CLK0,
    FCLK_CLK1,
    FCLK_CLK2,
    FCLK_CLK3,
    FCLK_RESET0_N,
    FCLK_RESET1_N,
    FCLK_RESET2_N,
    FCLK_RESET3_N,
    FIXED_IO_ddr_vrn,
    FIXED_IO_ddr_vrp,
    FIXED_IO_mio,
    FIXED_IO_ps_clk,
    FIXED_IO_ps_porb,
    FIXED_IO_ps_srstb,
    M_AXI_GP0_ACLK,
    M_AXI_GP0_araddr,
    M_AXI_GP0_arburst,
    M_AXI_GP0_arcache,
    M_AXI_GP0_arid,
    M_AXI_GP0_arlen,
    M_AXI_GP0_arlock,
    M_AXI_GP0_arprot,
    M_AXI_GP0_arqos,
    M_AXI_GP0_arready,
    M_AXI_GP0_arsize,
    M_AXI_GP0_arvalid,
    M_AXI_GP0_awaddr,
    M_AXI_GP0_awburst,
    M_AXI_GP0_awcache,
    M_AXI_GP0_awid,
    M_AXI_GP0_awlen,
    M_AXI_GP0_awlock,
    M_AXI_GP0_awprot,
    M_AXI_GP0_awqos,
    M_AXI_GP0_awready,
    M_AXI_GP0_awsize,
    M_AXI_GP0_awvalid,
    M_AXI_GP0_bid,
    M_AXI_GP0_bready,
    M_AXI_GP0_bresp,
    M_AXI_GP0_bvalid,
    M_AXI_GP0_rdata,
    M_AXI_GP0_rid,
    M_AXI_GP0_rlast,
    M_AXI_GP0_rready,
    M_AXI_GP0_rresp,
    M_AXI_GP0_rvalid,
    M_AXI_GP0_wdata,
    M_AXI_GP0_wid,
    M_AXI_GP0_wlast,
    M_AXI_GP0_wready,
    M_AXI_GP0_wstrb,
    M_AXI_GP0_wvalid,
    S_AXI_HP0_aclk,
    S_AXI_HP0_araddr,
    S_AXI_HP0_arburst,
    S_AXI_HP0_arcache,
    S_AXI_HP0_arid,
    S_AXI_HP0_arlen,
    S_AXI_HP0_arlock,
    S_AXI_HP0_arprot,
    S_AXI_HP0_arqos,
    S_AXI_HP0_arready,
    S_AXI_HP0_arsize,
    S_AXI_HP0_arvalid,
    S_AXI_HP0_awaddr,
    S_AXI_HP0_awburst,
    S_AXI_HP0_awcache,
    S_AXI_HP0_awid,
    S_AXI_HP0_awlen,
    S_AXI_HP0_awlock,
    S_AXI_HP0_awprot,
    S_AXI_HP0_awqos,
    S_AXI_HP0_awready,
    S_AXI_HP0_awsize,
    S_AXI_HP0_awvalid,
    S_AXI_HP0_bid,
    S_AXI_HP0_bready,
    S_AXI_HP0_bresp,
    S_AXI_HP0_bvalid,
    S_AXI_HP0_rdata,
    S_AXI_HP0_rid,
    S_AXI_HP0_rlast,
    S_AXI_HP0_rready,
    S_AXI_HP0_rresp,
    S_AXI_HP0_rvalid,
    S_AXI_HP0_wdata,
    S_AXI_HP0_wid,
    S_AXI_HP0_wlast,
    S_AXI_HP0_wready,
    S_AXI_HP0_wstrb,
    S_AXI_HP0_wvalid,
    S_AXI_HP1_aclk,
    S_AXI_HP1_araddr,
    S_AXI_HP1_arburst,
    S_AXI_HP1_arcache,
    S_AXI_HP1_arid,
    S_AXI_HP1_arlen,
    S_AXI_HP1_arlock,
    S_AXI_HP1_arprot,
    S_AXI_HP1_arqos,
    S_AXI_HP1_arready,
    S_AXI_HP1_arsize,
    S_AXI_HP1_arvalid,
    S_AXI_HP1_awaddr,
    S_AXI_HP1_awburst,
    S_AXI_HP1_awcache,
    S_AXI_HP1_awid,
    S_AXI_HP1_awlen,
    S_AXI_HP1_awlock,
    S_AXI_HP1_awprot,
    S_AXI_HP1_awqos,
    S_AXI_HP1_awready,
    S_AXI_HP1_awsize,
    S_AXI_HP1_awvalid,
    S_AXI_HP1_bid,
    S_AXI_HP1_bready,
    S_AXI_HP1_bresp,
    S_AXI_HP1_bvalid,
    S_AXI_HP1_rdata,
    S_AXI_HP1_rid,
    S_AXI_HP1_rlast,
    S_AXI_HP1_rready,
    S_AXI_HP1_rresp,
    S_AXI_HP1_rvalid,
    S_AXI_HP1_wdata,
    S_AXI_HP1_wid,
    S_AXI_HP1_wlast,
    S_AXI_HP1_wready,
    S_AXI_HP1_wstrb,
    S_AXI_HP1_wvalid,
    Vaux0_v_n,
    Vaux0_v_p,
    Vaux1_v_n,
    Vaux1_v_p,
    Vaux8_v_n,
    Vaux8_v_p,
    Vaux9_v_n,
    Vaux9_v_p,
    Vp_Vn_v_n,
    Vp_Vn_v_p);
  inout [14:0]DDR_addr;
  inout [2:0]DDR_ba;
  inout DDR_cas_n;
  inout DDR_ck_n;
  inout DDR_ck_p;
  inout DDR_cke;
  inout DDR_cs_n;
  inout [3:0]DDR_dm;
  inout [31:0]DDR_dq;
  inout [3:0]DDR_dqs_n;
  inout [3:0]DDR_dqs_p;
  inout DDR_odt;
  inout DDR_ras_n;
  inout DDR_reset_n;
  inout DDR_we_n;
  output FCLK_CLK0;
  output FCLK_CLK1;
  output FCLK_CLK2;
  output FCLK_CLK3;
  output FCLK_RESET0_N;
  output FCLK_RESET1_N;
  output FCLK_RESET2_N;
  output FCLK_RESET3_N;
  inout FIXED_IO_ddr_vrn;
  inout FIXED_IO_ddr_vrp;
  inout [53:0]FIXED_IO_mio;
  inout FIXED_IO_ps_clk;
  inout FIXED_IO_ps_porb;
  inout FIXED_IO_ps_srstb;
  input M_AXI_GP0_ACLK;
  output [31:0]M_AXI_GP0_araddr;
  output [1:0]M_AXI_GP0_arburst;
  output [3:0]M_AXI_GP0_arcache;
  output [11:0]M_AXI_GP0_arid;
  output [3:0]M_AXI_GP0_arlen;
  output [1:0]M_AXI_GP0_arlock;
  output [2:0]M_AXI_GP0_arprot;
  output [3:0]M_AXI_GP0_arqos;
  input M_AXI_GP0_arready;
  output [2:0]M_AXI_GP0_arsize;
  output M_AXI_GP0_arvalid;
  output [31:0]M_AXI_GP0_awaddr;
  output [1:0]M_AXI_GP0_awburst;
  output [3:0]M_AXI_GP0_awcache;
  output [11:0]M_AXI_GP0_awid;
  output [3:0]M_AXI_GP0_awlen;
  output [1:0]M_AXI_GP0_awlock;
  output [2:0]M_AXI_GP0_awprot;
  output [3:0]M_AXI_GP0_awqos;
  input M_AXI_GP0_awready;
  output [2:0]M_AXI_GP0_awsize;
  output M_AXI_GP0_awvalid;
  input [11:0]M_AXI_GP0_bid;
  output M_AXI_GP0_bready;
  input [1:0]M_AXI_GP0_bresp;
  input M_AXI_GP0_bvalid;
  input [31:0]M_AXI_GP0_rdata;
  input [11:0]M_AXI_GP0_rid;
  input M_AXI_GP0_rlast;
  output M_AXI_GP0_rready;
  input [1:0]M_AXI_GP0_rresp;
  input M_AXI_GP0_rvalid;
  output [31:0]M_AXI_GP0_wdata;
  output [11:0]M_AXI_GP0_wid;
  output M_AXI_GP0_wlast;
  input M_AXI_GP0_wready;
  output [3:0]M_AXI_GP0_wstrb;
  output M_AXI_GP0_wvalid;
  input S_AXI_HP0_aclk;
  input [31:0]S_AXI_HP0_araddr;
  input [1:0]S_AXI_HP0_arburst;
  input [3:0]S_AXI_HP0_arcache;
  input [5:0]S_AXI_HP0_arid;
  input [3:0]S_AXI_HP0_arlen;
  input [1:0]S_AXI_HP0_arlock;
  input [2:0]S_AXI_HP0_arprot;
  input [3:0]S_AXI_HP0_arqos;
  output S_AXI_HP0_arready;
  input [2:0]S_AXI_HP0_arsize;
  input S_AXI_HP0_arvalid;
  input [31:0]S_AXI_HP0_awaddr;
  input [1:0]S_AXI_HP0_awburst;
  input [3:0]S_AXI_HP0_awcache;
  input [5:0]S_AXI_HP0_awid;
  input [3:0]S_AXI_HP0_awlen;
  input [1:0]S_AXI_HP0_awlock;
  input [2:0]S_AXI_HP0_awprot;
  input [3:0]S_AXI_HP0_awqos;
  output S_AXI_HP0_awready;
  input [2:0]S_AXI_HP0_awsize;
  input S_AXI_HP0_awvalid;
  output [5:0]S_AXI_HP0_bid;
  input S_AXI_HP0_bready;
  output [1:0]S_AXI_HP0_bresp;
  output S_AXI_HP0_bvalid;
  output [63:0]S_AXI_HP0_rdata;
  output [5:0]S_AXI_HP0_rid;
  output S_AXI_HP0_rlast;
  input S_AXI_HP0_rready;
  output [1:0]S_AXI_HP0_rresp;
  output S_AXI_HP0_rvalid;
  input [63:0]S_AXI_HP0_wdata;
  input [5:0]S_AXI_HP0_wid;
  input S_AXI_HP0_wlast;
  output S_AXI_HP0_wready;
  input [7:0]S_AXI_HP0_wstrb;
  input S_AXI_HP0_wvalid;
  input S_AXI_HP1_aclk;
  input [31:0]S_AXI_HP1_araddr;
  input [1:0]S_AXI_HP1_arburst;
  input [3:0]S_AXI_HP1_arcache;
  input [5:0]S_AXI_HP1_arid;
  input [3:0]S_AXI_HP1_arlen;
  input [1:0]S_AXI_HP1_arlock;
  input [2:0]S_AXI_HP1_arprot;
  input [3:0]S_AXI_HP1_arqos;
  output S_AXI_HP1_arready;
  input [2:0]S_AXI_HP1_arsize;
  input S_AXI_HP1_arvalid;
  input [31:0]S_AXI_HP1_awaddr;
  input [1:0]S_AXI_HP1_awburst;
  input [3:0]S_AXI_HP1_awcache;
  input [5:0]S_AXI_HP1_awid;
  input [3:0]S_AXI_HP1_awlen;
  input [1:0]S_AXI_HP1_awlock;
  input [2:0]S_AXI_HP1_awprot;
  input [3:0]S_AXI_HP1_awqos;
  output S_AXI_HP1_awready;
  input [2:0]S_AXI_HP1_awsize;
  input S_AXI_HP1_awvalid;
  output [5:0]S_AXI_HP1_bid;
  input S_AXI_HP1_bready;
  output [1:0]S_AXI_HP1_bresp;
  output S_AXI_HP1_bvalid;
  output [63:0]S_AXI_HP1_rdata;
  output [5:0]S_AXI_HP1_rid;
  output S_AXI_HP1_rlast;
  input S_AXI_HP1_rready;
  output [1:0]S_AXI_HP1_rresp;
  output S_AXI_HP1_rvalid;
  input [63:0]S_AXI_HP1_wdata;
  input [5:0]S_AXI_HP1_wid;
  input S_AXI_HP1_wlast;
  output S_AXI_HP1_wready;
  input [7:0]S_AXI_HP1_wstrb;
  input S_AXI_HP1_wvalid;
  input Vaux0_v_n;
  input Vaux0_v_p;
  input Vaux1_v_n;
  input Vaux1_v_p;
  input Vaux8_v_n;
  input Vaux8_v_p;
  input Vaux9_v_n;
  input Vaux9_v_p;
  input Vp_Vn_v_n;
  input Vp_Vn_v_p;

  wire GND_1;
  wire VCC_1;
  wire Vaux0_1_V_N;
  wire Vaux0_1_V_P;
  wire Vaux1_1_V_N;
  wire Vaux1_1_V_P;
  wire Vaux8_1_V_N;
  wire Vaux8_1_V_P;
  wire Vaux9_1_V_N;
  wire Vaux9_1_V_P;
  wire Vp_Vn_1_V_N;
  wire Vp_Vn_1_V_P;
  wire [31:0]axi_protocol_converter_0_M_AXI_ARADDR;
  wire axi_protocol_converter_0_M_AXI_ARREADY;
  wire axi_protocol_converter_0_M_AXI_ARVALID;
  wire [31:0]axi_protocol_converter_0_M_AXI_AWADDR;
  wire axi_protocol_converter_0_M_AXI_AWREADY;
  wire axi_protocol_converter_0_M_AXI_AWVALID;
  wire axi_protocol_converter_0_M_AXI_BREADY;
  wire [1:0]axi_protocol_converter_0_M_AXI_BRESP;
  wire axi_protocol_converter_0_M_AXI_BVALID;
  wire [31:0]axi_protocol_converter_0_M_AXI_RDATA;
  wire axi_protocol_converter_0_M_AXI_RREADY;
  wire [1:0]axi_protocol_converter_0_M_AXI_RRESP;
  wire axi_protocol_converter_0_M_AXI_RVALID;
  wire [31:0]axi_protocol_converter_0_M_AXI_WDATA;
  wire axi_protocol_converter_0_M_AXI_WREADY;
  wire [3:0]axi_protocol_converter_0_M_AXI_WSTRB;
  wire axi_protocol_converter_0_M_AXI_WVALID;
  wire m_axi_gp0_aclk_1;
  wire [0:0]proc_sys_reset_0_interconnect_aresetn;
  wire [0:0]proc_sys_reset_0_peripheral_aresetn;
  wire [31:0]processing_system7_0_M_AXI_GP0_ARADDR;
  wire [1:0]processing_system7_0_M_AXI_GP0_ARBURST;
  wire [3:0]processing_system7_0_M_AXI_GP0_ARCACHE;
  wire [11:0]processing_system7_0_M_AXI_GP0_ARID;
  wire [3:0]processing_system7_0_M_AXI_GP0_ARLEN;
  wire [1:0]processing_system7_0_M_AXI_GP0_ARLOCK;
  wire [2:0]processing_system7_0_M_AXI_GP0_ARPROT;
  wire [3:0]processing_system7_0_M_AXI_GP0_ARQOS;
  wire processing_system7_0_M_AXI_GP0_ARREADY;
  wire [2:0]processing_system7_0_M_AXI_GP0_ARSIZE;
  wire processing_system7_0_M_AXI_GP0_ARVALID;
  wire [31:0]processing_system7_0_M_AXI_GP0_AWADDR;
  wire [1:0]processing_system7_0_M_AXI_GP0_AWBURST;
  wire [3:0]processing_system7_0_M_AXI_GP0_AWCACHE;
  wire [11:0]processing_system7_0_M_AXI_GP0_AWID;
  wire [3:0]processing_system7_0_M_AXI_GP0_AWLEN;
  wire [1:0]processing_system7_0_M_AXI_GP0_AWLOCK;
  wire [2:0]processing_system7_0_M_AXI_GP0_AWPROT;
  wire [3:0]processing_system7_0_M_AXI_GP0_AWQOS;
  wire processing_system7_0_M_AXI_GP0_AWREADY;
  wire [2:0]processing_system7_0_M_AXI_GP0_AWSIZE;
  wire processing_system7_0_M_AXI_GP0_AWVALID;
  wire [11:0]processing_system7_0_M_AXI_GP0_BID;
  wire processing_system7_0_M_AXI_GP0_BREADY;
  wire [1:0]processing_system7_0_M_AXI_GP0_BRESP;
  wire processing_system7_0_M_AXI_GP0_BVALID;
  wire [31:0]processing_system7_0_M_AXI_GP0_RDATA;
  wire [11:0]processing_system7_0_M_AXI_GP0_RID;
  wire processing_system7_0_M_AXI_GP0_RLAST;
  wire processing_system7_0_M_AXI_GP0_RREADY;
  wire [1:0]processing_system7_0_M_AXI_GP0_RRESP;
  wire processing_system7_0_M_AXI_GP0_RVALID;
  wire [31:0]processing_system7_0_M_AXI_GP0_WDATA;
  wire [11:0]processing_system7_0_M_AXI_GP0_WID;
  wire processing_system7_0_M_AXI_GP0_WLAST;
  wire processing_system7_0_M_AXI_GP0_WREADY;
  wire [3:0]processing_system7_0_M_AXI_GP0_WSTRB;
  wire processing_system7_0_M_AXI_GP0_WVALID;
  wire [31:0]processing_system7_0_M_AXI_GP1_ARADDR;
  wire [1:0]processing_system7_0_M_AXI_GP1_ARBURST;
  wire [3:0]processing_system7_0_M_AXI_GP1_ARCACHE;
  wire [11:0]processing_system7_0_M_AXI_GP1_ARID;
  wire [3:0]processing_system7_0_M_AXI_GP1_ARLEN;
  wire [1:0]processing_system7_0_M_AXI_GP1_ARLOCK;
  wire [2:0]processing_system7_0_M_AXI_GP1_ARPROT;
  wire [3:0]processing_system7_0_M_AXI_GP1_ARQOS;
  wire processing_system7_0_M_AXI_GP1_ARREADY;
  wire [2:0]processing_system7_0_M_AXI_GP1_ARSIZE;
  wire processing_system7_0_M_AXI_GP1_ARVALID;
  wire [31:0]processing_system7_0_M_AXI_GP1_AWADDR;
  wire [1:0]processing_system7_0_M_AXI_GP1_AWBURST;
  wire [3:0]processing_system7_0_M_AXI_GP1_AWCACHE;
  wire [11:0]processing_system7_0_M_AXI_GP1_AWID;
  wire [3:0]processing_system7_0_M_AXI_GP1_AWLEN;
  wire [1:0]processing_system7_0_M_AXI_GP1_AWLOCK;
  wire [2:0]processing_system7_0_M_AXI_GP1_AWPROT;
  wire [3:0]processing_system7_0_M_AXI_GP1_AWQOS;
  wire processing_system7_0_M_AXI_GP1_AWREADY;
  wire [2:0]processing_system7_0_M_AXI_GP1_AWSIZE;
  wire processing_system7_0_M_AXI_GP1_AWVALID;
  wire [11:0]processing_system7_0_M_AXI_GP1_BID;
  wire processing_system7_0_M_AXI_GP1_BREADY;
  wire [1:0]processing_system7_0_M_AXI_GP1_BRESP;
  wire processing_system7_0_M_AXI_GP1_BVALID;
  wire [31:0]processing_system7_0_M_AXI_GP1_RDATA;
  wire [11:0]processing_system7_0_M_AXI_GP1_RID;
  wire processing_system7_0_M_AXI_GP1_RLAST;
  wire processing_system7_0_M_AXI_GP1_RREADY;
  wire [1:0]processing_system7_0_M_AXI_GP1_RRESP;
  wire processing_system7_0_M_AXI_GP1_RVALID;
  wire [31:0]processing_system7_0_M_AXI_GP1_WDATA;
  wire [11:0]processing_system7_0_M_AXI_GP1_WID;
  wire processing_system7_0_M_AXI_GP1_WLAST;
  wire processing_system7_0_M_AXI_GP1_WREADY;
  wire [3:0]processing_system7_0_M_AXI_GP1_WSTRB;
  wire processing_system7_0_M_AXI_GP1_WVALID;
  wire [14:0]processing_system7_0_ddr_ADDR;
  wire [2:0]processing_system7_0_ddr_BA;
  wire processing_system7_0_ddr_CAS_N;
  wire processing_system7_0_ddr_CKE;
  wire processing_system7_0_ddr_CK_N;
  wire processing_system7_0_ddr_CK_P;
  wire processing_system7_0_ddr_CS_N;
  wire [3:0]processing_system7_0_ddr_DM;
  wire [31:0]processing_system7_0_ddr_DQ;
  wire [3:0]processing_system7_0_ddr_DQS_N;
  wire [3:0]processing_system7_0_ddr_DQS_P;
  wire processing_system7_0_ddr_ODT;
  wire processing_system7_0_ddr_RAS_N;
  wire processing_system7_0_ddr_RESET_N;
  wire processing_system7_0_ddr_WE_N;
  wire processing_system7_0_fclk_clk0;
  wire processing_system7_0_fclk_clk1;
  wire processing_system7_0_fclk_clk2;
  wire processing_system7_0_fclk_clk3;
  wire processing_system7_0_fclk_reset0_n;
  wire processing_system7_0_fclk_reset1_n;
  wire processing_system7_0_fclk_reset2_n;
  wire processing_system7_0_fclk_reset3_n;
  wire processing_system7_0_fixed_io_DDR_VRN;
  wire processing_system7_0_fixed_io_DDR_VRP;
  wire [53:0]processing_system7_0_fixed_io_MIO;
  wire processing_system7_0_fixed_io_PS_CLK;
  wire processing_system7_0_fixed_io_PS_PORB;
  wire processing_system7_0_fixed_io_PS_SRSTB;
  wire [31:0]s_axi_hp0_1_ARADDR;
  wire [1:0]s_axi_hp0_1_ARBURST;
  wire [3:0]s_axi_hp0_1_ARCACHE;
  wire [5:0]s_axi_hp0_1_ARID;
  wire [3:0]s_axi_hp0_1_ARLEN;
  wire [1:0]s_axi_hp0_1_ARLOCK;
  wire [2:0]s_axi_hp0_1_ARPROT;
  wire [3:0]s_axi_hp0_1_ARQOS;
  wire s_axi_hp0_1_ARREADY;
  wire [2:0]s_axi_hp0_1_ARSIZE;
  wire s_axi_hp0_1_ARVALID;
  wire [31:0]s_axi_hp0_1_AWADDR;
  wire [1:0]s_axi_hp0_1_AWBURST;
  wire [3:0]s_axi_hp0_1_AWCACHE;
  wire [5:0]s_axi_hp0_1_AWID;
  wire [3:0]s_axi_hp0_1_AWLEN;
  wire [1:0]s_axi_hp0_1_AWLOCK;
  wire [2:0]s_axi_hp0_1_AWPROT;
  wire [3:0]s_axi_hp0_1_AWQOS;
  wire s_axi_hp0_1_AWREADY;
  wire [2:0]s_axi_hp0_1_AWSIZE;
  wire s_axi_hp0_1_AWVALID;
  wire [5:0]s_axi_hp0_1_BID;
  wire s_axi_hp0_1_BREADY;
  wire [1:0]s_axi_hp0_1_BRESP;
  wire s_axi_hp0_1_BVALID;
  wire [63:0]s_axi_hp0_1_RDATA;
  wire [5:0]s_axi_hp0_1_RID;
  wire s_axi_hp0_1_RLAST;
  wire s_axi_hp0_1_RREADY;
  wire [1:0]s_axi_hp0_1_RRESP;
  wire s_axi_hp0_1_RVALID;
  wire [63:0]s_axi_hp0_1_WDATA;
  wire [5:0]s_axi_hp0_1_WID;
  wire s_axi_hp0_1_WLAST;
  wire s_axi_hp0_1_WREADY;
  wire [7:0]s_axi_hp0_1_WSTRB;
  wire s_axi_hp0_1_WVALID;
  wire s_axi_hp0_aclk;
  wire [31:0]s_axi_hp1_1_ARADDR;
  wire [1:0]s_axi_hp1_1_ARBURST;
  wire [3:0]s_axi_hp1_1_ARCACHE;
  wire [5:0]s_axi_hp1_1_ARID;
  wire [3:0]s_axi_hp1_1_ARLEN;
  wire [1:0]s_axi_hp1_1_ARLOCK;
  wire [2:0]s_axi_hp1_1_ARPROT;
  wire [3:0]s_axi_hp1_1_ARQOS;
  wire s_axi_hp1_1_ARREADY;
  wire [2:0]s_axi_hp1_1_ARSIZE;
  wire s_axi_hp1_1_ARVALID;
  wire [31:0]s_axi_hp1_1_AWADDR;
  wire [1:0]s_axi_hp1_1_AWBURST;
  wire [3:0]s_axi_hp1_1_AWCACHE;
  wire [5:0]s_axi_hp1_1_AWID;
  wire [3:0]s_axi_hp1_1_AWLEN;
  wire [1:0]s_axi_hp1_1_AWLOCK;
  wire [2:0]s_axi_hp1_1_AWPROT;
  wire [3:0]s_axi_hp1_1_AWQOS;
  wire s_axi_hp1_1_AWREADY;
  wire [2:0]s_axi_hp1_1_AWSIZE;
  wire s_axi_hp1_1_AWVALID;
  wire [5:0]s_axi_hp1_1_BID;
  wire s_axi_hp1_1_BREADY;
  wire [1:0]s_axi_hp1_1_BRESP;
  wire s_axi_hp1_1_BVALID;
  wire [63:0]s_axi_hp1_1_RDATA;
  wire [5:0]s_axi_hp1_1_RID;
  wire s_axi_hp1_1_RLAST;
  wire s_axi_hp1_1_RREADY;
  wire [1:0]s_axi_hp1_1_RRESP;
  wire s_axi_hp1_1_RVALID;
  wire [63:0]s_axi_hp1_1_WDATA;
  wire [5:0]s_axi_hp1_1_WID;
  wire s_axi_hp1_1_WLAST;
  wire s_axi_hp1_1_WREADY;
  wire [7:0]s_axi_hp1_1_WSTRB;
  wire s_axi_hp1_1_WVALID;
  wire s_axi_hp1_aclk;
  wire xadc_wiz_0_ip2intc_irpt;
  wire [0:0]xlconstant_dout;

  assign FCLK_CLK0 = processing_system7_0_fclk_clk0;
  assign FCLK_CLK1 = processing_system7_0_fclk_clk1;
  assign FCLK_CLK2 = processing_system7_0_fclk_clk2;
  assign FCLK_CLK3 = processing_system7_0_fclk_clk3;
  assign FCLK_RESET0_N = processing_system7_0_fclk_reset0_n;
  assign FCLK_RESET1_N = processing_system7_0_fclk_reset1_n;
  assign FCLK_RESET2_N = processing_system7_0_fclk_reset2_n;
  assign FCLK_RESET3_N = processing_system7_0_fclk_reset3_n;
  assign M_AXI_GP0_araddr[31:0] = processing_system7_0_M_AXI_GP0_ARADDR;
  assign M_AXI_GP0_arburst[1:0] = processing_system7_0_M_AXI_GP0_ARBURST;
  assign M_AXI_GP0_arcache[3:0] = processing_system7_0_M_AXI_GP0_ARCACHE;
  assign M_AXI_GP0_arid[11:0] = processing_system7_0_M_AXI_GP0_ARID;
  assign M_AXI_GP0_arlen[3:0] = processing_system7_0_M_AXI_GP0_ARLEN;
  assign M_AXI_GP0_arlock[1:0] = processing_system7_0_M_AXI_GP0_ARLOCK;
  assign M_AXI_GP0_arprot[2:0] = processing_system7_0_M_AXI_GP0_ARPROT;
  assign M_AXI_GP0_arqos[3:0] = processing_system7_0_M_AXI_GP0_ARQOS;
  assign M_AXI_GP0_arsize[2:0] = processing_system7_0_M_AXI_GP0_ARSIZE;
  assign M_AXI_GP0_arvalid = processing_system7_0_M_AXI_GP0_ARVALID;
  assign M_AXI_GP0_awaddr[31:0] = processing_system7_0_M_AXI_GP0_AWADDR;
  assign M_AXI_GP0_awburst[1:0] = processing_system7_0_M_AXI_GP0_AWBURST;
  assign M_AXI_GP0_awcache[3:0] = processing_system7_0_M_AXI_GP0_AWCACHE;
  assign M_AXI_GP0_awid[11:0] = processing_system7_0_M_AXI_GP0_AWID;
  assign M_AXI_GP0_awlen[3:0] = processing_system7_0_M_AXI_GP0_AWLEN;
  assign M_AXI_GP0_awlock[1:0] = processing_system7_0_M_AXI_GP0_AWLOCK;
  assign M_AXI_GP0_awprot[2:0] = processing_system7_0_M_AXI_GP0_AWPROT;
  assign M_AXI_GP0_awqos[3:0] = processing_system7_0_M_AXI_GP0_AWQOS;
  assign M_AXI_GP0_awsize[2:0] = processing_system7_0_M_AXI_GP0_AWSIZE;
  assign M_AXI_GP0_awvalid = processing_system7_0_M_AXI_GP0_AWVALID;
  assign M_AXI_GP0_bready = processing_system7_0_M_AXI_GP0_BREADY;
  assign M_AXI_GP0_rready = processing_system7_0_M_AXI_GP0_RREADY;
  assign M_AXI_GP0_wdata[31:0] = processing_system7_0_M_AXI_GP0_WDATA;
  assign M_AXI_GP0_wid[11:0] = processing_system7_0_M_AXI_GP0_WID;
  assign M_AXI_GP0_wlast = processing_system7_0_M_AXI_GP0_WLAST;
  assign M_AXI_GP0_wstrb[3:0] = processing_system7_0_M_AXI_GP0_WSTRB;
  assign M_AXI_GP0_wvalid = processing_system7_0_M_AXI_GP0_WVALID;
  assign S_AXI_HP0_arready = s_axi_hp0_1_ARREADY;
  assign S_AXI_HP0_awready = s_axi_hp0_1_AWREADY;
  assign S_AXI_HP0_bid[5:0] = s_axi_hp0_1_BID;
  assign S_AXI_HP0_bresp[1:0] = s_axi_hp0_1_BRESP;
  assign S_AXI_HP0_bvalid = s_axi_hp0_1_BVALID;
  assign S_AXI_HP0_rdata[63:0] = s_axi_hp0_1_RDATA;
  assign S_AXI_HP0_rid[5:0] = s_axi_hp0_1_RID;
  assign S_AXI_HP0_rlast = s_axi_hp0_1_RLAST;
  assign S_AXI_HP0_rresp[1:0] = s_axi_hp0_1_RRESP;
  assign S_AXI_HP0_rvalid = s_axi_hp0_1_RVALID;
  assign S_AXI_HP0_wready = s_axi_hp0_1_WREADY;
  assign S_AXI_HP1_arready = s_axi_hp1_1_ARREADY;
  assign S_AXI_HP1_awready = s_axi_hp1_1_AWREADY;
  assign S_AXI_HP1_bid[5:0] = s_axi_hp1_1_BID;
  assign S_AXI_HP1_bresp[1:0] = s_axi_hp1_1_BRESP;
  assign S_AXI_HP1_bvalid = s_axi_hp1_1_BVALID;
  assign S_AXI_HP1_rdata[63:0] = s_axi_hp1_1_RDATA;
  assign S_AXI_HP1_rid[5:0] = s_axi_hp1_1_RID;
  assign S_AXI_HP1_rlast = s_axi_hp1_1_RLAST;
  assign S_AXI_HP1_rresp[1:0] = s_axi_hp1_1_RRESP;
  assign S_AXI_HP1_rvalid = s_axi_hp1_1_RVALID;
  assign S_AXI_HP1_wready = s_axi_hp1_1_WREADY;
  assign Vaux0_1_V_N = Vaux0_v_n;
  assign Vaux0_1_V_P = Vaux0_v_p;
  assign Vaux1_1_V_N = Vaux1_v_n;
  assign Vaux1_1_V_P = Vaux1_v_p;
  assign Vaux8_1_V_N = Vaux8_v_n;
  assign Vaux8_1_V_P = Vaux8_v_p;
  assign Vaux9_1_V_N = Vaux9_v_n;
  assign Vaux9_1_V_P = Vaux9_v_p;
  assign Vp_Vn_1_V_N = Vp_Vn_v_n;
  assign Vp_Vn_1_V_P = Vp_Vn_v_p;
  assign m_axi_gp0_aclk_1 = M_AXI_GP0_ACLK;
  assign processing_system7_0_M_AXI_GP0_ARREADY = M_AXI_GP0_arready;
  assign processing_system7_0_M_AXI_GP0_AWREADY = M_AXI_GP0_awready;
  assign processing_system7_0_M_AXI_GP0_BID = M_AXI_GP0_bid[11:0];
  assign processing_system7_0_M_AXI_GP0_BRESP = M_AXI_GP0_bresp[1:0];
  assign processing_system7_0_M_AXI_GP0_BVALID = M_AXI_GP0_bvalid;
  assign processing_system7_0_M_AXI_GP0_RDATA = M_AXI_GP0_rdata[31:0];
  assign processing_system7_0_M_AXI_GP0_RID = M_AXI_GP0_rid[11:0];
  assign processing_system7_0_M_AXI_GP0_RLAST = M_AXI_GP0_rlast;
  assign processing_system7_0_M_AXI_GP0_RRESP = M_AXI_GP0_rresp[1:0];
  assign processing_system7_0_M_AXI_GP0_RVALID = M_AXI_GP0_rvalid;
  assign processing_system7_0_M_AXI_GP0_WREADY = M_AXI_GP0_wready;
  assign s_axi_hp0_1_ARADDR = S_AXI_HP0_araddr[31:0];
  assign s_axi_hp0_1_ARBURST = S_AXI_HP0_arburst[1:0];
  assign s_axi_hp0_1_ARCACHE = S_AXI_HP0_arcache[3:0];
  assign s_axi_hp0_1_ARID = S_AXI_HP0_arid[5:0];
  assign s_axi_hp0_1_ARLEN = S_AXI_HP0_arlen[3:0];
  assign s_axi_hp0_1_ARLOCK = S_AXI_HP0_arlock[1:0];
  assign s_axi_hp0_1_ARPROT = S_AXI_HP0_arprot[2:0];
  assign s_axi_hp0_1_ARQOS = S_AXI_HP0_arqos[3:0];
  assign s_axi_hp0_1_ARSIZE = S_AXI_HP0_arsize[2:0];
  assign s_axi_hp0_1_ARVALID = S_AXI_HP0_arvalid;
  assign s_axi_hp0_1_AWADDR = S_AXI_HP0_awaddr[31:0];
  assign s_axi_hp0_1_AWBURST = S_AXI_HP0_awburst[1:0];
  assign s_axi_hp0_1_AWCACHE = S_AXI_HP0_awcache[3:0];
  assign s_axi_hp0_1_AWID = S_AXI_HP0_awid[5:0];
  assign s_axi_hp0_1_AWLEN = S_AXI_HP0_awlen[3:0];
  assign s_axi_hp0_1_AWLOCK = S_AXI_HP0_awlock[1:0];
  assign s_axi_hp0_1_AWPROT = S_AXI_HP0_awprot[2:0];
  assign s_axi_hp0_1_AWQOS = S_AXI_HP0_awqos[3:0];
  assign s_axi_hp0_1_AWSIZE = S_AXI_HP0_awsize[2:0];
  assign s_axi_hp0_1_AWVALID = S_AXI_HP0_awvalid;
  assign s_axi_hp0_1_BREADY = S_AXI_HP0_bready;
  assign s_axi_hp0_1_RREADY = S_AXI_HP0_rready;
  assign s_axi_hp0_1_WDATA = S_AXI_HP0_wdata[63:0];
  assign s_axi_hp0_1_WID = S_AXI_HP0_wid[5:0];
  assign s_axi_hp0_1_WLAST = S_AXI_HP0_wlast;
  assign s_axi_hp0_1_WSTRB = S_AXI_HP0_wstrb[7:0];
  assign s_axi_hp0_1_WVALID = S_AXI_HP0_wvalid;
  assign s_axi_hp0_aclk = S_AXI_HP0_aclk;
  assign s_axi_hp1_1_ARADDR = S_AXI_HP1_araddr[31:0];
  assign s_axi_hp1_1_ARBURST = S_AXI_HP1_arburst[1:0];
  assign s_axi_hp1_1_ARCACHE = S_AXI_HP1_arcache[3:0];
  assign s_axi_hp1_1_ARID = S_AXI_HP1_arid[5:0];
  assign s_axi_hp1_1_ARLEN = S_AXI_HP1_arlen[3:0];
  assign s_axi_hp1_1_ARLOCK = S_AXI_HP1_arlock[1:0];
  assign s_axi_hp1_1_ARPROT = S_AXI_HP1_arprot[2:0];
  assign s_axi_hp1_1_ARQOS = S_AXI_HP1_arqos[3:0];
  assign s_axi_hp1_1_ARSIZE = S_AXI_HP1_arsize[2:0];
  assign s_axi_hp1_1_ARVALID = S_AXI_HP1_arvalid;
  assign s_axi_hp1_1_AWADDR = S_AXI_HP1_awaddr[31:0];
  assign s_axi_hp1_1_AWBURST = S_AXI_HP1_awburst[1:0];
  assign s_axi_hp1_1_AWCACHE = S_AXI_HP1_awcache[3:0];
  assign s_axi_hp1_1_AWID = S_AXI_HP1_awid[5:0];
  assign s_axi_hp1_1_AWLEN = S_AXI_HP1_awlen[3:0];
  assign s_axi_hp1_1_AWLOCK = S_AXI_HP1_awlock[1:0];
  assign s_axi_hp1_1_AWPROT = S_AXI_HP1_awprot[2:0];
  assign s_axi_hp1_1_AWQOS = S_AXI_HP1_awqos[3:0];
  assign s_axi_hp1_1_AWSIZE = S_AXI_HP1_awsize[2:0];
  assign s_axi_hp1_1_AWVALID = S_AXI_HP1_awvalid;
  assign s_axi_hp1_1_BREADY = S_AXI_HP1_bready;
  assign s_axi_hp1_1_RREADY = S_AXI_HP1_rready;
  assign s_axi_hp1_1_WDATA = S_AXI_HP1_wdata[63:0];
  assign s_axi_hp1_1_WID = S_AXI_HP1_wid[5:0];
  assign s_axi_hp1_1_WLAST = S_AXI_HP1_wlast;
  assign s_axi_hp1_1_WSTRB = S_AXI_HP1_wstrb[7:0];
  assign s_axi_hp1_1_WVALID = S_AXI_HP1_wvalid;
  assign s_axi_hp1_aclk = S_AXI_HP1_aclk;
  GND GND
       (.G(GND_1));
  VCC VCC
       (.P(VCC_1));
  system_axi_protocol_converter_0_0 axi_protocol_converter_0
       (.aclk(processing_system7_0_fclk_clk3),
        .aresetn(proc_sys_reset_0_interconnect_aresetn),
        .m_axi_araddr(axi_protocol_converter_0_M_AXI_ARADDR),
        .m_axi_arready(axi_protocol_converter_0_M_AXI_ARREADY),
        .m_axi_arvalid(axi_protocol_converter_0_M_AXI_ARVALID),
        .m_axi_awaddr(axi_protocol_converter_0_M_AXI_AWADDR),
        .m_axi_awready(axi_protocol_converter_0_M_AXI_AWREADY),
        .m_axi_awvalid(axi_protocol_converter_0_M_AXI_AWVALID),
        .m_axi_bready(axi_protocol_converter_0_M_AXI_BREADY),
        .m_axi_bresp(axi_protocol_converter_0_M_AXI_BRESP),
        .m_axi_bvalid(axi_protocol_converter_0_M_AXI_BVALID),
        .m_axi_rdata(axi_protocol_converter_0_M_AXI_RDATA),
        .m_axi_rready(axi_protocol_converter_0_M_AXI_RREADY),
        .m_axi_rresp(axi_protocol_converter_0_M_AXI_RRESP),
        .m_axi_rvalid(axi_protocol_converter_0_M_AXI_RVALID),
        .m_axi_wdata(axi_protocol_converter_0_M_AXI_WDATA),
        .m_axi_wready(axi_protocol_converter_0_M_AXI_WREADY),
        .m_axi_wstrb(axi_protocol_converter_0_M_AXI_WSTRB),
        .m_axi_wvalid(axi_protocol_converter_0_M_AXI_WVALID),
        .s_axi_araddr(processing_system7_0_M_AXI_GP1_ARADDR),
        .s_axi_arburst(processing_system7_0_M_AXI_GP1_ARBURST),
        .s_axi_arcache(processing_system7_0_M_AXI_GP1_ARCACHE),
        .s_axi_arid(processing_system7_0_M_AXI_GP1_ARID),
        .s_axi_arlen(processing_system7_0_M_AXI_GP1_ARLEN),
        .s_axi_arlock(processing_system7_0_M_AXI_GP1_ARLOCK),
        .s_axi_arprot(processing_system7_0_M_AXI_GP1_ARPROT),
        .s_axi_arqos(processing_system7_0_M_AXI_GP1_ARQOS),
        .s_axi_arready(processing_system7_0_M_AXI_GP1_ARREADY),
        .s_axi_arsize(processing_system7_0_M_AXI_GP1_ARSIZE),
        .s_axi_arvalid(processing_system7_0_M_AXI_GP1_ARVALID),
        .s_axi_awaddr(processing_system7_0_M_AXI_GP1_AWADDR),
        .s_axi_awburst(processing_system7_0_M_AXI_GP1_AWBURST),
        .s_axi_awcache(processing_system7_0_M_AXI_GP1_AWCACHE),
        .s_axi_awid(processing_system7_0_M_AXI_GP1_AWID),
        .s_axi_awlen(processing_system7_0_M_AXI_GP1_AWLEN),
        .s_axi_awlock(processing_system7_0_M_AXI_GP1_AWLOCK),
        .s_axi_awprot(processing_system7_0_M_AXI_GP1_AWPROT),
        .s_axi_awqos(processing_system7_0_M_AXI_GP1_AWQOS),
        .s_axi_awready(processing_system7_0_M_AXI_GP1_AWREADY),
        .s_axi_awsize(processing_system7_0_M_AXI_GP1_AWSIZE),
        .s_axi_awvalid(processing_system7_0_M_AXI_GP1_AWVALID),
        .s_axi_bid(processing_system7_0_M_AXI_GP1_BID),
        .s_axi_bready(processing_system7_0_M_AXI_GP1_BREADY),
        .s_axi_bresp(processing_system7_0_M_AXI_GP1_BRESP),
        .s_axi_bvalid(processing_system7_0_M_AXI_GP1_BVALID),
        .s_axi_rdata(processing_system7_0_M_AXI_GP1_RDATA),
        .s_axi_rid(processing_system7_0_M_AXI_GP1_RID),
        .s_axi_rlast(processing_system7_0_M_AXI_GP1_RLAST),
        .s_axi_rready(processing_system7_0_M_AXI_GP1_RREADY),
        .s_axi_rresp(processing_system7_0_M_AXI_GP1_RRESP),
        .s_axi_rvalid(processing_system7_0_M_AXI_GP1_RVALID),
        .s_axi_wdata(processing_system7_0_M_AXI_GP1_WDATA),
        .s_axi_wid(processing_system7_0_M_AXI_GP1_WID),
        .s_axi_wlast(processing_system7_0_M_AXI_GP1_WLAST),
        .s_axi_wready(processing_system7_0_M_AXI_GP1_WREADY),
        .s_axi_wstrb(processing_system7_0_M_AXI_GP1_WSTRB),
        .s_axi_wvalid(processing_system7_0_M_AXI_GP1_WVALID));
  system_proc_sys_reset_0 proc_sys_reset
       (.aux_reset_in(xlconstant_dout),
        .dcm_locked(VCC_1),
        .ext_reset_in(processing_system7_0_fclk_reset3_n),
        .interconnect_aresetn(proc_sys_reset_0_interconnect_aresetn),
        .mb_debug_sys_rst(GND_1),
        .peripheral_aresetn(proc_sys_reset_0_peripheral_aresetn),
        .slowest_sync_clk(processing_system7_0_fclk_clk3));
  system_processing_system7_0 processing_system7
       (.DDR_Addr(DDR_addr[14:0]),
        .DDR_BankAddr(DDR_ba[2:0]),
        .DDR_CAS_n(DDR_cas_n),
        .DDR_CKE(DDR_cke),
        .DDR_CS_n(DDR_cs_n),
        .DDR_Clk(DDR_ck_p),
        .DDR_Clk_n(DDR_ck_n),
        .DDR_DM(DDR_dm[3:0]),
        .DDR_DQ(DDR_dq[31:0]),
        .DDR_DQS(DDR_dqs_p[3:0]),
        .DDR_DQS_n(DDR_dqs_n[3:0]),
        .DDR_DRSTB(DDR_reset_n),
        .DDR_ODT(DDR_odt),
        .DDR_RAS_n(DDR_ras_n),
        .DDR_VRN(FIXED_IO_ddr_vrn),
        .DDR_VRP(FIXED_IO_ddr_vrp),
        .DDR_WEB(DDR_we_n),
        .FCLK_CLK0(processing_system7_0_fclk_clk0),
        .FCLK_CLK1(processing_system7_0_fclk_clk1),
        .FCLK_CLK2(processing_system7_0_fclk_clk2),
        .FCLK_CLK3(processing_system7_0_fclk_clk3),
        .FCLK_RESET0_N(processing_system7_0_fclk_reset0_n),
        .FCLK_RESET1_N(processing_system7_0_fclk_reset1_n),
        .FCLK_RESET2_N(processing_system7_0_fclk_reset2_n),
        .FCLK_RESET3_N(processing_system7_0_fclk_reset3_n),
        .IRQ_F2P(xadc_wiz_0_ip2intc_irpt),
        .MIO(FIXED_IO_mio[53:0]),
        .M_AXI_GP0_ACLK(m_axi_gp0_aclk_1),
        .M_AXI_GP0_ARADDR(processing_system7_0_M_AXI_GP0_ARADDR),
        .M_AXI_GP0_ARBURST(processing_system7_0_M_AXI_GP0_ARBURST),
        .M_AXI_GP0_ARCACHE(processing_system7_0_M_AXI_GP0_ARCACHE),
        .M_AXI_GP0_ARID(processing_system7_0_M_AXI_GP0_ARID),
        .M_AXI_GP0_ARLEN(processing_system7_0_M_AXI_GP0_ARLEN),
        .M_AXI_GP0_ARLOCK(processing_system7_0_M_AXI_GP0_ARLOCK),
        .M_AXI_GP0_ARPROT(processing_system7_0_M_AXI_GP0_ARPROT),
        .M_AXI_GP0_ARQOS(processing_system7_0_M_AXI_GP0_ARQOS),
        .M_AXI_GP0_ARREADY(processing_system7_0_M_AXI_GP0_ARREADY),
        .M_AXI_GP0_ARSIZE(processing_system7_0_M_AXI_GP0_ARSIZE),
        .M_AXI_GP0_ARVALID(processing_system7_0_M_AXI_GP0_ARVALID),
        .M_AXI_GP0_AWADDR(processing_system7_0_M_AXI_GP0_AWADDR),
        .M_AXI_GP0_AWBURST(processing_system7_0_M_AXI_GP0_AWBURST),
        .M_AXI_GP0_AWCACHE(processing_system7_0_M_AXI_GP0_AWCACHE),
        .M_AXI_GP0_AWID(processing_system7_0_M_AXI_GP0_AWID),
        .M_AXI_GP0_AWLEN(processing_system7_0_M_AXI_GP0_AWLEN),
        .M_AXI_GP0_AWLOCK(processing_system7_0_M_AXI_GP0_AWLOCK),
        .M_AXI_GP0_AWPROT(processing_system7_0_M_AXI_GP0_AWPROT),
        .M_AXI_GP0_AWQOS(processing_system7_0_M_AXI_GP0_AWQOS),
        .M_AXI_GP0_AWREADY(processing_system7_0_M_AXI_GP0_AWREADY),
        .M_AXI_GP0_AWSIZE(processing_system7_0_M_AXI_GP0_AWSIZE),
        .M_AXI_GP0_AWVALID(processing_system7_0_M_AXI_GP0_AWVALID),
        .M_AXI_GP0_BID(processing_system7_0_M_AXI_GP0_BID),
        .M_AXI_GP0_BREADY(processing_system7_0_M_AXI_GP0_BREADY),
        .M_AXI_GP0_BRESP(processing_system7_0_M_AXI_GP0_BRESP),
        .M_AXI_GP0_BVALID(processing_system7_0_M_AXI_GP0_BVALID),
        .M_AXI_GP0_RDATA(processing_system7_0_M_AXI_GP0_RDATA),
        .M_AXI_GP0_RID(processing_system7_0_M_AXI_GP0_RID),
        .M_AXI_GP0_RLAST(processing_system7_0_M_AXI_GP0_RLAST),
        .M_AXI_GP0_RREADY(processing_system7_0_M_AXI_GP0_RREADY),
        .M_AXI_GP0_RRESP(processing_system7_0_M_AXI_GP0_RRESP),
        .M_AXI_GP0_RVALID(processing_system7_0_M_AXI_GP0_RVALID),
        .M_AXI_GP0_WDATA(processing_system7_0_M_AXI_GP0_WDATA),
        .M_AXI_GP0_WID(processing_system7_0_M_AXI_GP0_WID),
        .M_AXI_GP0_WLAST(processing_system7_0_M_AXI_GP0_WLAST),
        .M_AXI_GP0_WREADY(processing_system7_0_M_AXI_GP0_WREADY),
        .M_AXI_GP0_WSTRB(processing_system7_0_M_AXI_GP0_WSTRB),
        .M_AXI_GP0_WVALID(processing_system7_0_M_AXI_GP0_WVALID),
        .M_AXI_GP1_ACLK(processing_system7_0_fclk_clk3),
        .M_AXI_GP1_ARADDR(processing_system7_0_M_AXI_GP1_ARADDR),
        .M_AXI_GP1_ARBURST(processing_system7_0_M_AXI_GP1_ARBURST),
        .M_AXI_GP1_ARCACHE(processing_system7_0_M_AXI_GP1_ARCACHE),
        .M_AXI_GP1_ARID(processing_system7_0_M_AXI_GP1_ARID),
        .M_AXI_GP1_ARLEN(processing_system7_0_M_AXI_GP1_ARLEN),
        .M_AXI_GP1_ARLOCK(processing_system7_0_M_AXI_GP1_ARLOCK),
        .M_AXI_GP1_ARPROT(processing_system7_0_M_AXI_GP1_ARPROT),
        .M_AXI_GP1_ARQOS(processing_system7_0_M_AXI_GP1_ARQOS),
        .M_AXI_GP1_ARREADY(processing_system7_0_M_AXI_GP1_ARREADY),
        .M_AXI_GP1_ARSIZE(processing_system7_0_M_AXI_GP1_ARSIZE),
        .M_AXI_GP1_ARVALID(processing_system7_0_M_AXI_GP1_ARVALID),
        .M_AXI_GP1_AWADDR(processing_system7_0_M_AXI_GP1_AWADDR),
        .M_AXI_GP1_AWBURST(processing_system7_0_M_AXI_GP1_AWBURST),
        .M_AXI_GP1_AWCACHE(processing_system7_0_M_AXI_GP1_AWCACHE),
        .M_AXI_GP1_AWID(processing_system7_0_M_AXI_GP1_AWID),
        .M_AXI_GP1_AWLEN(processing_system7_0_M_AXI_GP1_AWLEN),
        .M_AXI_GP1_AWLOCK(processing_system7_0_M_AXI_GP1_AWLOCK),
        .M_AXI_GP1_AWPROT(processing_system7_0_M_AXI_GP1_AWPROT),
        .M_AXI_GP1_AWQOS(processing_system7_0_M_AXI_GP1_AWQOS),
        .M_AXI_GP1_AWREADY(processing_system7_0_M_AXI_GP1_AWREADY),
        .M_AXI_GP1_AWSIZE(processing_system7_0_M_AXI_GP1_AWSIZE),
        .M_AXI_GP1_AWVALID(processing_system7_0_M_AXI_GP1_AWVALID),
        .M_AXI_GP1_BID(processing_system7_0_M_AXI_GP1_BID),
        .M_AXI_GP1_BREADY(processing_system7_0_M_AXI_GP1_BREADY),
        .M_AXI_GP1_BRESP(processing_system7_0_M_AXI_GP1_BRESP),
        .M_AXI_GP1_BVALID(processing_system7_0_M_AXI_GP1_BVALID),
        .M_AXI_GP1_RDATA(processing_system7_0_M_AXI_GP1_RDATA),
        .M_AXI_GP1_RID(processing_system7_0_M_AXI_GP1_RID),
        .M_AXI_GP1_RLAST(processing_system7_0_M_AXI_GP1_RLAST),
        .M_AXI_GP1_RREADY(processing_system7_0_M_AXI_GP1_RREADY),
        .M_AXI_GP1_RRESP(processing_system7_0_M_AXI_GP1_RRESP),
        .M_AXI_GP1_RVALID(processing_system7_0_M_AXI_GP1_RVALID),
        .M_AXI_GP1_WDATA(processing_system7_0_M_AXI_GP1_WDATA),
        .M_AXI_GP1_WID(processing_system7_0_M_AXI_GP1_WID),
        .M_AXI_GP1_WLAST(processing_system7_0_M_AXI_GP1_WLAST),
        .M_AXI_GP1_WREADY(processing_system7_0_M_AXI_GP1_WREADY),
        .M_AXI_GP1_WSTRB(processing_system7_0_M_AXI_GP1_WSTRB),
        .M_AXI_GP1_WVALID(processing_system7_0_M_AXI_GP1_WVALID),
        .PS_CLK(FIXED_IO_ps_clk),
        .PS_PORB(FIXED_IO_ps_porb),
        .PS_SRSTB(FIXED_IO_ps_srstb),
        .S_AXI_HP0_ACLK(s_axi_hp0_aclk),
        .S_AXI_HP0_ARADDR(s_axi_hp0_1_ARADDR),
        .S_AXI_HP0_ARBURST(s_axi_hp0_1_ARBURST),
        .S_AXI_HP0_ARCACHE(s_axi_hp0_1_ARCACHE),
        .S_AXI_HP0_ARID(s_axi_hp0_1_ARID),
        .S_AXI_HP0_ARLEN(s_axi_hp0_1_ARLEN),
        .S_AXI_HP0_ARLOCK(s_axi_hp0_1_ARLOCK),
        .S_AXI_HP0_ARPROT(s_axi_hp0_1_ARPROT),
        .S_AXI_HP0_ARQOS(s_axi_hp0_1_ARQOS),
        .S_AXI_HP0_ARREADY(s_axi_hp0_1_ARREADY),
        .S_AXI_HP0_ARSIZE(s_axi_hp0_1_ARSIZE),
        .S_AXI_HP0_ARVALID(s_axi_hp0_1_ARVALID),
        .S_AXI_HP0_AWADDR(s_axi_hp0_1_AWADDR),
        .S_AXI_HP0_AWBURST(s_axi_hp0_1_AWBURST),
        .S_AXI_HP0_AWCACHE(s_axi_hp0_1_AWCACHE),
        .S_AXI_HP0_AWID(s_axi_hp0_1_AWID),
        .S_AXI_HP0_AWLEN(s_axi_hp0_1_AWLEN),
        .S_AXI_HP0_AWLOCK(s_axi_hp0_1_AWLOCK),
        .S_AXI_HP0_AWPROT(s_axi_hp0_1_AWPROT),
        .S_AXI_HP0_AWQOS(s_axi_hp0_1_AWQOS),
        .S_AXI_HP0_AWREADY(s_axi_hp0_1_AWREADY),
        .S_AXI_HP0_AWSIZE(s_axi_hp0_1_AWSIZE),
        .S_AXI_HP0_AWVALID(s_axi_hp0_1_AWVALID),
        .S_AXI_HP0_BID(s_axi_hp0_1_BID),
        .S_AXI_HP0_BREADY(s_axi_hp0_1_BREADY),
        .S_AXI_HP0_BRESP(s_axi_hp0_1_BRESP),
        .S_AXI_HP0_BVALID(s_axi_hp0_1_BVALID),
        .S_AXI_HP0_RDATA(s_axi_hp0_1_RDATA),
        .S_AXI_HP0_RDISSUECAP1_EN(GND_1),
        .S_AXI_HP0_RID(s_axi_hp0_1_RID),
        .S_AXI_HP0_RLAST(s_axi_hp0_1_RLAST),
        .S_AXI_HP0_RREADY(s_axi_hp0_1_RREADY),
        .S_AXI_HP0_RRESP(s_axi_hp0_1_RRESP),
        .S_AXI_HP0_RVALID(s_axi_hp0_1_RVALID),
        .S_AXI_HP0_WDATA(s_axi_hp0_1_WDATA),
        .S_AXI_HP0_WID(s_axi_hp0_1_WID),
        .S_AXI_HP0_WLAST(s_axi_hp0_1_WLAST),
        .S_AXI_HP0_WREADY(s_axi_hp0_1_WREADY),
        .S_AXI_HP0_WRISSUECAP1_EN(GND_1),
        .S_AXI_HP0_WSTRB(s_axi_hp0_1_WSTRB),
        .S_AXI_HP0_WVALID(s_axi_hp0_1_WVALID),
        .S_AXI_HP1_ACLK(s_axi_hp1_aclk),
        .S_AXI_HP1_ARADDR(s_axi_hp1_1_ARADDR),
        .S_AXI_HP1_ARBURST(s_axi_hp1_1_ARBURST),
        .S_AXI_HP1_ARCACHE(s_axi_hp1_1_ARCACHE),
        .S_AXI_HP1_ARID(s_axi_hp1_1_ARID),
        .S_AXI_HP1_ARLEN(s_axi_hp1_1_ARLEN),
        .S_AXI_HP1_ARLOCK(s_axi_hp1_1_ARLOCK),
        .S_AXI_HP1_ARPROT(s_axi_hp1_1_ARPROT),
        .S_AXI_HP1_ARQOS(s_axi_hp1_1_ARQOS),
        .S_AXI_HP1_ARREADY(s_axi_hp1_1_ARREADY),
        .S_AXI_HP1_ARSIZE(s_axi_hp1_1_ARSIZE),
        .S_AXI_HP1_ARVALID(s_axi_hp1_1_ARVALID),
        .S_AXI_HP1_AWADDR(s_axi_hp1_1_AWADDR),
        .S_AXI_HP1_AWBURST(s_axi_hp1_1_AWBURST),
        .S_AXI_HP1_AWCACHE(s_axi_hp1_1_AWCACHE),
        .S_AXI_HP1_AWID(s_axi_hp1_1_AWID),
        .S_AXI_HP1_AWLEN(s_axi_hp1_1_AWLEN),
        .S_AXI_HP1_AWLOCK(s_axi_hp1_1_AWLOCK),
        .S_AXI_HP1_AWPROT(s_axi_hp1_1_AWPROT),
        .S_AXI_HP1_AWQOS(s_axi_hp1_1_AWQOS),
        .S_AXI_HP1_AWREADY(s_axi_hp1_1_AWREADY),
        .S_AXI_HP1_AWSIZE(s_axi_hp1_1_AWSIZE),
        .S_AXI_HP1_AWVALID(s_axi_hp1_1_AWVALID),
        .S_AXI_HP1_BID(s_axi_hp1_1_BID),
        .S_AXI_HP1_BREADY(s_axi_hp1_1_BREADY),
        .S_AXI_HP1_BRESP(s_axi_hp1_1_BRESP),
        .S_AXI_HP1_BVALID(s_axi_hp1_1_BVALID),
        .S_AXI_HP1_RDATA(s_axi_hp1_1_RDATA),
        .S_AXI_HP1_RDISSUECAP1_EN(GND_1),
        .S_AXI_HP1_RID(s_axi_hp1_1_RID),
        .S_AXI_HP1_RLAST(s_axi_hp1_1_RLAST),
        .S_AXI_HP1_RREADY(s_axi_hp1_1_RREADY),
        .S_AXI_HP1_RRESP(s_axi_hp1_1_RRESP),
        .S_AXI_HP1_RVALID(s_axi_hp1_1_RVALID),
        .S_AXI_HP1_WDATA(s_axi_hp1_1_WDATA),
        .S_AXI_HP1_WID(s_axi_hp1_1_WID),
        .S_AXI_HP1_WLAST(s_axi_hp1_1_WLAST),
        .S_AXI_HP1_WREADY(s_axi_hp1_1_WREADY),
        .S_AXI_HP1_WRISSUECAP1_EN(GND_1),
        .S_AXI_HP1_WSTRB(s_axi_hp1_1_WSTRB),
        .S_AXI_HP1_WVALID(s_axi_hp1_1_WVALID),
        .USB0_VBUS_PWRFAULT(GND_1));
  system_xadc_0 xadc
       (.ip2intc_irpt(xadc_wiz_0_ip2intc_irpt),
        .s_axi_aclk(processing_system7_0_fclk_clk3),
        .s_axi_araddr(axi_protocol_converter_0_M_AXI_ARADDR[10:0]),
        .s_axi_aresetn(proc_sys_reset_0_peripheral_aresetn),
        .s_axi_arready(axi_protocol_converter_0_M_AXI_ARREADY),
        .s_axi_arvalid(axi_protocol_converter_0_M_AXI_ARVALID),
        .s_axi_awaddr(axi_protocol_converter_0_M_AXI_AWADDR[10:0]),
        .s_axi_awready(axi_protocol_converter_0_M_AXI_AWREADY),
        .s_axi_awvalid(axi_protocol_converter_0_M_AXI_AWVALID),
        .s_axi_bready(axi_protocol_converter_0_M_AXI_BREADY),
        .s_axi_bresp(axi_protocol_converter_0_M_AXI_BRESP),
        .s_axi_bvalid(axi_protocol_converter_0_M_AXI_BVALID),
        .s_axi_rdata(axi_protocol_converter_0_M_AXI_RDATA),
        .s_axi_rready(axi_protocol_converter_0_M_AXI_RREADY),
        .s_axi_rresp(axi_protocol_converter_0_M_AXI_RRESP),
        .s_axi_rvalid(axi_protocol_converter_0_M_AXI_RVALID),
        .s_axi_wdata(axi_protocol_converter_0_M_AXI_WDATA),
        .s_axi_wready(axi_protocol_converter_0_M_AXI_WREADY),
        .s_axi_wstrb(axi_protocol_converter_0_M_AXI_WSTRB),
        .s_axi_wvalid(axi_protocol_converter_0_M_AXI_WVALID),
        .vauxn0(Vaux0_1_V_N),
        .vauxn1(Vaux1_1_V_N),
        .vauxn8(Vaux8_1_V_N),
        .vauxn9(Vaux9_1_V_N),
        .vauxp0(Vaux0_1_V_P),
        .vauxp1(Vaux1_1_V_P),
        .vauxp8(Vaux8_1_V_P),
        .vauxp9(Vaux9_1_V_P),
        .vn_in(Vp_Vn_1_V_N),
        .vp_in(Vp_Vn_1_V_P));
  system_xlconstant_0 xlconstant
       (.dout(xlconstant_dout));
endmodule
