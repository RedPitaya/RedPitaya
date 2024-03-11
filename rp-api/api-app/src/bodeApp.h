/**
 * $Id: $
 *
 * @brief Red Pitaya library Bode analyzer module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */


#ifndef __BODE_API_H
#define __BODE_API_H

#include <stdint.h>
#include <stdbool.h>
#include <vector>
#include <cstddef>
#include "rp.h"

#define BA_CALIB_FILENAME "/tmp/ba_calib.data"

#ifdef __cplusplus
extern "C" {
#endif


struct rp_ba_buffer_t{
	std::vector<float> ch1;
	std::vector<float> ch2;
	explicit rp_ba_buffer_t(size_t size): ch1(size), ch2(size) {}
};

int rpApp_BaDataAnalysis(const rp_ba_buffer_t &buffer,uint32_t size, float samplesPerSecond,float _freq,  int  samples_period, float *gain, float *phase_out);
int rpApp_BaSafeThreadAcqPrepare();
int rpApp_BaSafeThreadGen(rp_channel_t _channel, float _frequency, float _ampl, float _dc_bias);
int rpApp_BaSafeThreadAcqData(rp_ba_buffer_t &_buffer, rp_acq_decimation_t _decimation, int _acq_size, int _dec, float _trigger);
int rpApp_BaGetAmplPhase(float _amplitude_in, float _dc_bias, int _periods_number, rp_ba_buffer_t &_buffer, float* _amplitude, float* _phase, float _freq,float _input_threshold);

float rpApp_BaCalibGain(float _freq, float _ampl);
float rpApp_BaCalibPhase(float _freq, float _phase);
int rpApp_BaResetCalibration();
int rpApp_BaReadCalibration();
int rpApp_BaWriteCalib(float _current_freq,float _amplitude,float _phase_out);
bool rpApp_BaGetCalibStatus();

uint8_t rpApp_BaGetADCChannels();
uint8_t rpApp_BaGetDACChannels();
uint32_t rpApp_BaGetADCSpeed();

#ifdef __cplusplus
}
#endif

#endif // __BA_API_H
