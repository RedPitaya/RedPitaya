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
#include "calib_universal.h"


rp_calib_error rp_CalibInit(){
    return calib_Init(false);
}

rp_calib_error rp_CalibInitSpecific(rp_HPeModels_t model){
    return calib_InitModel(model,false,false);
}

rp_calib_params_t rp_GetCalibrationSettings(){
    return calib_GetParams();
}

rp_calib_params_t rp_GetDefaultCalibrationSettings(){
    return calib_GetDefaultCalib(false);
}

rp_calib_error rp_CalibrationReset(bool use_factory_zone,bool is_new_format,bool setFilterZero){
    return calib_Reset(use_factory_zone,is_new_format,setFilterZero);
}

rp_calib_error rp_CalibrationFactoryReset(bool convert_to_new){
    return calib_LoadFromFactoryZone(convert_to_new);
}

rp_calib_error rp_CalibrationWriteParams(rp_calib_params_t calib_params,bool use_factory_zone){
    return calib_WriteDirectlyParams(&calib_params,use_factory_zone,false);
}

rp_calib_error rp_CalibrationWriteParamsEx(rp_calib_params_t calib_params,bool use_factory_zone){
    return calib_WriteDirectlyParams(&calib_params,use_factory_zone,true);
}


rp_calib_error rp_CalibrationSetParams(rp_calib_params_t calib_params){
    return calib_SetParams(&calib_params);
}

rp_calib_error rp_CalibGetEEPROM(uint8_t **data,uint16_t *size,bool use_factory_zone){
    return calib_GetEEPROM(data,size,use_factory_zone);
}

rp_calib_error rp_CalibConvertEEPROM(uint8_t *data,uint16_t size,rp_calib_params_t *out){
    return calib_ConvertEEPROM(data,size,out);
}

rp_calib_error rp_CalibConvertToOld(rp_calib_params_t *out){
    return calib_ConvertToOld(out);
}


rp_calib_error rp_CalibPrint(rp_calib_params_t *calib){
    return calib_Print(calib);
}

rp_calib_error rp_CalibGetFastADCFilter(rp_channel_calib_t channel,channel_filter_t *out){
    return calib_GetFastADCFilter(channel,out);
}

rp_calib_error rp_CalibGetFastADCFilter_1_20(rp_channel_calib_t channel,channel_filter_t *out){
    return calib_GetFastADCFilter_1_20(channel,out);
}

rp_calib_error rp_CalibGetFastADCCalibValue(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, double *gain,int32_t *offset){
    uint_gain_calib_t c;
    return calib_GetFastADCCalibValue(channel,mode,gain,offset,&c);
}

rp_calib_error rp_CalibGetFastADCCalibValue_1_20(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, double *gain,int32_t *offset){
    uint_gain_calib_t c;
    return calib_GetFastADCCalibValue_1_20(channel,mode,gain,offset,&c);
}

rp_calib_error rp_CalibGetFastADCCalibValueI(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, uint_gain_calib_t *calib){
    double gain;
    return calib_GetFastADCCalibValue(channel,mode,&gain,&calib->offset,calib);
}

rp_calib_error rp_CalibGetFastADCCalibValue_1_20I(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, uint_gain_calib_t *calib){
    double gain;
    return calib_GetFastADCCalibValue_1_20(channel,mode,&gain,&calib->offset,calib);
}

rp_calib_error rp_CalibGetFastDACCalibValue(rp_channel_calib_t channel,rp_gen_gain_calib_t gain_mode,  rp_gen_load_calib_t mode,  double *gain,int32_t *offset){
    uint_gain_calib_t c;
    return calib_GetFastDACCalibValue(channel,gain_mode,mode ,gain,offset,&c);
}

rp_calib_error rp_CalibGetFastDACCalibValueI(rp_channel_calib_t channel,rp_gen_gain_calib_t gain_mode, rp_gen_load_calib_t mode,  uint_gain_calib_t *calib){
    double gain;
    return calib_GetFastDACCalibValue(channel,gain_mode,mode,&gain,&calib->offset,calib);
}

rp_calib_error rp_GetNameOfUniversalId(uint16_t id, char** name){
    *name = getNameOfUniversalId(id);
    return name ? RP_HP_OK : RP_HW_CALIB_EIP;
}
