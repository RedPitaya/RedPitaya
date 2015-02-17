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

#include <sys/socket.h>
#include <float.h>
#include "math.h"
#include "common.h"
#include "generate.h"
#include "gen_handler.h"

double chA_amplitude = 1, chB_amplitude = 1;
double chA_offset = 0, chB_offset = 0;
double chA_dutyCycle, chB_dutyCycle;
double chA_frequency, chB_frequency;
double chA_phase,     chB_phase;
int chA_burstCount = 1, chB_burstCount = 1;
int chA_burstRepetition = 1, chB_burstRepetition = 1;
uint32_t chA_burstPeriod = 0, chB_burstPeriod = 0;
rp_waveform_t chA_waveform, chB_waveform;
uint32_t chA_size = BUFFER_LENGTH, chB_size = BUFFER_LENGTH;

float chA_arbitraryData[BUFFER_LENGTH];
float chB_arbitraryData[BUFFER_LENGTH];
uint32_t chA_arb_size = BUFFER_LENGTH, chB_arb_size = BUFFER_LENGTH;

int gen_SetDefaultValues() {
    ECHECK(gen_Disable(RP_CH_1));
    ECHECK(gen_Disable(RP_CH_2));
    ECHECK(gen_Frequency(RP_CH_1, 1000));
    ECHECK(gen_Frequency(RP_CH_2, 1000));
    ECHECK(gen_BurstRepetitions(RP_CH_1, 1));
    ECHECK(gen_BurstRepetitions(RP_CH_2, 1));
    ECHECK(gen_BurstPeriod(RP_CH_1, (uint32_t)(1/1000.0 * MICRO)));   // period = 1/frequency in us
    ECHECK(gen_BurstPeriod(RP_CH_2, (uint32_t)(1/1000.0 * MICRO)));   // period = 1/frequency in us
    ECHECK(gen_Waveform(RP_CH_1, RP_WAVEFORM_SINE));
    ECHECK(gen_Waveform(RP_CH_2, RP_WAVEFORM_SINE));
    ECHECK(gen_setAmplitude(RP_CH_1, 1));
    ECHECK(gen_setAmplitude(RP_CH_2, 1));
    ECHECK(generate_setDCOffset(RP_CH_1, 0));
    ECHECK(generate_setDCOffset(RP_CH_2, 0));
    ECHECK(gen_DutyCycle(RP_CH_1, 0.5));
    ECHECK(gen_DutyCycle(RP_CH_2, 0.5));
    ECHECK(gen_GenMode(RP_CH_1, RP_GEN_MODE_CONTINUOUS));
    ECHECK(gen_GenMode(RP_CH_2, RP_GEN_MODE_CONTINUOUS));
    ECHECK(gen_BurstCount(RP_CH_1, 1));
    ECHECK(gen_BurstCount(RP_CH_2, 1));
    ECHECK(gen_TriggerSource(RP_CH_1, RP_GEN_TRIG_SRC_INTERNAL));
    ECHECK(gen_TriggerSource(RP_CH_2, RP_GEN_TRIG_SRC_INTERNAL));
    ECHECK(gen_Phase(RP_CH_1, 0.0));
    ECHECK(gen_Phase(RP_CH_2, 0.0));
    return RP_OK;
}

int gen_Disable(rp_channel_t channel) {
    return generate_setOutputDisable(channel, true);
}

int gen_Enable(rp_channel_t channel) {
    return generate_setOutputDisable(channel, false);
}

int gen_checkAmplitudeAndOffset(double amplitude, double offset) {
    if (fabs(amplitude) + fabs(offset) > LEVEL_MAX) {
        return RP_EOOR;
    }
    return RP_OK;
}

int gen_setAmplitude(rp_channel_t channel, double amplitude) {
    double offset;
    CHECK_OUTPUT(offset = chA_offset,
                 offset = chB_offset)
    ECHECK(gen_checkAmplitudeAndOffset(amplitude, offset));

    CHECK_OUTPUT(chA_amplitude = amplitude,
                 chB_amplitude = amplitude)
    return generate_setAmplitude(channel, (float) amplitude);
}

int gen_Offset(rp_channel_t channel, double offset) {
    double amplitude;
    CHECK_OUTPUT(amplitude = chA_amplitude,
                 amplitude = chB_amplitude)
    ECHECK(gen_checkAmplitudeAndOffset(amplitude, offset));

    CHECK_OUTPUT(chA_offset = offset,
                 chB_offset = offset)
    return generate_setDCOffset(channel, (float)offset);
}

int gen_Frequency(rp_channel_t channel, double frequency) {
    if (frequency < FREQUENCY_MIN || frequency > FREQUENCY_MAX) {
        return RP_EOOR;
    }

    if (channel == RP_CH_1) {
        chA_frequency = frequency;
        gen_BurstPeriod(channel, chA_burstPeriod);
    }
    else if (channel == RP_CH_2) {
        chB_frequency = frequency;
        gen_BurstPeriod(channel, chB_burstPeriod);
    }
    else {
        return RP_EPN;
    }

    ECHECK(generate_setFrequency(channel, (float)frequency));
    ECHECK(synthesize_signal(channel));
    return gen_Synchronise();
}

int gen_Phase(rp_channel_t channel, double phase) {
    if (phase < PHASE_MIN || phase > PHASE_MAX) {
        return RP_EOOR;
    }
    if (phase < 0) {
        phase += 360;
    }
    CHECK_OUTPUT(chA_phase = phase,
                 chB_phase = phase)

    ECHECK(synthesize_signal(channel));
    return gen_Synchronise();
}

int gen_Waveform(rp_channel_t channel, rp_waveform_t type) {
    CHECK_OUTPUT(chA_waveform = type,
                 chB_waveform = type)
    if (type == RP_WAVEFORM_ARBITRARY) {
        CHECK_OUTPUT(chA_size = chA_arb_size,
                     chB_size = chB_arb_size)
    }
    else{
        CHECK_OUTPUT(chA_size = BUFFER_LENGTH,
                     chB_size = BUFFER_LENGTH)
    }
    return synthesize_signal(channel);
}

int gen_ArbWaveform(rp_channel_t channel, float *data, uint32_t length) {
    // Check if data is normalized
    float min = FLT_MAX, max = FLT_MIN; // initial values
    int i;
    for(i = 0; i < length; i++) {
        if (data[i] < min)
            min = data[i];
        if (data[i] > max)
            max = data[i];
    }
    if (min < ARBITRARY_MIN || max > ARBITRARY_MAX) {
        return RP_ENN;
    }

    // Save data
    float *pointer;
    CHECK_OUTPUT(pointer = chA_arbitraryData,
                 pointer = chB_arbitraryData)
    for(i = 0; i < length; i++) {
        pointer[i] = data[i];
    }
    for(i = length; i < BUFFER_LENGTH; i++) { // clear the rest of the buffer
        pointer[i] = 0;
    }

    if (channel == RP_CH_1) {
        chA_arb_size = length;
        if(chA_waveform==RP_WAVEFORM_ARBITRARY){
        	return synthesize_signal(channel);
        }
    }
    else if (channel == RP_CH_2) {
    	chA_arb_size = length;
        if(chB_waveform==RP_WAVEFORM_ARBITRARY){
        	return synthesize_signal(channel);
        }
    }
    else {
        return RP_EPN;
    }

    return RP_OK;
}

int gen_DutyCycle(rp_channel_t channel, double ratio) {
    if (ratio < DUTY_CYCLE_MIN || ratio > DUTY_CYCLE_MAX) {
        return RP_EOOR;
    }
    CHECK_OUTPUT(chA_dutyCycle = ratio,
                 chB_dutyCycle = ratio)
    return synthesize_signal(channel);
}

int gen_GenMode(rp_channel_t channel, rp_gen_mode_t mode) {
    if (mode == RP_GEN_MODE_CONTINUOUS) {
        ECHECK(generate_setGatedBurst(channel, 0));
        ECHECK(generate_setBurstDelay(channel, 0));
        ECHECK(generate_setBurstRepetitions(channel, 0));
        return generate_setBurstCount(channel, 0);
    }
    else if (mode == RP_GEN_MODE_BURST) {
        ECHECK(gen_BurstCount(channel,       channel == RP_CH_1 ? chA_burstCount : chB_burstCount));
        ECHECK(gen_BurstRepetitions(channel, channel == RP_CH_1 ? chA_burstRepetition : chB_burstRepetition));
        ECHECK(gen_BurstPeriod(channel,      channel == RP_CH_1 ? chA_burstPeriod : chB_burstPeriod));
        return RP_OK;
    }
    else if (mode == RP_GEN_MODE_STREAM) {
        return RP_EUF;
    }
    else {
        return RP_EIPV;
    }
}

int gen_BurstCount(rp_channel_t channel, int num) {
    if ((num < BURST_COUNT_MIN || num > BURST_COUNT_MAX) && num == 0) {
        return RP_EOOR;
    }
    CHECK_OUTPUT(chA_burstCount = num,
            chB_burstCount = num)
    if (num == -1) {    // -1 represents infinity. In FPGA value 0 represents infinity
        num = 0;
    }
    ECHECK(generate_setBurstCount(channel, num));
    return generate_triggerIfInternal(channel);
}

int gen_BurstRepetitions(rp_channel_t channel, int repetitions) {
    if ((repetitions < BURST_REPETITIONS_MIN || repetitions > BURST_REPETITIONS_MAX) && repetitions != -1) {
        return RP_EOOR;
    }
    CHECK_OUTPUT(chA_burstRepetition = repetitions,
            chB_burstRepetition = repetitions)
    if (repetitions == -1) {
        repetitions = 0;
    }
    return generate_setBurstRepetitions(channel, repetitions-1);
}

int gen_BurstPeriod(rp_channel_t channel, uint32_t period) {
    if (period < BURST_PERIOD_MIN || period > BURST_PERIOD_MAX) {
        return RP_EOOR;
    }
    int burstCount;
    CHECK_OUTPUT(burstCount = chA_burstCount,
                 burstCount = chB_burstCount)
    // period = signal_time * burst_count + delay_time
    int delay = (int) (period - (1 / (channel == RP_CH_1 ? chA_frequency : chB_frequency) * MICRO) * burstCount);
    if (delay <= 0) {
        // if delay is 0, then FPGA generates continuous signal
        delay = 1;
    }
    CHECK_OUTPUT(chA_burstPeriod = period,
                 chB_burstPeriod = period)
    return generate_setBurstDelay(channel, delay);
}

int gen_TriggerSource(rp_channel_t channel, rp_trig_src_t src) {
    if (src == RP_GEN_TRIG_GATED_BURST) {
        ECHECK(generate_setGatedBurst(channel, 1));
        ECHECK(gen_GenMode(channel, RP_GEN_MODE_BURST));
        return generate_setTriggerSource(channel, 2);
    }
    else {
        ECHECK(generate_setGatedBurst(channel, 0));
    }

    if (src == RP_GEN_TRIG_SRC_INTERNAL) {
        ECHECK(gen_GenMode(channel, RP_GEN_MODE_CONTINUOUS));
        return generate_setTriggerSource(channel, 1);
    }
    else if (src == RP_GEN_TRIG_SRC_EXT_PE) {
        ECHECK(gen_GenMode(channel, RP_GEN_MODE_BURST));
        return generate_setTriggerSource(channel, 2);
    }
    else if (src == RP_GEN_TRIG_SRC_EXT_NE) {
        ECHECK(gen_GenMode(channel, RP_GEN_MODE_BURST));
        return generate_setTriggerSource(channel, 3);
    }
    else {
        return RP_EIPV;
    }
}

int gen_Trigger(int mask) {
    switch (mask) {
        case 1:
            ECHECK(gen_GenMode(RP_CH_1, RP_GEN_MODE_BURST));
            return generate_setTriggerSource(RP_CH_1, RP_GEN_TRIG_SRC_INTERNAL);
        case 2:
            ECHECK(gen_GenMode(RP_CH_2, RP_GEN_MODE_BURST));
            return generate_setTriggerSource(RP_CH_2, RP_GEN_TRIG_SRC_INTERNAL);
        case 3:
            ECHECK(gen_GenMode(RP_CH_1, RP_GEN_MODE_BURST));
            ECHECK(gen_GenMode(RP_CH_2, RP_GEN_MODE_BURST));
            return generate_simultaneousTrigger();
        default:
            return RP_EOOR;
    }
}

int gen_Synchronise() {
    return generate_Synchronise();
}

int synthesize_signal(rp_channel_t channel) {
    float data[BUFFER_LENGTH];
    rp_waveform_t waveform;
    double dutyCycle, frequency;
    uint32_t size, phase;

    if (channel == RP_CH_1) {
        waveform = chA_waveform;
        dutyCycle = chA_dutyCycle;
        frequency = chA_frequency;
        size = chA_size;
        phase = (uint32_t) (chA_phase * BUFFER_LENGTH / 360.0);
    }
    else if (channel == RP_CH_2) {
        waveform = chB_waveform;
        dutyCycle = chB_dutyCycle;
        frequency = chB_frequency;
    	size = chB_size;
        phase = (uint32_t) (chB_phase * BUFFER_LENGTH / 360.0);
    }
    else{
        return RP_EPN;
    }


    switch (waveform) {
        case RP_WAVEFORM_SINE:
            synthesis_sin(data);
            break;
        case RP_WAVEFORM_TRIANGLE:
            synthesis_triangle(data);
            break;
        case RP_WAVEFORM_SQUARE:
            synthesis_square(frequency, data);
            break;
        case RP_WAVEFORM_RAMP_UP:
            synthesis_rampUp(data);
            break;
        case RP_WAVEFORM_RAMP_DOWN:
            synthesis_rampDown(data);
            break;
        case RP_WAVEFORM_DC:
            synthesis_DC(data);
            break;
        case RP_WAVEFORM_PWM:
            synthesis_PWM(dutyCycle, data);
            break;
        case RP_WAVEFORM_ARBITRARY:
            synthesis_arbitrary(channel, data, &size);
            break;
        default:
            return RP_EIPV;
    }
    return generate_writeData(channel, data, phase, size);
}

int synthesis_sin(float *data_out) {
    int i;
    for(i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = (float) (sin(2 * M_PI * (double) i / (double) BUFFER_LENGTH));
    }
    return RP_OK;
}

int synthesis_triangle(float *data_out) {
    int i;
    for(i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = (float) ((asin(sin(2 * M_PI * (double) i / (double) BUFFER_LENGTH)) / M_PI * 2));
    }
    return RP_OK;
}

int synthesis_rampUp(float *data_out) {
    int i;
    data_out[BUFFER_LENGTH -1] = 0;
    for(i = 0; i < BUFFER_LENGTH-1; i++) {
        data_out[BUFFER_LENGTH - i-2] = (float) (-1.0 * (acos(cos(M_PI * (double) i / (double) BUFFER_LENGTH)) / M_PI - 1));
    }
    return RP_OK;
}

int synthesis_rampDown(float *data_out) {
    int i;
    for(i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = (float) (-1.0 * (acos(cos(M_PI * (double) i / (double) BUFFER_LENGTH)) / M_PI - 1));
    }
    return RP_OK;
}

int synthesis_DC(float *data_out) {
    int i;
    for(i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = 1.0;
    }
    return RP_OK;
}

int synthesis_PWM(double ratio, float *data_out) {
    int i;

    // calculate number of samples that need to be high
    int h = (int) (BUFFER_LENGTH/2 * ratio);

    for(i = 0; i < BUFFER_LENGTH; i++) {
        if (i < h || i >= BUFFER_LENGTH - h) {
            data_out[i] = 1.0;
        }
        else {
            data_out[i] = (float) -1.0;
        }
    }
    return RP_OK;
}

int synthesis_arbitrary(rp_channel_t channel, float *data_out, uint32_t * size) {
    float *pointer;
    CHECK_OUTPUT(pointer = chA_arbitraryData,
                 pointer = chB_arbitraryData)
    for (int i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = pointer[i];
    }
    CHECK_OUTPUT(*size = chA_arb_size,
                 *size = chB_arb_size)
    return RP_OK;
}

int synthesis_square(double frequency, float *data_out) {
    uint32_t i;

    // Various locally used constants - HW specific parameters
    const int trans0 = 30;
    const int trans1 = 300;
    const double tt2 = 0.249;

    int trans = (int) (frequency / 1e6 * trans1); // 300 samples at 1 MHz

    if (trans <= 10) {
        trans = trans0;
    }

    for(i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = (float) (sin(2 * M_PI * (double) i / (double) BUFFER_LENGTH));
        if (data_out[i] > 0)
            data_out[i] = 1.0;
        else
            data_out[i] = (float) -1.0;

        // Soft linear transitions
        double mm, qq, xx, xm;
        double x1, x2, y1, y2;

        xx = i;
        xm = BUFFER_LENGTH;

        x1 = xm * tt2;
        x2 = xm * tt2 + (double) trans;

        if ((xx > x1) && (xx <= x2)) {

            y1 = 1.0;
            y2 = -1.0;

            mm = (y2 - y1) / (x2 - x1);
            qq = y1 - mm * x1;

            data_out[i] = (int32_t) round(mm * xx + qq);
        }

        x1 = xm * 0.75;
        x2 = xm * 0.75 + trans;

        if ((xx > x1) && (xx <= x2)) {

            y1 = -1.0;
            y2 = 1.0;

            mm = (y2 - y1) / (x2 - x1);
            qq = y1 - mm * x1;

            data_out[i] = (float) (mm * xx + qq);
        }
    }
    return RP_OK;
}
