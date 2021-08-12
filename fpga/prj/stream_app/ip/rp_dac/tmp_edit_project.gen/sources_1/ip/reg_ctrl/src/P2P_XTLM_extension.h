// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689
#ifndef __P2P_XTLM_EXTENSION_HH_
#define __P2P_XTLM_EXTENSION_HH_
#include "xtlm.h"

/**
 * XTLM extension to support peer to peer buffer allocation
 * Master requests buffer allocation with a base address and size and slave responds with corresponding
 * mmaped buffer file name
 */
namespace xsc {
namespace extension {
enum TYPE {
ALLOC_BO,
FREE_BO,
IMPORT_BO,
COPY_BO
};

class P2P_XTLM_extension: public xtlm::xtlm_extension<P2P_XTLM_extension> {

  public:
	P2P_XTLM_extension() {
		m_address=0;
		m_size=0;
    m_file_name = "";
		m_resp_valid=false;
		m_resp=false;
    m_type=TYPE::ALLOC_BO;
    m_src_offset = 0;
    m_dst_offset = 0;
	}

	~P2P_XTLM_extension(){}
  xtlm::xtlm_extension_base* clone() const {
		// Must override pure virtual clone method
		P2P_XTLM_extension* t = new P2P_XTLM_extension();
		return t;
	}
	// Must override pure virtual copy_from method
	void copy_from(xtlm::xtlm_extension_base const &ext) {
		m_address     = static_cast<P2P_XTLM_extension const &>(ext).m_address;
		m_size       = static_cast<P2P_XTLM_extension const &>(ext).m_size;
		m_file_name  = static_cast<P2P_XTLM_extension const &>(ext).m_file_name;
		m_resp_valid = static_cast<P2P_XTLM_extension const &>(ext).m_resp_valid;
		m_resp       = static_cast<P2P_XTLM_extension const &>(ext).m_resp;
		m_type       = static_cast<P2P_XTLM_extension const &>(ext).m_type;
		m_src_offset = static_cast<P2P_XTLM_extension const &>(ext).m_src_offset;
		m_dst_offset = static_cast<P2P_XTLM_extension const &>(ext).m_dst_offset;
	}
	
  void setAddress(uint64_t address) { 	m_address = address; }
	uint64_t getAddress()const        {   return m_address; }

  void setSize(uint64_t size) { m_size = size; }
	uint64_t getSize()const { return m_size; }

	void setFileName(std::string file_name) { m_file_name = file_name; }
	const std::string&  getFileName()const { return m_file_name; }

	void setIsResponseValid(bool resp_valid) { m_resp_valid = resp_valid;}
	bool getIsResponseValid() { return m_resp_valid; }

	void setResponse(bool resp){ m_resp = resp;}
	bool getResponse()  const  { return m_resp;}
  
  void setSrcOffset(uint64_t src_offset) { m_src_offset = src_offset; } 
  uint64_t getSrcOffset() const { return m_src_offset; }

  void setDstOffset(uint64_t dst_offset) { m_dst_offset = dst_offset; } 
  uint64_t getDstOffset() const { return m_dst_offset; }

  void setType(TYPE type) { m_type = type; }
  TYPE getType()  const { return m_type;}
	
private :
	uint64_t     m_address;
	uint64_t     m_size;
	std::string  m_file_name;
	bool         m_resp_valid;
	bool         m_resp;
  uint64_t     m_src_offset;
  uint64_t     m_dst_offset;
  TYPE         m_type;
};
}
}
#endif

