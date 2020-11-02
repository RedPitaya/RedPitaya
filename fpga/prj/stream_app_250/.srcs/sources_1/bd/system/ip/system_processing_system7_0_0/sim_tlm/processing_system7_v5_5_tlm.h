

// (c) Copyright 1995-2013 Xilinx, Inc. All rights reserved.
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


// IP VLNV: xilinx.com:ip:processing_system7_vip:1.0
// IP Revision: 1
#ifndef __PS7_H__
#define __PS7_H__

#include "systemc.h"
#include "xtlm.h"
#include "xtlm_adaptors/xaximm_xtlm2tlm.h"
#include "xtlm_adaptors/xaximm_tlm2xtlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "genattr.h"
#include "xilinx-zynq.h"
#include "b_transport_converter.h"

/***************************************************************************************
*
* A Simple Converter which converts Remote-port's simplae_intiator_sockets<32>->b_transport()
* calls to xTLM sockets bn_transport_x() calls..
* 
* This is Only specific to remote-port so not creating seperate header for it.
*
***************************************************************************************/
template <int IN_WIDTH, int OUT_WIDTH>
class rptlm2xtlm_converter : public sc_module{
    public:
    tlm::tlm_target_socket<IN_WIDTH> target_socket;
    xtlm::xtlm_aximm_initiator_socket wr_socket;
    xtlm::xtlm_aximm_initiator_socket rd_socket;
    rptlm2xtlm_converter<IN_WIDTH, OUT_WIDTH>(sc_module_name name);//:sc_module(name)
	void registerUserExtensionHandlerCallback(
			void (*callback)(xtlm::aximm_payload*,
					const tlm::tlm_generic_payload*));

    private:
    b_transport_converter<IN_WIDTH, OUT_WIDTH> m_btrans_conv;
    xtlm::xaximm_tlm2xtlm_t<OUT_WIDTH> xtlm_bridge;
};

/***************************************************************************************
*   Global method, get registered with tlm2xtlm bridge
*   This function is called when tlm2xtlm bridge convert tlm payload to xtlm payload.
*
*   caller:     tlm2xtlm bridge
*   purpose:    To get master id and other parameters out of genattr_extension 
*               and use master id to AxUSER PIN of xtlm payload.
*
*
***************************************************************************************/
extern void get_extensions_from_tlm(xtlm::aximm_payload* xtlm_pay, const tlm::tlm_generic_payload* gp);

/***************************************************************************************
*   Global method, get registered with xtlm2tlm bridge
*   This function is called when xtlm2tlm bridge convert xtlm payload to tlm payload.
*
*   caller:     xtlm2tlm bridge
*   purpose:    To create and add master id and other parameters to genattr_extension.
*               Master id red from AxID PIN of xtlm payload.
*
*
***************************************************************************************/
extern void add_extensions_to_tlm(const xtlm::aximm_payload* xtlm_pay, tlm::tlm_generic_payload* gp);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                              //
// File:            processing_system7_tlm.h                                                                       //
//                                                                                                              //
// Description:     zynq_ultra_ps_e_tlm class is a sc_module, act as intermediate layer between                 //
//                  xilinx_zynq qemu wrapper and Vivado generated systemc simulation ip wrapper.              //
//                  it's basically created for supporting tlm based xilinx_zynq from xtlm based vivado        //
//                  generated systemc wrapper. this wrapper is live only when SELECTED_SIM_MODEL is set         //
//                  to tlm. it's also act as bridge between vivado wrapper and xilinx_zynq wrapper.           //
//                  it fill the the gap between input/output ports of vivado generated wrapper to               //
//                  xilinx_zynq wrapper signals. This wrapper is auto generated by ttcl scripts               //
//                  based on IP configuration in vivado.                                                        //
//                                                                                                              //
//                                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class processing_system7_v5_5_tlm : public sc_core::sc_module   {
    
    public:
    // Non-AXI ports are declared here
    sc_core::sc_in<sc_dt::sc_bv<24> >  GPIO_I;
    sc_core::sc_out<sc_dt::sc_bv<24> >  GPIO_O;
    sc_core::sc_out<sc_dt::sc_bv<24> >  GPIO_T;
    sc_core::sc_out<bool> TTC0_WAVE0_OUT;
    sc_core::sc_out<bool> TTC0_WAVE1_OUT;
    sc_core::sc_out<bool> TTC0_WAVE2_OUT;
    sc_core::sc_out<sc_dt::sc_bv<2> >  USB0_PORT_INDCTL;
    sc_core::sc_out<bool> USB0_VBUS_PWRSELECT;
    sc_core::sc_in<bool> USB0_VBUS_PWRFAULT;
    sc_core::sc_in<bool> M_AXI_GP0_ACLK;
    sc_core::sc_out<sc_dt::sc_bv<8> >  S_AXI_HP0_RCOUNT;
    sc_core::sc_out<sc_dt::sc_bv<8> >  S_AXI_HP0_WCOUNT;
    sc_core::sc_out<sc_dt::sc_bv<3> >  S_AXI_HP0_RACOUNT;
    sc_core::sc_out<sc_dt::sc_bv<6> >  S_AXI_HP0_WACOUNT;
    sc_core::sc_in<bool> S_AXI_HP0_ACLK;
    sc_core::sc_in<bool> S_AXI_HP0_RDISSUECAP1_EN;
    sc_core::sc_in<bool> S_AXI_HP0_WRISSUECAP1_EN;
    sc_core::sc_in<sc_dt::sc_bv<2> >  IRQ_F2P;
    sc_core::sc_out<bool> FCLK_CLK0;
    sc_core::sc_out<bool> FCLK_CLK1;
    sc_core::sc_out<bool> FCLK_RESET0_N;
    sc_core::sc_inout<sc_dt::sc_bv<54> >  MIO;
    sc_core::sc_inout<bool> DDR_CAS_n;
    sc_core::sc_inout<bool> DDR_CKE;
    sc_core::sc_inout<bool> DDR_Clk_n;
    sc_core::sc_inout<bool> DDR_Clk;
    sc_core::sc_inout<bool> DDR_CS_n;
    sc_core::sc_inout<bool> DDR_DRSTB;
    sc_core::sc_inout<bool> DDR_ODT;
    sc_core::sc_inout<bool> DDR_RAS_n;
    sc_core::sc_inout<bool> DDR_WEB;
    sc_core::sc_inout<sc_dt::sc_bv<3> >  DDR_BankAddr;
    sc_core::sc_inout<sc_dt::sc_bv<15> >  DDR_Addr;
    sc_core::sc_inout<bool> DDR_VRN;
    sc_core::sc_inout<bool> DDR_VRP;
    sc_core::sc_inout<sc_dt::sc_bv<4> >  DDR_DM;
    sc_core::sc_inout<sc_dt::sc_bv<32> >  DDR_DQ;
    sc_core::sc_inout<sc_dt::sc_bv<4> >  DDR_DQS_n;
    sc_core::sc_inout<sc_dt::sc_bv<4> >  DDR_DQS;
    sc_core::sc_inout<bool> PS_SRSTB;
    sc_core::sc_inout<bool> PS_CLK;
    sc_core::sc_inout<bool> PS_PORB;

    xtlm::xtlm_aximm_initiator_socket*      M_AXI_GP0_wr_socket;
    xtlm::xtlm_aximm_initiator_socket*      M_AXI_GP0_rd_socket;
    xtlm::xtlm_aximm_target_socket*         S_AXI_HP0_wr_socket;
    xtlm::xtlm_aximm_target_socket*         S_AXI_HP0_rd_socket;

    //constructor having three paramters
    // 1. module name in sc_module_name objec, 
    // 2. reference to map object of name and integer value pairs 
    // 3. reference to map object of name and string value pairs
    // All the model parameters (integer and string) which are configuration parameters 
    // of Processing System 7 IP propogated from Vivado
processing_system7_v5_5_tlm(sc_core::sc_module_name name,
    xsc::common_cpp::properties&);
    
    ~processing_system7_v5_5_tlm();
    SC_HAS_PROCESS(processing_system7_v5_5_tlm);
    
    private:
    
    //zynq tlm wrapper provided by Edgar
    //module with interfaces of standard tlm 
    //and input/output ports at signal level
    xilinx_zynq* m_zynq_tlm_model;

    // Xtlm2tlm_t Bridges
    // Converts Xtlm transactions to tlm transactions
    // Bridge's Xtlm wr/rd target sockets binds with 
    // xtlm initiator sockets of processing_system7_tlm and tlm simple initiator 
    // socket with xilinx_zynq's target socket
    xtlm::xaximm_xtlm2tlm_t<64,32> S_AXI_HP0_xtlm_brdg;
    zynq_tlm::xsc_xtlm_aximm_tran_buffer *S_AXI_HP0_buff;

    // This Bridges converts b_transport to nb_transports and also
    // Converts tlm transactions to xtlm transactions.
    // Bridge's tlm simple target socket binds with 
    // simple initiator socket of xilinx_zynqmp and xtlm 
    // socket with xilinx_zynq's simple target socket
    rptlm2xtlm_converter<32, 32> m_rp_bridge_M_AXI_GP0;     
    
    // sc_clocks for generating pl clocks
    // output pins FCLK_CLK0..3 are drived by these clocks
    sc_core::sc_clock FCLK_CLK0_clk;
    sc_core::sc_clock FCLK_CLK1_clk;

    
    //Method which is sentive to FCLK_CLK0_clk sc_clock object
    //FCLK_CLK0 pin written based on FCLK_CLK0_clk clock value 
    void trigger_FCLK_CLK0_pin();
    //Method which is sentive to FCLK_CLK1_clk sc_clock object
    //FCLK_CLK1 pin written based on FCLK_CLK1_clk clock value 
    void trigger_FCLK_CLK1_pin();
    
    void IRQ_F2P_method();
    //FCLK_RESET0 output reset pin get toggle when emio bank 2's 31th signal gets toggled
    //EMIO[2] bank 31th(GPIO[95] signal)acts as reset signal to the PL(refer Zynq UltraScale+ TRM, page no:761)
    void FCLK_RESET0_N_trigger();

    sc_signal<bool> qemu_rst;
    void start_of_simulation();

    xsc::common_cpp::properties prop;

};
#endif
