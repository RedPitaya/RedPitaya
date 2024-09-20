#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "memory_manager.h"

#include "logger_lib/file_logger.h"
#include "data_lib/network_header.h"

#define MR_MEMORY_BLOCK_SIZE (32 * 1024)
#define MIN_BLOCK_COUNT 12
#define MIN_ADC_BLOCK MIN_BLOCK_COUNT
#define MIN_DAC_BLOCK MIN_BLOCK_COUNT
#define MIN_GPIO_BLOCK MIN_BLOCK_COUNT

using namespace uio_lib;

int getReservedMemory(uint32_t *_startAddress, uint32_t *_size)
{
	*_startAddress = 0;
	*_size = 0;
	int fd = 0;
	if ((fd = open("/sys/firmware/devicetree/base/reserved-memory/buffer@1000000/reg", O_RDONLY)) == -1) {
		fprintf(stderr, "[FATAL ERROR] Error open: /sys/firmware/devicetree/base/reserved-memory/buffer@1000000/reg\n");
		return -1;
	}
	char data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int sz = read(fd, &data, 8);

	if (close(fd) < 0) {
		return -1;
	}
	if (sz == 8) {
		*_startAddress = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
		*_size = data[4] << 24 | data[5] << 16 | data[6] << 8 | data[7];
	} else {
		return -1;
	}
	return 0;
}

auto CMemoryManager::instance() -> Ptr
{
	static auto s = std::make_shared<CMemoryManager>();
	return s;
}

CMemoryManager::CMemoryManager()
	: m_lowReservedAddress(0)
	, m_highReservedAddress(0)
	, m_mem_fd(0)
	, m_memory(MAP_FAILED)
	, m_blockSize(MR_MEMORY_BLOCK_SIZE + DataLib::sizeHeader())
{
	if (getReservedMemory(&m_startRAMAddress, &m_ramSize)) {
		FATAL("Unable to get the reserved memory area.")
	}
	m_lowReservedAddress = m_startRAMAddress;
	m_highReservedAddress = m_startRAMAddress + m_ramSize;
	TRACE_SHORT("Reserved memory 0x%X - 0x%X", m_lowReservedAddress, m_highReservedAddress)

	m_mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (m_mem_fd < 0) {
		FATAL("Error opening device: /dev/mem")
	}

	if (m_lowReservedAddress % sysconf(_SC_PAGESIZE) != 0) {
		FATAL("Error size. offset %% sysconf(_SC_PAGESIZE) = %ld  must be zero. sysconf(_SC_PAGESIZE) = %ld\n",
			  (m_lowReservedAddress % sysconf(_SC_PAGESIZE)),
			  sysconf(_SC_PAGESIZE))
	}

	m_memory = mmap(NULL, m_ramSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_mem_fd, m_lowReservedAddress);

	if (m_memory == MAP_FAILED) {
		FATAL("Error mmap memory")
	}
	if (!setReserverdMemory(MM_ADC, MIN_ADC_BLOCK * m_blockSize)) {
		FATAL("Can't reserve memory for ADC")
	}
	if (!setReserverdMemory(MM_DAC, MIN_DAC_BLOCK * m_blockSize)) {
		FATAL("Can't reserve memory for DAC")
	}
	if (!setReserverdMemory(MM_GPIO, MIN_GPIO_BLOCK * m_blockSize)) {
		FATAL("Can't reserve memory for GPIO")
	}
}

CMemoryManager::~CMemoryManager()
{
	if (m_memory != MAP_FAILED) {
		if (munmap(m_memory, m_highReservedAddress - m_lowReservedAddress) < 0) {
			ERROR_LOG("Error unmap memory")
		}
		m_memory = MAP_FAILED;
	}

	if (m_mem_fd) {
		if (close(m_mem_fd) < 0) {
			ERROR_LOG("Error close device: /dev/mem")
		}
		m_mem_fd = 0;
	}
}

auto CMemoryManager::reallocateBlocks() -> bool
{
	if (m_ramSize < getMinRequiredRAM()) {
		WARNING("Not enough memory to split into blocks. DMA RAM size %d. Required RAM size %d", m_ramSize, getMinRequiredRAM())
		return false;
	}

	uint32_t blockCount = m_ramSize / getMemoryBlockSize();
	m_heap.clear();
	m_heap.resize(blockCount);
	for (auto i = 0u; i < blockCount; i++) {
		MemoryRegionT itm;
		itm.start = m_startRAMAddress + i * getMemoryBlockSize();
		itm.end = m_startRAMAddress + (i + 1) * getMemoryBlockSize();
		itm.startMemory = (uint8_t *) m_memory + i * getMemoryBlockSize();
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
	return getMinRAMSize(getMemoryBlockSize(), _tag);
}

auto CMemoryManager::getMinRAMSize(uint32_t _blockSize, MemoryTAG _tag) -> uint32_t
{
	switch (_tag) {
		case MM_ADC_RESERVE_SKIP:
			return _blockSize;
		case MM_ADC:
			return _blockSize * MIN_ADC_BLOCK;
		case MM_DAC:
			return _blockSize * MIN_DAC_BLOCK;
		case MM_GPIO:
			return _blockSize * MIN_GPIO_BLOCK;
		default:
			break;
	}
	return 0;
}

auto CMemoryManager::getMaxBlockSize() -> uint32_t
{
	return m_ramSize / std::max({MIN_ADC_BLOCK, MIN_DAC_BLOCK, MIN_GPIO_BLOCK});
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

auto CMemoryManager::getMinRequiredRAM() -> uint32_t
{
	auto blocks = std::max({MIN_ADC_BLOCK, MIN_DAC_BLOCK, MIN_GPIO_BLOCK});
	return blocks * getMemoryBlockSize();
}

auto CMemoryManager::getRAMSize() -> uint32_t
{
	return m_ramSize;
}

auto CMemoryManager::isValidSize(MemoryTAG _tag) -> bool
{
	auto s = getReserverdMemory(_tag);
	auto minsize = getMinRAMSize(_tag);
	return s >= minsize;
}
