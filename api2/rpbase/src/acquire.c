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

#include "common.h"
#include "redpitaya/rp.h"
#include "acquire.h"

// structure containing available ranges
static const float ranges [2] = {1.0, 20.0};

// The FPGA register structure for oscilloscope
static volatile acq_regset_t (*regset)[RP_MNA];

/**
 * general
 */

int acq_Init() {
    cmn_Map(ACQ_BASE_SIZE, ACQ_BASE_ADDR, (void**)&regset);
    return RP_OK;
}

int acq_Release() {
    cmn_Unmap(ACQ_BASE_SIZE, (void**)&regset);
    return RP_OK;
}

/**
 * Equalization filters
 */

int rp_AcqSetEqFilters(int unsigned channel, int32_t aa, int32_t bb, int32_t kk, int32_t pp) {
    if (channel >= RP_MNA)  return RP_EPN;
    iowrite32(aa, &regset[channel]->cfg_faa);
    iowrite32(bb, &regset[channel]->cfg_fbb);
    iowrite32(kk, &regset[channel]->cfg_fkk);
    iowrite32(pp, &regset[channel]->cfg_fpp);
    return RP_OK;
}

int rp_AcqGetEqFilters(int unsigned channel, int32_t *aa, int32_t *bb, int32_t *kk, int32_t *pp) {
    if (channel >= RP_MNA)  return RP_EPN;
    *aa = ioread32(&regset[channel]->cfg_faa);
    *bb = ioread32(&regset[channel]->cfg_fbb);
    *kk = ioread32(&regset[channel]->cfg_fkk);
    *pp = ioread32(&regset[channel]->cfg_fpp);
    return RP_OK;
}

int rp_AcqSetAveraging(int unsigned channel, bool averaging) {
    if (channel >= RP_MNA)  return RP_EPN;
    iowrite32(averaging, &regset[channel]->cfg_avg);
    return RP_OK;
}

int rp_AcqGetAveraging(int unsigned channel, bool *averaging) {
    if (channel >= RP_MNA)  return RP_EPN;
    *averaging = ioread32(&regset[channel]->cfg_avg);
    return RP_OK;
}

int rp_AcqSetDecimation(int unsigned channel, uint32_t decimation) {
    if (channel >= RP_MNA)  return RP_EPN;
    iowrite32(decimation, &regset[channel]->cfg_dec);
    return RP_OK;
}

int rp_AcqGetDecimation(int unsigned channel, uint32_t *decimation) {
    if (channel >= RP_MNA)  return RP_EPN;
    *decimation = ioread32(&regset[channel]->cfg_dec);
    return RP_OK;
}

int rp_AcqSetShiftRight(int unsigned channel, uint32_t shift) {
    if (channel >= RP_MNA)  return RP_EPN;
    iowrite32(shift, &regset[channel]->cfg_shr);
    return RP_OK;
}

int rp_AcqGetShiftRight(int unsigned channel, uint32_t *shift) {
    if (channel >= RP_MNA)  return RP_EPN;
    *shift = ioread32(&regset[channel]->cfg_shr);
    return RP_OK;
}

int rp_AcqSetTriggerSrc(int unsigned channel, rp_acq_trig_src_t source) {
    if (channel >= RP_MNA)  return RP_EPN;
    iowrite32(source, &regset[channel]->cfg_lvl);
    return RP_OK;
}

int rp_AcqGetTriggerSrc(int unsigned channel, rp_acq_trig_src_t *source) {
    if (channel >= RP_MNA)  return RP_EPN;
    *source = (rp_acq_trig_src_t) ioread32(&regset[channel]->cfg_sel);
    return RP_OK;
}

int rp_AcqSetTriggerDelay(int unsigned channel, uint32_t value) {
    if (channel >= RP_MNA)  return RP_EPN;
    iowrite32(value, &regset[channel]->cfg_dly);
    return RP_OK;
}

int rp_AcqGetTriggerDelay(int unsigned channel, uint32_t *value) {
    if (channel >= RP_MNA)  return RP_EPN;
    *value = ioread32(&regset[channel]->cfg_dly);
    return RP_OK;
}

int rp_AcqSetTriggerLevel(int unsigned channel, float voltage) {
    if (channel >= RP_MNA)  return RP_EPN;
    float range = ranges[ioread32(&regset[channel]->cfg_rng)];
    iowrite32((int32_t) (voltage / range * (1 << RP_ACQ_DWI)), &regset[channel]->cfg_lvl);
    return RP_OK;
}

int rp_AcqGetTriggerLevel(int unsigned channel, float *voltage) {
    if (channel >= RP_MNA)  return RP_EPN;
    float range = ranges[ioread32(&regset[channel]->cfg_rng)];
    *voltage = ((float) (ioread32(&regset[channel]->cfg_lvl) >> RP_ACQ_DWI)) * range;
    return RP_OK;
}

int rp_AcqSetTriggerHyst(int unsigned channel, float voltage) {
    if (channel >= RP_MNA)  return RP_EPN;
    float range = ranges[ioread32(&regset[channel]->cfg_rng)];
    iowrite32((uint32_t) (voltage / range * (1 << RP_ACQ_DWI)), &regset[channel]->cfg_hst);
    return RP_OK;
}

int rp_AcqGetTriggerHyst(int unsigned channel, float *voltage) {
    if (channel >= RP_MNA)  return RP_EPN;
    float range = ranges[ioread32(&regset[channel]->cfg_rng)];
    *voltage = ((float) (ioread32(&regset[channel]->cfg_hst) >> RP_ACQ_DWI)) * range;
    return RP_OK;
}

int rp_AcqGetTriggerState(int unsigned channel, rp_acq_trig_state_t* state) {
    if (channel >= RP_MNA)  return RP_EPN;
    *state = (ioread32(&regset[channel]->ctl) & RP_ACQ_CTL_TRG_MASK) != 0;
    return RP_OK;
}

int rp_AcqStart(int unsigned channel) {
    if (channel >= RP_MNA)  return RP_EPN;
    iowrite32(RP_ACQ_CTL_ACQ_MASK, &regset[channel]->ctl);
    return RP_OK;
}

int rp_AcqStop(int unsigned channel) {
    if (channel >= RP_MNA)  return RP_EPN;
    iowrite32(0x0, &regset[channel]->ctl);
    return RP_OK;
}

int rp_AcqReset(int unsigned channel) {
    if (channel >= RP_MNA)  return RP_EPN;
    iowrite32(RP_ACQ_CTL_RST_MASK, &regset[channel]->ctl);
    return RP_OK;
}




int rp_AcqGetPreTriggerCounter(int unsigned channel, uint32_t* value) {
    if (channel >= RP_MNA)  return RP_EPN;
    return RP_OK;
}

int rp_AcqGetData(rp_channel_t channel, uint32_t *size, int16_t *buffer) {
    if (channel >= RP_MNA)  return RP_EPN;
    return RP_OK;
}

int rp_AcqGetBufSize(int unsigned channel, uint32_t *size) {
    if (channel >= RP_MNA)  return RP_EPN;
    return RP_OK;
}
