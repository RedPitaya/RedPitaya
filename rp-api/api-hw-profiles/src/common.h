/**
 * $Id: $
 *
 * @brief Red Pitaya Hardware Profiles.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef RP_HW_PROFILES_COMMON_H
#define RP_HW_PROFILES_COMMON_H

#define MAX_CHANNELS 4

#include <string>
#include "rp_hw-profiles.h"

#define ADC_BASE_RATE_PATH "/.config/redpitaya/fast_adc_rate_"
#define DAC_BASE_RATE_PATH "/.config/redpitaya/fast_dac_rate_"
#define SPEC_ADC_PATH "/.config/redpitaya/fast_adc_spectrum_resolution_"
#define ADC_LP_FILTER_PATH "/.config/redpitaya/fast_adc_low_pass_filter_"
#define DAC_LP_FILTER_PATH "/.config/redpitaya/fast_dac_low_pass_filter_"
#define GEN_MIN_RATE_PATH "/.config/redpitaya/gen_min_speed_"
#define GEN_MAX_RATE_PATH "/.config/redpitaya/gen_max_speed_"

typedef struct {
    // Signed value
    bool is_signed;
    // Bit depth
    uint8_t bits;
    // Full scale in Volt
    float fullScale;
} ADC_DAC_t;

typedef struct {
    rp_HPeModels_t boardModel;
    char boardName[255];
    char boardModelEEPROM[255];
    char boardETH_MAC[20];
    rp_HPeZynqModels_t zynqCPUModel;
    uint32_t oscillator_rate;

    float fast_adc_full_scale;
    uint32_t fast_adc_rate;
    bool fast_adc_is_sign;
    uint8_t fast_adc_bits;
    uint8_t fast_adc_count_channels;
    float fast_adc_gain[RP_HP_ADC_GAIN_HIGH + 1][MAX_CHANNELS];

    bool is_dac_present;
    float fast_dac_full_scale;
    uint32_t fast_dac_rate;
    bool fast_dac_is_sign;
    uint8_t fast_dac_bits;
    uint8_t fast_dac_count_channels;
    float fast_dac_out_full_scale[MAX_CHANNELS];  // Without multipliers like x5 as in 250-12

    bool is_LV_HV_mode;  // Mode 1:1 and 1:20
    bool is_AC_DC_mode;  // Support for AC and DC modes

    uint8_t slow_adc_count_channels;
    ADC_DAC_t slow_adc[4];
    uint8_t slow_dac_count_channels;
    ADC_DAC_t slow_dac[4];

    bool is_DAC_gain_x5;
    bool is_fast_calibration;
    bool is_pll_control_present;
    bool is_fast_adc_filter_present;
    bool is_fast_dac_temp_protection;
    bool is_attenuator_controller_present;
    bool is_ext_trigger_level_available;
    uint8_t external_trigger_full_scale;
    bool is_ext_trigger_signed;
    uint32_t fast_adc_spectrum_resolution;
    uint32_t fast_adc_low_pass_filter;
    uint32_t fast_dac_low_pass_filter;
    bool is_daisy_chain_clock_sync;
    bool is_dma_mode_v0_94;
    bool is_DAC_50_Ohm_mode;

    bool is_split_osc_triggers;

    uint8_t gpio_N_count;
    uint8_t gpio_P_count;

    uint16_t ramMB;

    bool is_E3_high_speed_gpio;
    bool is_E3_mmc_qspi;
    uint32_t E3_high_speed_gpio_rate;
    bool is_E3_present;

    bool is_calib_in_fpga;

    bool is_fast_adc_16b_mode;

    bool is_xstreaming;

    uint32_t gen_min_speed;
    uint32_t gen_max_speed;

} profiles_t;

int hp_cmn_Init();
profiles_t* hp_cmn_GetLoadedProfile();
bool hp_cmn_isEppromValid();
profiles_t* hp_cmn_getProfile(rp_HPeModels_t model);
int hp_cmn_Print(profiles_t* p);

void hp_cmn_PrintKeyHelp();
void hp_cmn_PrintPivotTable(char* keys);
int hp_cmn_GetFPGAVersion(rp_HPeModels_t model, const char** _no_free_value);
int hp_cmn_GetDTSVersion(rp_HPeModels_t model, const char** _no_free_value);

void applyRate(uint32_t& target, uint32_t original, const std::string& path, rp_HPeModels_t boardModel);
int hp_cmn_GetFromConfig(rp_HPeModels_t model, const std::string& path, bool& noerror);
int hp_cmn_WriteConfig(rp_HPeModels_t model, const char* key, int value);
int hp_cmn_DeleteConfig(rp_HPeModels_t model, const char* key);

#endif
