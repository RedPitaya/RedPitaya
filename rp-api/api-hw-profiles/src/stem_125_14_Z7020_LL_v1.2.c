#include "stem_125_14_Z7020_LL_v1.2.h"


profiles_t stem_125_14_Z7020_LL_v1_2 = {
    .boardModel = STEM_125_14_Z7020_LL_v1_2,
    .boardName = "STEMlab 125-14 LL v1.2",
    .boardModelEEPROM = "",
    .boardETH_MAC =  "",
    .zynqCPUModel = Z7020,
    .oscillator_rate = 125000000,

    .fast_adc_rate = 125000000,
    .fast_adc_count_channels = 2,
    .fast_adc_is_sign = true,
    .fast_adc_bits = 16,

    .fast_adc_gain[RP_HP_ADC_GAIN_NORMAL][0] = 1,
    .fast_adc_gain[RP_HP_ADC_GAIN_NORMAL][1] = 1,
    .fast_adc_gain[RP_HP_ADC_GAIN_NORMAL][2] = 0,
    .fast_adc_gain[RP_HP_ADC_GAIN_NORMAL][3] = 0,

    .fast_adc_gain[RP_HP_ADC_GAIN_HIGH][0] = 20,
    .fast_adc_gain[RP_HP_ADC_GAIN_HIGH][1] = 20,
    .fast_adc_gain[RP_HP_ADC_GAIN_HIGH][2] = 0,
    .fast_adc_gain[RP_HP_ADC_GAIN_HIGH][3] = 0,

    .is_dac_present = true,
    .fast_dac_rate = 125000000,
    .fast_dac_count_channels = 2,
    .fast_dac_is_sign = true,
    .fast_dac_bits = 14,
    .fast_dac_out_full_scale[0] = 2,
    .fast_dac_out_full_scale[1] = 2,
    .fast_dac_out_full_scale[2] = 0,
    .fast_dac_out_full_scale[3] = 0,

    .is_LV_HV_mode = true,
    .is_AC_DC_mode = false,

    .slow_adc_count_channels = 4,
    .slow_adc[0].is_signed = false,
    .slow_adc[0].bits = 11, // Real 12 bits, but we need to exclude the sign
    .slow_adc[0].fullScale = 3.5f,

    .slow_adc[1].is_signed = false,
    .slow_adc[1].bits = 11, // Real 12 bits, but we need to exclude the sign
    .slow_adc[1].fullScale = 3.5f,

    .slow_adc[2].is_signed = false,
    .slow_adc[2].bits = 11, // Real 12 bits, but we need to exclude the sign
    .slow_adc[2].fullScale = 3.5f,

    .slow_adc[3].is_signed = false,
    .slow_adc[3].bits = 11,
    .slow_adc[3].fullScale = 3.5f,

    .slow_dac_count_channels = 4,
    .slow_dac[0].is_signed = false,
    .slow_dac[0].bits = 8,
    .slow_dac[0].fullScale = 1.8f,

    .slow_dac[1].is_signed = false,
    .slow_dac[1].bits = 8,
    .slow_dac[1].fullScale = 1.8f,

    .slow_dac[2].is_signed = false,
    .slow_dac[2].bits = 8,
    .slow_dac[2].fullScale = 1.8f,

    .slow_dac[3].is_signed = false,
    .slow_dac[3].bits = 8,
    .slow_dac[3].fullScale = 1.8f,

    .is_DAC_gain_x5 = false,

    .is_fast_calibration = true,

    .is_pll_control_present = false,

    .is_fast_adc_filter_present = true,

    .is_fast_dac_temp_protection = false,

    .is_attenuator_controller_present = false,

    .is_ext_trigger_level_available = false,
    .external_trigger_full_scale = 0,
    .is_ext_trigger_signed = false,

    .fast_adc_spectrum_resolution = 62500000,

    .fast_adc_full_scale = 1,
    .fast_dac_full_scale = 2,

    .is_daisy_chain_clock_sync = true,

    .is_dma_mode_v0_94 = true,
    .is_DAC_50_Ohm_mode = true,
    .is_split_osc_triggers = true,

    .gpio_N_count = 8,
    .gpio_P_count = 8,

    .ramMB = 512,

    .is_E3_high_speed_gpio = false,
    .is_E3_mmc_qspi = false,
    .E3_high_speed_gpio_rate = 0,
    .is_E3_present = false,

    .is_calib_in_fpga = false

};

profiles_t* getProfile_STEM_125_14_Z7020_LL_v1_2(){
    return &stem_125_14_Z7020_LL_v1_2;
}