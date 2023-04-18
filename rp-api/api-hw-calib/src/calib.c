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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "calib.h"

static rp_calib_params_t g_calib;
static bool g_model_loaded = false;
static rp_HPeModels_t g_model = STEM_125_10_v1_0;

int calib_InitModel(rp_HPeModels_t model,bool use_factory_zone){
    switch (model)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:{
            uint16_t size = sizeof(rp_calib_params_v1_t);
            uint8_t* buffer = use_factory_zone ? readFromFactoryEpprom(&size) : readFromEpprom(&size);
            if (buffer && size == sizeof(rp_calib_params_v1_t)){
                rp_calib_params_v1_t calib_v1;
                memcpy(&calib_v1,buffer,size);
                g_calib = convertV1toCommon(&calib_v1);
                if (buffer) free(buffer);

            }else{
                if (buffer) free(buffer);
                g_calib = getDefault(model);
                fprintf(stderr,"[Error:calib_InitModel] Cann't load calibration v1. Set by default.\n");
                return RP_HW_CALIB_ERE;
            }
            break;
        }

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:{
            uint16_t size = sizeof(rp_calib_params_v1_t);
            uint8_t* buffer = use_factory_zone ? readFromFactoryEpprom(&size) : readFromEpprom(&size);
            if (buffer && size == sizeof(rp_calib_params_v1_t)){
                rp_calib_params_v1_t calib_v1;
                memcpy(&calib_v1,buffer,size);
                g_calib = convertV4toCommon(&calib_v1);
                if (buffer) free(buffer);

            }else{
                if (buffer) free(buffer);
                g_calib = getDefault(model);
                fprintf(stderr,"[Error:calib_InitModel] Cann't load calibration v1. Set by default.\n");
                return RP_HW_CALIB_ERE;
            }
            break;
        }

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:{
            uint16_t size = sizeof(rp_calib_params_v2_t);
            uint8_t* buffer = use_factory_zone ? readFromFactoryEpprom(&size) : readFromEpprom(&size);
            if (buffer && size == sizeof(rp_calib_params_v2_t)){
                rp_calib_params_v2_t calib_v2;
                memcpy(&calib_v2,buffer,size);
                g_calib = convertV2toCommon(&calib_v2);

            }else{
                g_calib = getDefault(model);
                fprintf(stderr,"[Error:calib_InitModel] Cann't load calibration v2. Set by default.\n");
                return RP_HW_CALIB_ERE;
            }
            break;
        }

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_120:{
            uint16_t size = sizeof(rp_calib_params_v3_t);
            uint8_t* buffer = use_factory_zone ? readFromFactoryEpprom(&size) : readFromEpprom(&size);
            if (buffer && size == sizeof(rp_calib_params_v3_t)){
                rp_calib_params_v3_t calib_v3;
                memcpy(&calib_v3,buffer,size);
                g_calib = convertV3toCommon(&calib_v3);
                if (buffer) free(buffer);

            }else{
                if (buffer) free(buffer);
                g_calib = getDefault(model);
                fprintf(stderr,"[Error:calib_InitModel] Cann't load calibration v3. Set by default.\n");
                return RP_HW_CALIB_ERE;
            }
            break;
        }
        default:
            fprintf(stderr,"[Error:calib_InitModel] Unknown model: %d.\n",model);
            break;
    }
    if (!recalculateGain(&g_calib)){
        fprintf(stderr,"[Error:calib_InitModel] Cannot correctly recalculate gain on calibration.\n");
    }
    return RP_HW_CALIB_OK;
}

int calib_Init(bool use_factory_zone){
    rp_HPeModels_t model = STEM_125_14_v1_1; // Default model
    int res = rp_HPGetModel(&model);
    if (res != RP_HP_OK){
        fprintf(stderr,"[Error:calib_Init] Can't load RP model version. Err: %d\n",res);
        g_model_loaded = false;
        g_model = STEM_125_14_v1_1;
    }else{
        g_model_loaded = true;
        g_model = model;
    }

    return calib_InitModel(model,use_factory_zone);
}

int calib_WriteParams(rp_HPeModels_t model, rp_calib_params_t *calib_params,bool use_factory_zone){

    if (!recalculateCalibValue(calib_params)){
        fprintf(stderr,"[Error:calib_WriteParams] Cannot correctly recalculate calib values on calibration.\n");
        return RP_HW_CALIB_EWE;
    }

    switch (model)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:{

            uint16_t size = sizeof(rp_calib_params_v1_t);
            uint8_t* buf = (uint8_t *)malloc(size);
	        if (!buf) {
                fprintf(stderr,"[Error:calib_WriteParams] Memory allocation error.\n");
		        return RP_HW_CALIB_EWE;
	        }
            rp_calib_params_v1_t calib_v1;
            if (!convertV1(calib_params,&calib_v1)){
                free(buf);
                fprintf(stderr,"[Error:calib_WriteParams] Error converting calibration V1 parameters.\n");
                return RP_HW_CALIB_EWE;
            }
            calib_v1.wpCheck++;
            memcpy(buf,&calib_v1,size);
            uint16_t ws = use_factory_zone ? writeToFactoryEpprom(buf,size) : writeToEpprom(buf,size);
            free(buf);
            if (ws != size) {
                fprintf(stderr,"[Error:calib_WriteParams] Eeprom write error.\n");
                return RP_HW_CALIB_EWE;
            }

            break;
        }

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:{

            uint16_t size = sizeof(rp_calib_params_v1_t);
            uint8_t* buf = (uint8_t *)malloc(size);
	        if (!buf) {
                fprintf(stderr,"[Error:calib_WriteParams] Memory allocation error.\n");
		        return RP_HW_CALIB_EWE;
	        }
            rp_calib_params_v1_t calib_v1;
            if (!convertV4(calib_params,&calib_v1)){
                free(buf);
                fprintf(stderr,"[Error:calib_WriteParams] Error converting calibration V4 parameters.\n");
                return RP_HW_CALIB_EWE;
            }
            calib_v1.wpCheck++;
            memcpy(buf,&calib_v1,size);
            uint16_t ws = use_factory_zone ? writeToFactoryEpprom(buf,size) : writeToEpprom(buf,size);
            free(buf);
            if (ws != size) {
                fprintf(stderr,"[Error:calib_WriteParams] Eeprom write error.\n");
                return RP_HW_CALIB_EWE;
            }

            break;
        }

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:{

            uint16_t size = sizeof(rp_calib_params_v2_t);
            uint8_t* buf = (uint8_t *)malloc(size);
	        if (!buf) {
                fprintf(stderr,"[Error:calib_WriteParams] Memory allocation error.\n");
		        return RP_HW_CALIB_EWE;
	        }
            rp_calib_params_v2_t calib_v2;
            if (!convertV2(calib_params,&calib_v2)){
                free(buf);
                fprintf(stderr,"[Error:calib_WriteParams] Error converting calibration V2 parameters.\n");
                return RP_HW_CALIB_EWE;
            }
            calib_v2.wpCheck++;
            memcpy(buf,&calib_v2,size);
            uint16_t ws = use_factory_zone ? writeToFactoryEpprom(buf,size) : writeToEpprom(buf,size);
            free(buf);
            if (ws != size) {
                fprintf(stderr,"[Error:calib_WriteParams] Eeprom write error.\n");
                return RP_HW_CALIB_EWE;
            }
            break;
        }

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_120:{

            uint16_t size = sizeof(rp_calib_params_v3_t);
            uint8_t* buf = (uint8_t *)malloc(size);
	        if (!buf) {
                fprintf(stderr,"[Error:calib_WriteParams] Memory allocation error.\n");
		        return RP_HW_CALIB_EWE;
	        }
            rp_calib_params_v3_t calib_v3;
            if (!convertV3(calib_params,&calib_v3)){
                free(buf);
                fprintf(stderr,"[Error:calib_WriteParams] Error converting calibration V3 parameters.\n");
                return RP_HW_CALIB_EWE;
            }
            calib_v3.wpCheck++;
            memcpy(buf,&calib_v3,size);
            uint16_t ws = use_factory_zone ? writeToFactoryEpprom(buf,size) : writeToEpprom(buf,size);
            free(buf);
            if (ws != size) {
                fprintf(stderr,"[Error:calib_WriteParams] Eeprom write error.\n");
                return RP_HW_CALIB_EWE;
            }
            break;
        }

        default:
            fprintf(stderr,"[Error:calib_WriteParams] Unknown model: %d.\n",model);
            break;
    }
    return RP_HW_CALIB_OK;
}

rp_calib_params_t calib_GetParams()
{
    return g_calib;
}

rp_calib_params_t calib_GetDefaultCalib(){
    if (!g_model_loaded){
        rp_HPeModels_t model = STEM_125_14_v1_1; // Default model
        int res = rp_HPGetModel(&model);
        if (res != RP_HP_OK){
            fprintf(stderr,"[Error:calib_GetDefaultCalib] Can't load RP model version. Err: %d\n",res);
        }
        return getDefault(model);
    }
    return getDefault(g_model);
}

int calib_WriteDirectlyParams(rp_calib_params_t *calib_params,bool use_factory_zone){
    rp_HPeModels_t model = STEM_125_14_v1_1; // Default model
    if (!g_model_loaded){
        int res = rp_HPGetModel(&model);
        if (res != RP_HP_OK){
            fprintf(stderr,"[Error:calib_WriteDirectlyParams] Can't load RP model version. Err: %d\n",res);
            return RP_HW_CALIB_EDM;
        }

    }else{
        model = g_model;
    }
    return calib_WriteParams(model,calib_params,use_factory_zone);
}

void calib_SetToZero() {
    g_calib = calib_GetDefaultCalib();
}

int calib_Reset(bool use_factory_zone) {
    if (g_model_loaded){
        rp_calib_params_t calib = g_calib;
        calib_SetToZero();
        int res = calib_WriteParams(g_model,&g_calib,use_factory_zone);
        if (res != RP_HW_CALIB_OK){
            g_calib = calib;
            return res;
        }
        return calib_Init(use_factory_zone);
    }
    else{
        return RP_HW_CALIB_ENI;
    }
}

int calib_LoadFromFactoryZone(){
    rp_HPeModels_t model = STEM_125_14_v1_1; // Default model
    if (!g_model_loaded){
        int res = rp_HPGetModel(&model);
        if (res != RP_HP_OK){
            fprintf(stderr,"[Error:calib_LoadFromFactoryZone] Can't load RP model version. Err: %d\n",res);
            return RP_HW_CALIB_EDM;
        }

    }else{
        model = g_model;
    }

    uint16_t size = 0;
    switch (model)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:{
            size = sizeof(rp_calib_params_v1_t);
            break;
        }

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:{
            size = sizeof(rp_calib_params_v1_t);
            break;
        }

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:{
            size = sizeof(rp_calib_params_v2_t);
            break;
        }

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_120:{
            size = sizeof(rp_calib_params_v3_t);
            break;
        }

        default:{
            fprintf(stderr,"[Error:calib_LoadFromFactoryZone] Unknown model: %d.\n",model);
            return RP_HW_CALIB_EDM;
            break;
        }
    }

    uint16_t rs = size;
    uint8_t *buffer = readFromFactoryEpprom(&size);

    if (!buffer || rs != size) {
        if (buffer) free(buffer);
        fprintf(stderr,"[Error:calib_LoadFromFactoryZone] Eeprom read from factory zone.\n");
        return RP_HW_CALIB_ERE;
    }
    size = writeToEpprom(buffer,size);
    if (rs != size) {
        fprintf(stderr,"[Error:calib_LoadFromFactoryZone] Eeprom write to eeprom error.\n");
        return RP_HW_CALIB_EWE;
    }
    if (buffer) free(buffer);
    return RP_HW_CALIB_OK;
}

int calib_SetParams(rp_calib_params_t *calib_params){
    g_calib = *calib_params;
    //calib_PrintEx(stderr,&g_calib);
    return RP_HW_CALIB_OK;
}

int calib_GetEEPROM(rp_eepromWpData_t *calib_params,bool use_factory_zone){
    rp_HPeModels_t model = STEM_125_14_v1_1; // Default model
    if (!g_model_loaded){
        int res = calib_Init(use_factory_zone);
        if (res != RP_HP_OK){
            fprintf(stderr,"[Error:calib_GetEEPROM] Err: %d\n",res);
            return res;
        }
        model = g_model;
    }else{
        model = g_model;
    }
    uint16_t size = 0;
    switch (model)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:{
            size = sizeof(rp_calib_params_v1_t);
            rp_calib_params_v1_t p_v1;
            memset(&p_v1,0,size);
            if (!convertV1(&g_calib,&p_v1)){
                fprintf(stderr,"[Error:calib_GetEEPROM] Error converting calibration parameters.\n");
            }
            memcpy(calib_params,&p_v1,size);
            return RP_HW_CALIB_OK;
        }

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:{
            size = sizeof(rp_calib_params_v1_t);
            rp_calib_params_v1_t p_v1;
            memset(&p_v1,0,size);
            if (!convertV4(&g_calib,&p_v1)){
                fprintf(stderr,"[Error:calib_GetEEPROM] Error converting calibration parameters.\n");
            }
            memcpy(calib_params,&p_v1,size);
            return RP_HW_CALIB_OK;
        }

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:{
            size = sizeof(rp_calib_params_v2_t);
            rp_calib_params_v2_t p_v2;
            memset(&p_v2,0,size);
            if (!convertV2(&g_calib,&p_v2)){
                fprintf(stderr,"[Error:calib_GetEEPROM] Error converting calibration parameters.\n");
            }
            memcpy(calib_params,&p_v2,size);
            return RP_HW_CALIB_OK;
        }

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_120:{
            size = sizeof(rp_calib_params_v3_t);
            rp_calib_params_v3_t p_v3;
            memset(&p_v3,0,size);
            if (!convertV3(&g_calib,&p_v3)){
                fprintf(stderr,"[Error:calib_GetEEPROM] Error converting calibration parameters.\n");
            }
            memcpy(calib_params,&p_v3,size);
            return RP_HW_CALIB_OK;
        }

        default:{
            fprintf(stderr,"[Error:calib_LoadFromFactoryZone] Unknown model: %d.\n",model);
            return RP_HW_CALIB_EDM;
            break;
        }
    }
    return RP_HW_CALIB_EDM;
}

int calib_ConvertEEPROM(rp_eepromWpData_t *calib_params,rp_calib_params_t *out){
    if (g_model_loaded){
        uint16_t size = 0;
        switch (g_model)
        {
            case STEM_125_10_v1_0:
            case STEM_125_14_v1_0:
            case STEM_125_14_v1_1:
            case STEM_125_14_LN_v1_1:
            case STEM_125_14_Z7020_v1_0:
            case STEM_125_14_Z7020_LN_v1_1:{
                size = sizeof(rp_calib_params_v1_t);
                rp_calib_params_v1_t p_v1;
                memcpy(&p_v1,calib_params,size);
                *out = convertV1toCommon(&p_v1);
                recalculateGain(out);
                return RP_HW_CALIB_OK;
            }

            case STEM_122_16SDR_v1_0:
            case STEM_122_16SDR_v1_1:{
                size = sizeof(rp_calib_params_v1_t);
                rp_calib_params_v1_t p_v1;
                memcpy(&p_v1,calib_params,size);
                *out = convertV4toCommon(&p_v1);
                recalculateGain(out);
                return RP_HW_CALIB_OK;
            }

            case STEM_125_14_Z7020_4IN_v1_0:
            case STEM_125_14_Z7020_4IN_v1_2:
            case STEM_125_14_Z7020_4IN_v1_3:{
                size = sizeof(rp_calib_params_v2_t);
                rp_calib_params_v2_t p_v2;
                memcpy(&p_v2,calib_params,size);
                *out = convertV2toCommon(&p_v2);
                recalculateGain(out);
                return RP_HW_CALIB_OK;
            }

            case STEM_250_12_v1_0:
            case STEM_250_12_v1_1:
            case STEM_250_12_v1_2:
            case STEM_250_12_v1_2a:
            case STEM_250_12_120:{
                size = sizeof(rp_calib_params_v3_t);
                rp_calib_params_v3_t p_v3;
                memcpy(&p_v3,calib_params,size);
                *out = convertV3toCommon(&p_v3);
                recalculateGain(out);
                return RP_HW_CALIB_OK;
            }

            default:{
                fprintf(stderr,"[Error:calib_ConvertEEPROM] Unknown model: %d.\n",g_model);
                return RP_HW_CALIB_EDM;
                break;
            }
        }
    }
    return RP_HW_CALIB_EDM;
}


int calib_PrintEx(FILE *__restrict out,rp_calib_params_t *calib){
    fprintf(out,"dataStructureId: %d\n",calib->dataStructureId);
    fprintf(out,"wpCheck: %d\n\n",calib->wpCheck);
    fprintf(out,"fast_adc_count_1_1: %d\n\n",calib->fast_adc_count_1_1);
    for(int i = 0 ;i< calib->fast_adc_count_1_1; ++i){
        fprintf(out,"\tChannel %d:\n",i+1);
        fprintf(out,"\t\t* baseScale: %f:\n",calib->fast_adc_1_1[i].baseScale);
        fprintf(out,"\t\t* calibValue: %d:\n",calib->fast_adc_1_1[i].calibValue);
        fprintf(out,"\t\t* offset: %d:\n",calib->fast_adc_1_1[i].offset);
        fprintf(out,"\t\t* gainCalc: %f:\n\n",calib->fast_adc_1_1[i].gainCalc);
        fprintf(out,"\t\t* AA: %d:\n",calib->fast_adc_filter_1_1[i].aa);
        fprintf(out,"\t\t* BB: %d:\n",calib->fast_adc_filter_1_1[i].bb);
        fprintf(out,"\t\t* PP: %d:\n",calib->fast_adc_filter_1_1[i].pp);
        fprintf(out,"\t\t* KK: %d:\n",calib->fast_adc_filter_1_1[i].kk);
    }

    fprintf(out,"fast_adc_count_1_20: %d\n\n",calib->fast_adc_count_1_1);
    for(int i = 0 ;i< calib->fast_adc_count_1_20; ++i){
        fprintf(out,"\tChannel %d:\n",i+1);
        fprintf(out,"\t\t* baseScale: %f:\n",calib->fast_adc_1_20[i].baseScale);
        fprintf(out,"\t\t* calibValue: %d:\n",calib->fast_adc_1_20[i].calibValue);
        fprintf(out,"\t\t* offset: %d:\n",calib->fast_adc_1_20[i].offset);
        fprintf(out,"\t\t* gainCalc: %f:\n\n",calib->fast_adc_1_20[i].gainCalc);
        fprintf(out,"\t\t* AA: %d:\n",calib->fast_adc_filter_1_20[i].aa);
        fprintf(out,"\t\t* BB: %d:\n",calib->fast_adc_filter_1_20[i].bb);
        fprintf(out,"\t\t* PP: %d:\n",calib->fast_adc_filter_1_20[i].pp);
        fprintf(out,"\t\t* KK: %d:\n",calib->fast_adc_filter_1_20[i].kk);
    }

    fprintf(out,"fast_adc_count_1_1_ac: %d\n\n",calib->fast_adc_count_1_1_ac);
    for(int i = 0 ;i< calib->fast_adc_count_1_1_ac; ++i){
        fprintf(out,"\tChannel %d:\n",i+1);
        fprintf(out,"\t\t* baseScale: %f:\n",calib->fast_adc_1_1_ac[i].baseScale);
        fprintf(out,"\t\t* calibValue: %d:\n",calib->fast_adc_1_1_ac[i].calibValue);
        fprintf(out,"\t\t* offset: %d:\n",calib->fast_adc_1_1_ac[i].offset);
        fprintf(out,"\t\t* gainCalc: %f:\n\n",calib->fast_adc_1_1_ac[i].gainCalc);
    }

    fprintf(out,"fast_adc_count_1_20_ac: %d\n\n",calib->fast_adc_count_1_1_ac);
    for(int i = 0 ;i< calib->fast_adc_count_1_20_ac; ++i){
        fprintf(out,"\tChannel %d:\n",i+1);
        fprintf(out,"\t\t* baseScale: %f:\n",calib->fast_adc_1_20_ac[i].baseScale);
        fprintf(out,"\t\t* calibValue: %d:\n",calib->fast_adc_1_20_ac[i].calibValue);
        fprintf(out,"\t\t* offset: %d:\n",calib->fast_adc_1_20_ac[i].offset);
        fprintf(out,"\t\t* gainCalc: %f:\n\n",calib->fast_adc_1_20_ac[i].gainCalc);
    }

    fprintf(out,"fast_dac_count_x1: %d\n\n",calib->fast_dac_count_x1);
    for(int i = 0 ;i< calib->fast_dac_count_x1; ++i){
        fprintf(out,"\tChannel %d:\n",i+1);
        fprintf(out,"\t\t* baseScale: %f:\n",calib->fast_dac_x1[i].baseScale);
        fprintf(out,"\t\t* calibValue: %d:\n",calib->fast_dac_x1[i].calibValue);
        fprintf(out,"\t\t* offset: %d:\n",calib->fast_dac_x1[i].offset);
        fprintf(out,"\t\t* gainCalc: %f:\n\n",calib->fast_dac_x1[i].gainCalc);
    }

    fprintf(out,"fast_dac_count_x5: %d\n\n",calib->fast_dac_count_x5);
    for(int i = 0 ;i< calib->fast_dac_count_x5; ++i){
        fprintf(out,"\tChannel %d:\n",i+1);
        fprintf(out,"\t\t* baseScale: %f:\n",calib->fast_dac_x5[i].baseScale);
        fprintf(out,"\t\t* calibValue: %d:\n",calib->fast_dac_x5[i].calibValue);
        fprintf(out,"\t\t* offset: %d:\n",calib->fast_dac_x5[i].offset);
        fprintf(out,"\t\t* gainCalc: %f:\n\n",calib->fast_dac_x5[i].gainCalc);
    }
    return RP_HW_CALIB_OK;
}

int calib_Print(rp_calib_params_t *calib){
    return calib_PrintEx(stdout,calib);
}

int calib_GetFastADCFilter(rp_channel_calib_t channel,channel_filter_t *out){
    if (!g_model_loaded){
        int res = calib_Init(false);
        if (res != RP_HP_OK){
            fprintf(stderr,"[Error:calib_GetFastADCFilter] Err: %d\n",res);
            return res;
        }
    }

    if (g_calib.fast_adc_count_1_1 <= channel){
        fprintf(stderr,"[Error:calib_GetFastADCFilter] Wrong channel: %d\n",channel);
        return RP_HW_CALIB_ECH;
    }

    *out = g_calib.fast_adc_filter_1_1[channel];
    return RP_HW_CALIB_OK;
}

int calib_GetFastADCFilter_1_20(rp_channel_calib_t channel,channel_filter_t *out){
    if (!g_model_loaded){
        int res = calib_Init(false);
        if (res != RP_HP_OK){
            fprintf(stderr,"[Error:calib_GetFastADCFilter] Err: %d\n",res);
            return res;
        }
    }

    if (g_calib.fast_adc_count_1_20 <= channel){
        fprintf(stderr,"[Error:calib_GetFastADCFilter] Wrong channel: %d\n",channel);
        return RP_HW_CALIB_ECH;
    }

    *out = g_calib.fast_adc_filter_1_20[channel];
    return RP_HW_CALIB_OK;
}

int calib_GetFastADCCalibValue(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, double *gain,int32_t *offset, uint_gain_calib_t *calib){
    if (!g_model_loaded){
        int res = calib_Init(false);
        if (res != RP_HP_OK){
            fprintf(stderr,"[Error:calib_GetFastADCFilter] Err: %d\n",res);
            return res;
        }
    }

    if (g_calib.fast_adc_count_1_1 <= channel && mode == RP_DC_CALIB){
        fprintf(stderr,"[Error:calib_GetFastADCCalibValue] Wrong channel: %d in DC mode\n",channel);
        return RP_HW_CALIB_ECH;
    }

    if (g_calib.fast_adc_count_1_1_ac <= channel && mode == RP_AC_CALIB){
        fprintf(stderr,"[Error:calib_GetFastADCCalibValue] Wrong channel: %d in AC mode\n",channel);
        return RP_HW_CALIB_ECH;
    }
    switch (mode)
    {
        case RP_DC_CALIB:{
            *gain = g_calib.fast_adc_1_1[channel].gainCalc;
            *offset = g_calib.fast_adc_1_1[channel].offset;
            *calib = convertFloatToInt(&g_calib.fast_adc_1_1[channel],15);
            break;
        }

        case RP_AC_CALIB:{
            *gain = g_calib.fast_adc_1_1_ac[channel].gainCalc;
            *offset = g_calib.fast_adc_1_1_ac[channel].offset;
            *calib = convertFloatToInt(&g_calib.fast_adc_1_1_ac[channel],15);
            break;
        }

        default:
            return RP_HW_CALIB_EIP;
    }

    return RP_HW_CALIB_OK;
}

int calib_GetFastADCCalibValue_1_20(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, double *gain,int32_t *offset, uint_gain_calib_t *calib){
    if (!g_model_loaded){
        int res = calib_Init(false);
        if (res != RP_HP_OK){
            fprintf(stderr,"[Error:calib_GetFastADCFilter] Err: %d\n",res);
            return res;
        }
    }

    if (g_calib.fast_adc_count_1_1 <= channel && mode == RP_DC_CALIB){
        fprintf(stderr,"[Error:calib_GetFastADCCalibValue] Wrong channel: %d in DC mode\n",channel);
        return RP_HW_CALIB_ECH;
    }

    if (g_calib.fast_adc_count_1_1_ac <= channel && mode == RP_AC_CALIB){
        fprintf(stderr,"[Error:calib_GetFastADCCalibValue] Wrong channel: %d in AC mode\n",channel);
        return RP_HW_CALIB_ECH;
    }
    switch (mode)
    {
        case RP_DC_CALIB:{
            *gain = g_calib.fast_adc_1_20[channel].gainCalc;
            *offset = g_calib.fast_adc_1_20[channel].offset;
            *calib = convertFloatToInt(&g_calib.fast_adc_1_20[channel],15);
            break;
        }

        case RP_AC_CALIB:{
            *gain = g_calib.fast_adc_1_20_ac[channel].gainCalc;
            *offset = g_calib.fast_adc_1_20_ac[channel].offset;
            *calib = convertFloatToInt(&g_calib.fast_adc_1_20_ac[channel],15);
            break;
        }

        default:
            return RP_HW_CALIB_EIP;
    }

    return RP_HW_CALIB_OK;
}


int calib_GetFastDACCalibValue(rp_channel_calib_t channel,rp_gen_gain_calib_t mode, double *gain,int32_t *offset, uint_gain_calib_t *calib){
    if (!g_model_loaded){
        int res = calib_Init(false);
        if (res != RP_HP_OK){
            fprintf(stderr,"[Error:calib_GetFastDACCalibValue] Err: %d\n",res);
            return res;
        }
    }

    if (g_calib.fast_dac_count_x1 <= channel && mode == RP_GAIN_CALIB_1X){
        fprintf(stderr,"[Error:calib_GetFastDACCalibValue] Wrong channel: %d in x1 mode\n",channel);
        return RP_HW_CALIB_ECH;
    }

    if (g_calib.fast_dac_count_x5 <= channel && mode == RP_GAIN_CALIB_5X){
        fprintf(stderr,"[Error:calib_GetFastDACCalibValue] Wrong channel: %d in x5 mode\n",channel);
        return RP_HW_CALIB_ECH;
    }

    switch (mode)
    {
        case RP_GAIN_CALIB_1X:{
            *gain = g_calib.fast_dac_x1[channel].gainCalc;
            *offset = g_calib.fast_dac_x1[channel].offset;
            *calib = convertFloatToInt(&g_calib.fast_dac_x1[channel],15);
            break;
        }

        case RP_GAIN_CALIB_5X:{
            *gain = g_calib.fast_dac_x5[channel].gainCalc;
            *offset = g_calib.fast_dac_x5[channel].offset;
            *calib = convertFloatToInt(&g_calib.fast_dac_x5[channel],15);
            break;
        }

        default:
            return RP_HW_CALIB_EIP;
    }

    return RP_HW_CALIB_OK;
}
