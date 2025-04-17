/**
* $Id: $
*
* @brief Red Pitaya library Generate handler interface
*
* @Author Red Pitaya
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#include "gen_handler.h"
#include <float.h>
#include <time.h>
#include "axi_manager.h"
#include "common.h"
#include "convert.hpp"
#include "generate.h"
#include "math.h"
#include "rp-i2c-max7311-c.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define CHECK_CHANNEL                                                                       \
    uint8_t channels_rp_HPGetFastDACChannelsCount = 0;                                      \
    if (rp_HPGetFastDACChannelsCount(&channels_rp_HPGetFastDACChannelsCount) != RP_HP_OK) { \
        ERROR_LOG("Can't get fast DAC channels count");                                     \
        return RP_NOTS;                                                                     \
    }                                                                                       \
    if (channel >= channels_rp_HPGetFastDACChannelsCount) {                                 \
        ERROR_LOG("Channel is larger than allowed");                                        \
        return RP_NOTS;                                                                     \
    }

float ch_amplitude[2] = {1, 1};
float ch_offset[2] = {0, 0};
float ch_dutyCycle[2] = {0, 0};
float ch_riseTime[2] = {1, 1};
float ch_fallTime[2] = {1, 1};
float ch_riseFallMin[2] = {0.1, 0.1};
float ch_riseFallMax[2] = {1000, 1000};
float ch_frequency[2];
float ch_sweepStartFrequency[2];
float ch_sweepEndFrequency[2];
float ch_phase[2] = {0, 0};
int ch_burstCount[2] = {1, 1};
int ch_burstRepetition[2] = {1, 1};
uint32_t ch_burstPeriod[2] = {0, 0};
float ch_burstLastValue[2] = {0, 0};
float ch_initValue[2] = {0, 0};

rp_waveform_t ch_waveform[2];
rp_gen_sweep_mode_t ch_sweepMode[2];
rp_gen_sweep_dir_t ch_sweepDir[2];
uint32_t ch_size[2] = {DAC_BUFFER_SIZE, DAC_BUFFER_SIZE};
uint32_t ch_arb_size[2] = {DAC_BUFFER_SIZE, DAC_BUFFER_SIZE};
rp_gen_mode_t ch_mode[2] = {RP_GEN_MODE_CONTINUOUS, RP_GEN_MODE_CONTINUOUS};
rp_gen_load_mode_t ch_load_mode[2] = {RP_GEN_HI_Z, RP_GEN_HI_Z};

bool ch_EnableTempProtection[2] = {0, 0};
bool ch_LatchTempAlarm[2] = {0, 0};

rp_gen_gain_t ch_gain[2] = {RP_GAIN_1X, RP_GAIN_1X};

float ch_arbitraryData[2][DAC_BUFFER_SIZE];
uint64_t asg_axi_mem_reserved_index[4] = {0, 0, 0, 0};
uint32_t asg_axi_reserved_samples_count[4] = {0, 0, 0, 0};

int gen_SetDefaultValues() {

    uint8_t channels = 0;
    if (rp_HPGetFastDACChannelsCount(&channels) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC channels count");
        return RP_NOTS;
    }

    bool x5_gain = false;
    if (rp_HPGetIsGainDACx5(&x5_gain) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC gain mode");
        return RP_NOTS;
    }

    for (int ch_i = 0; ch_i < channels; ch_i++) {
        rp_channel_t ch = convertChFromIndex(ch_i);
        gen_axi_SetEnable(ch, false);
        gen_Disable(ch);
        gen_setFrequency(ch, 1000);
        gen_setRiseFallMin(ch, 0.1);
        gen_setRiseFallMax(ch, 1000);
        gen_setRiseTime(ch, 1);
        gen_setFallTime(ch, 1);
        gen_setSweepStartFrequency(ch, 1000);
        gen_setSweepEndFrequency(ch, 1000);
        gen_setBurstRepetitions(ch, 1);
        gen_setBurstPeriod(ch, (uint32_t)(1 / 1000.0 * MICRO));  // period = 1/frequency in us
        gen_setWaveform(ch, RP_WAVEFORM_SINE);
        gen_setSweepMode(ch, RP_GEN_SWEEP_MODE_LINEAR);
        gen_setSweepDir(ch, RP_GEN_SWEEP_DIR_NORMAL);
        gen_setLoadMode(ch, RP_GEN_HI_Z);
        gen_setOffset(ch, 0);

        float fs = 0;
        if (rp_HPGetFastDACOutFullScale(convertCh(ch), &fs) != RP_HP_OK) {
            ERROR_LOG("Can't get fast DAC out full scale");
            return RP_NOTS;
        }

        gen_setAmplitude(ch, fs * 0.8);
        gen_setDutyCycle(ch, 0.5);
        gen_setBurstCount(ch, 1);
        gen_setBurstPeriod(ch, 1);
        gen_setBurstLastValue(ch, 0);
        gen_setTriggerSource(ch, RP_GEN_TRIG_SRC_INTERNAL);
        gen_setPhase(ch, 0.0);
        gen_setGenMode(ch, RP_GEN_MODE_CONTINUOUS);
        gen_setInitGenValue(ch, 0);
        if (x5_gain)
            gen_setGainOut(ch, RP_GAIN_1X);
    }

    generate_ResetSM();
    return RP_OK;
}

int gen_GetDACSamplePeriod(double* value) {
    *value = 0;
    uint32_t speed = 0;
    int ret = rp_HPGetBaseFastDACSpeedHz(&speed);
    if (ret == RP_HP_OK) {
        *value = (double)1e9 / speed;
    }
    return ret;
}

int gen_Disable(rp_channel_t channel) {

    CHECK_CHANNEL

    return generate_setOutputDisable(channel, true);
}

int gen_Enable(rp_channel_t channel) {

    CHECK_CHANNEL

    return generate_setOutputDisable(channel, false);
}

int gen_EnableSync(bool enable) {
    return generate_setOutputEnableSync(enable);
}

int gen_IsEnable(rp_channel_t channel, bool* value) {
    return generate_getOutputEnabled(channel, value);
}

int gen_checkAmplitudeAndOffset(rp_channel_t channel, float amplitude, float offset) {
    float fs = 0;
    if (rp_HPGetFastDACOutFullScale(convertCh(channel), &fs) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC out full scale");
        return RP_NOTS;
    }

    if ((int)((fabs(amplitude) + fabs(offset)) * 100000) > (int)(fs * 100000)) {
        return RP_EOOR;
    }
    return RP_OK;
}

int gen_setAmplitudeAndOffsetOrigin(rp_channel_t channel) {

    CHECK_CHANNEL

    return generate_setAmplitudeAndOffsetOrigin(channel, ch_gain[channel]);
}

int gen_setAmplitude(rp_channel_t channel, float amplitude) {

    CHECK_CHANNEL

    float koff = ch_load_mode[channel] == RP_GEN_50Ohm ? 2.0 : 1.0;

    if (gen_checkAmplitudeAndOffset(channel, amplitude * koff, ch_offset[channel] * koff) != RP_OK) {
        return RP_EOOR;
    }

    ch_amplitude[channel] = amplitude;
    return generate_setAmplitude(channel, ch_gain[channel], amplitude * koff);
}

int gen_getAmplitude(rp_channel_t channel, float* amplitude) {

    CHECK_CHANNEL

    *amplitude = ch_amplitude[channel];

    return RP_OK;
}

int gen_setOffset(rp_channel_t channel, float offset) {

    CHECK_CHANNEL

    float koff = ch_load_mode[channel] == RP_GEN_50Ohm ? 2.0 : 1.0;

    if (gen_checkAmplitudeAndOffset(channel, ch_amplitude[channel] * koff, offset * koff) != RP_OK) {
        return RP_EOOR;
    }

    ch_offset[channel] = offset;

    return generate_setDCOffset(channel, ch_gain[channel], offset * koff);
}

int gen_getOffset(rp_channel_t channel, float* offset) {

    CHECK_CHANNEL

    *offset = ch_offset[channel];

    return RP_OK;
}

int gen_setRiseFallMin(rp_channel_t channel, float min) {

    CHECK_CHANNEL

    ch_riseFallMin[channel] = min;
    if (ch_riseTime[channel] < ch_riseFallMin[channel]) {
        ch_riseTime[channel] = ch_riseFallMin[channel];
    }
    if (ch_fallTime[channel] < ch_riseFallMin[channel]) {
        ch_fallTime[channel] = ch_riseFallMin[channel];
    }
    return RP_OK;
}

int gen_getRiseFallMin(rp_channel_t channel, float* min) {

    CHECK_CHANNEL

    *min = ch_riseFallMin[channel];
    return RP_OK;
}

int gen_setRiseFallMax(rp_channel_t channel, float max) {

    CHECK_CHANNEL

    ch_riseFallMax[channel] = max;
    if (ch_riseTime[channel] > ch_riseFallMax[channel]) {
        ch_riseTime[channel] = ch_riseFallMax[channel];
    }
    if (ch_fallTime[channel] > ch_riseFallMax[channel]) {
        ch_fallTime[channel] = ch_riseFallMax[channel];
    }

    return RP_OK;
}

int gen_getRiseFallMax(rp_channel_t channel, float* max) {

    CHECK_CHANNEL

    *max = ch_riseFallMax[channel];
    return RP_OK;
}

int gen_setFrequency(rp_channel_t channel, float frequency) {

    CHECK_CHANNEL

    uint32_t base_freq = 0;
    if (rp_HPGetBaseFastDACSpeedHz(&base_freq) != RP_HP_OK) {
        ERROR_LOG("Can't get fast ADC base rate");
        return RP_NOTS;
    }

    if (frequency < 0 || frequency > base_freq / 2) {
        return RP_EOOR;
    }

    ch_frequency[channel] = frequency;
    gen_setBurstPeriod(channel, ch_burstPeriod[channel]);
    gen_setRiseFallMin(channel, 1000000.0 / frequency * RISE_FALL_MIN_RATIO);
    gen_setRiseFallMax(channel, 1000000.0 / frequency * RISE_FALL_MAX_RATIO);

    generate_setFrequency(channel, frequency, base_freq);
    return synthesize_signal(channel);
}

int gen_setFrequencyDirect(rp_channel_t channel, float frequency) {

    CHECK_CHANNEL

    uint32_t base_freq = 0;
    if (rp_HPGetBaseFastDACSpeedHz(&base_freq) != RP_HP_OK) {
        ERROR_LOG("Can't get fast ADC base rate");
        return RP_NOTS;
    }

    if (frequency < 0 || frequency > base_freq / 2) {
        return RP_EOOR;
    }

    ch_frequency[channel] = frequency;

    gen_setRiseFallMin(channel, 1000000.0 / frequency * RISE_FALL_MIN_RATIO);
    gen_setRiseFallMax(channel, 1000000.0 / frequency * RISE_FALL_MAX_RATIO);

    return generate_setFrequency(channel, frequency, base_freq);
}

int gen_getFrequency(rp_channel_t channel, float* frequency) {

    CHECK_CHANNEL

    // uint32_t base_freq = 0;
    // if (rp_HPGetBaseFastDACSpeedHz(&base_freq) != RP_HP_OK){
    //     ERROR_LOG("Can't get fast ADC base rate");
    //     return RP_NOTS;
    // }

    *frequency = ch_frequency[channel];
    return RP_OK;
    // return generate_getFrequency(channel, frequency,base_freq);
}

int gen_setSweepStartFrequency(rp_channel_t channel, float frequency) {

    CHECK_CHANNEL

    uint32_t base_freq = 0;
    if (rp_HPGetBaseFastDACSpeedHz(&base_freq) != RP_HP_OK) {
        ERROR_LOG("Can't get fast ADC base rate");
        return RP_NOTS;
    }

    if (frequency < 0 || frequency > base_freq / 2) {
        return RP_EOOR;
    }
    ch_sweepStartFrequency[channel] = frequency;
    return synthesize_signal(channel);
}

int gen_getSweepStartFrequency(rp_channel_t channel, float* frequency) {

    CHECK_CHANNEL

    *frequency = ch_sweepStartFrequency[channel];
    return RP_OK;
}

int gen_setSweepEndFrequency(rp_channel_t channel, float frequency) {

    CHECK_CHANNEL

    uint32_t base_freq = 0;
    if (rp_HPGetBaseFastDACSpeedHz(&base_freq) != RP_HP_OK) {
        ERROR_LOG("Can't get fast ADC base rate");
        return RP_NOTS;
    }

    if (frequency < 0 || frequency > base_freq / 2) {
        return RP_EOOR;
    }

    ch_sweepEndFrequency[channel] = frequency;
    return synthesize_signal(channel);
}

int gen_getSweepEndFrequency(rp_channel_t channel, float* frequency) {

    CHECK_CHANNEL

    *frequency = ch_sweepEndFrequency[channel];
    return RP_OK;
}

int gen_setPhase(rp_channel_t channel, float phase) {

    CHECK_CHANNEL

    if (phase < PHASE_MIN || phase > PHASE_MAX) {
        return RP_EOOR;
    }
    ch_phase[channel] = phase;
    return synthesize_signal(channel);
}

int gen_getPhase(rp_channel_t channel, float* phase) {

    CHECK_CHANNEL

    *phase = ch_phase[channel];
    return RP_OK;
}

int gen_setWaveform(rp_channel_t channel, rp_waveform_t type) {

    CHECK_CHANNEL

    ch_waveform[channel] = type;
    if (type == RP_WAVEFORM_ARBITRARY) {
        ch_size[channel] = ch_arb_size[channel];
    } else {
        ch_size[channel] = DAC_BUFFER_SIZE;
    }
    if (type != RP_WAVEFORM_NOISE) {
        generate_setEnableRandom(channel, false);
        return synthesize_signal(channel);
    } else {
        clock_t start = clock();
        generate_setRandomSeed(channel, (uint32_t)start);
        generate_setEnableRandom(channel, true);
        return RP_OK;
    }
}

int gen_getWaveform(rp_channel_t channel, rp_waveform_t* type) {

    CHECK_CHANNEL

    *type = ch_waveform[channel];
    return RP_OK;
}

int gen_setSweepMode(rp_channel_t channel, rp_gen_sweep_mode_t mode) {

    CHECK_CHANNEL

    ch_sweepMode[channel] = mode;

    return synthesize_signal(channel);
}

int gen_getSweepMode(rp_channel_t channel, rp_gen_sweep_mode_t* mode) {

    CHECK_CHANNEL

    *mode = ch_sweepMode[channel];
    return RP_OK;
}

int gen_setSweepDir(rp_channel_t channel, rp_gen_sweep_dir_t mode) {
    CHECK_CHANNEL

    ch_sweepDir[channel] = mode;
    return synthesize_signal(channel);
}

int gen_getSweepDir(rp_channel_t channel, rp_gen_sweep_dir_t* mode) {

    CHECK_CHANNEL

    *mode = ch_sweepDir[channel];
    return RP_OK;
}

int gen_setArbWaveform(rp_channel_t channel, float* data, uint32_t length) {

    CHECK_CHANNEL

    float fs = 0;
    if (rp_HPGetFastDACOutFullScale(channel, &fs) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC out full scale");
        return RP_NOTS;
    }

    bool is_sign = false;
    if (rp_HPGetFastDACIsSigned(&is_sign) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC sign value");
        return RP_NOTS;
    }

    // Check if data is normalized
    float min = FLT_MAX, max = -FLT_MAX;  // initial values
    uint32_t i;
    for (i = 0; i < length; i++) {
        if (data[i] < min)
            min = data[i];
        if (data[i] > max)
            max = data[i];
    }
    if (min < (is_sign ? -fs : 0) || max > fs) {
        ERROR_LOG("The signal is greater than acceptable.");
        return RP_ENN;
    }

    // Save data
    float* pointer = ch_arbitraryData[channel];

    for (i = 0; i < length; i++) {
        pointer[i] = data[i];
    }

    for (i = length; i < DAC_BUFFER_SIZE; i++) {  // clear the rest of the buffer
        pointer[i] = 0;
    }

    ch_arb_size[channel] = length;
    if (ch_waveform[channel] == RP_WAVEFORM_ARBITRARY) {
        return synthesize_signal(channel);
    }

    return RP_OK;
}

int gen_getArbWaveform(rp_channel_t channel, float* data, uint32_t* length) {

    CHECK_CHANNEL

    // If this data was not set, then this method will return incorrect data
    float* pointer = ch_arbitraryData[channel];
    *length = ch_arb_size[channel];

    for (uint32_t i = 0; i < *length; ++i) {
        data[i] = pointer[i];
    }
    return RP_OK;
}

int gen_setDutyCycle(rp_channel_t channel, float ratio) {

    CHECK_CHANNEL

    if (ratio < DUTY_CYCLE_MIN || ratio > DUTY_CYCLE_MAX) {
        return RP_EOOR;
    }

    ch_dutyCycle[channel] = ratio;
    return synthesize_signal(channel);
}

int gen_getDutyCycle(rp_channel_t channel, float* ratio) {

    CHECK_CHANNEL

    *ratio = ch_dutyCycle[channel];
    return RP_OK;
}

int gen_setRiseTime(rp_channel_t channel, float time) {

    CHECK_CHANNEL

    if (time < ch_riseFallMin[channel]) {
        time = ch_riseFallMin[channel];
    }
    if (time > ch_riseFallMax[channel]) {
        time = ch_riseFallMax[channel];
    }

    ch_riseTime[channel] = time;
    return synthesize_signal(channel);
}

int gen_getRiseTime(rp_channel_t channel, float* time) {

    CHECK_CHANNEL

    *time = ch_riseTime[channel];
    return RP_OK;
}

int gen_setFallTime(rp_channel_t channel, float time) {

    CHECK_CHANNEL

    if (time < ch_riseFallMin[channel]) {
        time = ch_riseFallMin[channel];
    }
    if (time > ch_riseFallMax[channel]) {
        time = ch_riseFallMax[channel];
    }

    ch_fallTime[channel] = time;
    return synthesize_signal(channel);
}

int gen_getFallTime(rp_channel_t channel, float* time) {

    CHECK_CHANNEL

    *time = ch_fallTime[channel];
    return RP_OK;
}

int gen_setGenMode(rp_channel_t channel, rp_gen_mode_t mode) {

    CHECK_CHANNEL

    ch_mode[channel] = mode;

    if (mode == RP_GEN_MODE_CONTINUOUS) {
        generate_setGatedBurst(channel, 0);
        generate_setBurstDelay(channel, 0);
        generate_setBurstRepetitions(channel, 0);
        generate_setBurstCount(channel, 0);
        return RP_OK;
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

int gen_getGenMode(rp_channel_t channel, rp_gen_mode_t* mode) {

    CHECK_CHANNEL

    *mode = ch_mode[channel];
    return RP_OK;
}

int gen_setBurstCount(rp_channel_t channel, int num) {

    CHECK_CHANNEL

    rp_gen_mode_t mode = ch_mode[channel];

    if (num < BURST_COUNT_MIN || num > BURST_COUNT_MAX) {
        return RP_EOOR;
    }

    ch_burstCount[channel] = num;
    gen_setBurstPeriod(channel, ch_burstPeriod[channel]);
    if (mode == RP_GEN_MODE_BURST) {
        int ret = generate_setBurstCount(channel, (uint32_t)num);
        return ret;
    }
    return RP_OK;
}

int gen_getBurstCount(rp_channel_t channel, int* num) {

    CHECK_CHANNEL

    *num = ch_burstCount[channel];
    return RP_OK;
}

int gen_setBurstLastValue(rp_channel_t channel, float amplitude) {

    CHECK_CHANNEL

    int ret = generate_setBurstLastValue(channel, ch_gain[channel], amplitude);
    if (ret == RP_OK) {
        ch_burstLastValue[channel] = amplitude;
    }
    return ret;
}

int gen_getBurstLastValue(rp_channel_t channel, float* amplitude) {

    CHECK_CHANNEL

    *amplitude = ch_burstLastValue[channel];
    return RP_OK;
}

int gen_setInitGenValue(rp_channel_t channel, float amplitude) {

    CHECK_CHANNEL

    int ret = generate_setInitGenValue(channel, ch_gain[channel], amplitude);
    if (ret == RP_OK) {
        ch_initValue[channel] = amplitude;
    }
    return ret;
}

int gen_getInitGenValue(rp_channel_t channel, float* amplitude) {

    CHECK_CHANNEL

    *amplitude = ch_initValue[channel];
    return RP_OK;
}

int gen_setBurstRepetitions(rp_channel_t channel, int repetitions) {

    CHECK_CHANNEL

    rp_gen_mode_t mode = ch_mode[channel];

    if (repetitions < BURST_REPETITIONS_MIN || repetitions > BURST_REPETITIONS_MAX) {
        return RP_EOOR;
    }

    ch_burstRepetition[channel] = repetitions;
    if (mode == RP_GEN_MODE_BURST) {
        int ret = generate_setBurstRepetitions(channel, (uint32_t)(repetitions - 1));
        return ret;
    }
    return RP_OK;
}

int gen_getBurstRepetitions(rp_channel_t channel, int* repetitions) {

    CHECK_CHANNEL

    *repetitions = ch_burstRepetition[channel];
    return RP_OK;
}

int gen_setBurstPeriod(rp_channel_t channel, uint32_t period) {

    CHECK_CHANNEL

    bool axi_enable = false;
    uint32_t axi_decimation = 0;
    generate_axi_GetEnable(channel, &axi_enable);
    generate_axi_GetDecimation(channel, &axi_decimation);

    rp_gen_mode_t mode = ch_mode[channel];

    if (period < BURST_PERIOD_MIN || period > BURST_PERIOD_MAX) {
        return RP_EOOR;
    }

    if (axi_decimation == 0 && axi_enable) {
        ERROR_LOG("Decimation is not set for AXI mode")
        return RP_EOOR;
    }

    if (asg_axi_reserved_samples_count[channel] == 0 && axi_enable) {
        ERROR_LOG("Memory size not set for AXI mode")
        return RP_EOOR;
    }

    // For non-axi mode, the buffer length is taken as 1. Since the calculation is carried out through the frequency of the signal, which is generated in 1 time.
    int samplesCount = axi_enable ? asg_axi_reserved_samples_count[channel] : 1;
    double freq = axi_enable ? rp_HPGetBaseFastDACSpeedHzOrDefault() / axi_decimation : ch_frequency[channel];

    int burstCount = ch_burstCount[channel];
    int delay = 0;

    int sigLen = (((double)samplesCount) / freq) * burstCount * MICRO;
    // period = signal_time * burst_count + delay_time
    if ((int)period - sigLen <= 0) {
        period = sigLen + 1;
    }

    delay = period - sigLen;

    TRACE("freq %f sigLen %d burstCount %d period %d delay %d", freq, sigLen, burstCount, period, delay)

    ch_burstPeriod[channel] = period;

    if (mode == RP_GEN_MODE_BURST) {
        int ret = generate_setBurstDelay(channel, (uint32_t)delay);
        return ret;
    }
    return RP_OK;
}

int gen_getBurstPeriod(rp_channel_t channel, uint32_t* period) {

    CHECK_CHANNEL

    *period = ch_burstPeriod[channel];
    return RP_OK;
}

int gen_setTriggerSource(rp_channel_t channel, rp_trig_src_t src) {

    CHECK_CHANNEL

    // if (src == RP_GEN_TRIG_GATED_BURST) {
    //     generate_setGatedBurst(channel, 1);
    //     return generate_setGatedBurst(channel, 2);
    // }
    // else {
    //     generate_setGatedBurst(channel, 0);
    // }

    if (src == RP_GEN_TRIG_SRC_INTERNAL) {
        return generate_setTriggerSource(channel, 1);
    } else if (src == RP_GEN_TRIG_SRC_EXT_PE) {
        return generate_setTriggerSource(channel, 2);
    } else if (src == RP_GEN_TRIG_SRC_EXT_NE) {
        return generate_setTriggerSource(channel, 3);
    } else {
        return RP_EIPV;
    }
}

int gen_getTriggerSource(rp_channel_t channel, rp_trig_src_t* src) {
    uint32_t val = 0;
    int ret = generate_getTriggerSource(channel, &val);
    *src = (rp_trig_src_t)val;
    return ret;
}

int gen_Trigger(uint32_t channel) {

    switch (channel) {
        case RP_CH_1:
        case RP_CH_2:
            gen_ResetChannelSM((rp_channel_t)channel);
            return generate_Trigger((rp_channel_t)channel);

        default:
            return RP_EOOR;
    }
}

int gen_TriggerOnly(uint32_t channel) {

    switch (channel) {
        case RP_CH_1:
        case RP_CH_2:
            return generate_Trigger((rp_channel_t)channel);

        default:
            return RP_EOOR;
    }
}

int gen_TriggerOnlyBoth() {
    return generate_TriggerBoth();
}

int gen_TriggerSync() {
    generate_ResetSM();
    return generate_simultaneousTrigger();
}

int gen_ResetChannelSM(rp_channel_t channel) {
    return generate_ResetChannelSM(channel);
}

int gen_SynchroniseSM() {
    return generate_ResetSM();
}

int synthesize_signal(rp_channel_t channel) {

    CHECK_CHANNEL

    float data[DAC_BUFFER_SIZE];

    rp_waveform_t waveform = ch_waveform[channel];
    float dutyCycle = ch_dutyCycle[channel];
    float frequency = ch_frequency[channel];
    float riseTime = ch_riseTime[channel];
    float fallTime = ch_fallTime[channel];
    float sweepStartFreq = ch_sweepStartFrequency[channel];
    float sweepEndFreq = ch_sweepEndFrequency[channel];
    rp_gen_sweep_mode_t sweep_mode = ch_sweepMode[channel];
    rp_gen_sweep_dir_t sweep_dir = ch_sweepDir[channel];
    uint32_t size = ch_size[channel];
    int32_t phase = (ch_phase[channel] * DAC_BUFFER_SIZE / 360.0);
    float phaseRad = ch_phase[channel] / 180.0 * M_PI;

    uint16_t buf_size = DAC_BUFFER_SIZE;
    if (waveform == RP_WAVEFORM_SWEEP)
        phase = 0;

    float scale = 1;

    switch (waveform) {
        case RP_WAVEFORM_SINE:
            synthesis_sin(scale, data, buf_size);
            break;
        case RP_WAVEFORM_TRIANGLE:
            synthesis_triangle(scale, data, buf_size);
            break;
        case RP_WAVEFORM_SQUARE:
            synthesis_square(scale, frequency, riseTime, fallTime, data, buf_size);
            break;
        case RP_WAVEFORM_RAMP_UP:
            synthesis_rampUp(scale, data, buf_size);
            break;
        case RP_WAVEFORM_RAMP_DOWN:
            synthesis_rampDown(scale, data, buf_size);
            break;
        case RP_WAVEFORM_DC:
            synthesis_DC(scale, data, buf_size);
            break;
        case RP_WAVEFORM_DC_NEG:
            synthesis_DC_NEG(scale, data, buf_size);
            break;
        case RP_WAVEFORM_PWM:
            synthesis_PWM(scale, dutyCycle, data, buf_size);
            break;
        case RP_WAVEFORM_ARBITRARY:
            synthesis_arbitrary(scale, channel, data, &size);
            break;
        case RP_WAVEFORM_SWEEP:
            synthesis_sweep(scale, frequency, sweepStartFreq, sweepEndFreq, phaseRad, sweep_mode, sweep_dir, data, buf_size);
            break;
        default:
            return RP_EIPV;
    }
    if (waveform != RP_WAVEFORM_ARBITRARY)
        size = buf_size;
    return generate_writeData(channel, data, phase, size);
}

int synthesis_sin(float scale, float* data_out, uint16_t buffSize) {
    for (int unsigned i = 0; i < DAC_BUFFER_SIZE; i++) {
        data_out[i] = (float)(sin(2 * M_PI * (float)i / (float)buffSize)) * scale;
    }
    return RP_OK;
}

int synthesis_triangle(float scale, float* data_out, uint16_t buffSize) {
    for (int unsigned i = 0; i < DAC_BUFFER_SIZE; i++) {
        data_out[i] = (float)((asin(sin(2 * M_PI * (float)i / (float)buffSize)) / M_PI * 2)) * scale;
    }
    return RP_OK;
}

int synthesis_rampUp(float scale, float* data_out, uint16_t buffSize) {
    data_out[DAC_BUFFER_SIZE - 1] = 0;
    for (int unsigned i = 0; i < DAC_BUFFER_SIZE - 1; i++) {
        data_out[DAC_BUFFER_SIZE - i - 2] = (float)(-1.0 * (acos(cos(M_PI * (float)i / (float)buffSize)) / M_PI - 1)) * scale;
    }
    return RP_OK;
}

int synthesis_rampDown(float scale, float* data_out, uint16_t buffSize) {
    for (int unsigned i = 0; i < DAC_BUFFER_SIZE; i++) {
        data_out[i] = (float)(-1.0 * (acos(cos(M_PI * (float)i / (float)buffSize)) / M_PI - 1)) * scale;
    }
    return RP_OK;
}

int synthesis_DC(float scale, float* data_out, uint16_t buffSize) {
    for (int unsigned i = 0; i < buffSize; i++) {
        data_out[i] = 1.0 * scale;
    }
    return RP_OK;
}

int synthesis_DC_NEG(float scale, float* data_out, uint16_t buffSize) {
    for (int unsigned i = 0; i < buffSize; i++) {
        data_out[i] = -1.0 * scale;
    }
    return RP_OK;
}

int synthesis_PWM(float scale, float ratio, float* data_out, uint16_t buffSize) {
    // calculate number of samples that need to be high
    int h = (int)(buffSize * ratio);

    for (int i = 0; i < DAC_BUFFER_SIZE; i++) {
        if (i < h) {
            data_out[i] = 1.0;
        } else {
            data_out[i] = (float)-1.0;
        }
        data_out[i] *= scale;
    }
    return RP_OK;
}

int synthesis_arbitrary(float scale, rp_channel_t channel, float* data_out, uint32_t* size) {

    CHECK_CHANNEL

    float* pointer = ch_arbitraryData[channel];

    for (int unsigned i = 0; i < DAC_BUFFER_SIZE; i++) {
        data_out[i] = pointer[i] * scale;
    }
    *size = ch_arb_size[channel];
    return RP_OK;
}

int synthesis_square(float scale, float frequency, float riseTime, float fallTime, float* data_out, uint16_t buffSize) {

    float period_us = 1000000.0 / frequency;
    uint16_t riseTimeSamples = (uint16_t)(riseTime / period_us * buffSize);
    if (riseTimeSamples == 0)
        riseTimeSamples = 1;
    uint16_t fallTimeSamples = (uint16_t)(fallTime / period_us * buffSize);
    if (fallTimeSamples == 0)
        fallTimeSamples = 1;

    for (int unsigned i = 0; i < DAC_BUFFER_SIZE; i++) {
        int x = (i % buffSize);
        if (x < riseTimeSamples / 2) {
            data_out[i] = (float)x / ((float)riseTimeSamples / 2.0f);
        } else if (x < (buffSize - fallTimeSamples) / 2) {
            data_out[i] = 1.0f;
        } else if (x < (buffSize + fallTimeSamples) / 2) {
            if (fallTimeSamples > 1) {
                data_out[i] = 1.0f - 2.0f * (float)(x - (buffSize - fallTimeSamples) / 2.0f) / ((float)fallTimeSamples);
                if (data_out[i] > 1) {
                    data_out[i] = 1;
                }
            } else {
                data_out[i] = 0;
            }
        } else if (x < buffSize - (riseTimeSamples / 2)) {
            data_out[i] = -1.0f;
        } else {
            data_out[i] = -1.0f + (float)(x - (buffSize - (riseTimeSamples / 2.0f))) / ((float)riseTimeSamples / 2.0f);
        }
        data_out[i] *= scale;
    }

    return RP_OK;
}

int synthesis_square_Z20_250(float scale, float frequency, float* data_out, uint16_t buffSize) {
    (void)(frequency);
    for (int unsigned i = 0; i < DAC_BUFFER_SIZE; i++) {
        if ((i % buffSize) < buffSize / 2.0)
            data_out[i] = 1.0f;
        else
            data_out[i] = -1.0f;
        data_out[i] *= scale;
    }

    return RP_OK;
}

int synthesis_sweep(float scale, float frequency, float frequency_start, float frequency_end, float phaseRad, rp_gen_sweep_mode_t mode, rp_gen_sweep_dir_t dir, float* data_out,
                    uint16_t buffSize) {

    bool inverDir = false;
    float sign = 1;
    if (frequency_end < frequency_start) {
        inverDir = true;
    }
    for (int unsigned i = 0; i < buffSize; i++) {
        double x = (double)i / (double)buffSize;
        if (dir == RP_GEN_SWEEP_DIR_UP_DOWN) {
            x = x * 2;
            if (x > 1) {
                x = 2 - x;
                sign = -1;
            }
        }

        double freq = 0;
        if (mode == RP_GEN_SWEEP_MODE_LINEAR) {
            freq = ((frequency_end - frequency_start) * x + frequency_start) * 2;
        }
        if (mode == RP_GEN_SWEEP_MODE_LOG) {
            freq = frequency_start * exp(x * log(frequency_end / frequency_start));
        }
        if (inverDir)
            x = 1 - x;
        data_out[i] = sin(freq * 2 * M_PI * (x) / frequency + phaseRad) * sign;
        data_out[i] *= scale;
    }
    return RP_OK;
}

int triggerIfInternal(rp_channel_t channel) {
    uint32_t value;
    generate_getTriggerSource(channel, &value);
    if (value == RP_GEN_TRIG_SRC_INTERNAL) {
        generate_setTriggerSource(channel, 1);
    }
    return RP_OK;
}

int gen_setEnableTempProtection(rp_channel_t channel, bool enable) {

    CHECK_CHANNEL

    if (!rp_HPGetFastDACIsTempProtectionOrDefault())
        return RP_NOTS;

    ch_EnableTempProtection[channel] = enable;
    return generate_setEnableTempProtection(channel, enable);
}

int gen_getEnableTempProtection(rp_channel_t channel, bool* enable) {

    CHECK_CHANNEL

    if (!rp_HPGetFastDACIsTempProtectionOrDefault())
        return RP_NOTS;

    return generate_getEnableTempProtection(channel, enable);
}

int gen_setLatchTempAlarm(rp_channel_t channel, bool status) {

    CHECK_CHANNEL

    if (!rp_HPGetFastDACIsTempProtectionOrDefault())
        return RP_NOTS;

    ch_LatchTempAlarm[channel] = status;
    return generate_setLatchTempAlarm(channel, status);
}

int gen_getLatchTempAlarm(rp_channel_t channel, bool* status) {

    CHECK_CHANNEL

    if (!rp_HPGetFastDACIsTempProtectionOrDefault())
        return RP_NOTS;

    return generate_getLatchTempAlarm(channel, status);
}

int gen_getRuntimeTempAlarm(rp_channel_t channel, bool* status) {

    CHECK_CHANNEL

    if (!rp_HPGetFastDACIsTempProtectionOrDefault())
        return RP_NOTS;

    return generate_getRuntimeTempAlarm(channel, status);
}

int gen_setGainOut(rp_channel_t channel, rp_gen_gain_t mode) {

    CHECK_CHANNEL

    if (!rp_HPGetIsGainDACx5OrDefault())
        return RP_NOTS;

    rp_gen_gain_t* gain = &ch_gain[channel];
    int ch = (channel == RP_CH_1 ? RP_MAX7311_OUT1 : RP_MAX7311_OUT2);
    int status = rp_setGainOut_C(ch, mode == RP_GAIN_1X ? RP_GAIN_2V : RP_GAIN_10V);
    if (status == RP_OK) {
        *gain = mode;
    } else {
        return status;
    }
    int ret = gen_setAmplitude(channel, ch_amplitude[channel]);
    if (ret != RP_OK) {
        return ret;
    }
    return gen_setOffset(channel, ch_offset[channel]);
}

int gen_getGainOut(rp_channel_t channel, rp_gen_gain_t* status) {

    CHECK_CHANNEL

    *status = ch_gain[channel];
    return RP_OK;
}

int gen_SetExtTriggerDebouncerUs(double value) {
    if (value < 0)
        return RP_EIPV;

    double sp = 0;
    int ret = gen_GetDACSamplePeriod(&sp);
    if (ret != RP_OK) {
        return ret;
    }
    uint32_t samples = (value * 1000.0) / sp;
    return generate_SetTriggerDebouncer(samples);
}

int gen_GetExtTriggerDebouncerUs(double* value) {
    double sp = 0;
    int ret = gen_GetDACSamplePeriod(&sp);
    if (ret != RP_OK) {
        return ret;
    }
    uint32_t samples = 0;
    generate_GetTriggerDebouncer(&samples);
    *value = (samples * sp) / 1000.0;
    return RP_OK;
}

int gen_setLoadMode(rp_channel_t channel, rp_gen_load_mode_t mode) {
    CHECK_CHANNEL
    ch_load_mode[channel] = mode;
    return RP_OK;
}

int gen_getLoadMode(rp_channel_t channel, rp_gen_load_mode_t* mode) {
    CHECK_CHANNEL
    *mode = ch_load_mode[channel];
    return RP_OK;
}

int gen_axi_SetEnable(rp_channel_t channel, bool enable) {

    CHECK_CHANNEL
    if (enable) {
        if (asg_axi_mem_reserved_index[channel] == 0) {
            ERROR_LOG("Memory not reserved.")
        }
    }

    return generate_axi_SetEnable(channel, enable);
}

int gen_axi_GetEnable(rp_channel_t channel, bool* enable) {

    CHECK_CHANNEL

    return generate_axi_GetEnable(channel, enable);
}

int gen_axi_ReserveMemory(rp_channel_t channel, uint32_t start, uint32_t end) {

    CHECK_CHANNEL

    ECHECK(axi_initManager())

    axi_releaseMemory(asg_axi_mem_reserved_index[channel]);
    if ((start + 8) >= end) {
        ERROR_LOG("The start address may be greater than the end address.")
        return RP_EOOR;
    }

    int size = end - start;
    if (size % 0x80) {
        ERROR_LOG("Memory size must be a multiple of 0x80.")
        return RP_EOOR;
    }

    uint64_t index;
    ECHECK(axi_reserveMemory(start, end - start, &index))
    asg_axi_mem_reserved_index[channel] = index;
    asg_axi_reserved_samples_count[channel] = size / 2;
    generate_axi_SetStartAddress(channel, start);
    generate_axi_SetEndAddress(channel, end - 8);
    return RP_OK;
}

int gen_axi_ReleaseMemory(rp_channel_t channel) {

    CHECK_CHANNEL

    axi_releaseMemory(asg_axi_mem_reserved_index[channel]);
    asg_axi_mem_reserved_index[channel] = 0;
    return RP_OK;
}

int gen_axi_SetDecimation(rp_channel_t channel, uint32_t decimation) {

    CHECK_CHANNEL

    if (decimation == 0 || decimation >= 65536)
        return RP_EOOR;

    return generate_axi_SetDecimation(channel, decimation);
}

int gen_axi_GetDecimation(rp_channel_t channel, uint32_t* decimation) {

    CHECK_CHANNEL

    return generate_axi_GetDecimation(channel, decimation);
}

int gen_axi_WriteWaveform(rp_channel_t channel, float* data, uint32_t length) {

    CHECK_CHANNEL

    if (asg_axi_mem_reserved_index[channel] == 0) {
        ERROR_LOG("Memory not reserved.")
        return RP_EOOR;
    }

    float fs = 0;
    if (rp_HPGetFastDACOutFullScale(channel, &fs) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC out full scale");
        return RP_NOTS;
    }

    bool is_sign = false;
    if (rp_HPGetFastDACIsSigned(&is_sign) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC sign value");
        return RP_NOTS;
    }

    uint8_t bits = 0;
    if (rp_HPGetFastDACBits(&bits) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC bits\n");
        return RP_NOTS;
    }

    uint16_t* buffer = NULL;
    uint32_t size = 0;
    if (axi_getMapped(asg_axi_mem_reserved_index[channel], &buffer, &size) == RP_OK) {
        if (size / 2 != length) {
            WARNING("Input buffer size does not match memory size")
        }
        if (size / 2 < length) {
            ERROR_LOG("The input buffer is larger than the reserved memory.")
            return RP_EOOR;
        }

        for (uint32_t i = 0; i < length; i++) {
            if (data[i] < (is_sign ? -fs : 0) || data[i] > fs) {
                ERROR_LOG("The signal is greater than acceptable. Min %f Max %f", (is_sign ? -fs : 0), fs);
                return RP_ENN;
            }
            buffer[i] = cmn_convertToCnt(data[i], bits, 1.0, is_sign, 1, 0);
        }

        return RP_OK;
    } else {
        ERROR_LOG("Error getting memory region.");
    }
    return RP_EOOR;
}
