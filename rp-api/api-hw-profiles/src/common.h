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

#define MAX_CHANNELS 4

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

    float               fast_adc_full_scale;
    uint32_t            fast_adc_rate;
    bool                fast_adc_is_sign;
    uint8_t             fast_adc_bits;
    uint8_t             fast_adc_count_channels;
    float               fast_adc_gain[RP_HP_ADC_GAIN_HIGH + 1][MAX_CHANNELS];


    bool                is_dac_present;
    float               fast_dac_full_scale;
    uint32_t            fast_dac_rate;
    bool                fast_dac_is_sign;
    uint8_t             fast_dac_bits;
    uint8_t             fast_dac_count_channels;
    float               fast_dac_gain[MAX_CHANNELS];

    bool                is_LV_HV_mode; // Mode 1:1 and 1:20
    bool                is_AC_DC_mode; // Support for AC and DC modes

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
    uint8_t             external_trigger_full_scale;
    bool                is_ext_trigger_signed;
    uint32_t            fast_adc_spectrum_resolution;
    bool                is_daisy_chain_clock_sync;
    bool                is_dma_mode_v0_94;
    bool                is_DAC_50_Ohm_mode;

    bool                is_split_osc_triggers;

    uint8_t             gpio_N_count;
    uint8_t             gpio_P_count;

    uint16_t            ramMB;

    bool                is_E3_high_speed_gpio;
    bool                is_E3_mcc_qspi;
    uint32_t            E3_high_speed_gpio_rate;

} profiles_t;


int hp_cmn_Init();

profiles_t* hp_cmn_GetLoadedProfile();
int hp_cmn_Print(profiles_t* p);

#endif
