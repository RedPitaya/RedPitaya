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

#ifndef GENERATE_HANDLER_H_
#define GENERATE_HANDLER_H_


#include "rp.h"

int gen_SetDefaultValues();
int gen_Disable(rp_channel_t chanel);
int gen_Enable(rp_channel_t chanel);
int gen_setAmplitude(rp_channel_t chanel, double amplitude);
int gen_Offset(rp_channel_t chanel, double offset) ;
int gen_Frequency(rp_channel_t chanel, double frequency);
int gen_Phase(rp_channel_t chanel, double phase);
int gen_Waveform(rp_channel_t chanel, rp_waveform_t type);
int gen_ArbWaveform(rp_channel_t chanel, float *data, uint32_t length);
int gen_DutyCycle(rp_channel_t chanel, double ratio);
int gen_GenMode(rp_channel_t chanel, rp_gen_mode_t mode);
int gen_BurstCount(rp_channel_t channel, int num);
int gen_BurstRepetitions(rp_channel_t channel, int repetitions);
int gen_BurstPeriod(rp_channel_t channel, uint32_t period);
int gen_TriggerSource(rp_channel_t chanel, rp_trig_src_t src);
int gen_Trigger(int mask);
int gen_Synchronise();

int synthesize_signal(rp_channel_t chanel);
int synthesis_sin(float *data_out);
int synthesis_triangle(float *data_out);
int synthesis_arbitrary(rp_channel_t channel, float *data_out, uint32_t * size);
int synthesis_square(double frequency, float *data_out);
int synthesis_rampUp(float *data_out);
int synthesis_rampDown(float *data_out);
int synthesis_DC(float *data_out);
int synthesis_PWM(double ratio, float *data_out);

#endif
