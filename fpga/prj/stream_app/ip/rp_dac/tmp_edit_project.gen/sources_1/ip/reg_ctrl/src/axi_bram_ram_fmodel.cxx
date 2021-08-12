// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689
// (c) Copyright(C) 2013 - 2018 by Xilinx, Inc. All rights reserved.
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

#include "axi_bram_ram_fmodel.h"

#include <sys/stat.h>
#include <sstream>

axi_bram_ram_fmodel::~axi_bram_ram_fmodel() {
	for (auto& mem : pageCache) {
		delete mem.second;
	}
}

axi_bram_ram_fmodel::axi_bram_ram_fmodel(
		std::string p_module_name,
		xsc::common_cpp::report_handler* report_handler, uint64_t addr_size) :
		axi_bram_fmodel_base(p_module_name, report_handler, addr_size) {
	std::stringstream msg;
	msg << p_module_name << " : ";
	msg << " Initializing " << std::endl;

	module_name = p_module_name;
	m_report_handler = report_handler;
	m_mem_addr_width = addr_size;
	report_handler->report("1", msg.str().c_str(), xsc::common_cpp::INFO,
			xsc::common_cpp::VERBOSITY::DEBUG);
}

unsigned int axi_bram_ram_fmodel::writeDevMem(uint64_t offset, void* src,
		unsigned int size) {
	if (m_report_handler->get_verbosity_level() == xsc::common_cpp::DEBUG) {
		std::stringstream msg;
		msg << module_name << " : ";
		msg << " Writing Memory Address = " << offset << " Size = " << size
				<< " Data =" << std::endl;
		uint64_t counter = 0;
		for (int i = 0; i < size; i++) {
			msg << (unsigned int) (((unsigned char*) src)[i]) << " ";
			if (counter <= 16)
				counter++;
			else {
				counter = 0;
				msg << std::endl;
			}
		}
		msg << std::endl;
		m_report_handler->report("1", msg.str().c_str(), xsc::common_cpp::INFO,
				xsc::common_cpp::VERBOSITY::DEBUG);
	}
	uint64_t written_bytes = 0;
	uint64_t addr = offset;
	while (written_bytes < size) {
		uint64_t src_offset = written_bytes;
		std::string sFileName("");
		unsigned char* page_ptr = get_page(addr, sFileName);
		uint64_t page_addr = addr & ~(-1 << ADDRBITS);

		unsigned char* dest_buf_ptr = page_ptr + page_addr;
		unsigned char* src_buf_ptr = (unsigned char*) (src) + src_offset;

		uint64_t remaining_bytes_to_write = size - written_bytes;
		uint64_t unaligned_bytes_in_addr = (addr & ~(-1 << ADDRBITS));
		uint64_t bytes_upto_next_alignment = (0x1 << ADDRBITS)
				- unaligned_bytes_in_addr;

		uint64_t buf_size = 0;
		if (bytes_upto_next_alignment > remaining_bytes_to_write) {
			buf_size = remaining_bytes_to_write;
		} else {
			buf_size = bytes_upto_next_alignment;
		}
		memcpy(dest_buf_ptr, src_buf_ptr, buf_size);
		written_bytes += buf_size;
		addr += buf_size;
	}
	return 0;
}

unsigned int axi_bram_ram_fmodel::readDevMem(uint64_t offset, void* dest,
		unsigned int size) {
	uint64_t read_bytes = 0;
	uint64_t addr = offset;

	while (read_bytes < size) {
		uint64_t dest_offset = read_bytes;
		std::string sFileName("");
		unsigned char* page_ptr = get_page(addr, sFileName);
		uint64_t page_addr = addr & ~(-1 << ADDRBITS);

		unsigned char* src_buf_ptr = page_ptr + page_addr;
		unsigned char* dest_buf_ptr = (unsigned char*) (dest) + dest_offset;

		uint64_t remaining_bytes_to_read = size - read_bytes;
		uint64_t unaligned_bytes_in_addr = (addr & ~(-1 << ADDRBITS));
		uint64_t bytes_upto_next_alignment = (0x1 << ADDRBITS)
				- unaligned_bytes_in_addr;

		uint64_t buf_size = 0;
		if (bytes_upto_next_alignment > remaining_bytes_to_read) {
			buf_size = remaining_bytes_to_read;
		} else {
			buf_size = bytes_upto_next_alignment;
		}
		memcpy(dest_buf_ptr, src_buf_ptr, buf_size);
		read_bytes += buf_size;
		addr += buf_size;
	}
	if (m_report_handler->get_verbosity_level() == xsc::common_cpp::DEBUG) {
		std::stringstream msg;
		msg << module_name << " : ";
		msg << " Reading Memory Address = " << offset << " Size = " << size
				<< " Data = " << std::endl;
		uint64_t counter = 0;
		for (int i = 0; i < size; i++) {
			msg << (unsigned int) (((unsigned char*) dest)[i]) << " ";
			if (counter <= 16)
				counter++;
			else {
				counter = 0;
				msg << std::endl;
			}
		}
		msg << std::endl;
		m_report_handler->report("1", msg.str().c_str(), xsc::common_cpp::INFO,
				xsc::common_cpp::VERBOSITY::DEBUG);
	}
	return 0;
}
unsigned char* axi_bram_ram_fmodel::get_page(uint64_t offset,
		std::string& p2pFileName, uint64_t size) {
	uint64_t page_idx = offset >> ADDRBITS;
	std::string file_name = get_mem_file_name(page_idx);
	if (pageCache.size() > N_1MBARRAYS) {
		std::cerr
				<< "Out of Memory. DDR model does not support this much of memory\n";
		exit(1);
	} else {
		FILE* pFile = NULL;
		if (pageCache.find(page_idx) != pageCache.end()) {
			return pageCache[page_idx];
		} else {

			if (size != 0) {
				int fd = -1;
				p2pFileName = file_name + "_shared";
				if ((fd = open(p2pFileName.c_str(), O_CREAT | O_RDWR,
						S_IRWXU | S_IRGRP | S_IROTH)) == -1) {
					printf("Error opening file.\n");
				}
				void* pageStartOSAddressVoid = mmap(0, size,
						PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fd,
						0/*sysconf(_SC_PAGESIZE)*/);
				if (fd < 0) {
					munmap(pageStartOSAddressVoid, size);
					p2pFileName = "";
				}

				if (ftruncate(fd, size) < 0) {
					close(fd);
					munmap(pageStartOSAddressVoid, size);
					p2pFileName = "";
				}
				if (!p2pFileName.empty()) {
					unsigned char* pagePtr =
							(unsigned char*) pageStartOSAddressVoid;
					while (size > 0) {
						pageCache[page_idx] = pagePtr;
						page_idx = page_idx + 1;
						pagePtr = pagePtr + PAGESIZE;
						if (size <= PAGESIZE)
							break;
						size = size - PAGESIZE;
						std::cout << "page_idx is " << page_idx << std::endl;
						std::cout << "size is " << size << std::endl;
					}
					return (unsigned char*) pageStartOSAddressVoid;
				}
			}

			pageCache[page_idx] = new unsigned char[PAGESIZE];
			//for(int i = 0; i < ONE_MB;i++){
			//  pageCache[page_idx][i] = 0x0;
			//}
			return pageCache[page_idx];
		}
	}
}

void axi_bram_ram_fmodel::init_fmodel() {

}

bool axi_bram_ram_fmodel::createMMappedBuffer(uint64_t base_address,
		uint64_t size, std::string& buffer_filename) {
	return true;
}

bool axi_bram_ram_fmodel::freePage(uint64_t base_address) {
	return true;
}

bool axi_bram_ram_fmodel::copyBO(uint64_t offset,
		std::string dst_filename, uint64_t size, uint64_t src_offset,
		uint64_t dst_offset) {
	return true;

}

bool axi_bram_ram_fmodel::importBO(uint64_t offset,
		std::string dst_filename, uint64_t size) {
	return true;
}

std::string axi_bram_ram_fmodel::get_mem_file_name(uint64_t pageIdx) //,enum fileType file_type)
		{
	std::string file_name;
	std::string socket_id;
	std::string pid;
	std::string deviceName("");
	std::string user("");
	if (getenv("EMULATION_SOCKETID")) {
		socket_id = getenv("EMULATION_SOCKETID");
		std::size_t foundLast = socket_id.find_last_of("_");
		std::size_t foundFirst = socket_id.find_first_of("_");
		if (foundLast != std::string::npos) {
			pid = socket_id.substr(foundLast + 1);
		}
		if (foundFirst != std::string::npos) {
			deviceName = socket_id.substr(0, foundFirst);
		}
	}

	if (getenv("USER") != NULL) {
		user = getenv("USER");
	}
	std::string file_path("");
	std::string sEmRunDir("");

	if (getenv("EMULATION_RUN_DIR")) {
		sEmRunDir = getenv("EMULATION_RUN_DIR");
	} else {
		sEmRunDir = "/tmp/" + user + "/hw_em/";
	}

	file_path = sEmRunDir + "/" + module_name + "/";
	std::stringstream mkdirCommand;
	mkdirCommand << "mkdir -p " << file_path;
	;
	struct stat statBuf;
	if (stat(file_path.c_str(), &statBuf) == -1) {
		system(mkdirCommand.str().c_str());
	}
	file_name = file_path + module_name + "_" + std::to_string(pageIdx);
	if (m_report_handler->get_verbosity_level()
			== xsc::common_cpp::VERBOSITY::DEBUG) {
		std::stringstream m_ss;
		m_ss.str("");
		m_ss << "ddr fmodel file_name: " << file_name << std::endl;
		m_report_handler->report("1", m_ss.str().c_str(), xsc::common_cpp::INFO,
				xsc::common_cpp::VERBOSITY::DEBUG);
	}
	return file_name;
}

