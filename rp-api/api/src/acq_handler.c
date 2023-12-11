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
#include <string.h>
#include <assert.h>

#include "common.h"
#include "oscilloscope.h"
#include "acq_handler.h"
#include "neon_asm.h"

#include "rp-i2c-mcp47x6-c.h"
#include "rp-i2c-max7311-c.h"

#define CHECK_CHANNEL(X) \
    uint8_t channels_rp_HPGetFastADCChannelsCount = 0; \
    if (rp_HPGetFastADCChannelsCount(&channels_rp_HPGetFastADCChannelsCount) != RP_HP_OK){ \
        fprintf(stderr,"[Error:%s] Can't get fast ADC channels count\n",X); \
        return RP_NOTS; \
    } \
    if (channel >= channels_rp_HPGetFastADCChannelsCount){ \
        fprintf(stderr,"[Error:%s] Channel is larger than allowed\n",X); \
        return RP_NOTS; \
    }

uint32_t axi_ch_buffer_size_in_samples[4] = {0,0,0,0};

/* @brief Trig. reg. value offset when set to 0 */
static const int32_t TRIG_DELAY_ZERO_OFFSET = ADC_BUFFER_SIZE / 2;


/* @brief Currently set Gain state */
static rp_pinState_t gain_ch[4] = {RP_LOW,RP_LOW,RP_LOW,RP_LOW};


/* @brief Currently set AC/DC state */
static rp_acq_ac_dc_mode_t power_mode_ch[4] = {RP_DC,RP_DC,RP_DC,RP_DC};

/* @brief Determines whether TriggerDelay was set in time or sample units */
static bool triggerDelayInNs = false;

rp_acq_trig_src_t last_trig_src = RP_TRIG_SRC_DISABLED;

float ch_hyst[4] = {0.005,0.005,0.005,0.005};
float ch_trash[4] = {0.005,0.005,0.005,0.005};
float ext_trig_trash = 0;


/*----------------------------------------------------------------------------*/
/**
 * @brief Converts time in [ns] to ADC samples
 *
 *
 * @param[in] time time, specified in [ns]
 * @retval int number of ADC samples
 */
static uint32_t cnvTimeToSmpls(int64_t time_ns){
    /* Calculate sampling period (including decimation) */

    uint32_t decimation;
    acq_GetDecimationFactor(&decimation);
    double sp = 0;

    if (acq_GetADCSamplePeriod(&sp) != RP_OK){
        return 0;
    }

    int64_t smpl_p = (sp * (int64_t)decimation);
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
static int64_t cnvSmplsToTime(int32_t samples){
    /* Calculate time (including decimation) */

    uint32_t decimation;
    acq_GetDecimationFactor(&decimation);

    double sp = 0;
    if (acq_GetADCSamplePeriod(&sp) != RP_OK){
        return 0;
    }

    return (int64_t)samples * sp * (int32_t)decimation;
}

static int setEqFilters(rp_channel_t channel){
    bool is_filter = false;
    if (rp_HPGetFastADCIsFilterPresent(&is_filter) != RP_HP_OK || is_filter == false){
        return RP_EOOR;
    }

    rp_pinState_t gain;
    if (acq_GetGain(channel, &gain) != RP_OK){
        return RP_EOOR;
    }

    CHECK_CHANNEL("setEqFilters")

    channel_filter_t filter;
    switch(gain){
        case RP_LOW:{
            if (rp_CalibGetFastADCFilter(convertCh(channel),&filter) != RP_HW_CALIB_OK){
                return RP_EOOR;
            }
            break;
        }
        case RP_HIGH:{
            if (rp_CalibGetFastADCFilter_1_20(convertCh(channel),&filter) != RP_HW_CALIB_OK){
                return RP_EOOR;
            }
            break;
        }
        default:{
            return RP_EOOR;
        }
    }

    // Update equalization filter with default coefficients
    if (channel == RP_CH_1) {
        return osc_SetEqFiltersChA(filter.aa, filter.bb, filter.kk, filter.pp);
    }

    if (channel == RP_CH_2) {
        return osc_SetEqFiltersChB(filter.aa, filter.bb, filter.kk, filter.pp);
    }

    if (channel == RP_CH_3) {
        return osc_SetEqFiltersChC(filter.aa, filter.bb, filter.kk, filter.pp);
    }

    if (channel == RP_CH_4) {
        return osc_SetEqFiltersChD(filter.aa, filter.bb, filter.kk, filter.pp);
    }

    return RP_EOOR;
}

/*----------------------------------------------------------------------------*/

int acq_GetADCSamplePeriod(double *value){
    *value = 0;
    uint32_t speed = 0;
    int ret = rp_HPGetBaseFastADCSpeedHz(&speed);
    if (ret == RP_HP_OK){
        *value = (double)1e9/speed;
    }
    return ret;
}

int acq_SetArmKeep(bool enable) {
    return osc_SetArmKeep(enable);
}

int acq_GetArmKeep(bool* state){
    return osc_GetArmKeep(state);
}

int acq_GetBufferFillState(bool* state){
    return osc_GetBufferFillState(state);
}

int acq_axi_GetBufferFillState(rp_channel_t channel, bool* state) {

    CHECK_CHANNEL("acq_axi_GetBufferFillState")

    switch (channel)
    {
    case RP_CH_1:
        return osc_axi_GetBufferFillStateChA(state);
    case RP_CH_2:
        return osc_axi_GetBufferFillStateChB(state);
    case RP_CH_3:
        return osc_axi_GetBufferFillStateChC(state);
    case RP_CH_4:
        return osc_axi_GetBufferFillStateChD(state);
    default:
        return RP_EIPV;
    }
}

int acq_SetGain(rp_channel_t channel, rp_pinState_t state){

    CHECK_CHANNEL("acq_SetGain")

    rp_pinState_t *gain = &gain_ch[channel];

    // Read old values which are dependent on the gain...
    rp_pinState_t old_gain;
    float ch_thr, ch_hyst;
    int status = 0;
    old_gain = *gain;

    int ret = acq_GetChannelThreshold(channel, &ch_thr);
    if (ret != RP_OK){
        fprintf(stderr,"[Error:acq_SetGain] Error get threshhold err: %d\n",ret);
        return ret;
    }

    ret = acq_GetChannelThresholdHyst(channel, &ch_hyst);
    if (ret != RP_OK){
        fprintf(stderr,"[Error:acq_SetGain] Error get threshhold hysteresis err: %d\n",ret);
        return ret;
    }

    bool is_attenuator = false;
    if (rp_HPGetIsAttenuatorControllerPresent(&is_attenuator) != RP_HP_OK){
        fprintf(stderr,"[Error:acq_SetGain] Error getting attenuator presence\n");
        return RP_EOOR;
    }
    if (is_attenuator){
        int ch = (channel == RP_CH_1 ? RP_MAX7311_IN1 : RP_MAX7311_IN2);
        int att = (state == RP_LOW ? RP_ATTENUATOR_1_1 : RP_ATTENUATOR_1_20);
        status = rp_setAttenuator_C(ch,att);
    }

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
        if (acq_SetChannelThreshold(channel, ch_thr) != RP_OK){
            fprintf(stderr,"[Error:acq_SetGain] Error setting threshold\n");
        }
        if (acq_SetChannelThresholdHyst(channel, ch_hyst) != RP_OK){
            fprintf(stderr,"[Error:acq_SetGain] Error setting threshold hysteresis\n");
        }
    }
    // At the end if everything is ok, update also equalization filters based on the new gain.
    // Updating eq filters should never fail...
    else {
        status = setEqFilters(channel);
    }
    return status;
}

int acq_GetGain(rp_channel_t channel, rp_pinState_t* state){

    CHECK_CHANNEL("acq_GetGain")

    *state = gain_ch[channel];
    return RP_OK;
}

int acq_GetGainV(rp_channel_t channel, float* voltage){

    CHECK_CHANNEL("acq_GetGainV")

    if (gain_ch[channel] == RP_LOW) {
        return rp_HPGetFastADCGain(channel,RP_HP_ADC_GAIN_NORMAL,voltage);
    }

    if (gain_ch[channel] == RP_HIGH) {
        return rp_HPGetFastADCGain(channel,RP_HP_ADC_GAIN_HIGH, voltage);
    }

    return 0;
}

int acq_SetDecimation(rp_acq_decimation_t decimation){
    int64_t time_ns = 0;

    if (triggerDelayInNs) {
        acq_GetTriggerDelayNs(&time_ns);
    }
    if (osc_SetDecimation((uint32_t)decimation)){
        return RP_EOOR;
    }
    // Now update trigger delay based on new decimation
    if (triggerDelayInNs) {
        acq_SetTriggerDelayNs(time_ns);
    }

    return RP_OK;
}

int acq_GetDecimation(rp_acq_decimation_t* decimation){
    uint32_t decimationVal;
    osc_GetDecimation(&decimationVal);
    *decimation = (rp_acq_decimation_t)decimationVal;
    return RP_OK;
}

int acq_SetDecimationFactor(uint32_t decimation){
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
        acq_SetTriggerDelayNs(time_ns);
    }

    return RP_OK;
}

int acq_axi_SetDecimationFactor(uint32_t decimation){

    int64_t time_ns[4] = {0,0,0,0};


    if (triggerDelayInNs) {
        acq_axi_GetTriggerDelayNs(RP_CH_1,&time_ns[0]);
        acq_axi_GetTriggerDelayNs(RP_CH_2,&time_ns[1]);
        acq_axi_GetTriggerDelayNs(RP_CH_3,&time_ns[2]);
        acq_axi_GetTriggerDelayNs(RP_CH_4,&time_ns[3]);
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
        acq_axi_SetTriggerDelayNs(RP_CH_1, time_ns[0]);
        acq_axi_SetTriggerDelayNs(RP_CH_2, time_ns[1]);
        acq_axi_SetTriggerDelayNs(RP_CH_3, time_ns[2]);
        acq_axi_SetTriggerDelayNs(RP_CH_4, time_ns[3]);
    }

    return RP_OK;
}

int acq_axi_GetDecimationFactor(uint32_t *decimation){
    osc_GetDecimation(decimation);
    return RP_OK;
}


int acq_GetDecimationFactor(uint32_t* decimation){
    osc_GetDecimation(decimation);
    return RP_OK;
}

int acq_ConvertFactorToDecimation(uint32_t factor, rp_acq_decimation_t* decimation){
    switch (factor){
        case RP_DEC_1 :
            *decimation = RP_DEC_1;
            break;
        case RP_DEC_2 :
            *decimation = RP_DEC_2;
            break;
        case RP_DEC_4 :
            *decimation = RP_DEC_4;
            break;
        case RP_DEC_8 :
            *decimation = RP_DEC_8;
            break;
        case RP_DEC_16 :
            *decimation = RP_DEC_16;
            break;
        case RP_DEC_32 :
            *decimation = RP_DEC_32;
            break;
        case RP_DEC_64 :
            *decimation = RP_DEC_64;
            break;
        case RP_DEC_128 :
            *decimation = RP_DEC_128;
            break;
        case RP_DEC_256 :
            *decimation = RP_DEC_256;
            break;
        case RP_DEC_512 :
            *decimation = RP_DEC_512;
            break;
        case RP_DEC_1024 :
            *decimation = RP_DEC_1024;
            break;
        case RP_DEC_2048 :
            *decimation = RP_DEC_2048;
            break;
        case RP_DEC_4096 :
            *decimation = RP_DEC_4096;
            break;
        case RP_DEC_8192 :
            *decimation = RP_DEC_8192;
            break;
        case RP_DEC_16384 :
            *decimation = RP_DEC_16384;
            break;
        case RP_DEC_32768 :
            *decimation = RP_DEC_32768;
            break;
        case RP_DEC_65536 :
            *decimation = RP_DEC_65536;
            break;
        default:
            return RP_EOOR;
    }
    return RP_OK;
}


int acq_GetSamplingRateHz(float* sampling_rate){
    float max_rate = 0;
    uint32_t speed = 0;
    int ret = rp_HPGetBaseFastADCSpeedHz(&speed);
    if (ret == RP_HP_OK){
        max_rate = speed;
    }else{
        return RP_EOOR;
    }

    uint32_t decimation;
    acq_GetDecimationFactor(&decimation);
    *sampling_rate = max_rate / (float)decimation;
    return RP_OK;
}

int acq_SetAveraging(bool enable){
    return osc_SetAveraging(enable);
}

int acq_GetAveraging(bool* enable){
    return osc_GetAveraging(enable);
}

int acq_SetTriggerSrc(rp_acq_trig_src_t source){
    last_trig_src = source;
    return osc_SetTriggerSource(source);
}

int acq_GetTriggerSrc(rp_acq_trig_src_t* source){
    return osc_GetTriggerSource(source);
}

int acq_GetTriggerState(rp_acq_trig_state_t* state){
    bool stateB;
    osc_GetTriggerState(&stateB);
    *state= stateB ? RP_TRIG_STATE_TRIGGERED : RP_TRIG_STATE_WAITING;
    return RP_OK;
}

int acq_SetTriggerDelay(int32_t decimated_data_num){
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

int acq_SetTriggerDelayDirect(uint32_t decimated_data_num){
    osc_SetTriggerDelay(decimated_data_num);
    triggerDelayInNs = false;
    return RP_OK;
}

int acq_axi_SetTriggerDelay(rp_channel_t channel, int32_t decimated_data_num)
{

    CHECK_CHANNEL("acq_axi_SetTriggerDelay")

    switch (channel)
    {
    case RP_CH_1:
        osc_axi_SetTriggerDelayChA(decimated_data_num);
        break;
    case RP_CH_2:
        osc_axi_SetTriggerDelayChB(decimated_data_num);
        break;
    case RP_CH_3:
        osc_axi_SetTriggerDelayChC(decimated_data_num);
        break;
    case RP_CH_4:
        osc_axi_SetTriggerDelayChD(decimated_data_num);
        break;
    default:
        return RP_EIPV;
    }
    triggerDelayInNs = false;
    return RP_OK;
}

int acq_SetTriggerDelayNs(int64_t time_ns){
    int32_t samples = cnvTimeToSmpls(time_ns);
    acq_SetTriggerDelay(samples);
    triggerDelayInNs = true;
    return RP_OK;
}

int acq_SetTriggerDelayNsDirect(uint64_t time_ns){
    int32_t samples = cnvTimeToSmpls(time_ns);
    acq_SetTriggerDelayDirect(samples);
    triggerDelayInNs = true;
    return RP_OK;
}

int acq_axi_SetTriggerDelayNs(rp_channel_t channel, int64_t time_ns)
{

    CHECK_CHANNEL("acq_axi_SetTriggerDelayNs")

    int32_t samples = cnvTimeToSmpls(time_ns);
    acq_axi_SetTriggerDelay(channel, samples);
    triggerDelayInNs = true;
    return RP_OK;
}

int acq_GetTriggerDelay(int32_t* decimated_data_num){
    uint32_t trig_dly;
    int r=osc_GetTriggerDelay(&trig_dly);
    *decimated_data_num=(int32_t)trig_dly - TRIG_DELAY_ZERO_OFFSET;
    return r;
}

int acq_GetTriggerDelayDirect(uint32_t* decimated_data_num){
    uint32_t trig_dly;
    int r=osc_GetTriggerDelay(&trig_dly);
    *decimated_data_num=trig_dly;
    return r;
}

int acq_axi_GetTriggerDelay(rp_channel_t channel, int32_t* decimated_data_num)
{

    CHECK_CHANNEL("acq_axi_GetTriggerDelay")

    uint32_t trig_dly;
    int r;
    switch (channel)
    {
    case RP_CH_1:
        r=osc_axi_GetTriggerDelayChA(&trig_dly);
        break;
    case RP_CH_2:
        r=osc_axi_GetTriggerDelayChB(&trig_dly);
        break;
    case RP_CH_3:
        r=osc_axi_GetTriggerDelayChC(&trig_dly);
        break;
    case RP_CH_4:
        r=osc_axi_GetTriggerDelayChD(&trig_dly);
        break;
    default:
        return RP_EIPV;
    }
    *decimated_data_num=(int32_t)trig_dly;
    return r;
}

int acq_GetTriggerDelayNs(int64_t* time_ns){
    int32_t samples;
    acq_GetTriggerDelay(&samples);
    *time_ns = cnvSmplsToTime(samples);
    return RP_OK;
}

int acq_GetTriggerDelayNsDirect(uint64_t* time_ns){
    uint32_t samples;
    acq_GetTriggerDelayDirect(&samples);
    *time_ns = cnvSmplsToTime(samples);
    return RP_OK;
}

int acq_axi_GetTriggerDelayNs(rp_channel_t channel, int64_t* time_ns)
{

    CHECK_CHANNEL("acq_axi_GetTriggerDelayNs")

    int32_t samples;
    acq_axi_GetTriggerDelay(channel, &samples);
    *time_ns=cnvSmplsToTime(samples);
    return RP_OK;
}

int acq_GetPreTriggerCounter(uint32_t* value){
    return osc_GetPreTriggerCounter(value);
}

int acq_GetWritePointer(uint32_t* pos){
    return osc_GetWritePointer(pos);
}

int acq_GetWritePointerAtTrig(uint32_t* pos){
    return osc_GetWritePointerAtTrig(pos);
}

int acq_axi_GetWritePointer(rp_channel_t channel, uint32_t* pos)
{

    CHECK_CHANNEL("acq_axi_GetWritePointer")

    switch (channel)
    {
    case RP_CH_1:
        return osc_axi_GetWritePointerChA(pos);
    case RP_CH_2:
        return osc_axi_GetWritePointerChB(pos);
    case RP_CH_3:
        return osc_axi_GetWritePointerChC(pos);
    case RP_CH_4:
        return osc_axi_GetWritePointerChD(pos);
    default:
        return RP_EIPV;
    }
}

int acq_axi_GetWritePointerAtTrig(rp_channel_t channel, uint32_t* pos)
{

    CHECK_CHANNEL("acq_axi_GetWritePointerAtTrig")

    switch (channel)
    {
    case RP_CH_1:
        return osc_axi_GetWritePointerAtTrigChA(pos);
    case RP_CH_2:
        return osc_axi_GetWritePointerAtTrigChB(pos);
    case RP_CH_3:
        return osc_axi_GetWritePointerAtTrigChC(pos);
    case RP_CH_4:
        return osc_axi_GetWritePointerAtTrigChD(pos);
    default:
        return RP_EIPV;
    }
}

int acq_SetTriggerLevel(rp_channel_trigger_t channel, float voltage){

    switch(channel){
        case RP_T_CH_1: return acq_SetChannelThreshold(RP_CH_1, voltage);
        case RP_T_CH_2: return acq_SetChannelThreshold(RP_CH_2, voltage);
        case RP_T_CH_3: return acq_SetChannelThreshold(RP_CH_3, voltage);
        case RP_T_CH_4: return acq_SetChannelThreshold(RP_CH_4, voltage);
        case RP_T_CH_EXT: {
                if (rp_HPGetIsExternalTriggerLevelPresentOrDefault()){
                    int ret = rp_setExtTriggerLevel(voltage);
                    switch(ret){
                        case RP_I2C_EOOR: return RP_EOOR;
                        case RP_I2C_EFRB: return RP_EFRB;
                        case RP_I2C_EFWB: return RP_EFWB;
                        case RP_I2C_OK: {
                            ext_trig_trash = voltage;
                            return RP_OK;
                        }
                        default:
                            return RP_EOOR;
                    }
                }else{
                    fprintf(stderr,"[Error:acq_SetTriggerLevel] Unsupported\n");
                    return RP_NOTS;
                }
        }
        default:
            fprintf(stderr,"[Error:acq_SetTriggerLevel] Channel is larger than allowed: %d\n",channel);
    }
    return RP_NOTS;
}

int acq_GetTriggerLevel(rp_channel_trigger_t channel,float *voltage){

    switch(channel){
        case RP_T_CH_1: return acq_GetChannelThreshold(RP_CH_1, voltage);
        case RP_T_CH_2: return acq_GetChannelThreshold(RP_CH_2, voltage);
        case RP_T_CH_3: return acq_GetChannelThreshold(RP_CH_3, voltage);
        case RP_T_CH_4: return acq_GetChannelThreshold(RP_CH_4, voltage);

        case RP_T_CH_EXT: {
                if (rp_HPGetIsExternalTriggerLevelPresentOrDefault()){
                    *voltage = ext_trig_trash;
                    return RP_OK;
                    // int ret = rp_getExtTriggerLevel(voltage);
                    // switch(ret){
                    //     case RP_I2C_EOOR: return RP_EOOR;
                    //     case RP_I2C_EFRB: return RP_EFRB;
                    //     case RP_I2C_EFWB: return RP_EFWB;
                    //     default:
                    //         return RP_OK;
                    // }
                }else{
                    fprintf(stderr,"[Error:acq_GetTriggerLevel] Unsupported\n");
                    return RP_NOTS;
                }
        }
        default:
            fprintf(stderr,"[Error:acq_GetTriggerLevel] Channel is larger than allowed: %d\n",channel);

    }
    return RP_NOTS;
}

int acq_SetChannelThreshold(rp_channel_t channel, float voltage){

    CHECK_CHANNEL("acq_SetChannelThreshold")

    float gainValue;
    float fullScale;
    rp_pinState_t mode;

    if (acq_GetGainV(channel, &gainValue) != RP_OK){
        return RP_EOOR;
    }

    if (!gainValue){
        return RP_EOOR;
    }

    if (rp_HPGetHWADCFullScale(&fullScale) != RP_OK){
        return RP_EOOR;
    }

    if (acq_GetGain(channel, &mode) != RP_OK){
        return RP_EOOR;
    }

    rp_acq_ac_dc_mode_t power_mode;
    if (acq_GetAC_DC(channel,&power_mode) != RP_OK){
        return RP_EOOR;
    }

    double gain = 1;
    int32_t offset = 0;
    uint8_t bits = 0;
    bool is_sign = true;

    int ret = 0;
    ret |= rp_HPGetFastADCBits(&bits);
    ret |= rp_HPGetFastADCIsSigned(&is_sign);

    switch (mode)
    {
        case RP_LOW:
            ret = rp_CalibGetFastADCCalibValue(convertCh(channel),convertPower(power_mode),&gain,&offset);
            break;

        case RP_HIGH:
            ret = rp_CalibGetFastADCCalibValue_1_20(convertCh(channel),convertPower(power_mode),&gain,&offset);
            break;

        default:
            fprintf(stderr,"[Error:acq_SetChannelThreshold] Unknown mode: %d\n",mode);
            return RP_EOOR;
            break;
    }

    if (ret != RP_HW_CALIB_OK){
        fprintf(stderr,"[Error:acq_SetChannelThreshold] Error get calibaration: %d\n",ret);
        return RP_EOOR;
    }

    if (voltage / gainValue > fullScale) {
        voltage = fullScale;
    }
    if (is_sign){
        if (voltage / gainValue < -fullScale) {
            voltage = -fullScale;
        }
    }else{
        if (voltage < 0) {
            voltage = 0;
        }
    }
    uint32_t cnt = cmn_convertToCnt(voltage / gainValue,bits,fullScale,is_sign,gain,offset);
    ch_trash[channel] = voltage;

    if (channel == RP_CH_1) {
        return osc_SetThresholdChA(cnt);
    }

    if (channel == RP_CH_2) {
        return osc_SetThresholdChB(cnt);
    }

    if (channel == RP_CH_3) {
        return osc_SetThresholdChC(cnt);
    }

    if (channel == RP_CH_4) {
        return osc_SetThresholdChD(cnt);
    }

    return RP_EOOR;
}

int acq_GetChannelThreshold(rp_channel_t channel, float* voltage){

    CHECK_CHANNEL("acq_GetChannelThreshold")

    *voltage = ch_trash[channel];
    return RP_OK;
}

int acq_SetTriggerHyst(float voltage){
    uint8_t channels = 0;
    if (rp_HPGetFastADCChannelsCount(&channels) != RP_HP_OK){
        channels = 0;
    }

    if (channels > 0)
        acq_SetChannelThresholdHyst(RP_CH_1, voltage);
    if (channels > 1)
        acq_SetChannelThresholdHyst(RP_CH_2, voltage);
    if (channels > 2)
        acq_SetChannelThresholdHyst(RP_CH_3, voltage);
    if (channels > 3)
        acq_SetChannelThresholdHyst(RP_CH_4, voltage);
    return RP_OK;
}

int acq_GetTriggerHyst(float *voltage){
    return acq_GetChannelThresholdHyst(RP_CH_1, voltage);
}


int acq_SetChannelThresholdHyst(rp_channel_t channel, float voltage){

    CHECK_CHANNEL("acq_SetChannelThresholdHyst")

    float fullScale;
    float gainValue;
    rp_pinState_t mode;

    if (acq_GetGainV(channel, &gainValue) != RP_OK){
        return RP_EOOR;
    }

    if (!gainValue){
        return RP_EOOR;
    }

    if (acq_GetGain(channel, &mode) != RP_OK){
        return RP_EOOR;
    }

    if (rp_HPGetHWADCFullScale(&fullScale) != RP_OK){
        return RP_EOOR;
    }

    if (fabs(voltage / gainValue) - fabs(fullScale) > FLOAT_EPS) {
        return RP_EOOR;
    }

    rp_acq_ac_dc_mode_t power_mode;
    if (acq_GetAC_DC(channel,&power_mode) != RP_OK){
        return RP_EOOR;
    }

    double gain = 1;
    int32_t offset = 0;
    uint8_t bits = 0;
    bool is_sign = true;

    int ret = rp_HPGetFastADCBits(&bits);
    ret |= rp_HPGetFastADCIsSigned(&is_sign);

    switch (mode)
    {
        case RP_LOW:
            ret = rp_CalibGetFastADCCalibValue(convertCh(channel),convertPower(power_mode),&gain,&offset);
            break;

        case RP_HIGH:
            ret = rp_CalibGetFastADCCalibValue_1_20(convertCh(channel),convertPower(power_mode),&gain,&offset);
            break;

        default:
            fprintf(stderr,"[Error:acq_SetChannelThreshold] Unknown mode: %d\n",mode);
            return RP_EOOR;
            break;
    }

    if (ret != RP_HW_CALIB_OK){
        fprintf(stderr,"[Error:acq_SetChannelThreshold] Error get calibaration: %d\n",ret);
        return RP_EOOR;
    }

    // No need calib with offeset!!!
    uint32_t cnt = cmn_convertToCnt(voltage/gainValue,bits,fullScale,is_sign,gain,0);
    ch_hyst[channel] = voltage;

    if (channel == RP_CH_1) {
        return osc_SetHysteresisChA(cnt);
    }

    if (channel == RP_CH_2) {
        return osc_SetHysteresisChB(cnt);
    }

    if (channel == RP_CH_3) {
        return osc_SetHysteresisChC(cnt);
    }

    if (channel == RP_CH_4) {
        return osc_SetHysteresisChD(cnt);
    }

    return RP_EOOR;
}

int acq_GetChannelThresholdHyst(rp_channel_t channel, float* voltage){

    CHECK_CHANNEL("acq_GetChannelThresholdHyst")

    *voltage = ch_hyst[channel];
    return RP_OK;
}

int acq_Start(){
    osc_WriteDataIntoMemory(true);
    acq_SetUnlockTrigger();
    return RP_OK;
}

int acq_Stop(){
    return osc_WriteDataIntoMemory(false);
}

int acq_Reset(){
    acq_SetDefault();
    return osc_ResetWriteStateMachine();
}

int acq_SetUnlockTrigger(){
    return osc_SetUnlockTrigger();
}

int acq_GetUnlockTrigger(bool *state){
    return osc_GetUnlockTrigger(state);
}


int acq_axi_Enable(rp_channel_t channel, bool enable)
{

    CHECK_CHANNEL("acq_axi_Enable")

    switch (channel)
    {
    case RP_CH_1:
        return osc_axi_EnableChA(enable);
    case RP_CH_2:
        return osc_axi_EnableChB(enable);
    case RP_CH_3:
        return osc_axi_EnableChC(enable);
    case RP_CH_4:
        return osc_axi_EnableChD(enable);
    default:
        return RP_EIPV;
    }
}


int acq_ResetFpga(){
    return osc_ResetWriteStateMachine();
}

static const volatile uint32_t* getRawBuffer(rp_channel_t channel){

    uint8_t channels = 0;
    if (rp_HPGetFastADCChannelsCount(&channels) != RP_HP_OK){
        fprintf(stderr,"[Error:getRawBuffer] Can't get fast ADC channels count\n");
        return NULL;
    }
    if (channel >= channels){
        fprintf(stderr,"[Error:getRawBuffer] Channel is larger than allowed\n");
        return NULL;
    }

    if (channel == RP_CH_1) {
        return osc_GetDataBufferChA();
    }

    if (channel == RP_CH_2) {
        return osc_GetDataBufferChB();
    }

    if (channel == RP_CH_3) {
        return osc_GetDataBufferChC();
    }

    if (channel == RP_CH_4) {
        return osc_GetDataBufferChD();
    }

    return NULL;
}

static const volatile uint16_t* getAxiRawBuffer(rp_channel_t channel)
{

    switch (channel)
    {
    case RP_CH_1:
        return osc_axi_GetDataBufferChA();
    case RP_CH_2:
        return osc_axi_GetDataBufferChB();
    case RP_CH_3:
        return osc_axi_GetDataBufferChC();
    case RP_CH_4:
        return osc_axi_GetDataBufferChD();
    default:
        fprintf(stderr,"[Error:getAxiRawBuffer] Channel is larger than allowed\n");
        return NULL;
    }
}

static uint32_t getSizeFromStartEndPos(uint32_t start_pos, uint32_t end_pos){
    end_pos = acq_GetNormalizedDataPos(end_pos);
    start_pos = acq_GetNormalizedDataPos(start_pos);

    if (end_pos < start_pos) {
        end_pos += ADC_BUFFER_SIZE;
    }

    return end_pos - start_pos + 1;
}

uint32_t acq_GetNormalizedDataPos(uint32_t pos){
    return (pos % ADC_BUFFER_SIZE);
}

int acq_GetDataRaw(rp_channel_t channel, uint32_t pos, uint32_t* size, int16_t* buffer,bool use_calib){

    CHECK_CHANNEL("acq_GetDataRaw")

    *size = MIN(*size, ADC_BUFFER_SIZE);

    const volatile uint32_t* raw_buffer = getRawBuffer(channel);

    if (!raw_buffer) {
        return RP_EOOR;
    }

    rp_pinState_t mode;

    if (acq_GetGain(channel, &mode) != RP_OK){
        return RP_EOOR;
    }

    rp_acq_ac_dc_mode_t power_mode;
    if (acq_GetAC_DC(channel,&power_mode) != RP_OK){
        return RP_EOOR;
    }

    uint_gain_calib_t calib;
    uint8_t bits = 0;
    bool is_sign = false;
    int ret = rp_HPGetFastADCBits(&bits);
    ret |= rp_HPGetFastADCIsSigned(&is_sign);

    switch (mode)
    {
        case RP_LOW:
            ret |= rp_CalibGetFastADCCalibValueI(convertCh(channel),convertPower(power_mode),&calib);
            break;

        case RP_HIGH:
            ret |= rp_CalibGetFastADCCalibValue_1_20I(convertCh(channel),convertPower(power_mode),&calib);
            break;

        default:
            fprintf(stderr,"[Error:acq_GetDataRaw] Unknown mode: %d\n",mode);
            return RP_EOOR;
            break;
    }

    if (ret != RP_HW_CALIB_OK){
        fprintf(stderr,"[Error:acq_GetDataRaw] Error get calibaration: %d\n",ret);
        return RP_EOOR;
    }

    uint32_t gain  = use_calib ?  calib.gain : 1;
    uint32_t g_base = use_calib ? calib.base : 1;
    int32_t offset  = use_calib ? calib.offset  : 0;

    uint32_t mask = ((uint64_t)1 << bits) - 1;

    for (uint32_t i = 0; i < (*size); ++i) {
        uint32_t cnts = (raw_buffer[(pos + i) % ADC_BUFFER_SIZE]) & mask;
        if (is_sign)
            buffer[i] = cmn_CalibCntsSigned(cnts,bits,gain,g_base,offset);
        else
            buffer[i] = cmn_CalibCntsUnsigned(cnts,bits,gain,g_base,offset);
    }

    return RP_OK;
}


int acq_axi_GetDataRaw(rp_channel_t channel, uint32_t pos, uint32_t* size, int16_t* buffer)
{
    CHECK_CHANNEL("acq_axi_GetDataRaw")

    const volatile uint16_t* raw_buffer = getAxiRawBuffer(channel);
    uint32_t buffer_size = 0;
    switch (channel)
    {
    case RP_CH_1:
        buffer_size = axi_ch_buffer_size_in_samples[0];
        break;
    case RP_CH_2:
        buffer_size = axi_ch_buffer_size_in_samples[1];
        break;
    case RP_CH_3:
        buffer_size = axi_ch_buffer_size_in_samples[2];
        break;
    case RP_CH_4:
        buffer_size = axi_ch_buffer_size_in_samples[3];
        break;
    default:
        return RP_EIPV;
    }

    if (!raw_buffer) {
        return RP_EOOR;
    }

    rp_pinState_t mode;

    if (acq_GetGain(channel, &mode) != RP_OK){
        return RP_EOOR;
    }

    uint8_t bits = 0;
    bool is_sign = false;
    int ret = rp_HPGetFastADCBits(&bits);
    ret |= rp_HPGetFastADCIsSigned(&is_sign);

    if (ret != RP_HW_CALIB_OK){
        fprintf(stderr,"[Error:acq_axi_GetDataRaw] Error get calibaration: %d\n",ret);
        return RP_EOOR;
    }

    uint32_t mask = ((uint64_t)1 << bits) - 1;

    for (uint32_t i = 0; i < (*size); ++i) {
        uint32_t cnts = (raw_buffer[(pos + i) % buffer_size]) & mask;
        if (is_sign)
            buffer[i] = cmn_CalibCntsSigned(cnts,bits,1,1,0);
        else
            buffer[i] = cmn_CalibCntsUnsigned(cnts,bits,1,1,0);
        // printf("i %d pos %x value %d\n",i,(pos + i) % buffer_size,buffer[i] );
    }
        // printf("\n");

    return RP_OK;
}

int acq_GetDataInBuffer(rp_channel_t channel, uint32_t pos, uint32_t* size,buffers_t *out){

    CHECK_CHANNEL("acq_GetDataInBuffer")

    *size = MIN(*size, ADC_BUFFER_SIZE);

    const volatile uint32_t* raw_buffer = getRawBuffer(channel);

    if (!raw_buffer) {
        return RP_EOOR;
    }

    rp_pinState_t mode;
    float fullScale;
    float gainValue;

    if (acq_GetGainV(channel, &gainValue) != RP_OK){
        return RP_EOOR;
    }

    if (acq_GetGain(channel, &mode) != RP_OK){
        return RP_EOOR;
    }

    rp_acq_ac_dc_mode_t power_mode;
    if (acq_GetAC_DC(channel,&power_mode) != RP_OK){
        return RP_EOOR;
    }

    if (rp_HPGetHWADCFullScale(&fullScale) != RP_OK){
        return RP_EOOR;
    }

    uint_gain_calib_t calib;
    uint8_t bits = 0;

    bool is_sign = false;
    int ret = rp_HPGetFastADCBits(&bits);
    ret |= rp_HPGetFastADCIsSigned(&is_sign);


    switch (mode)
    {
        case RP_LOW:
            ret |= rp_CalibGetFastADCCalibValueI(convertCh(channel),convertPower(power_mode),&calib);
            break;

        case RP_HIGH:
            ret |= rp_CalibGetFastADCCalibValue_1_20I(convertCh(channel),convertPower(power_mode),&calib);
            break;

        default:
            fprintf(stderr,"[Error:acq_GetDataInBuffer] Unknown mode: %d\n",mode);
            return RP_EOOR;
            break;
    }

    if (ret != RP_HW_CALIB_OK){
        fprintf(stderr,"[Error:acq_GetDataInBuffer] Error get calibaration: %d\n",ret);
        return RP_EOOR;
    }

    uint32_t gain_raw  = out->use_calib_for_raw ?  calib.gain : 1;
    uint32_t g_base_raw = out->use_calib_for_raw ? calib.base : 1;
    int32_t offset_raw  = out->use_calib_for_raw ? calib.offset  : 0;

    uint32_t gain_volt  = out->use_calib_for_volts ?  calib.gain : 1;
    uint32_t g_base_volt = out->use_calib_for_volts ? calib.base : 1;
    int32_t offset_volt  = out->use_calib_for_volts ? calib.offset  : 0;

    uint32_t mask = ((uint64_t)1 << bits) - 1;

    bool is_need_raw = out->ch_i[channel] != NULL;
    bool is_need_vold_f = out->ch_f[channel] != NULL;
    bool is_need_vold_d = out->ch_d[channel] != NULL;
    int16_t* iPtr = out->ch_i[channel];
    float* fPtr = out->ch_f[channel];
    double* dPtr = out->ch_d[channel];

    for (uint32_t i = 0; i < (*size); ++i) {

        uint32_t cnts = (raw_buffer[(pos + i) % ADC_BUFFER_SIZE]) & mask;

        if (is_need_raw){
            if (is_sign)
                iPtr[i] = cmn_CalibCntsSigned(cnts,bits,gain_raw,g_base_raw,offset_raw);
            else
                iPtr[i] = cmn_CalibCntsUnsigned(cnts,bits,gain_raw,g_base_raw,offset_raw);
        }

        if (is_need_vold_d || is_need_vold_f){
            float value = 0;
            if (is_sign)
                value = cmn_convertToVoltSigned(cnts,bits,fullScale,gain_volt,g_base_volt,offset_volt) * gainValue;
            else
                value = cmn_convertToVoltUnsigned(cnts,bits,fullScale,gain_volt,g_base_volt,offset_volt) * gainValue;
            if (is_need_vold_f) fPtr[i] = value;
            if (is_need_vold_d) dPtr[i] = value;
        }
    }

    return RP_OK;
}

int acq_GetData(uint32_t pos,buffers_t *out)
{
    uint8_t channels = 0;
    if (rp_HPGetFastADCChannelsCount(&channels) != RP_HP_OK){
        channels = 0;
    }

    bool fillData = false;
    out->size = MIN(out->size, ADC_BUFFER_SIZE);

    if (channels > 0){
        uint32_t size = out->size;
        int ret = acq_GetDataInBuffer(RP_CH_1,pos,&size,out);
        if (ret != RP_OK){
            return ret;
        }
        fillData = true;
    }

    if (channels > 1){
        uint32_t size = out->size;
        int ret = acq_GetDataInBuffer(RP_CH_2,pos,&size,out);
        if (ret != RP_OK){
            return ret;
        }
        fillData = true;
    }

    if (channels > 2){
        uint32_t size = out->size;
        int ret = acq_GetDataInBuffer(RP_CH_3,pos,&size,out);
        if (ret != RP_OK){
            return ret;
        }
        fillData = true;
    }

    if (channels > 3){
        uint32_t size = out->size;
        int ret = acq_GetDataInBuffer(RP_CH_4,pos,&size,out);
        if (ret != RP_OK){
            return ret;
        }
        fillData = true;
    }

    if (!fillData){
        return RP_EOOR;
    }

    return RP_OK;
}

int acq_GetDataPosRaw(rp_channel_t channel, uint32_t start_pos, uint32_t end_pos, int16_t* buffer, uint32_t *buffer_size)
{

    CHECK_CHANNEL("acq_GetDataPosRaw")

    uint32_t size = getSizeFromStartEndPos(start_pos, end_pos);

    if (size > *buffer_size) {
        return RP_BTS;
    }

    *buffer_size = size;
    return acq_GetDataRaw(channel, start_pos, buffer_size, buffer,false);
}

/**
 * Use only when write pointer has stopped...
 */
int acq_GetOldestDataRaw(rp_channel_t channel, uint32_t* size, int16_t* buffer)
{

    CHECK_CHANNEL("acq_GetOldestDataRaw")

    uint32_t pos;

    acq_GetWritePointer(&pos);
    pos++;

    return acq_GetDataRaw(channel, pos, size, buffer,false);
}

int acq_GetLatestDataRaw(rp_channel_t channel, uint32_t* size, int16_t* buffer)
{

    CHECK_CHANNEL("acq_GetLatestDataRaw")

    *size = MIN(*size, ADC_BUFFER_SIZE);

    uint32_t pos;
    acq_GetWritePointer(&pos);

    pos++;

    if ((*size) > pos) {
        pos += ADC_BUFFER_SIZE;
    }
    pos -= (*size);

    return acq_GetDataRaw(channel, pos, size, buffer,false);
}


int acq_GetDataVEx(rp_channel_t channel,  uint32_t pos, uint32_t* size, void* in_buffer,bool is_float){

    CHECK_CHANNEL("acq_GetDataVEx")

    *size = MIN(*size, ADC_BUFFER_SIZE);

    const volatile uint32_t* raw_buffer = getRawBuffer(channel);

    float fullScale;
    float gainValue;
    rp_pinState_t mode;

    if (acq_GetGainV(channel, &gainValue) != RP_OK){
        return RP_EOOR;
    }

    if (rp_HPGetHWADCFullScale(&fullScale) != RP_OK){
        return RP_EOOR;
    }

    if (acq_GetGain(channel, &mode) != RP_OK){
        return RP_EOOR;
    }

    rp_acq_ac_dc_mode_t power_mode;
    if (acq_GetAC_DC(channel,&power_mode) != RP_OK){
        return RP_EOOR;
    }

    uint8_t bits = 0;
    uint_gain_calib_t calib;
    bool is_sign = true;

    int ret = rp_HPGetFastADCBits(&bits);
    ret |= rp_HPGetFastADCIsSigned(&is_sign);

    switch (mode)
    {
        case RP_LOW:
            ret |= rp_CalibGetFastADCCalibValueI(convertCh(channel),convertPower(power_mode),&calib);
            break;

        case RP_HIGH:
            ret |= rp_CalibGetFastADCCalibValue_1_20I(convertCh(channel),convertPower(power_mode),&calib);
            break;

        default:
            fprintf(stderr,"[Error:acq_GetDataVEx] Unknown mode: %d\n",mode);
            return RP_EOOR;
            break;
    }

    if (ret != RP_HW_CALIB_OK){
        fprintf(stderr,"[Error:acq_GetDataVEx] Error get calibaration: %d\n",ret);
        return RP_EOOR;
    }

    float *buffer_f = is_float ? (float*)in_buffer: NULL;
    double *buffer_d = !is_float ? (double*)in_buffer: NULL;
    uint32_t cnts;

    uint32_t mask = ((uint64_t)1 << bits) - 1;

    for (uint32_t i = 0; i < (*size); ++i) {
        cnts = raw_buffer[(pos + i) % ADC_BUFFER_SIZE] & mask;
        float value = cmn_convertToVoltSigned(cnts,bits,fullScale,calib.gain,calib.base,calib.offset) * gainValue;
        if (buffer_f) buffer_f[i] = value;
        if (buffer_d) buffer_d[i] = value;
    }

    return RP_OK;
}

int acq_axi_GetDataVEx(rp_channel_t channel,  uint32_t pos, uint32_t* size, void* in_buffer, bool is_float){

    CHECK_CHANNEL("acq_axi_GetDataVEx")

    const volatile uint16_t* raw_buffer = getAxiRawBuffer(channel);
    uint32_t buffer_size = 0;
    switch (channel)
    {
    case RP_CH_1:
        buffer_size = axi_ch_buffer_size_in_samples[0];
        break;
    case RP_CH_2:
        buffer_size = axi_ch_buffer_size_in_samples[1];
        break;
    case RP_CH_3:
        buffer_size = axi_ch_buffer_size_in_samples[2];
        break;
    case RP_CH_4:
        buffer_size = axi_ch_buffer_size_in_samples[3];
        break;
    default:
        return RP_EIPV;
    }

    float fullScale;
    float gainValue;
    rp_pinState_t mode;

    if (acq_GetGainV(channel, &gainValue) != RP_OK){
        return RP_EOOR;
    }

    if (rp_HPGetHWADCFullScale(&fullScale) != RP_OK){
        return RP_EOOR;
    }

    if (acq_GetGain(channel, &mode) != RP_OK){
        return RP_EOOR;
    }

    rp_acq_ac_dc_mode_t power_mode;
    if (acq_GetAC_DC(channel,&power_mode) != RP_OK){
        return RP_EOOR;
    }

    if (!raw_buffer) {
        return RP_EOOR;
    }

    uint8_t bits = 0;
    uint_gain_calib_t calib;
    bool is_sign = true;

    int ret = rp_HPGetFastADCBits(&bits);
    ret |= rp_HPGetFastADCIsSigned(&is_sign);

    switch (mode)
    {
        case RP_LOW:
            ret |= rp_CalibGetFastADCCalibValueI(convertCh(channel),convertPower(power_mode),&calib);
            break;

        case RP_HIGH:
            ret |= rp_CalibGetFastADCCalibValue_1_20I(convertCh(channel),convertPower(power_mode),&calib);
            break;

        default:
            fprintf(stderr,"[Error:acq_axi_GetDataVEx] Unknown mode: %d\n",mode);
            return RP_EOOR;
            break;
    }

    if (ret != RP_HW_CALIB_OK){
        fprintf(stderr,"[Error:acq_axi_GetDataVEx] Error get calibaration: %d\n",ret);
        return RP_EOOR;
    }

    float *buffer_f = is_float ? (float*)in_buffer: NULL;
    double *buffer_d = !is_float ? (double*)in_buffer: NULL;
    uint32_t cnts;
    uint32_t mask = ((uint64_t)1 << bits) - 1;

    for (uint32_t i = 0; i < (*size); ++i) {
        cnts = raw_buffer[(pos + i) % buffer_size] & mask;
        float value = cmn_convertToVoltSigned(cnts,bits,fullScale,calib.gain,calib.base,calib.offset) * gainValue;
        if (buffer_f) buffer_f[i] = value;
        if (buffer_d) buffer_d[i] = value;
    }

    return RP_OK;
}

int acq_GetDataV(rp_channel_t channel,  uint32_t pos, uint32_t* size, float* buffer){
    return acq_GetDataVEx(channel,pos,size,buffer,true);
}

int acq_axi_GetDataV(rp_channel_t channel,  uint32_t pos, uint32_t* size, float* buffer){
    return acq_axi_GetDataVEx(channel,pos,size,buffer,true);
}

int acq_GetDataPosV(rp_channel_t channel,  uint32_t start_pos, uint32_t end_pos, float* buffer, uint32_t *buffer_size)
{

    CHECK_CHANNEL("acq_GetDataPosV")

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

    CHECK_CHANNEL("acq_GetOldestDataV")

    uint32_t pos;

    acq_GetWritePointer(&pos);
    pos++;

    return acq_GetDataV(channel, pos, size, buffer);
}

int acq_GetLatestDataV(rp_channel_t channel, uint32_t* size, float* buffer)
{

    CHECK_CHANNEL("acq_GetLatestDataV")

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

int acq_axi_SetBufferSamples(rp_channel_t channel, uint32_t address, uint32_t _samples) {

    CHECK_CHANNEL("acq_axi_SetBuffer")

    uint32_t start,res_size;
    osc_axi_GetMemoryRegion(&start,&res_size);

    if (address < start){
        fprintf(stderr,"[Error:acq_axi_SetBuffer] Start address lower than reserved. Address: 0x%X reserved 0x%X\n",address,start);
        return RP_EOOR;
    }

    if (address + (_samples - 4) * 2 > start + res_size){
        fprintf(stderr,"[Error:acq_axi_SetBuffer] The specified buffer size is greater than the reserved memory - 8 bytes. End address: 0x%X End reserved 0x%X\n",address + _samples * 2,start + res_size);
        return RP_EOOR;
    }


    if (channel == RP_CH_1) {
        osc_axi_SetAddressStartChA(address);
        osc_axi_SetAddressEndChA(address + _samples * 2);
        axi_ch_buffer_size_in_samples[0] = _samples;
    }

    if (channel == RP_CH_2) {
        osc_axi_SetAddressStartChB(address);
        osc_axi_SetAddressEndChB(address + _samples * 2);
        axi_ch_buffer_size_in_samples[1] = _samples;
    }

    if (channel == RP_CH_3) {
        osc_axi_SetAddressStartChC(address);
        osc_axi_SetAddressEndChC(address + _samples * 2);
        axi_ch_buffer_size_in_samples[2] = _samples;
    }

    if (channel == RP_CH_4) {
        osc_axi_SetAddressStartChD(address);
        osc_axi_SetAddressEndChD(address + _samples * 2);
        axi_ch_buffer_size_in_samples[3] = _samples;
    }

    return RP_OK;
}

int acq_axi_SetBufferBytes(rp_channel_t channel, uint32_t address, uint32_t _size){
    if (_size % 2){
        fprintf(stderr,"[Error:acq_axi_SetBuffer] Buffer size must be a multiple of 2\n");
        return RP_EOOR;
    }
    return acq_axi_SetBufferSamples(channel,address,_size / 2);
}



/**
 * Sets default configuration
 * @return
 */
int acq_SetDefault() {
    uint32_t start,size;
    osc_axi_GetMemoryRegion(&start,&size);

    uint8_t channels = 0;
    if (rp_HPGetFastADCChannelsCount(&channels) != RP_HP_OK){
        fprintf(stderr,"[Error:acq_SetDefault] Can't get fast ADC channels count\n");
        return RP_NOTS;
    }

    acq_SetDecimation(RP_DEC_1);
    acq_SetAveraging(true);
    acq_SetTriggerSrc(RP_TRIG_SRC_DISABLED);
    acq_SetTriggerDelay(0);
    acq_SetTriggerDelayNs(0);
    acq_SetArmKeep(false);

    for(int i = 0; i < channels; i++){
        acq_SetChannelThreshold((rp_channel_t)i, 0.0);
        acq_SetChannelThresholdHyst((rp_channel_t)i, 0.005);
        acq_SetGain((rp_channel_t)i, RP_LOW);
        acq_axi_Enable((rp_channel_t)i, false);
        acq_axi_SetBufferBytes((rp_channel_t)i, start, 0);
        acq_axi_SetTriggerDelay((rp_channel_t)i, 0);
        acq_axi_SetTriggerDelayNs((rp_channel_t)i, 0);

        if(rp_HPGetFastADCIsAC_DCOrDefault()){
            acq_SetAC_DC((rp_channel_t)i,RP_DC);
        }
    }
    return RP_OK;
}



int acq_SetAC_DC(rp_channel_t channel,rp_acq_ac_dc_mode_t mode){

    CHECK_CHANNEL("acq_SetAC_DC")

    uint8_t channels = 0;
    if (rp_HPGetFastADCChannelsCount(&channels) != RP_HP_OK){
        fprintf(stderr,"[Error:acq_SetAC_DC] Can't get fast ADC channels count\n");
        return RP_NOTS;
    }

    if (channel >= channels && mode == RP_DC){
        fprintf(stderr,"[Error:acq_SetAC_DC] Channel is larger than allowed\n");
        return RP_NOTS;
    }

    bool is_ac_dc = rp_HPGetFastADCIsAC_DCOrDefault();

    if (channel >= channels && mode == RP_AC && is_ac_dc){
        fprintf(stderr,"[Error:acq_SetAC_DC] Channel is larger than allowed\n");
        return RP_NOTS;
    }

    rp_acq_ac_dc_mode_t *power_mode = &power_mode_ch[channel];
    int status = RP_NOTS;
    if (is_ac_dc){
        int ch = (channel == RP_CH_1 ? RP_MAX7311_IN1 : RP_MAX7311_IN2);
        status = rp_setAC_DC_C(ch,mode  == RP_DC ? RP_DC_MODE : RP_AC_MODE);
    }

    *power_mode = status == RP_OK ? mode : RP_DC;

    return status;
}

int acq_GetAC_DC(rp_channel_t channel,rp_acq_ac_dc_mode_t *status){

    CHECK_CHANNEL("acq_GetAC_DC")

    *status = power_mode_ch[channel];
    return RP_OK;
}

int acq_axi_GetMemoryRegion(uint32_t *_start,uint32_t *_size){
    return osc_axi_GetMemoryRegion(_start,_size);
}

int acq_UpdateAcqFilter(rp_channel_t channel){

    CHECK_CHANNEL("acq_UpdateAcqFilter")

    return setEqFilters(channel);
}

int acq_GetFilterCalibValue(rp_channel_t channel,uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp){

    CHECK_CHANNEL("acq_GetFilterCalibValue")

    if (channel == RP_CH_1){
        return osc_GetEqFiltersChA(coef_aa,coef_bb,coef_kk,coef_pp);
    }
    if (channel == RP_CH_2){
        return osc_GetEqFiltersChB(coef_aa,coef_bb,coef_kk,coef_pp);
    }
    if (channel == RP_CH_3){
        return osc_GetEqFiltersChC(coef_aa,coef_bb,coef_kk,coef_pp);
    }
    if (channel == RP_CH_4){
        return osc_GetEqFiltersChD(coef_aa,coef_bb,coef_kk,coef_pp);
    }
    return RP_EOOR;
}


int acq_SetExtTriggerDebouncerUs(double value){
    if (value < 0)
        return RP_EIPV;

    double sp = 0;
    int ret = acq_GetADCSamplePeriod(&sp);
    if (ret != RP_OK){
        return ret;
    }
    uint32_t samples = (value * 1000.0) / sp;
    return osc_SetExtTriggerDebouncer(samples);
}

int acq_GetExtTriggerDebouncerUs(double *value){
    double sp = 0;
    int ret = acq_GetADCSamplePeriod(&sp);
    if (ret != RP_OK){
        return ret;
    }
    uint32_t samples = 0;
    osc_GetExtTriggerDebouncer(&samples);
    *value = (samples * sp) / 1000.0;
    return RP_OK;
}