
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

#include"processing_system7_v5_5_tlm.h"
#include<string>

template <int IN_WIDTH, int OUT_WIDTH>
rptlm2xtlm_converter<IN_WIDTH, OUT_WIDTH>::rptlm2xtlm_converter(sc_module_name name):sc_module(name)
    ,target_socket("target_socket")
    ,wr_socket("init_wr_socket",OUT_WIDTH)
    ,rd_socket("init_rd_socket",OUT_WIDTH)
    ,m_btrans_conv("b_transport_converter")
    ,xtlm_bridge("tlm2xtlmbridge")
{
    target_socket.bind(m_btrans_conv.target_socket);
    m_btrans_conv.initiator_socket.bind(xtlm_bridge.target_socket);
    xtlm_bridge.rd_socket->bind(rd_socket);
    xtlm_bridge.wr_socket->bind(wr_socket);
}
template <int IN_WIDTH, int OUT_WIDTH>
void rptlm2xtlm_converter<IN_WIDTH, OUT_WIDTH>::registerUserExtensionHandlerCallback(
		void (*callback)(xtlm::aximm_payload*,
				const tlm::tlm_generic_payload*)) {
    xtlm_bridge.registerUserExtensionHandlerCallback(callback);
}

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

void get_extensions_from_tlm(xtlm::aximm_payload* xtlm_pay, const tlm::tlm_generic_payload* gp)
{
    if((xtlm_pay == NULL) || (gp == NULL))
        return;
    if((gp->get_command() == tlm::TLM_WRITE_COMMAND) && (xtlm_pay->get_awuser_size() > 0))
    {
        genattr_extension* ext = NULL;
        gp->get_extension(ext);
        if(ext == NULL)
            return;
        //Portion of master ID(master_id[5:0]) are transfered on AxUSER bits(refere Zynq UltraScale+ TRM page.no:414)
        uint32_t val = ext->get_master_id() && 0x3F;
        unsigned char* ptr = xtlm_pay->get_awuser_ptr();
        unsigned int size  = xtlm_pay->get_awuser_size();
        *ptr = (unsigned char)val;

    }
    else if((gp->get_command() == tlm::TLM_READ_COMMAND) && (xtlm_pay->get_aruser_size() > 0))
    {
        genattr_extension* ext = NULL;
        gp->get_extension(ext);
        if(ext == NULL)
            return;
        //Portion of master ID(master_id[5:0]) are transfered on AxUSER bits(refere Zynq UltraScale+ TRM page.no:414)
        uint32_t val = ext->get_master_id() && 0x3F;
        unsigned char* ptr = xtlm_pay->get_aruser_ptr();
        unsigned int size  = xtlm_pay->get_aruser_size();
        *ptr = (unsigned char)val;
    }
}

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
void add_extensions_to_tlm(const xtlm::aximm_payload* xtlm_pay, tlm::tlm_generic_payload* gp)
{
    if(gp == NULL)
        return;
    uint8_t val = 0;
    if((gp->get_command() != tlm::TLM_WRITE_COMMAND) && (gp->get_command() != tlm::TLM_READ_COMMAND))
        return;
    //portion of master ID bits(master_id[5:0]) are derived from the AXI ID(AWID/ARID). (refere Zynq UltraScale+ TRM page.no:414,415)
    //val = (*(uint8_t*)(xtlm_pay->get_axi_id())) && 0x3F;
    genattr_extension* ext = new genattr_extension;
    ext->set_master_id(val);
    gp->set_extension(ext);    
    gp->set_streaming_width(gp->get_data_length());
    if(gp->get_command() != tlm::TLM_WRITE_COMMAND)
    {
        gp->set_byte_enable_length(0);
        gp->set_byte_enable_ptr(0);
    }
}

processing_system7_v5_5_tlm :: processing_system7_v5_5_tlm (sc_core::sc_module_name name,
    xsc::common_cpp::properties& _prop): sc_module(name)//registering module name with parent
        ,GPIO_I("GPIO_I")
        ,GPIO_O("GPIO_O")
        ,GPIO_T("GPIO_T")
        ,TTC0_WAVE0_OUT("TTC0_WAVE0_OUT")
        ,TTC0_WAVE1_OUT("TTC0_WAVE1_OUT")
        ,TTC0_WAVE2_OUT("TTC0_WAVE2_OUT")
        ,USB0_PORT_INDCTL("USB0_PORT_INDCTL")
        ,USB0_VBUS_PWRSELECT("USB0_VBUS_PWRSELECT")
        ,USB0_VBUS_PWRFAULT("USB0_VBUS_PWRFAULT")
        ,M_AXI_GP0_ACLK("M_AXI_GP0_ACLK")
        ,S_AXI_HP0_RCOUNT("S_AXI_HP0_RCOUNT")
        ,S_AXI_HP0_WCOUNT("S_AXI_HP0_WCOUNT")
        ,S_AXI_HP0_RACOUNT("S_AXI_HP0_RACOUNT")
        ,S_AXI_HP0_WACOUNT("S_AXI_HP0_WACOUNT")
        ,S_AXI_HP0_ACLK("S_AXI_HP0_ACLK")
        ,S_AXI_HP0_RDISSUECAP1_EN("S_AXI_HP0_RDISSUECAP1_EN")
        ,S_AXI_HP0_WRISSUECAP1_EN("S_AXI_HP0_WRISSUECAP1_EN")
        ,IRQ_F2P("IRQ_F2P")
        ,FCLK_CLK0("FCLK_CLK0")
        ,FCLK_CLK1("FCLK_CLK1")
        ,FCLK_RESET0_N("FCLK_RESET0_N")
        ,MIO("MIO")
        ,DDR_CAS_n("DDR_CAS_n")
        ,DDR_CKE("DDR_CKE")
        ,DDR_Clk_n("DDR_Clk_n")
        ,DDR_Clk("DDR_Clk")
        ,DDR_CS_n("DDR_CS_n")
        ,DDR_DRSTB("DDR_DRSTB")
        ,DDR_ODT("DDR_ODT")
        ,DDR_RAS_n("DDR_RAS_n")
        ,DDR_WEB("DDR_WEB")
        ,DDR_BankAddr("DDR_BankAddr")
        ,DDR_Addr("DDR_Addr")
        ,DDR_VRN("DDR_VRN")
        ,DDR_VRP("DDR_VRP")
        ,DDR_DM("DDR_DM")
        ,DDR_DQ("DDR_DQ")
        ,DDR_DQS_n("DDR_DQS_n")
        ,DDR_DQS("DDR_DQS")
        ,PS_SRSTB("PS_SRSTB")
        ,PS_CLK("PS_CLK")
        ,PS_PORB("PS_PORB")
    ,S_AXI_HP0_xtlm_brdg("S_AXI_HP0_xtlm_brdg")
    ,m_rp_bridge_M_AXI_GP0("m_rp_bridge_M_AXI_GP0")     
        ,FCLK_CLK0_clk("FCLK_CLK0_clk", sc_time(20000.0,sc_core::SC_PS))//clock period in picoseconds = 1000000/freq(in MZ)
        ,FCLK_CLK1_clk("FCLK_CLK1_clk", sc_time(40000.0,sc_core::SC_PS))//clock period in picoseconds = 1000000/freq(in MZ)
    ,prop(_prop)
    {
        //creating instances of xtlm slave sockets
        S_AXI_HP0_wr_socket = new xtlm::xtlm_aximm_target_socket("S_AXI_HP0_wr_socket", 64);
        S_AXI_HP0_rd_socket = new xtlm::xtlm_aximm_target_socket("S_AXI_HP0_rd_socket", 64);
        //creating instances of xtlm master sockets
        M_AXI_GP0_wr_socket = new xtlm::xtlm_aximm_initiator_socket("M_AXI_GP0_wr_socket", 32);
        M_AXI_GP0_rd_socket = new xtlm::xtlm_aximm_initiator_socket("M_AXI_GP0_rd_socket", 32);

	    char* unix_path = getenv("COSIM_MACHINE_PATH");
	    char* tcpip_addr = getenv("COSIM_MACHINE_TCPIP_ADDRESS");
	    char* dir_path_to_test_machine;
	    bool unix_socket_en = false;        
	    if (unix_path != nullptr) {
	    	dir_path_to_test_machine = strdup(unix_path);
	    	unix_socket_en = true;
	    }
	    if ((unix_socket_en == false) && (tcpip_addr != nullptr)) {
	    	dir_path_to_test_machine = strdup(tcpip_addr);
	    } else if (unix_socket_en == false) {
	    	printf(
	    			"ERROR: Environment Variables Either COSIM_MACHINE_TCPIP_ADDRESS or COSIM_MACHINE_PATH is not specified.\n 1. Specify  COSIM_MACHINE_PATH for Unix Socket Communication.\n 2. Specify COSIM_MACHINE_TCPIP_ADDRESS for TCP Socket Communication.\n");
	    	exit(0);
	    }
	    std::string skt_name;
	    if (unix_socket_en) {
	    	skt_name.append("unix:");
	    	skt_name.append(dir_path_to_test_machine);
	    	skt_name.append("//qemu-rport-_cosim@0");
	    } else {
	    	skt_name.append(dir_path_to_test_machine);
	    }

	    const char* skt = skt_name.c_str();
        m_zynq_tlm_model = new xilinx_zynq("xilinx_zynq",skt);

        //instantiating XTLM2TLM bridge and stiching it between 
        //S_AXI_HP0_wr_socket/rd_socket sockets to s_axi_hp[0] target socket of Zynq Qemu tlm wrapper
        S_AXI_HP0_buff = new zynq_tlm::xsc_xtlm_aximm_tran_buffer("S_AXI_HP0_buff");
        S_AXI_HP0_rd_socket->bind(*S_AXI_HP0_buff->in_rd_socket);
        S_AXI_HP0_wr_socket->bind(*S_AXI_HP0_buff->in_wr_socket);
        S_AXI_HP0_buff->out_wr_socket->bind(*S_AXI_HP0_xtlm_brdg.wr_socket);
        S_AXI_HP0_buff->out_rd_socket->bind(*S_AXI_HP0_xtlm_brdg.rd_socket);
        m_zynq_tlm_model->s_axi_hp[0]->bind(S_AXI_HP0_xtlm_brdg.initiator_socket);

        //instantiating TLM2XTLM bridge and stiching it between 
        //s_axi_gp[0] initiator socket of zynq Qemu tlm wrapper to M_AXI_GP0_wr_socket/rd_socket sockets 
        m_rp_bridge_M_AXI_GP0.wr_socket->bind(*M_AXI_GP0_wr_socket);
        m_rp_bridge_M_AXI_GP0.rd_socket->bind(*M_AXI_GP0_rd_socket);
        m_rp_bridge_M_AXI_GP0.target_socket.bind(*m_zynq_tlm_model->m_axi_gp[0]);

        m_zynq_tlm_model->tie_off();
        
 
        SC_METHOD(IRQ_F2P_method);
        sensitive << IRQ_F2P ;
        dont_initialize();

        SC_METHOD(trigger_FCLK_CLK0_pin);
        sensitive << FCLK_CLK0_clk;
        dont_initialize();
        SC_METHOD(trigger_FCLK_CLK1_pin);
        sensitive << FCLK_CLK1_clk;
        dont_initialize();
        S_AXI_HP0_xtlm_brdg.registerUserExtensionHandlerCallback(&add_extensions_to_tlm);
        m_rp_bridge_M_AXI_GP0.registerUserExtensionHandlerCallback(&get_extensions_from_tlm);
        m_zynq_tlm_model->rst(qemu_rst);
    }
processing_system7_v5_5_tlm :: ~processing_system7_v5_5_tlm() {
        //deleteing dynamically created objects 
        delete S_AXI_HP0_wr_socket;
        delete S_AXI_HP0_rd_socket;
        delete S_AXI_HP0_buff;
        delete M_AXI_GP0_wr_socket;
        delete M_AXI_GP0_rd_socket;
    }
    
    //Method which is sentive to FCLK_CLK0_clk sc_clock object
    //FCLK_CLK0 pin written based on FCLK_CLK0_clk clock value 
    void processing_system7_v5_5_tlm ::trigger_FCLK_CLK0_pin()    {
        FCLK_CLK0.write(FCLK_CLK0_clk.read());
    }
    //Method which is sentive to FCLK_CLK1_clk sc_clock object
    //FCLK_CLK1 pin written based on FCLK_CLK1_clk clock value 
    void processing_system7_v5_5_tlm ::trigger_FCLK_CLK1_pin()    {
        FCLK_CLK1.write(FCLK_CLK1_clk.read());
    }
    void processing_system7_v5_5_tlm ::IRQ_F2P_method()    {
        int irq = ((IRQ_F2P.read().to_uint()) & 0xFFFF);
        for(int i = 0; i < prop.getLongLong("C_NUM_F2P_INTR_INPUTS"); i++)   {
            if(irq & (0x1<<i))  {
                m_zynq_tlm_model->pl2ps_irq[i].write(true);
            }
            else{
                m_zynq_tlm_model->pl2ps_irq[i].write(false);
            }
        }
    }
    //ps2pl_rst[0] output reset pin
    void processing_system7_v5_5_tlm :: FCLK_RESET0_N_trigger()   {
        FCLK_RESET0_N.write(m_zynq_tlm_model->ps2pl_rst[0].read());
    }
    void processing_system7_v5_5_tlm ::start_of_simulation()
    {
    //temporary fix to drive the enabled reset pin 
        FCLK_RESET0_N.write(true);
        qemu_rst.write(false);
    }
