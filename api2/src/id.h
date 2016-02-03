/**
 * $Id: $
 *
 * @brief Red Pitaya library id module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */



#ifndef __ID_H
#define __ID_H

#include <stdint.h>
#include <stdbool.h>

// Base Id address
static const int ID_BASE_SIZE = 0x1000;

// Id structure declaration
typedef struct {
    uint32_t id;            // 0x00
    uint32_t reserved_04;   // 0x04
    uint64_t dna;           // 0x04 - 0x08
    uint32_t gith[5];       // 0x10 - 0x20
} id_regset_t;

int rp_IdOpen(char *dev, rp_handle_uio_t *handle);
int rp_IdClose(rp_handle_uio_t *handle);

/**
* Gets FPGA Synthesized ID
*/
int rp_IdGetID(rp_handle_uio_t *handle, uint32_t *id);

/**
* Gets FPGA Unique DNA
*/
int rp_IdGetDNA(rp_handle_uio_t *handle, uint64_t *dna);

// get 160 bit GIT HASH
int rp_IdGetGITH(rp_handle_uio_t *handle, uint32_t *gith);

#endif //__ID_H
