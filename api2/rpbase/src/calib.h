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

#include "redpitaya/rp.h"

#define CONSTANT_SIGNAL_AMPLITUDE 0.8

int calib_Init();
int calib_Release();

int calib_GetParams(rp_calib_params_t *calib_params);
int calib_SetParams(rp_calib_params_t *calib_params);

int calib_ReadParams(rp_calib_params_t *calib_params);
int calib_WriteParams(rp_calib_params_t *calib_params);

int calib_Reset();

#endif //__CALIB_H
