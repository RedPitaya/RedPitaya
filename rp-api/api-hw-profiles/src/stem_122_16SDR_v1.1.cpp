#include "stem_122_16SDR_v1.1.h"
#include "common.h"

profiles_t stem_122_16SDR_v1_1 = {.boardModel = STEM_122_16SDR_v1_1,
                                  .boardName = "SDRlab 122-16 v1.1",
                                  .boardModelEEPROM = "",
                                  .boardETH_MAC = "",
                                  .zynqCPUModel = Z7020,
                                  .oscillator_rate = 122880000,

                                  .fast_adc_full_scale = 1,
                                  .fast_adc_rate = 122880000,
                                  .fast_adc_is_sign = true,
                                  .fast_adc_bits = 16,
                                  .fast_adc_count_channels = 2,
                                  .fast_adc_gain = {{0.5, 0.5, 0, 0}, {0, 0, 0, 0}},

                                  .is_dac_present = true,
                                  .fast_dac_full_scale = 1,
                                  .fast_dac_rate = 122880000,
                                  .fast_dac_is_sign = true,
                                  .fast_dac_bits = 14,
                                  .fast_dac_count_channels = 2,
                                  .fast_dac_out_full_scale = {0.5, 0.5, 0, 0},

                                  .is_LV_HV_mode = false,
                                  .is_AC_DC_mode = false,

                                  .slow_adc_count_channels = 4,
                                  .slow_adc = {{false, 11, 3.5f}, {false, 11, 3.5f}, {false, 11, 3.5f}, {false, 11, 3.5f}},

                                  .slow_dac_count_channels = 4,
                                  .slow_dac = {{false, 8, 1.8f}, {false, 8, 1.8f}, {false, 8, 1.8f}, {false, 8, 1.8f}},

                                  .is_DAC_gain_x5 = false,
                                  .is_fast_calibration = false,
                                  .is_pll_control_present = false,
                                  .is_fast_adc_filter_present = false,
                                  .is_fast_dac_temp_protection = false,
                                  .is_attenuator_controller_present = false,
                                  .is_ext_trigger_level_available = false,
                                  .external_trigger_full_scale = 0,
                                  .is_ext_trigger_signed = false,
                                  .fast_adc_spectrum_resolution = 61440000,
                                  .fast_adc_low_pass_filter = 122880000,
                                  .fast_dac_low_pass_filter = 122880000,
                                  .is_daisy_chain_clock_sync = false,
                                  .is_dma_mode_v0_94 = true,
                                  .is_DAC_50_Ohm_mode = false,
                                  .is_split_osc_triggers = true,
                                  .gpio_N_count = 11,
                                  .gpio_P_count = 11,
                                  .ramMB = 512,
                                  .is_E3_high_speed_gpio = false,
                                  .is_E3_mmc_qspi = false,
                                  .E3_high_speed_gpio_rate = 0,
                                  .is_E3_present = false,
                                  .is_calib_in_fpga = false,
                                  .is_fast_adc_16b_mode = false,
                                  .is_xstreaming = false,
                                  .gen_min_speed = 300000,
                                  .gen_max_speed = 122880000 / 2};

profiles_t* getProfile_STEM_122_16SDR_v1_1() {
    profiles_t* profile = []() -> profiles_t* {
        uint32_t orig_adc = stem_122_16SDR_v1_1.fast_adc_rate;
        uint32_t orig_dac = stem_122_16SDR_v1_1.fast_dac_rate;
        uint32_t orig_spec = stem_122_16SDR_v1_1.fast_adc_spectrum_resolution;
        uint32_t orig_adc_fp = stem_122_16SDR_v1_1.fast_adc_low_pass_filter;
        uint32_t orig_dac_fp = stem_122_16SDR_v1_1.fast_dac_low_pass_filter;
        uint32_t orig_gen_min_speed = stem_122_16SDR_v1_1.gen_min_speed;
        uint32_t orig_gen_max_speed = stem_122_16SDR_v1_1.gen_max_speed;

        applyRate(stem_122_16SDR_v1_1.fast_adc_rate, orig_adc, ADC_BASE_RATE_PATH, stem_122_16SDR_v1_1.boardModel);
        applyRate(stem_122_16SDR_v1_1.fast_dac_rate, orig_dac, DAC_BASE_RATE_PATH, stem_122_16SDR_v1_1.boardModel);
        applyRate(stem_122_16SDR_v1_1.fast_adc_spectrum_resolution, orig_spec, SPEC_ADC_PATH, stem_122_16SDR_v1_1.boardModel);
        applyRate(stem_122_16SDR_v1_1.fast_adc_low_pass_filter, orig_adc_fp, ADC_LP_FILTER_PATH, stem_122_16SDR_v1_1.boardModel);
        applyRate(stem_122_16SDR_v1_1.fast_dac_low_pass_filter, orig_dac_fp, DAC_LP_FILTER_PATH, stem_122_16SDR_v1_1.boardModel);
        applyRate(stem_122_16SDR_v1_1.gen_min_speed, orig_gen_min_speed, GEN_MIN_RATE_PATH, stem_122_16SDR_v1_1.boardModel);
        applyRate(stem_122_16SDR_v1_1.gen_max_speed, orig_gen_max_speed, GEN_MAX_RATE_PATH, stem_122_16SDR_v1_1.boardModel);

        return &stem_122_16SDR_v1_1;
    }();

    return profile;
}