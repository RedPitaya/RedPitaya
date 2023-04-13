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
#include <stdio.h>
#include "rp_hw-calib.h"
#include "calib_common.h"

int calib_Init(bool use_factory_zone);
int calib_InitModel(rp_HPeModels_t model,bool use_factory_zone);

rp_calib_params_t calib_GetParams();
rp_calib_params_t calib_GetDefaultCalib();

int calib_WriteParams(rp_HPeModels_t model, rp_calib_params_t *calib_params,bool use_factory_zone);
int calib_SetParams(rp_calib_params_t *calib_params);
int calib_WriteDirectlyParams(rp_calib_params_t *calib_params,bool use_factory_zone);

void calib_SetToZero();
int calib_LoadFromFactoryZone();
int calib_Reset(bool use_factory_zone);
int calib_GetEEPROM(rp_eepromWpData_t *calib_params,bool use_factory_zone);
int calib_ConvertEEPROM(rp_eepromWpData_t *calib_params,rp_calib_params_t *out);

int calib_Print(rp_calib_params_t *calib);
int calib_PrintEx(FILE *__restrict out,rp_calib_params_t *calib);

int calib_GetFastADCFilter(rp_channel_calib_t channel,channel_filter_t *out);
int calib_GetFastADCFilter_1_20(rp_channel_calib_t channel,channel_filter_t *out);

int calib_GetFastADCCalibValue(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, double *gain,int32_t *offset, uint_gain_calib_t *calib);
int calib_GetFastADCCalibValue_1_20(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, double *gain,int32_t *offset, uint_gain_calib_t *calib);

int calib_GetFastDACCalibValue(rp_channel_calib_t channel,rp_gen_gain_calib_t mode, double *gain,int32_t *offset, uint_gain_calib_t *calib);

#endif //__CALIB_H
