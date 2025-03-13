/**
* $Id: $
*
* @brief Red Pitaya library housekeeping
*
* @Author Red Pitaya
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#include "housekeeping.h"

volatile housekeeping_control_t* hk = NULL;

hk_version_t house_getHKVersion() {
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get board model");
    }

    switch (c) {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
        case STEM_125_14_v2_0:
        case STEM_125_14_Pro_v2_0:
        case STEM_125_14_Z7020_Pro_v1_0:
        case STEM_125_14_Z7020_Pro_v2_0:
        case STEM_125_14_Z7020_Ind_v2_0:
            return HK_V1;

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return HK_V1;

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            return HK_V2;

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
        case STEM_250_12_120:
            return HK_V3;

        case STEM_125_14_Z7020_LL_v1_1:
        case STEM_65_16_Z7020_LL_v1_1:
            return HK_V4;
        default:
            ERROR_LOG("Can't get board model");
    }
    return HK_V1;
}

int hk_Init() {
    int ret = cmn_Map(HOUSEKEEPING_BASE_SIZE, HOUSEKEEPING_BASE_ADDR, (void**)&hk);
    return ret;
}

int hk_Reset() {
    if (rp_HPGetIsPLLControlEnableOrDefault()) {
        return house_SetPllControlEnable(false);
    }
    return RP_OK;
}

int hk_Release() {
    cmn_Unmap(HOUSEKEEPING_BASE_SIZE, (void**)&hk);
    hk = NULL;
    return RP_OK;
}

volatile housekeeping_control_t* hk_GetHK() {
    return hk;
}

int hk_printRegset() {
    volatile housekeeping_control_t* hk = NULL;
    int fd = -1;

    int ret = cmn_InitMap(HOUSEKEEPING_BASE_SIZE, HOUSEKEEPING_BASE_ADDR, (void**)&hk, &fd);
    if (ret != RP_OK) {
        return ret;
    }

    hk_version_t ver = house_getHKVersion();
    switch (ver) {
        case HK_V1: {
            volatile housekeeping_control_v1_t* hk_v1 = (volatile housekeeping_control_v1_t*)hk;
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "ID", offsetof(housekeeping_control_v1_t, id), hk_v1->id);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "DNA LOW", offsetof(housekeeping_control_v1_t, dna_lo), hk_v1->dna_lo);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "DNA HIGH", offsetof(housekeeping_control_v1_t, dna_hi), hk_v1->dna_hi);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Digital Loopback", offsetof(housekeeping_control_v1_t, digital_loop), hk_v1->digital_loop);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Direction P", offsetof(housekeeping_control_v1_t, ex_cd_p), hk_v1->ex_cd_p);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Direction N", offsetof(housekeeping_control_v1_t, ex_cd_n), hk_v1->ex_cd_n);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Output P", offsetof(housekeeping_control_v1_t, ex_co_p), hk_v1->ex_co_p);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Output N", offsetof(housekeeping_control_v1_t, ex_co_n), hk_v1->ex_co_n);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Input P", offsetof(housekeeping_control_v1_t, ex_ci_p), hk_v1->ex_ci_p);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Input N", offsetof(housekeeping_control_v1_t, ex_ci_n), hk_v1->ex_ci_n);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Reserved 2", offsetof(housekeeping_control_v1_t, reserved_2), hk_v1->reserved_2);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Reserved 3", offsetof(housekeeping_control_v1_t, reserved_3), hk_v1->reserved_3);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "LED control", offsetof(housekeeping_control_v1_t, led_control), hk_v1->led_control);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "CAN control", offsetof(housekeeping_control_v1_t, can_control), hk_v1->can_control.reg_full);
            hk_v1->can_control.reg.print();

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "FPGA ready", offsetof(housekeeping_control_v1_t, fpga_ready), hk_v1->fpga_ready);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "ADC clock frequency meter", offsetof(housekeeping_control_v1_t, acq_clock_counter),
                     hk_v1->acq_clock_counter);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "External trigger", offsetof(housekeeping_control_v1_t, ext_trigger), hk_v1->ext_trigger.reg_full);
            hk_v1->ext_trigger.reg.print();

            break;
        }
        case HK_V2: {
            volatile housekeeping_control_v2_t* hk_v2 = (volatile housekeeping_control_v2_t*)hk;
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "ID", offsetof(housekeeping_control_v2_t, id), hk_v2->id);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "DNA LOW", offsetof(housekeeping_control_v2_t, dna_lo), hk_v2->dna_lo);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "DNA HIGH", offsetof(housekeeping_control_v2_t, dna_hi), hk_v2->dna_hi);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Digital Loopback", offsetof(housekeeping_control_v2_t, digital_loop), hk_v2->digital_loop);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Direction P", offsetof(housekeeping_control_v2_t, ex_cd_p), hk_v2->ex_cd_p);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Direction N", offsetof(housekeeping_control_v2_t, ex_cd_n), hk_v2->ex_cd_n);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Output P", offsetof(housekeeping_control_v2_t, ex_co_p), hk_v2->ex_co_p);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Output N", offsetof(housekeeping_control_v2_t, ex_co_n), hk_v2->ex_co_n);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Input P", offsetof(housekeeping_control_v2_t, ex_ci_p), hk_v2->ex_ci_p);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Input N", offsetof(housekeeping_control_v2_t, ex_ci_n), hk_v2->ex_ci_n);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Reserved 2", offsetof(housekeeping_control_v2_t, reserved_2), hk_v2->reserved_2);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Reserved 3", offsetof(housekeeping_control_v2_t, reserved_3), hk_v2->reserved_3);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "LED control", offsetof(housekeeping_control_v2_t, led_control), hk_v2->led_control);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "CAN control", offsetof(housekeeping_control_v2_t, can_control), hk_v2->can_control.reg_full);
            hk_v2->can_control.reg.print();

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "PLL control", offsetof(housekeeping_control_v2_t, pll_control), hk_v2->pll_control.reg_full);
            hk_v2->pll_control.reg.print();

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "IDELAY reset", offsetof(housekeeping_control_v2_t, idelay_reset), hk_v2->idelay_reset);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "IDELAY CHA", offsetof(housekeeping_control_v2_t, idelay_cha), hk_v2->idelay_cha);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "IDELAY CHB", offsetof(housekeeping_control_v2_t, idelay_chb), hk_v2->idelay_chb);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "IDELAY CHC", offsetof(housekeeping_control_v2_t, idelay_chc), hk_v2->idelay_chc);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "IDELAY CHD", offsetof(housekeeping_control_v2_t, idelay_chd), hk_v2->idelay_chd);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "FPGA ready", offsetof(housekeeping_control_v2_t, fpga_ready), hk_v2->fpga_ready);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "ADC clock frequency meter", offsetof(housekeeping_control_v2_t, acq_clock_counter),
                     hk_v2->acq_clock_counter);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "External trigger", offsetof(housekeeping_control_v2_t, ext_trigger), hk_v2->ext_trigger.reg_full);
            hk_v2->ext_trigger.reg.print();

            break;
        }
        case HK_V3: {
            volatile housekeeping_control_v3_t* hk_v3 = (volatile housekeeping_control_v3_t*)hk;
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "ID", offsetof(housekeeping_control_v3_t, id), hk_v3->id);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "DNA LOW", offsetof(housekeeping_control_v3_t, dna_lo), hk_v3->dna_lo);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "DNA HIGH", offsetof(housekeeping_control_v3_t, dna_hi), hk_v3->dna_hi);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Digital Loopback", offsetof(housekeeping_control_v3_t, digital_loop), hk_v3->digital_loop);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Direction P", offsetof(housekeeping_control_v3_t, ex_cd_p), hk_v3->ex_cd_p);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Direction N", offsetof(housekeeping_control_v3_t, ex_cd_n), hk_v3->ex_cd_n);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Output P", offsetof(housekeeping_control_v3_t, ex_co_p), hk_v3->ex_co_p);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Output N", offsetof(housekeeping_control_v3_t, ex_co_n), hk_v3->ex_co_n);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Input P", offsetof(housekeeping_control_v3_t, ex_ci_p), hk_v3->ex_ci_p);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Input N", offsetof(housekeeping_control_v3_t, ex_ci_n), hk_v3->ex_ci_n);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Reserved 2", offsetof(housekeeping_control_v3_t, reserved_2), hk_v3->reserved_2);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Reserved 3", offsetof(housekeeping_control_v3_t, reserved_3), hk_v3->reserved_3);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "LED control", offsetof(housekeeping_control_v3_t, led_control), hk_v3->led_control);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "CAN control", offsetof(housekeeping_control_v3_t, can_control), hk_v3->can_control.reg_full);
            hk_v3->can_control.reg.print();

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "PLL control", offsetof(housekeeping_control_v3_t, pll_control), hk_v3->pll_control.reg_full);
            hk_v3->pll_control.reg.print();

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "IDELAY reset", offsetof(housekeeping_control_v3_t, idelay_reset), hk_v3->idelay_reset);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "IDELAY CHA", offsetof(housekeeping_control_v3_t, idelay_cha), hk_v3->idelay_cha);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "IDELAY CHB", offsetof(housekeeping_control_v3_t, idelay_chb), hk_v3->idelay_chb);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "ADC SPI Control word", offsetof(housekeeping_control_v3_t, adc_spi_cw), hk_v3->adc_spi_cw);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "ADC SPI Write data / start transfer", offsetof(housekeeping_control_v3_t, adc_spi_wd), hk_v3->adc_spi_wd);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "ADC SPI Read data / Transfer busy", offsetof(housekeeping_control_v3_t, adc_spi_rd), hk_v3->adc_spi_rd);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "DAC SPI Control word", offsetof(housekeeping_control_v3_t, dac_spi_cw), hk_v3->dac_spi_cw);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "DAC SPI Write data / start transfer", offsetof(housekeeping_control_v3_t, dac_spi_wd), hk_v3->dac_spi_wd);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "DAC SPI Read data / Transfer busy", offsetof(housekeeping_control_v3_t, dac_spi_rd), hk_v3->dac_spi_rd);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "FPGA ready", offsetof(housekeeping_control_v3_t, fpga_ready), hk_v3->fpga_ready);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "ADC clock frequency meter", offsetof(housekeeping_control_v3_t, acq_clock_counter),
                     hk_v3->acq_clock_counter);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "External trigger", offsetof(housekeeping_control_v3_t, ext_trigger), hk_v3->ext_trigger.reg_full);
            hk_v3->ext_trigger.reg.print();

            break;
        }
        case HK_V4: {
            volatile housekeeping_control_v4_t* hk_v4 = (volatile housekeeping_control_v4_t*)hk;
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "ID", offsetof(housekeeping_control_v4_t, id), hk_v4->id);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "DNA LOW", offsetof(housekeeping_control_v4_t, dna_lo), hk_v4->dna_lo);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "DNA HIGH", offsetof(housekeeping_control_v4_t, dna_hi), hk_v4->dna_hi);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Digital Loopback", offsetof(housekeeping_control_v4_t, digital_loop), hk_v4->digital_loop);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Direction P", offsetof(housekeeping_control_v4_t, ex_cd_p), hk_v4->ex_cd_p);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Direction N", offsetof(housekeeping_control_v4_t, ex_cd_n), hk_v4->ex_cd_n);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Output P", offsetof(housekeeping_control_v4_t, ex_co_p), hk_v4->ex_co_p);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Output N", offsetof(housekeeping_control_v4_t, ex_co_n), hk_v4->ex_co_n);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Input P", offsetof(housekeeping_control_v4_t, ex_ci_p), hk_v4->ex_ci_p);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Input N", offsetof(housekeeping_control_v4_t, ex_ci_n), hk_v4->ex_ci_n);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Reserved 2", offsetof(housekeeping_control_v4_t, reserved_2), hk_v4->reserved_2);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Reserved 3", offsetof(housekeeping_control_v4_t, reserved_3), hk_v4->reserved_3);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "LED control", offsetof(housekeeping_control_v4_t, led_control), hk_v4->led_control);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "CAN control", offsetof(housekeeping_control_v4_t, can_control), hk_v4->can_control.reg_full);
            hk_v4->can_control.reg.print();

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "IDELAY reset", offsetof(housekeeping_control_v4_t, idelay_control), hk_v4->idelay_control);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "ADC SPI Control word", offsetof(housekeeping_control_v4_t, adc_spi_cw), hk_v4->adc_spi_cw);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "ADC SPI Write data / start transfer", offsetof(housekeeping_control_v4_t, adc_spi_wd), hk_v4->adc_spi_wd);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "ADC SPI Read data / Transfer busy", offsetof(housekeeping_control_v4_t, adc_spi_rd), hk_v4->adc_spi_rd);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "FPGA ready", offsetof(housekeeping_control_v4_t, fpga_ready), hk_v4->fpga_ready);
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "ADC clock frequency meter", offsetof(housekeeping_control_v4_t, acq_clock_counter),
                     hk_v4->acq_clock_counter);

            break;
        }
        default:
            return RP_NOTS;
    }
    return cmn_ReleaseClose(fd, HOUSEKEEPING_BASE_SIZE, (void**)&hk);
}

int house_GetPllControlEnable(bool* enable) {
    if (rp_HPGetIsPLLControlEnableOrDefault()) {
        hk_version_t ver = house_getHKVersion();
        switch (ver) {
            case HK_V2: {
                volatile housekeeping_control_v2_t* hk_v2 = (volatile housekeeping_control_v2_t*)hk;
                *enable = hk_v2->pll_control.reg.enable;
                return RP_OK;
            }
            case HK_V3: {
                volatile housekeeping_control_v3_t* hk_v3 = (volatile housekeeping_control_v3_t*)hk;
                *enable = hk_v3->pll_control.reg.enable;
                return RP_OK;
            }
            default:
                return RP_NOTS;
        }
    }
    return RP_NOTS;
}

int house_SetPllControlEnable(bool enable) {
    if (rp_HPGetIsPLLControlEnableOrDefault()) {
        hk_version_t ver = house_getHKVersion();
        switch (ver) {
            case HK_V2: {
                volatile housekeeping_control_v2_t* hk_v2 = (volatile housekeeping_control_v2_t*)hk;
                hk_v2->pll_control.reg.enable = enable;
                return RP_OK;
            }
            case HK_V3: {
                volatile housekeeping_control_v3_t* hk_v3 = (volatile housekeeping_control_v3_t*)hk;
                hk_v3->pll_control.reg.enable = enable;
                return RP_OK;
            }
            default:
                return RP_NOTS;
        }
    }
    return RP_NOTS;
}

int house_GetPllControlLocked(bool* status) {
    if (rp_HPGetIsPLLControlEnableOrDefault()) {
        hk_version_t ver = house_getHKVersion();
        switch (ver) {
            case HK_V2: {
                volatile housekeeping_control_v2_t* hk_v2 = (volatile housekeeping_control_v2_t*)hk;
                *status = hk_v2->pll_control.reg.locked;
                return RP_OK;
            }
            case HK_V3: {
                volatile housekeeping_control_v3_t* hk_v3 = (volatile housekeeping_control_v3_t*)hk;
                *status = hk_v3->pll_control.reg.locked;
                return RP_OK;
            }
            default:
                return RP_NOTS;
        }
    }
    return RP_NOTS;
}

int house_SetEnableDaisyChainSync(bool enable) {
    hk_version_t ver = house_getHKVersion();
    switch (ver) {
        case HK_V1: {
            volatile housekeeping_control_v1_t* hk_v1 = (volatile housekeeping_control_v1_t*)hk;
            hk_v1->ext_trigger.reg.enable = enable;
            return RP_OK;
        }
        case HK_V2: {
            volatile housekeeping_control_v2_t* hk_v2 = (volatile housekeeping_control_v2_t*)hk;
            hk_v2->ext_trigger.reg.enable = enable;
            return RP_OK;
        }
        case HK_V3: {
            volatile housekeeping_control_v3_t* hk_v3 = (volatile housekeeping_control_v3_t*)hk;
            hk_v3->ext_trigger.reg.enable = enable;
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
}

int house_GetEnableDaisyChainSync(bool* status) {
    hk_version_t ver = house_getHKVersion();
    switch (ver) {
        case HK_V1: {
            volatile housekeeping_control_v1_t* hk_v1 = (volatile housekeeping_control_v1_t*)hk;
            *status = hk_v1->ext_trigger.reg.enable;
            return RP_OK;
        }
        case HK_V2: {
            volatile housekeeping_control_v2_t* hk_v2 = (volatile housekeeping_control_v2_t*)hk;
            *status = hk_v2->ext_trigger.reg.enable;
            return RP_OK;
        }
        case HK_V3: {
            volatile housekeeping_control_v3_t* hk_v3 = (volatile housekeeping_control_v3_t*)hk;
            *status = hk_v3->ext_trigger.reg.enable;
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
}

int house_SetDpinEnableTrigOutput(bool enable) {
    hk_version_t ver = house_getHKVersion();
    switch (ver) {
        case HK_V1: {
            volatile housekeeping_control_v1_t* hk_v1 = (volatile housekeeping_control_v1_t*)hk;
            hk_v1->ext_trigger.reg.gpio_adc_dac = enable;
            return RP_OK;
        }
        case HK_V2: {
            volatile housekeeping_control_v2_t* hk_v2 = (volatile housekeeping_control_v2_t*)hk;
            hk_v2->ext_trigger.reg.gpio_adc_dac = enable;
            return RP_OK;
        }
        case HK_V3: {
            volatile housekeeping_control_v3_t* hk_v3 = (volatile housekeeping_control_v3_t*)hk;
            hk_v3->ext_trigger.reg.gpio_adc_dac = enable;
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
}

int house_GetDpinEnableTrigOutput(bool* enable) {
    hk_version_t ver = house_getHKVersion();
    switch (ver) {
        case HK_V1: {
            volatile housekeeping_control_v1_t* hk_v1 = (volatile housekeeping_control_v1_t*)hk;
            *enable = hk_v1->ext_trigger.reg.gpio_adc_dac;
            return RP_OK;
        }
        case HK_V2: {
            volatile housekeeping_control_v2_t* hk_v2 = (volatile housekeeping_control_v2_t*)hk;
            *enable = hk_v2->ext_trigger.reg.gpio_adc_dac;
            return RP_OK;
        }
        case HK_V3: {
            volatile housekeeping_control_v3_t* hk_v3 = (volatile housekeeping_control_v3_t*)hk;
            *enable = hk_v3->ext_trigger.reg.gpio_adc_dac;
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
}

int house_SetSourceTrigOutput(rp_outTiggerMode_t mode) {
    hk_version_t ver = house_getHKVersion();
    switch (ver) {
        case HK_V1: {
            volatile housekeeping_control_v1_t* hk_v1 = (volatile housekeeping_control_v1_t*)hk;
            hk_v1->ext_trigger.reg.out_selector = mode;
            return RP_OK;
        }
        case HK_V2: {
            volatile housekeeping_control_v2_t* hk_v2 = (volatile housekeeping_control_v2_t*)hk;
            hk_v2->ext_trigger.reg.out_selector = mode;
            return RP_OK;
        }
        case HK_V3: {
            volatile housekeeping_control_v3_t* hk_v3 = (volatile housekeeping_control_v3_t*)hk;
            hk_v3->ext_trigger.reg.out_selector = mode;
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
}

int house_GetSourceTrigOutput(rp_outTiggerMode_t* mode) {
    hk_version_t ver = house_getHKVersion();
    switch (ver) {
        case HK_V1: {
            volatile housekeeping_control_v1_t* hk_v1 = (volatile housekeeping_control_v1_t*)hk;
            *mode = (rp_outTiggerMode_t)hk_v1->ext_trigger.reg.out_selector;
            return RP_OK;
        }
        case HK_V2: {
            volatile housekeeping_control_v2_t* hk_v2 = (volatile housekeeping_control_v2_t*)hk;
            *mode = (rp_outTiggerMode_t)hk_v2->ext_trigger.reg.out_selector;
            return RP_OK;
        }
        case HK_V3: {
            volatile housekeeping_control_v3_t* hk_v3 = (volatile housekeeping_control_v3_t*)hk;
            *mode = (rp_outTiggerMode_t)hk_v3->ext_trigger.reg.out_selector;
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
}

int house_SetCANModeEnable(bool _enable) {
    hk_version_t ver = house_getHKVersion();
    switch (ver) {
        case HK_V1: {
            volatile housekeeping_control_v1_t* hk_v1 = (volatile housekeeping_control_v1_t*)hk;
            hk_v1->can_control.reg.enable = _enable;
            return RP_OK;
        }
        case HK_V2: {
            volatile housekeeping_control_v2_t* hk_v2 = (volatile housekeeping_control_v2_t*)hk;
            hk_v2->can_control.reg.enable = _enable;
            return RP_OK;
        }
        case HK_V3: {
            volatile housekeeping_control_v3_t* hk_v3 = (volatile housekeeping_control_v3_t*)hk;
            hk_v3->can_control.reg.enable = _enable;
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
}

int house_GetCANModeEnable(bool* _enable) {
    hk_version_t ver = house_getHKVersion();
    switch (ver) {
        case HK_V1: {
            volatile housekeeping_control_v1_t* hk_v1 = (volatile housekeeping_control_v1_t*)hk;
            *_enable = hk_v1->can_control.reg.enable;
            return RP_OK;
        }
        case HK_V2: {
            volatile housekeeping_control_v2_t* hk_v2 = (volatile housekeeping_control_v2_t*)hk;
            *_enable = hk_v2->can_control.reg.enable;
            return RP_OK;
        }
        case HK_V3: {
            volatile housekeeping_control_v3_t* hk_v3 = (volatile housekeeping_control_v3_t*)hk;
            *_enable = hk_v3->can_control.reg.enable;
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
}