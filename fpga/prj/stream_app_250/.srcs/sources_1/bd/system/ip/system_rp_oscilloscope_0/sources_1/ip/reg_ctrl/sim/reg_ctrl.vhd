-- (c) Copyright 1995-2020 Xilinx, Inc. All rights reserved.
-- 
-- This file contains confidential and proprietary information
-- of Xilinx, Inc. and is protected under U.S. and
-- international copyright and other intellectual property
-- laws.
-- 
-- DISCLAIMER
-- This disclaimer is not a license and does not grant any
-- rights to the materials distributed herewith. Except as
-- otherwise provided in a valid license issued to you by
-- Xilinx, and to the maximum extent permitted by applicable
-- law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
-- WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
-- AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
-- BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
-- INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
-- (2) Xilinx shall not be liable (whether in contract or tort,
-- including negligence, or under any other theory of
-- liability) for any loss or damage of any kind or nature
-- related to, arising under or in connection with these
-- materials, including for any direct, or any indirect,
-- special, incidental, or consequential loss or damage
-- (including loss of data, profits, goodwill, or any type of
-- loss or damage suffered as a result of any action brought
-- by a third party) even if such damage or loss was
-- reasonably foreseeable or Xilinx had been advised of the
-- possibility of the same.
-- 
-- CRITICAL APPLICATIONS
-- Xilinx products are not designed or intended to be fail-
-- safe, or for use in any application requiring fail-safe
-- performance, such as life-support or safety devices or
-- systems, Class III medical devices, nuclear facilities,
-- applications related to the deployment of airbags, or any
-- other applications that could lead to death, personal
-- injury, or severe property or environmental damage
-- (individually and collectively, "Critical
-- Applications"). Customer assumes the sole risk and
-- liability of any use of Xilinx products in Critical
-- Applications, subject only to applicable laws and
-- regulations governing limitations on product liability.
-- 
-- THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
-- PART OF THIS FILE AT ALL TIMES.
-- 
-- DO NOT MODIFY THIS FILE.

-- IP VLNV: xilinx.com:ip:axi_bram_ctrl:4.1
-- IP Revision: 3

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

LIBRARY axi_bram_ctrl_v4_1_3;
USE axi_bram_ctrl_v4_1_3.axi_bram_ctrl;

ENTITY reg_ctrl IS
  PORT (
    s_axi_aclk : IN STD_LOGIC;
    s_axi_aresetn : IN STD_LOGIC;
    s_axi_awaddr : IN STD_LOGIC_VECTOR(11 DOWNTO 0);
    s_axi_awprot : IN STD_LOGIC_VECTOR(2 DOWNTO 0);
    s_axi_awvalid : IN STD_LOGIC;
    s_axi_awready : OUT STD_LOGIC;
    s_axi_wdata : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
    s_axi_wstrb : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
    s_axi_wvalid : IN STD_LOGIC;
    s_axi_wready : OUT STD_LOGIC;
    s_axi_bresp : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
    s_axi_bvalid : OUT STD_LOGIC;
    s_axi_bready : IN STD_LOGIC;
    s_axi_araddr : IN STD_LOGIC_VECTOR(11 DOWNTO 0);
    s_axi_arprot : IN STD_LOGIC_VECTOR(2 DOWNTO 0);
    s_axi_arvalid : IN STD_LOGIC;
    s_axi_arready : OUT STD_LOGIC;
    s_axi_rdata : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
    s_axi_rresp : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
    s_axi_rvalid : OUT STD_LOGIC;
    s_axi_rready : IN STD_LOGIC;
    bram_rst_a : OUT STD_LOGIC;
    bram_clk_a : OUT STD_LOGIC;
    bram_en_a : OUT STD_LOGIC;
    bram_we_a : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
    bram_addr_a : OUT STD_LOGIC_VECTOR(11 DOWNTO 0);
    bram_wrdata_a : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
    bram_rddata_a : IN STD_LOGIC_VECTOR(31 DOWNTO 0)
  );
END reg_ctrl;

ARCHITECTURE reg_ctrl_arch OF reg_ctrl IS
  ATTRIBUTE DowngradeIPIdentifiedWarnings : STRING;
  ATTRIBUTE DowngradeIPIdentifiedWarnings OF reg_ctrl_arch: ARCHITECTURE IS "yes";
  COMPONENT axi_bram_ctrl IS
    GENERIC (
      C_BRAM_INST_MODE : STRING;
      C_MEMORY_DEPTH : INTEGER;
      C_BRAM_ADDR_WIDTH : INTEGER;
      C_S_AXI_ADDR_WIDTH : INTEGER;
      C_S_AXI_DATA_WIDTH : INTEGER;
      C_S_AXI_ID_WIDTH : INTEGER;
      C_S_AXI_PROTOCOL : STRING;
      C_S_AXI_SUPPORTS_NARROW_BURST : INTEGER;
      C_SINGLE_PORT_BRAM : INTEGER;
      C_FAMILY : STRING;
      C_READ_LATENCY : INTEGER;
      C_RD_CMD_OPTIMIZATION : INTEGER;
      C_S_AXI_CTRL_ADDR_WIDTH : INTEGER;
      C_S_AXI_CTRL_DATA_WIDTH : INTEGER;
      C_ECC : INTEGER;
      C_ECC_TYPE : INTEGER;
      C_FAULT_INJECT : INTEGER;
      C_ECC_ONOFF_RESET_VALUE : INTEGER
    );
    PORT (
      s_axi_aclk : IN STD_LOGIC;
      s_axi_aresetn : IN STD_LOGIC;
      ecc_interrupt : OUT STD_LOGIC;
      ecc_ue : OUT STD_LOGIC;
      s_axi_awid : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
      s_axi_awaddr : IN STD_LOGIC_VECTOR(11 DOWNTO 0);
      s_axi_awlen : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
      s_axi_awsize : IN STD_LOGIC_VECTOR(2 DOWNTO 0);
      s_axi_awburst : IN STD_LOGIC_VECTOR(1 DOWNTO 0);
      s_axi_awlock : IN STD_LOGIC;
      s_axi_awcache : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
      s_axi_awprot : IN STD_LOGIC_VECTOR(2 DOWNTO 0);
      s_axi_awvalid : IN STD_LOGIC;
      s_axi_awready : OUT STD_LOGIC;
      s_axi_wdata : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
      s_axi_wstrb : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
      s_axi_wlast : IN STD_LOGIC;
      s_axi_wvalid : IN STD_LOGIC;
      s_axi_wready : OUT STD_LOGIC;
      s_axi_bid : OUT STD_LOGIC_VECTOR(0 DOWNTO 0);
      s_axi_bresp : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
      s_axi_bvalid : OUT STD_LOGIC;
      s_axi_bready : IN STD_LOGIC;
      s_axi_arid : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
      s_axi_araddr : IN STD_LOGIC_VECTOR(11 DOWNTO 0);
      s_axi_arlen : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
      s_axi_arsize : IN STD_LOGIC_VECTOR(2 DOWNTO 0);
      s_axi_arburst : IN STD_LOGIC_VECTOR(1 DOWNTO 0);
      s_axi_arlock : IN STD_LOGIC;
      s_axi_arcache : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
      s_axi_arprot : IN STD_LOGIC_VECTOR(2 DOWNTO 0);
      s_axi_arvalid : IN STD_LOGIC;
      s_axi_arready : OUT STD_LOGIC;
      s_axi_rid : OUT STD_LOGIC_VECTOR(0 DOWNTO 0);
      s_axi_rdata : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
      s_axi_rresp : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
      s_axi_rlast : OUT STD_LOGIC;
      s_axi_rvalid : OUT STD_LOGIC;
      s_axi_rready : IN STD_LOGIC;
      s_axi_ctrl_awvalid : IN STD_LOGIC;
      s_axi_ctrl_awready : OUT STD_LOGIC;
      s_axi_ctrl_awaddr : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
      s_axi_ctrl_wdata : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
      s_axi_ctrl_wvalid : IN STD_LOGIC;
      s_axi_ctrl_wready : OUT STD_LOGIC;
      s_axi_ctrl_bresp : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
      s_axi_ctrl_bvalid : OUT STD_LOGIC;
      s_axi_ctrl_bready : IN STD_LOGIC;
      s_axi_ctrl_araddr : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
      s_axi_ctrl_arvalid : IN STD_LOGIC;
      s_axi_ctrl_arready : OUT STD_LOGIC;
      s_axi_ctrl_rdata : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
      s_axi_ctrl_rresp : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
      s_axi_ctrl_rvalid : OUT STD_LOGIC;
      s_axi_ctrl_rready : IN STD_LOGIC;
      bram_rst_a : OUT STD_LOGIC;
      bram_clk_a : OUT STD_LOGIC;
      bram_en_a : OUT STD_LOGIC;
      bram_we_a : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
      bram_addr_a : OUT STD_LOGIC_VECTOR(11 DOWNTO 0);
      bram_wrdata_a : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
      bram_rddata_a : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
      bram_rst_b : OUT STD_LOGIC;
      bram_clk_b : OUT STD_LOGIC;
      bram_en_b : OUT STD_LOGIC;
      bram_we_b : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
      bram_addr_b : OUT STD_LOGIC_VECTOR(11 DOWNTO 0);
      bram_wrdata_b : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
      bram_rddata_b : IN STD_LOGIC_VECTOR(31 DOWNTO 0)
    );
  END COMPONENT axi_bram_ctrl;
  ATTRIBUTE X_INTERFACE_INFO : STRING;
  ATTRIBUTE X_INTERFACE_PARAMETER : STRING;
  ATTRIBUTE X_INTERFACE_INFO OF bram_rddata_a: SIGNAL IS "xilinx.com:interface:bram:1.0 BRAM_PORTA DOUT";
  ATTRIBUTE X_INTERFACE_INFO OF bram_wrdata_a: SIGNAL IS "xilinx.com:interface:bram:1.0 BRAM_PORTA DIN";
  ATTRIBUTE X_INTERFACE_INFO OF bram_addr_a: SIGNAL IS "xilinx.com:interface:bram:1.0 BRAM_PORTA ADDR";
  ATTRIBUTE X_INTERFACE_INFO OF bram_we_a: SIGNAL IS "xilinx.com:interface:bram:1.0 BRAM_PORTA WE";
  ATTRIBUTE X_INTERFACE_INFO OF bram_en_a: SIGNAL IS "xilinx.com:interface:bram:1.0 BRAM_PORTA EN";
  ATTRIBUTE X_INTERFACE_INFO OF bram_clk_a: SIGNAL IS "xilinx.com:interface:bram:1.0 BRAM_PORTA CLK";
  ATTRIBUTE X_INTERFACE_PARAMETER OF bram_rst_a: SIGNAL IS "XIL_INTERFACENAME BRAM_PORTA, MASTER_TYPE BRAM_CTRL, MEM_SIZE 8192, MEM_WIDTH 32, MEM_ECC NONE, READ_LATENCY 1";
  ATTRIBUTE X_INTERFACE_INFO OF bram_rst_a: SIGNAL IS "xilinx.com:interface:bram:1.0 BRAM_PORTA RST";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_rready: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI RREADY";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_rvalid: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI RVALID";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_rresp: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI RRESP";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_rdata: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI RDATA";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_arready: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI ARREADY";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_arvalid: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI ARVALID";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_arprot: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI ARPROT";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_araddr: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI ARADDR";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_bready: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI BREADY";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_bvalid: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI BVALID";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_bresp: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI BRESP";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_wready: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI WREADY";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_wvalid: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI WVALID";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_wstrb: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI WSTRB";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_wdata: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI WDATA";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_awready: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI AWREADY";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_awvalid: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI AWVALID";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_awprot: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI AWPROT";
  ATTRIBUTE X_INTERFACE_PARAMETER OF s_axi_awaddr: SIGNAL IS "XIL_INTERFACENAME S_AXI, DATA_WIDTH 32, PROTOCOL AXI4LITE, FREQ_HZ 100000000, ID_WIDTH 0, ADDR_WIDTH 12, AWUSER_WIDTH 0, ARUSER_WIDTH 0, WUSER_WIDTH 0, RUSER_WIDTH 0, BUSER_WIDTH 0, READ_WRITE_MODE READ_WRITE, HAS_BURST 0, HAS_LOCK 0, HAS_PROT 1, HAS_CACHE 0, HAS_QOS 0, HAS_REGION 0, HAS_WSTRB 1, HAS_BRESP 1, HAS_RRESP 1, SUPPORTS_NARROW_BURST 0, NUM_READ_OUTSTANDING 1, NUM_WRITE_OUTSTANDING 1, MAX_BURST_LENGTH 1, PHASE 0.000, NUM_READ_THREADS 1, NUM_WRITE_THREADS 1, RUSER_BITS_PER_BYTE 0, WUSER" & 
"_BITS_PER_BYTE 0, INSERT_VIP 0";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_awaddr: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI AWADDR";
  ATTRIBUTE X_INTERFACE_PARAMETER OF s_axi_aresetn: SIGNAL IS "XIL_INTERFACENAME RSTIF, POLARITY ACTIVE_LOW, INSERT_VIP 0";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_aresetn: SIGNAL IS "xilinx.com:signal:reset:1.0 RSTIF RST";
  ATTRIBUTE X_INTERFACE_PARAMETER OF s_axi_aclk: SIGNAL IS "XIL_INTERFACENAME CLKIF, ASSOCIATED_BUSIF S_AXI:S_AXI_CTRL, ASSOCIATED_RESET s_axi_aresetn, FREQ_HZ 100000000, FREQ_TOLERANCE_HZ 0, PHASE 0.000, INSERT_VIP 0";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_aclk: SIGNAL IS "xilinx.com:signal:clock:1.0 CLKIF CLK";
BEGIN
  U0 : axi_bram_ctrl
    GENERIC MAP (
      C_BRAM_INST_MODE => "EXTERNAL",
      C_MEMORY_DEPTH => 1024,
      C_BRAM_ADDR_WIDTH => 10,
      C_S_AXI_ADDR_WIDTH => 12,
      C_S_AXI_DATA_WIDTH => 32,
      C_S_AXI_ID_WIDTH => 1,
      C_S_AXI_PROTOCOL => "AXI4LITE",
      C_S_AXI_SUPPORTS_NARROW_BURST => 0,
      C_SINGLE_PORT_BRAM => 1,
      C_FAMILY => "zynq",
      C_READ_LATENCY => 1,
      C_RD_CMD_OPTIMIZATION => 0,
      C_S_AXI_CTRL_ADDR_WIDTH => 32,
      C_S_AXI_CTRL_DATA_WIDTH => 32,
      C_ECC => 0,
      C_ECC_TYPE => 0,
      C_FAULT_INJECT => 0,
      C_ECC_ONOFF_RESET_VALUE => 0
    )
    PORT MAP (
      s_axi_aclk => s_axi_aclk,
      s_axi_aresetn => s_axi_aresetn,
      s_axi_awid => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 1)),
      s_axi_awaddr => s_axi_awaddr,
      s_axi_awlen => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 8)),
      s_axi_awsize => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 3)),
      s_axi_awburst => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 2)),
      s_axi_awlock => '0',
      s_axi_awcache => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 4)),
      s_axi_awprot => s_axi_awprot,
      s_axi_awvalid => s_axi_awvalid,
      s_axi_awready => s_axi_awready,
      s_axi_wdata => s_axi_wdata,
      s_axi_wstrb => s_axi_wstrb,
      s_axi_wlast => '0',
      s_axi_wvalid => s_axi_wvalid,
      s_axi_wready => s_axi_wready,
      s_axi_bresp => s_axi_bresp,
      s_axi_bvalid => s_axi_bvalid,
      s_axi_bready => s_axi_bready,
      s_axi_arid => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 1)),
      s_axi_araddr => s_axi_araddr,
      s_axi_arlen => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 8)),
      s_axi_arsize => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 3)),
      s_axi_arburst => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 2)),
      s_axi_arlock => '0',
      s_axi_arcache => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 4)),
      s_axi_arprot => s_axi_arprot,
      s_axi_arvalid => s_axi_arvalid,
      s_axi_arready => s_axi_arready,
      s_axi_rdata => s_axi_rdata,
      s_axi_rresp => s_axi_rresp,
      s_axi_rvalid => s_axi_rvalid,
      s_axi_rready => s_axi_rready,
      s_axi_ctrl_awvalid => '0',
      s_axi_ctrl_awaddr => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 32)),
      s_axi_ctrl_wdata => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 32)),
      s_axi_ctrl_wvalid => '0',
      s_axi_ctrl_bready => '0',
      s_axi_ctrl_araddr => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 32)),
      s_axi_ctrl_arvalid => '0',
      s_axi_ctrl_rready => '0',
      bram_rst_a => bram_rst_a,
      bram_clk_a => bram_clk_a,
      bram_en_a => bram_en_a,
      bram_we_a => bram_we_a,
      bram_addr_a => bram_addr_a,
      bram_wrdata_a => bram_wrdata_a,
      bram_rddata_a => bram_rddata_a,
      bram_rddata_b => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 32))
    );
END reg_ctrl_arch;
