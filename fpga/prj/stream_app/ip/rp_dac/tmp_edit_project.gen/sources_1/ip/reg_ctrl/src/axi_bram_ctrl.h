#ifndef _SIMHBMX_H_
#define _SIMHBMX_H_

#include "xtlm.h"
#include <list>

#include "axi_bram_memory_imp.h"
#include "utils/xtlm_aximm_target_stub.h"
#include "utils/xsc_stub_port.h"

class axi_bram_ctrl: public sc_core::sc_module {
public:
	axi_bram_ctrl(const sc_module_name&  module_name,xsc::common_cpp::properties&);
	virtual ~axi_bram_ctrl();
	SC_HAS_PROCESS(axi_bram_ctrl);
	//Slave interface used to R/W to HBM Memory
	xtlm::xtlm_aximm_target_socket*    target_0_rd_socket;
	xtlm::xtlm_aximm_target_socket*    target_0_wr_socket;
  xtlm::xtlm_aximm_target_socket*    s_axi_ctrl_target_rd_socket;
	xtlm::xtlm_aximm_target_socket*    s_axi_ctrl_target_wr_socket;
  std::vector<xtlm::xtlm_aximm_target_stub*> stubInitSkt;
  void clock_finder();
	sc_in<bool>                        s_axi_aclk;
	sc_in<bool>                        s_axi_aresetn;
  xsc::utils::xsc_stub_port ecc_interrupt;
  xsc::utils::xsc_stub_port ecc_ue;  
  xsc::utils::xsc_stub_port bram_rst_a;
  xsc::utils::xsc_stub_port bram_clk_a;
  xsc::utils::xsc_stub_port bram_en_a;
  xsc::utils::xsc_stub_port bram_we_a;
  xsc::utils::xsc_stub_port bram_addr_a;
  xsc::utils::xsc_stub_port bram_wrdata_a;
  xsc::utils::xsc_stub_port bram_rddata_a;
  xsc::utils::xsc_stub_port bram_rst_b;
  xsc::utils::xsc_stub_port bram_clk_b;
  xsc::utils::xsc_stub_port bram_en_b;
  xsc::utils::xsc_stub_port bram_we_b;
  xsc::utils::xsc_stub_port bram_addr_b;
  xsc::utils::xsc_stub_port bram_wrdata_b;
  xsc::utils::xsc_stub_port bram_rddata_b;
  sc_core::sc_signal<bool>bram_rst_b1;
  sc_core::sc_signal<bool>bram_clk_b1;
  sc_core::sc_signal<bool>bram_en_b1;
  sc_core::sc_signal<bool>bram_we_b1;
  sc_core::sc_signal<bool>bram_addr_b1;
  sc_core::sc_signal<bool>bram_wrdata_b1;
  sc_core::sc_signal<bool>bram_rddata_b1;

 
private :
	axi_bram_memory_imp* m_imp;
};

#endif /* _SIMHBMX_H_ */

// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689
