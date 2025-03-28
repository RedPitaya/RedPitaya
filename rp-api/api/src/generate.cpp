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

#include "generate.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "convert.hpp"

static volatile generate_control_t* generate = NULL;
static volatile int32_t* data_ch[2] = {NULL, NULL};
static volatile uint64_t asg_axi_mem_reserved_index[4] = {0, 0, 0, 0};

int generate_Init() {
    int ret = cmn_Map(GENERATE_BASE_SIZE, GENERATE_BASE_ADDR, (void**)&generate);
    if (ret != RP_OK) {
        return ret;
    }
    data_ch[0] = (int32_t*)((char*)generate + (CHA_DATA_OFFSET));
    data_ch[1] = (int32_t*)((char*)generate + (CHB_DATA_OFFSET));
    return ret;
}

int generate_Release() {
    int ret = RP_OK;
    if (generate)
        ret = cmn_Unmap(GENERATE_BASE_SIZE, (void**)&generate);
    data_ch[0] = NULL;
    data_ch[1] = NULL;
    return ret;
}

// int getChannelPropertiesAddress(volatile ch_properties_t** ch_properties, rp_channel_t channel) {
//     CHANNEL_ACTION(channel, *ch_properties = &generate->properties_chA, *ch_properties = &generate->properties_chB)
//     return RP_OK;
// }

int generate_printRegset() {
    if (!rp_HPIsFastDAC_PresentOrDefault()) {
        return RP_NOTS;
    }

    auto channels = rp_HPGetFastADCChannelsCountOrDefault();

    volatile generate_control_t* generate = NULL;

    int fd1 = -1;
    int ret = cmn_InitMap(GENERATE_BASE_SIZE, GENERATE_BASE_ADDR, (void**)&generate, &fd1);
    if (ret != RP_OK) {
        return ret;
    }
    asg_config_control_u_t config;
    config.reg_full = generate->config;
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Config", GENERATE_BASE_ADDR + offsetof(generate_control_t, config), generate->config);
    for (auto i = 0; i < channels; i++) {
        printf("Channel %d\n", i + 1);
        config.reg[i].print();
    }

    asg_ch_amp_scale_u_t prop;
    prop.reg_full = generate->ampAndScale_ch1;
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Amplitude scale & offset", GENERATE_BASE_ADDR + offsetof(generate_control_t, ampAndScale_ch1),
             generate->ampAndScale_ch1);
    prop.reg.print();
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Counter Wrap", GENERATE_BASE_ADDR + offsetof(generate_control_t, counterWrap_ch1), generate->counterWrap_ch1);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Start offset", GENERATE_BASE_ADDR + offsetof(generate_control_t, startOffset_ch1), generate->startOffset_ch1);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Counter step", GENERATE_BASE_ADDR + offsetof(generate_control_t, counterStep_ch1), generate->counterStep_ch1);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Counter step lower", GENERATE_BASE_ADDR + offsetof(generate_control_t, counterStepLower_ch1),
             generate->counterStepLower_ch1);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Cycles In One Burst", GENERATE_BASE_ADDR + offsetof(generate_control_t, cyclesInOneBurst_ch1),
             generate->cyclesInOneBurst_ch1);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Burst Repetitions", GENERATE_BASE_ADDR + offsetof(generate_control_t, burstRepetitions_ch1),
             generate->burstRepetitions_ch1);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Delay Burst Repetitions", GENERATE_BASE_ADDR + offsetof(generate_control_t, delayBetweenBurstRepetitions_ch1),
             generate->delayBetweenBurstRepetitions_ch1);

    prop.reg_full = generate->ampAndScale_ch2;
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Amplitude scale & offset", GENERATE_BASE_ADDR + offsetof(generate_control_t, ampAndScale_ch2),
             generate->ampAndScale_ch2);
    prop.reg.print();
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Counter Wrap", GENERATE_BASE_ADDR + offsetof(generate_control_t, counterWrap_ch2), generate->counterWrap_ch2);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Start offset", GENERATE_BASE_ADDR + offsetof(generate_control_t, startOffset_ch2), generate->startOffset_ch2);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Counter step", GENERATE_BASE_ADDR + offsetof(generate_control_t, counterStep_ch2), generate->counterStep_ch2);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Counter step lower", GENERATE_BASE_ADDR + offsetof(generate_control_t, counterStepLower_ch2),
             generate->counterStepLower_ch2);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Cycles In One Burst", GENERATE_BASE_ADDR + offsetof(generate_control_t, cyclesInOneBurst_ch2),
             generate->cyclesInOneBurst_ch2);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Burst Repetitions", GENERATE_BASE_ADDR + offsetof(generate_control_t, burstRepetitions_ch2),
             generate->burstRepetitions_ch2);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Delay Burst Repetitions", GENERATE_BASE_ADDR + offsetof(generate_control_t, delayBetweenBurstRepetitions_ch2),
             generate->delayBetweenBurstRepetitions_ch2);

    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Burst Final Value", GENERATE_BASE_ADDR + offsetof(generate_control_t, BurstFinalValue_ch1),
             generate->BurstFinalValue_ch1);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Burst Final Value", GENERATE_BASE_ADDR + offsetof(generate_control_t, BurstFinalValue_ch2),
             generate->BurstFinalValue_ch2);

    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Trigger debuncer", GENERATE_BASE_ADDR + offsetof(generate_control_t, trig_dbc), generate->trig_dbc);

    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Init Gen Value", GENERATE_BASE_ADDR + offsetof(generate_control_t, initGenValue_ch1),
             generate->initGenValue_ch1);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Init Gen Value", GENERATE_BASE_ADDR + offsetof(generate_control_t, initGenValue_ch2),
             generate->initGenValue_ch2);

    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Length Last Value State", GENERATE_BASE_ADDR + offsetof(generate_control_t, lengthLastValueState_ch1),
             generate->lengthLastValueState_ch1);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Length Last Value State", GENERATE_BASE_ADDR + offsetof(generate_control_t, lengthLastValueState_ch2),
             generate->lengthLastValueState_ch2);

    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Random Seed", GENERATE_BASE_ADDR + offsetof(generate_control_t, randomSeed_ch1), generate->randomSeed_ch1);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Random Seed", GENERATE_BASE_ADDR + offsetof(generate_control_t, randomSeed_ch2), generate->randomSeed_ch2);

    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Enable Noise", GENERATE_BASE_ADDR + offsetof(generate_control_t, enableNoise_ch1), generate->enableNoise_ch1);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Enable Noise", GENERATE_BASE_ADDR + offsetof(generate_control_t, enableNoise_ch2), generate->enableNoise_ch2);

    asg_axi_state_u_t asg_axi_state;
    asg_axi_state.reg_full = generate->axi_state;
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "AXI state", GENERATE_BASE_ADDR + offsetof(generate_control_t, axi_state), generate->axi_state);
    asg_axi_state.reg.print();

    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Enable AXI", GENERATE_BASE_ADDR + offsetof(generate_control_t, enableAXI_Ch1), generate->enableAXI_Ch1);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Axi Start Address", GENERATE_BASE_ADDR + offsetof(generate_control_t, axi_start_address_Ch1),
             generate->axi_start_address_Ch1);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Axi End Address", GENERATE_BASE_ADDR + offsetof(generate_control_t, axi_end_address_Ch1),
             generate->axi_end_address_Ch1);

    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Enable AXI", GENERATE_BASE_ADDR + offsetof(generate_control_t, enableAXI_Ch2), generate->enableAXI_Ch2);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Axi Start Address", GENERATE_BASE_ADDR + offsetof(generate_control_t, axi_start_address_Ch2),
             generate->axi_start_address_Ch2);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Axi End Address", GENERATE_BASE_ADDR + offsetof(generate_control_t, axi_end_address_Ch2),
             generate->axi_end_address_Ch2);

    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Axi Error Read Count", GENERATE_BASE_ADDR + offsetof(generate_control_t, axi_error_read_count_Ch1),
             generate->axi_error_read_count_Ch1);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Axi Transfer Count", GENERATE_BASE_ADDR + offsetof(generate_control_t, axi_transfer_count_Ch1),
             generate->axi_transfer_count_Ch1);

    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Axi Error Read Count", GENERATE_BASE_ADDR + offsetof(generate_control_t, axi_error_read_count_Ch2),
             generate->axi_error_read_count_Ch2);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Axi Transfer Count", GENERATE_BASE_ADDR + offsetof(generate_control_t, axi_transfer_count_Ch2),
             generate->axi_transfer_count_Ch2);

    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch1 Axi Decimation", GENERATE_BASE_ADDR + offsetof(generate_control_t, axi_decimation_Ch1),
             generate->axi_decimation_Ch1);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Ch2 Axi Decimation", GENERATE_BASE_ADDR + offsetof(generate_control_t, axi_decimation_Ch2),
             generate->axi_decimation_Ch2);

    return cmn_ReleaseClose(fd1, GENERATE_BASE_SIZE, (void**)&generate);
}

int generate_setOutputDisable(rp_channel_t channel, bool disable) {
    cmn_Debug("generate->config[%d]->setOutputTo0 <- 0x%X", channel, disable ? 1 : 0);
    asg_config_control_u_t conf;
    conf.reg_full = generate->config;
    conf.reg[channel].setOutputTo0 = disable ? 1 : 0;
    generate->config = conf.reg_full;
    return RP_OK;
}

int generate_getOutputEnabled(rp_channel_t channel, bool* enabled) {
    asg_config_control_u_t conf;
    conf.reg_full = generate->config;
    *enabled = conf.reg[channel].setOutputTo0 == 1 ? false : true;
    return RP_OK;
}

int generate_setFrequency(rp_channel_t channel, float frequency, float baseFreq) {
    double valuef = 65536.0 * (double)frequency / (double)baseFreq * (double)DAC_BUFFER_SIZE;
    uint32_t value = floor(valuef);

    if (channel == RP_CH_1) {
        cmn_Debug("[Ch%d] ch_properties->counterStep <- 0x%X", channel, value);
        generate->counterStep_ch1 = value;
        value = (valuef - (float)value) * 0xFFFFFFFF;
        cmn_Debug("[Ch%d] ch_properties->counterStepLower <- 0x%X", channel, value);
        generate->counterStepLower_ch1 = value;
    }

    if (channel == RP_CH_2) {
        cmn_Debug("[Ch%d] ch_properties->counterStep <- 0x%X", channel, value);
        generate->counterStep_ch2 = value;
        value = (valuef - (float)value) * 0xFFFFFFFF;
        cmn_Debug("[Ch%d] ch_properties->counterStepLower <- 0x%X", channel, value);
        generate->counterStepLower_ch2 = value;
    }

    uint32_t wrap_flag = 1;
    cmn_Debug("[Ch%d] generate->_SM_WrapPointer <- 0x%X", channel, wrap_flag);
    asg_config_control_u_t conf;
    conf.reg_full = generate->config;
    conf.reg[channel].SM_WrapPointer = wrap_flag;
    generate->config = conf.reg_full;
    return RP_OK;
}

int generate_getFrequency(rp_channel_t channel, float* frequency, float baseFreq) {
    uint32_t value = 0;
    if (channel == RP_CH_1) {
        value = generate->counterStep_ch1;
    }

    if (channel == RP_CH_2) {
        value = generate->counterStep_ch2;
    }

    *frequency = (float)round((value * baseFreq) / (65536 * DAC_BUFFER_SIZE));
    return RP_OK;
}

int generate_setWrapCounter(rp_channel_t channel, uint32_t size) {
    cmn_Debug("[Ch%d] generate->properties_ch_.counterWrap <- 0x%X", channel, 65536 * size - 1);
    if (channel == RP_CH_1) {
        generate->counterWrap_ch1 = 65536 * size - 1;
    }

    if (channel == RP_CH_2) {
        generate->counterWrap_ch2 = 65536 * size - 1;
    }
    return RP_OK;
}

int generate_setTriggerSource(rp_channel_t channel, unsigned short value) {
    cmn_Debug("[Ch%d] generate->_triggerSelector <- 0x%X", channel, value);
    asg_config_control_u_t conf;
    conf.reg_full = generate->config;
    conf.reg[channel].triggerSelector = value;
    generate->config = conf.reg_full;
    return RP_OK;
}

int generate_getTriggerSource(rp_channel_t channel, uint32_t* value) {
    asg_config_control_u_t conf;
    conf.reg_full = generate->config;
    *value = conf.reg[channel].triggerSelector;
    return RP_OK;
}

int generate_setGatedBurst(rp_channel_t channel, uint32_t value) {
    cmn_Debug("[Ch%d] generate->_gatedBursts <- 0x%X", channel, value);
    asg_config_control_u_t conf;
    conf.reg_full = generate->config;
    conf.reg[channel].gatedBursts = value;
    generate->config = conf.reg_full;
    return RP_OK;
}

int generate_getGatedBurst(rp_channel_t channel, uint32_t* value) {
    asg_config_control_u_t conf;
    conf.reg_full = generate->config;
    *value = conf.reg[channel].gatedBursts;
    return RP_OK;
}

int generate_setBurstCount(rp_channel_t channel, uint32_t num) {
    cmn_Debug("[Ch%d] ch_properties->cyclesInOneBurs <- 0x%X", channel, num);

    if (channel == RP_CH_1) {
        generate->cyclesInOneBurst_ch1 = num;
    }

    if (channel == RP_CH_2) {
        generate->cyclesInOneBurst_ch2 = num;
    }

    return RP_OK;
}

int generate_getBurstCount(rp_channel_t channel, uint32_t* num) {

    if (channel == RP_CH_1) {
        *num = generate->cyclesInOneBurst_ch1;
    }

    if (channel == RP_CH_2) {
        *num = generate->cyclesInOneBurst_ch2;
    }

    return RP_OK;
}

int generate_setBurstRepetitions(rp_channel_t channel, uint32_t repetitions) {
    cmn_Debug("[Ch%d] ch_properties->burstRepetitions <- 0x%X", channel, repetitions);

    if (channel == RP_CH_1) {
        generate->burstRepetitions_ch1 = repetitions;
    }

    if (channel == RP_CH_2) {
        generate->burstRepetitions_ch2 = repetitions;
    }
    return RP_OK;
}

int generate_getBurstRepetitions(rp_channel_t channel, uint32_t* repetitions) {

    if (channel == RP_CH_1) {
        *repetitions = generate->burstRepetitions_ch1;
    }

    if (channel == RP_CH_2) {
        *repetitions = generate->burstRepetitions_ch2;
    }

    return RP_OK;
}

int generate_setBurstDelay(rp_channel_t channel, uint32_t delay) {
    cmn_Debug("[Ch%d] ch_properties->delayBetweenBurstRepetitions <- 0x%X", channel, delay);

    if (channel == RP_CH_1) {
        generate->delayBetweenBurstRepetitions_ch1 = delay;
    }

    if (channel == RP_CH_2) {
        generate->delayBetweenBurstRepetitions_ch2 = delay;
    }

    return RP_OK;
}

int generate_getBurstDelay(rp_channel_t channel, uint32_t* delay) {

    if (channel == RP_CH_1) {
        *delay = generate->delayBetweenBurstRepetitions_ch1;
    }

    if (channel == RP_CH_2) {
        *delay = generate->delayBetweenBurstRepetitions_ch2;
    }
    return RP_OK;
}

int generate_Trigger(rp_channel_t channel) {
    uint32_t mask = 0;
    switch (channel) {
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
    if (cmn_GetValue((uint32_t*)generate, &curValue, mask) == RP_OK) {
        cmn_Debug("[Ch%d] cmn_UnsetBits((uint32_t *) generate) mask 0x000F000F <- 0x%X", channel, curValue);
        cmn_UnsetBits((uint32_t*)generate, curValue, mask);
        cmn_Debug("[Ch%d] cmn_SetBits((uint32_t *) generate) mask 0x000F000F <- 0x%X", channel, curValue);
        return cmn_SetBits((uint32_t*)generate, curValue, mask);
    }
    return RP_EOOR;
}

int generate_TriggerBoth() {
    uint32_t mask = 0x000F000F;
    uint32_t curValue = 0;
    if (cmn_GetValue((uint32_t*)generate, &curValue, mask) == RP_OK) {
        cmn_Debug("cmn_UnsetBits((uint32_t *) generate) mask 0x000F000F <- 0x%X", curValue);
        cmn_UnsetBits((uint32_t*)generate, curValue, mask);
        cmn_Debug("cmn_SetBits((uint32_t *) generate) mask 0x000F000F <- 0x%X", curValue);
        return cmn_SetBits((uint32_t*)generate, curValue, mask);
    }
    return RP_EOOR;
}

int generate_simultaneousTrigger() {
    bool enabled[2] = {false, false};
    generate_getOutputEnabled(RP_CH_1, &enabled[0]);
    generate_getOutputEnabled(RP_CH_2, &enabled[1]);
    uint32_t mask = 0;
    mask |= enabled[0] ? 0xF : 0;
    mask |= enabled[1] ? 0x000F0000 : 0;
    uint32_t curValue = 0;
    if (cmn_GetValue((uint32_t*)generate, &curValue, mask) == RP_OK) {
        cmn_Debug("cmn_UnsetBits((uint32_t *) generate) mask 0x000F000F <- 0x%X", curValue);
        cmn_UnsetBits((uint32_t*)generate, curValue, mask);
        cmn_Debug("cmn_SetBits((uint32_t *) generate) mask 0x000F000F <- 0x%X", curValue);
        return cmn_SetBits((uint32_t*)generate, curValue, mask);
    }
    return RP_EOOR;
}

int generate_setOutputEnableSync(bool enable) {
    if (enable) {
        cmn_Debug("cmn_UnsetBits((uint32_t *) generate) mask 0x00800080 <- 0x%X", 0x00800080);
        return cmn_UnsetBits((uint32_t*)generate, 0x00800080, 0x00800080);
    } else {
        cmn_Debug("cmn_SetBits((uint32_t *) generate) mask 0x00800080 <- 0x%X", 0x00800080);
        return cmn_SetBits((uint32_t*)generate, 0x00800080, 0x00800080);
    }
}

int generate_ResetSM() {
    // Both channels must be reset state machine
    cmn_Debug("cmn_SetBits((uint32_t *) generate) mask 0x00400040 <- 0x%X", 0x00400040);
    cmn_SetBits((uint32_t*)generate, 0x00400040, 0x00400040);
    cmn_Debug("cmn_UnsetBits((uint32_t *) generate) mask 0x00400040 <- 0x%X", 0x00400040);
    cmn_UnsetBits((uint32_t*)generate, 0x00400040, 0x00400040);
    return RP_OK;
}

int generate_ResetChannelSM(rp_channel_t channel) {
    uint32_t value = channel == RP_CH_1 ? 0x00000040 : 0x00400000;
    cmn_Debug("cmn_SetBits((uint32_t *) generate) mask 0x00400040 <- 0x%X", value);
    cmn_SetBits((uint32_t*)generate, value, value);
    cmn_Debug("cmn_UnsetBits((uint32_t *) generate) mask 0x00400040 <- 0x%X", value);
    cmn_UnsetBits((uint32_t*)generate, value, value);
    return RP_OK;
}

int generate_writeData(rp_channel_t channel, float* data, int32_t start, uint32_t length) {

    uint8_t bits = 0;
    if (rp_HPGetFastDACBits(&bits) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC bits\n");
        return RP_NOTS;
    }

    bool is_sign = false;
    if (rp_HPGetFastDACIsSigned(&is_sign) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC sign value\n");
        return RP_NOTS;
    }

    volatile int32_t* dataOut = data_ch[channel];

    generate_setWrapCounter(channel, length);

    if (start < 0)
        start += DAC_BUFFER_SIZE;
    for (int i = start; i < start + DAC_BUFFER_SIZE; i++) {
        dataOut[i % DAC_BUFFER_SIZE] = cmn_convertToCnt(data[i - start], bits, 1.0, is_sign, 1, 0);
    }
    return RP_OK;
}

int generate_setAmplitude(rp_channel_t channel, rp_gen_gain_t gain, rp_gen_load_mode_t mode, float amplitude) {

    float fsBase = 0;
    if (rp_HPGetHWDACFullScale(&fsBase) != RP_HP_OK) {
        ERROR_LOG("Can't get fast HW DAC full scale");
        return RP_NOTS;
    }

    if (fsBase == 0) {
        ERROR_LOG("HW DAC Full Scale is zero");
        return RP_NOTS;
    }

    uint8_t bits = 0;
    if (rp_HPGetFastDACBits(&bits) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC bits");
        return RP_NOTS;
    }

    bool is_sign = false;
    if (rp_HPGetFastDACIsSigned(&is_sign) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC sign value");
        return RP_NOTS;
    }

    bool x5_gain = false;
    if (rp_HPGetIsGainDACx5(&x5_gain) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC x5 gain");
        return RP_NOTS;
    }

    if (!x5_gain && gain == RP_GAIN_5X) {
        ERROR_LOG("Can't set gain on unsupported board");
        return RP_NOTS;
    }

    rp_gen_load_calib_t l_mode = rp_gen_load_calib_t::RP_CALIB_HIZ;
    switch (mode) {
        case RP_GEN_50Ohm:
            l_mode = rp_gen_load_calib_t::RP_CALIB_50Ohm;
            break;
        case RP_GEN_HI_Z:
            l_mode = rp_gen_load_calib_t::RP_CALIB_HIZ;
            break;
        default:
            ERROR_LOG("Unknown load mode: %d", mode);
            return RP_EOOR;
            break;
    }

    double gain_calib;
    int32_t offset;
    int ret = 0;
    switch (gain) {
        case RP_GAIN_1X:
            ret = rp_CalibGetFastDACCalibValue(convertCh(channel), RP_GAIN_CALIB_1X, l_mode, &gain_calib, &offset);
            break;
        case RP_GAIN_5X:
            ret = rp_CalibGetFastDACCalibValue(convertCh(channel), RP_GAIN_CALIB_5X, l_mode, &gain_calib, &offset);
            break;
        default:
            ERROR_LOG("Unknown gain: %d", gain);
            return RP_EOOR;
            break;
    }

    if (ret != RP_HW_CALIB_OK) {
        ERROR_LOG("Get calibaration: %d", ret);
        return RP_EOOR;
    }

    //uint32_t amp_max = calib_getGenScale(channel,gain);

    //uint32_t value = cmn_CnvVToCnt(DATA_BIT_LENGTH, amplitude, AMPLITUDE_MAX , false, amp_max, 0, 0.0);
    // Spechal convert from Volt to RAW. 0x2000 = 1x
    int32_t value = cmn_convertToCnt(amplitude * gain_calib, bits, fsBase, is_sign, 1, 0);
    //  fprintf(stderr,"Gain %f amplitude  %f ofcalb %d res %d bits %d fsBase %f\n",gain_calib,amplitude,0,value,bits,fsBase);
    cmn_Debug("[Ch%d] ch_properties->amplitudeScale <- 0x%X", channel, value);

    asg_ch_amp_scale_u_t prop;

    if (channel == RP_CH_1) {
        prop.reg_full = generate->ampAndScale_ch1;
        prop.reg.amplitudeScale = value;
        generate->ampAndScale_ch1 = prop.reg_full;
    }

    if (channel == RP_CH_2) {
        prop.reg_full = generate->ampAndScale_ch2;
        prop.reg.amplitudeScale = value;
        generate->ampAndScale_ch2 = prop.reg_full;
    }

    return RP_OK;
}

int generate_setDCOffset(rp_channel_t channel, rp_gen_gain_t gain, rp_gen_load_mode_t mode, float offset) {

    float fsBase = 0;
    if (rp_HPGetHWDACFullScale(&fsBase) != RP_HP_OK) {
        ERROR_LOG("Can't get fast HW DAC full scale");
        return RP_NOTS;
    }

    if (fsBase == 0) {
        ERROR_LOG("HW DAC Full Scale is zero");
        return RP_NOTS;
    }

    uint8_t bits = 0;
    if (rp_HPGetFastDACBits(&bits) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC bits");
        return RP_NOTS;
    }

    bool is_sign = false;
    if (rp_HPGetFastDACIsSigned(&is_sign) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC sign value");
        return RP_NOTS;
    }

    bool x5_gain = false;
    if (rp_HPGetIsGainDACx5(&x5_gain) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC x5 gain");
        return RP_NOTS;
    }

    if (!x5_gain && gain == RP_GAIN_5X) {
        ERROR_LOG("Can't set gain on unsupported board");
        return RP_NOTS;
    }

    rp_gen_load_calib_t l_mode = rp_gen_load_calib_t::RP_CALIB_HIZ;
    switch (mode) {
        case RP_GEN_50Ohm:
            l_mode = rp_gen_load_calib_t::RP_CALIB_50Ohm;
            break;
        case RP_GEN_HI_Z:
            l_mode = rp_gen_load_calib_t::RP_CALIB_HIZ;
            break;
        default:
            ERROR_LOG("Unknown load mode: %d", mode);
            return RP_EOOR;
            break;
    }

    double gain_calib;
    int32_t offset_calib;
    int ret = 0;
    switch (gain) {
        case RP_GAIN_1X:
            ret = rp_CalibGetFastDACCalibValue(convertCh(channel), RP_GAIN_CALIB_1X, l_mode, &gain_calib, &offset_calib);
            break;
        case RP_GAIN_5X:
            ret = rp_CalibGetFastDACCalibValue(convertCh(channel), RP_GAIN_CALIB_5X, l_mode, &gain_calib, &offset_calib);
            break;
        default:
            ERROR_LOG("Unknown gain: %d", gain);
            return RP_EOOR;
            break;
    }

    if (ret != RP_HW_CALIB_OK) {
        ERROR_LOG("Get calibaration: %d", ret);
        return RP_EOOR;
    }

    int32_t value = cmn_convertToCnt(offset * gain_calib, bits, fsBase, is_sign, 1, offset_calib);
    //    fprintf(stderr,"Gain %f offset  %f ofcalb %d res %d bits %d fsBase %f\n",gain_calib,offset,offset_calib,value,bits,fsBase);
    cmn_Debug("[Ch%d] ch_properties->amplitudeOffset <- 0x%X", channel, value);

    asg_ch_amp_scale_u_t prop;

    if (channel == RP_CH_1) {
        prop.reg_full = generate->ampAndScale_ch1;
        prop.reg.amplitudeOffset = value;
        generate->ampAndScale_ch1 = prop.reg_full;
    }

    if (channel == RP_CH_2) {
        prop.reg_full = generate->ampAndScale_ch2;
        prop.reg.amplitudeOffset = value;
        generate->ampAndScale_ch2 = prop.reg_full;
    }

    return RP_OK;
}

int generate_setAmplitudeAndOffsetOrigin(rp_channel_t channel, rp_gen_gain_t gain, rp_gen_load_mode_t mode) {

    bool x5_gain = false;
    if (rp_HPGetIsGainDACx5(&x5_gain) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC x5 gain");
        return RP_NOTS;
    }

    if (!x5_gain && gain == RP_GAIN_5X) {
        ERROR_LOG("Can't set gain on unsupported board");
        return RP_NOTS;
    }

    rp_gen_load_calib_t l_mode = rp_gen_load_calib_t::RP_CALIB_HIZ;
    switch (mode) {
        case RP_GEN_50Ohm:
            l_mode = rp_gen_load_calib_t::RP_CALIB_50Ohm;
            break;
        case RP_GEN_HI_Z:
            l_mode = rp_gen_load_calib_t::RP_CALIB_HIZ;
            break;
        default:
            ERROR_LOG("Unknown load mode: %d", mode);
            return RP_EOOR;
            break;
    }

    double gain_calib;
    int32_t offset;
    int ret = 0;
    switch (gain) {
        case RP_GAIN_1X:
            ret = rp_CalibGetFastDACCalibValue(convertCh(channel), RP_GAIN_CALIB_1X, l_mode, &gain_calib, &offset);
            break;
        case RP_GAIN_5X:
            ret = rp_CalibGetFastDACCalibValue(convertCh(channel), RP_GAIN_CALIB_5X, l_mode, &gain_calib, &offset);
            break;
        default:
            ERROR_LOG("Unknown gain: %d", gain);
            return RP_EOOR;
            break;
    }

    if (ret != RP_HW_CALIB_OK) {
        ERROR_LOG("Get calibaration: %d", ret);
        return RP_EOOR;
    }

    asg_ch_amp_scale_u_t prop;

    if (channel == RP_CH_1) {
        prop.reg_full = generate->ampAndScale_ch1;

        int32_t value = 0x2000 * gain_calib;
        cmn_Debug("[Ch%d] ch_properties->amplitudeScale <- 0x%X", channel, value);
        prop.reg.amplitudeScale = value;
        cmn_Debug("[Ch%d] ch_properties->amplitudeOffset <- 0x%X", channel, offset);
        prop.reg.amplitudeOffset = offset;

        generate->ampAndScale_ch1 = prop.reg_full;
    }

    if (channel == RP_CH_2) {
        prop.reg_full = generate->ampAndScale_ch2;

        int32_t value = 0x2000 * gain_calib;
        cmn_Debug("[Ch%d] ch_properties->amplitudeScale <- 0x%X", channel, value);
        prop.reg.amplitudeScale = value;
        cmn_Debug("[Ch%d] ch_properties->amplitudeOffset <- 0x%X", channel, offset);
        prop.reg.amplitudeOffset = offset;

        generate->ampAndScale_ch2 = prop.reg_full;
    }

    return RP_OK;
}

int generate_getEnableTempProtection(rp_channel_t channel, bool* enable) {
    asg_config_control_u_t conf;
    conf.reg_full = generate->config;
    *enable = conf.reg[channel].tempProtected;
    return RP_OK;
}

int generate_setEnableTempProtection(rp_channel_t channel, bool enable) {
    asg_config_control_u_t conf;
    conf.reg_full = generate->config;
    conf.reg[channel].tempProtected = enable ? 1 : 0;
    generate->config = conf.reg_full;
    return RP_OK;
}

int generate_getLatchTempAlarm(rp_channel_t channel, bool* state) {
    asg_config_control_u_t conf;
    conf.reg_full = generate->config;
    *state = conf.reg[channel].latchedTempAlarm;
    return RP_OK;
}

int generate_setLatchTempAlarm(rp_channel_t channel, bool state) {
    asg_config_control_u_t conf;
    conf.reg_full = generate->config;
    conf.reg[channel].latchedTempAlarm = state ? 1 : 0;
    generate->config = conf.reg_full;
    return RP_OK;
}

int generate_getRuntimeTempAlarm(rp_channel_t channel, bool* state) {
    asg_config_control_u_t conf;
    conf.reg_full = generate->config;
    *state = conf.reg[channel].runtimeTempAlarm;
    return RP_OK;
}

int generate_setBurstLastValue(rp_channel_t channel, rp_gen_gain_t gain, rp_gen_load_mode_t mode, float amplitude) {

    float fullScaleAmp = 0;
    if (rp_HPGetFastDACOutFullScale((uint8_t)channel, &fullScaleAmp) != RP_HP_OK) {
        ERROR_LOG("Can't get fast HW DAC out full scale");
        return RP_NOTS;
    }

    float fsBase = 0;
    if (rp_HPGetHWDACFullScale(&fsBase) != RP_HP_OK) {
        ERROR_LOG("Can't get fast HW DAC full scale");
        return RP_NOTS;
    }

    if (fullScaleAmp == 0) {
        ERROR_LOG("HW DAC Out Full Scale is zero");
        return RP_NOTS;
    }

    if (fsBase == 0) {
        ERROR_LOG("HW DAC Full Scale is zero");
        return RP_NOTS;
    }

    amplitude = amplitude / fullScaleAmp * fsBase;

    uint8_t bits = 0;
    if (rp_HPGetFastDACBits(&bits) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC bits");
        return RP_NOTS;
    }

    bool is_sign = false;
    if (rp_HPGetFastDACIsSigned(&is_sign) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC sign value");
        return RP_NOTS;
    }

    /// !!! No calibration required, calibration occurs at the FPGA level
    uint32_t cnt = cmn_convertToCnt(amplitude, bits, fsBase, is_sign, 1.0, 0);
    cmn_Debug("[Ch%d] generate->BurstFinalValue_ch <- 0x%X", channel, cnt);
    CHANNEL_ACTION(channel, generate->BurstFinalValue_ch1 = cnt, generate->BurstFinalValue_ch2 = cnt)
    return RP_OK;
}

int generate_setInitGenValue(rp_channel_t channel, rp_gen_gain_t gain, rp_gen_load_mode_t mode, float amplitude) {

    float fullScaleAmp = 0;
    if (rp_HPGetFastDACOutFullScale((uint8_t)channel, &fullScaleAmp) != RP_HP_OK) {
        ERROR_LOG("Can't get fast HW DAC out full scale");
        return RP_NOTS;
    }

    float fsBase = 0;
    if (rp_HPGetHWDACFullScale(&fsBase) != RP_HP_OK) {
        ERROR_LOG("Can't get fast HW DAC full scale");
        return RP_NOTS;
    }

    if (fullScaleAmp == 0) {
        ERROR_LOG("HW DAC Out Full Scale is zero");
        return RP_NOTS;
    }

    if (fsBase == 0) {
        ERROR_LOG("HW DAC Full Scale is zero");
        return RP_NOTS;
    }

    amplitude = amplitude / fullScaleAmp * fsBase;

    uint8_t bits = 0;
    if (rp_HPGetFastDACBits(&bits) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC bits");
        return RP_NOTS;
    }

    bool is_sign = false;
    if (rp_HPGetFastDACIsSigned(&is_sign) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC sign value");
        return RP_NOTS;
    }

    /// !!! No calibration required, calibration occurs at the FPGA level
    uint32_t cnt = cmn_convertToCnt(amplitude, bits, fsBase, is_sign, 1.0, 0);
    cmn_Debug("[Ch%d] generate->initGenValue_ch <- 0x%X", channel, cnt);
    CHANNEL_ACTION(channel, generate->initGenValue_ch1 = cnt, generate->initGenValue_ch2 = cnt)
    return RP_OK;
}

int generate_SetTriggerDebouncer(uint32_t value) {
    if (GEN_DEBAUNCER_MASK < value) {
        cmn_Debug("[generate_SetTriggerDebouncer] Error: osc_reg.trig_dbc_t <- 0x%X", value);
        return RP_EIPV;
    }
    cmn_Debug("[generate_SetTriggerDebouncer] osc_reg.trig_dbc_t <- 0x%X", value);
    generate->trig_dbc = value;

    return RP_OK;
}

int generate_GetTriggerDebouncer(uint32_t* value) {
    *value = generate->trig_dbc;
    cmn_Debug("[generate_GetTriggerDebouncer] osc_reg.trig_dbc_t -> 0x%X", *value);
    return RP_OK;
}

int generate_setRandomSeed(rp_channel_t channel, uint32_t seed) {
    cmn_Debug("[Ch%d] generate->randomSeed_ch <- 0x%X", channel, seed);
    CHANNEL_ACTION(channel, generate->randomSeed_ch1 = seed, generate->randomSeed_ch2 = seed)
    return RP_OK;
}

int generate_getRandomSeed(rp_channel_t channel, uint32_t* seed) {
    uint32_t value;
    CHANNEL_ACTION(channel, value = generate->randomSeed_ch1, value = generate->randomSeed_ch2)
    *seed = value;
    return RP_OK;
}

int generate_setEnableRandom(rp_channel_t channel, bool enable) {
    cmn_Debug("[Ch%d] generate->enableNoise_ch <- 0x%X", channel, enable);
    CHANNEL_ACTION(channel, generate->enableNoise_ch1 = enable, generate->enableNoise_ch2 = enable)
    return RP_OK;
}

int generate_getEnableRandom(rp_channel_t channel, bool* enable) {
    bool value;
    CHANNEL_ACTION(channel, value = generate->enableNoise_ch1, value = generate->enableNoise_ch2)
    *enable = value;
    return RP_OK;
}

int generate_axi_SetEnable(rp_channel_t channel, bool enable) {
    cmn_Debug("[Ch%d] generate->enableAXI_Ch <- 0x%X", channel, enable);
    CHANNEL_ACTION(channel, generate->enableAXI_Ch1 = enable, generate->enableAXI_Ch2 = enable)
    return RP_OK;
}

int generate_axi_GetEnable(rp_channel_t channel, bool* enable) {
    bool value;
    CHANNEL_ACTION(channel, value = generate->enableAXI_Ch1, value = generate->enableAXI_Ch2)
    *enable = value;
    return RP_OK;
}

int generate_axi_SetStartAddress(rp_channel_t channel, uint32_t address) {
    cmn_Debug("[Ch%d] generate->axi_start_address_Ch <- 0x%X", channel, address);
    CHANNEL_ACTION(channel, generate->axi_start_address_Ch1 = address, generate->axi_start_address_Ch2 = address)
    return RP_OK;
}

int generate_axi_SetEndAddress(rp_channel_t channel, uint32_t address) {
    cmn_Debug("[Ch%d] generate->axi_end_address_Ch <- 0x%X", channel, address);
    CHANNEL_ACTION(channel, generate->axi_end_address_Ch1 = address, generate->axi_end_address_Ch2 = address)
    return RP_OK;
}

int generate_axi_SetDecimation(rp_channel_t channel, uint32_t decimation) {
    cmn_Debug("[Ch%d] generate->axi_decimation_Ch <- 0x%X", channel, decimation);
    CHANNEL_ACTION(channel, generate->axi_decimation_Ch1 = decimation, generate->axi_decimation_Ch2 = decimation)
    return RP_OK;
}

int generate_axi_GetDecimation(rp_channel_t channel, uint32_t* decimation) {
    bool value;
    CHANNEL_ACTION(channel, value = generate->axi_decimation_Ch1, value = generate->axi_decimation_Ch2)
    *decimation = value;
    return RP_OK;
}