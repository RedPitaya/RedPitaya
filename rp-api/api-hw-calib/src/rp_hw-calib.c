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