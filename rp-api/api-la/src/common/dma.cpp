#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "dma.h"
#include "rp.h"

/*
 * User-space driver of the S2MM channel of the Xilinx AXI DMA (PG021) in cyclic
 * scatter gather mode, the way xilinx_dma runs a cyclic transfer: a ring of buffer
 * descriptors with the last one pointing back to the first, CYCLIC_BD_EN so the
 * engine ignores the completed bit, and the tail pointer on a descriptor outside
 * the ring so the engine never stops on it.
 */

#define RESMEM_DEVICE "/dev/rp_resmem"
#define RESMEM_SYSFS "/sys/class/misc/rp_resmem/"

// Register window of the AXI DMA
#define DMA_REGS_SIZE 0x10000

// S2MM registers (byte offsets)
#define S2MM_DMACR 0x30
#define S2MM_DMASR 0x34
#define S2MM_CURDESC 0x38
#define S2MM_CURDESC_MSB 0x3C
#define S2MM_TAILDESC 0x40
#define S2MM_TAILDESC_MSB 0x44

#define DMACR_RS (1u << 0)
#define DMACR_RESET (1u << 2)
#define DMACR_CYCLIC (1u << 4)

#define DMASR_HALTED (1u << 0)
#define DMASR_ERRORS 0x770u  // DMAIntErr, DMASlvErr, DMADecErr, SGIntErr, SGSlvErr, SGDecErr

// Buffer descriptor: 16 words, 64-byte aligned
#define DESC_SIZE 64
#define DESC_NEXT 0
#define DESC_NEXT_MSB 1
#define DESC_BUFFER 2
#define DESC_BUFFER_MSB 3
#define DESC_CONTROL 6
#define DESC_STATUS 7

// One descriptor moves less than 2^23 bytes (xlnx,sg-length-width = 23)
#define DESC_MAX_BYTES (4 * 1024 * 1024)

// The 64-bit memory side of the DMA writes whole words
#define RING_ALIGN 8

#define WAIT_US 10000

static uint32_t regRead(rp_handle_uio_t* handle, uint32_t offset) {
    return handle->dma_regs[offset / 4];
}

static void regWrite(rp_handle_uio_t* handle, uint32_t offset, uint32_t value) {
    handle->dma_regs[offset / 4] = value;
}

/*
 * The descriptors are in write-combined memory: the writes must reach the DDR
 * before the DMA is told to fetch them. The barrier orders them before the
 * register writes; the outer cache (PL310) store buffer, which the kernel drains
 * with outer_sync(), cannot be drained from user space, so the start waits for it.
 */
static void memoryBarrier() {
#if defined(__arm__) || defined(__aarch64__)
    asm volatile("dsb sy" ::: "memory");
#else
    __sync_synchronize();
#endif
}

static bool waitReg(rp_handle_uio_t* handle, uint32_t offset, uint32_t mask, uint32_t value) {
    for (int i = 0; i < WAIT_US; i++) {
        if ((regRead(handle, offset) & mask) == value) {
            return true;
        }
        usleep(1);
    }
    return (regRead(handle, offset) & mask) == value;
}

static bool readSysfsValue(const char* name, unsigned long long* value) {
    std::string path = std::string(RESMEM_SYSFS) + name;
    FILE* f = fopen(path.c_str(), "r");
    if (f == NULL) {
        return false;
    }
    bool ok = fscanf(f, "%llx", value) == 1;
    fclose(f);
    return ok;
}

int rp_dmaReservedMemory(uint32_t* _startAddress, uint32_t* _size) {
    *_startAddress = 0;
    *_size = 0;
    unsigned long long base = 0, size = 0;
    if (!readSysfsValue("base", &base) || !readSysfsValue("size", &size)) {
        ERROR_LOG("Error reading %s{base,size}: is the rp_resmem driver loaded?", RESMEM_SYSFS)
        return RP_EOMD;
    }
    if (base > UINT32_MAX || size > UINT32_MAX) {
        ERROR_LOG("The reserved region 0x%llX + 0x%llX is out of the 32-bit DMA range", base, size)
        return RP_EOMD;
    }
    *_startAddress = (uint32_t)base;
    *_size = (uint32_t)size;
    return RP_OK;
}

int rp_dmaOpen(const std::string dev, rp_handle_uio_t* handle) {
    uint32_t start = 0, size = 0;
    if (rp_dmaReservedMemory(&start, &size) != RP_OK) {
        return RP_EOMD;
    }
    if (size <= RP_DMA_DESC_AREA) {
        ERROR_LOG("The reserved region (0x%X bytes) is too small", size)
        return RP_EOMD;
    }

    handle->dma_dev = dev;
    handle->dma_reg_fd = open(dev.c_str(), O_RDWR | O_SYNC);
    if (handle->dma_reg_fd == -1) {
        ERROR_LOG("Error open device: %s", dev.c_str())
        return RP_EOED;
    }
    void* regs = mmap(NULL, DMA_REGS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, handle->dma_reg_fd, 0);
    if (regs == MAP_FAILED) {
        ERROR_LOG("Error map %s", dev.c_str())
        rp_dmaClose(handle);
        return RP_EMMD;
    }
    handle->dma_regs = (volatile uint32_t*)regs;

    handle->dma_mem_fd = open(RESMEM_DEVICE, O_RDWR);
    if (handle->dma_mem_fd == -1) {
        ERROR_LOG("Error open device: %s", RESMEM_DEVICE)
        rp_dmaClose(handle);
        return RP_EOMD;
    }
    void* mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, handle->dma_mem_fd, 0);
    if (mem == MAP_FAILED) {
        ERROR_LOG("Error map %s", RESMEM_DEVICE)
        rp_dmaClose(handle);
        return RP_EMMD;
    }
    handle->dma_mem = mem;
    handle->dma_phys = start;
    handle->dma_mem_size = size;
    handle->dma_size = 0;
    return RP_OK;
}

int rp_dmaClose(rp_handle_uio_t* handle) {
    int r = RP_OK;
    if (handle->dma_regs != NULL) {
        rp_dmaStop(handle);
        if (munmap((void*)handle->dma_regs, DMA_REGS_SIZE) == -1) {
            r = RP_EUMD;
        }
        handle->dma_regs = NULL;
    }
    if (handle->dma_mem != NULL) {
        if (munmap(handle->dma_mem, handle->dma_mem_size) == -1) {
            r = RP_EUMD;
        }
        handle->dma_mem = NULL;
    }
    if (handle->dma_reg_fd != -1) {
        if (close(handle->dma_reg_fd) == -1) {
            r = RP_ECMD;
        }
        handle->dma_reg_fd = -1;
    }
    if (handle->dma_mem_fd != -1) {
        if (close(handle->dma_mem_fd) == -1) {
            r = RP_ECMD;
        }
        handle->dma_mem_fd = -1;
    }
    handle->dma_size = 0;
    handle->dma_mem_size = 0;
    return r;
}

size_t rp_dmaCapacity(rp_handle_uio_t* handle) {
    if (handle->dma_mem_size <= RP_DMA_DESC_AREA) {
        return 0;
    }
    return (handle->dma_mem_size - RP_DMA_DESC_AREA) & ~(size_t)(RING_ALIGN - 1);
}

int rp_dmaSetRing(rp_handle_uio_t* handle, size_t bytes) {
    if (handle->dma_mem == NULL) {
        ERROR_LOG("The DMA is not open")
        return RP_EOOR;
    }
    if (bytes == 0 || bytes % RING_ALIGN != 0 || bytes > rp_dmaCapacity(handle)) {
        ERROR_LOG("Wrong DMA ring size 0x%zX (capacity 0x%zX)", bytes, rp_dmaCapacity(handle))
        return RP_EOOR;
    }
    // the ring descriptors and the tail descriptor
    if ((bytes + DESC_MAX_BYTES - 1) / DESC_MAX_BYTES + 1 > RP_DMA_DESC_AREA / DESC_SIZE) {
        ERROR_LOG("The DMA ring 0x%zX needs more descriptors than fit in 0x%X bytes", bytes, RP_DMA_DESC_AREA)
        return RP_EOOR;
    }
    handle->dma_size = bytes;
    return RP_OK;
}

/*
 * Writes the descriptors of the ring at the end of the region: one per
 * DESC_MAX_BYTES of the ring, the last one back to the first, then one more
 * outside the ring for the tail pointer.
 */
static uint32_t writeDescriptors(rp_handle_uio_t* handle) {
    size_t descOffset = handle->dma_mem_size - RP_DMA_DESC_AREA;
    uint32_t descPhys = handle->dma_phys + descOffset;
    volatile uint32_t* desc = (volatile uint32_t*)((uint8_t*)handle->dma_mem + descOffset);
    size_t count = (handle->dma_size + DESC_MAX_BYTES - 1) / DESC_MAX_BYTES;

    for (size_t i = 0; i <= count; i++) {
        volatile uint32_t* d = desc + i * DESC_SIZE / 4;
        for (int w = 0; w < DESC_SIZE / 4; w++) {
            d[w] = 0;
        }
        if (i == count) {
            break;  // the tail descriptor: never fetched
        }
        size_t offset = i * DESC_MAX_BYTES;
        size_t len = handle->dma_size - offset < DESC_MAX_BYTES ? handle->dma_size - offset : DESC_MAX_BYTES;
        d[DESC_NEXT] = descPhys + (i + 1 < count ? i + 1 : 0) * DESC_SIZE;
        d[DESC_BUFFER] = handle->dma_phys + offset;
        d[DESC_CONTROL] = len;
    }
    memoryBarrier();
    return descPhys;
}

int rp_dmaStart(rp_handle_uio_t* handle) {
    if (handle->dma_regs == NULL || handle->dma_size == 0) {
        ERROR_LOG("The DMA is not open")
        return RP_EOOR;
    }
    rp_dmaStop(handle);

    uint32_t first = writeDescriptors(handle);
    uint32_t count = (handle->dma_size + DESC_MAX_BYTES - 1) / DESC_MAX_BYTES;
    usleep(100);

    regWrite(handle, S2MM_DMACR, DMACR_CYCLIC);
    regWrite(handle, S2MM_CURDESC_MSB, 0);
    regWrite(handle, S2MM_CURDESC, first);
    regWrite(handle, S2MM_DMACR, DMACR_CYCLIC | DMACR_RS);
    if (!waitReg(handle, S2MM_DMASR, DMASR_HALTED, 0)) {
        ERROR_LOG("The DMA does not start: DMASR 0x%08X", regRead(handle, S2MM_DMASR))
        rp_dmaStop(handle);
        return RP_EOP;
    }
    regWrite(handle, S2MM_TAILDESC_MSB, 0);
    regWrite(handle, S2MM_TAILDESC, first + count * DESC_SIZE);
    return RP_OK;
}

int rp_dmaStop(rp_handle_uio_t* handle) {
    if (handle->dma_regs == NULL) {
        return RP_OK;
    }
    int r = RP_OK;
    uint32_t sr = regRead(handle, S2MM_DMASR);
    if (sr & DMASR_ERRORS) {
        ERROR_LOG("DMA error: DMASR 0x%08X", sr)
        r = RP_EOP;
    }
    // the reset of the core clears the run bit and the cyclic mode
    regWrite(handle, S2MM_DMACR, DMACR_RESET);
    if (!waitReg(handle, S2MM_DMACR, DMACR_RESET, 0)) {
        ERROR_LOG("The DMA reset does not end: DMACR 0x%08X", regRead(handle, S2MM_DMACR))
        r = RP_EOP;
    }
    return r;
}
