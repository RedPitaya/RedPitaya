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


#ifndef __BA_API_H
#define __BA_API_H

#include <stdint.h>
#include <stdbool.h>
#include <vector>
#include <cstddef>
#include "rp.h"

#define BA_CALIB_FILENAME "/tmp/ba_calib.data"

struct rp_ba_buffer_t{
	std::vector<float> ch1;
	std::vector<float> ch2;
	explicit rp_ba_buffer_t(size_t size): ch1(size), ch2(size) {}
};

  int rp_BaDataAnalysis(const rp_ba_buffer_t &buffer,uint32_t size,	float w_out, int decimation, float *gain, float *phase_out,float input_threshold);
  int rp_BaSafeThreadAcqPrepare();
  int rp_BaSafeThreadGen(rp_channel_t _channel, float _frequency, float _ampl, float _dc_bias);
  int rp_BaSafeThreadAcqData(rp_ba_buffer_t &_buffer, rp_acq_decimation_t _decimation, int _acq_size, int _dec, float _trigger);
  int rp_BaGetAmplPhase(float _amplitude_in, float _dc_bias, int _periods_number, rp_ba_buffer_t &_buffer, float* _amplitude, float* _phase, float _freq,float _input_threshold);

float rp_BaCalibGain(float _freq, float _ampl);
float rp_BaCalibPhase(float _freq, float _phase);
  int rp_BaResetCalibration();
  int rp_BaReadCalibration();
  int rp_BaWriteCalib(float _current_freq,float _amplitude,float _phase_out);
 bool rp_BaGetCalibStatus();
 
#endif // __BA_API_H
