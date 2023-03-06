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

auto rp_BaDataAnalysis(const rp_ba_buffer_t &buffer,uint32_t size, float samplesPerSecond,float _freq,  int  samples_period, float *gain, float *phase_out) -> int;
auto rp_BaSafeThreadAcqPrepare() -> int;
auto rp_BaSafeThreadGen(rp_channel_t _channel, float _frequency, float _ampl, float _dc_bias) -> int;
auto rp_BaSafeThreadAcqData(rp_ba_buffer_t &_buffer, rp_acq_decimation_t _decimation, int _acq_size, int _dec, float _trigger) -> int;
auto rp_BaGetAmplPhase(float _amplitude_in, float _dc_bias, int _periods_number, rp_ba_buffer_t &_buffer, float* _amplitude, float* _phase, float _freq,float _input_threshold) -> int;

auto rp_BaCalibGain(float _freq, float _ampl) -> float;
auto rp_BaCalibPhase(float _freq, float _phase) -> float;
auto rp_BaResetCalibration() -> int;
auto rp_BaReadCalibration() -> int;
auto rp_BaWriteCalib(float _current_freq,float _amplitude,float _phase_out) -> int;
auto rp_BaGetCalibStatus() -> bool;

auto rp_BaGetADCChannels() -> uint8_t;
auto rp_BaGetDACChannels() -> uint8_t;
auto rp_BaGetADCSpeed() -> uint32_t;


#endif // __BA_API_H
