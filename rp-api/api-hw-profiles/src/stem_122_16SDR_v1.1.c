#include "stem_122_16SDR_v1.1.h"


profiles_t stem_122_16SDR_v1_1 = {
    .boardModel = STEM_122_16SDR_v1_1,
    .boardName = "SDRlab 122-16 v1.1",
    .boardModelEEPROM = "",
    .boardETH_MAC =  "",
    .zynqCPUModel = Z7020,
    .oscillator_rate = 122880000,

    .fast_adc_rate = 122880000,
    .fast_adc_count_channels = 2,
    .fast_adc_is_sign = true,
    .fast_adc_bits = 16,

    .fast_adc_gain[RP_HP_ADC_GAIN_NORMAL][0] = 0.5,
    .fast_adc_gain[RP_HP_ADC_GAIN_NORMAL][1] = 0.5,
    .fast_adc_gain[RP_HP_ADC_GAIN_NORMAL][2] = 0,
    .fast_adc_gain[RP_HP_ADC_GAIN_NORMAL][3] = 0,

    .fast_adc_gain[RP_HP_ADC_GAIN_HIGH][0] = 0,
    .fast_adc_gain[RP_HP_ADC_GAIN_HIGH][1] = 0,
    .fast_adc_gain[RP_HP_ADC_GAIN_HIGH][2] = 0,
    .fast_adc_gain[RP_HP_ADC_GAIN_HIGH][3] = 0,

    .is_dac_present = true,
    .fast_dac_rate = 122880000,
    .fast_dac_count_channels = 2,
    .fast_dac_is_sign = true,
    .fast_dac_bits = 14,
    .fast_dac_gain[0] = 0.5,
    .fast_dac_gain[1] = 0.5,
    .fast_dac_gain[2] = 0,
    .fast_dac_gain[3] = 0,

    .is_LV_HV_mode = false,
    .is_AC_DC_mode = false,

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

    .is_fast_calibration = false,

    .is_pll_control_present = false,

    .is_fast_adc_filter_present = false,

    .is_fast_dac_temp_protection = false,

    .is_attenuator_controller_present = false,

    .is_ext_trigger_level_available = false,
    .external_trigger_full_scale = 0,

    .fast_adc_spectrum_resolution = 61440000,

    .fast_adc_full_scale = 1,
    .fast_dac_full_scale = 1,

    .is_daisy_chain_clock_sync = false,

    .is_dma_mode_v0_94 = true
};

profiles_t* getProfile_STEM_122_16SDR_v1_1(){
    return &stem_122_16SDR_v1_1;
}