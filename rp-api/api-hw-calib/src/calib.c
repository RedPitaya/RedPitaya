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
#include "calib_universal.h"

static rp_calib_params_t g_calib;
static bool g_model_loaded = false;
static rp_HPeModels_t g_model = STEM_125_10_v1_0;

rp_calib_error calib_InitModel(rp_HPeModels_t model,bool use_factory_zone, bool adjust){
    return calib_InitModelEx(model, use_factory_zone, &g_calib,adjust);
}

rp_calib_error calib_InitModelEx(rp_HPeModels_t model,bool use_factory_zone,rp_calib_params_t *calib,bool adjust){
    uint16_t header_size = 8;
    uint8_t *header = readHeader(&header_size,use_factory_zone);
    if (!header){
        *calib = getDefault(model);
        fprintf(stderr,"[Error:calib_InitModel] Cann't read calibration header. Set by default.\n");
        return RP_HW_CALIB_ERE;
    }
    uint8_t dataStructureId = header[0];
    uint16_t itemCount = ((uint16_t*)header)[1];
    free(header);

    if (dataStructureId == RP_HW_PACK_ID_V5){
        if (itemCount > MAX_UNIVERSAL_ITEMS_COUNT){
            *calib = getDefault(model);
            fprintf(stderr,"[Error:calib_InitModel] More elements for universal calibration than allowed %d max %d. Set by default.\n",itemCount,MAX_UNIVERSAL_ITEMS_COUNT);
            return RP_HW_CALIB_ERE;
        }
        uint16_t size = 8 + itemCount * 6;
        uint16_t readSize = size;
        uint8_t* buffer = use_factory_zone ? readFromFactoryEpprom(&readSize) : readFromEpprom(&readSize);
        if (buffer && size == readSize){
            rp_calib_params_universal_t calib_uni;
            memcpy(&calib_uni,buffer,size);
            *calib = convertUniversaltoCommon(model,&calib_uni);
            free(buffer);
        }else{
            free(buffer);
            *calib = getDefault(model);
            fprintf(stderr,"[Error:calib_InitModel] Cann't load universal calibration. Set by default.\n");
            return RP_HW_CALIB_ERE;
        }
    }else{
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
                    *calib = convertV1toCommon(&calib_v1,adjust);
                    free(buffer);
                }else{
                    free(buffer);
                    *calib = getDefault(model);
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
                    *calib = convertV4toCommon(&calib_v1,adjust);
                    free(buffer);
                }else{
                    free(buffer);
                    *calib = getDefault(model);
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
                    *calib = convertV2toCommon(&calib_v2,adjust);
                }else{
                    *calib = getDefault(model);
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
                    *calib = convertV3toCommon(&calib_v3,adjust);
                    free(buffer);
                }else{
                    free(buffer);
                    *calib = getDefault(model);
                    fprintf(stderr,"[Error:calib_InitModel] Cann't load calibration v3. Set by default.\n");
                    return RP_HW_CALIB_ERE;
                }
                break;
            }
            default:
                fprintf(stderr,"[Error:calib_InitModel] Unknown model: %d.\n",model);
                break;
        }
    }
    if (!recalculateGain(calib)){
        fprintf(stderr,"[Error:calib_InitModel] Cannot correctly recalculate gain on calibration.\n");
    }
    return RP_HW_CALIB_OK;
}

rp_calib_error calib_Init(bool use_factory_zone){
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

    return calib_InitModel(model,use_factory_zone,false);
}

rp_calib_error calib_WriteParams(rp_HPeModels_t model, rp_calib_params_t *calib_params,bool use_factory_zone, bool skip_recalculate){

    if (!skip_recalculate){
        if (!recalculateCalibValue(calib_params)){
            fprintf(stderr,"[Error:calib_WriteParams] Cannot correctly recalculate calib values on calibration.\n");
            return RP_HW_CALIB_EWE;
        }
    }
    if (calib_params->dataStructureId == RP_HW_PACK_ID_V5){

        rp_calib_params_universal_t calib;
        if (!convertUniversal(model,calib_params,&calib)){
            fprintf(stderr,"[Error:calib_WriteParams] Error converting universal calibration parameters.\n");
            return RP_HW_CALIB_EWE;
        }
        calib.wpCheck++;
        uint16_t size = 8 + calib.count * 6;
        uint8_t* buf = (uint8_t *)malloc(size);
        if (!buf) {
            fprintf(stderr,"[Error:calib_WriteParams] Memory allocation error.\n");
            return RP_HW_CALIB_EWE;
        }
        memset(buf,0,size);
        memcpy(buf,&calib,size);
        uint16_t ws = use_factory_zone ? writeToFactoryEpprom(buf,size) : writeToEpprom(buf,size);
        free(buf);
        if (ws != size) {
            fprintf(stderr,"[Error:calib_WriteParams] Eeprom write error.\n");
            return RP_HW_CALIB_EWE;
        }
    }else{
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

rp_calib_params_t calib_GetUniversalDefaultCalib(){
    if (!g_model_loaded){
        rp_HPeModels_t model = STEM_125_14_v1_1; // Default model
        int res = rp_HPGetModel(&model);
        if (res != RP_HP_OK){
            fprintf(stderr,"[Error:calib_GetUniversalDefaultCalib] Can't load RP model version. Err: %d\n",res);
        }
        return getDefaultUniversal(model);
    }
    return getDefaultUniversal(g_model);
}

rp_calib_error calib_WriteDirectlyParams(rp_calib_params_t *calib_params,bool use_factory_zone, bool skip_recalculate){
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
    return calib_WriteParams(model,calib_params,use_factory_zone,skip_recalculate);
}

void calib_SetToZero(bool is_new_format) {
    if (is_new_format)
        g_calib = calib_GetUniversalDefaultCalib();
    else
        g_calib = calib_GetDefaultCalib();
}

rp_calib_error calib_Reset(bool use_factory_zone,bool is_new_format) {
    if (g_model_loaded){
        rp_calib_params_t calib = g_calib;
        calib_SetToZero(is_new_format);
        int res = calib_WriteParams(g_model,&g_calib,use_factory_zone,false);
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

rp_calib_error calib_LoadFromFactoryZone(bool convert_to_new){

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
    rp_calib_params_t calib;
    rp_calib_error ret = calib_InitModelEx(model, true, &calib, true);
    if (ret != RP_HW_CALIB_OK){
        return ret;
    }
    if (convert_to_new){
        if (calib.dataStructureId != RP_HW_PACK_ID_V5){
            recalculateToUniversal(&calib);
        }
        calib.dataStructureId = RP_HW_PACK_ID_V5;
    }
    return calib_WriteParams(model,&calib,false,false);
}

rp_calib_error calib_SetParams(rp_calib_params_t *calib_params){
    g_calib = *calib_params;
    //calib_PrintEx(stderr,&g_calib);
    return RP_HW_CALIB_OK;
}

rp_calib_error calib_GetEEPROM(uint8_t **data,uint16_t *size,bool use_factory_zone){

    uint16_t header_size = 8;
    uint8_t *header = readHeader(&header_size,use_factory_zone);
    if (!header){
        *data = NULL;
        *size = 0;
        fprintf(stderr,"[Error:calib_GetEEPROM] Cann't read calibration header.\n");
        return RP_HW_CALIB_ERE;
    }

    uint8_t dataStructureId = header[0];
    uint16_t itemCount = ((uint16_t*)header)[1];
    free(header);

    switch (dataStructureId)
    {
        case RP_HW_PACK_ID_V1:{
            *size = sizeof(rp_calib_params_v1_t);
            *data = use_factory_zone ? readFromFactoryEpprom(size) : readFromEpprom(size);
            return RP_HW_CALIB_OK;
        }

        case RP_HW_PACK_ID_V3:{
            *size = sizeof(rp_calib_params_v2_t);
            *data = use_factory_zone ? readFromFactoryEpprom(size) : readFromEpprom(size);
            return RP_HW_CALIB_OK;
        }

        case RP_HW_PACK_ID_V2:{
            *size = sizeof(rp_calib_params_v3_t);
            *data = use_factory_zone ? readFromFactoryEpprom(size) : readFromEpprom(size);
            return RP_HW_CALIB_OK;
        }

        case RP_HW_PACK_ID_V4:{
            *size = sizeof(rp_calib_params_v1_t);
            *data = use_factory_zone ? readFromFactoryEpprom(size) : readFromEpprom(size);
            return RP_HW_CALIB_OK;
        }

        case RP_HW_PACK_ID_V5:{
            *size = 8 + itemCount * 6;;
            *data = use_factory_zone ? readFromFactoryEpprom(size) : readFromEpprom(size);
            return RP_HW_CALIB_OK;
        }

        default:{
            fprintf(stderr,"[Error:calib_GetEEPROM] Unknown pack ID: %d.\n",dataStructureId);
            return RP_HW_CALIB_EDM;
            break;
        }
    }
    return RP_HW_CALIB_EDM;
}

rp_calib_error calib_ConvertEEPROM(uint8_t *data,uint16_t size,rp_calib_params_t *out){
    uint8_t dataStructureId = data[0];
    switch (dataStructureId)
    {
        case RP_HW_PACK_ID_V1:{
            uint16_t ssize = sizeof(rp_calib_params_v1_t);
            if (ssize > size){
                fprintf(stderr,"[Error:calib_ConvertEEPROM] Invalid data size: %d required %d.\n",size,ssize);
                return RP_HW_CALIB_EDM;
            }
            rp_calib_params_v1_t p_v1;
            memcpy(&p_v1,data,size);
            *out = convertV1toCommon(&p_v1,false);
            recalculateGain(out);
            return RP_HW_CALIB_OK;
        }

        case RP_HW_PACK_ID_V3:{
            uint16_t ssize = sizeof(rp_calib_params_v2_t);
            if (ssize > size){
                fprintf(stderr,"[Error:calib_ConvertEEPROM] Invalid data size: %d required %d.\n",size,ssize);
                return RP_HW_CALIB_EDM;
            }
            rp_calib_params_v2_t p_v2;
            memcpy(&p_v2,data,size);
            *out = convertV2toCommon(&p_v2,false);
            recalculateGain(out);
            return RP_HW_CALIB_OK;
        }


        case RP_HW_PACK_ID_V2:{
            uint16_t ssize = sizeof(rp_calib_params_v3_t);
            if (ssize > size){
                fprintf(stderr,"[Error:calib_ConvertEEPROM] Invalid data size: %d required %d.\n",size,ssize);
                return RP_HW_CALIB_EDM;
            }
            rp_calib_params_v3_t p_v3;
            memcpy(&p_v3,data,size);
            *out = convertV3toCommon(&p_v3,false);
            recalculateGain(out);
            return RP_HW_CALIB_OK;
        }

        case RP_HW_PACK_ID_V4:{
            uint16_t ssize = sizeof(rp_calib_params_v1_t);
            if (ssize > size){
                fprintf(stderr,"[Error:calib_ConvertEEPROM] Invalid data size: %d required %d.\n",size,ssize);
                return RP_HW_CALIB_EDM;
            }
            rp_calib_params_v1_t p_v1;
            memcpy(&p_v1,data,size);
            *out = convertV4toCommon(&p_v1,false);
            recalculateGain(out);
            return RP_HW_CALIB_OK;
        }

        case RP_HW_PACK_ID_V5:{
            rp_HPeModels_t model = STEM_125_14_v1_1; // Default model
            int res = rp_HPGetModel(&model);
            if (res != RP_HP_OK){
                fprintf(stderr,"[Error:calib_Init] Can't load RP model version. Err: %d\n",res);
                return RP_HW_CALIB_EDM;
            }
            uint16_t itemCount = ((uint16_t*)data)[1];
            uint16_t ssize = 8 + itemCount * 6;
            if (ssize > size){
                fprintf(stderr,"[Error:calib_ConvertEEPROM] Invalid data size: %d required %d.\n",size,ssize);
                return RP_HW_CALIB_EDM;
            }
            rp_calib_params_universal_t p_u;
            memcpy(&p_u,data,size);
            *out = convertUniversaltoCommon(model,&p_u);
            recalculateGain(out);
            return RP_HW_CALIB_OK;
        }

        default:{
            fprintf(stderr,"[Error:calib_ConvertEEPROM] Unknown calibration: %d.\n",dataStructureId);
            return RP_HW_CALIB_UC;
            break;
        }
    }

    return RP_HW_CALIB_EDM;
}

rp_calib_error calib_ConvertToOld(rp_calib_params_t *out){
    if (out->dataStructureId != RP_HW_PACK_ID_V5){
        fprintf(stderr,"[Error:calib_ConvertToOld] Calibration should only be in the new format: %d.\n",out->dataStructureId);
        return RP_HW_CALIB_EIP;
    }
    rp_HPeModels_t model;
    if (!g_model_loaded){
        int res = rp_HPGetModel(&model);
        if (res != RP_HP_OK){
            fprintf(stderr,"[Error:calib_ConvertToOld] Can't load RP model version. Err: %d\n",res);
            return RP_HW_CALIB_EDM;
        }

    }else{
        model = g_model;
    }

    switch (model)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:{
            out->dataStructureId = RP_HW_PACK_ID_V1;
            for(int i = 0; i < 2; ++i){
                out->fast_adc_1_1[i].baseScale = 20.0;
                out->fast_adc_1_1[i].calibValue = calibBaseScaleFromVoltage(20.0,false) * out->fast_adc_1_1[i].gainCalc;

                out->fast_adc_1_20[i].baseScale = 1.0;
                out->fast_adc_1_20[i].calibValue = calibBaseScaleFromVoltage(1.0,false) * out->fast_adc_1_20[i].gainCalc;

                out->fast_dac_x1[i].baseScale = 1.0;
                out->fast_dac_x1[i].calibValue = calibBaseScaleFromVoltage(1.0,false) * (1.0 / out->fast_dac_x1[i].gainCalc);
            }
            break;
        }

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:{
            out->dataStructureId = RP_HW_PACK_ID_V4;
            for(int i = 0; i < 2; ++i){
                out->fast_adc_1_1[i].baseScale = 20.0;
                out->fast_adc_1_1[i].calibValue = calibBaseScaleFromVoltage(20.0,false) * out->fast_adc_1_1[i].gainCalc;

                out->fast_dac_x1[i].baseScale = 1.0;
                out->fast_dac_x1[i].calibValue = calibBaseScaleFromVoltage(1.0,false) * out->fast_dac_x1[i].gainCalc;
            }
            break;
        }

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:{
            out->dataStructureId = RP_HW_PACK_ID_V3;
            for(int i = 0; i < 4; ++i){
                out->fast_adc_1_1[i].baseScale = 20.0;
                out->fast_adc_1_1[i].calibValue = calibBaseScaleFromVoltage(20.0,false) * out->fast_adc_1_1[i].gainCalc;

                out->fast_adc_1_20[i].baseScale = 1.0;
                out->fast_adc_1_20[i].calibValue = calibBaseScaleFromVoltage(1.0,false) * out->fast_adc_1_20[i].gainCalc;
            }

            break;
        }

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_120:{
            out->dataStructureId = RP_HW_PACK_ID_V2;
            for(int i = 0; i < 2; ++i){
                out->fast_adc_1_1[i].baseScale = 20.0;
                out->fast_adc_1_1[i].calibValue = calibBaseScaleFromVoltage(20.0,false) * out->fast_adc_1_1[i].gainCalc;

                out->fast_adc_1_20[i].baseScale = 1.0;
                out->fast_adc_1_20[i].calibValue = calibBaseScaleFromVoltage(1.0,false) * out->fast_adc_1_20[i].gainCalc;

                out->fast_dac_x1[i].baseScale = 1.0;
                out->fast_dac_x1[i].calibValue = calibBaseScaleFromVoltage(1.0,false) * out->fast_dac_x1[i].gainCalc;

                out->fast_dac_x5[i].baseScale = 2.0;
                out->fast_dac_x5[i].calibValue = calibBaseScaleFromVoltage(2.0,false) * out->fast_dac_x5[i].gainCalc;
            }
            break;
        }

        default:
            fprintf(stderr,"[Error:calib_ConvertToOld] Unknown model: %d.\n",model);
            return RP_HW_CALIB_ENI;
    }
    return RP_HW_CALIB_OK;
}


rp_calib_error calib_PrintEx(FILE *__restrict out,rp_calib_params_t *calib){
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

rp_calib_error calib_Print(rp_calib_params_t *calib){
    return calib_PrintEx(stdout,calib);
}

rp_calib_error calib_GetFastADCFilter(rp_channel_calib_t channel,channel_filter_t *out){
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

rp_calib_error calib_GetFastADCFilter_1_20(rp_channel_calib_t channel,channel_filter_t *out){
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

rp_calib_error calib_GetFastADCCalibValue(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, double *gain,int32_t *offset, uint_gain_calib_t *calib){
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

rp_calib_error calib_GetFastADCCalibValue_1_20(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, double *gain,int32_t *offset, uint_gain_calib_t *calib){
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


rp_calib_error calib_GetFastDACCalibValue(rp_channel_calib_t channel,rp_gen_gain_calib_t mode, double *gain,int32_t *offset, uint_gain_calib_t *calib){
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
