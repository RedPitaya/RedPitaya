/**
 * $Id: $
 *
 * @brief Red Pitaya library housekeeping module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */



#ifndef __HOUSEKEEPING_H
#define __HOUSEKEEPING_H

#include <stdint.h>
#include <stdbool.h>

// Base Housekeeping address
static const int HOUSEKEEPING_BASE_SIZE = 0x30;

// Housekeeping structure declaration
typedef struct {
    uint32_t id;            // 0x00
    uint32_t dna_lo;        // 0x04
    uint32_t dna_hi;        // 0x08
} housekeeping_regset_t;

int rp_HousekeepingInit(char *dev, rp_handle_uio_t *handle);
int rp_HousekeepingRelease(rp_handle_uio_t *handle);

/**
* Gets FPGA Synthesized ID
*/
int rp_IdGetID(rp_handle_uio_t *handle, uint32_t *id);

/**
* Gets FPGA Unique DNA
*/
int rp_IdGetDNA(rp_handle_uio_t *handle, uint64_t *dna);

#endif //__HOUSEKEEPING_H
