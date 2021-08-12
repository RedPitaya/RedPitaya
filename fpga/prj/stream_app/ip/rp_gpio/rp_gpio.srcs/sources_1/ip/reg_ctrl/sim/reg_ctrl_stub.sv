// (c) Copyright 1995-2021 Xilinx, Inc. All rights reserved.
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


//------------------------------------------------------------------------------------
// Filename:    reg_ctrl_stub.sv
// Description: This HDL file is intended to be used with following simulators only:
//
//   Vivado Simulator (XSim)
//   Cadence Xcelium Simulator
//   Aldec Riviera-PRO Simulator
//
//------------------------------------------------------------------------------------
`timescale 1ps/1ps

`ifdef XILINX_SIMULATOR

`ifndef XILINX_SIMULATOR_BITASBOOL
`define XILINX_SIMULATOR_BITASBOOL
typedef bit bit_as_bool;
`endif

(* SC_MODULE_EXPORT *)
module reg_ctrl (
  input bit_as_bool s_axi_aclk,
  input bit_as_bool s_axi_aresetn,
  input bit [11 : 0] s_axi_awaddr,
  input bit [2 : 0] s_axi_awprot,
  input bit_as_bool s_axi_awvalid,
  output bit_as_bool s_axi_awready,
  input bit [31 : 0] s_axi_wdata,
  input bit [3 : 0] s_axi_wstrb,
  input bit_as_bool s_axi_wvalid,
  output bit_as_bool s_axi_wready,
  output bit [1 : 0] s_axi_bresp,
  output bit_as_bool s_axi_bvalid,
  input bit_as_bool s_axi_bready,
  input bit [11 : 0] s_axi_araddr,
  input bit [2 : 0] s_axi_arprot,
  input bit_as_bool s_axi_arvalid,
  output bit_as_bool s_axi_arready,
  output bit [31 : 0] s_axi_rdata,
  output bit [1 : 0] s_axi_rresp,
  output bit_as_bool s_axi_rvalid,
  input bit_as_bool s_axi_rready,
  output bit_as_bool bram_rst_a,
  output bit_as_bool bram_clk_a,
  output bit_as_bool bram_en_a,
  output bit [3 : 0] bram_we_a,
  output bit [11 : 0] bram_addr_a,
  output bit [31 : 0] bram_wrdata_a,
  input bit [31 : 0] bram_rddata_a
);
endmodule
`endif

`ifdef XCELIUM
(* XMSC_MODULE_EXPORT *)
module reg_ctrl (s_axi_aclk,s_axi_aresetn,s_axi_awaddr,s_axi_awprot,s_axi_awvalid,s_axi_awready,s_axi_wdata,s_axi_wstrb,s_axi_wvalid,s_axi_wready,s_axi_bresp,s_axi_bvalid,s_axi_bready,s_axi_araddr,s_axi_arprot,s_axi_arvalid,s_axi_arready,s_axi_rdata,s_axi_rresp,s_axi_rvalid,s_axi_rready,bram_rst_a,bram_clk_a,bram_en_a,bram_we_a,bram_addr_a,bram_wrdata_a,bram_rddata_a)
(* integer foreign = "SystemC";
*);
  input bit s_axi_aclk;
  input bit s_axi_aresetn;
  input bit [11 : 0] s_axi_awaddr;
  input bit [2 : 0] s_axi_awprot;
  input bit s_axi_awvalid;
  output wire s_axi_awready;
  input bit [31 : 0] s_axi_wdata;
  input bit [3 : 0] s_axi_wstrb;
  input bit s_axi_wvalid;
  output wire s_axi_wready;
  output wire [1 : 0] s_axi_bresp;
  output wire s_axi_bvalid;
  input bit s_axi_bready;
  input bit [11 : 0] s_axi_araddr;
  input bit [2 : 0] s_axi_arprot;
  input bit s_axi_arvalid;
  output wire s_axi_arready;
  output wire [31 : 0] s_axi_rdata;
  output wire [1 : 0] s_axi_rresp;
  output wire s_axi_rvalid;
  input bit s_axi_rready;
  output wire bram_rst_a;
  output wire bram_clk_a;
  output wire bram_en_a;
  output wire [3 : 0] bram_we_a;
  output wire [11 : 0] bram_addr_a;
  output wire [31 : 0] bram_wrdata_a;
  input bit [31 : 0] bram_rddata_a;
endmodule
`endif

`ifdef RIVIERA
(* SC_MODULE_EXPORT *)
module reg_ctrl (s_axi_aclk,s_axi_aresetn,s_axi_awaddr,s_axi_awprot,s_axi_awvalid,s_axi_awready,s_axi_wdata,s_axi_wstrb,s_axi_wvalid,s_axi_wready,s_axi_bresp,s_axi_bvalid,s_axi_bready,s_axi_araddr,s_axi_arprot,s_axi_arvalid,s_axi_arready,s_axi_rdata,s_axi_rresp,s_axi_rvalid,s_axi_rready,bram_rst_a,bram_clk_a,bram_en_a,bram_we_a,bram_addr_a,bram_wrdata_a,bram_rddata_a)
  input bit s_axi_aclk;
  input bit s_axi_aresetn;
  input bit [11 : 0] s_axi_awaddr;
  input bit [2 : 0] s_axi_awprot;
  input bit s_axi_awvalid;
  output wire s_axi_awready;
  input bit [31 : 0] s_axi_wdata;
  input bit [3 : 0] s_axi_wstrb;
  input bit s_axi_wvalid;
  output wire s_axi_wready;
  output wire [1 : 0] s_axi_bresp;
  output wire s_axi_bvalid;
  input bit s_axi_bready;
  input bit [11 : 0] s_axi_araddr;
  input bit [2 : 0] s_axi_arprot;
  input bit s_axi_arvalid;
  output wire s_axi_arready;
  output wire [31 : 0] s_axi_rdata;
  output wire [1 : 0] s_axi_rresp;
  output wire s_axi_rvalid;
  input bit s_axi_rready;
  output wire bram_rst_a;
  output wire bram_clk_a;
  output wire bram_en_a;
  output wire [3 : 0] bram_we_a;
  output wire [11 : 0] bram_addr_a;
  output wire [31 : 0] bram_wrdata_a;
  input bit [31 : 0] bram_rddata_a;
endmodule
`endif
