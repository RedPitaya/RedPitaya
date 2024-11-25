/**
 * $Id: $
 *
 * @brief Red Pitaya API Calibration Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __RP_HW_CALIB_UNIVERSAL_H
#define __RP_HW_CALIB_UNIVERSAL_H

#include <stdint.h>
#include "calib_structs.h"
#include "rp_hw-calib.h"

typedef enum {
    UC_DAC_CH1_GAIN             = 1,
    UC_DAC_CH1_OFFSET           = 2,

    UC_DAC_CH2_GAIN             = 3,
    UC_DAC_CH2_OFFSET           = 4,

    UC_DAC_CH1_GAIN_X5          = 5,
    UC_DAC_CH1_OFFSET_X5        = 6,

    UC_DAC_CH2_GAIN_X5          = 7,
    UC_DAC_CH2_OFFSET_X5        = 8,

    UC_ADC_CH1_GAIN_1_1         = 9,
    UC_ADC_CH1_OFFSET_1_1       = 10,

    UC_ADC_CH2_GAIN_1_1         = 11,
    UC_ADC_CH2_OFFSET_1_1       = 12,

    UC_ADC_CH3_GAIN_1_1         = 13,
    UC_ADC_CH3_OFFSET_1_1       = 14,

    UC_ADC_CH4_GAIN_1_1         = 15,
    UC_ADC_CH4_OFFSET_1_1       = 16,

    UC_ADC_CH1_GAIN_1_20        = 17,
    UC_ADC_CH1_OFFSET_1_20      = 18,

    UC_ADC_CH2_GAIN_1_20        = 19,
    UC_ADC_CH2_OFFSET_1_20      = 20,

    UC_ADC_CH3_GAIN_1_20        = 21,
    UC_ADC_CH3_OFFSET_1_20      = 22,

    UC_ADC_CH4_GAIN_1_20        = 23,
    UC_ADC_CH4_OFFSET_1_20      = 24,

    UC_ADC_CH1_GAIN_1_1_AC      = 25,
    UC_ADC_CH1_OFFSET_1_1_AC    = 26,

    UC_ADC_CH2_GAIN_1_1_AC      = 27,
    UC_ADC_CH2_OFFSET_1_1_AC    = 28,

    UC_ADC_CH1_GAIN_1_20_AC     = 29,
    UC_ADC_CH1_OFFSET_1_20_AC   = 30,

    UC_ADC_CH2_GAIN_1_20_AC     = 31,
    UC_ADC_CH2_OFFSET_1_20_AC   = 32,

    UC_ADC_CH1_AA_1_1           = 33,
    UC_ADC_CH1_BB_1_1           = 34,
    UC_ADC_CH1_PP_1_1           = 35,
    UC_ADC_CH1_KK_1_1           = 36,

    UC_ADC_CH2_AA_1_1           = 37,
    UC_ADC_CH2_BB_1_1           = 38,
    UC_ADC_CH2_PP_1_1           = 39,
    UC_ADC_CH2_KK_1_1           = 40,

    UC_ADC_CH3_AA_1_1           = 41,
    UC_ADC_CH3_BB_1_1           = 42,
    UC_ADC_CH3_PP_1_1           = 43,
    UC_ADC_CH3_KK_1_1           = 44,

    UC_ADC_CH4_AA_1_1           = 45,
    UC_ADC_CH4_BB_1_1           = 46,
    UC_ADC_CH4_PP_1_1           = 47,
    UC_ADC_CH4_KK_1_1           = 48,

    UC_ADC_CH1_AA_1_20          = 49,
    UC_ADC_CH1_BB_1_20          = 50,
    UC_ADC_CH1_PP_1_20          = 51,
    UC_ADC_CH1_KK_1_20          = 52,

    UC_ADC_CH2_AA_1_20          = 53,
    UC_ADC_CH2_BB_1_20          = 54,
    UC_ADC_CH2_PP_1_20          = 55,
    UC_ADC_CH2_KK_1_20          = 56,

    UC_ADC_CH3_AA_1_20          = 57,
    UC_ADC_CH3_BB_1_20          = 58,
    UC_ADC_CH3_PP_1_20          = 59,
    UC_ADC_CH3_KK_1_20          = 60,

    UC_ADC_CH4_AA_1_20          = 61,
    UC_ADC_CH4_BB_1_20          = 62,
    UC_ADC_CH4_PP_1_20          = 63,
    UC_ADC_CH4_KK_1_20          = 64

} rp_calib_universal_id_t;

char* getNameOfUniversalId(uint16_t id);
bool convertUniversal(rp_HPeModels_t model,rp_calib_params_t *param,rp_calib_params_universal_t *out);
rp_calib_params_t convertUniversaltoCommon(rp_HPeModels_t model, rp_calib_params_universal_t *param);
rp_calib_params_t getDefaultUniversal(rp_HPeModels_t model,bool setFilterZero);
int recalculateToUniversal(rp_calib_params_t *param);

#endif