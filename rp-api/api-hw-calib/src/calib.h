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
#include "calib_common.h"
#include "rp_hw_calib.h"

rp_calib_error calib_Init(bool use_factory_zone);
rp_calib_error calib_InitModel(rp_HPeModels_t model, bool use_factory_zone, bool adjust);
rp_calib_error calib_InitModelEx(rp_HPeModels_t model, bool use_factory_zone, rp_calib_params_t* calib, bool adjust);

rp_calib_params_t calib_GetParams();
rp_calib_params_t calib_GetDefaultCalib(bool setFilterZero);
rp_calib_params_t calib_GetUniversalDefaultCalib(bool setFilterZero, uint8_t version);

rp_calib_error calib_GetVersion(uint8_t* version);

rp_calib_error calib_WriteParams(rp_HPeModels_t model, rp_calib_params_t* calib_params, bool use_factory_zone, bool skip_recalculate);
rp_calib_error calib_SetParams(rp_calib_params_t* calib_params);
rp_calib_error calib_WriteDirectlyParams(rp_calib_params_t* calib_params, bool use_factory_zone, bool skip_recalculate);

void calib_SetToZero(bool is_new_format, bool setFilterZero, uint8_t version);
rp_calib_error calib_LoadFromFactoryZone(bool convert_to_new);
rp_calib_error calib_Reset(bool use_factory_zone, bool is_new_format, bool setFilterZero, uint8_t version);
rp_calib_error calib_GetEEPROM(uint8_t** data, uint16_t* size, bool use_factory_zone);
rp_calib_error calib_ConvertEEPROM(uint8_t* data, uint16_t size, rp_calib_params_t* out);
rp_calib_error calib_ConvertToOld(rp_calib_params_t* out);

rp_calib_error calib_Print(rp_calib_params_t* calib);
rp_calib_error calib_PrintEx(FILE* __restrict out, rp_calib_params_t* calib);

rp_calib_error calib_GetFastADCFilter(rp_channel_calib_t channel, channel_filter_t* out);
rp_calib_error calib_GetFastADCFilter_1_20(rp_channel_calib_t channel, channel_filter_t* out);

rp_calib_error calib_GetFastADCCalibValue(rp_channel_calib_t channel, rp_acq_ac_dc_mode_calib_t mode, double* gain, int32_t* offset, uint_gain_calib_t* calib);
rp_calib_error calib_GetFastADCCalibValue_1_20(rp_channel_calib_t channel, rp_acq_ac_dc_mode_calib_t mode, double* gain, int32_t* offset, uint_gain_calib_t* calib);

rp_calib_error calib_GetFastDACCalibValue(rp_channel_calib_t channel, rp_gen_gain_calib_t mode, double* gain, int32_t* offset, uint_gain_calib_t* calib);

#endif  //__CALIB_H
