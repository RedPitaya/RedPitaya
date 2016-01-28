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

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include "common.h"
#include "pdm.h"

int rp_PdmOpen(char *dev, rp_handle_uio_t *handle) {
    handle->length = PDM_BASE_SIZE;
    int status = common_Open (dev, handle);
    if (status != RP_OK) {
        return status;
    }
    return RP_OK;
}

int rp_PdmClose(rp_handle_uio_t *handle) {
    int status = common_Close (handle); 
    if (status != RP_OK) {
        return status;
    }
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

