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

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "acquire.h"

// structure containing available ranges
static const float ranges [2] = {1.0, 20.0};

int rp_AcqOpen(char *dev, rp_handle_uio_t *handle) {
    handle->length = ACQUIRE_BASE_SIZE;
    int status = common_Open (dev, handle);
    if (status != RP_OK) {
        return status;
    }
    return RP_OK;
}

int rp_AcqClose(rp_handle_uio_t *handle) {
    int status = common_Close (handle); 
    if (status != RP_OK) {
        return status;
    }
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
