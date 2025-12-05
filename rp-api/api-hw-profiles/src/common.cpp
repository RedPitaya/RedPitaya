/**
 * $Id: $
 *
 * @brief Red Pitaya Hardware Profiles
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include "common.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <mtd/mtd-user.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "stem_122_16SDR_v1.0.h"
#include "stem_122_16SDR_v1.1.h"
#include "stem_125_10_v1.0.h"
#include "stem_125_14_LN_BO_v1.1.h"
#include "stem_125_14_LN_CE1_v1.1.h"
#include "stem_125_14_LN_CE2_v1.1.h"
#include "stem_125_14_LN_v1.1.h"
#include "stem_125_14_Pro_v2.0.h"
#include "stem_125_14_Z7020_4IN_BO_v1.3.h"
#include "stem_125_14_Z7020_4IN_v1.0.h"
#include "stem_125_14_Z7020_4IN_v1.2.h"
#include "stem_125_14_Z7020_4IN_v1.3.h"
#include "stem_125_14_Z7020_Ind_v2.0.h"
#include "stem_125_14_Z7020_LN_v1.1.h"
#include "stem_125_14_Z7020_Pro_v1.0.h"
#include "stem_125_14_Z7020_Pro_v2.0.h"
#include "stem_125_14_Z7020_v1.0.h"
#include "stem_125_14_v1.0.h"
#include "stem_125_14_v1.1.h"
#include "stem_125_14_v2.0.h"
#include "stem_250_12_120.h"
#include "stem_250_12_v1.0.h"
#include "stem_250_12_v1.1.h"
#include "stem_250_12_v1.2.h"
#include "stem_250_12_v1.2a.h"
#include "stem_250_12_v1.2b.h"
#include "stem_special.h"

#include "stem_125_14_Z7020_LL_v1.1.h"
#include "stem_125_14_Z7020_LL_v1.2.h"
#include "stem_125_14_Z7020_TI_v1.3.h"
#include "stem_65_16_Z7020_LL_v1.1.h"
#include "stem_65_16_Z7020_TI_v1.3.h"

#define LINE_LENGTH 0x400

const char* table_keys_help[] = {"all",
                                 "All parameters",
                                 "model",
                                 "ID of board",
                                 "fpga_path",
                                 "Path to FPGA bitstream",
                                 "zynq",
                                 "Zynq CPU type 0 - 7010,1 - 7020",
                                 "osc_rate",
                                 "Oscillator Rate (Hz)",
                                 "f_adc_fs",
                                 "Fast ADC full scale (V)",
                                 "f_adc_rate",
                                 "Fast ADC rate (Hz)",
                                 "f_adc_is_sign",
                                 "Signed value for Fast ADC",
                                 "f_adc_bits",
                                 "Number of bits in Fast ADC",
                                 "f_adc_count",
                                 "Number of channels in Fast ADC",
                                 "f_adc_gain",
                                 "Gain in Fast ADC",
                                 "f_is_dac",
                                 "Fast DAC present",
                                 "f_dac_fs",
                                 "Fast DAC full scale (V)",
                                 "f_dac_rate",
                                 "Fast DAC rate (Hz)",
                                 "f_dac_is_sign",
                                 "Signed value for Fast DAC",
                                 "f_dac_bits",
                                 "Number of bits in Fast DAC",
                                 "f_dac_count",
                                 "Number of channels in Fast DAC",
                                 "f_dac_gain",
                                 "Gain in Fast DAC",
                                 "is_hv_lv",
                                 "There is a 1:1 and 1:20 divider on the board",
                                 "is_ac_dc",
                                 "AD/DC switches are present",
                                 "s_adc_count",
                                 "Number of slow ADC channels",
                                 "s_adc_fs",
                                 "Slow ADC full scale (V)",
                                 "s_adc_bits",
                                 "Number of bits in Slow ADC",
                                 "s_adc_is_sign",
                                 "Signed value for Slow ADC",
                                 "s_dac_count",
                                 "Number of slow DAC channels",
                                 "s_dac_fs",
                                 "Slow DAC full scale (V)",
                                 "s_dac_bits",
                                 "Number of bits in Slow DAC",
                                 "s_dac_is_sign",
                                 "Signed value for Slow DAC",

                                 "is_dac_x5",
                                 "There is a x5 amplifier on the DAC",
                                 "is_f_calib",
                                 "Fast DAC/ADC calibration capability is available",
                                 "is_pll_control",
                                 "PLL control is present",
                                 "is_f_adc_filter",
                                 "Filter for Fast ADC is available",
                                 "is_f_dac_t_prot",
                                 "Overheat protection for Fast DAC is available",
                                 "is_att_controller",
                                 "Divider controller available",

                                 "is_ext_trig_lev",
                                 "External trigger level setting is present",
                                 "is_ext_trig_fs",
                                 "Full scale for external trigger",
                                 "is_ext_trig_is_sign",
                                 "Signed value at the external trigger level",

                                 "spec_max_rate",
                                 "Maximum frequency value for spectrum analyzer",

                                 "is_daisy_clock_sync",
                                 "Synchronization via daisy chain",

                                 "is_dma_094",
                                 "DMA mode is available in FPGA 0.94",

                                 "is_dac_50ohm",
                                 "Support 50 ohm load mode for DAC",

                                 "is_split_trig",
                                 "Support split trigger mode",

                                 "gpio_count",
                                 "Number of GPIO outputs",
                                 "ram",
                                 "Maximum amount of RAM",

                                 "is_e3",
                                 "High-speed E3 connector is present",
                                 "is_e3_hs_gpio",
                                 "High-speed E3 connector for GPIO is present",
                                 "is_e3_hs_rate",
                                 "Rate in E3 HS gpio",
                                 "is_e3_qspi",
                                 "QSPI is present in E3",
                                 "is_fpga_calib",
                                 "Fast ADC Calibration on FPGA",
                                 NULL};

#define ADC_BASE_RATE_PATH hp_cmn_GetHomeDirectory() + "/.config/redpitaya/adc_base_rate_"
#define DAC_BASE_RATE_PATH hp_cmn_GetHomeDirectory() + "/.config/redpitaya/dac_base_rate_"

profiles_t* g_profile = NULL;

// "STEM_125-10_v1.0"
// "STEM_14_B_v1.0"
// "STEM_125-14_v1.0"
// "STEM_125-14_v1.1"
// "STEM_125-14_LN_v1.1"
// "STEM_125-14_Z7020_v1.0"
// "STEM_125-14_Z7020_LN_v1.1"
// "STEM_122-16SDR_v1.0"
// "STEM_122-16SDR_v1.1"
// "STEM_125-14_Z7020_4IN_v1.0"
// "STEM_125-14_Z7020_4IN_v1.2"
// "STEM_125-14_Z7020_4IN_v1.3"
// "STEM_125-14_Z7020_4IN_BO_v1.3"
// "STEM_250-12_v1.1"
// "STEM_250-12_v1.2"
// "STEM_250-12_v1.2a"
// "STEM_250-12_v1.2b"
// "STEM_250-12_v1.0"
// "STEM_250-12_120"
// "STEM_125-14_LN_BO_v1.1"
// "STEM_125-14_LN_CE1_v1.1"
// "STEM_125-14_LN_CE2_v1.1"

// Gen2

// "STEM_125-14_v2.0"
// "STEM_125-14_Pro_v2.0"
// "STEM_125-14_Z7020_Pro_v1.0"
// "STEM_125-14_Z7020_Pro_v2.0"
// "STEM_125-14_Ind_v2.0"

// Low latency
// "STEM_125-14_Z7020_LL_v1.1"
// "STEM_65-16_LL_v1.1"
// "STEM_125-14_Z7020_LL_v1.2"
// "STEM_125-14_TI_v1.3"
// "STEM_65-16_TI_v1.3"

std::string hp_cmn_GetHomeDirectory() {
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        return std::string(pw->pw_dir);
    }
    return "";
}

void convertToLowerCase(char* buff) {
    int size = strlen(buff);
    while (size > 0) {
        size--;
        buff[size] = tolower(buff[size]);
    }
}

void hp_checkModel(char* model, char* eth_mac) {
    char modelOrig[255];
    strcpy(modelOrig, model);
    convertToLowerCase(model);
    if (!model)
        return;

    if (strcmp(model, "stem_125-10_v1.0") == 0) {
        g_profile = getProfile_STEM_125_10_v1_0();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_v1.0") == 0) {
        g_profile = getProfile_STEM_125_14_v1_0();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_14_b_v1.0") == 0) {
        g_profile = getProfile_STEM_125_14_v1_0();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_v1.1") == 0) {
        g_profile = getProfile_STEM_125_14_v1_1();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_122-16sdr_v1.0") == 0) {
        g_profile = getProfile_STEM_122_16SDR_v1_0();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_122-16sdr_v1.1") == 0) {
        g_profile = getProfile_STEM_122_16SDR_v1_1();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_ln_v1.1") == 0) {
        g_profile = getProfile_STEM_125_14_LN_v1_1();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_z7020_v1.0") == 0) {
        g_profile = getProfile_STEM_125_14_Z7020_v1_0();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_z7020_ln_v1.1") == 0) {
        g_profile = getProfile_STEM_125_14_Z7020_LN_v1_1();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_z7020_4in_v1.0") == 0) {
        g_profile = getProfile_STEM_125_14_Z7020_4IN_v1_0();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_z7020_4in_v1.2") == 0) {
        g_profile = getProfile_STEM_125_14_Z7020_4IN_v1_2();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_z7020_4in_v1.3") == 0) {
        g_profile = getProfile_STEM_125_14_Z7020_4IN_v1_3();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_z7020_4in_bo_v1.3") == 0) {
        g_profile = getProfile_STEM_125_14_Z7020_4IN_BO_v1_3();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_250-12_v1.0") == 0) {
        g_profile = getProfile_STEM_250_12_v1_0();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_250-12_v1.1") == 0) {
        g_profile = getProfile_STEM_250_12_v1_1();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_250-12_v1.2") == 0) {
        g_profile = getProfile_STEM_250_12_v1_2();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_250-12_v1.2a") == 0) {
        g_profile = getProfile_STEM_250_12_v1_2a();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_250-12_v1.2b") == 0) {
        g_profile = getProfile_STEM_250_12_v1_2b();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_250-12_120") == 0) {
        g_profile = getProfile_STEM_250_12_120();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_ln_bo_v1.1") == 0) {
        g_profile = getProfile_STEM_125_14_LN_BO_v1_1();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_ln_ce1_v1.1") == 0) {
        g_profile = getProfile_STEM_125_14_LN_CE1_v1_1();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_ln_ce2_v1.1") == 0) {
        g_profile = getProfile_STEM_125_14_LN_CE2_v1_1();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_v2.0") == 0) {  // STEM_125-14_v2.0
        g_profile = getProfile_STEM_125_14_v2_0();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_pro_v2.0") == 0) {  // STEM_125-14_Pro_v2.0
        g_profile = getProfile_STEM_125_14_Pro_v2_0();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_z7020_pro_v1.0") == 0) {  // STEM_125-14_Z7020_Pro_v1.0
        g_profile = getProfile_STEM_125_14_Z7020_Pro_v1_0();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_z7020_pro_v2.0") == 0) {  // STEM_125-14_Z7020_Pro_v2.0
        g_profile = getProfile_STEM_125_14_Z7020_Pro_v2_0();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_ind_v2.0") == 0) {  // STEM_125-14_Ind_v2.0
        g_profile = getProfile_STEM_125_14_Z7020_Ind_v2_0();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_z7020_ll_v1.1") == 0) {  // STEM_125-14_Z7020_LL_v1.1
        g_profile = getProfile_STEM_125_14_Z7020_LL_v1_1();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_z7020_ll_v1.2") == 0) {  // STEM_125-14_Z7020_LL_v1.2
        g_profile = getProfile_STEM_125_14_Z7020_LL_v1_2();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_65-16_ll_v1.1") == 0) {  // STEM_65-16_LL_v1.1
        g_profile = getProfile_STEM_65_16_Z7020_LL_v1_1();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_65-16_ti_v1.3") == 0) {  // STEM_65-16_TI_v1.3
        g_profile = getProfile_STEM_65_16_Z7020_TI_v1_3();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    if (strcmp(model, "stem_125-14_ti_v1.3") == 0) {  // STEM_125-14_TI_v1.3
        g_profile = getProfile_STEM_125_14_Z7020_TI_v1_3();
        strcpy(g_profile->boardModelEEPROM, modelOrig);
        if (eth_mac)
            strcpy(g_profile->boardETH_MAC, eth_mac);
        return;
    }

    fprintf(stderr, "[Fatal error] Unknown model \"%s\"", model);
}

int hp_cmn_Init() {
    char* buf;
    char *name, *value;
    char* model = NULL;
    char* eth_mac = NULL;

    FILE* fp = fopen("/sys/bus/i2c/devices/0-0050/eeprom", "r");
    if (!fp) {
        fprintf(stderr, "[hp_cmn_Init] Error open eeprom: %s\n", strerror(errno));
        return RP_HP_ERE;
    }

    if (fseek(fp, 0x1804, SEEK_SET) < 0) {
        fclose(fp);
        fprintf(stderr, "[hp_cmn_Init] Error open eeprom\n");
        return RP_HP_ERE;
    }

    buf = (char*)malloc(LINE_LENGTH);
    if (!buf) {
        fclose(fp);
        return RP_HP_EAL;
    }

    int size = fread(buf, sizeof(char), LINE_LENGTH, fp);
    int position = 0;
    while (position < size) {
        int slen = strlen(&buf[position]);
        if (!slen)
            break;
        name = &buf[position];
        value = strchr(name, '=');
        if (!value) {
            position += slen + 1;
            continue;
        }
        *value++ = '\0';
        if (!strlen(value))
            value = NULL;

        if (!strcmp(name, "hw_rev") && value != NULL) {
            if (strlen(value) + 1 < 255) {
                model = (char*)malloc(strlen(value) + 1);
                if (model)
                    strcpy(model, value);
            }
        }

        if (!strcmp(name, "ethaddr") && value != NULL) {
            if (strlen(value) + 1 < 20) {
                eth_mac = (char*)malloc(strlen(value) + 1);
                if (eth_mac)
                    strcpy(eth_mac, value);
            }
        }
        position += slen + 1;
    }

    fclose(fp);
    free(buf);
    if (!model) {
        if (eth_mac)
            free(eth_mac);
        return RP_HP_ERM;
    }
    hp_checkModel(model, eth_mac);
    if (model)
        free(model);
    if (eth_mac)
        free(eth_mac);
    return RP_HP_OK;
}

profiles_t* hp_cmn_GetLoadedProfile() {
    return g_profile;
}

const char* getGainName(rp_HPADCGainMode_t mode) {
    switch (mode) {
        case RP_HP_ADC_GAIN_NORMAL:
            return "NORMAL (LV)";
        case RP_HP_ADC_GAIN_HIGH:
            return "HIGH";
        default:
            return "ERROR GAIN MODE";
            break;
    }
}

int hp_cmn_Print(profiles_t* p) {
    if (!p) {
        return RP_HP_EU;
    }
    fprintf(stdout, "***********************************************************************\n");
    fprintf(stdout, "Board\n");
    fprintf(stdout, "\t* Board model (rp_HPeModels_t) %d\n", p->boardModel);
    fprintf(stdout, "\t* Board name: %s\n", p->boardName);
    fprintf(stdout, "\t* Board model from eeprom: %s\n", p->boardModelEEPROM);
    fprintf(stdout, "\t* Board MAC address from eeprom: %s\n", p->boardETH_MAC);
    fprintf(stdout, "\t* Zynq model (rp_HPeZynqModels_t) %d\n", p->zynqCPUModel);
    fprintf(stdout, "\t* RAM size: %d MB\n", p->ramMB);
    fprintf(stdout, "\t* Oscillator Rate: %u\n", p->oscillator_rate);
    fprintf(stdout, "\t* ADC chip Full Scale: %f\n", p->fast_adc_full_scale);
    fprintf(stdout, "\t* DAC chip Full Scale: %f\n", p->fast_dac_full_scale);

    fprintf(stdout, "FAST ADC\n");
    fprintf(stdout, "\t* Rate: %u\n", p->fast_adc_rate);
    fprintf(stdout, "\t* Filter present: %u\n", p->is_fast_adc_filter_present);
    fprintf(stdout, "\t* Spectrum resolution: %u\n", p->fast_adc_spectrum_resolution);
    fprintf(stdout, "\t* Count: %u\n", p->fast_adc_count_channels);
    fprintf(stdout, "\t* Is signed: %u\n", p->fast_adc_is_sign);
    fprintf(stdout, "\t* Bits: %u\n", p->fast_adc_bits);
    fprintf(stdout, "\t* HV mode (1:20): %d\n", p->is_LV_HV_mode);
    fprintf(stdout, "\t* AD/DC mode: %d\n", p->is_AC_DC_mode);

    for (int i = 0; i < p->fast_adc_count_channels; i++) {
        for (int g = 0; g <= RP_HP_ADC_GAIN_HIGH; g++) {
            fprintf(stdout, "\t\t- Channel: %d Gain %s = %f\n", i + 1, getGainName((rp_HPADCGainMode_t)g), p->fast_adc_gain[g][i]);
        }
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "FAST DAC\n");
    fprintf(stdout, "\t* Is present: %u\n", p->is_dac_present);
    fprintf(stdout, "\t* Rate: %u\n", p->fast_dac_rate);
    fprintf(stdout, "\t* Count: %u\n", p->fast_dac_count_channels);
    fprintf(stdout, "\t* Is signed: %u\n", p->fast_dac_is_sign);
    fprintf(stdout, "\t* Bits: %u\n", p->fast_dac_bits);

    for (int i = 0; i < p->fast_dac_count_channels; i++) {
        fprintf(stdout, "\t\t- Channel: %d Out full scale %f\n", i + 1, p->fast_dac_out_full_scale[i]);
    }

    fprintf(stdout, "SLOW ADC\n");
    fprintf(stdout, "\t* Count: %u\n", p->slow_adc_count_channels);

    for (int i = 0; i < p->slow_adc_count_channels; i++) {
        fprintf(stdout, "\t\t- Is signed: %u\n", p->slow_adc[i].is_signed);
        fprintf(stdout, "\t\t- Bits: %u\n", p->slow_adc[i].bits);
        fprintf(stdout, "\t\t- Full scale: %f\n", p->slow_adc[i].fullScale);
    }

    fprintf(stdout, "SLOW DAC\n");
    fprintf(stdout, "\t* Count: %u\n", p->slow_dac_count_channels);

    for (int i = 0; i < p->slow_dac_count_channels; i++) {
        fprintf(stdout, "\t\t- Is signed: %u\n", p->slow_dac[i].is_signed);
        fprintf(stdout, "\t\t- Bits: %u\n", p->slow_dac[i].bits);
        fprintf(stdout, "\t\t- Full scale: %f\n", p->slow_dac[i].fullScale);
    }

    fprintf(stdout, "FAST DAC x5 gain: %d\n", p->is_DAC_gain_x5);
    fprintf(stdout, "FAST DAC 50 Ohm mode (Hi-Z): %d\n", p->is_DAC_50_Ohm_mode);
    fprintf(stdout, "FAST DAC overheating protection: %d\n", p->is_fast_dac_temp_protection);
    fprintf(stdout, "FAST ADC/DAC calibration: %d\n", p->is_fast_calibration);
    fprintf(stdout, "FAST ADC attenuator controller: %d\n", p->is_attenuator_controller_present);
    fprintf(stdout, "FAST ADC External trigger level available: %d\n", p->is_ext_trigger_level_available);
    fprintf(stdout, "FAST ADC External trigger level full scale: %d\n", p->external_trigger_full_scale);
    fprintf(stdout, "FAST ADC External trigger level is signed: %d\n", p->is_ext_trigger_signed);
    fprintf(stdout, "FAST ADC DMA mode support (v0.94): %d\n", p->is_dma_mode_v0_94);

    fprintf(stdout, "FAST ADC Split trigger mode (v0.94): %d\n", p->is_split_osc_triggers);

    fprintf(stdout, "\nDaisy chain clock sync support: %u\n", p->is_daisy_chain_clock_sync);

    fprintf(stdout, "GPIO DIO_N count: %u\n", p->gpio_N_count);
    fprintf(stdout, "GPIO DIO_P count: %u\n", p->gpio_P_count);

    fprintf(stdout, "\nE3 Is present: %u\n", p->is_E3_present);
    fprintf(stdout, "E3 High speed GPIO support: %d\n", p->is_E3_high_speed_gpio);
    fprintf(stdout, "E3 High speed GPIO rate: %u\n", p->E3_high_speed_gpio_rate);
    fprintf(stdout, "E3 QSPI for eMMC support: %d\n", p->is_E3_high_speed_gpio);

    fprintf(stdout, "Support for calibration on FPGA: %d\n", p->is_calib_in_fpga);

    fprintf(stdout, "***********************************************************************\n");
    return RP_HP_OK;
}

void hp_cmn_PrintKeyHelp() {
    uint32_t index = 0;
    while (table_keys_help[index] != NULL) {
        const char* key = table_keys_help[index++];
        const char* desc = table_keys_help[index++];
        fprintf(stdout, "\t\t\t%s - %s\n", key, desc);
    }
}

profiles_t* hp_cmn_getProfile(rp_HPeModels_t model) {
    switch (model) {
        case STEM_125_10_v1_0:
            return getProfile_STEM_125_10_v1_0();
        case STEM_125_14_v1_0:
            return getProfile_STEM_125_14_v1_0();
        case STEM_125_14_v1_1:
            return getProfile_STEM_125_14_v1_1();
        case STEM_122_16SDR_v1_0:
            return getProfile_STEM_122_16SDR_v1_0();
        case STEM_122_16SDR_v1_1:
            return getProfile_STEM_122_16SDR_v1_1();
        case STEM_125_14_LN_v1_1:
            return getProfile_STEM_125_14_LN_v1_1();
        case STEM_125_14_Z7020_v1_0:
            return getProfile_STEM_125_14_Z7020_v1_0();
        case STEM_125_14_Z7020_LN_v1_1:
            return getProfile_STEM_125_14_Z7020_LN_v1_1();
        case STEM_125_14_Z7020_4IN_v1_0:
            return getProfile_STEM_125_14_Z7020_4IN_v1_0();
        case STEM_125_14_Z7020_4IN_v1_2:
            return getProfile_STEM_125_14_Z7020_4IN_v1_2();
        case STEM_125_14_Z7020_4IN_v1_3:
            return getProfile_STEM_125_14_Z7020_4IN_v1_3();
        case STEM_125_14_Z7020_4IN_BO_v1_3:
            return getProfile_STEM_125_14_Z7020_4IN_BO_v1_3();
        case STEM_250_12_v1_0:
            return getProfile_STEM_250_12_v1_0();
        case STEM_250_12_v1_1:
            return getProfile_STEM_250_12_v1_1();
        case STEM_250_12_v1_2:
            return getProfile_STEM_250_12_v1_2();
        case STEM_250_12_120:
            return getProfile_STEM_250_12_v1_2a();
        case STEM_250_12_v1_2a:
            return getProfile_STEM_250_12_v1_2b();
        case STEM_250_12_v1_2b:
            return getProfile_STEM_250_12_120();
        case STEM_125_14_LN_BO_v1_1:
            return getProfile_STEM_125_14_LN_BO_v1_1();
        case STEM_125_14_LN_CE1_v1_1:
            return getProfile_STEM_125_14_LN_CE1_v1_1();
        case STEM_125_14_LN_CE2_v1_1:
            return getProfile_STEM_125_14_LN_CE2_v1_1();
        case STEM_125_14_v2_0:
            return getProfile_STEM_125_14_v2_0();
        case STEM_125_14_Pro_v2_0:
            return getProfile_STEM_125_14_Pro_v2_0();
        case STEM_125_14_Z7020_Pro_v2_0:
            return getProfile_STEM_125_14_Z7020_Pro_v1_0();
        case STEM_125_14_Z7020_Ind_v2_0:
            return getProfile_STEM_125_14_Z7020_Pro_v2_0();
        case STEM_125_14_Z7020_Pro_v1_0:
            return getProfile_STEM_125_14_Z7020_Ind_v2_0();
        case STEM_125_14_Z7020_LL_v1_1:
            return getProfile_STEM_125_14_Z7020_LL_v1_1();
        case STEM_65_16_Z7020_LL_v1_1:
            return getProfile_STEM_125_14_Z7020_LL_v1_2();
        case STEM_125_14_Z7020_LL_v1_2:
            return getProfile_STEM_65_16_Z7020_LL_v1_1();
        case STEM_125_14_Z7020_TI_v1_3:
            return getProfile_STEM_125_14_Z7020_TI_v1_3();
        case STEM_65_16_Z7020_TI_v1_3:
            return getProfile_STEM_65_16_Z7020_TI_v1_3();
        default:
            return NULL;
    }
}
std::string float_to_string_trim(float num) {
    std::string s = std::to_string(num);
    s.erase(s.find_last_not_of('0') + 1, std::string::npos);
    if (s.back() == '.') {
        s.pop_back();
    }
    return s;
}

int hp_cmn_GetFPGAVersion(rp_HPeModels_t model, const char** _no_free_value) {
    switch (model) {
        case STEM_125_10_v1_0:
            *_no_free_value = "z10_125";
            break;
        case STEM_125_14_v1_0:
            *_no_free_value = "z10_125";
            break;
        case STEM_125_14_v1_1:
            *_no_free_value = "z10_125";
            break;
        case STEM_125_14_LN_v1_1:
            *_no_free_value = "z10_125";
            break;
        case STEM_125_14_LN_BO_v1_1:
            *_no_free_value = "z10_125";
            break;
        case STEM_125_14_LN_CE1_v1_1:
            *_no_free_value = "z10_125";
            break;
        case STEM_125_14_LN_CE2_v1_1:
            *_no_free_value = "z10_125";
            break;
        case STEM_122_16SDR_v1_0:
            *_no_free_value = "z20_122";
            break;
        case STEM_122_16SDR_v1_1:
            *_no_free_value = "z20_122";
            break;
        case STEM_125_14_Z7020_v1_0:
            *_no_free_value = "z20_125";
            break;
        case STEM_125_14_Z7020_LN_v1_1:
            *_no_free_value = "z20_125";
            break;
        case STEM_125_14_Z7020_4IN_v1_0:
            *_no_free_value = "z20_125_4ch";
            break;
        case STEM_125_14_Z7020_4IN_v1_2:
            *_no_free_value = "z20_125_4ch";
            break;
        case STEM_125_14_Z7020_4IN_v1_3:
            *_no_free_value = "z20_125_4ch";
            break;
        case STEM_125_14_Z7020_4IN_BO_v1_3:
            *_no_free_value = "z20_125_4ch";
            break;
        case STEM_250_12_v1_0:
            *_no_free_value = "z20_250_1_0";
            break;
        case STEM_250_12_v1_1:
            *_no_free_value = "z20_250";
            break;
        case STEM_250_12_v1_2:
            *_no_free_value = "z20_250";
            break;
        case STEM_250_12_v1_2a:
            *_no_free_value = "z20_250";
            break;
        case STEM_250_12_v1_2b:
            *_no_free_value = "z20_250";
            break;
        case STEM_250_12_120:
            *_no_free_value = "z20_250";
            break;
        case STEM_125_14_v2_0:
            *_no_free_value = "z10_125_v2";
            break;
        case STEM_125_14_Pro_v2_0:
            *_no_free_value = "z10_125_pro_v2";
            break;
        case STEM_125_14_Z7020_Pro_v1_0:
            *_no_free_value = "z20_125_v2";
            break;
        case STEM_125_14_Z7020_Pro_v2_0:
            *_no_free_value = "z20_125_v2";
            break;
        case STEM_125_14_Z7020_Ind_v2_0:
            *_no_free_value = "z20_125_v2";
            break;
        case STEM_125_14_Z7020_LL_v1_1:
            *_no_free_value = "z20_125_ll";
            break;
        case STEM_65_16_Z7020_LL_v1_1:
            *_no_free_value = "z20_125_ll";
            break;
        case STEM_125_14_Z7020_LL_v1_2:
            *_no_free_value = "z20_125_ll";
            break;
        case STEM_65_16_Z7020_TI_v1_3:
            *_no_free_value = "z20_125_ll";
            break;
        case STEM_125_14_Z7020_TI_v1_3:
            *_no_free_value = "z20_125_ll";
            break;
        default:
            *_no_free_value = "";
            return RP_HP_EMU;
            break;
    }
    return RP_HP_OK;
}

std::string getValueForKey(rp_HPeModels_t model, std::string key) {
    auto p = hp_cmn_getProfile(model);
    if (p == NULL) {
        fprintf(stderr, "[Error] Unknown model %d\n", model);
        return "";
    }

    if (key == "name") {
        return p->boardName;
    }

    if (key == "model") {
        return std::to_string(p->boardModel);
    }

    if (key == "fpga_path") {
        const char* modelFPGA = NULL;
        hp_cmn_GetFPGAVersion(model, &modelFPGA);
        return std::string(modelFPGA);
    }

    if (key == "zynq") {
        switch (p->zynqCPUModel) {
            case Z7010:
                return "Z7010";
            case Z7020:
                return "Z7020";
            default:
                break;
        }
        return "Error";
    }

    if (key == "osc_rate") {
        return std::to_string(p->oscillator_rate);
    }

    if (key == "f_adc_fs") {
        return float_to_string_trim(p->fast_adc_full_scale);
    }

    if (key == "f_adc_rate") {
        return std::to_string(p->fast_adc_rate);
    }

    if (key == "f_adc_is_sign") {
        return std::to_string(p->fast_adc_is_sign);
    }

    if (key == "f_adc_bits") {
        return std::to_string(p->fast_adc_bits);
    }

    if (key == "f_adc_count") {
        return std::to_string(p->fast_adc_count_channels);
    }

    if (key == "f_adc_gain") {
        std::string g = "";
        for (uint8_t i = 0; i < p->fast_adc_count_channels; i++) {
            g += "(1:1 - " + float_to_string_trim(p->fast_adc_gain[RP_HP_ADC_GAIN_NORMAL][i]);
            g += "/1:20 - " + float_to_string_trim(p->fast_adc_gain[RP_HP_ADC_GAIN_HIGH][i]) + ")";
        }
        return g;
    }

    if (key == "f_is_dac") {
        return std::to_string(p->is_dac_present);
    }

    if (key == "f_dac_fs") {
        return float_to_string_trim(p->fast_dac_full_scale);
    }

    if (key == "f_dac_rate") {
        return std::to_string(p->fast_dac_rate);
    }

    if (key == "f_dac_is_sign") {
        return std::to_string(p->fast_dac_is_sign);
    }

    if (key == "f_dac_bits") {
        return std::to_string(p->fast_dac_bits);
    }

    if (key == "f_dac_count") {
        return std::to_string(p->fast_dac_count_channels);
    }

    if (key == "f_dac_gain") {
        std::string g = "";
        for (uint8_t i = 0; i < p->fast_dac_count_channels; i++) {
            g += "(" + float_to_string_trim(p->fast_dac_out_full_scale[i]) + ")";
        }
        return g;
    }

    if (key == "is_hv_lv") {
        return std::to_string(p->is_LV_HV_mode);
    }

    if (key == "is_ac_dc") {
        return std::to_string(p->is_AC_DC_mode);
    }

    if (key == "s_adc_count") {
        return std::to_string(p->slow_adc_count_channels);
    }

    if (key == "s_adc_fs") {
        std::string g = "";
        for (uint8_t i = 0; i < p->slow_adc_count_channels; i++) {
            g += "(" + float_to_string_trim(p->slow_adc[i].fullScale) + ")";
        }
        return g;
    }

    if (key == "s_adc_bits") {
        std::string g = "";
        for (uint8_t i = 0; i < p->slow_adc_count_channels; i++) {
            g += "(" + std::to_string(p->slow_adc[i].bits) + ")";
        }
        return g;
    }

    if (key == "s_adc_is_sign") {
        std::string g = "";
        for (uint8_t i = 0; i < p->slow_adc_count_channels; i++) {
            g += "(" + std::to_string(p->slow_adc[i].is_signed) + ")";
        }
        return g;
    }

    if (key == "s_dac_count") {
        return std::to_string(p->slow_dac_count_channels);
    }

    if (key == "s_dac_fs") {
        std::string g = "";
        for (uint8_t i = 0; i < p->slow_dac_count_channels; i++) {
            g += "(" + float_to_string_trim(p->slow_dac[i].fullScale) + ")";
        }
        return g;
    }

    if (key == "s_dac_bits") {
        std::string g = "";
        for (uint8_t i = 0; i < p->slow_dac_count_channels; i++) {
            g += "(" + std::to_string(p->slow_dac[i].bits) + ")";
        }
        return g;
    }

    if (key == "s_dac_is_sign") {
        std::string g = "";
        for (uint8_t i = 0; i < p->slow_dac_count_channels; i++) {
            g += "(" + std::to_string(p->slow_dac[i].is_signed) + ")";
        }
        return g;
    }

    if (key == "is_dac_x5") {
        return std::to_string(p->is_DAC_gain_x5);
    }

    if (key == "is_f_calib") {
        return std::to_string(p->is_fast_calibration);
    }

    if (key == "is_pll_control") {
        return std::to_string(p->is_pll_control_present);
    }

    if (key == "is_f_adc_filter") {
        return std::to_string(p->is_fast_adc_filter_present);
    }

    if (key == "is_f_dac_t_prot") {
        return std::to_string(p->is_fast_dac_temp_protection);
    }

    if (key == "is_att_controller") {
        return std::to_string(p->is_attenuator_controller_present);
    }

    if (key == "is_ext_trig_lev") {
        return std::to_string(p->is_ext_trigger_level_available);
    }

    if (key == "is_ext_trig_fs") {
        return std::to_string(p->external_trigger_full_scale);
    }

    if (key == "is_ext_trig_is_sign") {
        return std::to_string(p->is_ext_trigger_signed);
    }

    if (key == "spec_max_rate") {
        return std::to_string(p->fast_adc_spectrum_resolution);
    }

    if (key == "is_daisy_clock_sync") {
        return std::to_string(p->is_daisy_chain_clock_sync);
    }

    if (key == "is_dma_094") {
        return std::to_string(p->is_dma_mode_v0_94);
    }

    if (key == "is_dac_50ohm") {
        return std::to_string(p->is_DAC_50_Ohm_mode);
    }

    if (key == "is_split_trig") {
        return std::to_string(p->is_split_osc_triggers);
    }

    if (key == "gpio_count") {
        return "N=" + std::to_string(p->gpio_N_count) + " / P= " + std::to_string(p->gpio_P_count);
    }

    if (key == "ram") {
        return std::to_string(p->ramMB);
    }

    if (key == "is_e3") {
        return std::to_string(p->is_E3_present);
    }

    if (key == "is_e3_hs_gpio") {
        return std::to_string(p->is_E3_high_speed_gpio);
    }

    if (key == "is_e3_qspi") {
        return std::to_string(p->is_E3_mmc_qspi);
    }

    if (key == "is_e3_hs_rate") {
        return std::to_string(p->E3_high_speed_gpio_rate);
    }

    if (key == "is_fpga_calib") {
        return std::to_string(p->is_calib_in_fpga);
    }

    return "";
}

std::vector<std::string> splitStringByComma(const char* input) {
    std::vector<std::string> result;
    std::stringstream ss(input);
    std::string item;

    while (std::getline(ss, item, ',')) {
        item.erase(item.begin(), std::find_if(item.begin(), item.end(), [](int ch) { return !std::isspace(ch); }));
        item.erase(std::find_if(item.rbegin(), item.rend(), [](int ch) { return !std::isspace(ch); }).base(), item.end());

        if (!item.empty()) {
            result.push_back(item);
        }
    }

    return result;
}

std::string padString(const std::string& input, size_t total_length) {
    // printf("%s %d %d\n", input.c_str(), total_length, input.length());
    size_t actual_trailing_spaces = total_length - input.length() - 1;

    std::string result = " " + input;

    result.append(actual_trailing_spaces, ' ');

    if (result.length() > total_length) {
        result.resize(total_length);
    } else if (result.length() < total_length) {
        result.append(total_length - result.length(), ' ');
    }

    return result;
}
bool containsCaseInsensitive(const std::string& str, const std::string& substr) {
    auto it = std::search(str.begin(), str.end(), substr.begin(), substr.end(), [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); });
    return it != str.end();
}

void hp_cmn_PrintPivotTable(char* _keys) {

    auto keys = splitStringByComma(_keys);
    if (std::find(keys.begin(), keys.end(), "all") != keys.end()) {
        keys.clear();
        uint32_t index = 2;
        while (table_keys_help[index] != NULL) {
            const char* key = table_keys_help[index++];
            index++;
            keys.push_back(key);
        }
    }
    std::vector<std::string> new_keys;
    for (auto& par : keys) {
        std::vector<std::string> find_keys;
        uint32_t index = 2;
        while (table_keys_help[index] != NULL) {
            const char* key = table_keys_help[index++];
            index++;
            if (containsCaseInsensitive(key, par)) {
                find_keys.push_back(key);
            }
        }
        new_keys.insert(new_keys.end(), find_keys.begin(), find_keys.end());
    }
    keys = new_keys;
    std::map<std::string, std::map<rp_HPeModels_t, std::string>> values;
    std::map<std::string, uint32_t> values_max_len;
    std::vector<std::string> cols;

    if (values.count("name") == 0) {
        // values["name"] = std::vector<std::pair<rp_HPeModels_t, std::string>>();
        values_max_len["name"] = 4;
        cols.push_back("name");
    }

    for (int i = 0; i <= STEM_65_16_Z7020_TI_v1_3; i++) {
        auto s = getValueForKey((rp_HPeModels_t)i, "name");
        values["name"][(rp_HPeModels_t)i] = s;
        if (values_max_len["name"] < s.length())
            values_max_len["name"] = s.length();
    }
    for (auto& key : keys) {
        if (values.count(key) == 0) {
            // values[key] = std::vector<std::pair<rp_HPeModels_t, std::string>>();
            values_max_len[key] = key.length();
            cols.push_back(key);
        }

        for (int i = 0; i <= STEM_65_16_Z7020_TI_v1_3; i++) {
            auto s = getValueForKey((rp_HPeModels_t)i, key);
            values[key][(rp_HPeModels_t)i] = s;
            if (values_max_len[key] < s.length())
                values_max_len[key] = s.length();
        }
    }

    uint32_t table_len = 0;
    for (const auto& pair : values_max_len) {
        table_len += pair.second + 2;
    }
    table_len += values_max_len.size() > 0 ? values_max_len.size() - 1 : 0;
    auto tab_split = std::string(table_len, '*');
    auto row_split = std::string(table_len, '-');
    printf("%s\n", tab_split.c_str());

    bool skip_first = false;
    for (const auto& col : cols) {
        if (skip_first) {
            printf("|");
        }
        skip_first = true;
        printf("%s", padString(col, values_max_len[col] + 2).c_str());
    }
    printf("\n");
    printf("%s\n", tab_split.c_str());

    for (int i = 0; i <= STEM_65_16_Z7020_TI_v1_3; i++) {
        skip_first = false;
        for (const auto& col : cols) {
            if (skip_first) {
                printf("|");
            }
            skip_first = true;
            auto v = values[col][(rp_HPeModels_t)i];
            printf("%s", padString(v, values_max_len[col] + 2).c_str());
        }
        printf("\n");
        // printf("%s\n", row_split.c_str());
    }
}

int hp_cmn_GetADCBaseRateFromConfig(rp_HPeModels_t model) {
    std::ifstream file(ADC_BASE_RATE_PATH + std::to_string((int)model) + ".conf");
    int value = 0;
    if (file.is_open()) {
        file >> value;
        file.close();
    }
    return value;
}

int hp_cmn_GetDACBaseRateFromConfig(rp_HPeModels_t model) {
    std::ifstream file(DAC_BASE_RATE_PATH + std::to_string((int)model) + ".conf");
    int value = 0;
    if (file.is_open()) {
        file >> value;
        file.close();
    }
    return value;
}