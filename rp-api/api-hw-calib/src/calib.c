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

rp_calib_params_t g_calib;
bool g_model_loaded = false;
rp_HPeModels_t g_model = STEM_125_10_v1_0;

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
                fprintf(stderr,"[Error] Cann't load calibration v1. Set by default.\n");
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
                g_calib = convertV1toCommon(&calib_v1);
                if (buffer) free(buffer);

            }else{
                if (buffer) free(buffer);
                g_calib = getDefault(model);
                fprintf(stderr,"[Error] Cann't load calibration v1. Set by default.\n");
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
                fprintf(stderr,"[Error] Cann't load calibration v2. Set by default.\n");
                return RP_HW_CALIB_ERE;
            }
            break;
        }

        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:{
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
                fprintf(stderr,"[Error] Cann't load calibration v3. Set by default.\n");
                return RP_HW_CALIB_ERE;
            }
            break;
        }
        default:
            fprintf(stderr,"[Error] Unknown model: %d.\n",model);
            break;
    }
    if (!recalculateGain(&g_calib)){
        fprintf(stderr,"[Error] Cannot correctly recalculate gain on calibration.\n");
    }
    return RP_HW_CALIB_OK;
}

int calib_Init(bool use_factory_zone){
    rp_HPeModels_t model = STEM_125_14_v1_1; // Default model
    int res = rp_HPGetModel(&model);
    if (res != RP_HP_OK){
        fprintf(stderr,"[Error] Can't load RP model version. Err: %d\n",res);
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
        fprintf(stderr,"[Error] Cannot correctly recalculate calib values on calibration.\n");
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
                fprintf(stderr,"[Error] Memory allocation error.\n");
		        return RP_HW_CALIB_EWE;
	        }
            rp_calib_params_v1_t calib_v1;
            if (!convertV1(calib_params,&calib_v1)){
                free(buf);
                fprintf(stderr,"[Error] Error converting calibration parameters.\n");
                return RP_HW_CALIB_EWE;
            }
            memcpy(buf,&calib_v1,size);
            uint16_t ws = use_factory_zone ? writeToFactoryEpprom(buf,size) : writeToEpprom(buf,size);
            free(buf);
            if (ws != size) {
                fprintf(stderr,"[Error] Eeprom write error.\n");
                return RP_HW_CALIB_EWE;
            }

            break;
        }

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:{

            uint16_t size = sizeof(rp_calib_params_v1_t);
            uint8_t* buf = (uint8_t *)malloc(size);
	        if (!buf) {
                fprintf(stderr,"[Error] Memory allocation error.\n");
		        return RP_HW_CALIB_EWE;
	        }
            rp_calib_params_v1_t calib_v1;
            if (!convertV1(calib_params,&calib_v1)){
                free(buf);
                fprintf(stderr,"[Error] Error converting calibration parameters.\n");
                return RP_HW_CALIB_EWE;
            }
            memcpy(buf,&calib_v1,size);
            uint16_t ws = use_factory_zone ? writeToFactoryEpprom(buf,size) : writeToEpprom(buf,size);
            free(buf);
            if (ws != size) {
                fprintf(stderr,"[Error] Eeprom write error.\n");
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
                fprintf(stderr,"[Error] Memory allocation error.\n");
		        return RP_HW_CALIB_EWE;
	        }
            rp_calib_params_v2_t calib_v2;
            if (!convertV2(calib_params,&calib_v2)){
                free(buf);
                fprintf(stderr,"[Error] Error converting calibration parameters.\n");
                return RP_HW_CALIB_EWE;
            }
            memcpy(buf,&calib_v2,size);
            uint16_t ws = use_factory_zone ? writeToFactoryEpprom(buf,size) : writeToEpprom(buf,size);
            free(buf);
            if (ws != size) {
                fprintf(stderr,"[Error] Eeprom write error.\n");
                return RP_HW_CALIB_EWE;
            }
            break;
        }

        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:{

            uint16_t size = sizeof(rp_calib_params_v3_t);
            uint8_t* buf = (uint8_t *)malloc(size);
	        if (!buf) {
                fprintf(stderr,"[Error] Memory allocation error.\n");
		        return RP_HW_CALIB_EWE;
	        }
            rp_calib_params_v3_t calib_v3;
            if (!convertV3(calib_params,&calib_v3)){
                free(buf);
                fprintf(stderr,"[Error] Error converting calibration parameters.\n");
                return RP_HW_CALIB_EWE;
            }
            memcpy(buf,&calib_v3,size);
            uint16_t ws = use_factory_zone ? writeToFactoryEpprom(buf,size) : writeToEpprom(buf,size);
            free(buf);
            if (ws != size) {
                fprintf(stderr,"[Error] Eeprom write error.\n");
                return RP_HW_CALIB_EWE;
            }
            break;
        }

        default:
            fprintf(stderr,"[Error] Unknown model: %d.\n",model);
            break;
    }
    if (recalculateGain(&g_calib)){
        fprintf(stderr,"[Error] Cannot correctly recalculate gain on calibration.\n");
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
            fprintf(stderr,"[Error] Can't load RP model version. Err: %d\n",res);
        }
        return getDefault(model);
    }
    return getDefault(g_model);
}

int calib_WriteDirectlyParams(rp_calib_params_t *calib_params){
    rp_HPeModels_t model = STEM_125_14_v1_1; // Default model
    if (!g_model_loaded){
        int res = rp_HPGetModel(&model);
        if (res != RP_HP_OK){
            fprintf(stderr,"[Error] Can't load RP model version. Err: %d\n",res);
            return RP_HW_CALIB_EDM;
        }

    }else{
        model = g_model;
    }
    return calib_WriteParams(model,&calib_params,false);
}

void calib_SetToZero() {
    g_calib = calib_GetDefaultCalib();
}

int calib_Reset() {
    if (g_model_loaded){
        rp_calib_params_t calib = g_calib;
        calib_SetToZero();
        int res = calib_WriteParams(g_model,&g_calib,false);
        if (res != RP_HW_CALIB_OK){
            g_calib = calib;
            return res;
        }
        return calib_Init(false);
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
            fprintf(stderr,"[Error] Can't load RP model version. Err: %d\n",res);
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

        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:{
            size = sizeof(rp_calib_params_v3_t);
            break;
        }

        default:{
            fprintf(stderr,"[Error] Unknown model: %d.\n",model);
            return RP_HW_CALIB_EDM;
            break;
        }
    }

    uint16_t rs = size;
    uint8_t *buffer = readFromFactoryEpprom(&size);

    if (!buffer || rs != size) {
        if (buffer) free(buffer);
        fprintf(stderr,"[Error] Eeprom read from factory zone.\n");
        return RP_HW_CALIB_ERE;
    }
    size = writeToEpprom(buffer,size);
    if (rs != size) {
        fprintf(stderr,"[Error] Eeprom write to eeprom error.\n");
        return RP_HW_CALIB_EWE;
    }
    if (buffer) free(buffer);
    return RP_HW_CALIB_OK;
}

int calib_SetParams(rp_calib_params_t *calib_params){
    g_calib = *calib_params;
    return RP_HW_CALIB_OK;
}

// /**
//  * @brief Read calibration parameters from EEPROM device.
//  *
//  * Function reads calibration parameters from EEPROM device and stores them to the
//  * specified buffer. Communication to the EEPROM device is taken place through
//  * appropriate system driver accessed through the file system device
//  * /sys/bus/i2c/devices/0-0050/eeprom.
//  *
//  * @param[out]   calib_params  Pointer to destination buffer.
//  * @retval       0 Success
//  * @retval       >0 Failure
//  *
//  */
// int calib_ReadParams(rp_calib_params_t *calib_params,bool use_factory_zone)
// {
//     FILE   *fp;
//     size_t  size;

//     /* sanity check */
//     if(calib_params == NULL) {
//         return RP_UIA;
//     }

//     /* open EEPROM device */
//     fp = fopen(eeprom_device, "r");
//     if(fp == NULL) {
//         return RP_EOED;
//     }

//     /* ...and seek to the appropriate storage offset */
//     int offset = use_factory_zone ? eeprom_calib_factory_off : eeprom_calib_off;
//     if(fseek(fp, offset, SEEK_SET) < 0) {
//         fclose(fp);
//         return RP_FCA;
//     }

//     /* read data from EEPROM component and store it to the specified buffer */
//     size = fread(calib_params, sizeof(char), sizeof(rp_calib_params_t), fp);
//     if(size != sizeof(rp_calib_params_t)) {
//         fclose(fp);
//         return RP_RCA;
//     }
//     fclose(fp);

// #if defined Z10 || defined Z20_125
//     if (calib_params->magic != CALIB_MAGIC && calib_params->magic != CALIB_MAGIC_FILTER) {
// 		calib_params->fe_ch1_hi_offs = calib_params->fe_ch1_lo_offs;
// 		calib_params->fe_ch2_hi_offs = calib_params->fe_ch2_lo_offs;
// 	}
//     else if (calib_params->magic != CALIB_MAGIC_FILTER){
//         calib_params->low_filter_aa_ch1 = GAIN_LO_FILT_AA;
//         calib_params->low_filter_bb_ch1 = GAIN_LO_FILT_BB;
//         calib_params->low_filter_pp_ch1 = GAIN_LO_FILT_PP;
//         calib_params->low_filter_kk_ch1 = GAIN_LO_FILT_KK;
//         calib_params->low_filter_aa_ch2 = GAIN_LO_FILT_AA;
//         calib_params->low_filter_bb_ch2 = GAIN_LO_FILT_BB;
//         calib_params->low_filter_pp_ch2 = GAIN_LO_FILT_PP;
//         calib_params->low_filter_kk_ch2 = GAIN_LO_FILT_KK;

//         calib_params->hi_filter_aa_ch1 = GAIN_HI_FILT_AA;
//         calib_params->hi_filter_bb_ch1 = GAIN_HI_FILT_BB;
//         calib_params->hi_filter_pp_ch1 = GAIN_HI_FILT_PP;
//         calib_params->hi_filter_kk_ch1 = GAIN_HI_FILT_KK;
//         calib_params->hi_filter_aa_ch2 = GAIN_HI_FILT_AA;
//         calib_params->hi_filter_bb_ch2 = GAIN_HI_FILT_BB;
//         calib_params->hi_filter_pp_ch2 = GAIN_HI_FILT_PP;
//         calib_params->hi_filter_kk_ch2 = GAIN_HI_FILT_KK;
//     }
// #endif

//     return 0;
// }








// int calib_SetParams(rp_calib_params_t calib_params){
//     calib = calib_params;
//     return RP_OK;
// }

// rp_calib_params_t getDefualtCalib(){
//     rp_calib_params_t calib;
//     calib.magic = CALIB_MAGIC;
//     calib.be_ch1_dc_offs = 0;
//     calib.be_ch2_dc_offs = 0;
//     calib.fe_ch1_lo_offs = 0;
//     calib.fe_ch2_lo_offs = 0;
//     calib.fe_ch1_hi_offs = 0;
//     calib.fe_ch2_hi_offs = 0;

//     float coff = 1;
// #ifdef Z20
//     coff = 0.5;
// #endif

//     calib.be_ch1_fs      = cmn_CalibFullScaleFromVoltage(1);
//     calib.be_ch2_fs      = cmn_CalibFullScaleFromVoltage(1);
//     calib.fe_ch1_fs_g_lo = cmn_CalibFullScaleFromVoltage(20.0 );
//     calib.fe_ch1_fs_g_hi = cmn_CalibFullScaleFromVoltage(coff );
//     calib.fe_ch2_fs_g_lo = cmn_CalibFullScaleFromVoltage(20.0 );
//     calib.fe_ch2_fs_g_hi = cmn_CalibFullScaleFromVoltage(coff );

// #if defined Z10 || defined Z20_125
//     calib.magic = CALIB_MAGIC_FILTER;
//     calib.low_filter_aa_ch1 = GAIN_LO_FILT_AA;
//     calib.low_filter_bb_ch1 = GAIN_LO_FILT_BB;
//     calib.low_filter_pp_ch1 = GAIN_LO_FILT_PP;
//     calib.low_filter_kk_ch1 = GAIN_LO_FILT_KK;
//     calib.low_filter_aa_ch2 = GAIN_LO_FILT_AA;
//     calib.low_filter_bb_ch2 = GAIN_LO_FILT_BB;
//     calib.low_filter_pp_ch2 = GAIN_LO_FILT_PP;
//     calib.low_filter_kk_ch2 = GAIN_LO_FILT_KK;

//     calib.hi_filter_aa_ch1 = GAIN_HI_FILT_AA;
//     calib.hi_filter_bb_ch1 = GAIN_HI_FILT_BB;
//     calib.hi_filter_pp_ch1 = GAIN_HI_FILT_PP;
//     calib.hi_filter_kk_ch1 = GAIN_HI_FILT_KK;
//     calib.hi_filter_aa_ch2 = GAIN_HI_FILT_AA;
//     calib.hi_filter_bb_ch2 = GAIN_HI_FILT_BB;
//     calib.hi_filter_pp_ch2 = GAIN_HI_FILT_PP;
//     calib.hi_filter_kk_ch2 = GAIN_HI_FILT_KK;
// #endif
//     return calib;
// }




// uint32_t calib_GetFrontEndScale(rp_channel_t channel, rp_pinState_t gain) {
//     if (gain == RP_HIGH) {
//         return (channel == RP_CH_1 ? calib.fe_ch1_fs_g_hi : calib.fe_ch2_fs_g_hi);
//     }
//     else {
//         return (channel == RP_CH_1 ? calib.fe_ch1_fs_g_lo : calib.fe_ch2_fs_g_lo);
//     }
// }

// int calib_SetFrontEndOffset(rp_channel_t channel, rp_pinState_t gain, rp_calib_params_t* out_params) {
//     rp_calib_params_t params;
//     calib_ReadParams(&params,false);
// 	failsafa_params = params;

//     /* Reset current calibration parameters*/
//     if (gain == RP_LOW) {
// 		CHANNEL_ACTION(channel,
//             params.fe_ch1_lo_offs = 0,
//             params.fe_ch2_lo_offs = 0)
//     } else {
// 		CHANNEL_ACTION(channel,
//             params.fe_ch1_hi_offs = 0,
//             params.fe_ch2_hi_offs = 0)
// 	}
//     /* Acquire uses this calibration parameters - reset them */
//     calib = params;

// 	if (gain == RP_LOW) {
// 		CHANNEL_ACTION(channel,
// 			params.fe_ch1_lo_offs = calib_GetDataMedian(channel, RP_LOW),
// 			params.fe_ch2_lo_offs = calib_GetDataMedian(channel, RP_LOW))
// 	} else {
// 		CHANNEL_ACTION(channel,
// 			params.fe_ch1_hi_offs = calib_GetDataMedian(channel, RP_HIGH),
// 			params.fe_ch2_hi_offs = calib_GetDataMedian(channel, RP_HIGH))
// 	}

//     /* Set new local parameter */
//     if  (out_params) {
// 		//	*out_params = params;
// 		if (gain == RP_LOW) {
// 			CHANNEL_ACTION(channel,
// 				out_params->fe_ch1_lo_offs = params.fe_ch1_lo_offs,
// 				out_params->fe_ch2_lo_offs = params.fe_ch2_lo_offs)
// 		} else {
// 			CHANNEL_ACTION(channel,
// 				out_params->fe_ch1_hi_offs = params.fe_ch1_hi_offs,
// 				out_params->fe_ch2_hi_offs = params.fe_ch2_hi_offs)
// 		}
// 	}
//     else
// 		calib_WriteParams(params,false);
//     return calib_Init();
// }

// int calib_SetFrontEndScaleLV(rp_channel_t channel, float referentialVoltage, rp_calib_params_t* out_params) {
//     rp_calib_params_t params;
//     calib_ReadParams(&params,false);
// 	failsafa_params = params;

//     /* Reset current calibration parameters*/
//     CHANNEL_ACTION(channel,
//             params.fe_ch1_fs_g_lo = cmn_CalibFullScaleFromVoltage(20),
//             params.fe_ch2_fs_g_lo = cmn_CalibFullScaleFromVoltage(20))
//     /* Acquire uses this calibration parameters - reset them */
//     calib = params;

//     /* Calculate real max adc voltage */
//     float value = calib_GetDataMedianFloat(channel, RP_LOW);
//     uint32_t calibValue = cmn_CalibFullScaleFromVoltage(20.f * referentialVoltage / value);

//     CHANNEL_ACTION(channel,
//             params.fe_ch1_fs_g_lo = calibValue,
//             params.fe_ch2_fs_g_lo = calibValue )

//     /* Set new local parameter */
//     if  (out_params) {
// 		//	*out_params = params;
// 		CHANNEL_ACTION(channel,
// 				out_params->fe_ch1_fs_g_lo = params.fe_ch1_fs_g_lo,
// 				out_params->fe_ch2_fs_g_lo = params.fe_ch2_fs_g_lo)
// 	}
//     else
// 		calib_WriteParams(params,false);
//     return calib_Init();
// }

// int calib_SetFrontEndScaleHV(rp_channel_t channel, float referentialVoltage, rp_calib_params_t* out_params) {
//     rp_calib_params_t params;
//     calib_ReadParams(&params,false);
//     failsafa_params = params;

//     /* Reset current calibration parameters*/
//     CHANNEL_ACTION(channel,
//             params.fe_ch1_fs_g_hi = cmn_CalibFullScaleFromVoltage(1),
//             params.fe_ch2_fs_g_hi = cmn_CalibFullScaleFromVoltage(1))
//     /* Acquire uses this calibration parameters - reset them */
//     calib = params;

//     /* Calculate real max adc voltage */
//     float value = calib_GetDataMedianFloat(channel, RP_HIGH);
//     uint32_t calibValue = cmn_CalibFullScaleFromVoltage(referentialVoltage / value);

//     CHANNEL_ACTION(channel,
//             params.fe_ch1_fs_g_hi = calibValue,
//             params.fe_ch2_fs_g_hi = calibValue )

//     /* Set new local parameter */
//     if  (out_params) {
// 		//	*out_params = params;
// 		CHANNEL_ACTION(channel,
// 				out_params->fe_ch1_fs_g_hi = params.fe_ch1_fs_g_hi,
// 				out_params->fe_ch2_fs_g_hi = params.fe_ch2_fs_g_hi)
// 	}
//     else
// 		calib_WriteParams(params,false);
//     return calib_Init();
// }

// int calib_SetBackEndOffset(rp_channel_t channel) {
//     rp_calib_params_t params;
//     calib_ReadParams(&params,false);

// 	failsafa_params = params;
//     /* Reset current calibration parameters*/
//     CHANNEL_ACTION(channel,
//             params.be_ch1_dc_offs = 0,
//             params.be_ch2_dc_offs = 0)
//     /* Generate uses this calibration parameters - reset them */
//     calib = params;

//     /* Generate zero signal */
//     rp_GenReset();
//     rp_GenWaveform(channel, RP_WAVEFORM_SINE);
//     rp_GenAmp(channel, 0);
//     rp_GenOffset(channel, 0);
//     rp_GenOutEnable(channel);

//     CHANNEL_ACTION(channel,
//             params.be_ch1_dc_offs = -calib_GetDataMedian(channel, RP_LOW),
//             params.be_ch2_dc_offs = -calib_GetDataMedian(channel, RP_LOW))

//     /* Set new local parameter */
// 	calib_WriteParams(params,false);
//     return calib_Init();
// }

// int calib_SetBackEndScale(rp_channel_t channel) {
//     rp_calib_params_t params;
//     calib_ReadParams(&params,false);
// 	failsafa_params = params;

//     /* Reset current calibration parameters*/
//     CHANNEL_ACTION(channel,
//             params.be_ch1_fs = cmn_CalibFullScaleFromVoltage(1),
//             params.be_ch2_fs = cmn_CalibFullScaleFromVoltage(1))
//     /* Generate uses this calibration parameters - reset them */
//     calib = params;

//     /* Generate constant signal signal */
//     rp_GenReset();
//     rp_GenWaveform(channel, RP_WAVEFORM_PWM);
//     rp_GenDutyCycle(channel, 1);
//     rp_GenAmp(channel, CONSTANT_SIGNAL_AMPLITUDE);
//     rp_GenOffset(channel, 0);
//     rp_GenOutEnable(channel);

//     /* Calculate real max adc voltage */
//     float value = calib_GetDataMedianFloat(channel, RP_LOW);
//     uint32_t calibValue = cmn_CalibFullScaleFromVoltage((float) (value / CONSTANT_SIGNAL_AMPLITUDE));

//     CHANNEL_ACTION(channel,
//             params.be_ch1_fs = calibValue,
//             params.be_ch2_fs = calibValue)

//     /* Set new local parameter */
// 	calib_WriteParams(params,false);
//     return calib_Init();
// }

// static int getGenAmp(rp_channel_t channel, float amp, float* min, float* max) {
//     rp_GenReset();
//     rp_GenWaveform(channel, RP_WAVEFORM_SINE);
//     rp_GenAmp(channel, amp);
//     rp_GenOffset(channel, 0);
//     rp_GenOutEnable(channel);

//     return calib_GetDataMinMaxFloat(channel, RP_LOW, min, max);
// }

// static int getGenDC_int(rp_channel_t channel, float dc) {
//     rp_GenReset();
//     rp_GenWaveform(channel, RP_WAVEFORM_DC);
//     rp_GenAmp(channel, 0);
//     rp_GenOffset(channel, dc);
//     rp_GenOutEnable(channel);

//     return calib_GetDataMedian(channel, RP_LOW);
// }

// int calib_CalibrateBackEnd(rp_channel_t channel, rp_calib_params_t* out_params) {
//     rp_calib_params_t params;
//     calib_ReadParams(&params,false);

//     /* Reset current calibration parameters*/
//     CHANNEL_ACTION(channel,
//             params.be_ch1_fs = cmn_CalibFullScaleFromVoltage(1),
//             params.be_ch2_fs = cmn_CalibFullScaleFromVoltage(1))

//     CHANNEL_ACTION(channel,
//             params.be_ch1_dc_offs = 0,
//             params.be_ch2_dc_offs = 0)

//     /* Generate uses this calibration parameters - reset them */
//     calib = params;

//     float value1, value2;
//     getGenAmp(channel, CONSTANT_SIGNAL_AMPLITUDE, &value1, &value2);
//     float scale = (value2 - value1) / (2.f * CONSTANT_SIGNAL_AMPLITUDE);
//     fprintf(stderr, "v1: %f, v2: %f, scale: %f\n", value1, value2, scale);

//     int off1 = getGenDC_int(channel, -CONSTANT_SIGNAL_AMPLITUDE);
//     int off2 = getGenDC_int(channel, 0);
//     int off3 = getGenDC_int(channel, CONSTANT_SIGNAL_AMPLITUDE);
//     int offset = -(off1 + off2 + off3) / 3;

//     fprintf(stderr, "off1: %d, off2: %d, off3: %d, off: %d\n", off1, off2, off3, offset);
//     /* Generate constant signal signal */
//     uint32_t calibValue = cmn_CalibFullScaleFromVoltage(scale);

//     CHANNEL_ACTION(channel,
//             params.be_ch1_fs = calibValue,
//             params.be_ch2_fs = calibValue)

//     CHANNEL_ACTION(channel,
//             params.be_ch1_dc_offs = offset,
//             params.be_ch2_dc_offs = offset)

//     /* Set new local parameter */
//     if  (out_params) {
// 		//	*out_params = params;
// 		CHANNEL_ACTION(channel,
// 				out_params->be_ch1_fs = params.be_ch1_fs,
// 				out_params->be_ch2_fs = params.be_ch2_fs)
// 		CHANNEL_ACTION(channel,
// 				out_params->be_ch1_dc_offs = params.be_ch1_dc_offs,
// 				out_params->be_ch2_dc_offs = params.be_ch2_dc_offs)
// 	}
//     else
// 		calib_WriteParams(params,false);
//     return calib_Init();
// }



// int32_t calib_GetDataMedian(rp_channel_t channel, rp_pinState_t gain) {
//     /* Acquire data */
//     rp_AcqReset();
//     rp_AcqSetGain(channel, gain);
//     rp_AcqSetDecimation(RP_DEC_64);
//     rp_AcqStart();
//     rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);
//     usleep(1000000);
//     rp_AcqStop();

//     int16_t data[ADC_BUFFER_SIZE];
//     uint32_t bufferSize = (uint32_t) ADC_BUFFER_SIZE;
//     rp_AcqGetDataRaw(channel, 0, &bufferSize, data);

//     long long avg = 0;
//     for(int i = 0; i < ADC_BUFFER_SIZE; ++i)
//         avg += data[i];

//     avg /= ADC_BUFFER_SIZE;
//     fprintf(stderr, "\ncalib_GetDataMedian: avg = %d\n", (int32_t)avg);
//     return avg;
// }

// float calib_GetDataMedianFloat(rp_channel_t channel, rp_pinState_t gain) {
//     rp_AcqReset();
//     rp_AcqSetGain(channel, gain);
//     rp_AcqSetDecimation(RP_DEC_64);
//     rp_AcqStart();
//     rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);
//     usleep(1000000);
//     int BUF_SIZE = ADC_BUFFER_SIZE;

//     rp_AcqStop();

//     float data[BUF_SIZE];
//     uint32_t bufferSize = (uint32_t) BUF_SIZE;
//     rp_AcqGetDataV(channel, 0, &bufferSize, data);

//     double avg = 0;
//     for(int i = 0; i < ADC_BUFFER_SIZE; ++i)
//         avg += data[i];

//     avg /= ADC_BUFFER_SIZE;
//     fprintf(stderr, "\ncalib_GetDataMedianFloat: avg = %f\n", (float)avg);
//     return avg;
// }

// int calib_GetDataMinMaxFloat(rp_channel_t channel, rp_pinState_t gain, float* min, float* max) {
//     rp_AcqReset();
//     rp_AcqSetGain(channel, gain);
//     rp_AcqSetDecimation(RP_DEC_64);
//     rp_AcqStart();
//     rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);
//     usleep(1000000);
//     int BUF_SIZE = ADC_BUFFER_SIZE;

//     rp_AcqStop();

//     float data[BUF_SIZE];
//     uint32_t bufferSize = (uint32_t) BUF_SIZE;
//     rp_AcqGetDataV(channel, 0, &bufferSize, data);

//     float _min = data[0];
//     float _max = data[0];
//     for(int i = 1; i < ADC_BUFFER_SIZE; ++i) {
//         _min = (_min > data[i]) ? data[i] : _min;
//         _max = (_max < data[i]) ? data[i] : _max;
//     }

//     fprintf(stderr, "\ncalib_GetDataMinMaxFloat: min = %f, max = %f\n", _min, _max);
//     *min = _min;
//     *max = _max;
//     return RP_OK;
// }

// int calib_setCachedParams() {
// 	fprintf(stderr, "write FAILSAFE PARAMS\n");
//     calib_WriteParams(failsafa_params,false);
//     calib = failsafa_params;

//     return 0;
// }

// int32_t calib_getOffset(rp_channel_t channel, rp_pinState_t gain){
//     if (gain == RP_HIGH) {
//         return (channel == RP_CH_1 ? calib.fe_ch1_hi_offs : calib.fe_ch2_hi_offs);
//     }
//     else {
//         return (channel == RP_CH_1 ? calib.fe_ch1_lo_offs : calib.fe_ch2_lo_offs);
//     }
// }

// int32_t calib_getGenOffset(rp_channel_t channel){
//     return (channel == RP_CH_1 ?  calib.be_ch1_dc_offs: calib.be_ch2_dc_offs);
// }

// uint32_t calib_getGenScale(rp_channel_t channel){
//     return (channel == RP_CH_1 ?  calib.be_ch1_fs: calib.be_ch2_fs);
// }

// #if defined Z10 || defined Z20_125

// int calib_SetFilterCoff(rp_channel_t channel, rp_pinState_t gain, rp_eq_filter_cof_t coff , uint32_t value){
//     rp_calib_params_t params;
//     calib_ReadParams(&params,false);
//     if (params.magic == CALIB_MAGIC){
//         params.magic = CALIB_MAGIC_FILTER;
//         params.low_filter_aa_ch1 = GAIN_LO_FILT_AA;
//         params.low_filter_bb_ch1 = GAIN_LO_FILT_BB;
//         params.low_filter_pp_ch1 = GAIN_LO_FILT_PP;
//         params.low_filter_kk_ch1 = GAIN_LO_FILT_KK;
//         params.low_filter_aa_ch2 = GAIN_LO_FILT_AA;
//         params.low_filter_bb_ch2 = GAIN_LO_FILT_BB;
//         params.low_filter_pp_ch2 = GAIN_LO_FILT_PP;
//         params.low_filter_kk_ch2 = GAIN_LO_FILT_KK;

//         params.hi_filter_aa_ch1 = GAIN_HI_FILT_AA;
//         params.hi_filter_bb_ch1 = GAIN_HI_FILT_BB;
//         params.hi_filter_pp_ch1 = GAIN_HI_FILT_PP;
//         params.hi_filter_kk_ch1 = GAIN_HI_FILT_KK;
//         params.hi_filter_aa_ch2 = GAIN_HI_FILT_AA;
//         params.hi_filter_bb_ch2 = GAIN_HI_FILT_BB;
//         params.hi_filter_pp_ch2 = GAIN_HI_FILT_PP;
//         params.hi_filter_kk_ch2 = GAIN_HI_FILT_KK;
//     }
//     if (channel == RP_CH_1){
//         if (gain == RP_HIGH){
//             switch(coff){
//                 case AA: params.hi_filter_aa_ch1 = value; break;
//                 case BB: params.hi_filter_bb_ch1 = value; break;
//                 case PP: params.hi_filter_pp_ch1 = value; break;
//                 case KK: params.hi_filter_kk_ch1 = value; break;
//                 default:
//                   break;
//             }
//         }
//         if (gain == RP_LOW){
//             switch(coff){
//                 case AA: params.low_filter_aa_ch1 = value; break;
//                 case BB: params.low_filter_bb_ch1 = value; break;
//                 case PP: params.low_filter_pp_ch1 = value; break;
//                 case KK: params.low_filter_kk_ch1 = value; break;
//                 default:
//                     break;
//             }
//         }
//     }
//     if (channel == RP_CH_2){
//         if (gain == RP_HIGH){
//             switch(coff){
//                 case AA: params.hi_filter_aa_ch2 = value; break;
//                 case BB: params.hi_filter_bb_ch2 = value; break;
//                 case PP: params.hi_filter_pp_ch2 = value; break;
//                 case KK: params.hi_filter_kk_ch2 = value; break;
//                 default:
//                   break;
//             }
//         }
//         if (gain == RP_LOW){
//             switch(coff){
//                 case AA: params.low_filter_aa_ch2 = value; break;
//                 case BB: params.low_filter_bb_ch2 = value; break;
//                 case PP: params.low_filter_pp_ch2 = value; break;
//                 case KK: params.low_filter_kk_ch2 = value; break;
//                 default:
//                   break;
//             }
//         }
//     }
//     calib_WriteParams(calib,false);
//     return RP_OK;
// }

// uint32_t calib_GetFilterCoff(rp_channel_t channel, rp_pinState_t gain, rp_eq_filter_cof_t coff){
//      if (channel == RP_CH_1){
//         if (gain == RP_HIGH){
//             switch(coff){
//                 case AA:
//                 return calib.hi_filter_aa_ch1;
//                 case BB:
//                 return calib.hi_filter_bb_ch1;
//                 case PP:
//                 return calib.hi_filter_pp_ch1;
//                 case KK:
//                 return calib.hi_filter_kk_ch1;
//                 default:
//                   break;
//             }
//         }
//         if (gain == RP_LOW){
//             switch(coff){
//                 case AA:
//                 return calib.low_filter_aa_ch1;
//                 case BB:
//                 return calib.low_filter_bb_ch1;
//                 case PP:
//                 return calib.low_filter_pp_ch1;
//                 case KK:
//                 return calib.low_filter_kk_ch1;
//                 default:
//                   break;
//             }
//         }
//     }
//     if (channel == RP_CH_2){
//         if (gain == RP_HIGH){
//             switch(coff){
//                 case AA:
//                 return calib.hi_filter_aa_ch2;
//                 case BB:
//                 return calib.hi_filter_bb_ch2;
//                 case PP:
//                 return calib.hi_filter_pp_ch2;
//                 case KK:
//                 return calib.hi_filter_kk_ch2;
//                 default:
//                   break;
//             }
//         }
//         if (gain == RP_LOW){
//             switch(coff){
//                 case AA:
//                 return calib.low_filter_aa_ch2;
//                 case BB:
//                 return calib.low_filter_bb_ch2;
//                 case PP:
//                 return calib.low_filter_pp_ch2;
//                 case KK:
//                 return calib.low_filter_kk_ch2;
//                 default:
//                   break;
//             }
//         }
//     }
//     return 0;
// }

// #endif