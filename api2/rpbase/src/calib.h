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

static const char eeprom_device[]="/sys/bus/i2c/devices/0-0050/eeprom";
static const int  eeprom_calib_off=0x0008;

int calib_Init();
int calib_Release();

int calib_GetParams(rp_calib_params_t *calib_params);
int calib_SetParams(rp_calib_params_t *calib_params);

int calib_ReadParams(rp_calib_params_t *calib_params);
int calib_WriteParams(rp_calib_params_t *calib_params);

int calib_Reset();

#endif //__CALIB_H
