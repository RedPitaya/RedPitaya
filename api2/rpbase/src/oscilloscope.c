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
#include "oscilloscope.h"

// structure containing available ranges
static const float ranges [2] = {1.0, 20.0};

// The FPGA register structure for oscilloscope
static volatile regsetset_t *regset = NULL;

/**
 * general
 */

int osc_Init() {
    cmn_Map(OSC_BASE_SIZE, OSC_BASE_ADDR, (void**)&regset);
    return RP_OK;
}

int osc_Release() {
    cmn_Unmap(OSC_BASE_SIZE, (void**)&regset);
    return RP_OK;
}

/**
 * Equalization filters
 */

int rp_AcqSetEqFilters(int32_t aa, bb, kk, pp) {
    if (channel >= RP_MNG)  return RP_EPN;
    iowrite32(&regset[channel]->cfg_faa, aa);
    iowrite32(&regset[channel]->cfg_fbb, bb);
    iowrite32(&regset[channel]->cfg_fkk, kk);
    iowrite32(&regset[channel]->cfg_fpp, pp);
    return RP_OK;
}

int rp_AcqGetEqFilters(int32_t *aa, *bb, *kk, *pp) {
    if (channel >= RP_MNG)  return RP_EPN;
    ioread32(&regset[channel]->cfg_faa, aa);
    ioread32(&regset[channel]->cfg_fbb, bb);
    ioread32(&regset[channel]->cfg_fkk, kk);
    ioread32(&regset[channel]->cfg_fpp, pp);
    return RP_OK;
}

int rp_AcqSetAveraging(int unsigned channel, bool averaging) {
    if (channel >= RP_MNG)  return RP_EPN;
    iowrite32(averageing, &regset[channel]->cfg_avg);
    return RP_OK;
}

int rp_AcqGetAveraging(int unsigned channel, bool *averaging) {
    if (channel >= RP_MNG)  return RP_EPN;
    *averaging = ioread32(&regset[channel]->cfg_avg);
    return RP_OK;
}

int rp_AcqSetDecimation(int unsigned channel, uint32_t decimation) {
    if (channel >= RP_MNG)  return RP_EPN;
    iowrite32(decimation, &regset[channel]->cfg_dec);
    return RP_OK;
}

int rp_AcqGetDecimation(int unsigned channel, uint32_t *decimation) {
    if (channel >= RP_MNG)  return RP_EPN;
    *decimation = ioread32(&regset[channel]->cfg_dec);
    return RP_OK;
}

int rp_AcqSetShiftRight(int unsigned channel, uint32_t shift) {
    if (channel >= RP_MNG)  return RP_EPN;
    iowrite32(shift, &regset[channel]->cfg_shr);
    return RP_OK;
}

int rp_AcqGetShiftRight(int unsigned channel, uint32_t *shift) {
    if (channel >= RP_MNG)  return RP_EPN;
    *shift = ioread32(&regset[channel]->cfg_shr);
    return RP_OK;
}

int rp_AcqSetTriggerSrc(int unsigned channel, rp_acq_trig_src_t source) {
    if (channel >= RP_MNG)  return RP_EPN;
    iowrite32(source, &regset[channel]->cfg_lvl);
    return RP_OK;
}

int rp_AcqGetTriggerSrc(int unsigned channel, rp_acq_trig_src_t *source) {
    if (channel >= RP_MNG)  return RP_EPN;
    *source = (rp_acq_trig_src_t) ioread32(&regset[channel]->cfg_sel);
    return RP_OK;
}

int rp_AcqSetTriggerDelay(int unsigned channel, uint32_t value) {
    if (channel >= RP_MNG)  return RP_EPN;
    iowrite32(value, &regset[channel]->cfg_dly);
    return RP_OK;
}

int rp_AcqGetTriggerDelay(int unsigned channel, uint32_t *value) {
    if (channel >= RP_MNG)  return RP_EPN;
    *value = ioread32(&regset[channel]->cfg_dly);
    return RP_OK;
}

int rp_AcqSetTriggerLevel(int unsigned channel, float voltage) {
    if (channel >= RP_MNG)  return RP_EPN;
    float range = ranges[ioread32(&regset[channel]->cfg_rng)];
    iowrite32((uint32_t) (voltage / range * (1 << RP_ACQ_TODO)), &regset[channel]->cfg_lvl);
    return RP_OK;
}

int rp_AcqGetTriggerLevel(int unsigned channel, float *voltage) {
    if (channel >= RP_MNG)  return RP_EPN;
    float range = ranges[ioread32(&regset[channel]->cfg_rng)];
    *voltage = (float) (ioread32(&regset[channel]->cfg_lvl) >> RP_ACQ_TODO) * range);
    return RP_OK;
}

int rp_AcqSetTriggerHyst(int unsigned channel, float voltage) {
    if (channel >= RP_MNG)  return RP_EPN;
    float range = ranges[ioread32(&regset[channel]->cfg_rng)];
    iowrite32((uint32_t) (voltage / range * (1 << RP_ACQ_TODO)), &regset[channel]->cfg_hst);
    return RP_OK;
}

int rp_AcqGetTriggerHyst(int unsigned channel, float *voltage) {
    if (channel >= RP_MNG)  return RP_EPN;
    float range = ranges[ioread32(&regset[channel]->cfg_rng)];
    *voltage = (float) (ioread32(&regset[channel]->cfg_hst) >> RP_ACQ_TODO) * range);
    return RP_OK;
}




/**
 * Write pointer
 */
int osc_GetWritePointer(uint32_t* pos)
{
    return cmn_GetValue(&regset->wr_ptr_cur, pos, WRITE_POINTER_MASK);
}

int osc_GetWritePointerAtTrig(uint32_t* pos)
{
    return cmn_GetValue(&regset->wr_ptr_trigger, pos, WRITE_POINTER_MASK);
}

int rp_AcqGetPreTriggerCounter(uint32_t* value) {
    return acq_GetPreTriggerCounter(value);
}

int rp_AcqGetTriggerState(rp_acq_trig_state_t* state) {
    return acq_GetTriggerState(state);
}

int rp_AcqStart() {
    return acq_Start();
}

int rp_AcqStop() {
    return acq_Stop();
}

int rp_AcqReset() {
    return acq_Reset();
}

uint32_t rp_AcqGetNormalizedDataPos(uint32_t pos) {
    return acq_GetNormalizedDataPos(pos);
}

int rp_AcqGetDataRaw(rp_channel_t channel,  uint32_t pos, uint32_t* size, int16_t* buffer) {
    return acq_GetDataRaw(channel, pos, size, buffer);
}

int rp_AcqGetBufSize(uint32_t *size) {
    return acq_GetBufferSize(size);
}

float rp_CmnCnvCntToV(uint32_t field_len, uint32_t cnts, float adc_max_v) {
	return cmn_CnvCntToV(field_len, cnts, adc_max_v);
}

