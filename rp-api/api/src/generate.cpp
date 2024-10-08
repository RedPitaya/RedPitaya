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
static volatile int32_t *data_ch[2] = {NULL,NULL};


int generate_Init() {
    int ret = cmn_Map(GENERATE_BASE_SIZE, GENERATE_BASE_ADDR, (void **) &generate);
    if (ret != RP_OK){
        return ret;
    }
    data_ch[0] = (int32_t *) ((char *) generate + (CHA_DATA_OFFSET));
    data_ch[1] = (int32_t *) ((char *) generate + (CHB_DATA_OFFSET));
    return ret;
}

int generate_Release() {
    int ret = RP_OK;
    if (generate)
        ret = cmn_Unmap(GENERATE_BASE_SIZE, (void **) &generate);
    data_ch[0] = NULL;
    data_ch[1] = NULL;
    return ret;
}

int getChannelPropertiesAddress(volatile ch_properties_t **ch_properties, rp_channel_t channel) {
    CHANNEL_ACTION(channel,
            *ch_properties = &generate->properties_chA,
            *ch_properties = &generate->properties_chB)
    return RP_OK;
}

int generate_setOutputDisable(rp_channel_t channel, bool disable) {
    if (channel == RP_CH_1) {
        cmn_Debug("generate->AsetOutputTo0 <- 0x%X",disable ? 1 : 0);
        generate->AsetOutputTo0 = disable ? 1 : 0;
    }
    else if (channel == RP_CH_2) {
        cmn_Debug("generate->BsetOutputTo0 <- 0x%X",disable ? 1 : 0);
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

int generate_setFrequency(rp_channel_t channel, float frequency,float baseFreq) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    double valuef  = 65536.0 * (double)frequency / (double)baseFreq * (double)DAC_BUFFER_SIZE;
    uint32_t value = floor(valuef);
    cmn_Debug("[Ch%d] ch_properties->counterStep <- 0x%X",channel,value);
    ch_properties->counterStep = value;
    cmn_Debug("[Ch%d] ch_properties->cunterStepChLower <- 0x%X",channel,value);
    if (channel == RP_CH_1)
        generate->cunterStepChALower = (valuef - (float)value) * 0xFFFFFFFF;
    if (channel == RP_CH_2)
        generate->cunterStepChBLower = (valuef - (float)value) * 0xFFFFFFFF;
    uint32_t wrap_flag = 1;
    cmn_Debug("[Ch%d] generate->_SM_WrapPointer <- 0x%X",channel,wrap_flag);
    channel == RP_CH_1 ? (generate->ASM_WrapPointer = wrap_flag) : (generate->BSM_WrapPointer = wrap_flag);
    return RP_OK;
}

int generate_getFrequency(rp_channel_t channel, float *frequency,float baseFreq) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    *frequency = (float) round((ch_properties->counterStep * baseFreq) / (65536 * DAC_BUFFER_SIZE));
    return RP_OK;
}

int generate_setWrapCounter(rp_channel_t channel, uint32_t size) {
    cmn_Debug("[Ch%d] generate->properties_ch_.counterWrap <- 0x%X",channel,65536 * size - 1);
    CHANNEL_ACTION(channel,
            generate->properties_chA.counterWrap = 65536 * size - 1,
            generate->properties_chB.counterWrap = 65536 * size - 1)
    return RP_OK;
}

int generate_setTriggerSource(rp_channel_t channel, unsigned short value) {
    cmn_Debug("[Ch%d] generate->_triggerSelector <- 0x%X",channel,value);
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
    cmn_Debug("[Ch%d] generate->_gatedBursts <- 0x%X",channel,value);
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
    cmn_Debug("[Ch%d] ch_properties->cyclesInOneBurs <- 0x%X",channel,num);
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
    cmn_Debug("[Ch%d] ch_properties->burstRepetitions <- 0x%X",channel,repetitions);
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
    cmn_Debug("[Ch%d] ch_properties->delayBetweenBurstRepetitions <- 0x%X",channel,delay);
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
        cmn_Debug("[Ch%d] cmn_UnsetBits((uint32_t *) generate) mask 0x000F000F <- 0x%X",channel,curValue);
        cmn_UnsetBits((uint32_t *) generate, curValue, mask);
        cmn_Debug("[Ch%d] cmn_SetBits((uint32_t *) generate) mask 0x000F000F <- 0x%X",channel,curValue);
        return cmn_SetBits((uint32_t *) generate, curValue, mask);
    }
    return RP_EOOR;
}

int generate_simultaneousTrigger() {
    uint32_t mask = 0x000F000F;
    uint32_t curValue = 0;
    if (cmn_GetValue((uint32_t *) generate,&curValue,mask) == RP_OK){
        cmn_Debug("cmn_UnsetBits((uint32_t *) generate) mask 0x000F000F <- 0x%X",curValue);
        cmn_UnsetBits((uint32_t *) generate, curValue, mask);
        cmn_Debug("cmn_SetBits((uint32_t *) generate) mask 0x000F000F <- 0x%X",curValue);
        return cmn_SetBits((uint32_t *) generate, curValue, mask);
    }
    return RP_EOOR;
}

int generate_setOutputEnableSync(bool enable){
    if (enable) {
         cmn_Debug("cmn_UnsetBits((uint32_t *) generate) mask 0x00800080 <- 0x%X",0x00800080);
         return cmn_UnsetBits((uint32_t *) generate, 0x00800080 , 0x00800080);
    }else{
         cmn_Debug("cmn_SetBits((uint32_t *) generate) mask 0x00800080 <- 0x%X",0x00800080);
         return cmn_SetBits((uint32_t *) generate, 0x00800080 , 0x00800080);
    }
}


int generate_ResetSM() {
    // Both channels must be reset state machine
    cmn_Debug("cmn_SetBits((uint32_t *) generate) mask 0x00400040 <- 0x%X",0x00400040);
    cmn_SetBits((uint32_t *) generate, 0x00400040, 0x00400040);
    cmn_Debug("cmn_UnsetBits((uint32_t *) generate) mask 0x00400040 <- 0x%X",0x00400040);
    cmn_UnsetBits((uint32_t *) generate, 0x00400040, 0x00400040);
    return RP_OK;
}

int generate_ResetChannelSM(rp_channel_t channel){
    uint32_t value = channel == RP_CH_1 ? 0x00000040 : 0x00400000;
    cmn_Debug("cmn_SetBits((uint32_t *) generate) mask 0x00400040 <- 0x%X",value);
    cmn_SetBits((uint32_t *) generate, value, value);
    cmn_Debug("cmn_UnsetBits((uint32_t *) generate) mask 0x00400040 <- 0x%X",value);
    cmn_UnsetBits((uint32_t *) generate, value, value);
    return RP_OK;
}


int generate_writeData(rp_channel_t channel, float *data, int32_t start, uint32_t length) {

    uint8_t bits = 0;
    if (rp_HPGetFastDACBits(&bits) != RP_HP_OK){
        ERROR_LOG("Can't get fast DAC bits\n");
        return RP_NOTS;
    }

    bool is_sign = false;
    if (rp_HPGetFastDACIsSigned(&is_sign) != RP_HP_OK){
        ERROR_LOG("Can't get fast DAC sign value\n");
        return RP_NOTS;
    }

    volatile int32_t *dataOut = data_ch[channel];

    volatile ch_properties_t *properties;
    getChannelPropertiesAddress(&properties, channel);
    generate_setWrapCounter(channel, length);

    if (start < 0) start += DAC_BUFFER_SIZE;
    for(int i = start; i < start + DAC_BUFFER_SIZE; i++) {
        dataOut[i % DAC_BUFFER_SIZE] =  cmn_convertToCnt(data[i-start],bits,1.0,is_sign,1,0);
    }
    return RP_OK;
}

int generate_setAmplitude(rp_channel_t channel,rp_gen_gain_t gain, float amplitude) {

    float fsBase = 0;
    if (rp_HPGetHWDACFullScale(&fsBase) != RP_HP_OK){
        ERROR_LOG("Can't get fast HW DAC full scale");
        return RP_NOTS;
    }

    if (fsBase == 0){
        ERROR_LOG("HW DAC Full Scale is zero");
        return RP_NOTS;
    }

    uint8_t bits = 0;
    if (rp_HPGetFastDACBits(&bits) != RP_HP_OK){
        ERROR_LOG("Can't get fast DAC bits");
        return RP_NOTS;
    }

    bool is_sign = false;
    if (rp_HPGetFastDACIsSigned(&is_sign) != RP_HP_OK){
        ERROR_LOG("Can't get fast DAC sign value");
        return RP_NOTS;
    }

    bool x5_gain = false;
    if (rp_HPGetIsGainDACx5(&x5_gain) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC x5 gain");
        return RP_NOTS;
    }

    if (!x5_gain && gain == RP_GAIN_5X){
        ERROR_LOG("Can't set gain on unsupported board");
        return RP_NOTS;
    }

    double gain_calib;
    int32_t offset;
    int ret = 0;
    switch (gain)
    {
        case RP_GAIN_1X:
            ret = rp_CalibGetFastDACCalibValue(convertCh(channel),RP_GAIN_CALIB_1X,&gain_calib,&offset);
        break;
        case RP_GAIN_5X:
            ret = rp_CalibGetFastDACCalibValue(convertCh(channel),RP_GAIN_CALIB_5X,&gain_calib,&offset);
        break;
        default:
            ERROR_LOG("Unknown gain: %d",gain);
            return RP_EOOR;
            break;
    }

    if (ret != RP_HW_CALIB_OK){
        ERROR_LOG("Get calibaration: %d",ret);
        return RP_EOOR;
    }

    volatile ch_properties_t *ch_properties;
    //uint32_t amp_max = calib_getGenScale(channel,gain);
    getChannelPropertiesAddress(&ch_properties, channel);

    //uint32_t value = cmn_CnvVToCnt(DATA_BIT_LENGTH, amplitude, AMPLITUDE_MAX , false, amp_max, 0, 0.0);
    // Spechal convert from Volt to RAW. 0x2000 = 1x
    int32_t value = cmn_convertToCnt(amplitude * gain_calib, bits, fsBase,is_sign,1, 0);
    //  fprintf(stderr,"Gain %f amplitude  %f ofcalb %d res %d bits %d fsBase %f\n",gain_calib,amplitude,0,value,bits,fsBase);
    cmn_Debug("[Ch%d] ch_properties->amplitudeScale <- 0x%X",channel,value);
    ch_properties->amplitudeScale = value;
    return RP_OK;
}


int generate_setDCOffset(rp_channel_t channel,rp_gen_gain_t gain, float offset) {

    float fsBase = 0;
    if (rp_HPGetHWDACFullScale(&fsBase) != RP_HP_OK){
        ERROR_LOG("Can't get fast HW DAC full scale");
        return RP_NOTS;
    }

    if (fsBase == 0){
        ERROR_LOG("HW DAC Full Scale is zero");
        return RP_NOTS;
    }

    uint8_t bits = 0;
    if (rp_HPGetFastDACBits(&bits) != RP_HP_OK){
        ERROR_LOG("Can't get fast DAC bits");
        return RP_NOTS;
    }

    bool is_sign = false;
    if (rp_HPGetFastDACIsSigned(&is_sign) != RP_HP_OK){
        ERROR_LOG("Can't get fast DAC sign value");
        return RP_NOTS;
    }

    bool x5_gain = false;
    if (rp_HPGetIsGainDACx5(&x5_gain) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC x5 gain");
        return RP_NOTS;
    }

    if (!x5_gain && gain == RP_GAIN_5X){
        ERROR_LOG("Can't set gain on unsupported board");
        return RP_NOTS;
    }

    double gain_calib;
    int32_t offset_calib;
    int ret = 0;
    switch (gain)
    {
        case RP_GAIN_1X:
            ret = rp_CalibGetFastDACCalibValue(convertCh(channel),RP_GAIN_CALIB_1X,&gain_calib,&offset_calib);
        break;
        case RP_GAIN_5X:
            ret = rp_CalibGetFastDACCalibValue(convertCh(channel),RP_GAIN_CALIB_5X,&gain_calib,&offset_calib);
        break;
        default:
            ERROR_LOG("Unknown gain: %d",gain);
            return RP_EOOR;
            break;
    }

    if (ret != RP_HW_CALIB_OK){
        ERROR_LOG("Get calibaration: %d",ret);
        return RP_EOOR;
    }

    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    int32_t value = cmn_convertToCnt(offset * gain_calib,bits,fsBase,is_sign,1,offset_calib);
//    fprintf(stderr,"Gain %f offset  %f ofcalb %d res %d bits %d fsBase %f\n",gain_calib,offset,offset_calib,value,bits,fsBase);
    cmn_Debug("[Ch%d] ch_properties->amplitudeOffset <- 0x%X",channel,value);
    ch_properties->amplitudeOffset = value;
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
        cmn_Debug("generate->AtempProtected <- 0x%X",enable ? 1 : 0);
        generate->AtempProtected = enable ? 1 : 0;
    }
    else if (channel == RP_CH_2) {
        cmn_Debug("generate->BtempProtected <- 0x%X",enable ? 1 : 0);
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
        cmn_Debug("generate->AlatchedTempAlarm <- 0x%X",state ? 1 : 0);
        generate->AlatchedTempAlarm = state ? 1 : 0;
    }
    else if (channel == RP_CH_2) {
        cmn_Debug("generate->BlatchedTempAlarm <- 0x%X",state ? 1 : 0);
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

int generate_setBurstLastValue(rp_channel_t channel,rp_gen_gain_t gain, float amplitude){
    float fsBase = 0;
    if (rp_HPGetHWDACFullScale(&fsBase) != RP_HP_OK){
        ERROR_LOG("Can't get fast HW DAC full scale");
        return RP_NOTS;
    }

    if (fsBase == 0){
        ERROR_LOG("HW DAC Full Scale is zero");
        return RP_NOTS;
    }

    uint8_t bits = 0;
    if (rp_HPGetFastDACBits(&bits) != RP_HP_OK){
        ERROR_LOG("Can't get fast DAC bits");
        return RP_NOTS;
    }

    bool is_sign = false;
    if (rp_HPGetFastDACIsSigned(&is_sign) != RP_HP_OK){
        ERROR_LOG("Can't get fast DAC sign value");
        return RP_NOTS;
    }

    uint32_t cnt = cmn_convertToCnt(amplitude,bits,fsBase,is_sign,1.0,0);
    cmn_Debug("[Ch%d] generate->BurstFinalValue_ch <- 0x%X",channel,cnt);
    CHANNEL_ACTION(channel,
        generate->BurstFinalValue_chA = cnt,
        generate->BurstFinalValue_chB = cnt)
    return RP_OK;
}


int generate_setInitGenValue(rp_channel_t channel,rp_gen_gain_t gain, float amplitude){
    float fsBase = 0;
    if (rp_HPGetHWDACFullScale(&fsBase) != RP_HP_OK){
        ERROR_LOG("Can't get fast HW DAC full scale");
        return RP_NOTS;
    }

    if (fsBase == 0){
        ERROR_LOG("HW DAC Full Scale is zero");
        return RP_NOTS;
    }

    uint8_t bits = 0;
    if (rp_HPGetFastDACBits( &bits) != RP_HP_OK){
        ERROR_LOG("Can't get fast DAC bits");
        return RP_NOTS;
    }

    bool is_sign = false;
    if (rp_HPGetFastDACIsSigned(&is_sign) != RP_HP_OK){
        ERROR_LOG("Can't get fast DAC sign value");
        return RP_NOTS;
    }

    uint32_t cnt = cmn_convertToCnt(amplitude,bits,fsBase,is_sign,1.0,0);
    cmn_Debug("[Ch%d] generate->initGenValue_ch <- 0x%X",channel,cnt);
    CHANNEL_ACTION(channel,
        generate->initGenValue_chA = cnt,
        generate->initGenValue_chB = cnt)
    return RP_OK;
}

int generate_SetTriggerDebouncer(uint32_t value){
    if (DEBAUNCER_MASK < value) {
        cmn_Debug("[generate_SetTriggerDebouncer] Error: osc_reg.trig_dbc_t <- 0x%X",value);
        return RP_EIPV;
    }
    cmn_Debug("[generate_SetTriggerDebouncer] osc_reg.trig_dbc_t <- 0x%X",value);
    generate->trig_dbc_t = value;

    return RP_OK;
}

int generate_GetTriggerDebouncer(uint32_t *value){
    *value = generate->trig_dbc_t;
    cmn_Debug("[generate_GetTriggerDebouncer] osc_reg.trig_dbc_t -> 0x%X",*value);
    return RP_OK;
}

int generate_setRandomSeed(rp_channel_t channel, uint32_t seed){
    cmn_Debug("[Ch%d] generate->randomSeed_ch <- 0x%X",channel,seed);
    CHANNEL_ACTION(channel,
        generate->randomSeed_chA = seed,
        generate->randomSeed_chB = seed)
    return RP_OK;
}

int generate_getRandomSeed(rp_channel_t channel, uint32_t *seed){
    uint32_t value;
    CHANNEL_ACTION(channel,
            value = generate->randomSeed_chA,
            value = generate->randomSeed_chB)
    *seed = value;
    return RP_OK;
}

int generate_setEnableRandom(rp_channel_t channel, bool enable){
    cmn_Debug("[Ch%d] generate->enableNoise_ch <- 0x%X",channel,enable);
    CHANNEL_ACTION(channel,
        generate->enableNoise_chA = enable,
        generate->enableNoise_chB = enable)
    return RP_OK;
}

int generate_getEnableRandom(rp_channel_t channel, bool *enable){
    bool value;
    CHANNEL_ACTION(channel,
            value = generate->enableNoise_chA,
            value = generate->enableNoise_chB)
    *enable = value;
    return RP_OK;
}
