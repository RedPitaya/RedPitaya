#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "calib_common.h"
#include "calib_universal.h"

#define GET_PARAMETER(X,Y,Z)    { \
                                    bool err = false; \
                                    int32_t val = getParameter(Z,Y,&err); \
                                    if (err) { \
                                        fprintf(stderr,"[Error] Missing calib item %s (%d) .\n",getNameOfUniversalId(Y),Y); \
                                    } \
                                    X = val; \
                                }

#define SET_PARAMETER(X,Y)      { \
                                    out->item[count].id = X; \
                                    out->item[count++].value = Y; \
                                }


char* getNameOfUniversalId(uint16_t id){
    switch((rp_calib_universal_id_t)id){
        case UC_DAC_CH1_GAIN:           return "DAC Ch1 Gain";
        case UC_DAC_CH1_OFFSET:         return "DAC Ch1 Offset";
        case UC_DAC_CH2_GAIN:           return "DAC Ch2 Gain";
        case UC_DAC_CH2_OFFSET:         return "DAC Ch2 Offset";
        case UC_DAC_CH1_GAIN_X5:        return "DAC Ch1 Gain x5";
        case UC_DAC_CH1_OFFSET_X5:      return "DAC Ch1 Offset x5";
        case UC_DAC_CH2_GAIN_X5:        return "DAC Ch2 Gain x5";
        case UC_DAC_CH2_OFFSET_X5:      return "DAC Ch2 Offset x5";

        case UC_ADC_CH1_GAIN_1_1:       return "ADC Ch1 Gain 1/1";
        case UC_ADC_CH1_OFFSET_1_1:     return "ADC Ch1 Offset 1/1";
        case UC_ADC_CH2_GAIN_1_1:       return "ADC Ch2 Gain 1/1";
        case UC_ADC_CH2_OFFSET_1_1:     return "ADC Ch2 Offset 1/1";
        case UC_ADC_CH3_GAIN_1_1:       return "ADC Ch3 Gain 1/1";
        case UC_ADC_CH3_OFFSET_1_1:     return "ADC Ch3 Offset 1/1";
        case UC_ADC_CH4_GAIN_1_1:       return "ADC Ch4 Gain 1/1";
        case UC_ADC_CH4_OFFSET_1_1:     return "ADC Ch4 Offset 1/1";

        case UC_ADC_CH1_GAIN_1_20:       return "ADC Ch1 Gain 1/20";
        case UC_ADC_CH1_OFFSET_1_20:     return "ADC Ch1 Offset 1/20";
        case UC_ADC_CH2_GAIN_1_20:       return "ADC Ch2 Gain 1/20";
        case UC_ADC_CH2_OFFSET_1_20:     return "ADC Ch2 Offset 1/20";
        case UC_ADC_CH3_GAIN_1_20:       return "ADC Ch3 Gain 1/20";
        case UC_ADC_CH3_OFFSET_1_20:     return "ADC Ch3 Offset 1/20";
        case UC_ADC_CH4_GAIN_1_20:       return "ADC Ch4 Gain 1/20";
        case UC_ADC_CH4_OFFSET_1_20:     return "ADC Ch4 Offset 1/20";

        case UC_ADC_CH1_GAIN_1_1_AC:     return "ADC Ch1 Gain 1/1 AC";
        case UC_ADC_CH1_OFFSET_1_1_AC:   return "ADC Ch1 Offset 1/1 AC";
        case UC_ADC_CH2_GAIN_1_1_AC:     return "ADC Ch2 Gain 1/1 AC";
        case UC_ADC_CH2_OFFSET_1_1_AC:   return "ADC Ch2 Offset 1/1 AC";

        case UC_ADC_CH1_GAIN_1_20_AC:     return "ADC Ch1 Gain 1/20 AC";
        case UC_ADC_CH1_OFFSET_1_20_AC:   return "ADC Ch1 Offset 1/20 AC";
        case UC_ADC_CH2_GAIN_1_20_AC:     return "ADC Ch2 Gain 1/20 AC";
        case UC_ADC_CH2_OFFSET_1_20_AC:   return "ADC Ch2 Offset 1/20 AC";

        case UC_ADC_CH1_AA_1_1:           return "ADC Ch1 AA 1/1";
        case UC_ADC_CH1_BB_1_1:           return "ADC Ch1 BB 1/1";
        case UC_ADC_CH1_PP_1_1:           return "ADC Ch1 PP 1/1";
        case UC_ADC_CH1_KK_1_1:           return "ADC Ch1 KK 1/1";

        case UC_ADC_CH2_AA_1_1:           return "ADC Ch2 AA 1/1";
        case UC_ADC_CH2_BB_1_1:           return "ADC Ch2 BB 1/1";
        case UC_ADC_CH2_PP_1_1:           return "ADC Ch2 PP 1/1";
        case UC_ADC_CH2_KK_1_1:           return "ADC Ch2 KK 1/1";

        case UC_ADC_CH3_AA_1_1:           return "ADC Ch3 AA 1/1";
        case UC_ADC_CH3_BB_1_1:           return "ADC Ch3 BB 1/1";
        case UC_ADC_CH3_PP_1_1:           return "ADC Ch3 PP 1/1";
        case UC_ADC_CH3_KK_1_1:           return "ADC Ch3 KK 1/1";

        case UC_ADC_CH4_AA_1_1:           return "ADC Ch4 AA 1/1";
        case UC_ADC_CH4_BB_1_1:           return "ADC Ch4 BB 1/1";
        case UC_ADC_CH4_PP_1_1:           return "ADC Ch4 PP 1/1";
        case UC_ADC_CH4_KK_1_1:           return "ADC Ch4 KK 1/1";

        case UC_ADC_CH1_AA_1_20:           return "ADC Ch1 AA 1/20";
        case UC_ADC_CH1_BB_1_20:           return "ADC Ch1 BB 1/20";
        case UC_ADC_CH1_PP_1_20:           return "ADC Ch1 PP 1/20";
        case UC_ADC_CH1_KK_1_20:           return "ADC Ch1 KK 1/20";

        case UC_ADC_CH2_AA_1_20:           return "ADC Ch2 AA 1/20";
        case UC_ADC_CH2_BB_1_20:           return "ADC Ch2 BB 1/20";
        case UC_ADC_CH2_PP_1_20:           return "ADC Ch2 PP 1/20";
        case UC_ADC_CH2_KK_1_20:           return "ADC Ch2 KK 1/20";

        case UC_ADC_CH3_AA_1_20:           return "ADC Ch3 AA 1/20";
        case UC_ADC_CH3_BB_1_20:           return "ADC Ch3 BB 1/20";
        case UC_ADC_CH3_PP_1_20:           return "ADC Ch3 PP 1/20";
        case UC_ADC_CH3_KK_1_20:           return "ADC Ch3 KK 1/20";

        case UC_ADC_CH4_AA_1_20:           return "ADC Ch4 AA 1/20";
        case UC_ADC_CH4_BB_1_20:           return "ADC Ch4 BB 1/20";
        case UC_ADC_CH4_PP_1_20:           return "ADC Ch4 PP 1/20";
        case UC_ADC_CH4_KK_1_20:           return "ADC Ch4 KK 1/20";
    }
    return NULL;
}

int32_t getParameter(rp_calib_params_universal_t *param,rp_calib_universal_id_t id,bool *error){
    *error = false;
    if (param->count > MAX_UNIVERSAL_ITEMS_COUNT) {
        *error = true;
        return 0;
    }

    for(uint16_t i = 0; i < param->count; i++){
        if (param->item[i].id == id){
            return param->item[i].value;
        }
    }
    *error = true;
    return 0;
}

rp_calib_params_t convertUniversaltoCommon(rp_HPeModels_t model,rp_calib_params_universal_t *param){
    rp_calib_params_t calib;
    memset(&calib,0,sizeof(rp_calib_params_t));

    calib.dataStructureId = param->dataStructureId;
    calib.wpCheck = param->wpCheck;
    switch (model)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:{
            calib.fast_adc_count_1_1 = 2;
            calib.fast_adc_count_1_20 = 2;
            calib.fast_dac_count_x1 = 2;

            calib.fast_adc_1_1[0].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_1[0].calibValue,UC_ADC_CH1_GAIN_1_1,param)
            GET_PARAMETER(calib.fast_adc_1_1[0].offset,UC_ADC_CH1_OFFSET_1_1,param)

            calib.fast_adc_1_1[1].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_1[1].calibValue,UC_ADC_CH2_GAIN_1_1,param)
            GET_PARAMETER(calib.fast_adc_1_1[1].offset,UC_ADC_CH2_OFFSET_1_1,param)

            calib.fast_adc_1_20[0].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_20[0].calibValue,UC_ADC_CH1_GAIN_1_20,param)
            GET_PARAMETER(calib.fast_adc_1_20[0].offset,UC_ADC_CH1_OFFSET_1_20,param)

            calib.fast_adc_1_20[1].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_20[1].calibValue,UC_ADC_CH2_GAIN_1_20,param)
            GET_PARAMETER(calib.fast_adc_1_20[1].offset,UC_ADC_CH2_OFFSET_1_20,param)

            GET_PARAMETER(calib.fast_adc_filter_1_1[0].aa,UC_ADC_CH1_AA_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[0].bb,UC_ADC_CH1_BB_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[0].pp,UC_ADC_CH1_PP_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[0].kk,UC_ADC_CH1_KK_1_1,param)

            GET_PARAMETER(calib.fast_adc_filter_1_20[0].aa,UC_ADC_CH1_AA_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[0].bb,UC_ADC_CH1_BB_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[0].pp,UC_ADC_CH1_PP_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[0].kk,UC_ADC_CH1_KK_1_20,param)

            GET_PARAMETER(calib.fast_adc_filter_1_1[1].aa,UC_ADC_CH2_AA_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[1].bb,UC_ADC_CH2_BB_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[1].pp,UC_ADC_CH2_PP_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[1].kk,UC_ADC_CH2_KK_1_1,param)

            GET_PARAMETER(calib.fast_adc_filter_1_20[1].aa,UC_ADC_CH2_AA_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[1].bb,UC_ADC_CH2_BB_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[1].pp,UC_ADC_CH2_PP_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[1].kk,UC_ADC_CH2_KK_1_20,param)

            calib.fast_dac_x1[0].baseScale = 1.0;
            GET_PARAMETER(calib.fast_dac_x1[0].calibValue,UC_DAC_CH1_GAIN,param)
            GET_PARAMETER(calib.fast_dac_x1[0].offset,UC_DAC_CH1_OFFSET,param)

            calib.fast_dac_x1[1].baseScale = 1.0;
            GET_PARAMETER(calib.fast_dac_x1[1].calibValue,UC_DAC_CH2_GAIN,param)
            GET_PARAMETER(calib.fast_dac_x1[1].offset,UC_DAC_CH2_OFFSET,param)

            break;
        }

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:{
            calib.fast_adc_count_1_1 = 2;
            calib.fast_dac_count_x1 = 2;

            calib.fast_adc_1_1[0].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_1[0].calibValue,UC_ADC_CH1_GAIN_1_1,param)
            GET_PARAMETER(calib.fast_adc_1_1[0].offset,UC_ADC_CH1_OFFSET_1_1,param)

            calib.fast_adc_1_1[1].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_1[1].calibValue,UC_ADC_CH2_GAIN_1_1,param)
            GET_PARAMETER(calib.fast_adc_1_1[1].offset,UC_ADC_CH2_OFFSET_1_1,param)

            calib.fast_dac_x1[0].baseScale = 1.0;
            GET_PARAMETER(calib.fast_dac_x1[0].calibValue,UC_DAC_CH1_GAIN,param)
            GET_PARAMETER(calib.fast_dac_x1[0].offset,UC_DAC_CH1_OFFSET,param)

            calib.fast_dac_x1[1].baseScale = 1.0;
            GET_PARAMETER(calib.fast_dac_x1[1].calibValue,UC_DAC_CH2_GAIN,param)
            GET_PARAMETER(calib.fast_dac_x1[1].offset,UC_DAC_CH2_OFFSET,param)
            break;
        }

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:{
            calib.fast_adc_count_1_1 = 4;
            calib.fast_adc_count_1_20 = 4;

            calib.fast_adc_1_1[0].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_1[0].calibValue,UC_ADC_CH1_GAIN_1_1,param)
            GET_PARAMETER(calib.fast_adc_1_1[0].offset,UC_ADC_CH1_OFFSET_1_1,param)

            calib.fast_adc_1_1[1].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_1[1].calibValue,UC_ADC_CH2_GAIN_1_1,param)
            GET_PARAMETER(calib.fast_adc_1_1[1].offset,UC_ADC_CH2_OFFSET_1_1,param)

            calib.fast_adc_1_1[2].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_1[2].calibValue,UC_ADC_CH3_GAIN_1_1,param)
            GET_PARAMETER(calib.fast_adc_1_1[2].offset,UC_ADC_CH3_OFFSET_1_1,param)

            calib.fast_adc_1_1[3].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_1[3].calibValue,UC_ADC_CH4_GAIN_1_1,param)
            GET_PARAMETER(calib.fast_adc_1_1[3].offset,UC_ADC_CH4_OFFSET_1_1,param)

            GET_PARAMETER(calib.fast_adc_filter_1_1[0].aa,UC_ADC_CH1_AA_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[0].bb,UC_ADC_CH1_BB_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[0].pp,UC_ADC_CH1_PP_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[0].kk,UC_ADC_CH1_KK_1_1,param)

            GET_PARAMETER(calib.fast_adc_filter_1_1[1].aa,UC_ADC_CH2_AA_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[1].bb,UC_ADC_CH2_BB_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[1].pp,UC_ADC_CH2_PP_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[1].kk,UC_ADC_CH2_KK_1_1,param)

            GET_PARAMETER(calib.fast_adc_filter_1_1[2].aa,UC_ADC_CH3_AA_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[2].bb,UC_ADC_CH3_BB_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[2].pp,UC_ADC_CH3_PP_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[2].kk,UC_ADC_CH3_KK_1_1,param)

            GET_PARAMETER(calib.fast_adc_filter_1_1[3].aa,UC_ADC_CH4_AA_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[3].bb,UC_ADC_CH4_BB_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[3].pp,UC_ADC_CH4_PP_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[3].kk,UC_ADC_CH4_KK_1_1,param)

            calib.fast_adc_1_20[0].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_20[0].calibValue,UC_ADC_CH1_GAIN_1_20,param)
            GET_PARAMETER(calib.fast_adc_1_20[0].offset,UC_ADC_CH1_OFFSET_1_20,param)

            calib.fast_adc_1_20[1].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_20[1].calibValue,UC_ADC_CH2_GAIN_1_20,param)
            GET_PARAMETER(calib.fast_adc_1_20[1].offset,UC_ADC_CH2_OFFSET_1_20,param)

            calib.fast_adc_1_20[2].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_20[2].calibValue,UC_ADC_CH3_GAIN_1_20,param)
            GET_PARAMETER(calib.fast_adc_1_20[2].offset,UC_ADC_CH3_OFFSET_1_20,param)

            calib.fast_adc_1_20[3].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_20[3].calibValue,UC_ADC_CH4_GAIN_1_20,param)
            GET_PARAMETER(calib.fast_adc_1_20[3].offset,UC_ADC_CH4_OFFSET_1_20,param)

            GET_PARAMETER(calib.fast_adc_filter_1_20[0].aa,UC_ADC_CH1_AA_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[0].bb,UC_ADC_CH1_BB_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[0].pp,UC_ADC_CH1_PP_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[0].kk,UC_ADC_CH1_KK_1_20,param)

            GET_PARAMETER(calib.fast_adc_filter_1_20[1].aa,UC_ADC_CH2_AA_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[1].bb,UC_ADC_CH2_BB_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[1].pp,UC_ADC_CH2_PP_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[1].kk,UC_ADC_CH2_KK_1_20,param)

            GET_PARAMETER(calib.fast_adc_filter_1_20[2].aa,UC_ADC_CH3_AA_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[2].bb,UC_ADC_CH3_BB_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[2].pp,UC_ADC_CH3_PP_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[2].kk,UC_ADC_CH3_KK_1_20,param)

            GET_PARAMETER(calib.fast_adc_filter_1_20[3].aa,UC_ADC_CH4_AA_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[3].bb,UC_ADC_CH4_BB_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[3].pp,UC_ADC_CH4_PP_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[3].kk,UC_ADC_CH4_KK_1_20,param)
            break;
        }

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_120:{
            calib.fast_adc_count_1_1 = 2;
            calib.fast_adc_count_1_20 = 2;
            calib.fast_adc_count_1_1_ac = 2;
            calib.fast_adc_count_1_20_ac = 2;
            calib.fast_dac_count_x1 = 2;
            calib.fast_dac_count_x5 = 2;

            calib.fast_adc_1_1[0].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_1[0].calibValue,UC_ADC_CH1_GAIN_1_1,param)
            GET_PARAMETER(calib.fast_adc_1_1[0].offset,UC_ADC_CH1_OFFSET_1_1,param)

            calib.fast_adc_1_1[1].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_1[1].calibValue,UC_ADC_CH2_GAIN_1_1,param)
            GET_PARAMETER(calib.fast_adc_1_1[1].offset,UC_ADC_CH2_OFFSET_1_1,param)

            calib.fast_adc_1_20[0].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_20[0].calibValue,UC_ADC_CH1_GAIN_1_20,param)
            GET_PARAMETER(calib.fast_adc_1_20[0].offset,UC_ADC_CH1_OFFSET_1_20,param)

            calib.fast_adc_1_20[1].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_20[1].calibValue,UC_ADC_CH2_GAIN_1_20,param)
            GET_PARAMETER(calib.fast_adc_1_20[1].offset,UC_ADC_CH2_OFFSET_1_20,param)

            GET_PARAMETER(calib.fast_adc_filter_1_1[0].aa,UC_ADC_CH1_AA_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[0].bb,UC_ADC_CH1_BB_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[0].pp,UC_ADC_CH1_PP_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[0].kk,UC_ADC_CH1_KK_1_1,param)

            GET_PARAMETER(calib.fast_adc_filter_1_20[0].aa,UC_ADC_CH1_AA_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[0].bb,UC_ADC_CH1_BB_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[0].pp,UC_ADC_CH1_PP_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[0].kk,UC_ADC_CH1_KK_1_20,param)

            GET_PARAMETER(calib.fast_adc_filter_1_1[1].aa,UC_ADC_CH2_AA_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[1].bb,UC_ADC_CH2_BB_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[1].pp,UC_ADC_CH2_PP_1_1,param)
            GET_PARAMETER(calib.fast_adc_filter_1_1[1].kk,UC_ADC_CH2_KK_1_1,param)

            GET_PARAMETER(calib.fast_adc_filter_1_20[1].aa,UC_ADC_CH2_AA_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[1].bb,UC_ADC_CH2_BB_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[1].pp,UC_ADC_CH2_PP_1_20,param)
            GET_PARAMETER(calib.fast_adc_filter_1_20[1].kk,UC_ADC_CH2_KK_1_20,param)

            calib.fast_adc_1_1_ac[0].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_1_ac[0].calibValue,UC_ADC_CH1_GAIN_1_1_AC,param)
            GET_PARAMETER(calib.fast_adc_1_1_ac[0].offset,UC_ADC_CH1_OFFSET_1_1_AC,param)

            calib.fast_adc_1_1_ac[1].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_1_ac[1].calibValue,UC_ADC_CH2_GAIN_1_1_AC,param)
            GET_PARAMETER(calib.fast_adc_1_1_ac[1].offset,UC_ADC_CH2_OFFSET_1_1_AC,param)

            calib.fast_adc_1_20_ac[0].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_20_ac[0].calibValue,UC_ADC_CH1_GAIN_1_20_AC,param)
            GET_PARAMETER(calib.fast_adc_1_20_ac[0].offset,UC_ADC_CH1_OFFSET_1_20_AC,param)

            calib.fast_adc_1_20_ac[1].baseScale = 1.0;
            GET_PARAMETER(calib.fast_adc_1_20_ac[1].calibValue,UC_ADC_CH2_GAIN_1_20_AC,param)
            GET_PARAMETER(calib.fast_adc_1_20_ac[1].offset,UC_ADC_CH2_OFFSET_1_20_AC,param)


            calib.fast_dac_x1[0].baseScale = 1.0;
            GET_PARAMETER(calib.fast_dac_x1[0].calibValue,UC_DAC_CH1_GAIN,param)
            GET_PARAMETER(calib.fast_dac_x1[0].offset,UC_DAC_CH1_OFFSET,param)

            calib.fast_dac_x1[1].baseScale = 1.0;
            GET_PARAMETER(calib.fast_dac_x1[1].calibValue,UC_DAC_CH2_GAIN,param)
            GET_PARAMETER(calib.fast_dac_x1[1].offset,UC_DAC_CH2_OFFSET,param)

            calib.fast_dac_x5[0].baseScale = 1.0;
            GET_PARAMETER(calib.fast_dac_x5[0].calibValue,UC_DAC_CH1_GAIN_X5,param)
            GET_PARAMETER(calib.fast_dac_x5[0].offset,UC_DAC_CH1_OFFSET_X5,param)

            calib.fast_dac_x5[1].baseScale = 1.0;
            GET_PARAMETER(calib.fast_dac_x5[1].calibValue,UC_DAC_CH2_GAIN_X5,param)
            GET_PARAMETER(calib.fast_dac_x5[1].offset,UC_DAC_CH2_GAIN_X5,param)
            break;
        }

        default:
            fprintf(stderr,"[Error:calib_WriteParams] Unknown model: %d.\n",model);
            break;
    }

    return calib;
}


bool convertUniversal(rp_HPeModels_t model,rp_calib_params_t *param,rp_calib_params_universal_t *out){
    out->dataStructureId = param->dataStructureId;
    out->wpCheck = param->wpCheck;
    int count = 0;
    switch (model)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:{
            if (param->fast_adc_count_1_1 != 2){
                return false;
            }

            if (param->fast_adc_count_1_20 != 2){
                return false;
            }

            if (param->fast_adc_count_1_1_ac != 0){
                return false;
            }

            if (param->fast_adc_count_1_20_ac != 0){
                return false;
            }

            if (param->fast_dac_count_x1 != 2){
                return false;
            }

            if (param->fast_dac_count_x5 != 0){
                return false;
            }
            SET_PARAMETER(UC_DAC_CH1_GAIN,param->fast_dac_x1[0].calibValue)
            SET_PARAMETER(UC_DAC_CH1_OFFSET,param->fast_dac_x1[0].offset)
            SET_PARAMETER(UC_DAC_CH2_GAIN,param->fast_dac_x1[1].calibValue)
            SET_PARAMETER(UC_DAC_CH2_OFFSET,param->fast_dac_x1[1].offset)

            SET_PARAMETER(UC_ADC_CH1_GAIN_1_1,param->fast_adc_1_1[0].calibValue)
            SET_PARAMETER(UC_ADC_CH1_OFFSET_1_1,param->fast_adc_1_1[0].offset)
            SET_PARAMETER(UC_ADC_CH2_GAIN_1_1,param->fast_adc_1_1[1].calibValue)
            SET_PARAMETER(UC_ADC_CH2_OFFSET_1_1,param->fast_adc_1_1[1].offset)

            SET_PARAMETER(UC_ADC_CH1_GAIN_1_20,param->fast_adc_1_20[0].calibValue)
            SET_PARAMETER(UC_ADC_CH1_OFFSET_1_20,param->fast_adc_1_20[0].offset)
            SET_PARAMETER(UC_ADC_CH2_GAIN_1_20,param->fast_adc_1_20[1].calibValue)
            SET_PARAMETER(UC_ADC_CH2_OFFSET_1_20,param->fast_adc_1_20[1].offset)

            SET_PARAMETER(UC_ADC_CH1_AA_1_1,param->fast_adc_filter_1_1[0].aa)
            SET_PARAMETER(UC_ADC_CH1_BB_1_1,param->fast_adc_filter_1_1[0].bb)
            SET_PARAMETER(UC_ADC_CH1_PP_1_1,param->fast_adc_filter_1_1[0].pp)
            SET_PARAMETER(UC_ADC_CH1_KK_1_1,param->fast_adc_filter_1_1[0].kk)

            SET_PARAMETER(UC_ADC_CH2_AA_1_1,param->fast_adc_filter_1_1[1].aa)
            SET_PARAMETER(UC_ADC_CH2_BB_1_1,param->fast_adc_filter_1_1[1].bb)
            SET_PARAMETER(UC_ADC_CH2_PP_1_1,param->fast_adc_filter_1_1[1].pp)
            SET_PARAMETER(UC_ADC_CH2_KK_1_1,param->fast_adc_filter_1_1[1].kk)

            SET_PARAMETER(UC_ADC_CH1_AA_1_20,param->fast_adc_filter_1_20[0].aa)
            SET_PARAMETER(UC_ADC_CH1_BB_1_20,param->fast_adc_filter_1_20[0].bb)
            SET_PARAMETER(UC_ADC_CH1_PP_1_20,param->fast_adc_filter_1_20[0].pp)
            SET_PARAMETER(UC_ADC_CH1_KK_1_20,param->fast_adc_filter_1_20[0].kk)

            SET_PARAMETER(UC_ADC_CH2_AA_1_20,param->fast_adc_filter_1_20[1].aa)
            SET_PARAMETER(UC_ADC_CH2_BB_1_20,param->fast_adc_filter_1_20[1].bb)
            SET_PARAMETER(UC_ADC_CH2_PP_1_20,param->fast_adc_filter_1_20[1].pp)
            SET_PARAMETER(UC_ADC_CH2_KK_1_20,param->fast_adc_filter_1_20[1].kk)

            break;
        }

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:{
            if (param->fast_adc_count_1_1 != 2){
                return false;
            }

            if (param->fast_adc_count_1_20 != 0){
                return false;
            }

            if (param->fast_adc_count_1_1_ac != 0){
                return false;
            }

            if (param->fast_adc_count_1_20_ac != 0){
                return false;
            }

            if (param->fast_dac_count_x1 != 2){
                return false;
            }

            if (param->fast_dac_count_x5 != 0){
                return false;
            }

            SET_PARAMETER(UC_DAC_CH1_GAIN,param->fast_dac_x1[0].calibValue)
            SET_PARAMETER(UC_DAC_CH1_OFFSET,param->fast_dac_x1[0].offset)
            SET_PARAMETER(UC_DAC_CH2_GAIN,param->fast_dac_x1[1].calibValue)
            SET_PARAMETER(UC_DAC_CH2_OFFSET,param->fast_dac_x1[1].offset)

            SET_PARAMETER(UC_ADC_CH1_GAIN_1_1,param->fast_adc_1_1[0].calibValue)
            SET_PARAMETER(UC_ADC_CH1_OFFSET_1_1,param->fast_adc_1_1[0].offset)
            SET_PARAMETER(UC_ADC_CH2_GAIN_1_1,param->fast_adc_1_1[1].calibValue)
            SET_PARAMETER(UC_ADC_CH2_OFFSET_1_1,param->fast_adc_1_1[1].offset)

            break;
        }

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:{
            if (param->fast_adc_count_1_1 != 4){
                return false;
            }

            if (param->fast_adc_count_1_20 != 4){
                return false;
            }

            if (param->fast_adc_count_1_1_ac != 0){
                return false;
            }

            if (param->fast_adc_count_1_20_ac != 0){
                return false;
            }

            if (param->fast_dac_count_x1 != 0){
                return false;
            }

            if (param->fast_dac_count_x5 != 0){
                return false;
            }

            SET_PARAMETER(UC_ADC_CH1_GAIN_1_1,param->fast_adc_1_1[0].calibValue)
            SET_PARAMETER(UC_ADC_CH1_OFFSET_1_1,param->fast_adc_1_1[0].offset)

            SET_PARAMETER(UC_ADC_CH2_GAIN_1_1,param->fast_adc_1_1[1].calibValue)
            SET_PARAMETER(UC_ADC_CH2_OFFSET_1_1,param->fast_adc_1_1[1].offset)

            SET_PARAMETER(UC_ADC_CH3_GAIN_1_1,param->fast_adc_1_1[2].calibValue)
            SET_PARAMETER(UC_ADC_CH3_OFFSET_1_1,param->fast_adc_1_1[2].offset)

            SET_PARAMETER(UC_ADC_CH4_GAIN_1_1,param->fast_adc_1_1[3].calibValue)
            SET_PARAMETER(UC_ADC_CH4_OFFSET_1_1,param->fast_adc_1_1[3].offset)

            SET_PARAMETER(UC_ADC_CH1_GAIN_1_20,param->fast_adc_1_20[0].calibValue)
            SET_PARAMETER(UC_ADC_CH1_OFFSET_1_20,param->fast_adc_1_20[0].offset)

            SET_PARAMETER(UC_ADC_CH2_GAIN_1_20,param->fast_adc_1_20[1].calibValue)
            SET_PARAMETER(UC_ADC_CH2_OFFSET_1_20,param->fast_adc_1_20[2].offset)

            SET_PARAMETER(UC_ADC_CH3_GAIN_1_20,param->fast_adc_1_20[2].calibValue)
            SET_PARAMETER(UC_ADC_CH3_OFFSET_1_20,param->fast_adc_1_20[2].offset)

            SET_PARAMETER(UC_ADC_CH4_GAIN_1_20,param->fast_adc_1_20[3].calibValue)
            SET_PARAMETER(UC_ADC_CH4_OFFSET_1_20,param->fast_adc_1_20[3].offset)

            SET_PARAMETER(UC_ADC_CH1_AA_1_1,param->fast_adc_filter_1_1[0].aa)
            SET_PARAMETER(UC_ADC_CH1_BB_1_1,param->fast_adc_filter_1_1[0].bb)
            SET_PARAMETER(UC_ADC_CH1_PP_1_1,param->fast_adc_filter_1_1[0].pp)
            SET_PARAMETER(UC_ADC_CH1_KK_1_1,param->fast_adc_filter_1_1[0].kk)

            SET_PARAMETER(UC_ADC_CH2_AA_1_1,param->fast_adc_filter_1_1[1].aa)
            SET_PARAMETER(UC_ADC_CH2_BB_1_1,param->fast_adc_filter_1_1[1].bb)
            SET_PARAMETER(UC_ADC_CH2_PP_1_1,param->fast_adc_filter_1_1[1].pp)
            SET_PARAMETER(UC_ADC_CH2_KK_1_1,param->fast_adc_filter_1_1[1].kk)

            SET_PARAMETER(UC_ADC_CH3_AA_1_1,param->fast_adc_filter_1_1[2].aa)
            SET_PARAMETER(UC_ADC_CH3_BB_1_1,param->fast_adc_filter_1_1[2].bb)
            SET_PARAMETER(UC_ADC_CH3_PP_1_1,param->fast_adc_filter_1_1[2].pp)
            SET_PARAMETER(UC_ADC_CH3_KK_1_1,param->fast_adc_filter_1_1[2].kk)

            SET_PARAMETER(UC_ADC_CH4_AA_1_1,param->fast_adc_filter_1_1[3].aa)
            SET_PARAMETER(UC_ADC_CH4_BB_1_1,param->fast_adc_filter_1_1[3].bb)
            SET_PARAMETER(UC_ADC_CH4_PP_1_1,param->fast_adc_filter_1_1[3].pp)
            SET_PARAMETER(UC_ADC_CH4_KK_1_1,param->fast_adc_filter_1_1[3].kk)

            SET_PARAMETER(UC_ADC_CH1_AA_1_20,param->fast_adc_filter_1_20[0].aa)
            SET_PARAMETER(UC_ADC_CH1_BB_1_20,param->fast_adc_filter_1_20[0].bb)
            SET_PARAMETER(UC_ADC_CH1_PP_1_20,param->fast_adc_filter_1_20[0].pp)
            SET_PARAMETER(UC_ADC_CH1_KK_1_20,param->fast_adc_filter_1_20[0].kk)

            SET_PARAMETER(UC_ADC_CH2_AA_1_20,param->fast_adc_filter_1_20[1].aa)
            SET_PARAMETER(UC_ADC_CH2_BB_1_20,param->fast_adc_filter_1_20[1].bb)
            SET_PARAMETER(UC_ADC_CH2_PP_1_20,param->fast_adc_filter_1_20[1].pp)
            SET_PARAMETER(UC_ADC_CH2_KK_1_20,param->fast_adc_filter_1_20[1].kk)

            SET_PARAMETER(UC_ADC_CH3_AA_1_20,param->fast_adc_filter_1_20[2].aa)
            SET_PARAMETER(UC_ADC_CH3_BB_1_20,param->fast_adc_filter_1_20[2].bb)
            SET_PARAMETER(UC_ADC_CH3_PP_1_20,param->fast_adc_filter_1_20[2].pp)
            SET_PARAMETER(UC_ADC_CH3_KK_1_20,param->fast_adc_filter_1_20[2].kk)

            SET_PARAMETER(UC_ADC_CH4_AA_1_20,param->fast_adc_filter_1_20[3].aa)
            SET_PARAMETER(UC_ADC_CH4_BB_1_20,param->fast_adc_filter_1_20[3].bb)
            SET_PARAMETER(UC_ADC_CH4_PP_1_20,param->fast_adc_filter_1_20[3].pp)
            SET_PARAMETER(UC_ADC_CH4_KK_1_20,param->fast_adc_filter_1_20[3].kk)

            break;
        }

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_120:{
            if (param->fast_adc_count_1_1 != 2){
                return false;
            }

            if (param->fast_adc_count_1_20 != 2){
                return false;
            }

            if (param->fast_adc_count_1_1_ac != 2){
                return false;
            }

            if (param->fast_adc_count_1_20_ac != 2){
                return false;
            }

            if (param->fast_dac_count_x1 != 2){
                return false;
            }

            if (param->fast_dac_count_x5 != 2){
                return false;
            }

            SET_PARAMETER(UC_DAC_CH1_GAIN,param->fast_dac_x1[0].calibValue)
            SET_PARAMETER(UC_DAC_CH1_OFFSET,param->fast_dac_x1[0].offset)
            SET_PARAMETER(UC_DAC_CH2_GAIN,param->fast_dac_x1[1].calibValue)
            SET_PARAMETER(UC_DAC_CH2_OFFSET,param->fast_dac_x1[1].offset)

            SET_PARAMETER(UC_DAC_CH1_GAIN_X5,param->fast_dac_x5[0].calibValue)
            SET_PARAMETER(UC_DAC_CH1_OFFSET_X5,param->fast_dac_x5[0].offset)
            SET_PARAMETER(UC_DAC_CH2_GAIN_X5,param->fast_dac_x5[1].calibValue)
            SET_PARAMETER(UC_DAC_CH2_OFFSET_X5,param->fast_dac_x5[1].offset)

            SET_PARAMETER(UC_ADC_CH1_GAIN_1_1,param->fast_adc_1_1[0].calibValue)
            SET_PARAMETER(UC_ADC_CH1_OFFSET_1_1,param->fast_adc_1_1[0].offset)
            SET_PARAMETER(UC_ADC_CH2_GAIN_1_1,param->fast_adc_1_1[1].calibValue)
            SET_PARAMETER(UC_ADC_CH2_OFFSET_1_1,param->fast_adc_1_1[1].offset)

            SET_PARAMETER(UC_ADC_CH1_GAIN_1_20,param->fast_adc_1_20[0].calibValue)
            SET_PARAMETER(UC_ADC_CH1_OFFSET_1_20,param->fast_adc_1_20[0].offset)
            SET_PARAMETER(UC_ADC_CH2_GAIN_1_20,param->fast_adc_1_20[1].calibValue)
            SET_PARAMETER(UC_ADC_CH2_OFFSET_1_20,param->fast_adc_1_20[1].offset)

            SET_PARAMETER(UC_ADC_CH1_AA_1_1,param->fast_adc_filter_1_1[0].aa)
            SET_PARAMETER(UC_ADC_CH1_BB_1_1,param->fast_adc_filter_1_1[0].bb)
            SET_PARAMETER(UC_ADC_CH1_PP_1_1,param->fast_adc_filter_1_1[0].pp)
            SET_PARAMETER(UC_ADC_CH1_KK_1_1,param->fast_adc_filter_1_1[0].kk)

            SET_PARAMETER(UC_ADC_CH2_AA_1_1,param->fast_adc_filter_1_1[1].aa)
            SET_PARAMETER(UC_ADC_CH2_BB_1_1,param->fast_adc_filter_1_1[1].bb)
            SET_PARAMETER(UC_ADC_CH2_PP_1_1,param->fast_adc_filter_1_1[1].pp)
            SET_PARAMETER(UC_ADC_CH2_KK_1_1,param->fast_adc_filter_1_1[1].kk)

            SET_PARAMETER(UC_ADC_CH1_AA_1_20,param->fast_adc_filter_1_20[0].aa)
            SET_PARAMETER(UC_ADC_CH1_BB_1_20,param->fast_adc_filter_1_20[0].bb)
            SET_PARAMETER(UC_ADC_CH1_PP_1_20,param->fast_adc_filter_1_20[0].pp)
            SET_PARAMETER(UC_ADC_CH1_KK_1_20,param->fast_adc_filter_1_20[0].kk)

            SET_PARAMETER(UC_ADC_CH2_AA_1_20,param->fast_adc_filter_1_20[1].aa)
            SET_PARAMETER(UC_ADC_CH2_BB_1_20,param->fast_adc_filter_1_20[1].bb)
            SET_PARAMETER(UC_ADC_CH2_PP_1_20,param->fast_adc_filter_1_20[1].pp)
            SET_PARAMETER(UC_ADC_CH2_KK_1_20,param->fast_adc_filter_1_20[1].kk)

            SET_PARAMETER(UC_ADC_CH1_GAIN_1_1_AC,param->fast_adc_1_1_ac[0].calibValue)
            SET_PARAMETER(UC_ADC_CH1_OFFSET_1_1_AC,param->fast_adc_1_1_ac[0].offset)
            SET_PARAMETER(UC_ADC_CH2_GAIN_1_1_AC,param->fast_adc_1_1_ac[1].calibValue)
            SET_PARAMETER(UC_ADC_CH2_OFFSET_1_1_AC,param->fast_adc_1_1_ac[1].offset)

            SET_PARAMETER(UC_ADC_CH1_GAIN_1_20_AC,param->fast_adc_1_20_ac[0].calibValue)
            SET_PARAMETER(UC_ADC_CH1_OFFSET_1_20_AC,param->fast_adc_1_20_ac[0].offset)
            SET_PARAMETER(UC_ADC_CH2_GAIN_1_20_AC,param->fast_adc_1_20_ac[1].calibValue)
            SET_PARAMETER(UC_ADC_CH2_OFFSET_1_20_AC,param->fast_adc_1_20_ac[1].offset)

            break;
        }

        default:
            fprintf(stderr,"[Error:calib_WriteParams] Unknown model: %d.\n",model);
            return false;
    }
    out->count = count;
    return true;
}

rp_calib_params_t getDefaultUniversal(rp_HPeModels_t model){
    rp_calib_params_t calib;
    memset(&calib,0,sizeof(rp_calib_params_t));
    switch (model)
    {
    case STEM_125_10_v1_0:
    case STEM_125_14_v1_0:
    case STEM_125_14_v1_1:
    case STEM_125_14_LN_v1_1:
    case STEM_125_14_Z7020_v1_0:
    case STEM_125_14_Z7020_LN_v1_1:
        calib.fast_adc_count_1_1 = 2;
        calib.fast_adc_count_1_20 = 2;
        calib.fast_dac_count_x1 = 2;
        calib.dataStructureId = RP_HW_PACK_ID_V5;

        for(int i = 0; i < 2; ++i){
            calib.fast_adc_1_1[i].calibValue = calibBaseScaleFromVoltage(1.0,true);
            calib.fast_adc_1_1[i].offset = 0;
            calib.fast_adc_1_1[i].baseScale = 1.0;
            calib.fast_adc_1_1[i].gainCalc = 1.0;

            calib.fast_adc_1_20[i].calibValue = calibBaseScaleFromVoltage(1.0,true);
            calib.fast_adc_1_20[i].offset = 0;
            calib.fast_adc_1_20[i].baseScale = 1.0;
            calib.fast_adc_1_20[i].gainCalc = 1.0;

            calib.fast_adc_filter_1_1[i].aa = DEFAULT_1_1_FILT_AA;
            calib.fast_adc_filter_1_1[i].bb = DEFAULT_1_1_FILT_BB;
            calib.fast_adc_filter_1_1[i].kk = DEFAULT_1_1_FILT_KK;
            calib.fast_adc_filter_1_1[i].pp = DEFAULT_1_1_FILT_PP;

            calib.fast_adc_filter_1_20[i].aa = DEFAULT_1_20_FILT_AA;
            calib.fast_adc_filter_1_20[i].bb = DEFAULT_1_20_FILT_BB;
            calib.fast_adc_filter_1_20[i].kk = DEFAULT_1_20_FILT_KK;
            calib.fast_adc_filter_1_20[i].pp = DEFAULT_1_20_FILT_PP;

            calib.fast_dac_x1[i].calibValue = calibBaseScaleFromVoltage(1.0,true);
            calib.fast_dac_x1[i].offset = 0;
            calib.fast_dac_x1[i].baseScale = 1.0;
            calib.fast_dac_x1[i].gainCalc = 1.0;
        }

        break;

    case STEM_122_16SDR_v1_0:
    case STEM_122_16SDR_v1_1:
        calib.fast_adc_count_1_1 = 2;
        calib.fast_dac_count_x1 = 2;
        calib.dataStructureId = RP_HW_PACK_ID_V5;

        for(int i = 0; i < 2; ++i){
            calib.fast_adc_1_1[i].calibValue = calibBaseScaleFromVoltage(1.0,true);
            calib.fast_adc_1_1[i].offset = 0;
            calib.fast_adc_1_1[i].baseScale = 20;
            calib.fast_adc_1_1[i].gainCalc = 1.0;

            calib.fast_dac_x1[i].calibValue = calibBaseScaleFromVoltage(1.0,true);
            calib.fast_dac_x1[i].offset = 0;
            calib.fast_dac_x1[i].baseScale = 1.0;
            calib.fast_dac_x1[i].gainCalc = 1.0;

            calib.fast_adc_filter_1_1[i].aa = DEFAULT_1_1_FILT_AA;
            calib.fast_adc_filter_1_1[i].bb = DEFAULT_1_1_FILT_BB;
            calib.fast_adc_filter_1_1[i].kk = DEFAULT_1_1_FILT_KK;
            calib.fast_adc_filter_1_1[i].pp = DEFAULT_1_1_FILT_PP;
        }
        break;

    case STEM_125_14_Z7020_4IN_v1_0:
    case STEM_125_14_Z7020_4IN_v1_2:
    case STEM_125_14_Z7020_4IN_v1_3:
        calib.fast_adc_count_1_1 = 4;
        calib.fast_adc_count_1_20 = 4;
        calib.dataStructureId = RP_HW_PACK_ID_V5;

        for(int i = 0; i < 4; ++i){
            calib.fast_adc_1_1[i].calibValue = calibBaseScaleFromVoltage(1.0,true);
            calib.fast_adc_1_1[i].offset = 0;
            calib.fast_adc_1_1[i].baseScale = 1.0;
            calib.fast_adc_1_1[i].gainCalc = 1.0;

            calib.fast_adc_1_20[i].calibValue = calibBaseScaleFromVoltage(1.0,true);
            calib.fast_adc_1_20[i].offset = 0;
            calib.fast_adc_1_20[i].baseScale = 1.0;
            calib.fast_adc_1_20[i].gainCalc = 1.0;

            calib.fast_adc_filter_1_1[i].aa = DEFAULT_1_1_FILT_AA;
            calib.fast_adc_filter_1_1[i].bb = DEFAULT_1_1_FILT_BB;
            calib.fast_adc_filter_1_1[i].kk = DEFAULT_1_1_FILT_KK;
            calib.fast_adc_filter_1_1[i].pp = DEFAULT_1_1_FILT_PP;

            calib.fast_adc_filter_1_20[i].aa = DEFAULT_1_20_FILT_AA;
            calib.fast_adc_filter_1_20[i].bb = DEFAULT_1_20_FILT_BB;
            calib.fast_adc_filter_1_20[i].kk = DEFAULT_1_20_FILT_KK;
            calib.fast_adc_filter_1_20[i].pp = DEFAULT_1_20_FILT_PP;
        }
        break;

    case STEM_250_12_v1_0:
    case STEM_250_12_v1_1:
    case STEM_250_12_v1_2:
    case STEM_250_12_120:
        calib.fast_adc_count_1_1 = 2;
        calib.fast_adc_count_1_20 = 2;
        calib.fast_adc_count_1_1_ac = 2;
        calib.fast_adc_count_1_20_ac = 2;
        calib.fast_dac_count_x1 = 2;
        calib.fast_dac_count_x5 = 2;
        calib.dataStructureId = RP_HW_PACK_ID_V5;

        for(int i = 0; i < 2; ++i){
            calib.fast_adc_1_1[i].calibValue = calibBaseScaleFromVoltage(1.0,true);
            calib.fast_adc_1_1[i].offset = 0;
            calib.fast_adc_1_1[i].baseScale = 1.0;
            calib.fast_adc_1_1[i].gainCalc = 1.0;

            calib.fast_adc_1_20[i].calibValue = calibBaseScaleFromVoltage(1.0,true);
            calib.fast_adc_1_20[i].offset = 0;
            calib.fast_adc_1_20[i].baseScale = 1.0;
            calib.fast_adc_1_20[i].gainCalc = 1.0;

            calib.fast_adc_1_1_ac[i].calibValue = calibBaseScaleFromVoltage(1.0,true);
            calib.fast_adc_1_1_ac[i].offset = 0;
            calib.fast_adc_1_1_ac[i].baseScale = 1.0;
            calib.fast_adc_1_1_ac[i].gainCalc = 1.0;

            calib.fast_adc_1_20_ac[i].calibValue = calibBaseScaleFromVoltage(1.0,true);
            calib.fast_adc_1_20_ac[i].offset = 0;
            calib.fast_adc_1_20_ac[i].baseScale = 1.0;
            calib.fast_adc_1_20_ac[i].gainCalc = 1.0;

            calib.fast_dac_x1[i].calibValue = calibBaseScaleFromVoltage(1.0,true);
            calib.fast_dac_x1[i].offset = 0;
            calib.fast_dac_x1[i].baseScale = 1.0;
            calib.fast_dac_x1[i].gainCalc = 1.0;

            calib.fast_dac_x5[i].calibValue = calibBaseScaleFromVoltage(1.0,true);
            calib.fast_dac_x5[i].offset = 0;
            calib.fast_dac_x5[i].baseScale = 1.0;
            calib.fast_dac_x5[i].gainCalc = 1.0;

            calib.fast_adc_filter_1_1[i].aa = DEFAULT_1_1_FILT_AA;
            calib.fast_adc_filter_1_1[i].bb = DEFAULT_1_1_FILT_BB;
            calib.fast_adc_filter_1_1[i].kk = DEFAULT_1_1_FILT_KK;
            calib.fast_adc_filter_1_1[i].pp = DEFAULT_1_1_FILT_PP;

            calib.fast_adc_filter_1_20[i].aa = DEFAULT_1_20_FILT_AA;
            calib.fast_adc_filter_1_20[i].bb = DEFAULT_1_20_FILT_BB;
            calib.fast_adc_filter_1_20[i].kk = DEFAULT_1_20_FILT_KK;
            calib.fast_adc_filter_1_20[i].pp = DEFAULT_1_20_FILT_PP;
        }
        break;

    default:
        break;
    }
    return calib;
}


int recalculateToUniversal(rp_calib_params_t *param){
    for(int i = 0; i < param->fast_adc_count_1_1; i++){
        param->fast_adc_1_1[i].calibValue = calibBaseScaleFromVoltage(1.0,true) * param->fast_adc_1_1[i].gainCalc;
        param->fast_adc_1_1[i].baseScale = 1.0;
    }

    for(int i = 0; i < param->fast_adc_count_1_20; i++){
        param->fast_adc_1_20[i].calibValue = calibBaseScaleFromVoltage(1.0,true) * param->fast_adc_1_20[i].gainCalc;
        param->fast_adc_1_20[i].baseScale = 1.0;
    }

    for(int i = 0; i < param->fast_adc_count_1_1_ac; i++){
        param->fast_adc_1_1_ac[i].calibValue = calibBaseScaleFromVoltage(1.0,true) * param->fast_adc_1_1_ac[i].gainCalc;
        param->fast_adc_1_1_ac[i].baseScale = 1.0;
    }

    for(int i = 0; i < param->fast_adc_count_1_20_ac; i++){
        param->fast_adc_1_20_ac[i].calibValue = calibBaseScaleFromVoltage(1.0,true) * param->fast_adc_1_20_ac[i].gainCalc;
        param->fast_adc_1_20_ac[i].baseScale = 1.0;
    }

    for(int i = 0; i < param->fast_dac_count_x1; i++){
        param->fast_dac_x1[i].calibValue = calibBaseScaleFromVoltage(1.0,true) * param->fast_dac_x1[i].gainCalc;
        param->fast_dac_x1[i].baseScale = 1.0;
    }

    for(int i = 0; i < param->fast_dac_count_x5; i++){
        param->fast_dac_x5[i].calibValue = calibBaseScaleFromVoltage(1.0,true) * param->fast_dac_x5[i].gainCalc;
        param->fast_dac_x5[i].baseScale = 1.0;
    }
    return RP_HW_CALIB_OK;
}
