// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689
#include "axi_bram_memory_imp.h"
#include "P2P_XTLM_extension.h"

#include <sstream>
#include <bitset>

class axi_bram_memory_imp::target_rd_util: public xtlm::xtlm_aximm_target_rd_socket_util {
public:
	target_rd_util(sc_core::sc_module_name p_name,
			xtlm::aximm::granularity g_hint, int width_p,
			axi_bram_memory_imp* _parent) :
			xtlm::xtlm_aximm_target_rd_socket_util(p_name, g_hint, width_p) {
		this->parent = _parent;
	}
	unsigned int transport_dbg_cb(xtlm::aximm_payload& trans) {
		return parent->transport_dbg(trans);
	}
private:
	axi_bram_memory_imp* parent;
};

class axi_bram_memory_imp::target_wr_util: public xtlm::xtlm_aximm_target_wr_socket_util {
public:
	target_wr_util(sc_core::sc_module_name p_name,
			xtlm::aximm::granularity g_hint, int width_p,
			axi_bram_memory_imp* _parent) :
			xtlm::xtlm_aximm_target_wr_socket_util(p_name, g_hint, width_p) {
		this->parent = _parent;
	}
	unsigned int transport_dbg_cb(xtlm::aximm_payload& trans) {
		return parent->transport_dbg(trans);
	}
private:
	axi_bram_memory_imp* parent;
};

axi_bram_memory_imp::axi_bram_memory_imp(
		const sc_module_name& module_name,
		xsc::common_cpp::properties& properties) :
		sc_module(module_name) {
	m_data_width = properties.getLongLong("C_S_AXI_DATA_WIDTH");
	m_mem_addr_width = properties.getLongLong("C_MEMORY_DEPTH");
	m_mem_addr_width *= m_data_width;

	target_0_rd_socket = new xtlm::xtlm_aximm_target_socket(
			"saxi_hbmx_imp_rd_socket", m_data_width);
	target_0_wr_socket = new xtlm::xtlm_aximm_target_socket("saxi_cq_wrie_skt",
			m_data_width);
	m_target_0_rd_socket_util = new target_rd_util(
			"saxi_hbmx_imp_rd_socket_util",
			xtlm::aximm::granularity::TRANSACTION, m_data_width, this);
	m_target_0_wr_socket_util = new target_wr_util(
			"saxi_hbmx_imp_wr_socket_util",
			xtlm::aximm::granularity::TRANSACTION, m_data_width, this);
	target_0_rd_socket->bind((m_target_0_rd_socket_util->rd_socket));
	target_0_wr_socket->bind((m_target_0_wr_socket_util->wr_socket));
	std::string mod_name = name();
	m_report_handler = new xsc::common_cpp::report_handler(mod_name);
	if (!getenv("SDX_USE_SHARED_MEMORY"))
		m_mem_handler = new axi_bram_ram_fmodel(this->name(),
				m_report_handler, m_mem_addr_width); //name() returns hierarchical name of ddr systemc module instance
	else
		m_mem_handler = new axi_bram_fmodel_shared_memory(this->name(),
				m_report_handler, m_mem_addr_width); //name() returns hierarchical name of ddr systemc module instance

	//m_mem_handler=new axi_bram_memory_fmodel_base(mod_name,m_report_handler,m_mem_addr_width);

	SC_METHOD(methodProcessTransactionRead);
	sensitive << m_target_0_rd_socket_util->transaction_available;
	sensitive << m_target_0_rd_socket_util->data_sampled;
	dont_initialize();

	//m_report_handler->set_verbosity_level(xsc::common_cpp::VERBOSITY::DEBUG);
	SC_METHOD(methodProcessTransactionWrite);
	sensitive << m_target_0_wr_socket_util->transaction_available;
	sensitive << m_target_0_wr_socket_util->resp_sampled;
	dont_initialize();
}

axi_bram_memory_imp::~axi_bram_memory_imp() {
	delete target_0_rd_socket;
	delete target_0_wr_socket;
	delete m_target_0_rd_socket_util;
	delete m_target_0_wr_socket_util;
	delete m_report_handler;
	delete m_mem_handler;
}

void axi_bram_memory_imp::methodProcessTransactionRead() {
	std::stringstream ss;
	if (m_target_0_rd_socket_util->is_trans_available()) {
		xtlm::aximm_payload* trans =
				m_target_0_rd_socket_util->get_transaction();
		sc_core::sc_time delay = SC_ZERO_TIME;
		uint64_t max_address = m_mem_addr_width;
		uint64_t address = trans->get_address() % max_address;
		uint64_t tranSize = trans->get_data_length();
		ss.clear();
		std::string des;
		if (m_report_handler->get_verbosity_level() == xsc::common_cpp::DEBUG) {
			trans->get_log(des, 1);
			ss << name() << ": Received Read Request " << des << std::endl;
			XSC_REPORT_INFO_VERB((*m_report_handler), "1", ss.str().c_str(),
					DEBUG);
		}
		if ((address + tranSize) > max_address) {
			ss.clear();
			ss << name() << ": Invalid Read Request " << std::endl << des
					<< std::endl;
			XSC_REPORT_ERROR((*m_report_handler), "1", ss.str().c_str());
			trans->set_response_status(xtlm::XTLM_ADDRESS_ERROR_RESPONSE);
		} else {
			ss.clear();
			trans->set_response_status(xtlm::XTLM_OK_RESPONSE);
			unsigned char* data_ptr = trans->get_data_ptr();
			m_mem_handler->readDevMem(address, data_ptr, tranSize);
		}
		m_rd_req_vec.push_back(trans);
	}
	if (m_rd_req_vec.empty() == false
			&& m_target_0_rd_socket_util->is_master_ready()) {
		ss.clear();
		xtlm::aximm_payload* trans = m_rd_req_vec.front();
		if (m_report_handler->get_verbosity_level() == xsc::common_cpp::DEBUG) {
			std::string des;
			trans->get_log(des, 1);
			ss << name() << ": Sending Read Response " << std::endl << des
					<< std::endl;
			XSC_REPORT_INFO_VERB((*m_report_handler), "1", ss.str().c_str(),
					DEBUG);
		}
		sc_core::sc_time delay = SC_ZERO_TIME;
		m_target_0_rd_socket_util->send_data(*trans, delay);
		m_rd_req_vec.pop_front();
	}
}

void axi_bram_memory_imp::methodProcessTransactionWrite() {
	std::stringstream ss;
	if (m_target_0_wr_socket_util->is_trans_available()) {
		xtlm::aximm_payload* trans =
				m_target_0_wr_socket_util->get_transaction();
		uint64_t max_address = m_mem_addr_width;
		uint64_t address = trans->get_address() % max_address;
		uint64_t tranSize = trans->get_data_length();
		//Check for the address range
		trans->set_response_status(xtlm::XTLM_OK_RESPONSE);
		if ((address + tranSize) > max_address) {
			ss.clear();
			std::string des;
			trans->get_log(des, 1);
			ss << name() << ": Invalid Write Request " << std::endl << des
					<< std::endl;
			XSC_REPORT_ERROR((*m_report_handler), "1", ss.str().c_str());
			trans->set_response_status(xtlm::XTLM_ADDRESS_ERROR_RESPONSE);

		} else {
			xtlm::xtlm_extension<xsc::extension::P2P_XTLM_extension>* bExt;
			trans->get_extension(bExt);
			xsc::extension::P2P_XTLM_extension * ext =
					dynamic_cast<xsc::extension::P2P_XTLM_extension *>(bExt);
			if (ext != nullptr) {
				std::string filename;
				bool ret = true;
				if (ext->getType() == xsc::extension::TYPE::ALLOC_BO)
					ret = m_mem_handler->createMMappedBuffer(
							address, ext->getSize(), filename);
				else if (ext->getType() == xsc::extension::TYPE::FREE_BO)
					ret = m_mem_handler->freePage(address);
				else if (ext->getType() == xsc::extension::TYPE::IMPORT_BO) {
					ret = m_mem_handler->importBO(address,
							ext->getFileName(), ext->getSize());
				} else if (ext->getType() == xsc::extension::TYPE::COPY_BO) {
					ret = m_mem_handler->copyBO(address,
							ext->getFileName(), ext->getSize(),
							ext->getSrcOffset(), ext->getDstOffset());
				}
				ext->setResponse(ret);
				ext->setFileName(filename);
			}

			else if (!trans->get_byte_enable_ptr()) {
				unsigned char* data_ptr = trans->get_data_ptr();
				if (m_report_handler->get_verbosity_level()
						== xsc::common_cpp::DEBUG) {
					ss.clear();
					std::string des;
					trans->get_log(des, 1);
					ss << name() << ": Received Write Request " << std::endl
							<< des << std::endl;
					XSC_REPORT_INFO_VERB((*m_report_handler), "1",
							ss.str().c_str(), DEBUG);
				}

				m_mem_handler->writeDevMem(address, data_ptr, tranSize);
			} else {
				if (m_report_handler->get_verbosity_level()
						== xsc::common_cpp::DEBUG) {
					ss.clear();
					std::string des;
					trans->get_log(des, 1);
					ss << name() << ": Received Write Request " << std::endl
							<< des << std::endl;
					XSC_REPORT_INFO_VERB((*m_report_handler), "1",
							ss.str().c_str(), DEBUG);
				}
				unsigned char* data_ptr = trans->get_data_ptr();
				unsigned char* byte_enable_ptr = trans->get_byte_enable_ptr();
				uint64_t byte_enable_len = trans->get_byte_enable_length();
				uint32_t index = 0;
				for (index = 1; index <= tranSize; index++) {
					bool byteEn = byte_enable_ptr[(index - 1) % byte_enable_len];
					if (byteEn) {
						m_mem_handler->writeDevMem(index + address - 1,
								data_ptr + (index - 1), 1);
					}
				}
			}
		}
		m_wr_resp_vec.push_back(trans);
	}
	if (m_wr_resp_vec.empty() == false
			&& m_target_0_wr_socket_util->is_master_ready()) {
		std::stringstream msg;
		sc_core::sc_time delay = SC_ZERO_TIME;
		xtlm::aximm_payload* trans = m_wr_resp_vec.front();
		if (m_report_handler->get_verbosity_level() == xsc::common_cpp::DEBUG) {
			ss.clear();
			std::string des;
			trans->get_log(des, 1);
			ss << name() << ": Sending Write Request " << std::endl << des
					<< std::endl;
			XSC_REPORT_INFO_VERB((*m_report_handler), "1", ss.str().c_str(),
					DEBUG);
		}
		m_target_0_wr_socket_util->send_resp(*trans, delay);
		m_wr_resp_vec.pop_front();
	}

}

unsigned int axi_bram_memory_imp::transport_dbg(xtlm::aximm_payload& trans) {
	if (trans.get_command() == xtlm::XTLM_READ_COMMAND) {
		return transport_dbg_rd(trans);
	} else if (trans.get_command() == xtlm::XTLM_WRITE_COMMAND) {
		return transport_dbg_wr(trans);
	}
	return 0;
}

unsigned int axi_bram_memory_imp::transport_dbg_rd(
		xtlm::aximm_payload& trans) {
	sc_core::sc_time delay = SC_ZERO_TIME;
	uint64_t max_address = m_mem_addr_width;
	uint64_t address = trans.get_address() % max_address;
	uint64_t tranSize = trans.get_data_length();
	std::stringstream ss;
	ss.clear();
	std::string des;
	if (m_report_handler->get_verbosity_level() == xsc::common_cpp::DEBUG) {
		trans.get_log(des, 1);
		ss << name() << ": Received Debug Read Request " << des << std::endl;
		XSC_REPORT_INFO_VERB((*m_report_handler), "1", ss.str().c_str(), DEBUG);
	}
	if ((address + tranSize) > max_address) {
		ss.clear();
		ss << name() << ": Invalid Debug Read Request " << std::endl << des
				<< std::endl;
		XSC_REPORT_ERROR((*m_report_handler), "1", ss.str().c_str());
		trans.set_response_status(xtlm::XTLM_ADDRESS_ERROR_RESPONSE);
		return 0;
	} else {
		ss.clear();
		trans.set_response_status(xtlm::XTLM_OK_RESPONSE);
		unsigned char* data_ptr = trans.get_data_ptr();
		m_mem_handler->readDevMem(address, data_ptr, tranSize);
		return trans.get_data_length();
	}
}

unsigned int axi_bram_memory_imp::transport_dbg_wr(
		xtlm::aximm_payload& trans) {
	std::stringstream ss;
	uint64_t max_address = m_mem_addr_width;
	uint64_t address = trans.get_address() % max_address;
	uint64_t tranSize = trans.get_data_length();
	//Check for the address range
	trans.set_response_status(xtlm::XTLM_OK_RESPONSE);
	if ((address + tranSize) > max_address) {
		ss.clear();
		std::string des;
		trans.get_log(des, 1);
		ss << name() << ": Invalid Debug Write Request " << std::endl << des
				<< std::endl;
		XSC_REPORT_ERROR((*m_report_handler), "1", ss.str().c_str());
		trans.set_response_status(xtlm::XTLM_ADDRESS_ERROR_RESPONSE);
		return 0;
	}
	xtlm::xtlm_extension<xsc::extension::P2P_XTLM_extension>* bExt;
	trans.get_extension(bExt);
	xsc::extension::P2P_XTLM_extension * ext =
			dynamic_cast<xsc::extension::P2P_XTLM_extension *>(bExt);
	if (ext != nullptr) {
		std::string filename;
		bool ret = true;
		if (ext->getType() == xsc::extension::TYPE::ALLOC_BO)
			ret = m_mem_handler->createMMappedBuffer(address,
					ext->getSize(), filename);
		else if (ext->getType() == xsc::extension::TYPE::FREE_BO)
			ret = m_mem_handler->freePage(address);
		else if (ext->getType() == xsc::extension::TYPE::IMPORT_BO) {
			ret = m_mem_handler->importBO(address,
					ext->getFileName(), ext->getSize());
		} else if (ext->getType() == xsc::extension::TYPE::COPY_BO) {
			ret = m_mem_handler->copyBO(address, ext->getFileName(),
					ext->getSize(), ext->getSrcOffset(), ext->getDstOffset());
		}
		ext->setResponse(ret);
		ext->setFileName(filename);
		return 0;
	}

	else if (!trans.get_byte_enable_ptr()) {
		unsigned char* data_ptr = trans.get_data_ptr();
		if (m_report_handler->get_verbosity_level() == xsc::common_cpp::DEBUG) {
			ss.clear();
			std::string des;
			trans.get_log(des, 1);
			ss << name() << ": Received Debug Write Request " << std::endl
					<< des << std::endl;
			XSC_REPORT_INFO_VERB((*m_report_handler), "1", ss.str().c_str(),
					DEBUG);
		}
		m_mem_handler->writeDevMem(address, data_ptr, tranSize);
		return tranSize;
	} else {
		if (m_report_handler->get_verbosity_level() == xsc::common_cpp::DEBUG) {
			ss.clear();
			std::string des;
			trans.get_log(des, 1);
			ss << name() << ": Received Debug Write Request " << std::endl
					<< des << std::endl;
			XSC_REPORT_INFO_VERB((*m_report_handler), "1", ss.str().c_str(),
					DEBUG);
		}
		unsigned char* data_ptr = trans.get_data_ptr();
		unsigned char* byte_enable_ptr = trans.get_byte_enable_ptr();
		uint64_t byte_enable_len = trans.get_byte_enable_length();
		uint32_t index = 0;
		for (index = 1; index <= tranSize; index++) {
			bool byteEn = byte_enable_ptr[(index - 1) % byte_enable_len];
			if (byteEn) {
				m_mem_handler->writeDevMem(index + address - 1,
						data_ptr + (index - 1), 1);
			}
		}
		return tranSize;
	}
}

