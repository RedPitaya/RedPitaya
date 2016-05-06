/**
 * @brief Red Pitaya library API interface implementation
 * @Author Red Pitaya
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <stdlib.h>
#include <stdint.h>

#include "common.h"
#include "id.h"

int rp_IdOpen(char *dev, rp_handle_uio_t *handle) {
    handle->length = ID_BASE_SIZE;
    int status = common_Open (dev, handle);
    if (status != RP_OK) {
        return status;
    }
    return RP_OK;
}

int rp_IdClose(rp_handle_uio_t *handle) {
    int status = common_Close (handle); 
    if (status != RP_OK) {
        return status;
    }
    return RP_OK;
}

/**
 * Identification
 */

int rp_IdGetID(rp_handle_uio_t *handle, uint32_t *id) {
    id_regset_t *regset = (id_regset_t *) handle->regset;
    *id = ioread32(&regset->id);
    return RP_OK;
}

int rp_IdGetEFUSE(rp_handle_uio_t *handle, uint32_t *efuse) {
    id_regset_t *regset = (id_regset_t *) handle->regset;
    *efuse = ioread32(&regset->efuse);
    return RP_OK;
}

int rp_IdGetDNA(rp_handle_uio_t *handle, uint64_t *dna) {
    id_regset_t *regset = (id_regset_t *) handle->regset;
    *dna = ((uint64_t) ioread32(&regset->dna_hi) << 32)
         | ((uint64_t) ioread32(&regset->dna_lo) <<  0);
// TODO: the current FPGA AXI4 bus implementation does not support 64bit transfers
//    *dna = ioread64(&regset->dna);
    return RP_OK;
}

int rp_IdGetGITH(rp_handle_uio_t *handle, uint32_t *gith) {
    id_regset_t *regset = (id_regset_t *) handle->regset;
    for (int unsigned i=0; i<5; i++)
        gith[i] = ioread32(&regset->gith[i]);
    return RP_OK;
}
