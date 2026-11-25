/**
 * $Id: $
 *
 * @brief Red Pitaya DMA library
 *
 * The LA capture goes through the Xilinx AXI DMA (axi_dma_2 of the logic project,
 * S2MM, scatter gather) into the reserved memory region. The DMA registers are
 * mapped through generic-uio and the region through /dev/rp_resmem (the driver the
 * streaming uses), so no DMA client driver is needed in the kernel.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __RP_LA_DMA_H
#define __RP_LA_DMA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string>

#include "structs.h"

// The descriptors of the ring take the end of the reserved region
#define RP_DMA_DESC_AREA 4096

/** Maps the DMA registers (uio device) and the reserved region (/dev/rp_resmem) */
int rp_dmaOpen(const std::string dev, rp_handle_uio_t* handle);
int rp_dmaClose(rp_handle_uio_t* handle);

/** Bytes the ring can take: the reserved region without the descriptors */
size_t rp_dmaCapacity(rp_handle_uio_t* handle);

/** Sets the ring of the next capture from the start of the region (a multiple of 8 bytes, at most the capacity) */
int rp_dmaSetRing(rp_handle_uio_t* handle, size_t bytes);

/** Starts the cyclic transfer into the ring; the stream overwrites the oldest data */
int rp_dmaStart(rp_handle_uio_t* handle);

/** Stops the transfer (resets the DMA); reports a DMA error of the last transfer */
int rp_dmaStop(rp_handle_uio_t* handle);

/** Start address and size of the reserved region (sysfs of /dev/rp_resmem) */
int rp_dmaReservedMemory(uint32_t* _startAddress, uint32_t* _size);

#endif
