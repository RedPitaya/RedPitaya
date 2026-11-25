/**
 * $Id: $
 *
 * @brief Red Pitaya DMA library
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __RP_LA_STRUCTS_H
#define __RP_LA_STRUCTS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string>

typedef struct {
    std::string name;
    std::string dev;
    size_t length;
    int fd;
    volatile void* regset = NULL;
    void* context;
    int struct_size;  ///< only used to reserve dummy memory space for testing

    // DMA (common/dma.cpp): AXI DMA registers through uio, the ring in /dev/rp_resmem
    std::string dma_dev;
    int dma_reg_fd = -1;
    volatile uint32_t* dma_regs = NULL;
    int dma_mem_fd = -1;
    void* dma_mem = NULL;     ///< the whole reserved region
    uint32_t dma_phys = 0;    ///< physical address of the region
    size_t dma_mem_size = 0;  ///< size of the region
    size_t dma_size = 0;      ///< the ring of the capture, from the start of the region
} rp_handle_uio_t;

typedef struct {
    bool inProcess;
    bool isCapture;
    bool isTimeout;
    int16_t* buf = NULL;
    size_t buf_size;
    uint32_t pre_samples;
    uint32_t post_samples;
    uint32_t trig_blockIndexRLE;  // RLE: index of pair (count,value).
    uint32_t trig_indexNonRLE;
    uint32_t last_blockIndexRLE;
    uint32_t last_indexNonRLE;

    auto reset() {
        inProcess = false;
        isCapture = false;
        isTimeout = false;
        buf = NULL;
        buf_size = 0;
        pre_samples = 0;
        post_samples = 0;
        trig_blockIndexRLE = 0;
        trig_indexNonRLE = 0;
        last_blockIndexRLE = 0;
        last_indexNonRLE = 0;
    }
} rp_acq_data_t;

#define RP_TRG_LOA_PAT_MASK (1 << 1)  ///< logic analyzer pattern trigger
#define RP_TRG_LOA_SWE_MASK (1 << 0)  ///< logic analyzer software trigger

#define RP_TRG_ALL_MASK 0

// /** control register masks  */
#define RP_CTL_STO_MASK (1 << 3)  ///< 1 - ACQ: stops / aborts the acq. ; returns 1 when acq. is stopped
#define RP_CTL_STA_MASK (1 << 2)  ///< 1 - ACQ: starts acq.
#define RP_CTL_SWT_MASK (1 << 1)  ///< 1 - sw trigger bit (sw trigger must be enabled)
#define RP_CTL_RST_MASK (1 << 0)  ///< 1 - reset state machine so that it is in known state

#endif