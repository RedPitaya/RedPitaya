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
#include "math.h"
#include "common.h"
#include "generate.h"
#include "gen_handler.h"

double chA_amplitude = 1, chB_amplitude = 1;
double chA_dutyCycle, chB_dutyCycle;
double chA_frequency, chB_frequency;
rp_waveform_t chA_waveform, chB_waveform;

int gen_Disable(rp_channel_t chanel) {
    return generate_setOutputDisable(chanel, true);
}

int gen_Enable(rp_channel_t chanel) {
    return generate_setOutputDisable(chanel, false);
}

int gen_setAmplitude(rp_channel_t chanel, double amplitude) {
    if (amplitude < AMPLITUDE_MIN || amplitude > AMPLITUDE_MAX) {
        return RP_EOOR;
    }
    CHECK_OUTPUT(chA_amplitude = amplitude,
                 chB_amplitude = amplitude)

    ECHECK(synthesize_signal(chanel));
    return RP_OK;
}

int gen_Offset(rp_channel_t chanel, double offset) {
    if (offset < OFFSET_MIN || offset > OFFSET_MAX) {
        return RP_EOOR;
    }
    ECHECK(generate_setDCOffset(chanel, cmn_CnvVToCnt(DATA_BIT_LENGTH, (float) offset, OFFSET_MAX/2, 0, 0)));
    ECHECK(synthesize_signal(chanel));
    return RP_OK;
}

int gen_Frequency(rp_channel_t chanel, double frequency) {
    if (frequency < FREQUENCY_MIN || frequency > FREQUENCY_MAX) {
        return RP_EOOR;
    }
    CHECK_OUTPUT(chA_frequency = frequency,
                 chB_frequency = frequency)
    ECHECK(generate_setFrequency(chanel, (uint32_t) round(65536 * frequency / DAC_FREQUENCY * BUFFER_LENGTH)));
    ECHECK(synthesize_signal(chanel));
    return RP_OK;
}

int gen_Phase(rp_channel_t chanel, double phase) {
    if (phase < PHASE_MIN || phase > PHASE_MAX) {
        return RP_EOOR;
    }
    ECHECK(generate_setPhase(chanel, (uint32_t) (phase / PHASE_MAX * BUFFER_LENGTH / 2)));
    ECHECK(synthesize_signal(chanel));
    return RP_OK;
}

int gen_Waveform(rp_channel_t chanel, rp_waveform_t type) {
    CHECK_OUTPUT(chA_waveform = type,
                 chB_waveform = type)

    ECHECK(synthesize_signal(chanel));
    return RP_OK;
}

int gen_ArbWaveform(rp_channel_t chanel, float *waveform, uint32_t length) {
    CHECK_OUTPUT(chA_waveform = RP_WAVEFORM_ARBITRARY,
                 chB_waveform = RP_WAVEFORM_ARBITRARY)

    return generate_writeData(chanel, waveform, length);
}

int gen_DutyCycle(rp_channel_t chanel, double ratio) {
    if (ratio < DUTY_CYCLE_MIN || ratio > DUTY_CYCLE_MAX) {
        return RP_EOOR;
    }
    CHECK_OUTPUT(chA_dutyCycle = ratio,
                 chB_dutyCycle = ratio)

    ECHECK(synthesize_signal(chanel));
    return RP_OK;
}

int gen_GenMode(rp_channel_t chanel, rp_gen_mode_t mode) {
    if (mode == RP_GEN_MODE_CONTINUOUS) {
        generate_setOneTimeTrigger(chanel, 0);
        return RP_OK;
    }
    else if (mode == RP_GEN_MODE_BURST) {
        return RP_EUF;
    }
    else if (mode == RP_GEN_MODE_STREAM) {
        //TODO
        return RP_OK;
    }
    else {
        return RP_EIPV;
    }
}

int gen_BurstCount(rp_channel_t chanel, int num) {
    if (num != 1) {
        return RP_EUF;
    }
    return RP_OK;
}

int gen_TriggerSource(rp_channel_t chanel, rp_trig_src_t src) {
    if (src == RP_TRIG_SRC_INTERNAL) {
        return generate_setTriggerSource(chanel, 1);
    }
    else if(src == RP_TRIG_SRC_EXTERNAL) {
        return generate_setTriggerSource(chanel, 3);
    }
    else {
        return RP_EIPV;
    }
}

int gen_Trigger(int mask) {
    switch (mask) {
        case 1:
            return generate_GenTrigger(RP_CH_A);
        case 2:
            return generate_GenTrigger(RP_CH_B);
        case 3:
            ECHECK(generate_GenTrigger(RP_CH_A));
            ECHECK(generate_GenTrigger(RP_CH_B));
            return RP_OK;
        default:
            return RP_EOOR;
    }
}

int synthesize_signal(rp_channel_t chanel) {
    float data[BUFFER_LENGTH];
    rp_waveform_t waveform;
    double amplitude, dutyCycle, frequency;

    if (chanel == RP_CH_A) {
        waveform = chA_waveform;
        amplitude = chA_amplitude;
        dutyCycle = chA_dutyCycle;
        frequency = chA_frequency;
    }
    else if (chanel == RP_CH_B) {
        waveform = chB_waveform;
        amplitude = chB_amplitude;
        dutyCycle = chB_dutyCycle;
        frequency = chB_frequency;
    }
    else{
        return RP_EPN;
    }

    switch (waveform) {
        case RP_WAVEFORM_SINE:
            synthesis_sin(amplitude, data);
            break;
        case RP_WAVEFORM_TRIANGLE:
            synthesis_triangle(amplitude, data);
            break;
        case RP_WAVEFORM_SQUARE:
            synthesis_square(amplitude, frequency, data);
            break;
        case RP_WAVEFORM_RAMP_UP:
            synthesis_rampUp(amplitude, data);
            break;
        case RP_WAVEFORM_RAMP_DOWN:
            synthesis_rampDown(amplitude, data);
            break;
        case RP_WAVEFORM_DC:
            synthesis_DC(amplitude, data);
            break;
        case RP_WAVEFORM_PWM:
            synthesis_PWM(amplitude, dutyCycle, data);
            break;
        case RP_WAVEFORM_ARBITRARY:
            // don't do anything
            break;
        default:
            return RP_EIPV;
    }
    return generate_writeData(chanel, data, BUFFER_LENGTH);
}

int synthesis_sin(double amplitude, float *data_out) {
    int i;
    for(i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = (float) (amplitude * cos(2 * M_PI * (double) i / (double) BUFFER_LENGTH));
    }
    return RP_OK;
}

int synthesis_triangle(double amplitude, float *data_out) {
    int i;
    for(i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = (float) (-1.0 * amplitude * (acos(cos(2 * M_PI * (double) i / (double) BUFFER_LENGTH)) / M_PI * 2 - 1));
    }
    return RP_OK;
}

int synthesis_rampUp(double amplitude, float *data_out) {
    int i;
    for(i = 0; i < BUFFER_LENGTH; i++) {
        data_out[BUFFER_LENGTH - i-1] = (float) (-amplitude * (acos(cos(M_PI * (double) i / (double) BUFFER_LENGTH)) / M_PI - 1));
    }
    return RP_OK;
}

int synthesis_rampDown(double amplitude, float *data_out) {
    int i;
    for(i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = (float) (-amplitude * (acos(cos(M_PI * (double) i / (double) BUFFER_LENGTH)) / M_PI - 1));
    }
    return RP_OK;
}

int synthesis_DC(double amplitude, float *data_out) {
    int i;
    for(i = 0; i < BUFFER_LENGTH; i++) {
        data_out[i] = (float) amplitude;
    }
    return RP_OK;
}

int synthesis_PWM(double amplitude, double ratio, float *data_out) {
    int i;

    // calculate number of samples that need to be high
    int h = (int) (BUFFER_LENGTH/2 * ratio);

    for(i = 0; i < BUFFER_LENGTH; i++) {
        if (i < h || i >= BUFFER_LENGTH - h) {
            data_out[i] = (float) amplitude;
        }
        else {
            data_out[i] = (float) -amplitude;
        }
    }
    return 0;
}

int synthesis_square(double amplitude, double frequency, float *data_out) {
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
        data_out[i] = (float) (amplitude * cos(2 * M_PI * (double) i / (double) BUFFER_LENGTH));
        if (data_out[i] > 0)
            data_out[i] = (float) amplitude;
        else
            data_out[i] = (float) -amplitude;

        // Soft linear transitions
        double mm, qq, xx, xm;
        double x1, x2, y1, y2;

        xx = i;
        xm = BUFFER_LENGTH;

        x1 = xm * tt2;
        x2 = xm * tt2 + (double) trans;

        if ((xx > x1) && (xx <= x2)) {

            y1 = amplitude;
            y2 = -amplitude;

            mm = (y2 - y1) / (x2 - x1);
            qq = y1 - mm * x1;

            data_out[i] = (int32_t) round(mm * xx + qq);
        }

        x1 = xm * 0.75;
        x2 = xm * 0.75 + trans;

        if ((xx > x1) && (xx <= x2)) {

            y1 = -amplitude;
            y2 = amplitude;

            mm = (y2 - y1) / (x2 - x1);
            qq = y1 - mm * x1;

            data_out[i] = (float) (mm * xx + qq);
        }
    }
    return RP_OK;
}