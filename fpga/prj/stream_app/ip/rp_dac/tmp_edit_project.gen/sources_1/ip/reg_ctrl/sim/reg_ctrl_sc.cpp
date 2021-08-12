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


#include "reg_ctrl_sc.h"

#include "axi_bram_ctrl.h"

#include <map>
#include <string>

reg_ctrl_sc::reg_ctrl_sc(const sc_core::sc_module_name& nm) : sc_core::sc_module(nm), mp_impl(NULL)
{
  // configure connectivity manager
  xsc::utils::xsc_sim_manager::addInstance("reg_ctrl", this);

  // initialize module
    xsc::common_cpp::properties model_param_props;
    model_param_props.addLong("C_MEMORY_DEPTH", "1024");
    model_param_props.addLong("C_BRAM_ADDR_WIDTH", "10");
    model_param_props.addLong("C_S_AXI_ADDR_WIDTH", "12");
    model_param_props.addLong("C_S_AXI_DATA_WIDTH", "32");
    model_param_props.addLong("C_S_AXI_ID_WIDTH", "1");
    model_param_props.addLong("C_S_AXI_SUPPORTS_NARROW_BURST", "0");
    model_param_props.addLong("C_SINGLE_PORT_BRAM", "1");
    model_param_props.addLong("C_READ_LATENCY", "1");
    model_param_props.addLong("C_RD_CMD_OPTIMIZATION", "0");
    model_param_props.addLong("C_S_AXI_CTRL_ADDR_WIDTH", "32");
    model_param_props.addLong("C_S_AXI_CTRL_DATA_WIDTH", "32");
    model_param_props.addLong("C_ECC", "0");
    model_param_props.addLong("C_ECC_TYPE", "0");
    model_param_props.addLong("C_FAULT_INJECT", "0");
    model_param_props.addLong("C_ECC_ONOFF_RESET_VALUE", "0");
    model_param_props.addString("C_BRAM_INST_MODE", "EXTERNAL");
    model_param_props.addString("C_S_AXI_PROTOCOL", "AXI4LITE");
    model_param_props.addString("C_FAMILY", "zynq");

  mp_impl = new axi_bram_ctrl("inst", model_param_props);

  // initialize AXI sockets
  target_0_rd_socket = mp_impl->target_0_rd_socket;
  target_0_wr_socket = mp_impl->target_0_wr_socket;
}

reg_ctrl_sc::~reg_ctrl_sc()
{
  xsc::utils::xsc_sim_manager::clean();

  delete mp_impl;
}

