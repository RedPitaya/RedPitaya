#ifndef _axi_protocol_converter_
#define _axi_protocol_converter_
#include <xtlm.h>
#include <utils/xtlm_aximm_passthru_module.h>
#include <systemc>

class axi_protocol_converter:public sc_module{
	public:
		axi_protocol_converter(sc_core::sc_module_name module_name,xsc::common_cpp::properties&);
		virtual ~axi_protocol_converter();
		SC_HAS_PROCESS(axi_protocol_converter);
	xtlm::xtlm_aximm_target_socket* target_rd_socket;
	xtlm::xtlm_aximm_target_socket* target_wr_socket;
	xtlm::xtlm_aximm_initiator_socket*  initiator_rd_socket;
	xtlm::xtlm_aximm_initiator_socket* initiator_wr_socket;
  	sc_in<bool> aclk;
	sc_in<bool> aresetn;
	private:
	xtlm::xtlm_aximm_passthru_module *P1;
	xtlm::xtlm_aximm_passthru_module *P2;	
};

#endif

