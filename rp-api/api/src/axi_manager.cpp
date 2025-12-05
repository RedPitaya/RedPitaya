/**
 * $Id: $
 *
 * @brief Red Pitaya library
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include "axi_manager.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <algorithm>
#include <memory>
#include <mutex>
#include <vector>
#include "rp_log.h"

struct block_t;
class CHolder {
   public:
    CHolder(){};
    ~CHolder() { axi_releaseManager(); };
};

uint32_t g_startRegion = 0;
uint32_t g_sizeRegion = 0;
uint64_t g_index = 0;
std::vector<block_t> g_reserved;
std::shared_ptr<CHolder> g_holder = NULL;
std::mutex g_mutex;

static int g_mem_fd = -1;

int osc_axi_map(size_t size, size_t offset, void** mapped);
int osc_axi_unmap(size_t size, void** mapped);

struct block_t {
    uint32_t start;
    uint32_t end;
    uint32_t size;
    uint64_t index;
    uint16_t* mapped = (uint16_t*)MAP_FAILED;
    block_t(uint32_t _start, uint32_t _size, uint64_t _index) : start(_start), end(_start + _size), size(_size), index(_index){};
};

int osc_axi_map(size_t size, size_t offset, void** mapped) {
    if (g_mem_fd == -1) {
        return RP_EMMD;
    }
    if (offset < g_startRegion) {
        return RP_EOOR;
    }
    if (offset + size > (g_startRegion + g_sizeRegion)) {
        return RP_EOOR;
    }
    if (offset % sysconf(_SC_PAGESIZE) != 0) {
        ERROR_LOG("Error size. offset %% sysconf(_SC_PAGESIZE) = %ld  must be zero. sysconf(_SC_PAGESIZE) = %ld\nOffset %ld must be a multiple of sysconf(_SC_PAGESIZE) = %ld\n",
                  offset % sysconf(_SC_PAGESIZE), sysconf(_SC_PAGESIZE), (long int)offset, sysconf(_SC_PAGESIZE));
        return RP_EMMD;
    }
    *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, g_mem_fd, offset);
    if (*mapped == MAP_FAILED) {
        ERROR_LOG("Error osc_axi_map: %s\n", strerror(errno));
        return RP_EMMD;
    }
    return RP_OK;
}

int osc_axi_unmap(size_t size, void** mapped) {
    if (g_mem_fd == -1) {
        return RP_EUMD;
    }
    if ((mapped == MAP_FAILED) || (mapped == NULL)) {
        return RP_EUMD;
    }
    if ((*mapped == MAP_FAILED) || (*mapped == NULL)) {
        return RP_EUMD;
    }
    if (munmap(*mapped, size) < 0) {
        return RP_EUMD;
    }
    *mapped = MAP_FAILED;
    return RP_OK;
}

int axi_initManager() {
    std::lock_guard lock(g_mutex);
    if (g_mem_fd == -1) {
        ECHECK(axi_getOSReservedRegion(&g_startRegion, &g_sizeRegion))
        g_reserved.clear();
        g_index = 0;
        g_mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
        if (g_mem_fd < 0) {
            g_mem_fd = -1;
            return RP_EOMD;
        }
    }
    if (g_holder == NULL) {
        g_holder = std::make_shared<CHolder>();
    }
    return RP_OK;
}

int axi_releaseManager() {
    std::lock_guard lock(g_mutex);
    for (auto& it : g_reserved) {
        osc_axi_unmap(it.size, (void**)&it.mapped);
    }
    g_reserved.clear();
    if (g_mem_fd != -1) {
        if (close(g_mem_fd) < 0) {
            return RP_ECMD;
        }
        g_mem_fd = -1;
    }
    return RP_OK;
}

int axi_getOSReservedRegion(uint32_t* _startAddress, uint32_t* _size) {
    return cmn_GetReservedMemory(_startAddress, _size);
}

int axi_checkOverlapped(const block_t& block) {
    bool overlaped = false;
    for (auto& it : g_reserved) {
        if (it.start <= block.start && block.start <= it.end) {
            overlaped |= true;
        }
        if (it.start <= block.end && block.end <= it.end) {
            overlaped |= true;
        }
        if (block.start <= it.start && it.end <= block.end) {
            overlaped |= true;
        }
    }
    return overlaped;
};

int axi_checkOverlapped(uint32_t _startAddress, uint32_t _size) {
    block_t block{_startAddress, _size, 0};
    return axi_checkOverlapped(block);
};

int axi_reserveMemory(uint32_t _startAddress, uint32_t _size, uint64_t* _index) {
    std::lock_guard lock(g_mutex);
    *_index = ++g_index;
    block_t block{_startAddress, _size, *_index};
    if (axi_checkOverlapped(block)) {
        ERROR_LOG("Memory overlapped")
        return -1;
    }

    if (_startAddress == 0 && _size == 0) {
        ERROR_LOG("AXI mode not initialized");
        return RP_EOOR;
    }

    if (_startAddress < 8) {
        ERROR_LOG("The address must be greater than 8 bytes. Address: 0x%X", _startAddress);
        return RP_EOOR;
    }

    if (_startAddress < g_startRegion) {
        ERROR_LOG("Start address lower than reserved. Address: 0x%X reserved 0x%X", _startAddress, g_startRegion);
        return RP_EOOR;
    }

    if ((_startAddress + _size) > (g_startRegion + g_sizeRegion)) {
        ERROR_LOG("The specified buffer size is greater than the reserved memory. End address: 0x%X End reserved 0x%X", _startAddress + _size, g_startRegion + g_sizeRegion);
        return RP_EOOR;
    }

    ECHECK(osc_axi_map(block.size, block.start, (void**)&block.mapped))
    g_reserved.push_back(block);
    return RP_OK;
}

int axi_releaseMemory(uint64_t _index) {
    std::lock_guard lock(g_mutex);
    auto position = std::find_if(g_reserved.begin(), g_reserved.end(), [_index](const block_t& m) -> bool { return _index == m.index; });
    if (position != g_reserved.end()) {
        osc_axi_unmap(position->size, (void**)&position->mapped);
        g_reserved.erase(position);
        return RP_OK;
    }
    return -1;
}

int axi_getMapped(uint64_t _index, uint16_t** _mapped, uint32_t* size) {
    std::lock_guard lock(g_mutex);
    auto position = std::find_if(g_reserved.begin(), g_reserved.end(), [_index](const block_t& m) -> bool { return _index == m.index; });
    if (position != g_reserved.end()) {
        *_mapped = position->mapped;
        *size = position->size;
        return RP_OK;
    }
    *_mapped = (uint16_t*)MAP_FAILED;
    return -1;
}
