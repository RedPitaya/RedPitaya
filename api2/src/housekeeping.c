/**
 * @brief Red Pitaya library API interface implementation
 * @Author Red Pitaya
 * (c) Red Pitaya  http://www.redpitaya.com
 */

// for Init
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <stdlib.h>
#include <stdint.h>

#include "common.h"
#include "housekeeping.h"

int rp_HousekeepingInit(char *dev, rp_handle_uio_t *handle) {
    // make a copy of the device path
    handle->dev = (char*) malloc((strlen(dev)+1) * sizeof(char));
    strncpy(handle->dev, dev, strlen(dev)+1);
    // try opening the device
    handle->fd = open(handle->dev, O_RDWR);
    if (!handle->fd) {
        return -1;
    } else {
        // get regset pointer
        handle->regset = mmap(NULL, HOUSEKEEPING_BASE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, handle->fd, 0x0);
        if (handle->regset == NULL) {
            return -1;
        }
    }
    return RP_OK;
}

int rp_HousekeepingRelease(rp_handle_uio_t *handle) {
    // release regset
    munmap((void *) handle->regset, HOUSEKEEPING_BASE_SIZE);
    // close device
    close (handle->fd);
    // free device path
    free(handle->dev);
    // free name
    // TODO
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

