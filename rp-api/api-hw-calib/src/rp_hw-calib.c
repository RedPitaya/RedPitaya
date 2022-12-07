/**
 * $Id: $
 *
 * @brief Red Pitaya library API Hardware interface implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <stdint.h>

#include "rp_hw-calib.h"
#include "calib.h"


int rp_CalibInit(){
    return calib_Init(false);
}

int rp_CalibInitSpecific(rp_HPeModels_t model){
    return calib_InitModel(model,false);
}

rp_calib_params_t rp_GetCalibrationSettings(){
    return calib_GetParams();
}

rp_calib_params_t rp_GetDefaultCalibrationSettings(){
    return calib_GetDefaultCalib();
}

int rp_CalibrationReset(bool use_factory_zone){
    return calib_Reset(use_factory_zone);
}

int rp_CalibrationFactoryReset(){
    return calib_LoadFromFactoryZone();
}

int rp_CalibrationWriteParams(rp_calib_params_t calib_params,bool use_factory_zone){
    return calib_WriteDirectlyParams(&calib_params,use_factory_zone);
}

int rp_CalibrationSetParams(rp_calib_params_t calib_params){
    return calib_SetParams(&calib_params);
}

int rp_CalibGetEEPROM(rp_eepromWpData_t *calib_params,bool use_factory_zone){
    return calib_GetEEPROM(calib_params,use_factory_zone);
}

int rp_CalibConvertEEPROM(rp_eepromWpData_t *calib_params,rp_calib_params_t *out){
    return calib_ConvertEEPROM(calib_params,out);
}

int rp_CalibPrint(rp_calib_params_t *calib){
    return calib_Print(calib);
}

int rp_CalibGetFastADCFilter(rp_channel_calib_t channel,channel_filter_t *out){
    return calib_GetFastADCFilter(channel,out);
}

int rp_CalibGetFastADCFilter_1_20(rp_channel_calib_t channel,channel_filter_t *out){
    return calib_GetFastADCFilter_1_20(channel,out);
}

int rp_CalibGetFastADCCalibValue(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, double *gain,int32_t *offset){
    uint_gain_calib_t c;
    return calib_GetFastADCCalibValue(channel,mode,gain,offset,&c);
}

int rp_CalibGetFastADCCalibValue_1_20(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, double *gain,int32_t *offset){
    uint_gain_calib_t c;
    return calib_GetFastADCCalibValue_1_20(channel,mode,gain,offset,&c);
}

int rp_CalibGetFastADCCalibValue_1_20I(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, uint_gain_calib_t *calib){
    double gain;
    return calib_GetFastADCCalibValue(channel,mode,&gain,&calib->offset,calib);
}

int rp_CalibGetFastADCCalibValueI(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, uint_gain_calib_t *calib){
    double gain;
    return calib_GetFastADCCalibValue_1_20(channel,mode,&gain,&calib->offset,calib);
}

int rp_CalibGetFastDACCalibValue(rp_channel_calib_t channel,rp_gen_gain_calib_t mode, double *gain,int32_t *offset){
    uint_gain_calib_t c;
    return calib_GetFastDACCalibValue(channel,mode,gain,offset,&c);
}

int rp_CalibGetFastDACCalibValueI(rp_channel_calib_t channel,rp_gen_gain_calib_t mode, uint_gain_calib_t *calib){
    double gain;
    return calib_GetFastDACCalibValue(channel,mode,&gain,&calib->offset,calib);
}
