// (c) Copyright 1995-2020 Xilinx, Inc. All rights reserved.
// 
// This file contains confidential and proprietary information
// of Xilinx, Inc. and is protected under U.S. and
// international copyright and other intellectual property
// laws.
// 
// DISCLAIMER
// This disclaimer is not a license and does not grant any
// rights to the materials distributed herewith. Except as
// otherwise provided in a valid license issued to you by
// Xilinx, and to the maximum extent permitted by applicable
// law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
// WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
// AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
// BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
// INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
// (2) Xilinx shall not be liable (whether in contract or tort,
// including negligence, or under any other theory of
// liability) for any loss or damage of any kind or nature
// related to, arising under or in connection with these
// materials, including for any direct, or any indirect,
// special, incidental, or consequential loss or damage
// (including loss of data, profits, goodwill, or any type of
// loss or damage suffered as a result of any action brought
// by a third party) even if such damage or loss was
// reasonably foreseeable or Xilinx had been advised of the
// possibility of the same.
// 
// CRITICAL APPLICATIONS
// Xilinx products are not designed or intended to be fail-
// safe, or for use in any application requiring fail-safe
// performance, such as life-support or safety devices or
// systems, Class III medical devices, nuclear facilities,
// applications related to the deployment of airbags, or any
// other applications that could lead to death, personal
// injury, or severe property or environmental damage
// (individually and collectively, "Critical
// Applications"). Customer assumes the sole risk and
// liability of any use of Xilinx products in Critical
// Applications, subject only to applicable laws and
// regulations governing limitations on product liability.
// 
// THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
// PART OF THIS FILE AT ALL TIMES.
// 
// DO NOT MODIFY THIS FILE.


// IP VLNV: redpitaya.com:user:rp_oscilloscope:1.16
// IP Revision: 36

`timescale 1ns/1ps

(* IP_DEFINITION_SOURCE = "package_project" *)
(* DowngradeIPIdentifiedWarnings = "yes" *)
module system_rp_oscilloscope_0 (
  clk,
  rst_n,
  intr,
  adc_data_ch1,
  adc_data_ch2,
  event_ip_trig,
  event_ip_stop,
  event_ip_start,
  event_ip_reset,
  trig_ip,
  osc1_event_op,
  osc1_trig_op,
  osc2_event_op,
  osc2_trig_op,
  s_axi_reg_aclk,
  s_axi_reg_aresetn,
  s_axi_reg_awaddr,
  s_axi_reg_awprot,
  s_axi_reg_awvalid,
  s_axi_reg_awready,
  s_axi_reg_wdata,
  s_axi_reg_wstrb,
  s_axi_reg_wvalid,
  s_axi_reg_wready,
  s_axi_reg_bresp,
  s_axi_reg_bvalid,
  s_axi_reg_bready,
  s_axi_reg_araddr,
  s_axi_reg_arprot,
  s_axi_reg_arvalid,
  s_axi_reg_arready,
  s_axi_reg_rdata,
  s_axi_reg_rresp,
  s_axi_reg_rvalid,
  s_axi_reg_rready,
  m_axi_osc1_aclk,
  m_axi_osc1_aresetn,
  m_axi_osc1_awaddr,
  m_axi_osc1_awlen,
  m_axi_osc1_awsize,
  m_axi_osc1_awburst,
  m_axi_osc1_awprot,
  m_axi_osc1_awcache,
  m_axi_osc1_awvalid,
  m_axi_osc1_awready,
  m_axi_osc1_wdata,
  m_axi_osc1_wstrb,
  m_axi_osc1_wlast,
  m_axi_osc1_wvalid,
  m_axi_osc1_wready,
  m_axi_osc1_bresp,
  m_axi_osc1_bvalid,
  m_axi_osc1_bready,
  m_axi_osc2_aclk,
  m_axi_osc2_aresetn,
  m_axi_osc2_awaddr,
  m_axi_osc2_awlen,
  m_axi_osc2_awsize,
  m_axi_osc2_awburst,
  m_axi_osc2_awprot,
  m_axi_osc2_awcache,
  m_axi_osc2_awvalid,
  m_axi_osc2_awready,
  m_axi_osc2_wdata,
  m_axi_osc2_wstrb,
  m_axi_osc2_wlast,
  m_axi_osc2_wvalid,
  m_axi_osc2_wready,
  m_axi_osc2_bresp,
  m_axi_osc2_bvalid,
  m_axi_osc2_bready
);

(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME clk, FREQ_HZ 125000000, FREQ_TOLERANCE_HZ 0, PHASE 0.0, CLK_DOMAIN system_clk_gen_0_clk_125, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 clk CLK" *)
input wire clk;
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME rst_n, POLARITY ACTIVE_LOW, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 rst_n RST" *)
input wire rst_n;
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME intr, SENSITIVITY LEVEL_HIGH, PortWidth 1" *)
(* X_INTERFACE_INFO = "xilinx.com:signal:interrupt:1.0 intr INTERRUPT" *)
output wire intr;
input wire [13 : 0] adc_data_ch1;
input wire [13 : 0] adc_data_ch2;
input wire [4 : 0] event_ip_trig;
input wire [4 : 0] event_ip_stop;
input wire [4 : 0] event_ip_start;
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME event_ip_reset, POLARITY ACTIVE_LOW, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 event_ip_reset RST" *)
input wire [4 : 0] event_ip_reset;
input wire [4 : 0] trig_ip;
output wire [3 : 0] osc1_event_op;
output wire osc1_trig_op;
output wire [3 : 0] osc2_event_op;
output wire osc2_trig_op;
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME s_axi_reg_aclk, ASSOCIATED_BUSIF s_axi_reg, ASSOCIATED_RESET s_axi_reg_aresetn, FREQ_HZ 125000000, FREQ_TOLERANCE_HZ 0, PHASE 0.0, CLK_DOMAIN system_clk_gen_0_clk_125, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 s_axi_reg_aclk CLK" *)
input wire s_axi_reg_aclk;
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME s_axi_reg_aresetn, POLARITY ACTIVE_LOW, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 s_axi_reg_aresetn RST" *)
input wire s_axi_reg_aresetn;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg AWADDR" *)
input wire [11 : 0] s_axi_reg_awaddr;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg AWPROT" *)
input wire [2 : 0] s_axi_reg_awprot;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg AWVALID" *)
input wire s_axi_reg_awvalid;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg AWREADY" *)
output wire s_axi_reg_awready;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg WDATA" *)
input wire [31 : 0] s_axi_reg_wdata;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg WSTRB" *)
input wire [3 : 0] s_axi_reg_wstrb;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg WVALID" *)
input wire s_axi_reg_wvalid;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg WREADY" *)
output wire s_axi_reg_wready;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg BRESP" *)
output wire [1 : 0] s_axi_reg_bresp;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg BVALID" *)
output wire s_axi_reg_bvalid;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg BREADY" *)
input wire s_axi_reg_bready;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg ARADDR" *)
input wire [11 : 0] s_axi_reg_araddr;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg ARPROT" *)
input wire [2 : 0] s_axi_reg_arprot;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg ARVALID" *)
input wire s_axi_reg_arvalid;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg ARREADY" *)
output wire s_axi_reg_arready;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg RDATA" *)
output wire [31 : 0] s_axi_reg_rdata;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg RRESP" *)
output wire [1 : 0] s_axi_reg_rresp;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg RVALID" *)
output wire s_axi_reg_rvalid;
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME s_axi_reg, DATA_WIDTH 32, PROTOCOL AXI4LITE, FREQ_HZ 125000000, ID_WIDTH 0, ADDR_WIDTH 12, AWUSER_WIDTH 0, ARUSER_WIDTH 0, WUSER_WIDTH 0, RUSER_WIDTH 0, BUSER_WIDTH 0, READ_WRITE_MODE READ_WRITE, HAS_BURST 0, HAS_LOCK 0, HAS_PROT 1, HAS_CACHE 0, HAS_QOS 0, HAS_REGION 0, HAS_WSTRB 1, HAS_BRESP 1, HAS_RRESP 1, SUPPORTS_NARROW_BURST 0, NUM_READ_OUTSTANDING 1, NUM_WRITE_OUTSTANDING 1, MAX_BURST_LENGTH 1, PHASE 0.0, CLK_DOMAIN system_clk_gen_0_clk_125, NUM_READ_THREADS 4, NUM_WRITE_\
THREADS 4, RUSER_BITS_PER_BYTE 0, WUSER_BITS_PER_BYTE 0, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_reg RREADY" *)
input wire s_axi_reg_rready;
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME m_axi_osc1_aclk, ASSOCIATED_BUSIF m_axi_osc1, ASSOCIATED_RESET m_axi_osc1_aresetn, FREQ_HZ 125000000, FREQ_TOLERANCE_HZ 0, PHASE 0.0, CLK_DOMAIN system_clk_gen_0_clk_125, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 m_axi_osc1_aclk CLK" *)
input wire m_axi_osc1_aclk;
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME m_axi_osc1_aresetn, POLARITY ACTIVE_LOW, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 m_axi_osc1_aresetn RST" *)
input wire m_axi_osc1_aresetn;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc1 AWADDR" *)
output wire [31 : 0] m_axi_osc1_awaddr;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc1 AWLEN" *)
output wire [7 : 0] m_axi_osc1_awlen;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc1 AWSIZE" *)
output wire [2 : 0] m_axi_osc1_awsize;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc1 AWBURST" *)
output wire [1 : 0] m_axi_osc1_awburst;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc1 AWPROT" *)
output wire [2 : 0] m_axi_osc1_awprot;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc1 AWCACHE" *)
output wire [3 : 0] m_axi_osc1_awcache;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc1 AWVALID" *)
output wire m_axi_osc1_awvalid;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc1 AWREADY" *)
input wire m_axi_osc1_awready;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc1 WDATA" *)
output wire [63 : 0] m_axi_osc1_wdata;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc1 WSTRB" *)
output wire [7 : 0] m_axi_osc1_wstrb;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc1 WLAST" *)
output wire m_axi_osc1_wlast;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc1 WVALID" *)
output wire m_axi_osc1_wvalid;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc1 WREADY" *)
input wire m_axi_osc1_wready;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc1 BRESP" *)
input wire [1 : 0] m_axi_osc1_bresp;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc1 BVALID" *)
input wire m_axi_osc1_bvalid;
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME m_axi_osc1, DATA_WIDTH 64, PROTOCOL AXI4, FREQ_HZ 125000000, ID_WIDTH 0, ADDR_WIDTH 32, AWUSER_WIDTH 0, ARUSER_WIDTH 0, WUSER_WIDTH 0, RUSER_WIDTH 0, BUSER_WIDTH 0, READ_WRITE_MODE WRITE_ONLY, HAS_BURST 1, HAS_LOCK 0, HAS_PROT 1, HAS_CACHE 1, HAS_QOS 0, HAS_REGION 0, HAS_WSTRB 1, HAS_BRESP 1, HAS_RRESP 0, SUPPORTS_NARROW_BURST 1, NUM_READ_OUTSTANDING 2, NUM_WRITE_OUTSTANDING 2, MAX_BURST_LENGTH 256, PHASE 0.0, CLK_DOMAIN system_clk_gen_0_clk_125, NUM_READ_THREADS 1, NUM_WRITE_T\
HREADS 1, RUSER_BITS_PER_BYTE 0, WUSER_BITS_PER_BYTE 0, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc1 BREADY" *)
output wire m_axi_osc1_bready;
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME m_axi_osc2_aclk, ASSOCIATED_BUSIF m_axi_osc2, ASSOCIATED_RESET m_axi_osc2_aresetn, FREQ_HZ 125000000, FREQ_TOLERANCE_HZ 0, PHASE 0.0, CLK_DOMAIN system_clk_gen_0_clk_125, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 m_axi_osc2_aclk CLK" *)
input wire m_axi_osc2_aclk;
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME m_axi_osc2_aresetn, POLARITY ACTIVE_LOW, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 m_axi_osc2_aresetn RST" *)
input wire m_axi_osc2_aresetn;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc2 AWADDR" *)
output wire [31 : 0] m_axi_osc2_awaddr;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc2 AWLEN" *)
output wire [7 : 0] m_axi_osc2_awlen;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc2 AWSIZE" *)
output wire [2 : 0] m_axi_osc2_awsize;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc2 AWBURST" *)
output wire [1 : 0] m_axi_osc2_awburst;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc2 AWPROT" *)
output wire [2 : 0] m_axi_osc2_awprot;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc2 AWCACHE" *)
output wire [3 : 0] m_axi_osc2_awcache;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc2 AWVALID" *)
output wire m_axi_osc2_awvalid;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc2 AWREADY" *)
input wire m_axi_osc2_awready;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc2 WDATA" *)
output wire [63 : 0] m_axi_osc2_wdata;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc2 WSTRB" *)
output wire [7 : 0] m_axi_osc2_wstrb;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc2 WLAST" *)
output wire m_axi_osc2_wlast;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc2 WVALID" *)
output wire m_axi_osc2_wvalid;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc2 WREADY" *)
input wire m_axi_osc2_wready;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc2 BRESP" *)
input wire [1 : 0] m_axi_osc2_bresp;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc2 BVALID" *)
input wire m_axi_osc2_bvalid;
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME m_axi_osc2, DATA_WIDTH 64, PROTOCOL AXI4, FREQ_HZ 125000000, ID_WIDTH 0, ADDR_WIDTH 32, AWUSER_WIDTH 0, ARUSER_WIDTH 0, WUSER_WIDTH 0, RUSER_WIDTH 0, BUSER_WIDTH 0, READ_WRITE_MODE WRITE_ONLY, HAS_BURST 1, HAS_LOCK 0, HAS_PROT 1, HAS_CACHE 1, HAS_QOS 0, HAS_REGION 0, HAS_WSTRB 1, HAS_BRESP 1, HAS_RRESP 0, SUPPORTS_NARROW_BURST 1, NUM_READ_OUTSTANDING 2, NUM_WRITE_OUTSTANDING 2, MAX_BURST_LENGTH 256, PHASE 0.0, CLK_DOMAIN system_clk_gen_0_clk_125, NUM_READ_THREADS 1, NUM_WRITE_T\
HREADS 1, RUSER_BITS_PER_BYTE 0, WUSER_BITS_PER_BYTE 0, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_osc2 BREADY" *)
output wire m_axi_osc2_bready;

  rp_oscilloscope #(
    .S_AXI_REG_ADDR_BITS(12),
    .M_AXI_OSC1_ADDR_BITS(32),
    .M_AXI_OSC1_DATA_BITS(64),
    .M_AXI_OSC2_ADDR_BITS(32),
    .M_AXI_OSC2_DATA_BITS(64),
    .ADC_DATA_BITS(14),
    .EVENT_SRC_NUM(5),
    .TRIG_SRC_NUM(5)
  ) inst (
    .clk(clk),
    .rst_n(rst_n),
    .intr(intr),
    .adc_data_ch1(adc_data_ch1),
    .adc_data_ch2(adc_data_ch2),
    .event_ip_trig(event_ip_trig),
    .event_ip_stop(event_ip_stop),
    .event_ip_start(event_ip_start),
    .event_ip_reset(event_ip_reset),
    .trig_ip(trig_ip),
    .osc1_event_op(osc1_event_op),
    .osc1_trig_op(osc1_trig_op),
    .osc2_event_op(osc2_event_op),
    .osc2_trig_op(osc2_trig_op),
    .s_axi_reg_aclk(s_axi_reg_aclk),
    .s_axi_reg_aresetn(s_axi_reg_aresetn),
    .s_axi_reg_awaddr(s_axi_reg_awaddr),
    .s_axi_reg_awprot(s_axi_reg_awprot),
    .s_axi_reg_awvalid(s_axi_reg_awvalid),
    .s_axi_reg_awready(s_axi_reg_awready),
    .s_axi_reg_wdata(s_axi_reg_wdata),
    .s_axi_reg_wstrb(s_axi_reg_wstrb),
    .s_axi_reg_wvalid(s_axi_reg_wvalid),
    .s_axi_reg_wready(s_axi_reg_wready),
    .s_axi_reg_bresp(s_axi_reg_bresp),
    .s_axi_reg_bvalid(s_axi_reg_bvalid),
    .s_axi_reg_bready(s_axi_reg_bready),
    .s_axi_reg_araddr(s_axi_reg_araddr),
    .s_axi_reg_arprot(s_axi_reg_arprot),
    .s_axi_reg_arvalid(s_axi_reg_arvalid),
    .s_axi_reg_arready(s_axi_reg_arready),
    .s_axi_reg_rdata(s_axi_reg_rdata),
    .s_axi_reg_rresp(s_axi_reg_rresp),
    .s_axi_reg_rvalid(s_axi_reg_rvalid),
    .s_axi_reg_rready(s_axi_reg_rready),
    .m_axi_osc1_aclk(m_axi_osc1_aclk),
    .m_axi_osc1_aresetn(m_axi_osc1_aresetn),
    .m_axi_osc1_awaddr(m_axi_osc1_awaddr),
    .m_axi_osc1_awlen(m_axi_osc1_awlen),
    .m_axi_osc1_awsize(m_axi_osc1_awsize),
    .m_axi_osc1_awburst(m_axi_osc1_awburst),
    .m_axi_osc1_awprot(m_axi_osc1_awprot),
    .m_axi_osc1_awcache(m_axi_osc1_awcache),
    .m_axi_osc1_awvalid(m_axi_osc1_awvalid),
    .m_axi_osc1_awready(m_axi_osc1_awready),
    .m_axi_osc1_wdata(m_axi_osc1_wdata),
    .m_axi_osc1_wstrb(m_axi_osc1_wstrb),
    .m_axi_osc1_wlast(m_axi_osc1_wlast),
    .m_axi_osc1_wvalid(m_axi_osc1_wvalid),
    .m_axi_osc1_wready(m_axi_osc1_wready),
    .m_axi_osc1_bresp(m_axi_osc1_bresp),
    .m_axi_osc1_bvalid(m_axi_osc1_bvalid),
    .m_axi_osc1_bready(m_axi_osc1_bready),
    .m_axi_osc2_aclk(m_axi_osc2_aclk),
    .m_axi_osc2_aresetn(m_axi_osc2_aresetn),
    .m_axi_osc2_awaddr(m_axi_osc2_awaddr),
    .m_axi_osc2_awlen(m_axi_osc2_awlen),
    .m_axi_osc2_awsize(m_axi_osc2_awsize),
    .m_axi_osc2_awburst(m_axi_osc2_awburst),
    .m_axi_osc2_awprot(m_axi_osc2_awprot),
    .m_axi_osc2_awcache(m_axi_osc2_awcache),
    .m_axi_osc2_awvalid(m_axi_osc2_awvalid),
    .m_axi_osc2_awready(m_axi_osc2_awready),
    .m_axi_osc2_wdata(m_axi_osc2_wdata),
    .m_axi_osc2_wstrb(m_axi_osc2_wstrb),
    .m_axi_osc2_wlast(m_axi_osc2_wlast),
    .m_axi_osc2_wvalid(m_axi_osc2_wvalid),
    .m_axi_osc2_wready(m_axi_osc2_wready),
    .m_axi_osc2_bresp(m_axi_osc2_bresp),
    .m_axi_osc2_bvalid(m_axi_osc2_bvalid),
    .m_axi_osc2_bready(m_axi_osc2_bready)
  );
endmodule
