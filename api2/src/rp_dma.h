/**
 * $Id: $
 *
 * @brief Red Pitaya DMA library
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef _RP_DMA_H_
#define _RP_DMA_H_

#include "rp_dma.h"

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    RP_DMA_SINGLE,
    RP_DMA_CYCLIC,
    RP_DMA_STOP_RX
} RP_DMA_CTRL;


int rp_DmaOpen(const char *dev, rp_handle_uio_t *handle);
int rp_DmaCtrl(rp_handle_uio_t *handle, RP_DMA_CTRL ctrl);
int rp_SetSgmntC(rp_handle_uio_t *handle, unsigned long no);
int rp_SetSgmntS(rp_handle_uio_t *handle, unsigned long no);
int rp_DmaRead(rp_handle_uio_t *handle);
int rp_DmaMemDump(rp_handle_uio_t *handle);
int rp_DmaClose(rp_handle_uio_t *handle);

#endif // _RP_DMA_H_
