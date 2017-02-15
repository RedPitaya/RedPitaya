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
#include <float.h>

#include "redpitaya/rp1.h"
#include "common.h"
#include "gen.h"
#include "calib.h"

static volatile generate_control_t *generate = NULL;
static volatile int32_t *data_ch[2] = {NULL, NULL};

static int gen_Init() {
    cmn_Map(GENERATE_BASE_SIZE, GENERATE_BASE_ADDR, (void **) &generate);
    data_ch[0] = (int32_t *) ((char *) generate + (CHA_DATA_OFFSET));
    data_ch[1] = (int32_t *) ((char *) generate + (CHB_DATA_OFFSET));
    return RP_OK;
}

static int gen_Release() {
    cmn_Unmap(GENERATE_BASE_SIZE, (void **) &generate);
    data_ch[0] = NULL;
    data_ch[1] = NULL;
    return RP_OK;
}

static int generate_setTriggerSource(int unsigned channel, unsigned short value) {
    if (channel == 0)  generate->AtriggerSelector = value;
    else               generate->BtriggerSelector = value;
    return RP_OK;
}

static int generate_getTriggerSource(int unsigned channel, uint32_t *value) {
    if (channel == 0)  *value = generate->AtriggerSelector;
    else               *value = generate->BtriggerSelector;
    return RP_OK;
}

static int generate_setGatedBurst(int unsigned channel, uint32_t value) {
    if (channel == 0)  generate->AgatedBursts = value;
    else               generate->BgatedBursts = value;
    return RP_OK;
}

static int generate_setBurstCount(int unsigned channel, uint32_t num) {
    generate->properties_ch[channel].cyclesInOneBurst = num;
    return RP_OK;
}

static int generate_setBurstRepetitions(int unsigned channel, uint32_t repetitions) {
    generate->properties_ch[channel].burstRepetitions = repetitions;
    return RP_OK;
}

static int generate_setBurstDelay(int unsigned channel, uint32_t delay) {
    generate->properties_ch[channel].delayBetweenBurstRepetitions = delay;
    return RP_OK;
}

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

// global variables
// TODO: should be organized into a system status structure
float         ch_amplitude        [2] = {1, 1};
float         ch_offset           [2] = {0, 0};
float         ch_dutyCycle        [2] = {0, 0};
float         ch_frequency        [2];
float         ch_phase            [2] = {0, 0};
int           ch_burstCount       [2] = {1, 1};
int           ch_burstRepetition  [2] = {1, 1};
uint32_t      ch_burstPeriod      [2] = {0, 0};
rp_waveform_t ch_waveform         [2];
uint32_t      ch_size             [2] = {BUFFER_LENGTH, BUFFER_LENGTH};
uint32_t      ch_arb_size         [2] = {BUFFER_LENGTH, BUFFER_LENGTH};

float ch_arbitraryData[2][BUFFER_LENGTH];

static int synthesis_sin(float *data_out) {
    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = (float) (sin(2 * M_PI * (float) i / (float) BUFFER_LENGTH));
    }
    return RP_OK;
}

static int synthesis_triangle(float *data_out) {
    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = (float) ((asin(sin(2 * M_PI * (float) i / (float) BUFFER_LENGTH)) / M_PI * 2));
    }
    return RP_OK;
}

static int synthesis_rampUp(float *data_out) {
    data_out[BUFFER_LENGTH -1] = 0;
    for(int unsigned i = 0; i < BUFFER_LENGTH-1; i++) {
        data_out[BUFFER_LENGTH - i-2] = (float) (-1.0 * (acos(cos(M_PI * (float) i / (float) BUFFER_LENGTH)) / M_PI - 1));
    }
    return RP_OK;
}

static int synthesis_rampDown(float *data_out) {
    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = (float) (-1.0 * (acos(cos(M_PI * (float) i / (float) BUFFER_LENGTH)) / M_PI - 1));
    }
    return RP_OK;
}

static int synthesis_DC(float *data_out) {
    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = 1.0;
    }
    return RP_OK;
}

static int synthesis_PWM(float ratio, float *data_out) {
    // calculate number of samples that need to be high
    int h = (int) (BUFFER_LENGTH/2 * ratio);

    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        if (i < h || i >= BUFFER_LENGTH - h) {
            data_out[i] = 1.0;
        }
        else {
            data_out[i] = (float) -1.0;
        }
    }
    return RP_OK;
}

static int synthesis_arbitrary(int unsigned channel, float *data_out, uint32_t * size) {
    float *pointer;
    pointer = ch_arbitraryData[channel];
    for (int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = pointer[i];
    }
    *size = ch_arb_size[channel];
    return RP_OK;
}

static int synthesis_square(float frequency, float *data_out) {
    // Various locally used constants - HW specific parameters
    const int trans0 = 30;
    const int trans1 = 300;

    int trans = (int) (frequency / 1e6 * trans1); // 300 samples at 1 MHz

    if (trans <= 10)  trans = trans0;

    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        if      ((0 <= i                      ) && (i <  BUFFER_LENGTH/2 - trans))  data_out[i] =  1.0f;
        else if ((i >= BUFFER_LENGTH/2 - trans) && (i <  BUFFER_LENGTH/2        ))  data_out[i] =  1.0f - (2.0f / trans) * (i - (BUFFER_LENGTH/2 - trans));
        else if ((0 <= BUFFER_LENGTH/2        ) && (i <  BUFFER_LENGTH   - trans))  data_out[i] = -1.0f;
        else if ((i >= BUFFER_LENGTH   - trans) && (i <  BUFFER_LENGTH          ))  data_out[i] = -1.0f + (2.0f / trans) * (i - (BUFFER_LENGTH   - trans));
    }

    return RP_OK;
}

static int synthesize_signal(int unsigned channel) {
    float data[BUFFER_LENGTH];
    rp_waveform_t waveform;
    float dutyCycle, frequency;
    uint32_t size;

    waveform = ch_waveform[channel];
    dutyCycle = ch_dutyCycle[channel];
    frequency = ch_frequency[channel];
    size = ch_size[channel];

    switch (waveform) {
        case RP_WAVEFORM_SINE     : synthesis_sin      (data);                 break;
        case RP_WAVEFORM_TRIANGLE : synthesis_triangle (data);                 break;
        case RP_WAVEFORM_SQUARE   : synthesis_square   (frequency, data);      break;
        case RP_WAVEFORM_RAMP_UP  : synthesis_rampUp   (data);                 break;
        case RP_WAVEFORM_RAMP_DOWN: synthesis_rampDown (data);                 break;
        case RP_WAVEFORM_DC       : synthesis_DC       (data);                 break;
        case RP_WAVEFORM_PWM      : synthesis_PWM      (dutyCycle, data);      break;
        case RP_WAVEFORM_ARBITRARY: synthesis_arbitrary(channel, data, &size); break;
        default:                    return RP_EIPV;
    }

    generate->properties_ch[channel].counterWrap = 65536 * size - 1;

    for (int unsigned i=0; i<BUFFER_LENGTH; i++)
        data_ch[channel][i] = calib_Saturate(DATA_BIT_LENGTH, data[i] / AMPLITUDE_MAX);
    return RP_OK;
}

static int gen_Synchronise() {
    // Both channels must be reset simultaneously
    cmn_SetBits  ((uint32_t *) generate, 0x00400040, 0xFFFFFFFF);
    cmn_UnsetBits((uint32_t *) generate, 0x00400040, 0xFFFFFFFF);
    return RP_OK;
}

static int triggerIfInternal(int unsigned channel) {
    uint32_t value = 0;
    generate_getTriggerSource(channel, &value);
    if (value == RP_GEN_TRIG_SRC_INTERNAL)
        generate_setTriggerSource(channel, 1);
    return RP_OK;
}

/**
* Generate methods
*/

int rp_GenReset() {
    for (int unsigned i=0; i<2; i++) {
        rp_GenOutDisable(i);
        rp_GenFreq(i, 1000);
        rp_GenBurstRepetitions(i, 1);
        rp_GenBurstPeriod(i, (uint32_t) (1 / 1000.0 * MICRO));   // period = 1/frequency in us
        rp_GenWaveform(i, RP_WAVEFORM_SINE);
        rp_GenOffset(i, 0);
        rp_GenAmp(i, 1);
        rp_GenDutyCycle(i, 0.5);
        rp_GenMode(i, RP_GEN_MODE_CONTINUOUS);
        rp_GenBurstCount(i, 1);
        rp_GenBurstPeriod(i, BURST_PERIOD_MIN);
        rp_GenTriggerSource(i, RP_GEN_TRIG_SRC_INTERNAL);
        rp_GenPhase(i, 0.0);
    }
    return RP_OK;
}

int rp_GenOutDisable(int unsigned channel) {
    if (channel == 0)  generate->AsetOutputTo0 = 1;
    else               generate->BsetOutputTo0 = 1;
    return RP_OK;
}

int rp_GenOutEnable(int unsigned channel) {
    if (channel == 0)  generate->AsetOutputTo0 = 0;
    else               generate->BsetOutputTo0 = 0;
    return RP_OK;
}

int rp_GenOutIsEnabled(int unsigned channel, bool *value) {
    if (channel == 0)  *value = !generate->AsetOutputTo0;
    else               *value = !generate->BsetOutputTo0;
    return RP_OK;
}

int rp_GenAmp(int unsigned channel, float amplitude) {
    ch_amplitude[channel] = amplitude;
    float calib_scl = calib_GetGenScale(channel) / (float)(1<<13);
    generate->properties_ch[channel].amplitudeScale = calib_Saturate(14, (int32_t)(amplitude / calib_scl));
    return RP_OK;
}

int rp_GenGetAmp(int unsigned channel, float *amplitude) {
    float calib_scl = calib_GetGenScale(channel) / (float)(1<<13);
    *amplitude = (float)(generate->properties_ch[channel].amplitudeScale) * calib_scl;
    return RP_OK;
}

int rp_GenOffset(int unsigned channel, float offset) {
    ch_offset[channel] = offset;
    // TODO: calibration
    //int32_t calib_off = calib_GetGenOffset(channel);
    generate->properties_ch[channel].amplitudeOffset = calib_Saturate(14, (int32_t)(offset * (float)(1<<13)));
    return RP_OK;
}

int rp_GenGetOffset(int unsigned channel, float *offset) {
    // TODO: calibration
    //int32_t calib_off = calib_GetGenOffset(channel);
    *offset = (float)generate->properties_ch[channel].amplitudeOffset / (float)(1<<13);
    return RP_OK;
}

int rp_GenFreq(int unsigned channel, float frequency) {
    if (frequency < FREQUENCY_MIN || frequency > FREQUENCY_MAX)
        return RP_EOOR;
    ch_frequency[channel] = frequency;
    rp_GenBurstPeriod(channel, ch_burstPeriod[channel]);
    generate->properties_ch[channel].counterStep = (uint32_t) round(65536 * frequency / DAC_FREQUENCY * BUFFER_LENGTH);
    channel == 0 ? (generate->ASM_WrapPointer = 1) : (generate->BSM_WrapPointer = 1);
    synthesize_signal(channel);
    return gen_Synchronise();
}

int rp_GenGetFreq(int unsigned channel, float *frequency) {
    *frequency = (float) round((generate->properties_ch[channel].counterStep * DAC_FREQUENCY) / (65536 * BUFFER_LENGTH));
    return RP_OK;
}

int rp_GenPhase(int unsigned channel, float phase) {
    if (phase < PHASE_MIN || phase > PHASE_MAX)
        return RP_EOOR;
    if (phase < 0)
        phase += 360;
    ch_phase[channel] = phase;
    synthesize_signal(channel);
    return gen_Synchronise();
}

int rp_GenGetPhase(int unsigned channel, float *phase) {
    *phase = ch_phase[channel];
    return RP_OK;
}

int rp_GenWaveform(int unsigned channel, rp_waveform_t type) {
    ch_waveform[channel] = type;
    if (type == RP_WAVEFORM_ARBITRARY)  ch_size[channel] = ch_arb_size[channel];
    else                                ch_size[channel] = BUFFER_LENGTH;
    return synthesize_signal(channel);
}

int rp_GenGetWaveform(int unsigned channel, rp_waveform_t *type) {
    *type = ch_waveform[channel];
    return RP_OK;
}

int rp_GenArbWaveform(int unsigned channel, float *waveform, uint32_t length) {
    // Check if waveform is normalized
    float min = FLT_MAX, max = -FLT_MAX; // initial values
    for (int unsigned i=0; i<length; i++) {
        if (waveform[i] < min)  min = waveform[i];
        if (waveform[i] > max)  max = waveform[i];
    }
    if (min < ARBITRARY_MIN || max > ARBITRARY_MAX)
        return RP_ENN;
    // Save waveform
    float *pointer;
    pointer = ch_arbitraryData[channel];
    for(int unsigned i=0; i<length; i++)
        pointer[i] = waveform[i];
    for(int unsigned i=length; i<BUFFER_LENGTH; i++) // clear the rest of the buffer
        pointer[i] = 0;
    ch_arb_size[channel] = length;
    if (ch_waveform[channel] == RP_WAVEFORM_ARBITRARY)
    	return synthesize_signal(channel);
    return RP_OK;
}

int rp_GenGetArbWaveform(int unsigned channel, float *waveform, uint32_t *length) {
    // If this waveform was not set, then this method will return incorrect waveform
    float *pointer;
    *length = ch_arb_size[channel];
    pointer = ch_arbitraryData[channel];
    for (int unsigned i=0; i<*length; ++i)
        waveform[i] = pointer[i];
    return RP_OK;
}

int rp_GenDutyCycle(int unsigned channel, float ratio) {
    if (ratio < DUTY_CYCLE_MIN || ratio > DUTY_CYCLE_MAX)
        return RP_EOOR;
    ch_dutyCycle[channel] = ratio;
    return synthesize_signal(channel);
}

int rp_GenGetDutyCycle(int unsigned channel, float *ratio) {
    *ratio = ch_dutyCycle[channel];
    return RP_OK;
}

int rp_GenMode(int unsigned channel, rp_gen_mode_t mode) {
    if (mode == RP_GEN_MODE_CONTINUOUS) {
        generate_setGatedBurst(channel, 0);
        generate_setBurstDelay(channel, 0);
        generate_setBurstRepetitions(channel, 0);
        generate_setBurstCount(channel, 0);
        return triggerIfInternal(channel);
    }
    else if (mode == RP_GEN_MODE_BURST) {
        rp_GenBurstCount       (channel, ch_burstCount     [channel]);
        rp_GenBurstRepetitions (channel, ch_burstRepetition[channel]);
        rp_GenBurstPeriod      (channel, ch_burstPeriod    [channel]);
        return RP_OK;
    }
    else if (mode == RP_GEN_MODE_STREAM)
        return RP_EUF;
    else
        return RP_EIPV;
}

int rp_GenGetMode(int unsigned channel, rp_gen_mode_t *mode) {
    uint32_t num;
    rp_GenGetBurstCount(channel, &num);
    if (num != 0)  *mode = RP_GEN_MODE_BURST;
    else           *mode = RP_GEN_MODE_CONTINUOUS;
    return RP_OK;
}

int rp_GenBurstCount(int unsigned channel, int num) {
    if ((num < BURST_COUNT_MIN || num > BURST_COUNT_MAX) && num == 0)
        return RP_EOOR;
    ch_burstCount[channel] = num;
    if (num == -1)    // -1 represents infinity. In FPGA value 0 represents infinity
        num = 0;
    generate_setBurstCount(channel, (uint32_t) num);
    // trigger channel if internal trigger source
    return triggerIfInternal(channel);
}

int rp_GenGetBurstCount(int unsigned channel, uint32_t *num) {
    *num = generate->properties_ch[channel].cyclesInOneBurst;
    return RP_OK;
}

int rp_GenBurstRepetitions(int unsigned channel, int repetitions) {
    if ((repetitions < BURST_REPETITIONS_MIN || repetitions > BURST_REPETITIONS_MAX) && repetitions != -1)
        return RP_EOOR;
    ch_burstRepetition[channel] = repetitions;
    if (repetitions == -1)
        repetitions = 0;
    generate_setBurstRepetitions(channel, (uint32_t) (repetitions-1));
    // trigger channel if internal trigger source
    return triggerIfInternal(channel);
}

int rp_GenGetBurstRepetitions(int unsigned channel, int *repetitions) {
    *repetitions = generate->properties_ch[channel].burstRepetitions + 1;
    return RP_OK;
}

int rp_GenBurstPeriod(int unsigned channel, uint32_t period) {
    if (period < BURST_PERIOD_MIN || period > BURST_PERIOD_MAX)
        return RP_EOOR;
    int burstCount;
    burstCount = ch_burstCount[channel];
    // period = signal_time * burst_count + delay_time
    int delay = (int) (period - (1 / ch_frequency[channel] * MICRO) * burstCount);
    if (delay <= 0)
        delay = 1; // if delay is 0, then FPGA generates continuous signal
    generate_setBurstDelay(channel, (uint32_t) delay);
    ch_burstPeriod[channel] = period;
    // trigger channel if internal trigger source
    return triggerIfInternal(channel);
}

int rp_GenGetBurstPeriod(int unsigned channel, uint32_t *period) {
    uint32_t delay, burstCount;
    float frequency;
    delay = generate->properties_ch[channel].delayBetweenBurstRepetitions;
    rp_GenGetBurstCount(channel, &burstCount);
    rp_GenGetFreq(channel, &frequency);
    if (delay == 1) // if delay is 0, then FPGA generates continuous signal
        delay = 0;
    *period = (uint32_t) (delay + (1 / frequency * MICRO) * burstCount);
    return RP_OK;
}

int rp_GenTriggerSource(int unsigned channel, rp_trig_src_t src) {
    if (src == RP_GEN_TRIG_GATED_BURST) {
        generate_setGatedBurst(channel, 1);
        rp_GenMode(channel, RP_GEN_MODE_BURST);
        return generate_setTriggerSource(channel, 2);
    } else {
        generate_setGatedBurst(channel, 0);
    }

    if (src == RP_GEN_TRIG_SRC_INTERNAL) {
        rp_GenMode(channel, RP_GEN_MODE_CONTINUOUS);
        return generate_setTriggerSource(channel, 1);
    } else if (src == RP_GEN_TRIG_SRC_EXT_PE) {
        rp_GenMode(channel, RP_GEN_MODE_BURST);
        return generate_setTriggerSource(channel, 2);
    } else if (src == RP_GEN_TRIG_SRC_EXT_NE) {
        rp_GenMode(channel, RP_GEN_MODE_BURST);
        return generate_setTriggerSource(channel, 3);
    } else {
        return RP_EIPV;
    }
}

int rp_GenGetTriggerSource(int unsigned channel, rp_trig_src_t *src) {
    uint32_t gated;
    if (channel == 0)  gated = generate->AgatedBursts;
    else               gated = generate->BgatedBursts;
    if (gated == 1)
        *src = RP_GEN_TRIG_GATED_BURST;
    else
        generate_getTriggerSource(channel, (uint32_t *) &src);
    return RP_OK;
}

int rp_GenTrigger(uint32_t channel) {
    switch (channel) {
        case 0:
        case 1:
            rp_GenMode(channel, RP_GEN_MODE_BURST);
            return generate_setTriggerSource(channel, RP_GEN_TRIG_SRC_INTERNAL);
        case 2:
        case 3:
            rp_GenMode(0, RP_GEN_MODE_BURST);
            rp_GenMode(1, RP_GEN_MODE_BURST);
            // simultaneously trigger both channels
            return cmn_SetBits((uint32_t *) generate, 0x00010001, 0xFFFFFFFF);
        default:
            return RP_EOOR;
    }
}

