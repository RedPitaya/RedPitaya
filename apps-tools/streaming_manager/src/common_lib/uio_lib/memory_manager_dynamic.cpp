#include <algorithm>
#include <fcntl.h>
#include <stdint.h>

#ifndef _WIN32
#include <sys/statvfs.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "memory_manager.h"

#include "data_lib/network_header.h"
#include "logger_lib/file_logger.h"

#define MR_MEMORY_BLOCK_SIZE (32 * 1024)
#define MIN_RAM_SIZE 10 * 1024 * 1024
#define MIN_BLOCK_COUNT 12
#define MIN_ADC_BLOCK MIN_BLOCK_COUNT
#define MIN_DAC_BLOCK MIN_BLOCK_COUNT
#define MIN_GPIO_BLOCK MIN_BLOCK_COUNT

using namespace uio_lib;

auto getTotalSystemMemory = []() -> uint64_t {
#ifndef _WIN32
	uint64_t pages = sysconf(_SC_PHYS_PAGES);
	uint64_t page_size = sysconf(_SC_PAGE_SIZE);
	return pages * page_size;
#else
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);
	return status.ullTotalPhys;
#endif
};

auto CMemoryManager::instance() -> Ptr
{
	static auto s = std::make_shared<CMemoryManager>();
	return s;
}

CMemoryManager::CMemoryManager()
	: m_lowReservedAddress(0)
	, m_highReservedAddress(0)
	, m_blockSize(MR_MEMORY_BLOCK_SIZE + DataLib::sizeHeader())
{
	auto totalSize = getTotalSystemMemory();
	auto calc = totalSize * 0.05;
	m_ramSize = MIN_RAM_SIZE > calc ? MIN_RAM_SIZE : calc;
}

CMemoryManager::~CMemoryManager()
{
	for (auto i = 0u; i < m_heap.size(); i++) {
		delete[] m_heap[i].startMemory;
	}
}

auto CMemoryManager::reallocateBlocks() -> bool
{
	if (m_ramSize < getMinRequiredRAM()) {
		WARNING("Not enough memory to split into blocks. DMA RAM size %d. Required RAM size %d", m_ramSize, getMinRequiredRAM())
		return false;
	}

	for (auto i = 0u; i < m_heap.size(); i++) {
		delete[] m_heap[i].startMemory;
	}

	uint32_t blockCount = m_ramSize / getMemoryBlockSize();
	m_heap.clear();
	m_heap.resize(blockCount);
	for (auto i = 0u; i < blockCount; i++) {
		MemoryRegionT itm;
		itm.start = 0;
		itm.end = 0;
		itm.startMemory = new uint8_t[getMemoryBlockSize()];
		memset(itm.startMemory, 0, getMemoryBlockSize());
		itm.size = getMemoryBlockSize();
		itm.tag = MM_NONE;
		itm.isFree = true;
		m_heap[i] = itm;
	}
	TRACE("Realloc memory blockCount %d block size %d", blockCount, getMemoryBlockSize())
	return true;
}

auto CMemoryManager::reserveMemory(MemoryTAG tag, uint32_t blockCount, uint32_t channelsCount) -> uint32_t
{
	uint32_t reserverd = 0;
	{
		std::lock_guard lock(m_mtx);
		if (channelsCount == 0)
			return reserverd;
		for (auto i = 0u; reserverd < blockCount && i < m_heap.size(); i++) {
			if (m_heap[i].isFree) {
				m_heap[i].isFree = false;
				m_heap[i].tag = tag;
				reserverd++;
			}
		}
		uint32_t needRelease = reserverd % channelsCount;
		for (auto i = 0u; needRelease && i < m_heap.size(); i++) {
			if (m_heap[i].tag == tag) {
				m_heap[i].isFree = true;
				m_heap[i].tag = MM_NONE;
				reserverd--;
				needRelease--;
			}
		}
	}
	if (getMinRAMSize(tag) > reserverd * getMemoryBlockSize()) {
		WARNING("Not enough memory. Need %d. Reserved %d", getMinRAMSize(tag), reserverd * getMemoryBlockSize())
		releaseMemory(tag);
		return 0;
	}
	return reserverd;
}

auto CMemoryManager::releaseMemory(MemoryTAG tag) -> void
{
	std::lock_guard lock(m_mtx);
	for (auto i = 0u; i < m_heap.size(); i++) {
		if (m_heap[i].tag == tag) {
			m_heap[i].tag = MM_NONE;
			m_heap[i].isFree = true;
		}
	}
}

auto CMemoryManager::getMinRequiredRAM() -> uint32_t
{
	auto blocks = std::max({MIN_ADC_BLOCK, MIN_DAC_BLOCK, MIN_GPIO_BLOCK});
	return blocks * getMemoryBlockSize();
}

auto CMemoryManager::getRegions(MemoryTAG tag) -> std::vector<MemoryRegionT>
{
	std::lock_guard lock(m_mtx);
	std::vector<MemoryRegionT> list;
	for (auto i = 0u; i < m_heap.size(); i++) {
		if (m_heap[i].tag == tag) {
			list.push_back(m_heap[i]);
		}
	}
	return list;
}

auto CMemoryManager::getFreeBlockCount() -> uint32_t
{
	std::lock_guard lock(m_mtx);
	uint32_t freeBlock = 0;
	for (auto i = 0u; i < m_heap.size(); i++) {
		if (m_heap[i].isFree) {
			freeBlock++;
		}
	}
	return freeBlock;
}

auto CMemoryManager::setReserverdMemory(MemoryTAG _tag, uint32_t size) -> bool
{
	if (size > m_ramSize) {
		WARNING("Not enough memory. Size: %d. Need size: %d", m_ramSize, size)
		return false;
	}
	m_reservedMemory[_tag] = size;
	return true;
}

auto CMemoryManager::getReserverdMemory(MemoryTAG _tag) -> uint32_t
{
	if (_tag == MM_ADC_RESERVE_SKIP)
		return 0;
	if (_tag == MM_NONE)
		return 0;
	return m_reservedMemory[_tag];
}

auto CMemoryManager::setMemoryBlockSize(uint32_t _size) -> void
{
	m_blockSize = _size + DataLib::sizeHeader();
}

auto CMemoryManager::getMemoryBlockSize() -> uint32_t
{
	return m_blockSize;
}

auto CMemoryManager::getMinRAMSize(MemoryTAG _tag) -> uint32_t
{
	return getMinRAMSize(m_blockSize, _tag);
}

auto CMemoryManager::getMinRAMSize(uint32_t, MemoryTAG) -> uint32_t
{
	return 0;
}
