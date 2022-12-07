/**
 * $Id: $
 *
 * @brief Red Pitaya Hardware Profiles.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef RP_HW_PROFILES_COMMON_H
#define RP_HW_PROFILES_COMMON_H


#include "rp_hw-profiles.h"


typedef struct {
    // Signed value
    bool is_signed;
    // Bit depth
    uint8_t bits;
    // Full scale in Volt
    float   fullScale;
} ADC_DAC_t;

typedef struct {
    rp_HPeModels_t      boardModel;
    char                boardName[255];
    char                boardModelEEPROM[255];
    char                boardETH_MAC[20];
    rp_HPeZynqModels_t  zynqCPUModel;
    uint32_t            oscillator_rate;
    uint32_t            fast_adc_rate;
    uint8_t             fast_adc_count_channels;
    ADC_DAC_t           fast_adc[4];
    bool                is_dac_present;
    uint32_t            fast_dac_rate;
    uint8_t             fast_dac_count_channels;
    ADC_DAC_t           fast_dac[4];
    bool                is_LV_HV_mode; // Mode 1:1 and 1:20
    bool                is_AC_DC_mode; // Support for AC and DC modes
    ADC_DAC_t           fast_adc_1_20[4];

    uint8_t             slow_adc_count_channels;
    ADC_DAC_t           slow_adc[4];
    uint8_t             slow_dac_count_channels;
    ADC_DAC_t           slow_dac[4];

    bool                is_DAC_gain_x5;
    bool                is_fast_calibration;
    bool                is_pll_control_present;
    bool                is_fast_adc_filter_present;
    bool                is_fast_dac_temp_protection;
    bool                is_attenuator_controller_present;
    bool                is_ext_trigger_level_available;
} profiles_t;


int hp_cmn_Init();

profiles_t* hp_cmn_GetLoadedProfile();
int hp_cmn_Print(profiles_t* p);

#endif
