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
#include "housekeeping.h"

int rp_HousekeepingOpen(char *dev, rp_handle_uio_t *handle) {
    handle->length = HOUSEKEEPING_BASE_SIZE;
    int status = common_Open (dev, handle);
    if (status != RP_OK) {
        return status;
    }
    return RP_OK;
}

int rp_HousekeepingClose(rp_handle_uio_t *handle) {
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
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    *id = ioread32(&regset->id);
    return RP_OK;
}

int rp_IdGetDNA(rp_handle_uio_t *handle, uint64_t *dna) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    *dna = ((uint64_t) ioread32(&regset->dna_hi) << 32)
         | ((uint64_t) ioread32(&regset->dna_lo) <<  0);
    return RP_OK;
}

