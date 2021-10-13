// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689
#ifndef _SIM_HBMX_IMP_H_
#define _SIM_HBMX_IMP_H_

#include "xtlm.h"
#include "report_handler.h"
#include <list>
#include "axi_bram_fmodel_base.h"
#include "axi_bram_ram_fmodel.h"
#include "axi_bram_fmodel_shared_memory.h"

class axi_bram_memory_imp: public sc_core::sc_module {
public:
	//Workaround to implement Debug transport
	class target_rd_util;
	class target_wr_util;
	axi_bram_memory_imp(const sc_module_name& module_name,
			xsc::common_cpp::properties&);
	virtual ~axi_bram_memory_imp();SC_HAS_PROCESS(axi_bram_memory_imp);
	//Slave interface used to R/W to HBM Memory
	xtlm::xtlm_aximm_target_socket* target_0_rd_socket;
	xtlm::xtlm_aximm_target_socket* target_0_wr_socket;
private:
	target_rd_util* m_target_0_rd_socket_util;
	target_wr_util* m_target_0_wr_socket_util;
	uint64_t m_mem_addr_width;
	uint64_t m_data_width;
	std::list<xtlm::aximm_payload*> m_rd_req_vec;
	std::list<xtlm::aximm_payload*> m_wr_req_vec;
	std::list<xtlm::aximm_payload*> m_wr_data_vec;
	std::list<xtlm::aximm_payload*> m_wr_resp_vec;
	void methodProcessTransactionRead();
	void methodProcessTransactionWrite();
	xsc::common_cpp::report_handler* m_report_handler;
	axi_bram_fmodel_base* m_mem_handler;
	unsigned int transport_dbg(xtlm::aximm_payload&);
	unsigned int transport_dbg_wr(xtlm::aximm_payload&);
	unsigned int transport_dbg_rd(xtlm::aximm_payload&);
};

#endif /* _SIM_HBMX_IMP_H_ */

