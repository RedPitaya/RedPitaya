	/**
 * $Id: $
 *
 * @brief Red Pitaya Calibration Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __CALIB_H
#define __CALIB_H

#include <stdint.h>

#include "redpitaya/rp1.h"

#define CONSTANT_SIGNAL_AMPLITUDE 0.8

int calib_Init();
int calib_Release();

rp_calib_params_t calib_GetParams();
int calib_WriteParams(rp_calib_params_t calib_params);
void calib_SetToZero();

float   calib_FullScaleToVoltage  (uint32_t cnt);
int32_t calib_FullScaleFromVoltage(float    voltage);
int32_t calib_Saturate(int unsigned bits, int32_t cnt);

int32_t calib_GetAcqOffset(rp_channel_t channel, rp_pinState_t gain);
float   calib_GetAcqScale (rp_channel_t channel, rp_pinState_t gain);
int     calib_SetAcqOffset(rp_channel_t channel, rp_pinState_t gain, rp_calib_params_t* out_params);
int     calib_SetAcqScale (rp_channel_t channel, rp_pinState_t gain, float referentialVoltage, rp_calib_params_t* out_params);

int32_t calib_GetGenOffset(rp_channel_t channel);
float   calib_GetGenScale (rp_channel_t channel);
int     calib_SetGenOffset(rp_channel_t channel);
int     calib_SetGenScale (rp_channel_t channel);

int calib_CalibrateGen(rp_channel_t channel, rp_calib_params_t* out_params);

int calib_Reset();

int32_t calib_GetDataMedian     (rp_channel_t channel, rp_pinState_t gain);
float   calib_GetDataMedianFloat(rp_channel_t channel, rp_pinState_t gain);
int     calib_GetDataMinMaxFloat(rp_channel_t channel, rp_pinState_t gain, float* min, float* max);

int calib_setCachedParams();
#endif //__CALIB_H
