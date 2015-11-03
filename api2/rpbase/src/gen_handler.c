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
#include <math.h>
#include "common.h"
#include "generate.h"
#include "gen_handler.h"

#include "redpitaya/rp.h"

// global variables
// TODO: should be organized into a system status structure
float         ch_frequency       [2] = {0,0};
float         ch_phase           [2] = {0,0};
int           ch_burstCount      [2] = {1,1};
int           ch_burstRepetition [2] = {1,1};
uint32_t      ch_burstPeriod     [2] = {0,0};
rp_waveform_t ch_waveform        [2]        ;
uint32_t      ch_size            [2] = {BUFFER_LENGTH, BUFFER_LENGTH};
uint32_t      ch_arb_size        [2] = {BUFFER_LENGTH, BUFFER_LENGTH};

float ch_arbitraryData[2][BUFFER_LENGTH];


int gen_checkAmplitudeAndOffset(float amplitude, float offset) {
    if (fabs(amplitude) + fabs(offset) > LEVEL_MAX) {
        return RP_EOOR;
    }
    return RP_OK;
}

int gen_Synchronise() {
    return generate_Synchronise();
}

int synthesize_signal(rp_channel_t channel) {
    float data[BUFFER_LENGTH];
    rp_waveform_t waveform;
    float frequency;
    uint32_t size, phase;

    if (channel > RP_CH_2) {
        return RP_EPN;
    }
    waveform = ch_waveform[channel];
    frequency = ch_frequency[channel];
    size = ch_size[channel];
    phase = (uint32_t) (ch_phase[channel] * BUFFER_LENGTH / 360.0);

    switch (waveform) {
        case RP_WAVEFORM_SINE     : synthesis_sin      (data);                 break;
        case RP_WAVEFORM_TRIANGLE : synthesis_triangle (data);                 break;
        case RP_WAVEFORM_SQUARE   : synthesis_square   (frequency, data);      break;
        case RP_WAVEFORM_RAMP_UP  : synthesis_rampUp   (data);                 break;
        case RP_WAVEFORM_RAMP_DOWN: synthesis_rampDown (data);                 break;
        case RP_WAVEFORM_DC       : synthesis_DC       (data);                 break;
        case RP_WAVEFORM_ARBITRARY: synthesis_arbitrary(channel, data, &size); break;
        default:                    return RP_EIPV;
    }
    return generate_writeData(channel, data, phase, size);
}

int synthesis_sin(float *data_out) {
    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = (float) (sin(2 * M_PI * (float) i / (float) BUFFER_LENGTH));
    }
    return RP_OK;
}

int synthesis_triangle(float *data_out) {
    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = (float) ((asin(sin(2 * M_PI * (float) i / (float) BUFFER_LENGTH)) / M_PI * 2));
    }
    return RP_OK;
}

int synthesis_rampUp(float *data_out) {
    data_out[BUFFER_LENGTH -1] = 0;
    for(int unsigned i = 0; i < BUFFER_LENGTH-1; i++) {
        data_out[BUFFER_LENGTH - i-2] = (float) (-1.0 * (acos(cos(M_PI * (float) i / (float) BUFFER_LENGTH)) / M_PI - 1));
    }
    return RP_OK;
}

int synthesis_rampDown(float *data_out) {
    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = (float) (-1.0 * (acos(cos(M_PI * (float) i / (float) BUFFER_LENGTH)) / M_PI - 1));
    }
    return RP_OK;
}

int synthesis_DC(float *data_out) {
    for(int unsigned i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = 1.0;
    }
    return RP_OK;
}

int synthesis_PWM(float ratio, float *data_out) {
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

int synthesis_square(float frequency, float *data_out) {
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

