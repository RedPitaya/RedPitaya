/**
 * $Id: $
 *
 * @brief Red Pitaya library Generate module interface
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
#include <stdlib.h>
#include <math.h>
#include "rp_cross.h"
#include "common.h"
#include "generate.h"
#include "calib.h"

static volatile generate_control_t *generate = NULL;
static volatile int32_t *data_chA = NULL;
static volatile int32_t *data_chB = NULL;


int generate_Init() {
    cmn_Map(GENERATE_BASE_SIZE, GENERATE_BASE_ADDR, (void **) &generate);
    data_chA = (int32_t *) ((char *) generate + (CHA_DATA_OFFSET));
    data_chB = (int32_t *) ((char *) generate + (CHB_DATA_OFFSET));
    return RP_OK;
}

int generate_Release() {
    cmn_Unmap(GENERATE_BASE_SIZE, (void **) &generate);
    data_chA = NULL;
    data_chB = NULL;
    return RP_OK;
}

int getChannelPropertiesAddress(volatile ch_properties_t **ch_properties, rp_channel_t channel) {
    CHANNEL_ACTION(channel,
            *ch_properties = &generate->properties_chA,
            *ch_properties = &generate->properties_chB)
    return RP_OK;
}

int generate_setOutputDisable(rp_channel_t channel, bool disable) {
    if (channel == RP_CH_1) {
        generate->AsetOutputTo0 = disable ? 1 : 0;
    }
    else if (channel == RP_CH_2) {
        generate->BsetOutputTo0 = disable ? 1 : 0;
    }
    else {
        return RP_EPN;
    }
    return RP_OK;
}

int generate_getOutputEnabled(rp_channel_t channel, bool *enabled) {
    uint32_t value;
    CHANNEL_ACTION(channel,
            value = generate->AsetOutputTo0,
            value = generate->BsetOutputTo0)
    *enabled = value == 1 ? false : true;
    return RP_OK;
}

int generate_getEnableTempProtection(rp_channel_t channel, bool *enable){
    #ifdef Z20_250_12
        bool value;
        CHANNEL_ACTION(channel,
                value = generate->AtempProtected,
                value = generate->BtempProtected)
        *enable = value;
        return RP_OK;
    #else
        return RP_NOTS;
    #endif
}

int generate_setEnableTempProtection(rp_channel_t channel, bool enable) {
    #ifdef Z20_250_12
        if (channel == RP_CH_1) {
            generate->AtempProtected = enable ? 1 : 0;
        }
        else if (channel == RP_CH_2) {
            generate->BtempProtected = enable ? 1 : 0;
        }
        else {
            return RP_EPN;
        }
        return RP_OK;
    #else
        return RP_NOTS;
    #endif
}

int generate_getLatchTempAlarm(rp_channel_t channel, bool *state){
    #ifdef Z20_250_12
        bool value;
        CHANNEL_ACTION(channel,
                value = generate->AlatchedTempAlarm,
                value = generate->BlatchedTempAlarm)
        *state = value;
        return RP_OK;
    #else
        return RP_NOTS;
    #endif
}

int generate_setLatchTempAlarm(rp_channel_t channel, bool state) {
    #ifdef Z20_250_12
        if (channel == RP_CH_1) {
            generate->AlatchedTempAlarm = state ? 1 : 0;
        }
        else if (channel == RP_CH_2) {
            generate->BlatchedTempAlarm = state ? 1 : 0;
        }
        else {
            return RP_EPN;
        }
        return RP_OK;
    #else
        return RP_NOTS;
    #endif
}

int generate_getRuntimeTempAlarm(rp_channel_t channel, bool *state){
    #ifdef Z20_250_12
        bool value;
        CHANNEL_ACTION(channel,
                value = generate->AruntimeTempAlarm,
                value = generate->BruntimeTempAlarm)
        *state = value;
        return RP_OK;
    #else
        return RP_NOTS;
    #endif
}

#ifndef Z20_250_12

int generate_setAmplitude(rp_channel_t channel, float amplitude) {
    volatile ch_properties_t *ch_properties;
    uint32_t amp_max = calib_getGenScale(channel);
    getChannelPropertiesAddress(&ch_properties, channel);
    ch_properties->amplitudeScale = cmn_CnvVToCnt(DATA_BIT_LENGTH, amplitude , AMPLITUDE_MAX , false, amp_max, 0, 0.0);
    return RP_OK;
}

int generate_getAmplitude(rp_channel_t channel, float *amplitude) {
    volatile ch_properties_t *ch_properties;
    uint32_t amp_max = calib_getGenScale(channel);
    getChannelPropertiesAddress(&ch_properties, channel);
    *amplitude = cmn_CnvNormCntToV(DATA_BIT_LENGTH, ch_properties->amplitudeScale, AMPLITUDE_MAX , amp_max, 0, 0.0 , 1.0);
    return RP_OK;
}

int generate_setDCOffset(rp_channel_t channel, float offset) {
    volatile ch_properties_t *ch_properties;
    int dc_offs = calib_getGenOffset(channel);
    uint32_t amp_max = calib_getGenScale(channel);
    getChannelPropertiesAddress(&ch_properties, channel);
    ch_properties->amplitudeOffset = cmn_CnvVToCnt(DATA_BIT_LENGTH, offset , (float) (OFFSET_MAX/2.f), false, amp_max, dc_offs, 0);
    return RP_OK;
}

int generate_getDCOffset(rp_channel_t channel, float *offset) {
    volatile ch_properties_t *ch_properties;
    int dc_offs = calib_getGenOffset(channel);
    uint32_t amp_max = calib_getGenScale(channel);
    getChannelPropertiesAddress(&ch_properties, channel);
    *offset = cmn_CnvNormCntToV(DATA_BIT_LENGTH, ch_properties->amplitudeOffset, (float) (OFFSET_MAX/2.f), amp_max, dc_offs, 0 , 1.0);
    return RP_OK;
}

int generate_setBurstLastValue(rp_channel_t channel, float amplitude){
    int dc_offs = calib_getGenOffset(channel);
    uint32_t amp_max = calib_getGenScale(channel);
    uint32_t cnt = cmn_CnvVToCnt(DATA_BIT_LENGTH, amplitude , AMPLITUDE_MAX, false, amp_max, dc_offs, 0);
    CHANNEL_ACTION(channel,
        generate->BurstFinalValue_chA = cnt,
        generate->BurstFinalValue_chB = cnt)
    return RP_OK;
}

int generate_getBurstLastValue(rp_channel_t channel, float *amplitude){
    int dc_offs = calib_getGenOffset(channel);
    uint32_t amp_max = calib_getGenScale(channel);
    *amplitude = cmn_CnvNormCntToV(DATA_BIT_LENGTH, channel == RP_CH_1 ? (generate->BurstFinalValue_chA) : (generate->BurstFinalValue_chB)
        , AMPLITUDE_MAX , amp_max, dc_offs, 0.0 , 1.0);
    return RP_OK;
}


#else

int generate_setAmplitude(rp_channel_t channel,rp_gen_gain_t gain, float amplitude) {
    volatile ch_properties_t *ch_properties;
    uint32_t amp_max = calib_getGenScale(channel,gain);
    getChannelPropertiesAddress(&ch_properties, channel);
    ch_properties->amplitudeScale = cmn_CnvVToCnt(DATA_BIT_LENGTH, amplitude, AMPLITUDE_MAX , false, amp_max, 0, 0.0);
    return RP_OK;
}

int generate_getAmplitude(rp_channel_t channel,rp_gen_gain_t gain, float *amplitude) {
    volatile ch_properties_t *ch_properties;
    uint32_t amp_max = calib_getGenScale(channel,gain);
    getChannelPropertiesAddress(&ch_properties, channel);
    *amplitude = cmn_CnvNormCntToV(DATA_BIT_LENGTH, ch_properties->amplitudeScale, AMPLITUDE_MAX  , amp_max, 0, 0.0, 1.0);
    return RP_OK;
}

int generate_setDCOffset(rp_channel_t channel,rp_gen_gain_t gain, float offset) {
    volatile ch_properties_t *ch_properties;
    int dc_offs = calib_getGenOffset(channel,gain);
    uint32_t amp_max = calib_getGenScale(channel,gain);
    getChannelPropertiesAddress(&ch_properties, channel);
    ch_properties->amplitudeOffset = cmn_CnvVToCnt(DATA_BIT_LENGTH, offset, (float) (OFFSET_MAX/2.f), false, amp_max, dc_offs, 0);
    return RP_OK;
}

int generate_getDCOffset(rp_channel_t channel,rp_gen_gain_t gain, float *offset) {
    volatile ch_properties_t *ch_properties;
    int dc_offs = calib_getGenOffset(channel,gain);
    uint32_t amp_max = calib_getGenScale(channel,gain);
    getChannelPropertiesAddress(&ch_properties, channel);
    *offset = cmn_CnvNormCntToV(DATA_BIT_LENGTH, ch_properties->amplitudeOffset, (float) (OFFSET_MAX/2.f), amp_max, dc_offs, 0 , 1.0);
    return RP_OK;
}

#endif

int generate_setFrequency(rp_channel_t channel, float frequency) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    ch_properties->counterStep = (uint32_t) round(65536 * frequency / DAC_FREQUENCY * BUFFER_LENGTH);
    uint32_t wrap_flag = 1;
    channel == RP_CH_1 ? (generate->ASM_WrapPointer = wrap_flag) : (generate->BSM_WrapPointer = wrap_flag);
    return RP_OK;
}

int generate_getFrequency(rp_channel_t channel, float *frequency) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    *frequency = (float) round((ch_properties->counterStep * DAC_FREQUENCY) / (65536 * BUFFER_LENGTH));
    return RP_OK;
}

int generate_setWrapCounter(rp_channel_t channel, uint32_t size) {
    CHANNEL_ACTION(channel,
            generate->properties_chA.counterWrap = 65536 * size - 1,
            generate->properties_chB.counterWrap = 65536 * size - 1)
    return RP_OK;
}

int generate_setTriggerSource(rp_channel_t channel, unsigned short value) {
    CHANNEL_ACTION(channel,
            generate->AtriggerSelector = value,
            generate->BtriggerSelector = value)
    return RP_OK;
}

int generate_getTriggerSource(rp_channel_t channel, uint32_t *value) {
    CHANNEL_ACTION(channel,
            *value = generate->AtriggerSelector,
            *value = generate->BtriggerSelector)
    return RP_OK;
}

int generate_setGatedBurst(rp_channel_t channel, uint32_t value) {
    CHANNEL_ACTION(channel,
            generate->AgatedBursts = value,
            generate->BgatedBursts = value)
    return RP_OK;
}

int generate_getGatedBurst(rp_channel_t channel, uint32_t *value) {
    CHANNEL_ACTION(channel,
            *value = generate->AgatedBursts,
            *value = generate->BgatedBursts)
    return RP_OK;
}

int generate_setBurstCount(rp_channel_t channel, uint32_t num) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    ch_properties->cyclesInOneBurst = num;
    return RP_OK;
}

int generate_getBurstCount(rp_channel_t channel, uint32_t *num) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    *num = ch_properties->cyclesInOneBurst;
    return RP_OK;
}

int generate_setBurstRepetitions(rp_channel_t channel, uint32_t repetitions) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    ch_properties->burstRepetitions = repetitions;
    return RP_OK;
}

int generate_getBurstRepetitions(rp_channel_t channel, uint32_t *repetitions) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    *repetitions = ch_properties->burstRepetitions;
    return RP_OK;
}

int generate_setBurstDelay(rp_channel_t channel, uint32_t delay) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    ch_properties->delayBetweenBurstRepetitions = delay;
    return RP_OK;
}

int generate_getBurstDelay(rp_channel_t channel, uint32_t *delay) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    *delay = ch_properties->delayBetweenBurstRepetitions;
    return RP_OK;
}

int generate_simultaneousTrigger() {
    // simultaneously trigger both channels
    return cmn_SetBits((uint32_t *) generate, 0x00010001, 0xFFFFFFFF);
}

int generate_setOutputEnableSync(bool enable){
    if (enable) {
        cmn_UnsetBits((uint32_t *) generate, 0x00810081 , 0xFFFFFFFF);
        generate_Synchronise();
        return cmn_SetBits((uint32_t *) generate, 0x00110011 , 0xFFFFFFFF);
    }else{
        return cmn_SetBits((uint32_t *) generate, 0x00800080 , 0xFFFFFFFF);
    }
}


int generate_Synchronise() {
    // Both channels must be reset simultaneously
    cmn_SetBits((uint32_t *) generate, 0x00400040, 0xFFFFFFFF);
    cmn_UnsetBits((uint32_t *) generate, 0x00400040, 0xFFFFFFFF);
    return RP_OK;
}

int generate_writeData(rp_channel_t channel, float *data, uint32_t start, uint32_t length) {
    volatile int32_t *dataOut;
    CHANNEL_ACTION(channel,
            dataOut = data_chA,
            dataOut = data_chB)

    volatile ch_properties_t *properties;
    getChannelPropertiesAddress(&properties, channel);
    generate_setWrapCounter(channel, length);

    //rp_calib_params_t calib = calib_GetParams();
    int dc_offs = 0;//channel == RP_CH_1 ? calib.be_ch1_dc_offs: calib.be_ch2_dc_offs;
    uint32_t amp_max = 0; //channel == RP_CH_1 ? calib.be_ch1_fs: calib.be_ch2_fs;

    for(int i = start; i < start+BUFFER_LENGTH; i++) {
        dataOut[i % BUFFER_LENGTH] = cmn_CnvVToCnt(DATA_BIT_LENGTH, data[i-start] * AMPLITUDE_MAX , AMPLITUDE_MAX, false, amp_max, dc_offs, 0.0);
    }
    return RP_OK;
}
