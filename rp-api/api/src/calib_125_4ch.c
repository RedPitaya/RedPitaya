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

#define _XOPEN_SOURCE   600
#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rp_cross.h"
#include "common.h"
#include "generate.h"
#include "calib.h"


#define GAIN_LO_FILT_AA 0x7D93
#define GAIN_LO_FILT_BB 0x437C7
#define GAIN_LO_FILT_PP 0x2666
#define GAIN_LO_FILT_KK 0xd9999a
#define GAIN_HI_FILT_AA 0x4205
#define GAIN_HI_FILT_BB 0x2F38B
#define GAIN_HI_FILT_PP 0x2666
#define GAIN_HI_FILT_KK 0xd9999a

int calib_ReadParams(rp_calib_params_t *calib_params,bool use_factory_zone);
rp_calib_params_t getDefualtCalib();

static const char eeprom_device[]="/sys/bus/i2c/devices/0-0050/eeprom";
static const int  eeprom_calib_off=0x0008;
static const int  eeprom_calib_factory_off = 0x1c08;

// Cached parameter values.
static rp_calib_params_t calib, failsafa_params;

int calib_Init(){
    calib_ReadParams(&calib,false);
    return RP_OK;
}

int calib_Release(){
    return RP_OK;
}

/**
 * Returns cached parameter values
 * @return Cached parameters.
 */
rp_calib_params_t calib_GetParams(){
    return calib;
}

rp_calib_params_t calib_GetDefaultCalib(){
    return getDefualtCalib();
}

int calib_ReadParams(rp_calib_params_t *calib_params,bool use_factory_zone){
    FILE   *fp;
    size_t  size;

    /* sanity check */
    if(calib_params == NULL) {
        return RP_UIA;
    }

    /* open EEPROM device */
    fp = fopen(eeprom_device, "r");
    if(fp == NULL) {
        return RP_EOED;
    }

    /* ...and seek to the appropriate storage offset */
    int offset = use_factory_zone ? eeprom_calib_factory_off : eeprom_calib_off;
    if(fseek(fp, offset, SEEK_SET) < 0) {
        fclose(fp);
        return RP_FCA;
    }

    /* read data from EEPROM component and store it to the specified buffer */
    size = fread(calib_params, sizeof(char), sizeof(rp_calib_params_t), fp);
    if(size != sizeof(rp_calib_params_t)) {
        fclose(fp);
        return RP_RCA;
    }
    fclose(fp);
    return 0;
}


int calib_LoadFromFactoryZone(){
    rp_calib_params_t calib_values;
    int ret_val = calib_ReadParams(&calib_values,true);
    if (ret_val != 0)
        return ret_val;

    ret_val = calib_WriteParams(calib_values,false);
    if (ret_val != 0)
        return ret_val;

    return calib_Init();
}

int calib_WriteParams(rp_calib_params_t calib_params,bool use_factory_zone) {
    FILE   *fp;
    size_t  size;

    /* open EEPROM device */
    fp = fopen(eeprom_device, "w+");
    if(fp == NULL) {
        return RP_EOED;
    }

    /* ...and seek to the appropriate storage offset */
    int offset = use_factory_zone ? eeprom_calib_factory_off : eeprom_calib_off;
    if(fseek(fp, offset, SEEK_SET) < 0) {
        fclose(fp);
        return RP_FCA;
    }

    /* write data to EEPROM component */
    size = fwrite(&calib_params, sizeof(char), sizeof(rp_calib_params_t), fp);
    if(size != sizeof(rp_calib_params_t)) {
        fclose(fp);
        return RP_RCA;
    }
    fclose(fp);

    return RP_OK;
}

int calib_SetParams(rp_calib_params_t calib_params){
    calib = calib_params;
    return RP_OK;
}

rp_calib_params_t getDefualtCalib(){
    rp_calib_params_t calib;

    memset (&calib,0,sizeof(rp_calib_params_t));

    calib.chA_g_hi = calib.chB_g_hi = calib.chC_g_hi = calib.chD_g_hi = cmn_CalibFullScaleFromVoltage(1.0 );
    calib.chA_hi_offs = calib.chB_hi_offs = calib.chC_hi_offs = calib.chD_hi_offs = 0; 

    calib.chA_g_low = calib.chB_g_low = calib.chC_g_low = calib.chD_g_low = cmn_CalibFullScaleFromVoltage(20.0 );
    calib.chA_low_offs = calib.chB_low_offs = calib.chC_low_offs = calib.chD_low_offs = 0; 
    
    calib.chA_hi_aa = calib.chB_hi_aa = calib.chC_hi_aa = calib.chD_hi_aa = GAIN_HI_FILT_AA;
    calib.chA_hi_bb = calib.chB_hi_bb = calib.chC_hi_bb = calib.chD_hi_bb = GAIN_HI_FILT_BB;
    calib.chA_hi_pp = calib.chB_hi_pp = calib.chC_hi_pp = calib.chD_hi_pp = GAIN_HI_FILT_PP;
    calib.chA_hi_kk = calib.chB_hi_kk = calib.chC_hi_kk = calib.chD_hi_kk = GAIN_HI_FILT_KK;

    calib.chA_low_aa = calib.chB_low_aa = calib.chC_low_aa = calib.chD_low_aa = GAIN_LO_FILT_AA;
    calib.chA_low_bb = calib.chB_low_bb = calib.chC_low_bb = calib.chD_low_bb = GAIN_LO_FILT_BB;
    calib.chA_low_pp = calib.chB_low_pp = calib.chC_low_pp = calib.chD_low_pp = GAIN_LO_FILT_PP;
    calib.chA_low_kk = calib.chB_low_kk = calib.chC_low_kk = calib.chD_low_kk = GAIN_LO_FILT_KK;

    return calib;
}

void calib_SetToZero() {
    calib = getDefualtCalib();
}

uint32_t calib_GetFrontEndScale(rp_channel_t channel, rp_pinState_t gain) {
    if (gain == RP_HIGH) {
        switch(channel){
            case RP_CH_1:
                return calib.chA_g_hi;
            case RP_CH_2:
                return calib.chB_g_hi;
            case RP_CH_3:
                return calib.chC_g_hi;
            case RP_CH_4:
                return calib.chD_g_hi;
            default:
                return UINT32_MAX;
        }
    }
    else {
        switch(channel){
            case RP_CH_1:
                return calib.chA_g_low;
            case RP_CH_2:
                return calib.chB_g_low;
            case RP_CH_3:
                return calib.chC_g_low;
            case RP_CH_4:
                return calib.chD_g_low;
            default:
                return UINT32_MAX;
        }
    }
}

int calib_SetFrontEndOffset(rp_channel_t channel, rp_pinState_t gain, rp_calib_params_t* out_params) {
    rp_calib_params_t params;
    calib_ReadParams(&params,false);
	failsafa_params = params;

    /* Reset current calibration parameters*/
    if (gain == RP_LOW) {
		CHANNEL_ACTION_4CH(channel,
            params.chA_low_offs = 0,
            params.chB_low_offs = 0,
            params.chC_low_offs = 0,
            params.chD_low_offs = 0)
    } else {
		CHANNEL_ACTION_4CH(channel,
            params.chA_hi_offs = 0,
            params.chB_hi_offs = 0,
            params.chC_hi_offs = 0,
            params.chD_hi_offs = 0)
	}
    /* Acquire uses this calibration parameters - reset them */
    calib = params;

	if (gain == RP_LOW) {
		CHANNEL_ACTION_4CH(channel,
			params.chA_low_offs = calib_GetDataMedian(channel, RP_LOW),
			params.chB_low_offs = calib_GetDataMedian(channel, RP_LOW),
            params.chC_low_offs = calib_GetDataMedian(channel, RP_LOW),
            params.chD_low_offs = calib_GetDataMedian(channel, RP_LOW))
	} else {
		CHANNEL_ACTION_4CH(channel,
			params.chA_hi_offs = calib_GetDataMedian(channel, RP_HIGH),
			params.chB_hi_offs = calib_GetDataMedian(channel, RP_HIGH),
            params.chC_hi_offs = calib_GetDataMedian(channel, RP_HIGH),
            params.chD_hi_offs = calib_GetDataMedian(channel, RP_HIGH))
	}

    /* Set new local parameter */
    if  (out_params) {
		//	*out_params = params;
		if (gain == RP_LOW) {
			CHANNEL_ACTION_4CH(channel,
				out_params->chA_low_offs = params.chA_low_offs,
				out_params->chB_low_offs = params.chB_low_offs,
                out_params->chC_low_offs = params.chC_low_offs,
                out_params->chD_low_offs = params.chD_low_offs)
		} else {
			CHANNEL_ACTION_4CH(channel,
				out_params->chA_hi_offs = params.chA_hi_offs,
				out_params->chB_hi_offs = params.chB_hi_offs,
                out_params->chC_hi_offs = params.chC_hi_offs,
                out_params->chD_hi_offs = params.chD_hi_offs)
		}
	}
    else
		calib_WriteParams(params,false);
    return calib_Init();
}

int calib_SetFrontEndScaleLV(rp_channel_t channel, float referentialVoltage, rp_calib_params_t* out_params) {
    rp_calib_params_t params;
    calib_ReadParams(&params,false);
	failsafa_params = params;

    /* Reset current calibration parameters*/
    CHANNEL_ACTION_4CH(channel,
            params.chA_g_low = cmn_CalibFullScaleFromVoltage(20),
            params.chB_g_low = cmn_CalibFullScaleFromVoltage(20),
            params.chC_g_low = cmn_CalibFullScaleFromVoltage(20),
            params.chD_g_low = cmn_CalibFullScaleFromVoltage(20))
    /* Acquire uses this calibration parameters - reset them */
    calib = params;

    /* Calculate real max adc voltage */
    float value = calib_GetDataMedianFloat(channel, RP_LOW);
    uint32_t calibValue = cmn_CalibFullScaleFromVoltage(20.f * referentialVoltage / value);

    CHANNEL_ACTION_4CH(channel,
            params.chA_g_low = calibValue,
            params.chB_g_low = calibValue,
            params.chC_g_low = calibValue,
            params.chD_g_low = calibValue)

    /* Set new local parameter */
    if  (out_params) {
		//	*out_params = params;
		CHANNEL_ACTION_4CH(channel,
				out_params->chA_g_low = params.chA_g_low,
				out_params->chB_g_low = params.chB_g_low,
				out_params->chC_g_low = params.chC_g_low,
				out_params->chD_g_low = params.chD_g_low)
	}
    else
		calib_WriteParams(params,false);
    return calib_Init();
}

int calib_SetFrontEndScaleHV(rp_channel_t channel, float referentialVoltage, rp_calib_params_t* out_params) {
    rp_calib_params_t params;
    calib_ReadParams(&params,false);
    failsafa_params = params;

    /* Reset current calibration parameters*/
    CHANNEL_ACTION_4CH(channel,
            params.chA_g_hi = cmn_CalibFullScaleFromVoltage(1),
            params.chB_g_hi = cmn_CalibFullScaleFromVoltage(1),
            params.chC_g_hi = cmn_CalibFullScaleFromVoltage(1),
            params.chD_g_hi = cmn_CalibFullScaleFromVoltage(1))
    /* Acquire uses this calibration parameters - reset them */
    calib = params;

    /* Calculate real max adc voltage */
    float value = calib_GetDataMedianFloat(channel, RP_HIGH);
    uint32_t calibValue = cmn_CalibFullScaleFromVoltage(referentialVoltage / value);

    CHANNEL_ACTION_4CH(channel,
            params.chA_g_hi = calibValue,
            params.chB_g_hi = calibValue,
            params.chC_g_hi = calibValue,
            params.chD_g_hi = calibValue)

    /* Set new local parameter */
    if  (out_params) {
		//	*out_params = params;
		CHANNEL_ACTION_4CH(channel,
				out_params->chA_g_hi = params.chA_g_hi,
				out_params->chB_g_hi = params.chB_g_hi,
				out_params->chC_g_hi = params.chC_g_hi,
				out_params->chD_g_hi = params.chD_g_hi)
	}
    else
		calib_WriteParams(params,false);
    return calib_Init();
}

int calib_Reset() {
    calib_SetToZero();
    calib_WriteParams(calib,false);
    return calib_Init();
}

int32_t calib_GetDataMedian(rp_channel_t channel, rp_pinState_t gain) {
    /* Acquire data */
    rp_AcqReset();
    rp_AcqSetGain(channel, gain);
    rp_AcqSetDecimation(RP_DEC_64);
    rp_AcqStart();
    rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);
    usleep(1000000);
    rp_AcqStop();

    int16_t data[ADC_BUFFER_SIZE];
    uint32_t bufferSize = (uint32_t) ADC_BUFFER_SIZE;
    rp_AcqGetDataRaw(channel, 0, &bufferSize, data);

    long long avg = 0;
    for(int i = 0; i < ADC_BUFFER_SIZE; ++i)
        avg += data[i];

    avg /= ADC_BUFFER_SIZE;
    fprintf(stderr, "\ncalib_GetDataMedian: avg = %d\n", (int32_t)avg);
    return avg;
}

float calib_GetDataMedianFloat(rp_channel_t channel, rp_pinState_t gain) {
    rp_AcqReset();
    rp_AcqSetGain(channel, gain);
    rp_AcqSetDecimation(RP_DEC_64);
    rp_AcqStart();
    rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);
    usleep(1000000);
    int BUF_SIZE = ADC_BUFFER_SIZE;

    rp_AcqStop();

    float data[BUF_SIZE];
    uint32_t bufferSize = (uint32_t) BUF_SIZE;
    rp_AcqGetDataV(channel, 0, &bufferSize, data);

    double avg = 0;
    for(int i = 0; i < ADC_BUFFER_SIZE; ++i)
        avg += data[i];

    avg /= ADC_BUFFER_SIZE;
    fprintf(stderr, "\ncalib_GetDataMedianFloat: avg = %f\n", (float)avg);
    return avg;
}

int calib_GetDataMinMaxFloat(rp_channel_t channel, rp_pinState_t gain, float* min, float* max) {
    rp_AcqReset();
    rp_AcqSetGain(channel, gain);
    rp_AcqSetDecimation(RP_DEC_64);
    rp_AcqStart();
    rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);
    usleep(1000000);
    int BUF_SIZE = ADC_BUFFER_SIZE;

    rp_AcqStop();

    float data[BUF_SIZE];
    uint32_t bufferSize = (uint32_t) BUF_SIZE;
    rp_AcqGetDataV(channel, 0, &bufferSize, data);

    float _min = data[0];
    float _max = data[0];
    for(int i = 1; i < ADC_BUFFER_SIZE; ++i) {
        _min = (_min > data[i]) ? data[i] : _min;
        _max = (_max < data[i]) ? data[i] : _max;
    }

    fprintf(stderr, "\ncalib_GetDataMinMaxFloat: min = %f, max = %f\n", _min, _max);
    *min = _min;
    *max = _max;
    return RP_OK;
}

int calib_setCachedParams() {
	fprintf(stderr, "write FAILSAFE PARAMS\n");
    calib_WriteParams(failsafa_params,false);
    calib = failsafa_params;

    return 0;
}

int32_t calib_getOffset(rp_channel_t channel, rp_pinState_t gain){
    if (gain == RP_HIGH) {
        switch(channel){
            case RP_CH_1:
                return calib.chA_hi_offs;
            case RP_CH_2:
                return calib.chB_hi_offs;
            case RP_CH_3:
                return calib.chC_hi_offs;
            case RP_CH_4:
                return calib.chD_hi_offs;
            default:
                return UINT32_MAX;
        }
    }
    else {
        switch(channel){
            case RP_CH_1:
                return calib.chA_low_offs;
            case RP_CH_2:
                return calib.chB_low_offs;
            case RP_CH_3:
                return calib.chC_low_offs;
            case RP_CH_4:
                return calib.chD_low_offs;
            default:
                return UINT32_MAX;
        }
    }
}

int calib_SetFilterCoff(rp_channel_t channel, rp_pinState_t gain, rp_eq_filter_cof_t coff , uint32_t value){
    rp_calib_params_t params;
    calib_ReadParams(&params,false);
    if (channel == RP_CH_1){
        if (gain == RP_HIGH){
            switch(coff){
                case AA: params.chA_hi_aa = value; break;
                case BB: params.chA_hi_bb = value; break;
                case PP: params.chA_hi_pp = value; break;
                case KK: params.chA_hi_kk = value; break; 
                default:
                  break;
            }
        }
        if (gain == RP_LOW){
            switch(coff){
                case AA: params.chA_low_aa = value; break;
                case BB: params.chA_low_bb = value; break;
                case PP: params.chA_low_pp = value; break;
                case KK: params.chA_low_kk = value; break; 
                default:
                    break;
            }
        }
    }
    
    if (channel == RP_CH_2){
        if (gain == RP_HIGH){
            switch(coff){
                case AA: params.chB_hi_aa = value; break;
                case BB: params.chB_hi_bb = value; break;
                case PP: params.chB_hi_pp = value; break;
                case KK: params.chB_hi_kk = value; break; 
                default:
                  break;
            }
        }
        if (gain == RP_LOW){
            switch(coff){
                case AA: params.chB_low_aa = value; break;
                case BB: params.chB_low_bb = value; break;
                case PP: params.chB_low_pp = value; break;
                case KK: params.chB_low_kk = value; break; 
                default:
                    break;
            }
        }
    }

    if (channel == RP_CH_3){
        if (gain == RP_HIGH){
            switch(coff){
                case AA: params.chC_hi_aa = value; break;
                case BB: params.chC_hi_bb = value; break;
                case PP: params.chC_hi_pp = value; break;
                case KK: params.chC_hi_kk = value; break; 
                default:
                  break;
            }
        }
        if (gain == RP_LOW){
            switch(coff){
                case AA: params.chC_low_aa = value; break;
                case BB: params.chC_low_bb = value; break;
                case PP: params.chC_low_pp = value; break;
                case KK: params.chC_low_kk = value; break; 
                default:
                    break;
            }
        }
    }

    if (channel == RP_CH_4){
        if (gain == RP_HIGH){
            switch(coff){
                case AA: params.chD_hi_aa = value; break;
                case BB: params.chD_hi_bb = value; break;
                case PP: params.chD_hi_pp = value; break;
                case KK: params.chD_hi_kk = value; break; 
                default:
                  break;
            }
        }
        if (gain == RP_LOW){
            switch(coff){
                case AA: params.chD_low_aa = value; break;
                case BB: params.chD_low_bb = value; break;
                case PP: params.chD_low_pp = value; break;
                case KK: params.chD_low_kk = value; break; 
                default:
                    break;
            }
        }
    }
    calib_WriteParams(calib,false);
    return RP_OK;
}

uint32_t calib_GetFilterCoff(rp_channel_t channel, rp_pinState_t gain, rp_eq_filter_cof_t coff){
    if (channel == RP_CH_1){
        if (gain == RP_HIGH){
            switch(coff){
                case AA:
                return calib.chA_hi_aa;
                case BB:
                return calib.chA_hi_bb;
                case PP:
                return calib.chA_hi_pp;
                case KK:
                return calib.chA_hi_kk; 
                default:
                  break;
            }
        }
        if (gain == RP_LOW){
            switch(coff){
                case AA:
                return calib.chA_low_aa;
                case BB:
                return calib.chA_low_bb;
                case PP:
                return calib.chA_low_pp;
                case KK:
                return calib.chA_low_kk; 
                default:
                  break;
            }
        }
    }

    if (channel == RP_CH_2){
        if (gain == RP_HIGH){
            switch(coff){
                case AA:
                return calib.chB_hi_aa;
                case BB:
                return calib.chB_hi_bb;
                case PP:
                return calib.chB_hi_pp;
                case KK:
                return calib.chB_hi_kk; 
                default:
                  break;
            }
        }
        if (gain == RP_LOW){
            switch(coff){
                case AA:
                return calib.chB_low_aa;
                case BB:
                return calib.chB_low_bb;
                case PP:
                return calib.chB_low_pp;
                case KK:
                return calib.chB_low_kk; 
                default:
                  break;
            }
        }
    }

    if (channel == RP_CH_3){
        if (gain == RP_HIGH){
            switch(coff){
                case AA:
                return calib.chC_hi_aa;
                case BB:
                return calib.chC_hi_bb;
                case PP:
                return calib.chC_hi_pp;
                case KK:
                return calib.chC_hi_kk; 
                default:
                  break;
            }
        }
        if (gain == RP_LOW){
            switch(coff){
                case AA:
                return calib.chC_low_aa;
                case BB:
                return calib.chC_low_bb;
                case PP:
                return calib.chC_low_pp;
                case KK:
                return calib.chC_low_kk; 
                default:
                  break;
            }
        }
    }

    if (channel == RP_CH_4){
        if (gain == RP_HIGH){
            switch(coff){
                case AA:
                return calib.chD_hi_aa;
                case BB:
                return calib.chD_hi_bb;
                case PP:
                return calib.chD_hi_pp;
                case KK:
                return calib.chD_hi_kk; 
                default:
                  break;
            }
        }
        if (gain == RP_LOW){
            switch(coff){
                case AA:
                return calib.chD_low_aa;
                case BB:
                return calib.chD_low_bb;
                case PP:
                return calib.chD_low_pp;
                case KK:
                return calib.chD_low_kk; 
                default:
                  break;
            }
        }
    }
    
    return 0;
}
