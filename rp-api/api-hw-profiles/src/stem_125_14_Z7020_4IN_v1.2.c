#include "stem_125_14_Z7020_4IN_v1.2.h"


profiles_t stem_125_14_Z7020_4IN_v1_2 = {
    .boardModel = STEM_125_14_Z7020_4IN_v1_2,
    .boardName = "STEMlab 125-14 4-Inputs v1.2",
    .boardModelEEPROM = "",
    .boardETH_MAC =  "",
    .zynqCPUModel = Z7020,
    .oscillator_rate = 125000000,

    .fast_adc_rate = 125000000,
    .fast_adc_count_channels = 4,
    .fast_adc[0].is_signed = true,
    .fast_adc[0].bits = 14,
    .fast_adc[0].fullScale = 2,
    .fast_adc[1].is_signed = true,
    .fast_adc[1].bits = 14,
    .fast_adc[1].fullScale = 2,
    .fast_adc[2].is_signed = true,
    .fast_adc[2].bits = 14,
    .fast_adc[2].fullScale = 2,
    .fast_adc[3].is_signed = true,
    .fast_adc[3].bits = 14,
    .fast_adc[3].fullScale = 2,

    .is_dac_present = false,
    .fast_dac_rate = 0,
    .fast_dac_count_channels = 0,

    .is_LV_HV_mode = true,
    .is_AC_DC_mode = false,

    .fast_adc_1_20[0].is_signed = true,
    .fast_adc_1_20[0].bits = 14,
    .fast_adc_1_20[0].fullScale = 20,
    .fast_adc_1_20[1].is_signed = true,
    .fast_adc_1_20[1].bits = 14,
    .fast_adc_1_20[1].fullScale = 20,
    .fast_adc_1_20[2].is_signed = true,
    .fast_adc_1_20[2].bits = 14,
    .fast_adc_1_20[2].fullScale = 20,
    .fast_adc_1_20[3].is_signed = true,
    .fast_adc_1_20[3].bits = 14,
    .fast_adc_1_20[3].fullScale = 20,

    .slow_adc_count_channels = 4,
    .slow_adc[0].is_signed = false,
    .slow_adc[0].bits = 11, // Real 12 bits, but we need to exclude the sign
    .slow_adc[0].fullScale = 3.5,

    .slow_adc[1].is_signed = false,
    .slow_adc[1].bits = 11, // Real 12 bits, but we need to exclude the sign
    .slow_adc[1].fullScale = 3.5,

    .slow_adc[2].is_signed = false,
    .slow_adc[2].bits = 11, // Real 12 bits, but we need to exclude the sign
    .slow_adc[2].fullScale = 3.5,

    .slow_adc[3].is_signed = false,
    .slow_adc[3].bits = 11,
    .slow_adc[3].fullScale = 3.5,

    .slow_dac_count_channels = 4,
    .slow_dac[0].is_signed = false,
    .slow_dac[0].bits = 8,
    .slow_dac[0].fullScale = 1.8,

    .slow_dac[1].is_signed = false,
    .slow_dac[1].bits = 8,
    .slow_dac[1].fullScale = 1.8,

    .slow_dac[2].is_signed = false,
    .slow_dac[2].bits = 8,
    .slow_dac[2].fullScale = 1.8,

    .slow_dac[3].is_signed = false,
    .slow_dac[3].bits = 8,
    .slow_dac[3].fullScale = 1.8,

    .is_DAC_gain_x5 = false,

    .is_fast_calibration = true,

    .is_pll_control_present = false,

    .is_fast_adc_filter_present = true,

    .is_fast_dac_temp_protection = false,

    .is_attenuator_controller_present = false,

    .is_ext_trigger_level_available = false,
    .external_trigger_full_scale = 0,

    .fast_adc_spectrum_resolution = 62500000,

    .fast_adc_full_scale = 1,
    .fast_dac_full_scale = 1
};

profiles_t* getProfile_STEM_125_14_Z7020_4IN_v1_2(){
    return &stem_125_14_Z7020_4IN_v1_2;
}