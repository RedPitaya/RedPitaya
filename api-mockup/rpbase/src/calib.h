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

#include "rp.h"

int calib_Init();
int calib_Release();

rp_calib_params_t calib_GetParams();

int calib_ReadParams(rp_calib_params_t *calib_params);

#endif //__CALIB_H
