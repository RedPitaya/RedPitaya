/**
 * $Id: $
 *
 * @brief Red Pitaya library common module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef AXI_MANAGER_H
#define AXI_MANAGER_H

#include <stdint.h>
#include "common.h"

int axi_initManager();
int axi_releaseManager();
int axi_getOSReservedRegion(uint32_t *_startAddress, uint32_t *_size);
int axi_reserveMemory(uint32_t _startAddress, uint32_t _size, uint64_t *_index);
int axi_releaseMemory(uint64_t _index);
int axi_getMapped(uint64_t _index, uint16_t** _mapped, uint32_t *size);

#endif /* AXI_MANAGER_H */
