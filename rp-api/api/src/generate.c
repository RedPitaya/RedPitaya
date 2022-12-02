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
#include "common.h"
#include "generate.h"

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
        cmn_Debug("generate->AsetOutputTo0",disable ? 1 : 0);
        generate->AsetOutputTo0 = disable ? 1 : 0;
    }
    else if (channel == RP_CH_2) {
        cmn_Debug("generate->BsetOutputTo0",disable ? 1 : 0);
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

int generate_setFrequency(rp_channel_t channel, float frequency) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    uint32_t value = (uint32_t) round(65536 * frequency / DAC_FREQUENCY * DAC_BUFFER_SIZE);
    cmn_DebugCh("ch_properties->counterStep",channel,value);
    ch_properties->counterStep = value;
    uint32_t wrap_flag = 1;
    cmn_DebugCh("generate->_SM_WrapPointer",channel,wrap_flag);
    channel == RP_CH_1 ? (generate->ASM_WrapPointer = wrap_flag) : (generate->BSM_WrapPointer = wrap_flag);
    return RP_OK;
}

int generate_getFrequency(rp_channel_t channel, float *frequency) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    *frequency = (float) round((ch_properties->counterStep * DAC_FREQUENCY) / (65536 * DAC_BUFFER_SIZE));
    return RP_OK;
}

int generate_setWrapCounter(rp_channel_t channel, uint32_t size) {
    cmn_DebugCh("generate->properties_ch_.counterWrap",channel,65536 * size - 1);
    CHANNEL_ACTION(channel,
            generate->properties_chA.counterWrap = 65536 * size - 1,
            generate->properties_chB.counterWrap = 65536 * size - 1)
    return RP_OK;
}

int generate_setTriggerSource(rp_channel_t channel, unsigned short value) {
    cmn_DebugCh("generate->_triggerSelector",channel,value);
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
    cmn_DebugCh("generate->_gatedBursts",channel,value);
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
    cmn_DebugCh("ch_properties->cyclesInOneBurs",channel,num);
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
    cmn_DebugCh("ch_properties->burstRepetitions",channel,repetitions);
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
    cmn_DebugCh("ch_properties->delayBetweenBurstRepetitions",channel,delay);
    ch_properties->delayBetweenBurstRepetitions = delay;
    return RP_OK;
}

int generate_getBurstDelay(rp_channel_t channel, uint32_t *delay) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    *delay = ch_properties->delayBetweenBurstRepetitions;
    return RP_OK;
}

int generate_Trigger(rp_channel_t channel){
    uint32_t mask = 0;
    switch(channel){
        case RP_CH_1:
            mask = 0x0000000F;
            break;
        case RP_CH_2:
            mask = 0x000F0000;
            break;
        default:
            return RP_EOOR;
    }
    uint32_t curValue = 0;
    if (cmn_GetValue((uint32_t *) generate,&curValue,mask) == RP_OK){
        cmn_DebugCh("cmn_UnsetBits((uint32_t *) generate) mask 0x000F000F",channel,curValue);
        cmn_UnsetBits((uint32_t *) generate, curValue, mask);
        cmn_DebugCh("cmn_SetBits((uint32_t *) generate) mask 0x000F000F",channel,curValue);
        return cmn_SetBits((uint32_t *) generate, curValue, mask);
    }
    return RP_EOOR;
}

int generate_simultaneousTrigger() {
    uint32_t mask = 0x000F000F;
    uint32_t curValue = 0;
    if (cmn_GetValue((uint32_t *) generate,&curValue,mask) == RP_OK){
        cmn_Debug("cmn_UnsetBits((uint32_t *) generate) mask 0x000F000F",curValue);
        cmn_UnsetBits((uint32_t *) generate, curValue, mask);
        cmn_Debug("cmn_SetBits((uint32_t *) generate) mask 0x000F000F",curValue);
        return cmn_SetBits((uint32_t *) generate, curValue, mask);
    }
    return RP_EOOR;
}

int generate_setOutputEnableSync(bool enable){
    if (enable) {
         cmn_Debug("cmn_UnsetBits((uint32_t *) generate) mask 0x00800080",0x00800080);
         return cmn_UnsetBits((uint32_t *) generate, 0x00800080 , 0x00800080);
    }else{
         cmn_Debug("cmn_SetBits((uint32_t *) generate) mask 0x00800080",0x00800080);
         return cmn_SetBits((uint32_t *) generate, 0x00800080 , 0x00800080);
    }
}


int generate_ResetSM() {
    // Both channels must be reset state machine
    cmn_Debug("cmn_SetBits((uint32_t *) generate) mask 0x00400040",0x00400040);
    cmn_SetBits((uint32_t *) generate, 0x00400040, 0x00400040);
    cmn_Debug("cmn_UnsetBits((uint32_t *) generate) mask 0x00400040",0x00400040);
    cmn_UnsetBits((uint32_t *) generate, 0x00400040, 0x00400040);
    return RP_OK;
}

int generate_ResetChannelSM(rp_channel_t channel){
    uint32_t value = channel == RP_CH_1 ? 0x00000040 : 0x00400000;
    cmn_Debug("cmn_SetBits((uint32_t *) generate) mask 0x00400040",value);
    cmn_SetBits((uint32_t *) generate, value, value);
    cmn_Debug("cmn_UnsetBits((uint32_t *) generate) mask 0x00400040",value);
    cmn_UnsetBits((uint32_t *) generate, value, value);
    return RP_OK;
}


int generate_writeData(rp_channel_t channel, float *data, int32_t start, uint32_t length) {
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
    if (start < 0) start += DAC_BUFFER_SIZE;
    for(int i = start; i < start + DAC_BUFFER_SIZE; i++) {
        dataOut[i % DAC_BUFFER_SIZE] = cmn_CnvVToCnt(DATA_BIT_LENGTH, data[i-start] * AMPLITUDE_MAX , AMPLITUDE_MAX, false, amp_max, dc_offs, 0.0);
    }
    return RP_OK;
}


#if defined Z10 || defined Z20 || defined Z20_125 || defined Z20_125_4CH

int generate_setAmplitude(rp_channel_t channel, float amplitude) {
    volatile ch_properties_t *ch_properties;
    uint32_t amp_max = calib_getGenScale(channel);
    getChannelPropertiesAddress(&ch_properties, channel);
    uint32_t value = cmn_CnvVToCnt(DATA_BIT_LENGTH, amplitude , AMPLITUDE_MAX , false, amp_max, 0, 0.0);
    cmn_DebugCh("ch_properties->amplitudeScale",channel,value);
    ch_properties->amplitudeScale = value;
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
    uint32_t value = cmn_CnvVToCnt(DATA_BIT_LENGTH, offset , (float) (OFFSET_MAX/2.f), false, amp_max, dc_offs, 0);
    cmn_DebugCh("ch_properties->amplitudeScale",channel,value);
    ch_properties->amplitudeOffset = value;
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
    cmn_DebugCh("generate->BurstFinalValue_ch",channel,cnt);
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

#endif

#ifdef Z20_250_12

int generate_setAmplitude(rp_channel_t channel,rp_gen_gain_t gain, float amplitude) {
    volatile ch_properties_t *ch_properties;
    uint32_t amp_max = calib_getGenScale(channel,gain);
    getChannelPropertiesAddress(&ch_properties, channel);
    uint32_t value = cmn_CnvVToCnt(DATA_BIT_LENGTH, amplitude, AMPLITUDE_MAX , false, amp_max, 0, 0.0);
    cmn_DebugCh("ch_properties->amplitudeScale",channel,value);
    ch_properties->amplitudeScale = value;
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
    uint32_t value = cmn_CnvVToCnt(DATA_BIT_LENGTH, offset, (float) (OFFSET_MAX/2.f), false, amp_max, dc_offs, 0);
    cmn_DebugCh("ch_properties->amplitudeOffset",channel,value);
    ch_properties->amplitudeOffset = value;
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

int generate_getEnableTempProtection(rp_channel_t channel, bool *enable){
    bool value;
    CHANNEL_ACTION(channel,
            value = generate->AtempProtected,
            value = generate->BtempProtected)
    *enable = value;
    return RP_OK;
}

int generate_setEnableTempProtection(rp_channel_t channel, bool enable) {
    if (channel == RP_CH_1) {
        cmn_Debug("generate->AtempProtected",disable ? 1 : 0);
        generate->AtempProtected = enable ? 1 : 0;
    }
    else if (channel == RP_CH_2) {
        cmn_Debug("generate->BtempProtected",disable ? 1 : 0);
        generate->BtempProtected = enable ? 1 : 0;
    }
    else {
        return RP_EPN;
    }
    return RP_OK;
}

int generate_getLatchTempAlarm(rp_channel_t channel, bool *state){
    bool value;
    CHANNEL_ACTION(channel,
            value = generate->AlatchedTempAlarm,
            value = generate->BlatchedTempAlarm)
    *state = value;
    return RP_OK;
}

int generate_setLatchTempAlarm(rp_channel_t channel, bool state) {
    if (channel == RP_CH_1) {
        cmn_Debug("generate->AlatchedTempAlarm",disable ? 1 : 0);
        generate->AlatchedTempAlarm = state ? 1 : 0;
    }
    else if (channel == RP_CH_2) {
        cmn_Debug("generate->BlatchedTempAlarm",disable ? 1 : 0);
        generate->BlatchedTempAlarm = state ? 1 : 0;
    }
    else {
        return RP_EPN;
    }
    return RP_OK;
}

int generate_getRuntimeTempAlarm(rp_channel_t channel, bool *state){
    bool value;
    CHANNEL_ACTION(channel,
            value = generate->AruntimeTempAlarm,
            value = generate->BruntimeTempAlarm)
    *state = value;
    return RP_OK;
}

#endif