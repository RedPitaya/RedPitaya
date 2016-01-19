/**
 * $Id: $
 *
 * @brief Red Pitaya library oscilloscope module implementation
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

#include "common.h"
#include "acquire.h"

// structure containing available ranges
static const float ranges [2] = {1.0, 20.0};

int rp_AcqInit(char *dev, rp_handle_uio_t *handle) {
    // make a copy of the device path
    handle->dev = (char*) malloc((strlen(dev)+1) * sizeof(char));
    strncpy(handle->dev, dev, strlen(dev)+1);
    // try opening the device
    handle->fd = open(handle->dev, O_RDWR);
    if (!handle->fd) {
        return -1;
    } else {
        // get regset pointer
        handle->regset = mmap(NULL, ACQUIRE_BASE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, handle->fd, 0x0);
        if (handle->regset == NULL) {
            return -1;
        }
    }
    return RP_OK;
}

int rp_AcqRelease(rp_handle_uio_t *handle) {
    // release regset
    munmap((void *) handle->regset, ACQUIRE_BASE_SIZE);
    // close device
    close (handle->fd);
    // free device path
    free(handle->dev);
    // free name
    // TODO
    return RP_OK;
}

/**
 * Equalization filters
 */

int rp_AcqSetEqFilters(rp_handle_uio_t *handle, int32_t aa, int32_t bb, int32_t kk, int32_t pp) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    iowrite32(aa, &regset->cfg_faa);
    iowrite32(bb, &regset->cfg_fbb);
    iowrite32(kk, &regset->cfg_fkk);
    iowrite32(pp, &regset->cfg_fpp);
    return RP_OK;
}

int rp_AcqGetEqFilters(rp_handle_uio_t *handle, int32_t *aa, int32_t *bb, int32_t *kk, int32_t *pp) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    *aa = ioread32(&regset->cfg_faa);
    *bb = ioread32(&regset->cfg_fbb);
    *kk = ioread32(&regset->cfg_fkk);
    *pp = ioread32(&regset->cfg_fpp);
    return RP_OK;
}

int rp_AcqSetAveraging(rp_handle_uio_t *handle, bool averaging) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    iowrite32(averaging, &regset->cfg_avg);
    return RP_OK;
}

int rp_AcqGetAveraging(rp_handle_uio_t *handle, bool *averaging) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    *averaging = ioread32(&regset->cfg_avg);
    return RP_OK;
}

int rp_AcqSetDecimation(rp_handle_uio_t *handle, uint32_t decimation) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    iowrite32(decimation, &regset->cfg_dec);
    return RP_OK;
}

int rp_AcqGetDecimation(rp_handle_uio_t *handle, uint32_t *decimation) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    *decimation = ioread32(&regset->cfg_dec);
    return RP_OK;
}

int rp_AcqSetShiftRight(rp_handle_uio_t *handle, uint32_t shift) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    iowrite32(shift, &regset->cfg_shr);
    return RP_OK;
}

int rp_AcqGetShiftRight(rp_handle_uio_t *handle, uint32_t *shift) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    *shift = ioread32(&regset->cfg_shr);
    return RP_OK;
}

int rp_AcqSetTriggerSrc(rp_handle_uio_t *handle, rp_acq_trig_src_t source) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    iowrite32(source, &regset->cfg_lvl);
    return RP_OK;
}

int rp_AcqGetTriggerSrc(rp_handle_uio_t *handle, rp_acq_trig_src_t *source) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    *source = (rp_acq_trig_src_t) ioread32(&regset->cfg_sel);
    return RP_OK;
}

int rp_AcqSetTriggerDelay(rp_handle_uio_t *handle, uint32_t value) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    iowrite32(value, &regset->cfg_dly);
    return RP_OK;
}

int rp_AcqGetTriggerDelay(rp_handle_uio_t *handle, uint32_t *value) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    *value = ioread32(&regset->cfg_dly);
    return RP_OK;
}

int rp_AcqSetTriggerLevel(rp_handle_uio_t *handle, float voltage) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    float range = ranges[ioread32(&regset->cfg_rng)];
    iowrite32((int32_t) (voltage / range * (1 << RP_ACQ_DWI)), &regset->cfg_lvl);
    return RP_OK;
}

int rp_AcqGetTriggerLevel(rp_handle_uio_t *handle, float *voltage) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    float range = ranges[ioread32(&regset->cfg_rng)];
    *voltage = ((float) (ioread32(&regset->cfg_lvl) >> RP_ACQ_DWI)) * range;
    return RP_OK;
}

int rp_AcqSetTriggerHyst(rp_handle_uio_t *handle, float voltage) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    float range = ranges[ioread32(&regset->cfg_rng)];
    iowrite32((uint32_t) (voltage / range * (1 << RP_ACQ_DWI)), &regset->cfg_hst);
    return RP_OK;
}

int rp_AcqGetTriggerHyst(rp_handle_uio_t *handle, float *voltage) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    float range = ranges[ioread32(&regset->cfg_rng)];
    *voltage = ((float) (ioread32(&regset->cfg_hst) >> RP_ACQ_DWI)) * range;
    return RP_OK;
}

int rp_AcqGetTriggerState(rp_handle_uio_t *handle, rp_acq_trig_state_t* state) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    *state = (ioread32(&regset->ctl) & RP_ACQ_CTL_TRG_MASK) != 0;
    return RP_OK;
}

int rp_AcqStart(rp_handle_uio_t *handle) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    iowrite32(RP_ACQ_CTL_ACQ_MASK, &regset->ctl);
    return RP_OK;
}

int rp_AcqStop(rp_handle_uio_t *handle) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    iowrite32(0x0, &regset->ctl);
    return RP_OK;
}

int rp_AcqReset(rp_handle_uio_t *handle) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    iowrite32(RP_ACQ_CTL_RST_MASK, &regset->ctl);
    return RP_OK;
}




int rp_AcqGetPreTriggerCounter(rp_handle_uio_t *handle, uint32_t* value) {
    return RP_OK;
}

int rp_AcqGetData(rp_handle_uio_t *handle, uint32_t *size, int16_t *buffer) {
    return RP_OK;
}

int rp_AcqGetBufSize(rp_handle_uio_t *handle, uint32_t *size) {
    return RP_OK;
}
