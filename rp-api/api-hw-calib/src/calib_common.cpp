#include "calib_common.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rp_log.h"

static const char eeprom_device[] = "/sys/bus/i2c/devices/0-0050/eeprom";
static const int eeprom_calib_off = 0x0000;
static const int eeprom_calib_factory_off = 0x1c00;

uint32_t calibBaseScaleFromVoltage(float voltageScale, bool uni_is_calib) {
    uint64_t baseCalib = uni_is_calib ? 0x10000000 : (uint64_t)1 << 32;
    return (uint32_t)(voltageScale / 100.0 * (baseCalib));
}

uint_gain_calib_t convertFloatToInt(channel_calib_t* param, uint8_t precision) {
    uint_gain_calib_t calib;
    calib.precision = precision;
    calib.base = pow(2, precision);
    calib.gain = param->gainCalc * calib.base;
    calib.offset = param->offset;
    return calib;
}

uint8_t* readParams(uint16_t* size, bool use_factory_zone) {
    FILE* fp;

    /* open EEPROM device */
    fp = fopen(eeprom_device, "r");
    if (fp == NULL) {
        ERROR_LOG("Error opening eeprom file.");
        return NULL;
    }

    /* ...and seek to the appropriate storage offset */
    int offset = use_factory_zone ? eeprom_calib_factory_off : eeprom_calib_off;
    if (fseek(fp, offset, SEEK_SET) < 0) {
        fclose(fp);
        return NULL;
    }

    uint8_t* buf = (uint8_t*)malloc(*size);
    if (!buf) {
        ERROR_LOG("Memory allocation error.");
        fclose(fp);
        return NULL;
    }

    *size = fread(buf, sizeof(char), *size, fp);
    fclose(fp);
    return buf;
}

uint8_t* readHeader(uint16_t* size, bool use_factory_zone) {
    FILE* fp;

    /* open EEPROM device */
    fp = fopen(eeprom_device, "r");
    if (fp == NULL) {
        ERROR_LOG("Error opening eeprom file.");
        return NULL;
    }

    /* ...and seek to the appropriate storage offset */
    int offset = use_factory_zone ? eeprom_calib_factory_off : eeprom_calib_off;
    if (fseek(fp, offset, SEEK_SET) < 0) {
        fclose(fp);
        return NULL;
    }

    uint8_t* buf = (uint8_t*)malloc(*size);
    if (!buf) {
        ERROR_LOG("Memory allocation error.");
        fclose(fp);
        return NULL;
    }

    *size = fread(buf, sizeof(char), *size, fp);
    fclose(fp);
    return buf;
}

int writeParams(uint8_t* buffer, uint16_t size, bool use_factory_zone) {
    FILE* fp;

    /* open EEPROM device */
    fp = fopen(eeprom_device, "w+");
    if (fp == NULL) {
        ERROR_LOG("Error opening eeprom file.");
        return -1;
    }

    /* ...and seek to the appropriate storage offset */
    int offset = use_factory_zone ? eeprom_calib_factory_off : eeprom_calib_off;
    if (fseek(fp, offset, SEEK_SET) < 0) {
        fclose(fp);
        return -1;
    }

    /* write data to EEPROM component */
    size = fwrite(buffer, sizeof(char), size, fp);
    fclose(fp);

    return size;
}

uint8_t* readFromEpprom(uint16_t* size) {
    return readParams(size, false);
}

uint8_t* readFromFactoryEpprom(uint16_t* size) {
    return readParams(size, true);
}

uint16_t writeToEpprom(uint8_t* buffer, uint16_t size) {
    return writeParams(buffer, size, false);
}

uint16_t writeToFactoryEpprom(uint8_t* buffer, uint16_t size) {
    return writeParams(buffer, size, true);
}

bool convertV1(rp_calib_params_t* param, rp_calib_params_v1_t* out) {
    if (param->fast_adc_count_1_1 != 2) {
        return false;
    }

    if (param->fast_adc_count_1_20 != 2) {
        return false;
    }

    if (param->fast_adc_count_1_1_ac != 0) {
        return false;
    }

    if (param->fast_adc_count_1_20_ac != 0) {
        return false;
    }

    if (param->fast_dac_count_x1 != 2) {
        return false;
    }

    if (param->fast_dac_count_x5 != 0) {
        return false;
    }

    out->dataStructureId = param->dataStructureId;
    out->wpCheck = param->wpCheck;

    out->fe_ch1_fs_g_hi = param->fast_adc_1_20[0].calibValue;
    out->fe_ch2_fs_g_hi = param->fast_adc_1_20[1].calibValue;
    out->fe_ch1_hi_offs = param->fast_adc_1_20[0].offset;
    out->fe_ch2_hi_offs = param->fast_adc_1_20[1].offset;

    out->fe_ch1_fs_g_lo = param->fast_adc_1_1[0].calibValue;
    out->fe_ch2_fs_g_lo = param->fast_adc_1_1[1].calibValue;
    out->fe_ch1_lo_offs = param->fast_adc_1_1[0].offset;
    out->fe_ch2_lo_offs = param->fast_adc_1_1[1].offset;

    out->low_filter_aa_ch1 = param->fast_adc_filter_1_1[0].aa;
    out->low_filter_bb_ch1 = param->fast_adc_filter_1_1[0].bb;
    out->low_filter_pp_ch1 = param->fast_adc_filter_1_1[0].pp;
    out->low_filter_kk_ch1 = param->fast_adc_filter_1_1[0].kk;

    out->low_filter_aa_ch2 = param->fast_adc_filter_1_1[1].aa;
    out->low_filter_bb_ch2 = param->fast_adc_filter_1_1[1].bb;
    out->low_filter_pp_ch2 = param->fast_adc_filter_1_1[1].pp;
    out->low_filter_kk_ch2 = param->fast_adc_filter_1_1[1].kk;

    out->hi_filter_aa_ch1 = param->fast_adc_filter_1_20[0].aa;
    out->hi_filter_bb_ch1 = param->fast_adc_filter_1_20[0].bb;
    out->hi_filter_pp_ch1 = param->fast_adc_filter_1_20[0].pp;
    out->hi_filter_kk_ch1 = param->fast_adc_filter_1_20[0].kk;

    out->hi_filter_aa_ch2 = param->fast_adc_filter_1_20[1].aa;
    out->hi_filter_bb_ch2 = param->fast_adc_filter_1_20[1].bb;
    out->hi_filter_pp_ch2 = param->fast_adc_filter_1_20[1].pp;
    out->hi_filter_kk_ch2 = param->fast_adc_filter_1_20[1].kk;

    out->be_ch1_fs = param->fast_dac_x1[0].calibValue;
    out->be_ch2_fs = param->fast_dac_x1[1].calibValue;
    out->be_ch1_dc_offs = param->fast_dac_x1[0].offset;
    out->be_ch2_dc_offs = param->fast_dac_x1[1].offset;

    out->magic = CALIB_MAGIC;

    return true;
}

bool convertV2(rp_calib_params_t* param, rp_calib_params_v2_t* out) {

    if (param->fast_adc_count_1_1 != 4) {
        return false;
    }

    if (param->fast_adc_count_1_20 != 4) {
        return false;
    }

    if (param->fast_adc_count_1_1_ac != 0) {
        return false;
    }

    if (param->fast_adc_count_1_20_ac != 0) {
        return false;
    }

    if (param->fast_dac_count_x1 != 0) {
        return false;
    }

    if (param->fast_dac_count_x5 != 0) {
        return false;
    }

    out->dataStructureId = param->dataStructureId;
    out->wpCheck = param->wpCheck;

    out->chA_g_hi = param->fast_adc_1_20[0].calibValue;
    out->chB_g_hi = param->fast_adc_1_20[1].calibValue;
    out->chC_g_hi = param->fast_adc_1_20[2].calibValue;
    out->chD_g_hi = param->fast_adc_1_20[3].calibValue;

    out->chA_g_low = param->fast_adc_1_1[0].calibValue;
    out->chB_g_low = param->fast_adc_1_1[1].calibValue;
    out->chC_g_low = param->fast_adc_1_1[2].calibValue;
    out->chD_g_low = param->fast_adc_1_1[3].calibValue;

    out->chA_hi_offs = param->fast_adc_1_20[0].offset;
    out->chB_hi_offs = param->fast_adc_1_20[1].offset;
    out->chC_hi_offs = param->fast_adc_1_20[2].offset;
    out->chD_hi_offs = param->fast_adc_1_20[3].offset;

    out->chA_low_offs = param->fast_adc_1_1[0].offset;
    out->chB_low_offs = param->fast_adc_1_1[1].offset;
    out->chC_low_offs = param->fast_adc_1_1[2].offset;
    out->chD_low_offs = param->fast_adc_1_1[3].offset;

    out->chA_hi_aa = param->fast_adc_filter_1_20[0].aa;
    out->chA_hi_bb = param->fast_adc_filter_1_20[0].bb;
    out->chA_hi_pp = param->fast_adc_filter_1_20[0].pp;
    out->chA_hi_kk = param->fast_adc_filter_1_20[0].kk;

    out->chA_low_aa = param->fast_adc_filter_1_1[0].aa;
    out->chA_low_bb = param->fast_adc_filter_1_1[0].bb;
    out->chA_low_pp = param->fast_adc_filter_1_1[0].pp;
    out->chA_low_kk = param->fast_adc_filter_1_1[0].kk;

    out->chB_hi_aa = param->fast_adc_filter_1_20[1].aa;
    out->chB_hi_bb = param->fast_adc_filter_1_20[1].bb;
    out->chB_hi_pp = param->fast_adc_filter_1_20[1].pp;
    out->chB_hi_kk = param->fast_adc_filter_1_20[1].kk;

    out->chB_low_aa = param->fast_adc_filter_1_1[1].aa;
    out->chB_low_bb = param->fast_adc_filter_1_1[1].bb;
    out->chB_low_pp = param->fast_adc_filter_1_1[1].pp;
    out->chB_low_kk = param->fast_adc_filter_1_1[1].kk;

    out->chC_hi_aa = param->fast_adc_filter_1_20[2].aa;
    out->chC_hi_bb = param->fast_adc_filter_1_20[2].bb;
    out->chC_hi_pp = param->fast_adc_filter_1_20[2].pp;
    out->chC_hi_kk = param->fast_adc_filter_1_20[2].kk;

    out->chC_low_aa = param->fast_adc_filter_1_1[2].aa;
    out->chC_low_bb = param->fast_adc_filter_1_1[2].bb;
    out->chC_low_pp = param->fast_adc_filter_1_1[2].pp;
    out->chC_low_kk = param->fast_adc_filter_1_1[2].kk;

    out->chD_hi_aa = param->fast_adc_filter_1_20[3].aa;
    out->chD_hi_bb = param->fast_adc_filter_1_20[3].bb;
    out->chD_hi_pp = param->fast_adc_filter_1_20[3].pp;
    out->chD_hi_kk = param->fast_adc_filter_1_20[3].kk;

    out->chD_low_aa = param->fast_adc_filter_1_1[3].aa;
    out->chD_low_bb = param->fast_adc_filter_1_1[3].bb;
    out->chD_low_pp = param->fast_adc_filter_1_1[3].pp;
    out->chD_low_kk = param->fast_adc_filter_1_1[3].kk;

    return true;
}

bool convertV3(rp_calib_params_t* param, rp_calib_params_v3_t* out) {

    if (param->fast_adc_count_1_1 != 2) {
        return false;
    }

    if (param->fast_adc_count_1_20 != 2) {
        return false;
    }

    if (param->fast_adc_count_1_1_ac != 2) {
        return false;
    }

    if (param->fast_adc_count_1_20_ac != 2) {
        return false;
    }

    if (param->fast_dac_count_x1 != 2) {
        return false;
    }

    if (param->fast_dac_count_x5 != 2) {
        return false;
    }

    out->dataStructureId = param->dataStructureId;
    out->wpCheck = param->wpCheck;

    out->gen_ch1_g_1 = param->fast_dac_x1[0].calibValue;
    out->gen_ch2_g_1 = param->fast_dac_x1[1].calibValue;
    out->gen_ch1_off_1 = param->fast_dac_x1[0].offset;
    out->gen_ch2_off_1 = param->fast_dac_x1[1].offset;

    out->gen_ch1_g_5 = param->fast_dac_x5[0].calibValue;
    out->gen_ch2_g_5 = param->fast_dac_x5[1].calibValue;
    out->gen_ch1_off_5 = param->fast_dac_x5[0].offset;
    out->gen_ch2_off_5 = param->fast_dac_x5[1].offset;

    out->osc_ch1_g_1_ac = param->fast_adc_1_1_ac[0].calibValue;
    out->osc_ch2_g_1_ac = param->fast_adc_1_1_ac[1].calibValue;
    out->osc_ch1_off_1_ac = param->fast_adc_1_1_ac[0].offset;
    out->osc_ch2_off_1_ac = param->fast_adc_1_1_ac[1].offset;

    out->osc_ch1_g_1_dc = param->fast_adc_1_1[0].calibValue;
    out->osc_ch2_g_1_dc = param->fast_adc_1_1[1].calibValue;
    out->osc_ch1_off_1_dc = param->fast_adc_1_1[0].offset;
    out->osc_ch2_off_1_dc = param->fast_adc_1_1[1].offset;

    out->osc_ch1_g_20_ac = param->fast_adc_1_20_ac[0].calibValue;
    out->osc_ch2_g_20_ac = param->fast_adc_1_20_ac[1].calibValue;
    out->osc_ch1_off_20_ac = param->fast_adc_1_20_ac[0].offset;
    out->osc_ch2_off_20_ac = param->fast_adc_1_20_ac[1].offset;

    out->osc_ch1_g_20_dc = param->fast_adc_1_20[0].calibValue;
    out->osc_ch2_g_20_dc = param->fast_adc_1_20[1].calibValue;
    out->osc_ch1_off_20_dc = param->fast_adc_1_20[0].offset;
    out->osc_ch2_off_20_dc = param->fast_adc_1_20[1].offset;

    return true;
}

bool convertV4(rp_calib_params_t* param, rp_calib_params_v1_t* out) {
    if (param->fast_adc_count_1_1 != 2) {
        return false;
    }

    if (param->fast_adc_count_1_20 != 0) {
        return false;
    }

    if (param->fast_adc_count_1_1_ac != 0) {
        return false;
    }

    if (param->fast_adc_count_1_20_ac != 0) {
        return false;
    }

    if (param->fast_dac_count_x1 != 2) {
        return false;
    }

    if (param->fast_dac_count_x5 != 0) {
        return false;
    }
    out->dataStructureId = param->dataStructureId;
    out->wpCheck = param->wpCheck;

    out->fe_ch1_fs_g_lo = param->fast_adc_1_1[0].calibValue;
    out->fe_ch2_fs_g_lo = param->fast_adc_1_1[1].calibValue;
    out->fe_ch1_lo_offs = param->fast_adc_1_1[0].offset;
    out->fe_ch2_lo_offs = param->fast_adc_1_1[1].offset;

    out->low_filter_aa_ch1 = param->fast_adc_filter_1_1[0].aa;
    out->low_filter_bb_ch1 = param->fast_adc_filter_1_1[0].bb;
    out->low_filter_pp_ch1 = param->fast_adc_filter_1_1[0].pp;
    out->low_filter_kk_ch1 = param->fast_adc_filter_1_1[0].kk;

    out->low_filter_aa_ch2 = param->fast_adc_filter_1_1[1].aa;
    out->low_filter_bb_ch2 = param->fast_adc_filter_1_1[1].bb;
    out->low_filter_pp_ch2 = param->fast_adc_filter_1_1[1].pp;
    out->low_filter_kk_ch2 = param->fast_adc_filter_1_1[1].kk;

    out->be_ch1_fs = param->fast_dac_x1[0].calibValue;
    out->be_ch2_fs = param->fast_dac_x1[1].calibValue;
    out->be_ch1_dc_offs = param->fast_dac_x1[0].offset;
    out->be_ch2_dc_offs = param->fast_dac_x1[1].offset;

    out->magic = CALIB_MAGIC_FILTER;

    return true;
}

bool convertGen2(rp_calib_params_t* param, rp_calib_params_v1_t* out) {
    if (param->fast_adc_count_1_1 != 2) {
        return false;
    }

    if (param->fast_adc_count_1_20 != 2) {
        return false;
    }

    if (param->fast_adc_count_1_1_ac != 0) {
        return false;
    }

    if (param->fast_adc_count_1_20_ac != 0) {
        return false;
    }

    if (param->fast_dac_count_x1 != 2) {
        return false;
    }

    if (param->fast_dac_count_x5 != 0) {
        return false;
    }

    out->dataStructureId = param->dataStructureId;
    out->wpCheck = param->wpCheck;

    out->fe_ch1_fs_g_hi = param->fast_adc_1_20[0].calibValue;
    out->fe_ch2_fs_g_hi = param->fast_adc_1_20[1].calibValue;
    out->fe_ch1_hi_offs = param->fast_adc_1_20[0].offset;
    out->fe_ch2_hi_offs = param->fast_adc_1_20[1].offset;

    out->fe_ch1_fs_g_lo = param->fast_adc_1_1[0].calibValue;
    out->fe_ch2_fs_g_lo = param->fast_adc_1_1[1].calibValue;
    out->fe_ch1_lo_offs = param->fast_adc_1_1[0].offset;
    out->fe_ch2_lo_offs = param->fast_adc_1_1[1].offset;

    out->low_filter_aa_ch1 = param->fast_adc_filter_1_1[0].aa;
    out->low_filter_bb_ch1 = param->fast_adc_filter_1_1[0].bb;
    out->low_filter_pp_ch1 = param->fast_adc_filter_1_1[0].pp;
    out->low_filter_kk_ch1 = param->fast_adc_filter_1_1[0].kk;

    out->low_filter_aa_ch2 = param->fast_adc_filter_1_1[1].aa;
    out->low_filter_bb_ch2 = param->fast_adc_filter_1_1[1].bb;
    out->low_filter_pp_ch2 = param->fast_adc_filter_1_1[1].pp;
    out->low_filter_kk_ch2 = param->fast_adc_filter_1_1[1].kk;

    out->hi_filter_aa_ch1 = param->fast_adc_filter_1_20[0].aa;
    out->hi_filter_bb_ch1 = param->fast_adc_filter_1_20[0].bb;
    out->hi_filter_pp_ch1 = param->fast_adc_filter_1_20[0].pp;
    out->hi_filter_kk_ch1 = param->fast_adc_filter_1_20[0].kk;

    out->hi_filter_aa_ch2 = param->fast_adc_filter_1_20[1].aa;
    out->hi_filter_bb_ch2 = param->fast_adc_filter_1_20[1].bb;
    out->hi_filter_pp_ch2 = param->fast_adc_filter_1_20[1].pp;
    out->hi_filter_kk_ch2 = param->fast_adc_filter_1_20[1].kk;

    out->be_ch1_fs = param->fast_dac_x1[0].calibValue;
    out->be_ch2_fs = param->fast_dac_x1[1].calibValue;
    out->be_ch1_dc_offs = param->fast_dac_x1[0].offset;
    out->be_ch2_dc_offs = param->fast_dac_x1[1].offset;

    out->magic = CALIB_MAGIC;

    return true;
}

rp_calib_params_t getDefault(rp_HPeModels_t model, bool setFilterZero) {
    uint32_t aa = setFilterZero ? DISABLE_FILT_AA : DEFAULT_1_1_FILT_AA;
    uint32_t bb = setFilterZero ? DISABLE_FILT_BB : DEFAULT_1_1_FILT_BB;
    uint32_t kk = setFilterZero ? DISABLE_FILT_KK : DEFAULT_1_1_FILT_KK;
    uint32_t pp = setFilterZero ? DISABLE_FILT_PP : DEFAULT_1_1_FILT_PP;
    uint32_t aa20 = setFilterZero ? DISABLE_FILT_AA : DEFAULT_1_20_FILT_AA;
    uint32_t bb20 = setFilterZero ? DISABLE_FILT_BB : DEFAULT_1_20_FILT_BB;
    uint32_t kk20 = setFilterZero ? DISABLE_FILT_KK : DEFAULT_1_20_FILT_KK;
    uint32_t pp20 = setFilterZero ? DISABLE_FILT_PP : DEFAULT_1_20_FILT_PP;

    rp_calib_params_t calib;
    memset(&calib, 0, sizeof(rp_calib_params_t));
    switch (model) {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:

            calib.fast_adc_count_1_1 = 2;
            calib.fast_adc_count_1_20 = 2;
            calib.fast_dac_count_x1 = 2;
            calib.dataStructureId = RP_HW_PACK_ID_V1;

            for (int i = 0; i < 2; ++i) {
                calib.fast_adc_1_1[i].calibValue = calibBaseScaleFromVoltage(20.0, false);
                calib.fast_adc_1_1[i].offset = 0;
                calib.fast_adc_1_1[i].baseScale = 20.0;
                calib.fast_adc_1_1[i].gainCalc = 1.0;

                calib.fast_adc_1_20[i].calibValue = calibBaseScaleFromVoltage(1.0, false);
                calib.fast_adc_1_20[i].offset = 0;
                calib.fast_adc_1_20[i].baseScale = 1.0;
                calib.fast_adc_1_20[i].gainCalc = 1.0;

                calib.fast_adc_filter_1_1[i].aa = aa;
                calib.fast_adc_filter_1_1[i].bb = bb;
                calib.fast_adc_filter_1_1[i].kk = kk;
                calib.fast_adc_filter_1_1[i].pp = pp;

                calib.fast_adc_filter_1_20[i].aa = aa20;
                calib.fast_adc_filter_1_20[i].bb = bb20;
                calib.fast_adc_filter_1_20[i].kk = kk20;
                calib.fast_adc_filter_1_20[i].pp = pp20;

                calib.fast_dac_x1[i].calibValue = calibBaseScaleFromVoltage(1.0, false);
                calib.fast_dac_x1[i].offset = 0;
                calib.fast_dac_x1[i].baseScale = 1.0;
                calib.fast_dac_x1[i].gainCalc = 1.0;
            }

            break;

        case STEM_125_14_v2_0:
        case STEM_125_14_Pro_v2_0:
        case STEM_125_14_Z7020_Pro_v1_0:
        case STEM_125_14_Z7020_Pro_v2_0:
        case STEM_125_14_Z7020_Ind_v2_0:

        case STEM_125_14_Z7020_LL_v1_1:
        case STEM_65_16_Z7020_LL_v1_1:

            calib.fast_adc_count_1_1 = 2;
            calib.fast_adc_count_1_20 = 2;
            calib.fast_dac_count_x1 = 2;
            calib.dataStructureId = RP_HW_PACK_ID_V1;

            for (int i = 0; i < 2; ++i) {
                calib.fast_adc_1_1[i].calibValue = calibBaseScaleFromVoltage(20.0, false);
                calib.fast_adc_1_1[i].offset = 0;
                calib.fast_adc_1_1[i].baseScale = 20.0;
                calib.fast_adc_1_1[i].gainCalc = 1.0;

                calib.fast_adc_1_20[i].calibValue = calibBaseScaleFromVoltage(1.0, false);
                calib.fast_adc_1_20[i].offset = 0;
                calib.fast_adc_1_20[i].baseScale = 1.0;
                calib.fast_adc_1_20[i].gainCalc = 1.0;

                calib.fast_adc_filter_1_1[i].aa = aa;
                calib.fast_adc_filter_1_1[i].bb = bb;
                calib.fast_adc_filter_1_1[i].kk = kk;
                calib.fast_adc_filter_1_1[i].pp = pp;

                calib.fast_adc_filter_1_20[i].aa = aa20;
                calib.fast_adc_filter_1_20[i].bb = bb20;
                calib.fast_adc_filter_1_20[i].kk = kk20;
                calib.fast_adc_filter_1_20[i].pp = pp20;

                calib.fast_dac_x1[i].calibValue = calibBaseScaleFromVoltage(1.0, false);
                calib.fast_dac_x1[i].offset = 0;
                calib.fast_dac_x1[i].baseScale = 1.0;
                calib.fast_dac_x1[i].gainCalc = 1.0;
            }

            break;

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            calib.fast_adc_count_1_1 = 2;
            calib.fast_dac_count_x1 = 2;
            calib.dataStructureId = RP_HW_PACK_ID_V4;

            for (int i = 0; i < 2; ++i) {
                calib.fast_adc_1_1[i].calibValue = calibBaseScaleFromVoltage(20.0, false);
                calib.fast_adc_1_1[i].offset = 0;
                calib.fast_adc_1_1[i].baseScale = 20;
                calib.fast_adc_1_1[i].gainCalc = 1.0;

                calib.fast_dac_x1[i].calibValue = calibBaseScaleFromVoltage(1.0, false);
                calib.fast_dac_x1[i].offset = 0;
                calib.fast_dac_x1[i].baseScale = 1.0;
                calib.fast_dac_x1[i].gainCalc = 1.0;

                calib.fast_adc_filter_1_1[i].aa = aa;
                calib.fast_adc_filter_1_1[i].bb = bb;
                calib.fast_adc_filter_1_1[i].kk = kk;
                calib.fast_adc_filter_1_1[i].pp = pp;
            }
            break;

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            calib.fast_adc_count_1_1 = 4;
            calib.fast_adc_count_1_20 = 4;
            calib.dataStructureId = RP_HW_PACK_ID_V3;

            for (int i = 0; i < 4; ++i) {
                calib.fast_adc_1_1[i].calibValue = calibBaseScaleFromVoltage(20.0, false);
                calib.fast_adc_1_1[i].offset = 0;
                calib.fast_adc_1_1[i].baseScale = 20.0;
                calib.fast_adc_1_1[i].gainCalc = 1.0;

                calib.fast_adc_1_20[i].calibValue = calibBaseScaleFromVoltage(1.0, false);
                calib.fast_adc_1_20[i].offset = 0;
                calib.fast_adc_1_20[i].baseScale = 1.0;
                calib.fast_adc_1_20[i].gainCalc = 1.0;

                calib.fast_adc_filter_1_1[i].aa = aa;
                calib.fast_adc_filter_1_1[i].bb = bb;
                calib.fast_adc_filter_1_1[i].kk = kk;
                calib.fast_adc_filter_1_1[i].pp = pp;

                calib.fast_adc_filter_1_20[i].aa = aa20;
                calib.fast_adc_filter_1_20[i].bb = bb20;
                calib.fast_adc_filter_1_20[i].kk = kk20;
                calib.fast_adc_filter_1_20[i].pp = pp20;
            }
            break;

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
        case STEM_250_12_120:
            calib.fast_adc_count_1_1 = 2;
            calib.fast_adc_count_1_20 = 2;
            calib.fast_adc_count_1_1_ac = 2;
            calib.fast_adc_count_1_20_ac = 2;
            calib.fast_dac_count_x1 = 2;
            calib.fast_dac_count_x5 = 2;
            calib.dataStructureId = RP_HW_PACK_ID_V2;

            for (int i = 0; i < 2; ++i) {
                calib.fast_adc_1_1[i].calibValue = calibBaseScaleFromVoltage(20.0, false);
                calib.fast_adc_1_1[i].offset = 0;
                calib.fast_adc_1_1[i].baseScale = 20.0;
                calib.fast_adc_1_1[i].gainCalc = 1.0;

                calib.fast_adc_1_20[i].calibValue = calibBaseScaleFromVoltage(1.0, false);
                calib.fast_adc_1_20[i].offset = 0;
                calib.fast_adc_1_20[i].baseScale = 1.0;
                calib.fast_adc_1_20[i].gainCalc = 1.0;

                calib.fast_adc_1_1_ac[i].calibValue = calibBaseScaleFromVoltage(20.0, false);
                calib.fast_adc_1_1_ac[i].offset = 0;
                calib.fast_adc_1_1_ac[i].baseScale = 20.0;
                calib.fast_adc_1_1_ac[i].gainCalc = 1.0;

                calib.fast_adc_1_20_ac[i].calibValue = calibBaseScaleFromVoltage(1.0, false);
                calib.fast_adc_1_20_ac[i].offset = 0;
                calib.fast_adc_1_20_ac[i].baseScale = 1.0;
                calib.fast_adc_1_20_ac[i].gainCalc = 1.0;

                calib.fast_dac_x1[i].calibValue = calibBaseScaleFromVoltage(2.0, false);
                calib.fast_dac_x1[i].offset = 0;
                calib.fast_dac_x1[i].baseScale = 2.0;
                calib.fast_dac_x1[i].gainCalc = 1.0;

                calib.fast_dac_x5[i].calibValue = calibBaseScaleFromVoltage(2.0, false);
                calib.fast_dac_x5[i].offset = 0;
                calib.fast_dac_x5[i].baseScale = 2.0;
                calib.fast_dac_x5[i].gainCalc = 1.0;

                calib.fast_adc_filter_1_1[i].aa = aa;
                calib.fast_adc_filter_1_1[i].bb = bb;
                calib.fast_adc_filter_1_1[i].kk = kk;
                calib.fast_adc_filter_1_1[i].pp = pp;

                calib.fast_adc_filter_1_20[i].aa = aa20;
                calib.fast_adc_filter_1_20[i].bb = bb20;
                calib.fast_adc_filter_1_20[i].kk = kk20;
                calib.fast_adc_filter_1_20[i].pp = pp20;
            }
            break;

        default: {
            ERROR_LOG("Unknown model: %d.", model);
            break;
        }
    }
    return calib;
}

rp_calib_params_t convertV1toCommon(rp_calib_params_v1_t* param, bool adjust) {
    rp_calib_params_t calib;
    memset(&calib, 0, sizeof(rp_calib_params_t));
    calib.fast_adc_count_1_1 = 2;
    calib.fast_adc_count_1_20 = 2;
    calib.fast_dac_count_x1 = 2;
    calib.dataStructureId = param->dataStructureId;
    calib.wpCheck = param->wpCheck;

    calib.fast_adc_1_1[0].baseScale = 20.0;
    calib.fast_adc_1_1[0].calibValue = param->fe_ch1_fs_g_lo;
    calib.fast_adc_1_1[0].offset = param->fe_ch1_lo_offs;

    calib.fast_adc_1_1[1].baseScale = 20.0;
    calib.fast_adc_1_1[1].calibValue = param->fe_ch2_fs_g_lo;
    calib.fast_adc_1_1[1].offset = param->fe_ch2_lo_offs;

    calib.fast_adc_1_20[0].baseScale = 1.0;
    calib.fast_adc_1_20[0].calibValue = param->fe_ch1_fs_g_hi;
    calib.fast_adc_1_20[0].offset = param->fe_ch1_hi_offs;
    adjustingBaseScale(&calib.fast_adc_1_20[0], adjust);

    calib.fast_adc_1_20[1].baseScale = 1.0;
    calib.fast_adc_1_20[1].calibValue = param->fe_ch2_fs_g_hi;
    calib.fast_adc_1_20[1].offset = param->fe_ch2_hi_offs;
    adjustingBaseScale(&calib.fast_adc_1_20[1], adjust);

    calib.fast_dac_x1[0].baseScale = 1.0;
    calib.fast_dac_x1[0].calibValue = param->be_ch1_fs;
    calib.fast_dac_x1[0].offset = param->be_ch1_dc_offs;
    adjustingBaseScale(&calib.fast_dac_x1[0], adjust);

    calib.fast_dac_x1[1].baseScale = 1.0;
    calib.fast_dac_x1[1].calibValue = param->be_ch2_fs;
    calib.fast_dac_x1[1].offset = param->be_ch2_dc_offs;
    adjustingBaseScale(&calib.fast_dac_x1[1], adjust);

    // For very old boards
    if (param->magic != CALIB_MAGIC && param->magic != CALIB_MAGIC_FILTER) {
        calib.fast_adc_1_1[0].offset = param->fe_ch1_lo_offs;
        calib.fast_adc_1_1[1].offset = param->fe_ch2_lo_offs;
    }
    adjustingBaseScale(&calib.fast_adc_1_1[0], adjust);
    adjustingBaseScale(&calib.fast_adc_1_1[1], adjust);

    if (param->magic != CALIB_MAGIC_FILTER) {
        for (int i = 0; i < 2; ++i) {
            calib.fast_adc_filter_1_20[i].aa = DEFAULT_1_20_FILT_AA;
            calib.fast_adc_filter_1_20[i].bb = DEFAULT_1_20_FILT_BB;
            calib.fast_adc_filter_1_20[i].pp = DEFAULT_1_20_FILT_PP;
            calib.fast_adc_filter_1_20[i].kk = DEFAULT_1_20_FILT_KK;

            calib.fast_adc_filter_1_1[i].aa = DEFAULT_1_1_FILT_AA;
            calib.fast_adc_filter_1_1[i].bb = DEFAULT_1_1_FILT_BB;
            calib.fast_adc_filter_1_1[i].pp = DEFAULT_1_1_FILT_PP;
            calib.fast_adc_filter_1_1[i].kk = DEFAULT_1_1_FILT_KK;
        }
    } else {
        calib.fast_adc_filter_1_20[0].aa = param->hi_filter_aa_ch1;
        calib.fast_adc_filter_1_20[0].bb = param->hi_filter_bb_ch1;
        calib.fast_adc_filter_1_20[0].pp = param->hi_filter_pp_ch1;
        calib.fast_adc_filter_1_20[0].kk = param->hi_filter_kk_ch1;

        calib.fast_adc_filter_1_20[1].aa = param->hi_filter_aa_ch2;
        calib.fast_adc_filter_1_20[1].bb = param->hi_filter_bb_ch2;
        calib.fast_adc_filter_1_20[1].pp = param->hi_filter_pp_ch2;
        calib.fast_adc_filter_1_20[1].kk = param->hi_filter_kk_ch2;

        calib.fast_adc_filter_1_1[0].aa = param->low_filter_aa_ch1;
        calib.fast_adc_filter_1_1[0].bb = param->low_filter_bb_ch1;
        calib.fast_adc_filter_1_1[0].pp = param->low_filter_pp_ch1;
        calib.fast_adc_filter_1_1[0].kk = param->low_filter_kk_ch1;

        calib.fast_adc_filter_1_1[1].aa = param->low_filter_aa_ch2;
        calib.fast_adc_filter_1_1[1].bb = param->low_filter_bb_ch2;
        calib.fast_adc_filter_1_1[1].pp = param->low_filter_pp_ch2;
        calib.fast_adc_filter_1_1[1].kk = param->low_filter_kk_ch2;
    }
    return calib;
}

rp_calib_params_t convertV2toCommon(rp_calib_params_v2_t* param, bool adjust) {
    rp_calib_params_t calib;
    memset(&calib, 0, sizeof(rp_calib_params_t));
    calib.fast_adc_count_1_1 = 4;
    calib.fast_adc_count_1_20 = 4;
    calib.dataStructureId = param->dataStructureId;
    calib.wpCheck = param->wpCheck;

    calib.fast_adc_1_1[0].baseScale = 20.0;
    calib.fast_adc_1_1[0].calibValue = param->chA_g_low;
    calib.fast_adc_1_1[0].offset = param->chA_low_offs;
    adjustingBaseScale(&calib.fast_adc_1_1[0], adjust);

    calib.fast_adc_1_1[1].baseScale = 20.0;
    calib.fast_adc_1_1[1].calibValue = param->chB_g_low;
    calib.fast_adc_1_1[1].offset = param->chB_low_offs;
    adjustingBaseScale(&calib.fast_adc_1_1[1], adjust);

    calib.fast_adc_1_1[2].baseScale = 20.0;
    calib.fast_adc_1_1[2].calibValue = param->chC_g_low;
    calib.fast_adc_1_1[2].offset = param->chC_low_offs;
    adjustingBaseScale(&calib.fast_adc_1_1[2], adjust);

    calib.fast_adc_1_1[3].baseScale = 20.0;
    calib.fast_adc_1_1[3].calibValue = param->chD_g_low;
    calib.fast_adc_1_1[3].offset = param->chD_low_offs;
    adjustingBaseScale(&calib.fast_adc_1_1[3], adjust);

    calib.fast_adc_filter_1_1[0].aa = param->chA_low_aa;
    calib.fast_adc_filter_1_1[0].bb = param->chA_low_bb;
    calib.fast_adc_filter_1_1[0].pp = param->chA_low_pp;
    calib.fast_adc_filter_1_1[0].kk = param->chA_low_kk;

    calib.fast_adc_filter_1_1[1].aa = param->chB_low_aa;
    calib.fast_adc_filter_1_1[1].bb = param->chB_low_bb;
    calib.fast_adc_filter_1_1[1].pp = param->chB_low_pp;
    calib.fast_adc_filter_1_1[1].kk = param->chB_low_kk;

    calib.fast_adc_filter_1_1[2].aa = param->chC_low_aa;
    calib.fast_adc_filter_1_1[2].bb = param->chC_low_bb;
    calib.fast_adc_filter_1_1[2].pp = param->chC_low_pp;
    calib.fast_adc_filter_1_1[2].kk = param->chC_low_kk;

    calib.fast_adc_filter_1_1[3].aa = param->chD_low_aa;
    calib.fast_adc_filter_1_1[3].bb = param->chD_low_bb;
    calib.fast_adc_filter_1_1[3].pp = param->chD_low_pp;
    calib.fast_adc_filter_1_1[3].kk = param->chD_low_kk;

    calib.fast_adc_1_20[0].baseScale = 1.0;
    calib.fast_adc_1_20[0].calibValue = param->chA_g_hi;
    calib.fast_adc_1_20[0].offset = param->chA_hi_offs;
    adjustingBaseScale(&calib.fast_adc_1_20[0], adjust);

    calib.fast_adc_1_20[1].baseScale = 1.0;
    calib.fast_adc_1_20[1].calibValue = param->chB_g_hi;
    calib.fast_adc_1_20[1].offset = param->chB_hi_offs;
    adjustingBaseScale(&calib.fast_adc_1_20[1], adjust);

    calib.fast_adc_1_20[2].baseScale = 1.0;
    calib.fast_adc_1_20[2].calibValue = param->chC_g_hi;
    calib.fast_adc_1_20[2].offset = param->chC_hi_offs;
    adjustingBaseScale(&calib.fast_adc_1_20[2], adjust);

    calib.fast_adc_1_20[3].baseScale = 1.0;
    calib.fast_adc_1_20[3].calibValue = param->chD_g_hi;
    calib.fast_adc_1_20[3].offset = param->chD_hi_offs;
    adjustingBaseScale(&calib.fast_adc_1_20[3], adjust);

    calib.fast_adc_filter_1_20[0].aa = param->chA_hi_aa;
    calib.fast_adc_filter_1_20[0].bb = param->chA_hi_bb;
    calib.fast_adc_filter_1_20[0].pp = param->chA_hi_pp;
    calib.fast_adc_filter_1_20[0].kk = param->chA_hi_kk;

    calib.fast_adc_filter_1_20[1].aa = param->chB_hi_aa;
    calib.fast_adc_filter_1_20[1].bb = param->chB_hi_bb;
    calib.fast_adc_filter_1_20[1].pp = param->chB_hi_pp;
    calib.fast_adc_filter_1_20[1].kk = param->chB_hi_kk;

    calib.fast_adc_filter_1_20[2].aa = param->chC_hi_aa;
    calib.fast_adc_filter_1_20[2].bb = param->chC_hi_bb;
    calib.fast_adc_filter_1_20[2].pp = param->chC_hi_pp;
    calib.fast_adc_filter_1_20[2].kk = param->chC_hi_kk;

    calib.fast_adc_filter_1_20[3].aa = param->chD_hi_aa;
    calib.fast_adc_filter_1_20[3].bb = param->chD_hi_bb;
    calib.fast_adc_filter_1_20[3].pp = param->chD_hi_pp;
    calib.fast_adc_filter_1_20[3].kk = param->chD_hi_kk;

    return calib;
}

rp_calib_params_t convertV3toCommon(rp_calib_params_v3_t* param, bool adjust) {
    rp_calib_params_t calib;
    memset(&calib, 0, sizeof(rp_calib_params_t));

    calib.fast_adc_count_1_1 = 2;
    calib.fast_adc_count_1_1_ac = 2;
    calib.fast_adc_count_1_20 = 2;
    calib.fast_adc_count_1_20_ac = 2;
    calib.fast_dac_count_x1 = 2;
    calib.fast_dac_count_x5 = 2;
    calib.dataStructureId = param->dataStructureId;
    calib.wpCheck = param->wpCheck;

    calib.fast_adc_1_1[0].baseScale = 20.0;
    calib.fast_adc_1_1[0].calibValue = param->osc_ch1_g_1_dc;
    calib.fast_adc_1_1[0].offset = param->osc_ch1_off_1_dc;
    adjustingBaseScale(&calib.fast_adc_1_1[0], adjust);

    calib.fast_adc_1_1[1].baseScale = 20.0;
    calib.fast_adc_1_1[1].calibValue = param->osc_ch2_g_1_dc;
    calib.fast_adc_1_1[1].offset = param->osc_ch2_off_1_dc;
    adjustingBaseScale(&calib.fast_adc_1_1[1], adjust);

    calib.fast_adc_1_1_ac[0].baseScale = 20.0;
    calib.fast_adc_1_1_ac[0].calibValue = param->osc_ch1_g_1_ac;
    calib.fast_adc_1_1_ac[0].offset = param->osc_ch1_off_1_ac;
    adjustingBaseScale(&calib.fast_adc_1_1_ac[0], adjust);

    calib.fast_adc_1_1_ac[1].baseScale = 20.0;
    calib.fast_adc_1_1_ac[1].calibValue = param->osc_ch2_g_1_ac;
    calib.fast_adc_1_1_ac[1].offset = param->osc_ch2_off_1_ac;
    adjustingBaseScale(&calib.fast_adc_1_1_ac[1], adjust);

    calib.fast_adc_1_20[0].baseScale = 1.0;
    calib.fast_adc_1_20[0].calibValue = param->osc_ch1_g_20_dc;
    calib.fast_adc_1_20[0].offset = param->osc_ch1_off_20_dc;
    adjustingBaseScale(&calib.fast_adc_1_20[0], adjust);

    calib.fast_adc_1_20[1].baseScale = 1.0;
    calib.fast_adc_1_20[1].calibValue = param->osc_ch2_g_20_dc;
    calib.fast_adc_1_20[1].offset = param->osc_ch2_off_20_dc;
    adjustingBaseScale(&calib.fast_adc_1_20[1], adjust);

    calib.fast_adc_1_20_ac[0].baseScale = 1.0;
    calib.fast_adc_1_20_ac[0].calibValue = param->osc_ch1_g_20_ac;
    calib.fast_adc_1_20_ac[0].offset = param->osc_ch1_off_20_ac;
    adjustingBaseScale(&calib.fast_adc_1_20_ac[0], adjust);

    calib.fast_adc_1_20_ac[1].baseScale = 1.0;
    calib.fast_adc_1_20_ac[1].calibValue = param->osc_ch2_g_20_ac;
    calib.fast_adc_1_20_ac[1].offset = param->osc_ch2_off_20_ac;
    adjustingBaseScale(&calib.fast_adc_1_20_ac[1], adjust);

    for (int i = 0; i < 2; ++i) {
        calib.fast_adc_filter_1_20[i].aa = DEFAULT_1_20_FILT_AA;
        calib.fast_adc_filter_1_20[i].bb = DEFAULT_1_20_FILT_BB;
        calib.fast_adc_filter_1_20[i].pp = DEFAULT_1_20_FILT_PP;
        calib.fast_adc_filter_1_20[i].kk = DEFAULT_1_20_FILT_KK;

        calib.fast_adc_filter_1_1[i].aa = DEFAULT_1_1_FILT_AA;
        calib.fast_adc_filter_1_1[i].bb = DEFAULT_1_1_FILT_BB;
        calib.fast_adc_filter_1_1[i].pp = DEFAULT_1_1_FILT_PP;
        calib.fast_adc_filter_1_1[i].kk = DEFAULT_1_1_FILT_KK;
    }

    calib.fast_dac_x1[0].baseScale = 2.0;
    calib.fast_dac_x1[0].calibValue = param->gen_ch1_g_1;
    calib.fast_dac_x1[0].offset = param->gen_ch1_off_1;
    adjustingBaseScale(&calib.fast_dac_x1[0], adjust);

    calib.fast_dac_x1[1].baseScale = 2.0;
    calib.fast_dac_x1[1].calibValue = param->gen_ch2_g_1;
    calib.fast_dac_x1[1].offset = param->gen_ch2_off_1;
    adjustingBaseScale(&calib.fast_dac_x1[1], adjust);

    calib.fast_dac_x5[0].baseScale = 2.0;
    calib.fast_dac_x5[0].calibValue = param->gen_ch1_g_5;
    calib.fast_dac_x5[0].offset = param->gen_ch1_off_5;
    adjustingBaseScale(&calib.fast_dac_x5[0], adjust);

    calib.fast_dac_x5[1].baseScale = 2.0;
    calib.fast_dac_x5[1].calibValue = param->gen_ch2_g_5;
    calib.fast_dac_x5[1].offset = param->gen_ch2_off_5;
    adjustingBaseScale(&calib.fast_dac_x5[1], adjust);

    return calib;
}

rp_calib_params_t convertV4toCommon(rp_calib_params_v1_t* param, bool adjust) {
    rp_calib_params_t calib;
    memset(&calib, 0, sizeof(rp_calib_params_t));
    calib.fast_adc_count_1_1 = 2;
    calib.fast_dac_count_x1 = 2;
    calib.dataStructureId = param->dataStructureId;
    calib.wpCheck = param->wpCheck;

    calib.fast_adc_1_1[0].baseScale = 20.0;
    calib.fast_adc_1_1[0].calibValue = param->fe_ch1_fs_g_lo;
    calib.fast_adc_1_1[0].offset = param->fe_ch1_lo_offs;

    calib.fast_adc_1_1[1].baseScale = 20.0;
    calib.fast_adc_1_1[1].calibValue = param->fe_ch2_fs_g_lo;
    calib.fast_adc_1_1[1].offset = param->fe_ch2_lo_offs;

    calib.fast_dac_x1[0].baseScale = 1.0;
    calib.fast_dac_x1[0].calibValue = param->be_ch1_fs;
    calib.fast_dac_x1[0].offset = param->be_ch1_dc_offs;
    adjustingBaseScale(&calib.fast_dac_x1[0], adjust);

    calib.fast_dac_x1[1].baseScale = 1.0;
    calib.fast_dac_x1[1].calibValue = param->be_ch2_fs;
    calib.fast_dac_x1[1].offset = param->be_ch2_dc_offs;
    adjustingBaseScale(&calib.fast_dac_x1[1], adjust);

    // For very old boards
    if (param->magic != CALIB_MAGIC && param->magic != CALIB_MAGIC_FILTER) {
        calib.fast_adc_1_1[0].offset = param->fe_ch1_lo_offs;
        calib.fast_adc_1_1[1].offset = param->fe_ch2_lo_offs;
    }
    adjustingBaseScale(&calib.fast_adc_1_1[0], adjust);
    adjustingBaseScale(&calib.fast_adc_1_1[1], adjust);

    if (param->magic != CALIB_MAGIC_FILTER) {
        for (int i = 0; i < 2; ++i) {
            calib.fast_adc_filter_1_1[i].aa = DEFAULT_1_1_FILT_AA;
            calib.fast_adc_filter_1_1[i].bb = DEFAULT_1_1_FILT_BB;
            calib.fast_adc_filter_1_1[i].pp = DEFAULT_1_1_FILT_PP;
            calib.fast_adc_filter_1_1[i].kk = DEFAULT_1_1_FILT_KK;
        }
    } else {
        calib.fast_adc_filter_1_1[0].aa = param->low_filter_aa_ch1;
        calib.fast_adc_filter_1_1[0].bb = param->low_filter_bb_ch1;
        calib.fast_adc_filter_1_1[0].pp = param->low_filter_pp_ch1;
        calib.fast_adc_filter_1_1[0].kk = param->low_filter_kk_ch1;

        calib.fast_adc_filter_1_1[1].aa = param->low_filter_aa_ch2;
        calib.fast_adc_filter_1_1[1].bb = param->low_filter_bb_ch2;
        calib.fast_adc_filter_1_1[1].pp = param->low_filter_pp_ch2;
        calib.fast_adc_filter_1_1[1].kk = param->low_filter_kk_ch2;
    }
    return calib;
}

rp_calib_params_t convertGen2toCommon(rp_calib_params_v1_t* param, bool adjust) {
    rp_calib_params_t calib;
    memset(&calib, 0, sizeof(rp_calib_params_t));
    calib.fast_adc_count_1_1 = 2;
    calib.fast_adc_count_1_20 = 2;
    calib.fast_dac_count_x1 = 2;
    calib.dataStructureId = param->dataStructureId;
    calib.wpCheck = param->wpCheck;

    calib.fast_adc_1_1[0].baseScale = 20.0;
    calib.fast_adc_1_1[0].calibValue = param->fe_ch1_fs_g_lo;
    calib.fast_adc_1_1[0].offset = param->fe_ch1_lo_offs;

    calib.fast_adc_1_1[1].baseScale = 20.0;
    calib.fast_adc_1_1[1].calibValue = param->fe_ch2_fs_g_lo;
    calib.fast_adc_1_1[1].offset = param->fe_ch2_lo_offs;

    calib.fast_adc_1_20[0].baseScale = 1.0;
    calib.fast_adc_1_20[0].calibValue = param->fe_ch1_fs_g_hi;
    calib.fast_adc_1_20[0].offset = param->fe_ch1_hi_offs;
    adjustingBaseScale(&calib.fast_adc_1_20[0], adjust);

    calib.fast_adc_1_20[1].baseScale = 1.0;
    calib.fast_adc_1_20[1].calibValue = param->fe_ch2_fs_g_hi;
    calib.fast_adc_1_20[1].offset = param->fe_ch2_hi_offs;
    adjustingBaseScale(&calib.fast_adc_1_20[1], adjust);

    calib.fast_dac_x1[0].baseScale = 1.0;
    calib.fast_dac_x1[0].calibValue = param->be_ch1_fs;
    calib.fast_dac_x1[0].offset = param->be_ch1_dc_offs;
    adjustingBaseScale(&calib.fast_dac_x1[0], adjust);

    calib.fast_dac_x1[1].baseScale = 1.0;
    calib.fast_dac_x1[1].calibValue = param->be_ch2_fs;
    calib.fast_dac_x1[1].offset = param->be_ch2_dc_offs;
    adjustingBaseScale(&calib.fast_dac_x1[1], adjust);

    // For very old boards
    if (param->magic != CALIB_MAGIC && param->magic != CALIB_MAGIC_FILTER) {
        calib.fast_adc_1_1[0].offset = param->fe_ch1_lo_offs;
        calib.fast_adc_1_1[1].offset = param->fe_ch2_lo_offs;
    }
    adjustingBaseScale(&calib.fast_adc_1_1[0], adjust);
    adjustingBaseScale(&calib.fast_adc_1_1[1], adjust);

    if (param->magic != CALIB_MAGIC_FILTER) {
        for (int i = 0; i < 2; ++i) {
            calib.fast_adc_filter_1_20[i].aa = DISABLE_FILT_AA;
            calib.fast_adc_filter_1_20[i].bb = DISABLE_FILT_BB;
            calib.fast_adc_filter_1_20[i].pp = DISABLE_FILT_PP;
            calib.fast_adc_filter_1_20[i].kk = DISABLE_FILT_KK;

            calib.fast_adc_filter_1_1[i].aa = DISABLE_FILT_AA;
            calib.fast_adc_filter_1_1[i].bb = DISABLE_FILT_BB;
            calib.fast_adc_filter_1_1[i].pp = DISABLE_FILT_PP;
            calib.fast_adc_filter_1_1[i].kk = DISABLE_FILT_KK;
        }
    } else {
        calib.fast_adc_filter_1_20[0].aa = param->hi_filter_aa_ch1;
        calib.fast_adc_filter_1_20[0].bb = param->hi_filter_bb_ch1;
        calib.fast_adc_filter_1_20[0].pp = param->hi_filter_pp_ch1;
        calib.fast_adc_filter_1_20[0].kk = param->hi_filter_kk_ch1;

        calib.fast_adc_filter_1_20[1].aa = param->hi_filter_aa_ch2;
        calib.fast_adc_filter_1_20[1].bb = param->hi_filter_bb_ch2;
        calib.fast_adc_filter_1_20[1].pp = param->hi_filter_pp_ch2;
        calib.fast_adc_filter_1_20[1].kk = param->hi_filter_kk_ch2;

        calib.fast_adc_filter_1_1[0].aa = param->low_filter_aa_ch1;
        calib.fast_adc_filter_1_1[0].bb = param->low_filter_bb_ch1;
        calib.fast_adc_filter_1_1[0].pp = param->low_filter_pp_ch1;
        calib.fast_adc_filter_1_1[0].kk = param->low_filter_kk_ch1;

        calib.fast_adc_filter_1_1[1].aa = param->low_filter_aa_ch2;
        calib.fast_adc_filter_1_1[1].bb = param->low_filter_bb_ch2;
        calib.fast_adc_filter_1_1[1].pp = param->low_filter_pp_ch2;
        calib.fast_adc_filter_1_1[1].kk = param->low_filter_kk_ch2;
    }
    return calib;
}

bool recalculateGain(rp_calib_params_t* param) {

    if (param->fast_adc_count_1_1 > 4)
        return false;
    for (int i = 0; i < param->fast_adc_count_1_1; ++i) {
        double baseValue = calibBaseScaleFromVoltage(param->fast_adc_1_1[i].baseScale, isUniversalCalib(param->dataStructureId));
        param->fast_adc_1_1[i].gainCalc = (double)param->fast_adc_1_1[i].calibValue / baseValue;
        param->fast_adc_1_1[i].gainCalc = CHECK_GAIN_LIMIT(param->fast_adc_1_1[i].gainCalc);
        if (!CHECK_VALID_GAIN_LIMIT(param->fast_adc_1_1[i].gainCalc)) {
            ERROR_LOG("Invalid gain fast_adc_1_1[%d] = %f", i, param->fast_adc_1_1[i].gainCalc);
            return false;
        }
    }

    if (param->fast_adc_count_1_20 > 4)
        return false;
    for (int i = 0; i < param->fast_adc_count_1_20; ++i) {
        double baseValue = calibBaseScaleFromVoltage(param->fast_adc_1_20[i].baseScale, isUniversalCalib(param->dataStructureId));
        param->fast_adc_1_20[i].gainCalc = (double)param->fast_adc_1_20[i].calibValue / baseValue;
        param->fast_adc_1_20[i].gainCalc = CHECK_GAIN_LIMIT(param->fast_adc_1_20[i].gainCalc);
        if (!CHECK_VALID_GAIN_LIMIT(param->fast_adc_1_20[i].gainCalc)) {
            ERROR_LOG("Invalid gain fast_adc_1_20[%d] = %f", i, param->fast_adc_1_20[i].gainCalc);
            return false;
        }
    }

    if (param->fast_adc_count_1_1_ac > 4)
        return false;
    for (int i = 0; i < param->fast_adc_count_1_1_ac; ++i) {
        double baseValue = calibBaseScaleFromVoltage(param->fast_adc_1_1_ac[i].baseScale, isUniversalCalib(param->dataStructureId));
        param->fast_adc_1_1_ac[i].gainCalc = (double)param->fast_adc_1_1_ac[i].calibValue / baseValue;
        param->fast_adc_1_1_ac[i].gainCalc = CHECK_GAIN_LIMIT(param->fast_adc_1_1_ac[i].gainCalc);
        if (!CHECK_VALID_GAIN_LIMIT(param->fast_adc_1_1_ac[i].gainCalc)) {
            ERROR_LOG("Invalid gain fast_adc_1_1_ac[%d] = %f", i, param->fast_adc_1_1_ac[i].gainCalc);
            return false;
        }
    }

    if (param->fast_adc_count_1_20_ac > 4)
        return false;
    for (int i = 0; i < param->fast_adc_count_1_20_ac; ++i) {
        double baseValue = calibBaseScaleFromVoltage(param->fast_adc_1_20_ac[i].baseScale, isUniversalCalib(param->dataStructureId));
        param->fast_adc_1_20_ac[i].gainCalc = (double)param->fast_adc_1_20_ac[i].calibValue / baseValue;
        param->fast_adc_1_20_ac[i].gainCalc = CHECK_GAIN_LIMIT(param->fast_adc_1_20_ac[i].gainCalc);
        if (!CHECK_VALID_GAIN_LIMIT(param->fast_adc_1_20_ac[i].gainCalc)) {
            ERROR_LOG("Invalid gain fast_adc_1_20_ac[%d] = %f", i, param->fast_adc_1_20_ac[i].gainCalc);
            return false;
        }
    }

    if (param->fast_dac_count_x1 > 2)
        return false;
    for (int i = 0; i < param->fast_dac_count_x1; ++i) {
        double baseValue = calibBaseScaleFromVoltage(param->fast_dac_x1[i].baseScale, isUniversalCalib(param->dataStructureId));
        param->fast_dac_x1[i].gainCalc = (double)param->fast_dac_x1[i].calibValue / baseValue;
        param->fast_dac_x1[i].gainCalc = CHECK_GAIN_LIMIT(param->fast_dac_x1[i].gainCalc);
        if (!CHECK_VALID_GAIN_LIMIT(param->fast_dac_x1[i].gainCalc)) {
            ERROR_LOG("Invalid gain fast_dac_x1[%d] = %f", i, param->fast_dac_x1[i].gainCalc);
            return false;
        }
    }

    if (param->fast_dac_count_x5 > 2)
        return false;
    for (int i = 0; i < param->fast_dac_count_x5; ++i) {
        double baseValue = calibBaseScaleFromVoltage(param->fast_dac_x5[i].baseScale, isUniversalCalib(param->dataStructureId));
        param->fast_dac_x5[i].gainCalc = (double)param->fast_dac_x5[i].calibValue / baseValue;
        param->fast_dac_x5[i].gainCalc = CHECK_GAIN_LIMIT(param->fast_dac_x5[i].gainCalc);
        if (!CHECK_VALID_GAIN_LIMIT(param->fast_dac_x5[i].gainCalc)) {
            ERROR_LOG("Invalid gain fast_dac_x5[%d = %f", i, param->fast_dac_x5[i].gainCalc);
            return false;
        }
    }

    return true;
}

bool recalculateCalibValue(rp_calib_params_t* param) {

    if (param->fast_adc_count_1_1 > 4)
        return false;
    for (int i = 0; i < param->fast_adc_count_1_1; ++i) {
        double baseValue = calibBaseScaleFromVoltage(param->fast_adc_1_1[i].baseScale, isUniversalCalib(param->dataStructureId));
        param->fast_adc_1_1[i].calibValue = CHECK_GAIN_LIMIT(param->fast_adc_1_1[i].gainCalc) * (double)baseValue;
        if (!CHECK_VALID_GAIN_LIMIT(param->fast_adc_1_1[i].gainCalc)) {
            ERROR_LOG("Invalid gain fast_adc_1_1[%d] = %f", i, param->fast_adc_1_1[i].gainCalc);
            return false;
        }
    }

    if (param->fast_adc_count_1_20 > 4)
        return false;
    for (int i = 0; i < param->fast_adc_count_1_20; ++i) {
        double baseValue = calibBaseScaleFromVoltage(param->fast_adc_1_20[i].baseScale, isUniversalCalib(param->dataStructureId));
        param->fast_adc_1_20[i].calibValue = CHECK_GAIN_LIMIT(param->fast_adc_1_20[i].gainCalc) * (double)baseValue;
        if (!CHECK_VALID_GAIN_LIMIT(param->fast_adc_1_20[i].gainCalc)) {
            ERROR_LOG("Invalid gain fast_adc_1_20[%d] = %f", i, param->fast_adc_1_20[i].gainCalc);
            return false;
        }
    }

    if (param->fast_adc_count_1_1_ac > 4)
        return false;
    for (int i = 0; i < param->fast_adc_count_1_1_ac; ++i) {
        double baseValue = calibBaseScaleFromVoltage(param->fast_adc_1_1_ac[i].baseScale, isUniversalCalib(param->dataStructureId));
        param->fast_adc_1_1_ac[i].calibValue = CHECK_GAIN_LIMIT(param->fast_adc_1_1_ac[i].gainCalc) * (double)baseValue;
        if (!CHECK_VALID_GAIN_LIMIT(param->fast_adc_1_1_ac[i].gainCalc)) {
            ERROR_LOG("Invalid gain fast_adc_1_1_ac[%d] = %f", i, param->fast_adc_1_1_ac[i].gainCalc);
            return false;
        }
    }

    if (param->fast_adc_count_1_20_ac > 4)
        return false;
    for (int i = 0; i < param->fast_adc_count_1_20_ac; ++i) {
        double baseValue = calibBaseScaleFromVoltage(param->fast_adc_1_20_ac[i].baseScale, isUniversalCalib(param->dataStructureId));
        param->fast_adc_1_20_ac[i].calibValue = CHECK_GAIN_LIMIT(param->fast_adc_1_20_ac[i].gainCalc) * (double)baseValue;
        if (!CHECK_VALID_GAIN_LIMIT(param->fast_adc_1_20_ac[i].gainCalc)) {
            ERROR_LOG("Invalid gain fast_adc_1_20_ac[%d] = %f", i, param->fast_adc_1_20_ac[i].gainCalc);
            return false;
        }
    }

    if (param->fast_dac_count_x1 > 4)
        return false;
    for (int i = 0; i < param->fast_dac_count_x1; ++i) {
        double baseValue = calibBaseScaleFromVoltage(param->fast_dac_x1[i].baseScale, isUniversalCalib(param->dataStructureId));
        param->fast_dac_x1[i].calibValue = CHECK_GAIN_LIMIT(param->fast_dac_x1[i].gainCalc) * (double)baseValue;
        if (!CHECK_VALID_GAIN_LIMIT(param->fast_dac_x1[i].gainCalc)) {
            ERROR_LOG("Invalid gain fast_dac_x1[%d] = %f", i, param->fast_dac_x1[i].gainCalc);
            return false;
        }
    }

    if (param->fast_dac_count_x5 > 4)
        return false;
    for (int i = 0; i < param->fast_dac_count_x5; ++i) {
        double baseValue = calibBaseScaleFromVoltage(param->fast_dac_x5[i].baseScale, isUniversalCalib(param->dataStructureId));
        param->fast_dac_x5[i].calibValue = CHECK_GAIN_LIMIT(param->fast_dac_x5[i].gainCalc) * (double)baseValue;
        if (!CHECK_VALID_GAIN_LIMIT(param->fast_dac_x5[i].gainCalc)) {
            ERROR_LOG("Invalid gain fast_dac_x5[%d] = %f", i, param->fast_dac_x5[i].gainCalc);
            return false;
        }
    }

    return true;
}

rp_calib_error adjustingBaseScaleEx(float* baseScale, int32_t* offset, bool adjust, uint32_t* calibValue) {
    if (!adjust)
        return RP_HW_CALIB_OK;

    double curBsValue = calibBaseScaleFromVoltage(*baseScale, false);
    if (CHECK_VALID_GAIN_LIMIT(*calibValue / curBsValue)) {
        return RP_HW_CALIB_OK;
    }

    float coff[4] = {0.5, 1.0, 2.0, 20.0};
    float detectedCoff = -1;
    for (int i = 0; i < 4; i++) {
        double bvCoff = calibBaseScaleFromVoltage(coff[i], false);
        if (CHECK_VALID_GAIN_LIMIT(*calibValue / bvCoff)) {
            detectedCoff = coff[i];
        }
    }

    if (detectedCoff == -1) {
        ERROR_LOG("Base scale recognition error for value %d", *calibValue);
        return RP_HW_CALIB_EA;
    }

    float x = *baseScale / detectedCoff;
    *calibValue = *calibValue * x;
    //    *offset = *offset * x;
    WARNING("Calibration corrected from %f to %f. New calib gain value %d and offset %d", *baseScale, detectedCoff, *calibValue, *offset);
    return RP_HW_CALIB_OK;
}

rp_calib_error adjustingBaseScale(channel_calib_t* calib, bool adjust) {
    return adjustingBaseScaleEx(&calib->baseScale, &calib->offset, adjust, &calib->calibValue);
}

bool isUniversalCalib(uint16_t dataStructureId) {
    if (dataStructureId == RP_HW_PACK_ID_V5)
        return true;
    if (dataStructureId == RP_HW_PACK_ID_V6)
        return true;
    return false;
}
