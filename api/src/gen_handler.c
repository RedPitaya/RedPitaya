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

#include <float.h>
#include "math.h"
#include "common.h"
#include "generate.h"
#include "gen_handler.h"

#ifdef Z20_250_12
#include "rp-i2c-max7311-c.h"
#endif

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// global variables
// TODO: should be organized into a system status structure
float         chA_amplitude            = 1, chB_amplitude            = 1;
float         chA_offset               = 0, chB_offset               = 0;
float         chA_dutyCycle            = 0, chB_dutyCycle            = 0;
float         chA_frequency               , chB_frequency               ;
float         chA_phase                = 0, chB_phase                = 0;
int           chA_burstCount           = 1, chB_burstCount           = 1;
int           chA_burstRepetition      = 1, chB_burstRepetition      = 1;
uint32_t      chA_burstPeriod          = 0, chB_burstPeriod          = 0;
rp_waveform_t chA_waveform                , chB_waveform                ;
uint32_t      chA_size     = BUFFER_LENGTH, chB_size     = BUFFER_LENGTH;
uint32_t      chA_arb_size = BUFFER_LENGTH, chB_arb_size = BUFFER_LENGTH;

bool          chA_EnableTempProtection = 0, chB_EnableTempProtection = 0;
bool          chA_LatchTempAlarm       = 0, chB_LatchTempAlarm       = 0;

#ifdef Z20_250_12
rp_gen_gain_t chA_gain                    , chB_gain                    ;
#endif

float chA_arbitraryData[BUFFER_LENGTH];
float chB_arbitraryData[BUFFER_LENGTH];

int gen_SetDefaultValues() {
    gen_Disable(RP_CH_1);
    gen_Disable(RP_CH_2);
    gen_setFrequency(RP_CH_1, 1000);
    gen_setFrequency(RP_CH_2, 1000);
    gen_setBurstRepetitions(RP_CH_1, 1);
    gen_setBurstRepetitions(RP_CH_2, 1);
    gen_setBurstPeriod(RP_CH_1, (uint32_t) (1 / 1000.0 * MICRO));   // period = 1/frequency in us
    gen_setBurstPeriod(RP_CH_2, (uint32_t) (1 / 1000.0 * MICRO));   // period = 1/frequency in us
    gen_setWaveform(RP_CH_1, RP_WAVEFORM_SINE);
    gen_setWaveform(RP_CH_2, RP_WAVEFORM_SINE);
    gen_setOffset(RP_CH_1, 0);
    gen_setOffset(RP_CH_2, 0);
    gen_setAmplitude(RP_CH_1, AMPLITUDE_MAX);
    gen_setAmplitude(RP_CH_2, AMPLITUDE_MAX);
    gen_setDutyCycle(RP_CH_1, 0.5);
    gen_setDutyCycle(RP_CH_2, 0.5);
    gen_setBurstCount(RP_CH_1, 1);
    gen_setBurstCount(RP_CH_2, 1);
    gen_setBurstPeriod(RP_CH_1, BURST_PERIOD_MIN);
    gen_setBurstPeriod(RP_CH_2, BURST_PERIOD_MIN);
    gen_setTriggerSource(RP_CH_1, RP_GEN_TRIG_SRC_INTERNAL);
    gen_setTriggerSource(RP_CH_2, RP_GEN_TRIG_SRC_INTERNAL);
    gen_setPhase(RP_CH_1, 0.0);
    gen_setPhase(RP_CH_2, 0.0);
    gen_setGenMode(RP_CH_1, RP_GEN_MODE_CONTINUOUS);
    gen_setGenMode(RP_CH_2, RP_GEN_MODE_CONTINUOUS);
#ifdef Z20_250_12
    gen_setGainOut(RP_CH_1,RP_GAIN_1X);
    gen_setGainOut(RP_CH_2,RP_GAIN_1X);
#endif
    return RP_OK;
}

int gen_Disable(rp_channel_t channel) {
    return generate_setOutputDisable(channel, true);
}

int gen_Enable(rp_channel_t channel) {
    return generate_setOutputDisable(channel, false);
}

int gen_EnableSync(bool enable){
    return generate_setOutputEnableSync(enable); 
}

int gen_IsEnable(rp_channel_t channel, bool *value) {
    return generate_getOutputEnabled(channel, value);
}

int gen_checkAmplitudeAndOffset(float amplitude, float offset) {
    if (fabs(amplitude) + fabs(offset) > LEVEL_MAX) {
        return RP_EOOR;
    }
    return RP_OK;
}

int gen_setAmplitude(rp_channel_t channel, float amplitude) {
    float offset;
    CHANNEL_ACTION(channel,
            offset = chA_offset,
            offset = chB_offset)
    gen_checkAmplitudeAndOffset(amplitude, offset);

    CHANNEL_ACTION(channel,
            chA_amplitude = amplitude,
            chB_amplitude = amplitude)
#ifdef Z20_250_12
    rp_gen_gain_t gain;
        CHANNEL_ACTION(channel,
            gain = chA_gain,
            gain = chB_gain)
    return generate_setAmplitude(channel, gain , amplitude);
#else
    return generate_setAmplitude(channel, amplitude);
#endif
}

int gen_getAmplitude(rp_channel_t channel, float *amplitude) {
    if (channel == RP_CH_1) {
        *amplitude = chA_amplitude;
        return RP_OK;
    }
    if (channel == RP_CH_2) {
        *amplitude = chB_amplitude;
        return RP_OK;
    }
    return RP_EPN;
}

int gen_setOffset(rp_channel_t channel, float offset) {
    float amplitude;
    CHANNEL_ACTION(channel,
            amplitude = chA_amplitude,
            amplitude = chB_amplitude)
    gen_checkAmplitudeAndOffset(amplitude, offset);

    CHANNEL_ACTION(channel,
            chA_offset = offset,
            chB_offset = offset)
#ifdef Z20_250_12
    rp_gen_gain_t gain;
        CHANNEL_ACTION(channel,
            gain = chA_gain,
            gain = chB_gain)
    return generate_setDCOffset(channel, gain , offset);
#else
    return generate_setDCOffset(channel, offset);
#endif
}

int gen_getOffset(rp_channel_t channel, float *offset) {
    if (channel == RP_CH_1) {
        *offset = chA_offset;
        return RP_OK;
    }
    if (channel == RP_CH_2) {
        *offset = chB_offset;
        return RP_OK;
    }
    return RP_EPN;
}

int gen_setFrequency(rp_channel_t channel, float frequency) {
    if (frequency < FREQUENCY_MIN || frequency > FREQUENCY_MAX) {
        return RP_EOOR;
    }

    if (channel == RP_CH_1) {
        chA_frequency = frequency;
        gen_setBurstPeriod(channel, chA_burstPeriod);
    }
    else if (channel == RP_CH_2) {
        chB_frequency = frequency;
        gen_setBurstPeriod(channel, chB_burstPeriod);
    }
    else {
        return RP_EPN;
    }

    generate_setFrequency(channel, frequency);
    synthesize_signal(channel);
    return gen_Synchronise();
}

int gen_getFrequency(rp_channel_t channel, float *frequency) {
    return generate_getFrequency(channel, frequency);
}



int gen_setPhase(rp_channel_t channel, float phase) {
    if (phase < PHASE_MIN || phase > PHASE_MAX) {
        return RP_EOOR;
    }
    if (phase < 0) {
        phase += 360;
    }
    CHANNEL_ACTION(channel,
            chA_phase = phase,
            chB_phase = phase)

    synthesize_signal(channel);
    return gen_Synchronise();
}

int gen_getPhase(rp_channel_t channel, float *phase) {
    CHANNEL_ACTION(channel,
            *phase = chA_phase,
            *phase = chB_phase)
    return RP_OK;
}

int gen_setWaveform(rp_channel_t channel, rp_waveform_t type) {
    CHANNEL_ACTION(channel,
            chA_waveform = type,
            chB_waveform = type)
    if (type == RP_WAVEFORM_ARBITRARY) {
        CHANNEL_ACTION(channel,
                chA_size = chA_arb_size,
                chB_size = chB_arb_size)
    }
    else{
        CHANNEL_ACTION(channel,
                chA_size = BUFFER_LENGTH,
                chB_size = BUFFER_LENGTH)
    }
    return synthesize_signal(channel);
}

int gen_getWaveform(rp_channel_t channel, rp_waveform_t *type) {
    CHANNEL_ACTION(channel,
            *type = chA_waveform,
            *type = chB_waveform)
    return RP_OK;
}

int gen_setArbWaveform(rp_channel_t channel, float *data, uint32_t length) {
    // Check if data is normalized
    float min = FLT_MAX, max = -FLT_MAX; // initial values
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
    CHANNEL_ACTION(channel,
            pointer = chA_arbitraryData,
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
    	chB_arb_size = length;
        if(chB_waveform==RP_WAVEFORM_ARBITRARY){
        	return synthesize_signal(channel);
        }
    }
    else {
        return RP_EPN;
    }

    return RP_OK;
}

int gen_getArbWaveform(rp_channel_t channel, float *data, uint32_t *length) {
    // If this data was not set, then this method will return incorrect data
    float *pointer;
    if (channel == RP_CH_1) {
        *length = chA_arb_size;
        pointer = chA_arbitraryData;
    }
    else if (channel == RP_CH_2) {
        *length = chB_arb_size;
        pointer = chB_arbitraryData;
    }
    else {
        return RP_EPN;
    }
    for (int i = 0; i < *length; ++i) {
        data[i] = pointer[i];
    }
    return RP_OK;
}

int gen_setDutyCycle(rp_channel_t channel, float ratio) {
    if (ratio < DUTY_CYCLE_MIN || ratio > DUTY_CYCLE_MAX) {
        return RP_EOOR;
    }
    CHANNEL_ACTION(channel,
            chA_dutyCycle = ratio,
            chB_dutyCycle = ratio)
    return synthesize_signal(channel);
}

int gen_getDutyCycle(rp_channel_t channel, float *ratio) {
    CHANNEL_ACTION(channel,
            *ratio = chA_dutyCycle,
            *ratio = chB_dutyCycle)
    return RP_OK;
}

int gen_setGenMode(rp_channel_t channel, rp_gen_mode_t mode) {
    if (mode == RP_GEN_MODE_CONTINUOUS) {
        generate_setGatedBurst(channel, 0);
        generate_setBurstDelay(channel, 0);
        generate_setBurstRepetitions(channel, 0);
        generate_setBurstCount(channel, 0);
        return triggerIfInternal(channel);
    }
    else if (mode == RP_GEN_MODE_BURST) {
        gen_setBurstCount(channel, channel == RP_CH_1 ? chA_burstCount : chB_burstCount);
        gen_setBurstRepetitions(channel, channel == RP_CH_1 ? chA_burstRepetition : chB_burstRepetition);
        gen_setBurstPeriod(channel, channel == RP_CH_1 ? chA_burstPeriod : chB_burstPeriod);
        return RP_OK;
    }
    else if (mode == RP_GEN_MODE_STREAM) {
        return RP_EUF;
    }
    else {
        return RP_EIPV;
    }
}

int gen_getGenMode(rp_channel_t channel, rp_gen_mode_t *mode) {
    uint32_t num;
    generate_getBurstCount(channel, &num);
    if (num != 0) {
        *mode = RP_GEN_MODE_BURST;
    }
    else {
        *mode = RP_GEN_MODE_CONTINUOUS;
    }
    return RP_OK;
}

int gen_setBurstCount(rp_channel_t channel, int num) {
    if ((num < BURST_COUNT_MIN || num > BURST_COUNT_MAX) && num == 0) {
        return RP_EOOR;
    }
    CHANNEL_ACTION(channel,
            chA_burstCount = num,
            chB_burstCount = num)
    if (num == -1) {    // -1 represents infinity. In FPGA value 0 represents infinity
        num = 0;
    }
    generate_setBurstCount(channel, (uint32_t) num);

    // trigger channel if internal trigger source
    return triggerIfInternal(channel);
}

int gen_getBurstCount(rp_channel_t channel, int *num) {
    return generate_getBurstCount(channel, (uint32_t *) num);
}

#ifndef Z20_250_12
int gen_setBurstLastValue(rp_channel_t channel, float amplitude){
    return generate_setBurstLastValue(channel,  amplitude);
}

int gen_getBurstLastValue(rp_channel_t channel, float *amplitude){
    return generate_getBurstLastValue(channel, amplitude);
}
#endif

int gen_setBurstRepetitions(rp_channel_t channel, int repetitions) {
    if ((repetitions < BURST_REPETITIONS_MIN || repetitions > BURST_REPETITIONS_MAX) && repetitions != -1) {
        return RP_EOOR;
    }
    CHANNEL_ACTION(channel,
            chA_burstRepetition = repetitions,
            chB_burstRepetition = repetitions)
    if (repetitions == -1) {
        repetitions = 0;
    }
    generate_setBurstRepetitions(channel, (uint32_t) (repetitions-1));

    // trigger channel if internal trigger source
    return triggerIfInternal(channel);
}

int gen_getBurstRepetitions(rp_channel_t channel, int *repetitions) {
    uint32_t tmp;
    generate_getBurstRepetitions(channel, &tmp);
    *repetitions = tmp+1;
    return RP_OK;
}

int gen_setBurstPeriod(rp_channel_t channel, uint32_t period) {
    if (period < BURST_PERIOD_MIN || period > BURST_PERIOD_MAX) {
        return RP_EOOR;
    }
    int burstCount;
    CHANNEL_ACTION(channel,
            burstCount = chA_burstCount,
            burstCount = chB_burstCount)
    // period = signal_time * burst_count + delay_time
    int delay = (int) (period - (1 / (channel == RP_CH_1 ? chA_frequency : chB_frequency) * MICRO) * burstCount);
    if (delay <= 0) {
        // if delay is 0, then FPGA generates continuous signal
        delay = 1;
    }
    generate_setBurstDelay(channel, (uint32_t) delay);

    CHANNEL_ACTION(channel,
                   chA_burstPeriod = period,
                   chB_burstPeriod = period)

    // trigger channel if internal trigger source
    return triggerIfInternal(channel);
}

int gen_getBurstPeriod(rp_channel_t channel, uint32_t *period) {
    uint32_t delay, burstCount;
    float frequency;
    generate_getBurstDelay(channel, &delay);
    generate_getBurstCount(channel, &burstCount);
    generate_getFrequency(channel, &frequency);

    if (delay == 1) {    // if delay is 0, then FPGA generates continuous signal
        delay = 0;
    }
    *period = (uint32_t) (delay + (1 / frequency * MICRO) * burstCount);
    return RP_OK;
}

int gen_setTriggerSource(rp_channel_t channel, rp_trig_src_t src) {
    if (src == RP_GEN_TRIG_GATED_BURST) {
        generate_setGatedBurst(channel, 1);
        gen_setGenMode(channel, RP_GEN_MODE_BURST);
        return generate_setTriggerSource(channel, 2);
    }
    else {
        generate_setGatedBurst(channel, 0);
    }

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

int gen_getTriggerSource(rp_channel_t channel, rp_trig_src_t *src) {
    uint32_t gated;
    generate_getGatedBurst(channel, &gated);
    if (gated == 1) {
        *src = RP_GEN_TRIG_GATED_BURST;
    }
    else {
        generate_getTriggerSource(channel, (uint32_t *) &src);
    }
    return RP_OK;
}

int gen_Trigger(uint32_t channel) {
    switch (channel) {
        case 0:
        case 1:
            gen_setGenMode(channel, RP_GEN_MODE_BURST);
            return generate_setTriggerSource(channel, RP_GEN_TRIG_SRC_INTERNAL);
        case 2:
        case 3:
            gen_setGenMode(RP_CH_1, RP_GEN_MODE_BURST);
            gen_setGenMode(RP_CH_2, RP_GEN_MODE_BURST);
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
    float dutyCycle, frequency;
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
    uint16_t buf_size = BUFFER_LENGTH;

    switch (waveform) {
        case RP_WAVEFORM_SINE     : synthesis_sin      (data,buf_size);                 break;
        case RP_WAVEFORM_TRIANGLE : synthesis_triangle (data,buf_size);                 break;
        case RP_WAVEFORM_SQUARE   : synthesis_square   (frequency, data,buf_size);      break;
        case RP_WAVEFORM_RAMP_UP  : synthesis_rampUp   (data,buf_size);                 break;
        case RP_WAVEFORM_RAMP_DOWN: synthesis_rampDown (data,buf_size);                 break;
        case RP_WAVEFORM_DC       : synthesis_DC       (data,buf_size);                 break;
        case RP_WAVEFORM_DC_NEG   : synthesis_DC_NEG   (data,buf_size);                 break; 
        case RP_WAVEFORM_PWM      : synthesis_PWM      (dutyCycle, data,buf_size);      break;
        case RP_WAVEFORM_ARBITRARY: synthesis_arbitrary(channel, data, &size); break;
        default:                    return RP_EIPV;
    }
    return generate_writeData(channel, data, phase, size);
}

int synthesis_sin(float *data_out,uint16_t buffSize) {
    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = (float) (sin(2 * M_PI * (float) i / (float) buffSize));
    }
    return RP_OK;
}

int synthesis_triangle(float *data_out,uint16_t buffSize) {
    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = (float) ((asin(sin(2 * M_PI * (float) i / (float) buffSize)) / M_PI * 2));
    }
    return RP_OK;
}

int synthesis_rampUp(float *data_out,uint16_t buffSize) {
    data_out[BUFFER_LENGTH -1] = 0;
    for(int unsigned i = 0; i < BUFFER_LENGTH-1; i++) {
        data_out[BUFFER_LENGTH - i-2] = (float) (-1.0 * (acos(cos(M_PI * (float) i / (float) buffSize)) / M_PI - 1));
    }
    return RP_OK;
}

int synthesis_rampDown(float *data_out,uint16_t buffSize) {
    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = (float) (-1.0 * (acos(cos(M_PI * (float) i / (float) buffSize)) / M_PI - 1));
    }
    return RP_OK;
}

int synthesis_DC(float *data_out,uint16_t buffSize) {
    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = 1.0;
    }
    return RP_OK;
}

int synthesis_DC_NEG(float *data_out,uint16_t buffSize) {
    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = -1.0;
    }
    return RP_OK;
}

int synthesis_PWM(float ratio, float *data_out,uint16_t buffSize) {
    // calculate number of samples that need to be high
    int h = (int) (buffSize/2 * ratio);

    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        if (i < h || i >= buffSize - h) {
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
    CHANNEL_ACTION(channel,
            pointer = chA_arbitraryData,
            pointer = chB_arbitraryData)
    for (int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = pointer[i];
    }
    CHANNEL_ACTION(channel,
            *size = chA_arb_size,
            *size = chB_arb_size)
    return RP_OK;
}

int synthesis_square(float frequency, float *data_out,uint16_t buffSize) {
    // Various locally used constants - HW specific parameters
#ifdef Z20_250_12
        const int trans0 = 1;
        const int trans1 = 100;
#else
        const int trans0 = 30;
        const int trans1 = 300;
#endif
    
    int trans = (int) (frequency / 1e6 * trans1); // 300 samples at 1 MHz

    if (trans <= 10)  trans = trans0;

    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        int unsigned x = (i % buffSize);
        if      ((0 <= x                      ) && (x <  buffSize/2 - trans))  data_out[i] =  1.0f;
        else if ((x >= buffSize/2 - trans) && (x <  buffSize/2        ))  data_out[i] =  1.0f - (2.0f / trans) * (x - (buffSize/2 - trans));
        else if ((0 <= buffSize/2        ) && (x <  buffSize   - trans))  data_out[i] = -1.0f;
        else if ((x >= buffSize   - trans) && (x <  buffSize          ))  data_out[i] = -1.0f + (2.0f / trans) * (x - (buffSize   - trans));
    }

    return RP_OK;
}

int synthesis_square_Z20_250(float frequency, float *data_out,uint16_t buffSize) {

    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        if ((i % buffSize) <  buffSize / 2.0)  
            data_out[i] =  1.0f;
        else  
            data_out[i] = -1.0f;
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
    CHANNEL_ACTION(channel,
            chA_EnableTempProtection = enable,
            chB_EnableTempProtection = enable)
    return generate_setEnableTempProtection(channel, enable);
}

int gen_getEnableTempProtection(rp_channel_t channel, bool *enable) {
    return generate_getEnableTempProtection(channel, enable);
}

int gen_setLatchTempAlarm(rp_channel_t channel, bool status) {
    CHANNEL_ACTION(channel,
            chA_LatchTempAlarm = status,
            chB_LatchTempAlarm = status)
    return generate_setLatchTempAlarm(channel, status);
}

int gen_getLatchTempAlarm(rp_channel_t channel, bool *status) {
    return generate_getLatchTempAlarm(channel, status);
}

int gen_getRuntimeTempAlarm(rp_channel_t channel, bool *status) {
    return generate_getRuntimeTempAlarm(channel, status);
}


#ifdef Z20_250_12

int gen_setGainOut(rp_channel_t channel,rp_gen_gain_t mode){
    rp_gen_gain_t *gain = NULL;

    if (channel == RP_CH_1) {
        gain = &chA_gain;
    }
    else {
        gain = &chB_gain;
    }

    int ch = (channel == RP_CH_1 ? RP_MAX7311_OUT1 : RP_MAX7311_OUT2);
    int status = rp_setGainOut_C(ch,mode  == RP_GAIN_1X ? RP_GAIN_2V : RP_GAIN_10V);
    if (status == RP_OK){
        *gain = mode;
    }else{
        return status;
    }

    float offset;
    CHANNEL_ACTION(channel,
            offset = chA_offset,
            offset = chB_offset)
    return gen_setOffset(channel,offset);
}

int gen_getGainOut(rp_channel_t channel,rp_gen_gain_t *status){
    if (channel == RP_CH_1) {
        *status = chA_gain;
    }
    else {
        *status = chB_gain;
    }
    return RP_OK;
}

#endif