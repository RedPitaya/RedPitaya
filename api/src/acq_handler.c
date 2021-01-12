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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"
#include "calib.h"
#include "oscilloscope.h"
#include "acq_handler.h"
#include "calib.h"

#ifdef Z20_250_12
#include "rp-i2c-mcp47x6-c.h"
#include "rp-i2c-max7311-c.h"
#endif


// Decimation constants
static const uint32_t DEC_1     = 1;
static const uint32_t DEC_8     = 8;
static const uint32_t DEC_64    = 64;
static const uint32_t DEC_1024  = 1024;
static const uint32_t DEC_8192  = 8192;
static const uint32_t DEC_65536 = 65536;

/* @brief Trig. reg. value offset when set to 0 */
static const int32_t TRIG_DELAY_ZERO_OFFSET = ADC_BUFFER_SIZE/2;

/* @brief Sampling period (non-decimated) - 8 [ns]. */
static const uint64_t ADC_SAMPLE_PERIOD = ADC_SAMPLE_PERIOD_DEF;


/* @brief Currently set Gain state */
static rp_pinState_t gain_ch_a = RP_LOW;
static rp_pinState_t gain_ch_b = RP_LOW;

#ifdef Z20_250_12
/* @brief Currently set AC/DC state */
static rp_acq_ac_dc_mode_t power_mode_ch_a = RP_AC_MODE;
static rp_acq_ac_dc_mode_t power_mode_ch_b = RP_AC_MODE;
#endif

/* @brief Determines whether TriggerDelay was set in time or sample units */
static bool triggerDelayInNs = false;

rp_acq_trig_src_t last_trig_src = RP_TRIG_SRC_DISABLED;

/* @brief Default filter equalization coefficients */
static const uint32_t GAIN_LO_FILT_AA = 0x7D93;
static const uint32_t GAIN_LO_FILT_BB = 0x437C7;
static const uint32_t GAIN_LO_FILT_PP = 0x2666;
static const uint32_t GAIN_LO_FILT_KK = 0xd9999a;
static const uint32_t GAIN_HI_FILT_AA = 0x4C5F;
static const uint32_t GAIN_HI_FILT_BB = 0x2F38B;
static const uint32_t GAIN_HI_FILT_PP = 0x2666;
static const uint32_t GAIN_HI_FILT_KK = 0xd9999a;

void PrintLogInFile2(const char *message){
    FILE *f = fopen("/tmp/debug.log", "a+");
  	fprintf(f, "%s\n", message);
    fclose(f);
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Converts time in [ns] to ADC samples
 *
 *
 * @param[in] time time, specified in [ns]
 * @retval int number of ADC samples
 */
static uint32_t cnvTimeToSmpls(int64_t time_ns)
{
    /* Calculate sampling period (including decimation) */

    uint32_t decimation;
    acq_GetDecimationFactor(&decimation);

    int64_t smpl_p = (ADC_SAMPLE_PERIOD * (int64_t)decimation);
    return (int32_t)round((double)time_ns / smpl_p);
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Converts ADC samples to time in [ns]
 *
 *
 * @param[in] samples, number of ADC samples
 * @retval int time, specified in [ns]
 */
static int64_t cnvSmplsToTime(int32_t samples)
{
    /* Calculate time (including decimation) */

    uint32_t decimation;
    acq_GetDecimationFactor(&decimation);

    return (int64_t)samples * ADC_SAMPLE_PERIOD * (int32_t)decimation;
}

/**
 * Sets equalization filter with default coefficients per channel
 * @param channel Channel A or B
 * @return 0 when successful
 */
static int setEqFilters(rp_channel_t channel)
{

    rp_pinState_t gain;
    acq_GetGain(channel, &gain);
#if defined Z10 || defined Z20_125
    uint32_t aa = calib_GetFilterCoff(channel,gain,AA);
    uint32_t bb = calib_GetFilterCoff(channel,gain,BB);
    uint32_t kk = calib_GetFilterCoff(channel,gain,KK);
    uint32_t pp = calib_GetFilterCoff(channel,gain,PP);
  
    // Update equalization filter with default coefficients
    if (channel == RP_CH_1) {
        return osc_SetEqFiltersChA(aa, bb, kk, pp);
    } else {
        return osc_SetEqFiltersChB(aa, bb, kk, pp);
    }
#else
    if (channel == RP_CH_1) {
        if (gain == RP_HIGH)  return osc_SetEqFiltersChA(GAIN_HI_FILT_AA, GAIN_HI_FILT_BB, GAIN_HI_FILT_KK, GAIN_HI_FILT_PP);
        else                  return osc_SetEqFiltersChA(GAIN_LO_FILT_AA, GAIN_LO_FILT_BB, GAIN_LO_FILT_KK, GAIN_LO_FILT_PP);
    } else {
        if (gain == RP_HIGH)  return osc_SetEqFiltersChB(GAIN_HI_FILT_AA, GAIN_HI_FILT_BB, GAIN_HI_FILT_KK, GAIN_HI_FILT_PP);
        else                  return osc_SetEqFiltersChB(GAIN_LO_FILT_AA, GAIN_LO_FILT_BB, GAIN_LO_FILT_KK, GAIN_LO_FILT_PP);
    }
#endif
}

/*----------------------------------------------------------------------------*/

int acq_SetArmKeep(bool enable) {
    return osc_SetArmKeep(enable);
}

int acq_GetArmKeep(bool* state){
    return osc_GetArmKeep(state);
}

int acq_GetBufferFillState(bool* state){
    return osc_GetBufferFillState(state);
}

int acq_SetGain(rp_channel_t channel, rp_pinState_t state)
{

    rp_pinState_t *gain = NULL;

    if (channel == RP_CH_1) {
        gain = &gain_ch_a;
    }
    else {
        gain = &gain_ch_b;
    }

    // Read old values which are dependent on the gain...
    rp_pinState_t old_gain;
    float ch_thr, ch_hyst;
    int status = 0;
    old_gain = *gain;
    acq_GetChannelThreshold(channel, &ch_thr);
    acq_GetChannelThresholdHyst(channel, &ch_hyst);
    
#ifdef Z20_250_12
    int ch = (channel == RP_CH_1 ? RP_MAX7311_IN1 : RP_MAX7311_IN2);
    int att = (state == RP_LOW ? RP_ATTENUATOR_1_1 : RP_ATTENUATOR_1_20);
    status = rp_setAttenuator_C(ch,att);
    
#endif
    if (status == RP_OK) {
    // Now update the gain
        *gain = state;
    }
    // And recalculate new values...
    status = acq_SetChannelThreshold(channel, ch_thr);
    if (status == RP_OK) {
        status = acq_SetChannelThresholdHyst(channel, ch_hyst);
    }

    // In case of an error, put old values back and report the error
    if (status != RP_OK) {
        *gain = old_gain;
        acq_SetChannelThreshold(channel, ch_thr);
        acq_SetChannelThresholdHyst(channel, ch_hyst);
    }
    // At the end if everything is ok, update also equalization filters based on the new gain.
    // Updating eq filters should never fail...
    else {
        status = setEqFilters(channel);
    }


    return status;
}

int acq_GetGain(rp_channel_t channel, rp_pinState_t* state)
{
    if (channel == RP_CH_1) {
        *state = gain_ch_a;
    }
    else {
        *state = gain_ch_b;
    }
    return RP_OK;
}

/**
 * Returns currently set gain in Volts
 * @param state
 * @return
 */
int acq_GetGainV(rp_channel_t channel, float* voltage)
{
    #ifdef Z20 
        *voltage = 0.5;
    #else 
        rp_pinState_t *gain = NULL;

        if (channel == RP_CH_1) {
            gain = &gain_ch_a;
        }
        else {
            gain = &gain_ch_b;
        }

        if (*gain == RP_LOW) {
            *voltage = 1.0;
        }
        else {
            *voltage = 20.0;
        }
   #endif
    return RP_OK;
}

int acq_SetDecimation(rp_acq_decimation_t decimation)
{
    int64_t time_ns = 0;

    if (triggerDelayInNs) {
        acq_GetTriggerDelayNs(&time_ns);
    }

    switch (decimation) {
    case RP_DEC_1:     osc_SetDecimation(DEC_1);     break;
    case RP_DEC_8:     osc_SetDecimation(DEC_8);     break;
    case RP_DEC_64:    osc_SetDecimation(DEC_64);    break;
    case RP_DEC_1024:  osc_SetDecimation(DEC_1024);  break;
    case RP_DEC_8192:  osc_SetDecimation(DEC_8192);  break;
    case RP_DEC_65536: osc_SetDecimation(DEC_65536); break;
    default:
        return RP_EOOR;
    }

    // Now update trigger delay based on new decimation
    if (triggerDelayInNs) {
        acq_SetTriggerDelayNs(time_ns, true);
    }

    return RP_OK;
}

int acq_GetDecimation(rp_acq_decimation_t* decimation)
{
    uint32_t decimationVal;
    osc_GetDecimation(&decimationVal);

    if      (decimationVal == DEC_1)      *decimation = RP_DEC_1;
    else if (decimationVal == DEC_8)      *decimation = RP_DEC_8;
    else if (decimationVal == DEC_64)     *decimation = RP_DEC_64;
    else if (decimationVal == DEC_1024)   *decimation = RP_DEC_1024;
    else if (decimationVal == DEC_8192)   *decimation = RP_DEC_8192;
    else if (decimationVal == DEC_65536)  *decimation = RP_DEC_65536;
    else {
        return RP_EOOR;
    }

    return RP_OK;
}

int acq_SetDecimationFactor(uint32_t decimation)
{
    int64_t time_ns = 0;

    if (triggerDelayInNs) {
        acq_GetTriggerDelayNs(&time_ns);
    }

    bool check = false;
    if (decimation == 1)  check = true;
    if (decimation == 2)  check = true;
    if (decimation == 4)  check = true;
    if (decimation == 8)  check = true;
    if (decimation >= 16 && decimation <= 65536) check = true;
    
    if (!check) return RP_EOOR;
    osc_SetDecimation(decimation); 
    // Now update trigger delay based on new decimation
    if (triggerDelayInNs) {
        acq_SetTriggerDelayNs(time_ns, true);
    }

    return RP_OK;
}

int acq_GetDecimation2(uint32_t* decimation)
{
    osc_GetDecimation(decimation);
    return RP_OK;
}

int acq_GetDecimationFactor(uint32_t* decimation)
{
    osc_GetDecimation(decimation);
    return RP_OK;
}


int acq_SetSamplingRate(rp_acq_sampling_rate_t sampling_rate)
{
    switch ((int)sampling_rate) {
    case 0:  return acq_SetDecimation(RP_DEC_1);
    case 1:  return acq_SetDecimation(RP_DEC_8);
    case 2:  return acq_SetDecimation(RP_DEC_64);
    case 3:  return acq_SetDecimation(RP_DEC_1024);
    case 4:  return acq_SetDecimation(RP_DEC_8192);
    case 5:  return acq_SetDecimation(RP_DEC_65536);
    default:
        return RP_EOOR;
    }
}

int acq_GetSamplingRate(rp_acq_sampling_rate_t* sampling_rate)
{
    rp_acq_decimation_t decimation;
    acq_GetDecimation(&decimation);

    switch (decimation) {
    case RP_DEC_1:     *sampling_rate = (rp_acq_sampling_rate_t)0;  break;
    case RP_DEC_8:     *sampling_rate = (rp_acq_sampling_rate_t)1;  break;
    case RP_DEC_64:    *sampling_rate = (rp_acq_sampling_rate_t)2;  break;
    case RP_DEC_1024:  *sampling_rate = (rp_acq_sampling_rate_t)3;  break;
    case RP_DEC_8192:  *sampling_rate = (rp_acq_sampling_rate_t)4;  break;
    case RP_DEC_65536: *sampling_rate = (rp_acq_sampling_rate_t)5;  break;
    default:
        return RP_EOOR;
    }

    return RP_OK;
}

int acq_GetSamplingRateHz(float* sampling_rate)
{
    float max_rate = ADC_SAMPLE_RATE;

    rp_acq_decimation_t decimation;
    acq_GetDecimation(&decimation);

    switch(decimation){
        case RP_DEC_1:     *sampling_rate = max_rate / 1;     break;
        case RP_DEC_8:     *sampling_rate = max_rate / 8;     break;
        case RP_DEC_64:    *sampling_rate = max_rate / 64;    break;
        case RP_DEC_1024:  *sampling_rate = max_rate / 1024;  break;
        case RP_DEC_8192:  *sampling_rate = max_rate / 8192;  break;
        case RP_DEC_65536: *sampling_rate = max_rate / 65536; break;
    }

    return RP_OK;
}

int acq_SetAveraging(bool enable)
{
    return osc_SetAveraging(enable);
}

int acq_GetAveraging(bool* enable)
{
    return osc_GetAveraging(enable);
}



int acq_SetTriggerSrc(rp_acq_trig_src_t source)
{
    last_trig_src = source;
    return osc_SetTriggerSource(source);
}

int acq_GetTriggerSrc(rp_acq_trig_src_t* source)
{
    return osc_GetTriggerSource(source);
}

int acq_GetTriggerState(rp_acq_trig_state_t* state)
{
    bool stateB;
    osc_GetTriggerState(&stateB);

    if (stateB) {
        *state=RP_TRIG_STATE_TRIGGERED;
    }
    else{
        *state=RP_TRIG_STATE_WAITING;
    }

    return RP_OK;
}

int acq_SetTriggerDelay(int32_t decimated_data_num, bool updateMaxValue)
{
    int32_t trig_dly;
    if(decimated_data_num < -TRIG_DELAY_ZERO_OFFSET){
            trig_dly=0;
    }
    else{
        trig_dly = decimated_data_num + TRIG_DELAY_ZERO_OFFSET;
    }
    osc_SetTriggerDelay(trig_dly);
    
    triggerDelayInNs = false;
    return RP_OK;
}

int acq_SetTriggerDelayNs(int64_t time_ns, bool updateMaxValue)
{
    int32_t samples = cnvTimeToSmpls(time_ns);
    acq_SetTriggerDelay(samples, updateMaxValue);
    triggerDelayInNs = true;
    return RP_OK;
}

int acq_GetTriggerDelay(int32_t* decimated_data_num)
{
    uint32_t trig_dly;
    int r=osc_GetTriggerDelay(&trig_dly);
    *decimated_data_num=(int32_t)trig_dly-TRIG_DELAY_ZERO_OFFSET;
    return r;
}

int acq_GetTriggerDelayNs(int64_t* time_ns)
{
    int32_t samples;
    acq_GetTriggerDelay(&samples);
    *time_ns=cnvSmplsToTime(samples);
    return RP_OK;
}

int acq_GetPreTriggerCounter(uint32_t* value) {
    return osc_GetPreTriggerCounter(value);
}

int acq_GetWritePointer(uint32_t* pos)
{
    return osc_GetWritePointer(pos);
}

int acq_GetWritePointerAtTrig(uint32_t* pos)
{
    int ret = osc_GetWritePointerAtTrig(pos);
    // Fix FPGA bug with wrong trigger position at DEC 1
    rp_acq_decimation_t decimationVal;
    acq_GetDecimation(&decimationVal);
    if (decimationVal == RP_DEC_1){
        *pos = (*pos - 2) % ADC_BUFFER_SIZE;
    }
    
    return ret;
}

int acq_SetTriggerLevel(rp_channel_trigger_t channel, float voltage)
{
    switch(channel){
        case RP_T_CH_1: return acq_SetChannelThreshold(RP_CH_1, voltage);
        case RP_T_CH_2: return acq_SetChannelThreshold(RP_CH_2, voltage);
        #ifdef Z20_250_12
        case RP_T_CH_EXT: {
                int ret = rp_setExtTriggerLevel(voltage);
                switch(ret){
                    case RP_I2C_EOOR: return RP_EOOR;
                    case RP_I2C_EFRB: return RP_EFRB;
                    case RP_I2C_EFWB: return RP_EFWB;
                    default:
                        return RP_OK;
                }   
        }
        #endif
        default:;
    }
    return RP_NOTS;
}

int acq_GetTriggerLevel(rp_channel_trigger_t channel,float *voltage)
{
    switch(channel){
        case RP_T_CH_1: return acq_GetChannelThreshold(RP_CH_1, voltage);
        case RP_T_CH_2: return acq_GetChannelThreshold(RP_CH_2, voltage);
        #ifdef Z20_250_12
        case RP_T_CH_EXT: {
                int ret = rp_getExtTriggerLevel(voltage);
                switch(ret){
                    case RP_I2C_EOOR: return RP_EOOR;
                    case RP_I2C_EFRB: return RP_EFRB;
                    case RP_I2C_EFWB: return RP_EFWB;
                    default:
                        return RP_OK;
                }   
        }
        #endif
        default:;
    }
    return RP_NOTS;
}

int acq_SetChannelThreshold(rp_channel_t channel, float voltage)
{
    float gainV;
    rp_pinState_t gain;

    acq_GetGainV(channel, &gainV);
    acq_GetGain(channel, &gain);


    if (fabs(voltage) - fabs(gainV) > FLOAT_EPS) {
        return RP_EOOR;
    }


#ifdef Z20_250_12
    rp_acq_ac_dc_mode_t power_mode;
    acq_GetAC_DC(channel,&power_mode);
    int32_t dc_offs = calib_getOffset(channel, gain,power_mode);
    uint32_t calibScale = calib_GetFrontEndScale(channel, gain,power_mode);
#else
    int32_t dc_offs = calib_getOffset(channel, gain);
    uint32_t calibScale = calib_GetFrontEndScale(channel, gain);
#endif

    uint32_t cnt = cmn_CnvVToCnt(ADC_REG_BITS, voltage, gainV, gain == RP_HIGH ? false : true, calibScale, dc_offs, 0.0);

#ifndef Z20
    // We cut high bits of negative numbers
    cnt = cnt & ((1 << ADC_REG_BITS) - 1);
#endif
    if (channel == RP_CH_1) {
        return osc_SetThresholdChA(cnt);
    }
    else {
        return osc_SetThresholdChB(cnt);
    }
}

int acq_GetChannelThreshold(rp_channel_t channel, float* voltage)
{
    float gainV;
    rp_pinState_t gain;
    uint32_t cnts;

    if (channel == RP_CH_1) {
        osc_GetThresholdChA(&cnts);
    }
    else {
        osc_GetThresholdChB(&cnts);
    }

    acq_GetGainV(channel, &gainV);
    acq_GetGain(channel, &gain);

#ifdef Z20_250_12
    rp_acq_ac_dc_mode_t power_mode;
    acq_GetAC_DC(channel,&power_mode);
    int32_t dc_offs = calib_getOffset(channel, gain,power_mode);
    uint32_t calibScale = calib_GetFrontEndScale(channel, gain,power_mode);
#else
    int32_t dc_offs = calib_getOffset(channel, gain);
    uint32_t calibScale = calib_GetFrontEndScale(channel, gain);
#endif

    *voltage = cmn_CnvCntToV(ADC_REG_BITS, cnts & ADC_REG_BITS_MASK, gainV, calibScale, dc_offs, 0.0);

    return RP_OK;
}

int acq_SetTriggerHyst(float voltage)
{
    acq_SetChannelThresholdHyst(RP_CH_1, voltage);
    acq_SetChannelThresholdHyst(RP_CH_2, voltage);
    return RP_OK;
}

int acq_GetTriggerHyst(float *voltage)
{
    acq_GetChannelThresholdHyst(RP_CH_1, voltage);
    return RP_OK;
}


int acq_SetChannelThresholdHyst(rp_channel_t channel, float voltage)
{
    float gainV;
    rp_pinState_t gain;

    acq_GetGainV(channel, &gainV);
    acq_GetGain(channel, &gain);
    
  
    if (fabs(voltage) - fabs(gainV) > FLOAT_EPS) {
        return RP_EOOR;
    }

#ifdef Z20_250_12
    rp_acq_ac_dc_mode_t power_mode;
    acq_GetAC_DC(channel,&power_mode);
    int32_t dc_offs = calib_getOffset(channel, gain,power_mode);
    uint32_t calibScale = calib_GetFrontEndScale(channel, gain,power_mode);
#else
    int32_t dc_offs = calib_getOffset(channel, gain);
    uint32_t calibScale = calib_GetFrontEndScale(channel, gain);
#endif

    uint32_t cnt = cmn_CnvVToCnt(ADC_REG_BITS, voltage, gainV, gain == RP_HIGH ? false : true, calibScale, dc_offs, 0.0);
    if (channel == RP_CH_1) {
        return osc_SetHysteresisChA(cnt);
    }
    else {
        return osc_SetHysteresisChB(cnt);
    }
}

int acq_GetChannelThresholdHyst(rp_channel_t channel, float* voltage)
{
    float gainV;
    rp_pinState_t gain;
    uint32_t cnts;

    if (channel == RP_CH_1) {
        osc_GetHysteresisChA(&cnts);
    }
    else {
        osc_GetHysteresisChB(&cnts);
    }

    acq_GetGainV(channel, &gainV);
    acq_GetGain(channel, &gain);

#ifdef Z20_250_12
    rp_acq_ac_dc_mode_t power_mode;
    acq_GetAC_DC(channel,&power_mode);
    int32_t dc_offs = calib_getOffset(channel, gain,power_mode);
    uint32_t calibScale = calib_GetFrontEndScale(channel, gain,power_mode);
#else
    int32_t dc_offs = calib_getOffset(channel, gain);
    uint32_t calibScale = calib_GetFrontEndScale(channel, gain);
#endif

    *voltage = cmn_CnvCntToV(ADC_REG_BITS, cnts & ADC_REG_BITS_MASK, gainV, calibScale, dc_offs, 0.0);

    return RP_OK;
}

int acq_Start()
{
    osc_WriteDataIntoMemory(true);
    return RP_OK;
}

int acq_Stop()
{
    return osc_WriteDataIntoMemory(false);
}

int acq_Reset()
{
    acq_SetDefault();
    return osc_ResetWriteStateMachine();
}

static const volatile uint32_t* getRawBuffer(rp_channel_t channel)
{
    if (channel == RP_CH_1) {
        return osc_GetDataBufferChA();
    }
    else {
        return osc_GetDataBufferChB();
    }
}

static uint32_t getSizeFromStartEndPos(uint32_t start_pos, uint32_t end_pos)
{

    end_pos = acq_GetNormalizedDataPos(end_pos);
    start_pos = acq_GetNormalizedDataPos(start_pos);

    if (end_pos < start_pos) {
        end_pos += ADC_BUFFER_SIZE;
    }

    return end_pos - start_pos + 1;
}

uint32_t acq_GetNormalizedDataPos(uint32_t pos)
{
    return (pos % ADC_BUFFER_SIZE);
}

int acq_GetDataRaw(rp_channel_t channel, uint32_t pos, uint32_t* size, int16_t* buffer)
{

    *size = MIN(*size, ADC_BUFFER_SIZE);

    uint32_t cnts;

    const volatile uint32_t* raw_buffer = getRawBuffer(channel);

    rp_pinState_t gain;
    acq_GetGain(channel, &gain);
    
#ifdef Z20_250_12
    rp_acq_ac_dc_mode_t power_mode;
    acq_GetAC_DC(channel,&power_mode);
    int32_t dc_offs = calib_getOffset(channel, gain,power_mode);
#else
    int32_t dc_offs = calib_getOffset(channel, gain);
#endif

    for (uint32_t i = 0; i < (*size); ++i) {
        cnts = (raw_buffer[(pos + i) % ADC_BUFFER_SIZE]) & ADC_BITS_MASK;

        buffer[i] = cmn_CalibCnts(ADC_BITS, cnts, dc_offs);
    }

    return RP_OK;
}


int acq_GetDataRawV2(uint32_t pos, uint32_t* size, uint16_t* buffer, uint16_t* buffer2)
{

    *size = MIN(*size, ADC_BUFFER_SIZE);
    const volatile uint32_t* raw_buffer = getRawBuffer(RP_CH_1);
    const volatile uint32_t* raw_buffer2 = getRawBuffer(RP_CH_2);
    
    for (uint32_t i = 0; i < (*size); ++i) {
        buffer[i] =  (raw_buffer[(pos + i) % ADC_BUFFER_SIZE]) & ADC_BITS_MASK;
        buffer2[i] = (raw_buffer2[(pos + i) % ADC_BUFFER_SIZE]) & ADC_BITS_MASK;
    }

    return RP_OK;
}

int acq_GetDataPosRaw(rp_channel_t channel, uint32_t start_pos, uint32_t end_pos, int16_t* buffer, uint32_t *buffer_size)
{
    uint32_t size = getSizeFromStartEndPos(start_pos, end_pos);

    if (size > *buffer_size) {
        return RP_BTS;
    }

    *buffer_size = size;
    return acq_GetDataRaw(channel, start_pos, buffer_size, buffer);
}

/**
 * Use only when write pointer has stopped...
 */
int acq_GetOldestDataRaw(rp_channel_t channel, uint32_t* size, int16_t* buffer)
{
    uint32_t pos;

    acq_GetWritePointer(&pos);
    pos++;

    return acq_GetDataRaw(channel, pos, size, buffer);
}

int acq_GetLatestDataRaw(rp_channel_t channel, uint32_t* size, int16_t* buffer)
{
    *size = MIN(*size, ADC_BUFFER_SIZE);

    uint32_t pos;
    acq_GetWritePointer(&pos);

    pos++;

    if ((*size) > pos) {
        pos += ADC_BUFFER_SIZE;
    }
    pos -= (*size);

    return acq_GetDataRaw(channel, pos, size, buffer);
}

int acq_GetDataV(rp_channel_t channel,  uint32_t pos, uint32_t* size, float* buffer)
{
    *size = MIN(*size, ADC_BUFFER_SIZE);

    float gainV;
    rp_pinState_t gain;
    acq_GetGainV(channel, &gainV);
    acq_GetGain(channel, &gain);

#ifdef Z20_250_12
    rp_acq_ac_dc_mode_t power_mode;
    acq_GetAC_DC(channel,&power_mode);
    int32_t dc_offs = calib_getOffset(channel, gain,power_mode);
    uint32_t calibScale = calib_GetFrontEndScale(channel, gain,power_mode);
#else
    int32_t dc_offs = calib_getOffset(channel, gain);
    uint32_t calibScale = calib_GetFrontEndScale(channel, gain);
#endif

    const volatile uint32_t* raw_buffer = getRawBuffer(channel);

    uint32_t cnts;
    for (uint32_t i = 0; i < (*size); ++i) {
        cnts = raw_buffer[(pos + i) % ADC_BUFFER_SIZE];
        buffer[i] = cmn_CnvCntToV(ADC_BITS, cnts & ADC_BITS_MASK, gainV, calibScale, dc_offs, 0.0);
    }

    return RP_OK;
}

int acq_GetDataV2(uint32_t pos, uint32_t* size, float* buffer1, float* buffer2)
{
    *size = MIN(*size, ADC_BUFFER_SIZE);

    float gainV1, gainV2;
    rp_pinState_t gain1, gain2;
    acq_GetGainV(RP_CH_1, &gainV1);
    acq_GetGain(RP_CH_1, &gain1);
    acq_GetGainV(RP_CH_2, &gainV2);
    acq_GetGain(RP_CH_2, &gain2);

#ifdef Z20_250_12
    rp_acq_ac_dc_mode_t power_mode1;
    acq_GetAC_DC(RP_CH_1,&power_mode1);
    int32_t dc_offs1 = calib_getOffset(RP_CH_1, gain1,power_mode1);
    uint32_t calibScale1 = calib_GetFrontEndScale(RP_CH_1, gain1,power_mode1);
#else
    int32_t dc_offs1 = calib_getOffset(RP_CH_1, gain1);
    uint32_t calibScale1 = calib_GetFrontEndScale(RP_CH_1, gain1);
#endif

#ifdef Z20_250_12
    rp_acq_ac_dc_mode_t power_mode2;
    acq_GetAC_DC(RP_CH_2,&power_mode2);
    int32_t dc_offs2 = calib_getOffset(RP_CH_2, gain2,power_mode2);
    uint32_t calibScale2 = calib_GetFrontEndScale(RP_CH_2, gain2,power_mode2);
#else
    int32_t dc_offs2 = calib_getOffset(RP_CH_2, gain2);
    uint32_t calibScale2 = calib_GetFrontEndScale(RP_CH_2, gain2);
#endif

    const volatile uint32_t* raw_buffer1 = getRawBuffer(RP_CH_1);
    const volatile uint32_t* raw_buffer2 = getRawBuffer(RP_CH_2);

    uint32_t cnts1[*size];
    uint32_t cnts2[*size];
    uint32_t* ptr1 = cnts1;
    uint32_t* ptr2 = cnts2;

    for (uint32_t i = 0; i < (*size); ++i) {
        *ptr1++ = raw_buffer1[pos];
        *ptr2++ = raw_buffer2[pos];
        pos = (pos + 1) % ADC_BUFFER_SIZE;
    }

    ptr1 = cnts1;
    ptr2 = cnts2;

    // FILE *pFile = fopen ("/tmp/debugout.txt","a+");  

    for (uint32_t i = 0; i < (*size); ++i) {
        *buffer1++ = cmn_CnvCntToV(ADC_BITS, *ptr1++ & ADC_BITS_MASK, gainV1, calibScale1, dc_offs1, 0.0);
        *buffer2++ = cmn_CnvCntToV(ADC_BITS, *ptr2++ & ADC_BITS_MASK, gainV2, calibScale2, dc_offs2, 0.0);
//        fprintf (pFile, "%d - %f - %d - %d\n",*(ptr1-1),*(buffer1-1),calibScale1,dc_offs1);
    }
//    fclose (pFile);
    

    return RP_OK;
}

int acq_GetDataPosV(rp_channel_t channel,  uint32_t start_pos, uint32_t end_pos, float* buffer, uint32_t *buffer_size)
{
    uint32_t size = getSizeFromStartEndPos(start_pos, end_pos);
    if (size > *buffer_size) {
        return RP_BTS;
    }
    *buffer_size = size;
    return acq_GetDataV(channel, start_pos, buffer_size, buffer);
}

/**
 * Use only when write pointer has stopped...
 */
int acq_GetOldestDataV(rp_channel_t channel, uint32_t* size, float* buffer)
{
    uint32_t pos;

    acq_GetWritePointer(&pos);
    pos++;

    return acq_GetDataV(channel, pos, size, buffer);
}

int acq_GetLatestDataV(rp_channel_t channel, uint32_t* size, float* buffer)
{
    *size = MIN(*size, ADC_BUFFER_SIZE);

    uint32_t pos;
    acq_GetWritePointer(&pos);

    pos = (pos - (*size)) % ADC_BUFFER_SIZE;

    return acq_GetDataV(channel, pos, size, buffer);
}


int acq_GetBufferSize(uint32_t *size) {
    *size = ADC_BUFFER_SIZE;
    return RP_OK;
}

/**
 * Sets default configuration
 * @return
 */
int acq_SetDefault() {
    acq_SetChannelThreshold(RP_CH_1, 0.0);
    acq_SetChannelThreshold(RP_CH_2, 0.0);
    acq_SetChannelThresholdHyst(RP_CH_1, 0.0);
    acq_SetChannelThresholdHyst(RP_CH_2, 0.0);

    acq_SetGain(RP_CH_1, RP_LOW);
    acq_SetGain(RP_CH_2, RP_LOW);
    acq_SetDecimation(RP_DEC_1);
    acq_SetAveraging(true);
    acq_SetTriggerSrc(RP_TRIG_SRC_DISABLED);
    acq_SetTriggerDelay(0, false);
    acq_SetTriggerDelayNs(0, false);
#ifdef Z20_250_12
    acq_SetAC_DC(RP_CH_1,RP_AC);
    acq_SetAC_DC(RP_CH_2,RP_AC);
#endif
    return RP_OK;
}


#ifdef Z20_250_12

int acq_SetAC_DC(rp_channel_t channel,rp_acq_ac_dc_mode_t mode){
    rp_acq_ac_dc_mode_t *power_mode = NULL;

    if (channel == RP_CH_1) {
        power_mode = &power_mode_ch_a;
    }
    else {
        power_mode = &power_mode_ch_b;
    }

    int ch = (channel == RP_CH_1 ? RP_MAX7311_IN1 : RP_MAX7311_IN2);
    int status = rp_setAC_DC_C(ch,mode  == RP_DC ? RP_DC_MODE : RP_AC_MODE);
    if (status == RP_OK){
        *power_mode = mode;
    }
    return RP_OK;
}

int acq_GetAC_DC(rp_channel_t channel,rp_acq_ac_dc_mode_t *status){
    if (channel == RP_CH_1) {
        *status = power_mode_ch_a;
    }
    else {
        *status = power_mode_ch_b;
    }
    return RP_OK;
}


#endif

#if defined Z10 || defined Z20_125
int acq_UpdateAcqFilter(rp_channel_t channel){
    return setEqFilters(channel);
}

int acq_GetFilterCalibValue(rp_channel_t channel,uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp){
    if (channel == RP_CH_1){
        return osc_GetEqFiltersChA(coef_aa,coef_bb,coef_kk,coef_pp);
    }
    if (channel == RP_CH_2){
        return osc_GetEqFiltersChB(coef_aa,coef_bb,coef_kk,coef_pp);
    }
    return RP_EOOR;
}
#endif
