#include "stem_125_14_Z7020_Pro_v1.0.h"
#include "common.h"

profiles_t stem_125_14_Z7020_Pro_v1_0 = {.boardModel = STEM_125_14_Z7020_Pro_v1_0,
                                         .boardName = "STEMlab 125-14-Z7020 Pro v1.0",
                                         .boardModelEEPROM = "",
                                         .boardETH_MAC = "",
                                         .zynqCPUModel = Z7020,
                                         .oscillator_rate = 125000000,

                                         .fast_adc_full_scale = 1,
                                         .fast_adc_rate = 125000000,
                                         .fast_adc_is_sign = true,
                                         .fast_adc_bits = 14,
                                         .fast_adc_count_channels = 2,
                                         .fast_adc_gain = {{1, 1, 0, 0}, {20, 20, 0, 0}},

                                         .is_dac_present = true,
                                         .fast_dac_full_scale = 2,
                                         .fast_dac_rate = 125000000,
                                         .fast_dac_is_sign = true,
                                         .fast_dac_bits = 14,
                                         .fast_dac_count_channels = 2,
                                         .fast_dac_out_full_scale = {2, 2, 0, 0},

                                         .is_LV_HV_mode = true,
                                         .is_AC_DC_mode = false,

                                         .slow_adc_count_channels = 4,
                                         .slow_adc = {{false, 11, 3.5f}, {false, 11, 3.5f}, {false, 11, 3.5f}, {false, 11, 3.5f}},

                                         .slow_dac_count_channels = 4,
                                         .slow_dac = {{false, 8, 1.8f}, {false, 8, 1.8f}, {false, 8, 1.8f}, {false, 8, 1.8f}},

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
                                         .fast_adc_low_pass_filter = 50000000,
                                         .fast_dac_low_pass_filter = 50000000,
                                         .is_daisy_chain_clock_sync = true,
                                         .is_dma_mode_v0_94 = true,
                                         .is_DAC_50_Ohm_mode = true,
                                         .is_split_osc_triggers = true,
                                         .gpio_N_count = 11,
                                         .gpio_P_count = 11,
                                         .ramMB = 1024,
                                         .is_E3_high_speed_gpio = true,
                                         .is_E3_mmc_qspi = true,
                                         .E3_high_speed_gpio_rate = 1000000000,
                                         .is_E3_present = true,
                                         .is_calib_in_fpga = true,
                                         .is_fast_adc_16b_mode = true,
                                         .is_xstreaming = false,
                                         .gen_min_speed = 1,
                                         .gen_max_speed = 50000000};

profiles_t* getProfile_STEM_125_14_Z7020_Pro_v1_0() {
    profiles_t* profile = []() -> profiles_t* {
        uint32_t orig_adc = stem_125_14_Z7020_Pro_v1_0.fast_adc_rate;
        uint32_t orig_dac = stem_125_14_Z7020_Pro_v1_0.fast_dac_rate;
        uint32_t orig_spec = stem_125_14_Z7020_Pro_v1_0.fast_adc_spectrum_resolution;
        uint32_t orig_adc_fp = stem_125_14_Z7020_Pro_v1_0.fast_adc_low_pass_filter;
        uint32_t orig_dac_fp = stem_125_14_Z7020_Pro_v1_0.fast_dac_low_pass_filter;
        uint32_t orig_gen_min_speed = stem_125_14_Z7020_Pro_v1_0.gen_min_speed;
        uint32_t orig_gen_max_speed = stem_125_14_Z7020_Pro_v1_0.gen_max_speed;

        applyRate(stem_125_14_Z7020_Pro_v1_0.fast_adc_rate, orig_adc, ADC_BASE_RATE_PATH, stem_125_14_Z7020_Pro_v1_0.boardModel);
        applyRate(stem_125_14_Z7020_Pro_v1_0.fast_dac_rate, orig_dac, DAC_BASE_RATE_PATH, stem_125_14_Z7020_Pro_v1_0.boardModel);
        applyRate(stem_125_14_Z7020_Pro_v1_0.fast_adc_spectrum_resolution, orig_spec, SPEC_ADC_PATH, stem_125_14_Z7020_Pro_v1_0.boardModel);
        applyRate(stem_125_14_Z7020_Pro_v1_0.fast_adc_low_pass_filter, orig_adc_fp, ADC_LP_FILTER_PATH, stem_125_14_Z7020_Pro_v1_0.boardModel);
        applyRate(stem_125_14_Z7020_Pro_v1_0.fast_dac_low_pass_filter, orig_dac_fp, DAC_LP_FILTER_PATH, stem_125_14_Z7020_Pro_v1_0.boardModel);
        applyRate(stem_125_14_Z7020_Pro_v1_0.gen_min_speed, orig_gen_min_speed, GEN_MIN_RATE_PATH, stem_125_14_Z7020_Pro_v1_0.boardModel);
        applyRate(stem_125_14_Z7020_Pro_v1_0.gen_max_speed, orig_gen_max_speed, GEN_MAX_RATE_PATH, stem_125_14_Z7020_Pro_v1_0.boardModel);

        return &stem_125_14_Z7020_Pro_v1_0;
    }();

    return profile;
}