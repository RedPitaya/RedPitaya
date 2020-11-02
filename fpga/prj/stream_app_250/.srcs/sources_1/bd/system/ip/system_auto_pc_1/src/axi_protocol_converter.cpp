#include "axi_protocol_converter.h"
#include <sstream>

axi_protocol_converter::axi_protocol_converter(sc_core::sc_module_name module_name,xsc::common_cpp::properties&) :
	sc_module(module_name) {
		initiator_rd_socket = new xtlm::xtlm_aximm_initiator_socket("initiator_rd_socket",32);
		initiator_wr_socket = new xtlm::xtlm_aximm_initiator_socket("initiator_wr_socket",32);
	 	target_rd_socket = new xtlm::xtlm_aximm_target_socket("target_rd_socket",32);
		target_wr_socket = new xtlm::xtlm_aximm_target_socket("target_wr_socket",32);
		P1 = new xtlm::xtlm_aximm_passthru_module("P1");
		P2 = new xtlm::xtlm_aximm_passthru_module("P2");
		P1->initiator_socket->bind(*initiator_rd_socket);
		P2->initiator_socket->bind(*initiator_wr_socket);
		target_rd_socket->bind(*(P1->target_socket));
		target_wr_socket->bind(*(P2->target_socket));
		}

axi_protocol_converter::~axi_protocol_converter() {
	delete initiator_wr_socket;
	delete initiator_rd_socket;
	delete target_wr_socket;
	delete target_rd_socket;
	delete P1;
	delete P2;
}
