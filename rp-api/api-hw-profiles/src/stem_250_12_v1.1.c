#include "stem_250_12_v1.1.h"


profiles_t stem_250_12_v1_1 = {
    .boardModel = STEM_250_12_v1_1,
    .boardName = "SIGNALlab 250-12 v1.1",
    .boardModelEEPROM = "",
    .boardETH_MAC =  "",
    .zynqCPUModel = Z7020,
    .oscillator_rate = 250000000,

    .fast_adc_rate = 250000000,
    .fast_adc_count_channels = 2,
    .fast_adc[0].is_signed = true,
    .fast_adc[0].bits = 12,
    .fast_adc[0].fullScale = 1,
    .fast_adc[1].is_signed = true,
    .fast_adc[1].bits = 12,
    .fast_adc[1].fullScale = 1,

    .is_dac_present = true,
    .fast_dac_rate = 250000000,
    .fast_dac_count_channels = 2,
    .fast_dac[0].is_signed = true,
    .fast_dac[0].bits = 14,
    .fast_dac[0].fullScale = 1,
    .fast_dac[1].is_signed = true,
    .fast_dac[1].bits = 14,
    .fast_dac[1].fullScale = 1,

    .is_LV_HV_mode = true,
    .is_AC_DC_mode = true,

    .fast_adc_1_20[0].is_signed = true,
    .fast_adc_1_20[0].bits = 12,
    .fast_adc_1_20[0].fullScale = 20,
    .fast_adc_1_20[1].is_signed = true,
    .fast_adc_1_20[1].bits = 12,
    .fast_adc_1_20[1].fullScale = 20,

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

    .is_DAC_gain_x5 = true,

    .is_fast_calibration = true,

    .is_pll_control_present = true,

    .is_fast_adc_filter_present = true,

    .is_fast_dac_temp_protection = true,

    .is_attenuator_controller_present = true,

    .is_ext_trigger_level_available = true
};

profiles_t* getProfile_STEM_250_12_v1_1(){
    return &stem_250_12_v1_1;
}