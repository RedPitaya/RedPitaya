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

int rp_AcqSetEqFilter(rp_handle_uio_t *handle, rp_adc_eqfilter_regset_t *fil) {
    rp_adc_eqfilter_regset_t *regset = (rp_adc_eqfilter_regset_t *) &(((acq_regset_t *) handle->regset)->fil);
    iowrite32(fil->byp, &regset->byp);
    iowrite32(fil->aa , &regset->aa );
    iowrite32(fil->bb , &regset->bb );
    iowrite32(fil->kk , &regset->kk );
    iowrite32(fil->pp , &regset->pp );
    return RP_OK;
}

int rp_AcqGetEqFilter(rp_handle_uio_t *handle, rp_adc_eqfilter_regset_t *fil) {
    rp_adc_eqfilter_regset_t *regset = (rp_adc_eqfilter_regset_t *) &(((acq_regset_t *) handle->regset)->fil);
    fil->byp = ioread32(&regset->byp);
    fil->aa  = ioread32(&regset->aa );
    fil->bb  = ioread32(&regset->bb );
    fil->kk  = ioread32(&regset->kk );
    fil->pp  = ioread32(&regset->pp );
    return RP_OK;
}

/**
 * Decimation
 */

int rp_AcqSetDecimation(rp_handle_uio_t *handle, rp_scope_decimation_regset_t *dec) {
    rp_scope_decimation_regset_t *regset = (rp_scope_decimation_regset_t *) &(((acq_regset_t *) handle->regset)->dec);
    iowrite32(dec->avg, &regset->avg);
    iowrite32(dec->dec, &regset->dec);
    iowrite32(dec->shr, &regset->shr);
    return RP_OK;
}

int rp_AcqGetDecimation(rp_handle_uio_t *handle, rp_scope_decimation_regset_t *dec) {
    rp_scope_decimation_regset_t *regset = (rp_scope_decimation_regset_t *) &(((acq_regset_t *) handle->regset)->dec);
    dec->dec = ioread32(&regset->dec);
    dec->avg = ioread32(&regset->avg);
    dec->shr = ioread32(&regset->shr);
    return RP_OK;
}

/**
 * Trigger
 */

int rp_AcqSetTrigger(rp_handle_uio_t *handle, float lvl, float hst) {
    rp_scope_trigger_regset_t *regset = (rp_scope_trigger_regset_t *) &(((acq_regset_t *) handle->regset)->trg);
    float range = ranges[ioread32(&regset->rng)];
    iowrite32(( int32_t) (lvl / range * (1 << RP_ACQ_DWI)), &regset->lvl);
    iowrite32((uint32_t) (hst / range * (1 << RP_ACQ_DWI)), &regset->hst);
    return RP_OK;
}

int rp_AcqGetTrigger(rp_handle_uio_t *handle, float *lvl, float *hst) {
    rp_scope_trigger_regset_t *regset = (rp_scope_trigger_regset_t *) &(((acq_regset_t *) handle->regset)->trg);
    float range = ranges[ioread32(&regset->rng)];
    *lvl = ((float) (ioread32(&regset->lvl) >> RP_ACQ_DWI)) * range;
    *hst = ((float) (ioread32(&regset->hst) >> RP_ACQ_DWI)) * range;
    return RP_OK;
}

int rp_AcqSetTriggerSrc(rp_handle_uio_t *handle, uint32_t source) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    iowrite32(source, &regset->cfg_sel);
    return RP_OK;
}

int rp_AcqGetTriggerSrc(rp_handle_uio_t *handle, uint32_t *source) {
    acq_regset_t *regset = (acq_regset_t *) handle->regset;
    *source = ioread32(&regset->cfg_sel);
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

int rp_AcqGetTriggerState(rp_handle_uio_t *handle, uint32_t *state) {
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
