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


#include "reg_ctrl_sc.h"

#include "reg_ctrl.h"

#include "axi_bram_ctrl.h"

#include <map>
#include <string>





#ifdef XILINX_SIMULATOR
reg_ctrl::reg_ctrl(const sc_core::sc_module_name& nm) : reg_ctrl_sc(nm), s_axi_aclk("s_axi_aclk"), s_axi_aresetn("s_axi_aresetn"), s_axi_awaddr("s_axi_awaddr"), s_axi_awprot("s_axi_awprot"), s_axi_awvalid("s_axi_awvalid"), s_axi_awready("s_axi_awready"), s_axi_wdata("s_axi_wdata"), s_axi_wstrb("s_axi_wstrb"), s_axi_wvalid("s_axi_wvalid"), s_axi_wready("s_axi_wready"), s_axi_bresp("s_axi_bresp"), s_axi_bvalid("s_axi_bvalid"), s_axi_bready("s_axi_bready"), s_axi_araddr("s_axi_araddr"), s_axi_arprot("s_axi_arprot"), s_axi_arvalid("s_axi_arvalid"), s_axi_arready("s_axi_arready"), s_axi_rdata("s_axi_rdata"), s_axi_rresp("s_axi_rresp"), s_axi_rvalid("s_axi_rvalid"), s_axi_rready("s_axi_rready"), bram_rst_a("bram_rst_a"), bram_clk_a("bram_clk_a"), bram_en_a("bram_en_a"), bram_we_a("bram_we_a"), bram_addr_a("bram_addr_a"), bram_wrdata_a("bram_wrdata_a"), bram_rddata_a("bram_rddata_a")
{

  // initialize pins
  mp_impl->s_axi_aclk(s_axi_aclk);
  mp_impl->s_axi_aresetn(s_axi_aresetn);
  mp_impl->bram_rst_a(bram_rst_a);
  mp_impl->bram_clk_a(bram_clk_a);
  mp_impl->bram_en_a(bram_en_a);
  mp_impl->bram_we_a(bram_we_a);
  mp_impl->bram_addr_a(bram_addr_a);
  mp_impl->bram_wrdata_a(bram_wrdata_a);
  mp_impl->bram_rddata_a(bram_rddata_a);

  // initialize transactors
  mp_S_AXI_transactor = NULL;

  // initialize socket stubs

}

void reg_ctrl::before_end_of_elaboration()
{
  // configure 'S_AXI' transactor

  if (xsc::utils::xsc_sim_manager::getInstanceParameterInt("reg_ctrl", "S_AXI_TLM_MODE") != 1)
  {
    // Instantiate Socket Stubs

  // 'S_AXI' transactor parameters
    xsc::common_cpp::properties S_AXI_transactor_param_props;
    S_AXI_transactor_param_props.addLong("DATA_WIDTH", "32");
    S_AXI_transactor_param_props.addLong("FREQ_HZ", "100000000");
    S_AXI_transactor_param_props.addLong("ID_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("ADDR_WIDTH", "12");
    S_AXI_transactor_param_props.addLong("AWUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("ARUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("WUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("RUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("BUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("HAS_BURST", "0");
    S_AXI_transactor_param_props.addLong("HAS_LOCK", "0");
    S_AXI_transactor_param_props.addLong("HAS_PROT", "1");
    S_AXI_transactor_param_props.addLong("HAS_CACHE", "0");
    S_AXI_transactor_param_props.addLong("HAS_QOS", "0");
    S_AXI_transactor_param_props.addLong("HAS_REGION", "0");
    S_AXI_transactor_param_props.addLong("HAS_WSTRB", "1");
    S_AXI_transactor_param_props.addLong("HAS_BRESP", "1");
    S_AXI_transactor_param_props.addLong("HAS_RRESP", "1");
    S_AXI_transactor_param_props.addLong("SUPPORTS_NARROW_BURST", "0");
    S_AXI_transactor_param_props.addLong("NUM_READ_OUTSTANDING", "1");
    S_AXI_transactor_param_props.addLong("NUM_WRITE_OUTSTANDING", "1");
    S_AXI_transactor_param_props.addLong("MAX_BURST_LENGTH", "1");
    S_AXI_transactor_param_props.addLong("NUM_READ_THREADS", "1");
    S_AXI_transactor_param_props.addLong("NUM_WRITE_THREADS", "1");
    S_AXI_transactor_param_props.addLong("RUSER_BITS_PER_BYTE", "0");
    S_AXI_transactor_param_props.addLong("WUSER_BITS_PER_BYTE", "0");
    S_AXI_transactor_param_props.addLong("HAS_SIZE", "0");
    S_AXI_transactor_param_props.addLong("HAS_RESET", "1");
    S_AXI_transactor_param_props.addFloat("PHASE", "0.000");
    S_AXI_transactor_param_props.addString("PROTOCOL", "AXI4LITE");
    S_AXI_transactor_param_props.addString("READ_WRITE_MODE", "READ_WRITE");
    S_AXI_transactor_param_props.addString("CLK_DOMAIN", "");

    mp_S_AXI_transactor = new xtlm::xaximm_pin2xtlm_t<32,12,1,1,1,1,1,1>("S_AXI_transactor", S_AXI_transactor_param_props);

    // S_AXI' transactor ports

    mp_S_AXI_transactor->ARADDR(s_axi_araddr);
    mp_S_AXI_transactor->ARPROT(s_axi_arprot);
    mp_S_AXI_transactor->ARREADY(s_axi_arready);
    mp_S_AXI_transactor->ARVALID(s_axi_arvalid);
    mp_S_AXI_transactor->AWADDR(s_axi_awaddr);
    mp_S_AXI_transactor->AWPROT(s_axi_awprot);
    mp_S_AXI_transactor->AWREADY(s_axi_awready);
    mp_S_AXI_transactor->AWVALID(s_axi_awvalid);
    mp_S_AXI_transactor->BREADY(s_axi_bready);
    mp_S_AXI_transactor->BRESP(s_axi_bresp);
    mp_S_AXI_transactor->BVALID(s_axi_bvalid);
    mp_S_AXI_transactor->RDATA(s_axi_rdata);
    mp_S_AXI_transactor->RREADY(s_axi_rready);
    mp_S_AXI_transactor->RRESP(s_axi_rresp);
    mp_S_AXI_transactor->RVALID(s_axi_rvalid);
    mp_S_AXI_transactor->WDATA(s_axi_wdata);
    mp_S_AXI_transactor->WREADY(s_axi_wready);
    mp_S_AXI_transactor->WSTRB(s_axi_wstrb);
    mp_S_AXI_transactor->WVALID(s_axi_wvalid);
    mp_S_AXI_transactor->CLK(s_axi_aclk);
    mp_S_AXI_transactor->RST(s_axi_aresetn);

    // S_AXI' transactor sockets

    mp_impl->target_0_rd_socket->bind(*(mp_S_AXI_transactor->rd_socket));
    mp_impl->target_0_wr_socket->bind(*(mp_S_AXI_transactor->wr_socket));
  }

}

#endif // XILINX_SIMULATOR




#ifdef XM_SYSTEMC
reg_ctrl::reg_ctrl(const sc_core::sc_module_name& nm) : reg_ctrl_sc(nm), s_axi_aclk("s_axi_aclk"), s_axi_aresetn("s_axi_aresetn"), s_axi_awaddr("s_axi_awaddr"), s_axi_awprot("s_axi_awprot"), s_axi_awvalid("s_axi_awvalid"), s_axi_awready("s_axi_awready"), s_axi_wdata("s_axi_wdata"), s_axi_wstrb("s_axi_wstrb"), s_axi_wvalid("s_axi_wvalid"), s_axi_wready("s_axi_wready"), s_axi_bresp("s_axi_bresp"), s_axi_bvalid("s_axi_bvalid"), s_axi_bready("s_axi_bready"), s_axi_araddr("s_axi_araddr"), s_axi_arprot("s_axi_arprot"), s_axi_arvalid("s_axi_arvalid"), s_axi_arready("s_axi_arready"), s_axi_rdata("s_axi_rdata"), s_axi_rresp("s_axi_rresp"), s_axi_rvalid("s_axi_rvalid"), s_axi_rready("s_axi_rready"), bram_rst_a("bram_rst_a"), bram_clk_a("bram_clk_a"), bram_en_a("bram_en_a"), bram_we_a("bram_we_a"), bram_addr_a("bram_addr_a"), bram_wrdata_a("bram_wrdata_a"), bram_rddata_a("bram_rddata_a")
{

  // initialize pins
  mp_impl->s_axi_aclk(s_axi_aclk);
  mp_impl->s_axi_aresetn(s_axi_aresetn);
  mp_impl->bram_rst_a(bram_rst_a);
  mp_impl->bram_clk_a(bram_clk_a);
  mp_impl->bram_en_a(bram_en_a);
  mp_impl->bram_we_a(bram_we_a);
  mp_impl->bram_addr_a(bram_addr_a);
  mp_impl->bram_wrdata_a(bram_wrdata_a);
  mp_impl->bram_rddata_a(bram_rddata_a);

  // initialize transactors
  mp_S_AXI_transactor = NULL;

  // initialize socket stubs

}

void reg_ctrl::before_end_of_elaboration()
{
  // configure 'S_AXI' transactor

  if (xsc::utils::xsc_sim_manager::getInstanceParameterInt("reg_ctrl", "S_AXI_TLM_MODE") != 1)
  {
    // Instantiate Socket Stubs

  // 'S_AXI' transactor parameters
    xsc::common_cpp::properties S_AXI_transactor_param_props;
    S_AXI_transactor_param_props.addLong("DATA_WIDTH", "32");
    S_AXI_transactor_param_props.addLong("FREQ_HZ", "100000000");
    S_AXI_transactor_param_props.addLong("ID_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("ADDR_WIDTH", "12");
    S_AXI_transactor_param_props.addLong("AWUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("ARUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("WUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("RUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("BUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("HAS_BURST", "0");
    S_AXI_transactor_param_props.addLong("HAS_LOCK", "0");
    S_AXI_transactor_param_props.addLong("HAS_PROT", "1");
    S_AXI_transactor_param_props.addLong("HAS_CACHE", "0");
    S_AXI_transactor_param_props.addLong("HAS_QOS", "0");
    S_AXI_transactor_param_props.addLong("HAS_REGION", "0");
    S_AXI_transactor_param_props.addLong("HAS_WSTRB", "1");
    S_AXI_transactor_param_props.addLong("HAS_BRESP", "1");
    S_AXI_transactor_param_props.addLong("HAS_RRESP", "1");
    S_AXI_transactor_param_props.addLong("SUPPORTS_NARROW_BURST", "0");
    S_AXI_transactor_param_props.addLong("NUM_READ_OUTSTANDING", "1");
    S_AXI_transactor_param_props.addLong("NUM_WRITE_OUTSTANDING", "1");
    S_AXI_transactor_param_props.addLong("MAX_BURST_LENGTH", "1");
    S_AXI_transactor_param_props.addLong("NUM_READ_THREADS", "1");
    S_AXI_transactor_param_props.addLong("NUM_WRITE_THREADS", "1");
    S_AXI_transactor_param_props.addLong("RUSER_BITS_PER_BYTE", "0");
    S_AXI_transactor_param_props.addLong("WUSER_BITS_PER_BYTE", "0");
    S_AXI_transactor_param_props.addLong("HAS_SIZE", "0");
    S_AXI_transactor_param_props.addLong("HAS_RESET", "1");
    S_AXI_transactor_param_props.addFloat("PHASE", "0.000");
    S_AXI_transactor_param_props.addString("PROTOCOL", "AXI4LITE");
    S_AXI_transactor_param_props.addString("READ_WRITE_MODE", "READ_WRITE");
    S_AXI_transactor_param_props.addString("CLK_DOMAIN", "");

    mp_S_AXI_transactor = new xtlm::xaximm_pin2xtlm_t<32,12,1,1,1,1,1,1>("S_AXI_transactor", S_AXI_transactor_param_props);

    // S_AXI' transactor ports

    mp_S_AXI_transactor->ARADDR(s_axi_araddr);
    mp_S_AXI_transactor->ARPROT(s_axi_arprot);
    mp_S_AXI_transactor->ARREADY(s_axi_arready);
    mp_S_AXI_transactor->ARVALID(s_axi_arvalid);
    mp_S_AXI_transactor->AWADDR(s_axi_awaddr);
    mp_S_AXI_transactor->AWPROT(s_axi_awprot);
    mp_S_AXI_transactor->AWREADY(s_axi_awready);
    mp_S_AXI_transactor->AWVALID(s_axi_awvalid);
    mp_S_AXI_transactor->BREADY(s_axi_bready);
    mp_S_AXI_transactor->BRESP(s_axi_bresp);
    mp_S_AXI_transactor->BVALID(s_axi_bvalid);
    mp_S_AXI_transactor->RDATA(s_axi_rdata);
    mp_S_AXI_transactor->RREADY(s_axi_rready);
    mp_S_AXI_transactor->RRESP(s_axi_rresp);
    mp_S_AXI_transactor->RVALID(s_axi_rvalid);
    mp_S_AXI_transactor->WDATA(s_axi_wdata);
    mp_S_AXI_transactor->WREADY(s_axi_wready);
    mp_S_AXI_transactor->WSTRB(s_axi_wstrb);
    mp_S_AXI_transactor->WVALID(s_axi_wvalid);
    mp_S_AXI_transactor->CLK(s_axi_aclk);
    mp_S_AXI_transactor->RST(s_axi_aresetn);

    // S_AXI' transactor sockets

    mp_impl->target_0_rd_socket->bind(*(mp_S_AXI_transactor->rd_socket));
    mp_impl->target_0_wr_socket->bind(*(mp_S_AXI_transactor->wr_socket));
  }

}

#endif // XM_SYSTEMC




#ifdef RIVIERA
reg_ctrl::reg_ctrl(const sc_core::sc_module_name& nm) : reg_ctrl_sc(nm), s_axi_aclk("s_axi_aclk"), s_axi_aresetn("s_axi_aresetn"), s_axi_awaddr("s_axi_awaddr"), s_axi_awprot("s_axi_awprot"), s_axi_awvalid("s_axi_awvalid"), s_axi_awready("s_axi_awready"), s_axi_wdata("s_axi_wdata"), s_axi_wstrb("s_axi_wstrb"), s_axi_wvalid("s_axi_wvalid"), s_axi_wready("s_axi_wready"), s_axi_bresp("s_axi_bresp"), s_axi_bvalid("s_axi_bvalid"), s_axi_bready("s_axi_bready"), s_axi_araddr("s_axi_araddr"), s_axi_arprot("s_axi_arprot"), s_axi_arvalid("s_axi_arvalid"), s_axi_arready("s_axi_arready"), s_axi_rdata("s_axi_rdata"), s_axi_rresp("s_axi_rresp"), s_axi_rvalid("s_axi_rvalid"), s_axi_rready("s_axi_rready"), bram_rst_a("bram_rst_a"), bram_clk_a("bram_clk_a"), bram_en_a("bram_en_a"), bram_we_a("bram_we_a"), bram_addr_a("bram_addr_a"), bram_wrdata_a("bram_wrdata_a"), bram_rddata_a("bram_rddata_a")
{

  // initialize pins
  mp_impl->s_axi_aclk(s_axi_aclk);
  mp_impl->s_axi_aresetn(s_axi_aresetn);
  mp_impl->bram_rst_a(bram_rst_a);
  mp_impl->bram_clk_a(bram_clk_a);
  mp_impl->bram_en_a(bram_en_a);
  mp_impl->bram_we_a(bram_we_a);
  mp_impl->bram_addr_a(bram_addr_a);
  mp_impl->bram_wrdata_a(bram_wrdata_a);
  mp_impl->bram_rddata_a(bram_rddata_a);

  // initialize transactors
  mp_S_AXI_transactor = NULL;

  // initialize socket stubs

}

void reg_ctrl::before_end_of_elaboration()
{
  // configure 'S_AXI' transactor

  if (xsc::utils::xsc_sim_manager::getInstanceParameterInt("reg_ctrl", "S_AXI_TLM_MODE") != 1)
  {
    // Instantiate Socket Stubs

  // 'S_AXI' transactor parameters
    xsc::common_cpp::properties S_AXI_transactor_param_props;
    S_AXI_transactor_param_props.addLong("DATA_WIDTH", "32");
    S_AXI_transactor_param_props.addLong("FREQ_HZ", "100000000");
    S_AXI_transactor_param_props.addLong("ID_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("ADDR_WIDTH", "12");
    S_AXI_transactor_param_props.addLong("AWUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("ARUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("WUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("RUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("BUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("HAS_BURST", "0");
    S_AXI_transactor_param_props.addLong("HAS_LOCK", "0");
    S_AXI_transactor_param_props.addLong("HAS_PROT", "1");
    S_AXI_transactor_param_props.addLong("HAS_CACHE", "0");
    S_AXI_transactor_param_props.addLong("HAS_QOS", "0");
    S_AXI_transactor_param_props.addLong("HAS_REGION", "0");
    S_AXI_transactor_param_props.addLong("HAS_WSTRB", "1");
    S_AXI_transactor_param_props.addLong("HAS_BRESP", "1");
    S_AXI_transactor_param_props.addLong("HAS_RRESP", "1");
    S_AXI_transactor_param_props.addLong("SUPPORTS_NARROW_BURST", "0");
    S_AXI_transactor_param_props.addLong("NUM_READ_OUTSTANDING", "1");
    S_AXI_transactor_param_props.addLong("NUM_WRITE_OUTSTANDING", "1");
    S_AXI_transactor_param_props.addLong("MAX_BURST_LENGTH", "1");
    S_AXI_transactor_param_props.addLong("NUM_READ_THREADS", "1");
    S_AXI_transactor_param_props.addLong("NUM_WRITE_THREADS", "1");
    S_AXI_transactor_param_props.addLong("RUSER_BITS_PER_BYTE", "0");
    S_AXI_transactor_param_props.addLong("WUSER_BITS_PER_BYTE", "0");
    S_AXI_transactor_param_props.addLong("HAS_SIZE", "0");
    S_AXI_transactor_param_props.addLong("HAS_RESET", "1");
    S_AXI_transactor_param_props.addFloat("PHASE", "0.000");
    S_AXI_transactor_param_props.addString("PROTOCOL", "AXI4LITE");
    S_AXI_transactor_param_props.addString("READ_WRITE_MODE", "READ_WRITE");
    S_AXI_transactor_param_props.addString("CLK_DOMAIN", "");

    mp_S_AXI_transactor = new xtlm::xaximm_pin2xtlm_t<32,12,1,1,1,1,1,1>("S_AXI_transactor", S_AXI_transactor_param_props);

    // S_AXI' transactor ports

    mp_S_AXI_transactor->ARADDR(s_axi_araddr);
    mp_S_AXI_transactor->ARPROT(s_axi_arprot);
    mp_S_AXI_transactor->ARREADY(s_axi_arready);
    mp_S_AXI_transactor->ARVALID(s_axi_arvalid);
    mp_S_AXI_transactor->AWADDR(s_axi_awaddr);
    mp_S_AXI_transactor->AWPROT(s_axi_awprot);
    mp_S_AXI_transactor->AWREADY(s_axi_awready);
    mp_S_AXI_transactor->AWVALID(s_axi_awvalid);
    mp_S_AXI_transactor->BREADY(s_axi_bready);
    mp_S_AXI_transactor->BRESP(s_axi_bresp);
    mp_S_AXI_transactor->BVALID(s_axi_bvalid);
    mp_S_AXI_transactor->RDATA(s_axi_rdata);
    mp_S_AXI_transactor->RREADY(s_axi_rready);
    mp_S_AXI_transactor->RRESP(s_axi_rresp);
    mp_S_AXI_transactor->RVALID(s_axi_rvalid);
    mp_S_AXI_transactor->WDATA(s_axi_wdata);
    mp_S_AXI_transactor->WREADY(s_axi_wready);
    mp_S_AXI_transactor->WSTRB(s_axi_wstrb);
    mp_S_AXI_transactor->WVALID(s_axi_wvalid);
    mp_S_AXI_transactor->CLK(s_axi_aclk);
    mp_S_AXI_transactor->RST(s_axi_aresetn);

    // S_AXI' transactor sockets

    mp_impl->target_0_rd_socket->bind(*(mp_S_AXI_transactor->rd_socket));
    mp_impl->target_0_wr_socket->bind(*(mp_S_AXI_transactor->wr_socket));
  }

}

#endif // RIVIERA




#ifdef VCSSYSTEMC
reg_ctrl::reg_ctrl(const sc_core::sc_module_name& nm) : reg_ctrl_sc(nm),  s_axi_aclk("s_axi_aclk"), s_axi_aresetn("s_axi_aresetn"), s_axi_awaddr("s_axi_awaddr"), s_axi_awprot("s_axi_awprot"), s_axi_awvalid("s_axi_awvalid"), s_axi_awready("s_axi_awready"), s_axi_wdata("s_axi_wdata"), s_axi_wstrb("s_axi_wstrb"), s_axi_wvalid("s_axi_wvalid"), s_axi_wready("s_axi_wready"), s_axi_bresp("s_axi_bresp"), s_axi_bvalid("s_axi_bvalid"), s_axi_bready("s_axi_bready"), s_axi_araddr("s_axi_araddr"), s_axi_arprot("s_axi_arprot"), s_axi_arvalid("s_axi_arvalid"), s_axi_arready("s_axi_arready"), s_axi_rdata("s_axi_rdata"), s_axi_rresp("s_axi_rresp"), s_axi_rvalid("s_axi_rvalid"), s_axi_rready("s_axi_rready"), bram_rst_a("bram_rst_a"), bram_clk_a("bram_clk_a"), bram_en_a("bram_en_a"), bram_we_a("bram_we_a"), bram_addr_a("bram_addr_a"), bram_wrdata_a("bram_wrdata_a"), bram_rddata_a("bram_rddata_a")
{
  // initialize pins
  mp_impl->s_axi_aclk(s_axi_aclk);
  mp_impl->s_axi_aresetn(s_axi_aresetn);
  mp_impl->bram_rst_a(bram_rst_a);
  mp_impl->bram_clk_a(bram_clk_a);
  mp_impl->bram_en_a(bram_en_a);
  mp_impl->bram_we_a(bram_we_a);
  mp_impl->bram_addr_a(bram_addr_a);
  mp_impl->bram_wrdata_a(bram_wrdata_a);
  mp_impl->bram_rddata_a(bram_rddata_a);

  // initialize transactors
  mp_S_AXI_transactor = NULL;
  // Instantiate Socket Stubs

  // configure S_AXI_transactor
    xsc::common_cpp::properties S_AXI_transactor_param_props;
    S_AXI_transactor_param_props.addLong("DATA_WIDTH", "32");
    S_AXI_transactor_param_props.addLong("FREQ_HZ", "100000000");
    S_AXI_transactor_param_props.addLong("ID_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("ADDR_WIDTH", "12");
    S_AXI_transactor_param_props.addLong("AWUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("ARUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("WUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("RUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("BUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("HAS_BURST", "0");
    S_AXI_transactor_param_props.addLong("HAS_LOCK", "0");
    S_AXI_transactor_param_props.addLong("HAS_PROT", "1");
    S_AXI_transactor_param_props.addLong("HAS_CACHE", "0");
    S_AXI_transactor_param_props.addLong("HAS_QOS", "0");
    S_AXI_transactor_param_props.addLong("HAS_REGION", "0");
    S_AXI_transactor_param_props.addLong("HAS_WSTRB", "1");
    S_AXI_transactor_param_props.addLong("HAS_BRESP", "1");
    S_AXI_transactor_param_props.addLong("HAS_RRESP", "1");
    S_AXI_transactor_param_props.addLong("SUPPORTS_NARROW_BURST", "0");
    S_AXI_transactor_param_props.addLong("NUM_READ_OUTSTANDING", "1");
    S_AXI_transactor_param_props.addLong("NUM_WRITE_OUTSTANDING", "1");
    S_AXI_transactor_param_props.addLong("MAX_BURST_LENGTH", "1");
    S_AXI_transactor_param_props.addLong("NUM_READ_THREADS", "1");
    S_AXI_transactor_param_props.addLong("NUM_WRITE_THREADS", "1");
    S_AXI_transactor_param_props.addLong("RUSER_BITS_PER_BYTE", "0");
    S_AXI_transactor_param_props.addLong("WUSER_BITS_PER_BYTE", "0");
    S_AXI_transactor_param_props.addLong("HAS_SIZE", "0");
    S_AXI_transactor_param_props.addLong("HAS_RESET", "1");
    S_AXI_transactor_param_props.addFloat("PHASE", "0.000");
    S_AXI_transactor_param_props.addString("PROTOCOL", "AXI4LITE");
    S_AXI_transactor_param_props.addString("READ_WRITE_MODE", "READ_WRITE");
    S_AXI_transactor_param_props.addString("CLK_DOMAIN", "");

    mp_S_AXI_transactor = new xtlm::xaximm_pin2xtlm_t<32,12,1,1,1,1,1,1>("S_AXI_transactor", S_AXI_transactor_param_props);
  mp_S_AXI_transactor->ARADDR(s_axi_araddr);
  mp_S_AXI_transactor->ARPROT(s_axi_arprot);
  mp_S_AXI_transactor->ARREADY(s_axi_arready);
  mp_S_AXI_transactor->ARVALID(s_axi_arvalid);
  mp_S_AXI_transactor->AWADDR(s_axi_awaddr);
  mp_S_AXI_transactor->AWPROT(s_axi_awprot);
  mp_S_AXI_transactor->AWREADY(s_axi_awready);
  mp_S_AXI_transactor->AWVALID(s_axi_awvalid);
  mp_S_AXI_transactor->BREADY(s_axi_bready);
  mp_S_AXI_transactor->BRESP(s_axi_bresp);
  mp_S_AXI_transactor->BVALID(s_axi_bvalid);
  mp_S_AXI_transactor->RDATA(s_axi_rdata);
  mp_S_AXI_transactor->RREADY(s_axi_rready);
  mp_S_AXI_transactor->RRESP(s_axi_rresp);
  mp_S_AXI_transactor->RVALID(s_axi_rvalid);
  mp_S_AXI_transactor->WDATA(s_axi_wdata);
  mp_S_AXI_transactor->WREADY(s_axi_wready);
  mp_S_AXI_transactor->WSTRB(s_axi_wstrb);
  mp_S_AXI_transactor->WVALID(s_axi_wvalid);
  mp_S_AXI_transactor->CLK(s_axi_aclk);
  mp_S_AXI_transactor->RST(s_axi_aresetn);


  // initialize transactors stubs
  S_AXI_transactor_target_wr_socket_stub = nullptr;
  S_AXI_transactor_target_rd_socket_stub = nullptr;

}

void reg_ctrl::before_end_of_elaboration()
{
  // configure 'S_AXI' transactor
  if (xsc::utils::xsc_sim_manager::getInstanceParameterInt("reg_ctrl", "S_AXI_TLM_MODE") != 1)
  {
    mp_impl->target_0_rd_socket->bind(*(mp_S_AXI_transactor->rd_socket));
    mp_impl->target_0_wr_socket->bind(*(mp_S_AXI_transactor->wr_socket));
  }
  else
  {
    S_AXI_transactor_target_wr_socket_stub = new xtlm::xtlm_aximm_target_stub("wr_socket",0);
    S_AXI_transactor_target_wr_socket_stub->bind(*(mp_S_AXI_transactor->wr_socket));
    S_AXI_transactor_target_rd_socket_stub = new xtlm::xtlm_aximm_target_stub("rd_socket",0);
    S_AXI_transactor_target_rd_socket_stub->bind(*(mp_S_AXI_transactor->rd_socket));
    mp_S_AXI_transactor->disable_transactor();
  }

}

#endif // VCSSYSTEMC




#ifdef MTI_SYSTEMC
reg_ctrl::reg_ctrl(const sc_core::sc_module_name& nm) : reg_ctrl_sc(nm),  s_axi_aclk("s_axi_aclk"), s_axi_aresetn("s_axi_aresetn"), s_axi_awaddr("s_axi_awaddr"), s_axi_awprot("s_axi_awprot"), s_axi_awvalid("s_axi_awvalid"), s_axi_awready("s_axi_awready"), s_axi_wdata("s_axi_wdata"), s_axi_wstrb("s_axi_wstrb"), s_axi_wvalid("s_axi_wvalid"), s_axi_wready("s_axi_wready"), s_axi_bresp("s_axi_bresp"), s_axi_bvalid("s_axi_bvalid"), s_axi_bready("s_axi_bready"), s_axi_araddr("s_axi_araddr"), s_axi_arprot("s_axi_arprot"), s_axi_arvalid("s_axi_arvalid"), s_axi_arready("s_axi_arready"), s_axi_rdata("s_axi_rdata"), s_axi_rresp("s_axi_rresp"), s_axi_rvalid("s_axi_rvalid"), s_axi_rready("s_axi_rready"), bram_rst_a("bram_rst_a"), bram_clk_a("bram_clk_a"), bram_en_a("bram_en_a"), bram_we_a("bram_we_a"), bram_addr_a("bram_addr_a"), bram_wrdata_a("bram_wrdata_a"), bram_rddata_a("bram_rddata_a")
{
  // initialize pins
  mp_impl->s_axi_aclk(s_axi_aclk);
  mp_impl->s_axi_aresetn(s_axi_aresetn);
  mp_impl->bram_rst_a(bram_rst_a);
  mp_impl->bram_clk_a(bram_clk_a);
  mp_impl->bram_en_a(bram_en_a);
  mp_impl->bram_we_a(bram_we_a);
  mp_impl->bram_addr_a(bram_addr_a);
  mp_impl->bram_wrdata_a(bram_wrdata_a);
  mp_impl->bram_rddata_a(bram_rddata_a);

  // initialize transactors
  mp_S_AXI_transactor = NULL;
  // Instantiate Socket Stubs

  // configure S_AXI_transactor
    xsc::common_cpp::properties S_AXI_transactor_param_props;
    S_AXI_transactor_param_props.addLong("DATA_WIDTH", "32");
    S_AXI_transactor_param_props.addLong("FREQ_HZ", "100000000");
    S_AXI_transactor_param_props.addLong("ID_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("ADDR_WIDTH", "12");
    S_AXI_transactor_param_props.addLong("AWUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("ARUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("WUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("RUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("BUSER_WIDTH", "0");
    S_AXI_transactor_param_props.addLong("HAS_BURST", "0");
    S_AXI_transactor_param_props.addLong("HAS_LOCK", "0");
    S_AXI_transactor_param_props.addLong("HAS_PROT", "1");
    S_AXI_transactor_param_props.addLong("HAS_CACHE", "0");
    S_AXI_transactor_param_props.addLong("HAS_QOS", "0");
    S_AXI_transactor_param_props.addLong("HAS_REGION", "0");
    S_AXI_transactor_param_props.addLong("HAS_WSTRB", "1");
    S_AXI_transactor_param_props.addLong("HAS_BRESP", "1");
    S_AXI_transactor_param_props.addLong("HAS_RRESP", "1");
    S_AXI_transactor_param_props.addLong("SUPPORTS_NARROW_BURST", "0");
    S_AXI_transactor_param_props.addLong("NUM_READ_OUTSTANDING", "1");
    S_AXI_transactor_param_props.addLong("NUM_WRITE_OUTSTANDING", "1");
    S_AXI_transactor_param_props.addLong("MAX_BURST_LENGTH", "1");
    S_AXI_transactor_param_props.addLong("NUM_READ_THREADS", "1");
    S_AXI_transactor_param_props.addLong("NUM_WRITE_THREADS", "1");
    S_AXI_transactor_param_props.addLong("RUSER_BITS_PER_BYTE", "0");
    S_AXI_transactor_param_props.addLong("WUSER_BITS_PER_BYTE", "0");
    S_AXI_transactor_param_props.addLong("HAS_SIZE", "0");
    S_AXI_transactor_param_props.addLong("HAS_RESET", "1");
    S_AXI_transactor_param_props.addFloat("PHASE", "0.000");
    S_AXI_transactor_param_props.addString("PROTOCOL", "AXI4LITE");
    S_AXI_transactor_param_props.addString("READ_WRITE_MODE", "READ_WRITE");
    S_AXI_transactor_param_props.addString("CLK_DOMAIN", "");

    mp_S_AXI_transactor = new xtlm::xaximm_pin2xtlm_t<32,12,1,1,1,1,1,1>("S_AXI_transactor", S_AXI_transactor_param_props);
  mp_S_AXI_transactor->ARADDR(s_axi_araddr);
  mp_S_AXI_transactor->ARPROT(s_axi_arprot);
  mp_S_AXI_transactor->ARREADY(s_axi_arready);
  mp_S_AXI_transactor->ARVALID(s_axi_arvalid);
  mp_S_AXI_transactor->AWADDR(s_axi_awaddr);
  mp_S_AXI_transactor->AWPROT(s_axi_awprot);
  mp_S_AXI_transactor->AWREADY(s_axi_awready);
  mp_S_AXI_transactor->AWVALID(s_axi_awvalid);
  mp_S_AXI_transactor->BREADY(s_axi_bready);
  mp_S_AXI_transactor->BRESP(s_axi_bresp);
  mp_S_AXI_transactor->BVALID(s_axi_bvalid);
  mp_S_AXI_transactor->RDATA(s_axi_rdata);
  mp_S_AXI_transactor->RREADY(s_axi_rready);
  mp_S_AXI_transactor->RRESP(s_axi_rresp);
  mp_S_AXI_transactor->RVALID(s_axi_rvalid);
  mp_S_AXI_transactor->WDATA(s_axi_wdata);
  mp_S_AXI_transactor->WREADY(s_axi_wready);
  mp_S_AXI_transactor->WSTRB(s_axi_wstrb);
  mp_S_AXI_transactor->WVALID(s_axi_wvalid);
  mp_S_AXI_transactor->CLK(s_axi_aclk);
  mp_S_AXI_transactor->RST(s_axi_aresetn);


  // initialize transactors stubs
  S_AXI_transactor_target_wr_socket_stub = nullptr;
  S_AXI_transactor_target_rd_socket_stub = nullptr;

}

void reg_ctrl::before_end_of_elaboration()
{
  // configure 'S_AXI' transactor
  if (xsc::utils::xsc_sim_manager::getInstanceParameterInt("reg_ctrl", "S_AXI_TLM_MODE") != 1)
  {
    mp_impl->target_0_rd_socket->bind(*(mp_S_AXI_transactor->rd_socket));
    mp_impl->target_0_wr_socket->bind(*(mp_S_AXI_transactor->wr_socket));
  }
  else
  {
    S_AXI_transactor_target_wr_socket_stub = new xtlm::xtlm_aximm_target_stub("wr_socket",0);
    S_AXI_transactor_target_wr_socket_stub->bind(*(mp_S_AXI_transactor->wr_socket));
    S_AXI_transactor_target_rd_socket_stub = new xtlm::xtlm_aximm_target_stub("rd_socket",0);
    S_AXI_transactor_target_rd_socket_stub->bind(*(mp_S_AXI_transactor->rd_socket));
    mp_S_AXI_transactor->disable_transactor();
  }

}

#endif // MTI_SYSTEMC




reg_ctrl::~reg_ctrl()
{
  delete mp_S_AXI_transactor;

}

#ifdef MTI_SYSTEMC
SC_MODULE_EXPORT(reg_ctrl);
#endif

#ifdef XM_SYSTEMC
XMSC_MODULE_EXPORT(reg_ctrl);
#endif

#ifdef RIVIERA
SC_MODULE_EXPORT(reg_ctrl);
#endif

