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
#include "redpitaya/rp.h"
#include "common.h"
#include "generate.h"

static volatile generate_control_t *generate = NULL;
static volatile int32_t *data_ch[2] = {NULL, NULL};


int generate_Init() {
    cmn_Map(GENERATE_BASE_SIZE, GENERATE_BASE_ADDR, (void **) &generate);
    data_ch[0] = (int32_t *) ((char *) generate + (CHA_DATA_OFFSET));
    data_ch[1] = (int32_t *) ((char *) generate + (CHB_DATA_OFFSET));
    return RP_OK;
}

int generate_Release() {
    cmn_Unmap(GENERATE_BASE_SIZE, (void **) &generate);
    data_ch[0] = NULL;
    data_ch[1] = NULL;
    return RP_OK;
}

int getChannelPropertiesAddress(volatile ch_properties_t **ch_properties, rp_channel_t channel) {
    *ch_properties = &generate->properties_ch[channel];
    return RP_OK;
}

int rp_GenSetOutEnable (rp_channel_t channel, bool state) {
    if (channel == RP_CH_1) {
        iowrite32(state ? 0 : 1, &generate->AsetOutputTo0);
    } else if (channel == RP_CH_2) {
        iowrite32(state ? 0 : 1, &generate->BsetOutputTo0);
    } else {
        return RP_EPN;
    }
    return RP_OK;
}

int rp_GenGetOutEnable (rp_channel_t channel, bool *state) {
    if (channel == RP_CH_1) {
        *state = !ioread32(&generate->AsetOutputTo0);
    } else if (channel == RP_CH_2) {
        *state = !ioread32(&generate->BsetOutputTo0);
    } else {
        return RP_EPN;
    }
    return RP_OK;
}

int rp_GenReset() {
    rp_GenSetOutEnable (RP_CH_1, false);
    rp_GenSetOutEnable (RP_CH_2, false);
    gen_Disable(RP_CH_1);
    gen_Disable(RP_CH_2);
    rp_GenFreq(RP_CH_1, 1000);
    rp_GenFreq(RP_CH_2, 1000);
    gen_setBurstRepetitions(RP_CH_1, 1);
    gen_setBurstRepetitions(RP_CH_2, 1);
    gen_setBurstPeriod(RP_CH_1, (uint32_t) (1 / 1000.0 * MICRO));   // period = 1/frequency in us
    gen_setBurstPeriod(RP_CH_2, (uint32_t) (1 / 1000.0 * MICRO));   // period = 1/frequency in us
    gen_setWaveform(RP_CH_1, RP_WAVEFORM_SINE);
    gen_setWaveform(RP_CH_2, RP_WAVEFORM_SINE);
    rp_GenOffset(RP_CH_1, 0);
    rp_GenOffset(RP_CH_2, 0);
    rp_GenAmp(RP_CH_1, 1);
    rp_GenAmp(RP_CH_2, 1);
    gen_setGenMode(RP_CH_1, RP_GEN_MODE_CONTINUOUS);
    gen_setGenMode(RP_CH_2, RP_GEN_MODE_CONTINUOUS);
    gen_setBurstCount(RP_CH_1, 1);
    gen_setBurstCount(RP_CH_2, 1);
    gen_setBurstPeriod(RP_CH_1, BURST_PERIOD_MIN);
    gen_setBurstPeriod(RP_CH_2, BURST_PERIOD_MIN);
    gen_setTriggerSource(RP_CH_1, RP_GEN_TRIG_SRC_INTERNAL);
    gen_setTriggerSource(RP_CH_2, RP_GEN_TRIG_SRC_INTERNAL);
    return RP_OK;
}

int rp_GenSetAmp(rp_channel_t channel, float amplitude) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    ch_properties->amplitudeScale = cmn_CnvVToCnt(DATA_BIT_LENGTH, amplitude, AMPLITUDE_MAX, false);
    return RP_OK;
}

int rp_GenGetAmp(rp_channel_t channel, float *amplitude) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    *amplitude = cmn_CnvCntToV(DATA_BIT_LENGTH, ch_properties->amplitudeScale, AMPLITUDE_MAX);
    return RP_OK;
}

int rp_GenOffset(rp_channel_t channel, float offset) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    ch_properties->amplitudeOffset = cmn_CnvVToCnt(DATA_BIT_LENGTH, offset, (float) (OFFSET_MAX/2.f), false);
    return RP_OK;
}

int rp_GenGetOffset(rp_channel_t channel, float *offset) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    *offset = cmn_CnvCntToV(DATA_BIT_LENGTH, ch_properties->amplitudeOffset, (float) (OFFSET_MAX/2.f));
    return RP_OK;
}

int rp_GenFreq(rp_channel_t channel, float frequency) {
    if (frequency < FREQUENCY_MIN || frequency > FREQUENCY_MAX) {
        return RP_EOOR;
    }
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    ch_properties->counterStep = (uint32_t) ((1<<16) * frequency / DAC_FREQUENCY * BUFFER_LENGTH);
    channel == RP_CH_1 ? (generate->ASM_WrapPointer = 1) : (generate->BSM_WrapPointer = 1);
    return RP_OK;
}

int rp_GenGetFreq(rp_channel_t channel, float *frequency) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    *frequency = (float) (ch_properties->counterStep * DAC_FREQUENCY) / ((1<<16) * BUFFER_LENGTH);
    return RP_OK;
    return generate_getFrequency(channel, frequency);
}

int rp_GenPhase(rp_channel_t channel, float phase) {
    if (phase < PHASE_MIN || phase > PHASE_MAX) {
        return RP_EOOR;
    }
    // TODO
    return RP_OK;
}

int rp_GenGetPhase(rp_channel_t channel, float *phase) {
    // TODO
    return RP_OK;
}

int rp_GenSetWaveform(rp_channel_t channel, float *waveform, uint32_t length) {
    // Check if data is normalized, else saturate it
    float min =  1;
    float max = -1;
    for(int unsigned i = 0; i < length; i++) {
        if (data[i] < min)  min = data[i];
        if (data[i] > max)  max = data[i];
    }
    if (min < ARBITRARY_MIN || max > ARBITRARY_MAX) {
        return RP_ENN;
    }

    // Save data
    float *pointer;
    pointer = ch_arbitraryData[channel];
    for(i = 0; i < length; i++) {
        pointer[i] = data[i];
    }
    if (channel > RP_CH_2) {
        return RP_EPN;
    }
    volatile int32_t *dataOut;
    dataOut = data_ch[channel];

    volatile ch_properties_t *properties;
    getChannelPropertiesAddress(&properties, channel);
    generate->properties_ch[channel].counterWrap = (1<<16) * size - 1;
    for(int i = start; i < start+BUFFER_LENGTH; i++) {
        dataOut[i % BUFFER_LENGTH] = cmn_CnvVToCnt(DATA_BIT_LENGTH, data[i-start], AMPLITUDE_MAX, false);
    }
    return RP_OK;
}

int rp_GenGetWaveform(rp_channel_t channel, float *waveform, uint32_t *length) {
    // If this data was not set, then this method will return incorrect data
    float *pointer;
    if (channel > RP_CH_2) {
        return RP_EPN;
    }
    *length = ch_arb_size[channel];
    pointer = ch_arbitraryData[channel];
    for (int i = 0; i < *length; ++i) {
        data[i] = pointer[i];
    }
    return RP_OK;
}

int rp_GenMode(rp_channel_t channel, rp_gen_mode_t mode) {
    if (mode == RP_GEN_MODE_CONTINUOUS) {
        generate_setGatedBurst(channel, 0);
        generate_setBurstDelay(channel, 0);
        generate_setBurstRepetitions(channel, 0);
        generate_setBurstCount(channel, 0);
    } else if (mode == RP_GEN_MODE_BURST) {
        gen_setBurstCount(channel, ch_burstCount[channel]);
        gen_setBurstRepetitions(channel, ch_burstRepetition[channel]);
        gen_setBurstPeriod(channel, ch_burstPeriod[channel]);
        return RP_OK;
    } else if (mode == RP_GEN_MODE_STREAM) {
        return RP_EUF;
    } else {
        return RP_EIPV;
    }
}

int rp_GenGetMode(rp_channel_t channel, rp_gen_mode_t *mode) {
    uint32_t num;
    generate_getBurstCount(channel, &num);
    if (num != 0) {
        *mode = RP_GEN_MODE_BURST;
    } else {
        *mode = RP_GEN_MODE_CONTINUOUS;
    }
    return RP_OK;
}

int rp_GenBurstCount(rp_channel_t channel, int num) {
    if ((num < BURST_COUNT_MIN || num > BURST_COUNT_MAX) && num == 0) {
        return RP_EOOR;
    }
    ch_burstCount[channel] = num;
    if (num == -1) {    // -1 represents infinity. In FPGA value 0 represents infinity
        num = 0;
    }
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    ch_properties->cyclesInOneBurst = num;
    return RP_OK;
}

int rp_GenGetBurstCount(rp_channel_t channel, int *num) {
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    *num = ch_properties->cyclesInOneBurst;
    return RP_OK;
}

int rp_GenBurstRepetitions(rp_channel_t channel, int repetitions) {
    if ((repetitions < BURST_REPETITIONS_MIN || repetitions > BURST_REPETITIONS_MAX) && repetitions != -1) {
        return RP_EOOR;
    }
    ch_burstRepetition[channel] = repetitions;
    if (repetitions == -1) {
        repetitions = 0;
    }
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    ch_properties->burstRepetitions = repetitions - 1;
    return RP_OK;
}

int rp_GenGetBurstRepetitions(rp_channel_t channel, int *repetitions) {
    uint32_t tmp;
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    *repetitions = ch_properties->burstRepetitions;
    *repetitions += 1;
    return RP_OK;
}

int rp_GenBurstPeriod(rp_channel_t channel, uint32_t period) {
    if (period < BURST_PERIOD_MIN || period > BURST_PERIOD_MAX) {
        return RP_EOOR;
    }
    int burstCount;
    burstCount = ch_burstCount[channel];
    // period = signal_time * burst_count + delay_time
    int delay = (int) (period - (1 / ch_frequency[channel] * MICRO) * burstCount);
    if (delay <= 0) {
        // if delay is 0, then FPGA generates continuous signal
        delay = 1;
    }
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    ch_properties->delayBetweenBurstRepetitions = delay;
    return RP_OK;
}

int rp_GenGetBurstPeriod(rp_channel_t channel, uint32_t *period) {
    uint32_t delay, burstCount;
    float frequency;
    volatile ch_properties_t *ch_properties;
    getChannelPropertiesAddress(&ch_properties, channel);
    *delay = ch_properties->delayBetweenBurstRepetitions;
    return RP_OK;
    generate_getBurstCount(channel, &burstCount);
    rp_GenGetFreq(channel, &frequency);

    if (delay == 1) {    // if delay is 0, then FPGA generates continuous signal
        delay = 0;
    }
    *period = (uint32_t) (delay + (1 / frequency * MICRO) * burstCount);
    return RP_OK;
}

int rp_GenTriggerSource(rp_channel_t channel, rp_trig_src_t src) {
    if (src == RP_GEN_TRIG_SRC_INTERNAL) {
        gen_setGenMode(channel, RP_GEN_MODE_CONTINUOUS);
        return generate_setTriggerSource(channel, 1);
    }
    else if (src == RP_GEN_TRIG_SRC_EXT_PE) {
        gen_setGenMode(channel, RP_GEN_MODE_BURST);
        return generate_setTriggerSource(channel, 2);
    }
    else if (src == RP_GEN_TRIG_SRC_EXT_NE) {
        gen_setGenMode(channel, RP_GEN_MODE_BURST);
        return generate_setTriggerSource(channel, 3);
    }
    else {
        return RP_EIPV;
    }
}
int generate_setTriggerSource(rp_channel_t channel, unsigned short value) {
    CHANNEL_ACTION(channel,
            generate->AtriggerSelector = value,
            generate->BtriggerSelector = value)
    return RP_OK;
}

int rp_GenGetTriggerSource(rp_channel_t channel, rp_trig_src_t *src) {
    CHANNEL_ACTION(channel,
            *value = generate->AtriggerSelector,
            *value = generate->BtriggerSelector)
    return RP_OK;
}

int rp_GenTrigger(int mask) {
    switch (mask) {
        case 1:
            gen_setGenMode(RP_CH_1, RP_GEN_MODE_BURST);
            return generate_setTriggerSource(RP_CH_1, RP_GEN_TRIG_SRC_INTERNAL);
        case 2:
            gen_setGenMode(RP_CH_2, RP_GEN_MODE_BURST);
            return generate_setTriggerSource(RP_CH_2, RP_GEN_TRIG_SRC_INTERNAL);
        case 3:
            gen_setGenMode(RP_CH_1, RP_GEN_MODE_BURST);
            gen_setGenMode(RP_CH_2, RP_GEN_MODE_BURST);
            return generate_simultaneousTrigger();
            // simultaneously trigger both channels
            return cmn_SetBits((uint32_t *) generate, 0x00010001, 0xFFFFFFFF);
        default:
            return RP_EOOR;
    }
}

