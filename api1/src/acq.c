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

#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "common.h"
#include "acq.h"

// The FPGA register structure for oscilloscope
static volatile osc_control_t *osc_reg = NULL;

// The FPGA input signal buffer pointer for channel A/B
static volatile int32_t *osc_ch[2] = {NULL, NULL};

/**
 * general
 */

static int osc_Init() {
    cmn_Map(OSC_BASE_SIZE, OSC_BASE_ADDR, (void**)&osc_reg);
    osc_ch[0] = (int32_t*)((char*)osc_reg + OSC_CHA_OFFSET);
    osc_ch[1] = (int32_t*)((char*)osc_reg + OSC_CHB_OFFSET);
    return RP_OK;
}

static int osc_Release() {
    cmn_Unmap(OSC_BASE_SIZE, (void**)&osc_reg);
    osc_ch[0] = NULL;
    osc_ch[1] = NULL;
    return RP_OK;
}

/**
 * $Id: $
 *
 * @brief Red Pitaya library Acquire signal handler implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

/* @brief Trig. reg. value offset when set to 0 */
static const int32_t TRIG_DELAY_ZERO_OFFSET = ADC_BUFFER_SIZE/2;

/* @brief Sampling period (non-decimated) - 8 [ns]. */
static const uint64_t ADC_SAMPLE_PERIOD = 8;

/* @brief Currently set Gain state */
static int unsigned gain_ch [2] = {0, 0};

/* @brief Default filter equalization coefficients LO/HI */
static const uint32_t FILT_AA[] = {0x7D93  , 0x4C5F  };
static const uint32_t FILT_BB[] = {0x437C7 , 0x2F38B };
static const uint32_t FILT_PP[] = {0x2666  , 0x2666  };
static const uint32_t FILT_KK[] = {0xd9999a, 0xd9999a};

/**
 * Equalization filters
 */
static void osc_SetEqFiltersChA(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp) {
    osc_reg->cha_filt_aa = coef_aa;
    osc_reg->cha_filt_bb = coef_bb;
    osc_reg->cha_filt_kk = coef_kk;
    osc_reg->cha_filt_pp = coef_pp;
}

static void osc_SetEqFiltersChB(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp) {
    osc_reg->chb_filt_aa = coef_aa;
    osc_reg->chb_filt_bb = coef_bb;
    osc_reg->chb_filt_kk = coef_kk;
    osc_reg->chb_filt_pp = coef_pp;
}

//static void osc_GetEqFiltersChA(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp) {
//    coef_aa = osc_reg->cha_filt_aa;
//    coef_bb = osc_reg->cha_filt_bb;
//    coef_kk = osc_reg->cha_filt_kk;
//    coef_pp = osc_reg->cha_filt_pp;
//}
//
//static void osc_GetEqFiltersChB(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp) {
//    coef_aa = osc_reg->chb_filt_aa;
//    coef_bb = osc_reg->chb_filt_bb;
//    coef_kk = osc_reg->chb_filt_kk;
//    coef_pp = osc_reg->chb_filt_pp;
//}

/**
 * Sets equalization filter with default coefficients per channel
 * @param channel Channel A or B
 * @return 0 when successful
 */
static int setEqFilters(int unsigned channel) {
    int unsigned gain = gain_ch [channel];
    // Update equalization filter with default coefficients
    if (channel == 0)  osc_SetEqFiltersChA(FILT_AA[gain], FILT_BB[gain], FILT_KK[gain], FILT_PP[gain]);
    else               osc_SetEqFiltersChB(FILT_AA[gain], FILT_BB[gain], FILT_KK[gain], FILT_PP[gain]);
    return RP_OK;
}

/*----------------------------------------------------------------------------*/

static int acq_SetChannelThresholdHyst(int unsigned channel, float voltage) {
    int unsigned gain = gain_ch [channel];
    if (fabs(voltage) - fabs(GAIN_V(gain)) > FLOAT_EPS)
        return RP_EOOR;
    int32_t calib_off =                calib_GetAcqOffset(channel, gain);
    float   calib_scl = ADC_BITS_MAX / calib_GetAcqScale (channel, gain);
    osc_reg->hystersis[channel] = calib_Saturate(ADC_BITS, (int32_t) (voltage * calib_scl) + calib_off);
    return RP_OK;
}

static uint32_t getSizeFromStartEndPos(uint32_t start_pos, uint32_t end_pos) {
    end_pos   = end_pos   % ADC_BUFFER_SIZE;
    start_pos = start_pos % ADC_BUFFER_SIZE;
    if (end_pos < start_pos)
        end_pos += ADC_BUFFER_SIZE;
    return end_pos - start_pos + 1;
}

/**
 * Acquire methods
 */

int rp_AcqSetArmKeep(bool enable) {
    if (enable)  return cmn_SetBits  (&osc_reg->conf, 0x8, ARM_KEEP_MASK);
    else         return cmn_UnsetBits(&osc_reg->conf, 0x8, ARM_KEEP_MASK);
}


int rp_AcqSetDecimationFactor(uint32_t decimation) {
    osc_reg->data_dec = decimation & DATA_DEC_MASK;
    return RP_OK;
}

int rp_AcqGetDecimationFactor(uint32_t* decimation) {
    *decimation = osc_reg->data_dec;
    return RP_OK;
}

int rp_AcqSetAveraging(bool enabled) {
    osc_reg->other = enabled ? 1 : 0;
    return RP_OK;
}

int rp_AcqGetAveraging(bool *enabled) {
    *enabled = osc_reg->other;
    return RP_OK;
}

int rp_AcqSetTriggerSrc(rp_acq_trig_src_t source) {
    osc_reg->trig_source = source & TRIG_SRC_MASK;
    return RP_OK;
}

int rp_AcqGetTriggerSrc(rp_acq_trig_src_t* source) {
    *source = osc_reg->trig_source & TRIG_SRC_MASK;
    return RP_OK;
}

int rp_AcqGetTriggerState(rp_acq_trig_state_t* state) {
    uint32_t stateB = osc_reg->conf & TRIG_ST_MCH_MASK;
    if (stateB)  *state = RP_TRIG_STATE_TRIGGERED;
    else         *state = RP_TRIG_STATE_WAITING;
    return RP_OK;
}

int rp_AcqSetPostTriggerDelay(uint32_t delay) {
    osc_reg->dly_pst = delay;
    return RP_OK;
}

int rp_AcqGetPostTriggerDelay(uint32_t* delay) {
    *delay = osc_reg->dly_pst;
    return RP_OK;
}

int rp_AcqGetPreTriggerCounter(uint32_t* value) {
    *value = osc_reg->pre_trigger_counter;
    return RP_OK;
}

int rp_AcqGetGain(int unsigned channel, int unsigned* state) {
    return gain_ch [channel];
}

int rp_AcqGetGainV(int unsigned channel, float* voltage) {
    return GAIN_V(gain_ch[channel]);
}

static int acq_GetChannelThresholdHyst(int unsigned channel, float* voltage) {
    int unsigned gain = gain_ch [channel];
    int32_t calib_off = -calib_GetAcqOffset(channel, gain);
    float   calib_scl =  calib_GetAcqScale (channel, gain) / ADC_BITS_MAX;
    *voltage = (float) (osc_reg->hystersis[channel] + calib_off) * calib_scl;
    return RP_OK;
}

static int acq_GetChannelThreshold(int unsigned channel, float* voltage) {
    int unsigned gain = gain_ch [channel];
    int32_t calib_off = -calib_GetAcqOffset(channel, gain);
    float   calib_scl =  calib_GetAcqScale (channel, gain) / ADC_BITS_MAX;
    *voltage = (float) (osc_reg->thr[channel] + calib_off) * calib_scl;
    fprintf(stderr, "%s-1: scl = %f, off = %d, voltage = %f, cnt = %d\n", __func__, calib_scl, calib_off, *voltage, osc_reg->thr[channel]);
    return RP_OK;
}

int rp_AcqSetGain(int unsigned channel, int unsigned state) {
    int unsigned gain = gain_ch [channel];
    // Read old values which are dependent on the gain...
    int unsigned old_gain;
    float ch_thr, ch_hyst;
    old_gain = gain;
    acq_GetChannelThreshold    (channel, &ch_thr );
    acq_GetChannelThresholdHyst(channel, &ch_hyst);
    // Now update the gain
    gain = state;
    // And recalculate new values...
    int status = rp_AcqSetTriggerLevel(channel, ch_thr);
    if (status == RP_OK)
        status = acq_SetChannelThresholdHyst(channel, ch_hyst);
    // In case of an error, put old values back and report the error
    if (status != RP_OK) {
        gain = old_gain;
        rp_AcqSetTriggerLevel(channel, ch_thr);
        acq_SetChannelThresholdHyst(channel, ch_hyst);
    } else {
    // At the end if everything is ok, update also equalization filters based on the new gain.
    // Updating eq filters should never fail...
        status = setEqFilters(channel);
    }
    return status;
}

int rp_AcqGetTriggerLevel(float* voltage) {
    acq_GetChannelThreshold(0, voltage);
    return RP_OK;
}

int rp_AcqSetTriggerLevel(int unsigned channel, float voltage) {
    int unsigned gain = gain_ch [channel];
    if (fabs(voltage) - fabs(GAIN_V(gain)) > FLOAT_EPS)
        return RP_EOOR;
    int32_t calib_off =                calib_GetAcqOffset(channel, gain);
    float   calib_scl = ADC_BITS_MAX / calib_GetAcqScale (channel, gain);
    int32_t cnt = calib_Saturate(ADC_BITS, (int32_t)(voltage * calib_scl) + calib_off);
    fprintf(stderr, "%s-1: off = %d, scl = %f, cnt = %d, voltage = %f\n", __func__, calib_off, calib_scl, cnt, voltage);
    osc_reg->thr[channel] = cnt;
    return RP_OK;
}

int rp_AcqGetTriggerHyst(float* voltage) {
    return acq_GetChannelThresholdHyst(0, voltage);
}

int rp_AcqSetTriggerHyst(float voltage) {
    acq_SetChannelThresholdHyst(0, voltage);
    acq_SetChannelThresholdHyst(1, voltage);
    return RP_OK;
}

int rp_AcqGetWritePointer(uint32_t* pos) {
    *pos = osc_reg->wr_ptr_cur;
    return RP_OK;
}

int rp_AcqGetWritePointerAtTrig(uint32_t* pos) {
    *pos = osc_reg->wr_ptr_trigger;
    return RP_OK;
}

int rp_AcqStart() {
    return cmn_SetBits(&osc_reg->conf, 0x1, START_DATA_WRITE_MASK);
}

int rp_AcqStop() {
    return cmn_UnsetBits(&osc_reg->conf, 0x1, START_DATA_WRITE_MASK);
}

int rp_AcqReset() {
    for (int unsigned ch=0; ch<2; ch++) {
        rp_AcqSetTriggerLevel(ch, 0.0);
        rp_AcqSetTriggerHyst (0.0);
        rp_AcqSetGain(ch, 0);
    }
    rp_AcqSetDecimationFactor(1);
    rp_AcqSetAveraging(true);
    rp_AcqSetTriggerSrc(RP_TRIG_SRC_DISABLED);
    rp_AcqSetPostTriggerDelay(ADC_BUFFER_SIZE/2);
    return cmn_SetBits(&osc_reg->conf, (0x1 << 1), RST_WR_ST_MCH_MASK);
}

int rp_AcqGetDataPosRaw(int unsigned channel, uint32_t start_pos, uint32_t end_pos, int16_t* buffer, uint32_t* buffer_size) {
    uint32_t size = getSizeFromStartEndPos(start_pos, end_pos);
    if (size > *buffer_size)
        return RP_BTS;
    *buffer_size = size;
    return rp_AcqGetDataRaw(channel, start_pos, buffer_size, buffer);
}

int rp_AcqGetDataPosV(int unsigned channel, uint32_t start_pos, uint32_t end_pos, float* buffer, uint32_t* buffer_size) {
    uint32_t size = getSizeFromStartEndPos(start_pos, end_pos);
    if (size > *buffer_size)
        return RP_BTS;
    *buffer_size = size;
    return rp_AcqGetDataV(channel, start_pos, buffer_size, buffer);
}

int rp_AcqGetDataRaw(int unsigned channel,  uint32_t pos, uint32_t* size, int16_t* buffer) {
    *size = MIN(*size, ADC_BUFFER_SIZE);
    for (uint32_t i = 0; i < (*size); ++i)
        buffer[i] = osc_ch[channel][(pos + i) % ADC_BUFFER_SIZE];
    return RP_OK;
}

int rp_AcqGetDataRawV2(uint32_t pos, uint32_t* size, int16_t* buffer[2]) {
    for (int unsigned ch=0; ch<2; ch++)
        rp_AcqGetDataRaw(ch, pos, size, buffer[ch]);
    return RP_OK;
}

int rp_AcqGetOldestDataRaw(int unsigned channel, uint32_t* size, int16_t* buffer) {
    uint32_t pos;
    rp_AcqGetWritePointer(&pos);
    return rp_AcqGetDataRaw(channel, pos+1, size, buffer);
}

int rp_AcqGetLatestDataRaw(int unsigned channel, uint32_t* size, int16_t* buffer) {
    *size = MIN(*size, ADC_BUFFER_SIZE);
    uint32_t pos;
    rp_AcqGetWritePointer(&pos);
    pos = (ADC_BUFFER_SIZE + pos + 1 - (*size)) % ADC_BUFFER_SIZE;
    return rp_AcqGetDataRaw(channel, pos, size, buffer);
}

int rp_AcqGetDataV(int unsigned channel, uint32_t pos, uint32_t* size, float* buffer) {
    *size = MIN(*size, ADC_BUFFER_SIZE);
    int unsigned gain = gain_ch [channel];
    int32_t calib_off = -calib_GetAcqOffset(channel, gain);
    float   calib_scl =  calib_GetAcqScale (channel, gain) / ADC_BITS_MAX;
    for (uint32_t i = 0; i < (*size); ++i)
        buffer[i] = (float) (osc_ch[channel][(pos + i) % ADC_BUFFER_SIZE] + calib_off) * calib_scl;
    return RP_OK;
}

int rp_AcqGetDataV2(uint32_t pos, uint32_t* size, float* buffer[2]) {
    for (int unsigned ch=0; ch<2; ch++)
        rp_AcqGetDataV(ch, pos, size, buffer[ch]);
    return RP_OK;
}

int rp_AcqGetOldestDataV(int unsigned channel, uint32_t* size, float* buffer) {
    uint32_t pos;
    rp_AcqGetWritePointer(&pos);
    return rp_AcqGetDataV(channel, pos+1, size, buffer);
}

int rp_AcqGetLatestDataV(int unsigned channel, uint32_t* size, float* buffer) {
    *size = MIN(*size, ADC_BUFFER_SIZE);
    uint32_t pos;
    rp_AcqGetWritePointer(&pos);
    pos = (ADC_BUFFER_SIZE + pos + 1 - (*size)) % ADC_BUFFER_SIZE;
    return rp_AcqGetDataV(channel, pos, size, buffer);
}

int rp_AcqGetBufSize(uint32_t *size) {
    *size = ADC_BUFFER_SIZE;
    return RP_OK;
}

