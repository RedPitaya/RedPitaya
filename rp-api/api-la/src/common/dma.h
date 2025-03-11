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

#ifndef __RP_LA_DMA_H
#define __RP_LA_DMA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string>

#include "structs.h"

int rp_dmaOpen(const std::string dev, rp_handle_uio_t* handle);
int rp_dmaCtrl(rp_handle_uio_t* handle, RP_DMA_CTRL ctrl);
int rp_setSgmntC(rp_handle_uio_t* handle, unsigned long no);
int rp_setSgmntS(rp_handle_uio_t* handle, unsigned long no);
int rp_dmaRead(rp_handle_uio_t* handle, int timeout_s, bool* timeOut);
int rp_dmaClose(rp_handle_uio_t* handle);

#endif