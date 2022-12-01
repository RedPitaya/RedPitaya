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

int rp_CalibrationReset(){
    return calib_Reset();
}

int rp_CalibrationFactoryReset(){
    return calib_LoadFromFactoryZone();
}

int rp_CalibrationWriteParams(rp_calib_params_t calib_params){
    return calib_WriteDirectlyParams(&calib_params);
}

int rp_CalibrationSetParams(rp_calib_params_t calib_params){
    return calib_SetParams(&calib_params);
}