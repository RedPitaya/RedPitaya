/**
 * $Id: $
 *
 * @brief Red Pitaya library API interface implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
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
#include "pdm.h"

int rp_PdmInit(char *dev, rp_handle_uio_t *handle) {
    // make a copy of the device path
    handle->dev = (char*) malloc((strlen(dev)+1) * sizeof(char));
    strncpy(handle->dev, dev, strlen(dev)+1);
    // try opening the device
    handle->fd = open(handle->dev, O_RDWR);
    if (!handle->fd) {
        return -1;
    } else {
        // get regset pointer
        handle->regset = (volatile pdm_regset_t *) mmap(NULL, PDM_BASE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, handle->fd, 0x0);
        if (handle->regset == NULL) {
            return -1;
        }
    }
    return RP_OK;
}

int rp_PdmRelease(rp_handle_uio_t *handle) {
    // release regset
    munmap((void *) handle->regset, PDM_BASE_SIZE);
    // close device
    close (handle->fd);
    // free device path
    free(handle->dev);
    // free name
    // TODO
    return RP_OK;
}

int rp_PdmReset(rp_handle_uio_t *handle) {
    for (int unsigned pin=0; pin<4; pin++) {
        rp_PdmSetValueRaw(handle, pin, 0);
    }
    return RP_OK;
}

int rp_PdmSetValueRaw(rp_handle_uio_t *handle, int unsigned pin, uint32_t value) {
    pdm_regset_t *regset = (pdm_regset_t *) handle->regset;
    if (pin >= 4) {
        return RP_EPN;
    }
    if (value > PDM_MAX_VAL_INTEGER) {
        return RP_EOOR;

    }
    iowrite32(value & PDM_MASK, &regset->pdm_cfg[pin]);
    return RP_OK;
}

int rp_PdmSetValue(rp_handle_uio_t *handle, int unsigned pin, float value) {
    uint32_t value_raw = (uint32_t) (((value - PDM_MIN_VAL) / (PDM_MAX_VAL - PDM_MIN_VAL)) * PDM_MAX_VAL_INTEGER);
    return rp_PdmSetValueRaw(handle, pin, value_raw);
}

int rp_PdmGetValueRaw(rp_handle_uio_t *handle, int unsigned pin, uint32_t* value) {
    pdm_regset_t *regset = (pdm_regset_t *) handle->regset;
    if (pin >= 4) {
        return RP_EPN;
    }
    *value = ioread32(&regset->pdm_cfg[pin]) & PDM_MASK;
    return RP_OK;
}

int rp_PdmGetValue(rp_handle_uio_t *handle, int unsigned pin, float* value) {
    uint32_t value_raw;
    int result = rp_PdmGetValueRaw(handle, pin, &value_raw);
    *value = (((float)value_raw / PDM_MAX_VAL_INTEGER) * (PDM_MAX_VAL - PDM_MIN_VAL)) + PDM_MIN_VAL;
    return result;
}

int rp_PdmGetRange(rp_handle_uio_t *handle, int unsigned pin, float* min_val,  float* max_val) {
    *min_val = PDM_MIN_VAL;
    *max_val = PDM_MAX_VAL;
    return RP_OK;
}

