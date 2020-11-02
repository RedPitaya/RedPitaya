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


#include "system_processing_system7_0_0_sc.h"

#include "processing_system7_v5_5_tlm.h"

#include <map>
#include <string>

system_processing_system7_0_0_sc::system_processing_system7_0_0_sc(const sc_core::sc_module_name& nm) : sc_core::sc_module(nm), mp_impl(NULL)
{
  // configure connectivity manager
  xsc::utils::xsc_sim_manager::addInstance("system_processing_system7_0_0", this);

  // initialize module
    xsc::common_cpp::properties model_param_props;
    model_param_props.addLong("C_EN_EMIO_PJTAG", "0");
    model_param_props.addLong("C_EN_EMIO_ENET0", "0");
    model_param_props.addLong("C_EN_EMIO_ENET1", "0");
    model_param_props.addLong("C_EN_EMIO_TRACE", "0");
    model_param_props.addLong("C_INCLUDE_TRACE_BUFFER", "0");
    model_param_props.addLong("C_TRACE_BUFFER_FIFO_SIZE", "128");
    model_param_props.addLong("USE_TRACE_DATA_EDGE_DETECTOR", "0");
    model_param_props.addLong("C_TRACE_PIPELINE_WIDTH", "8");
    model_param_props.addLong("C_TRACE_BUFFER_CLOCK_DELAY", "12");
    model_param_props.addLong("C_EMIO_GPIO_WIDTH", "24");
    model_param_props.addLong("C_INCLUDE_ACP_TRANS_CHECK", "0");
    model_param_props.addLong("C_USE_DEFAULT_ACP_USER_VAL", "0");
    model_param_props.addLong("C_S_AXI_ACP_ARUSER_VAL", "31");
    model_param_props.addLong("C_S_AXI_ACP_AWUSER_VAL", "31");
    model_param_props.addLong("C_M_AXI_GP0_ID_WIDTH", "12");
    model_param_props.addLong("C_M_AXI_GP0_ENABLE_STATIC_REMAP", "0");
    model_param_props.addLong("C_M_AXI_GP1_ID_WIDTH", "12");
    model_param_props.addLong("C_M_AXI_GP1_ENABLE_STATIC_REMAP", "0");
    model_param_props.addLong("C_S_AXI_GP0_ID_WIDTH", "6");
    model_param_props.addLong("C_S_AXI_GP1_ID_WIDTH", "6");
    model_param_props.addLong("C_S_AXI_ACP_ID_WIDTH", "3");
    model_param_props.addLong("C_S_AXI_HP0_ID_WIDTH", "6");
    model_param_props.addLong("C_S_AXI_HP0_DATA_WIDTH", "64");
    model_param_props.addLong("C_S_AXI_HP1_ID_WIDTH", "6");
    model_param_props.addLong("C_S_AXI_HP1_DATA_WIDTH", "64");
    model_param_props.addLong("C_S_AXI_HP2_ID_WIDTH", "6");
    model_param_props.addLong("C_S_AXI_HP2_DATA_WIDTH", "64");
    model_param_props.addLong("C_S_AXI_HP3_ID_WIDTH", "6");
    model_param_props.addLong("C_S_AXI_HP3_DATA_WIDTH", "64");
    model_param_props.addLong("C_M_AXI_GP0_THREAD_ID_WIDTH", "12");
    model_param_props.addLong("C_M_AXI_GP1_THREAD_ID_WIDTH", "12");
    model_param_props.addLong("C_NUM_F2P_INTR_INPUTS", "2");
    model_param_props.addLong("C_DQ_WIDTH", "32");
    model_param_props.addLong("C_DQS_WIDTH", "4");
    model_param_props.addLong("C_DM_WIDTH", "4");
    model_param_props.addLong("C_MIO_PRIMITIVE", "54");
    model_param_props.addLong("C_TRACE_INTERNAL_WIDTH", "2");
    model_param_props.addLong("C_USE_AXI_NONSECURE", "0");
    model_param_props.addLong("C_USE_M_AXI_GP0", "1");
    model_param_props.addLong("C_USE_M_AXI_GP1", "0");
    model_param_props.addLong("C_USE_S_AXI_GP0", "0");
    model_param_props.addLong("C_USE_S_AXI_GP1", "0");
    model_param_props.addLong("C_USE_S_AXI_HP0", "1");
    model_param_props.addLong("C_USE_S_AXI_HP1", "0");
    model_param_props.addLong("C_USE_S_AXI_HP2", "0");
    model_param_props.addLong("C_USE_S_AXI_HP3", "0");
    model_param_props.addLong("C_USE_S_AXI_ACP", "0");
    model_param_props.addLong("C_GP0_EN_MODIFIABLE_TXN", "1");
    model_param_props.addLong("C_GP1_EN_MODIFIABLE_TXN", "1");
    model_param_props.addString("C_IRQ_F2P_MODE", "DIRECT");
    model_param_props.addString("C_PS7_SI_REV", "PRODUCTION");
    model_param_props.addString("C_FCLK_CLK0_BUF", "TRUE");
    model_param_props.addString("C_FCLK_CLK1_BUF", "TRUE");
    model_param_props.addString("C_FCLK_CLK2_BUF", "FALSE");
    model_param_props.addString("C_FCLK_CLK3_BUF", "FALSE");
    model_param_props.addString("C_PACKAGE_NAME", "clg400");

  mp_impl = new processing_system7_v5_5_tlm("inst", model_param_props);

  // initialize sockets
  M_AXI_GP0_rd_socket = mp_impl->M_AXI_GP0_rd_socket;
  M_AXI_GP0_wr_socket = mp_impl->M_AXI_GP0_wr_socket;
  S_AXI_HP0_rd_socket = mp_impl->S_AXI_HP0_rd_socket;
  S_AXI_HP0_wr_socket = mp_impl->S_AXI_HP0_wr_socket;
}

system_processing_system7_0_0_sc::~system_processing_system7_0_0_sc()
{
  xsc::utils::xsc_sim_manager::clean();

  delete mp_impl;
}

