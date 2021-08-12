#include "axi_bram_ctrl.h"

axi_bram_ctrl::axi_bram_ctrl(const sc_module_name& module_name, xsc::common_cpp::properties& properties) :
	sc_module(module_name) {
	//Get Width from property
	uint64_t data_width=properties.getLongLong("C_S_AXI_DATA_WIDTH");
  uint64_t dual_port=properties.getLongLong("C_SINGLE_PORT_BRAM");
  uint64_t ecc=properties.getLongLong("C_ECC");
  target_0_rd_socket = new xtlm::xtlm_aximm_target_socket("target_0_rd_socket", data_width);
	target_0_wr_socket = new xtlm::xtlm_aximm_target_socket("target_0_wr_socket", data_width);
	m_imp=new axi_bram_memory_imp("impl",properties);
	target_0_rd_socket->bind(*(m_imp->target_0_rd_socket));
	target_0_wr_socket->bind(*(m_imp->target_0_wr_socket));
  if(ecc==1)
  {
    s_axi_ctrl_target_rd_socket = new xtlm::xtlm_aximm_target_socket("target_1_rd_socket", 32);
	  s_axi_ctrl_target_wr_socket = new xtlm::xtlm_aximm_target_socket("target_1_wr_socket", 32);
    auto* stubWr = new xtlm::xtlm_aximm_target_stub("ifWrStubskt0", 32);
    s_axi_ctrl_target_wr_socket->bind(stubWr->target_socket);
    auto* stubRd = new xtlm::xtlm_aximm_target_stub("ifRdStubskt0", 32);
    s_axi_ctrl_target_rd_socket->bind(stubRd->target_socket);
    stubInitSkt.push_back(stubWr);
    stubInitSkt.push_back(stubRd);
  }
  if(dual_port==1)
  {
  bram_rst_b(bram_rst_b1);
  bram_clk_b(bram_clk_b1);
  bram_en_b(bram_en_b1);
  bram_we_b(bram_we_b1); 
  bram_addr_b(bram_addr_b1);
  bram_wrdata_b(bram_wrdata_b1);
  bram_rddata_b(bram_rddata_b1);
  }

  SC_THREAD(clock_finder);
  sensitive<< s_axi_aclk;
  dont_initialize();
  
  }
axi_bram_ctrl::~axi_bram_ctrl() {
	delete target_0_rd_socket;
	delete target_0_wr_socket;
	delete m_imp;
}
void axi_bram_ctrl::clock_finder() {
  sc_time clk1 = sc_time_stamp();
  wait();
  sc_time clk2= sc_time_stamp();
}
// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689
