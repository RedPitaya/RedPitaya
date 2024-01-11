/**
 * $Id: $
 *
 * @brief Red Pitaya library housekeeping module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */



#ifndef __HOUSEKEEPING_H
#define __HOUSEKEEPING_H

#include <stdint.h>
#include <stdbool.h>
#include "redpitaya/rp.h"
#include "rp_hw-profiles.h"

// Base Housekeeping address
static const int HOUSEKEEPING_BASE_ADDR = 0x00000000;
static const int HOUSEKEEPING_BASE_SIZE = 0x2000;

// Housekeeping structure declaration
typedef struct pll_control_s {
    uint32_t enable: 1, : 3, refDetected: 1, : 3, locked: 1,:23;
} pll_control_t;

typedef struct ext_trigger_s {
    uint32_t enable: 1;
    uint32_t gpio_adc_dac : 1;
    uint32_t out_selector: 1;
    uint32_t :29;
} ext_trigger_t;

typedef struct can_control_s {
    uint32_t enable: 1;
    uint32_t :31;
} can_control_t;

typedef enum {
    HK_V1,
    HK_V2,
    HK_V3
} hk_version_t;

typedef struct housekeeping_control_s {
    uint32_t data[2048];
} housekeeping_control_t;

// Classic board
typedef struct housekeeping_control_s_v1 {
    uint32_t id;                            // 0x0
    uint32_t dna_lo;                        // 0x4 **DNA part 1**
    uint32_t dna_hi;                        // 0x8 **DNA part 2**
    uint32_t digital_loop;                  // 0xC **Digital Loopback**
    uint32_t ex_cd_p;                       // 0x10 **Expansion connector direction P**
    uint32_t ex_cd_n;                       // 0x14 **Expansion connector direction N**
    uint32_t ex_co_p;                       // 0x18 **Expansion connector output P**
    uint32_t ex_co_n;                       // 0x1c **Expansion connector output N**
    uint32_t ex_ci_p;                       // 0x20 **Expansion connector input P**
    uint32_t ex_ci_n;                       // 0x24 **Expansion connector input N**
    uint32_t reserved_2;                    // 0x28
    uint32_t reserved_3;                    // 0x2C
    uint32_t led_control;                   // 0x30 **LED control**
    can_control_t can_control;              // 0x34 **CAN control**
    uint32_t reserved_4[50];                // 0x38 - 0x100
    uint32_t fpga_ready;                    // 0x100 **FPGA ready**
    uint32_t reserved_5[959];               // 0x104 - 0x1000
    ext_trigger_t ext_trigger;              // 0x1000 **External trigger override**
} housekeeping_control_v1_t;

// For 125-14-4ch board
typedef struct housekeeping_control_s_v2 {
    uint32_t id;                            // 0x0
    uint32_t dna_lo;                        // 0x4 **DNA part 1**
    uint32_t dna_hi;                        // 0x8 **DNA part 2**
    uint32_t digital_loop;                  // 0xC **Digital Loopback**
    uint32_t ex_cd_p;                       // 0x10 **Expansion connector direction P**
    uint32_t ex_cd_n;                       // 0x14 **Expansion connector direction N**
    uint32_t ex_co_p;                       // 0x18 **Expansion connector output P**
    uint32_t ex_co_n;                       // 0x1c **Expansion connector output N**
    uint32_t ex_ci_p;                       // 0x20 **Expansion connector input P**
    uint32_t ex_ci_n;                       // 0x24 **Expansion connector input N**
    uint32_t reserved_2;                    // 0x28
    uint32_t reserved_3;                    // 0x2C
    uint32_t led_control;                   // 0x30 **LED control**
    can_control_t can_control;              // 0x34 **CAN control**
    uint32_t reserved_4;                    // 0x38
    uint32_t reserved_5;                    // 0x3C
    pll_control_t pll_control;              // 0x40
    uint32_t idelay_reset;                  // 0x44 **IDELAY reset**
    uint32_t idelay_cha;                    // 0x48 **IDELAY CHA**
    uint32_t idelay_chb;                    // 0x4C **IDELAY CHB**
    uint32_t idelay_chc;                    // 0x50 **IDELAY CHC**
    uint32_t idelay_chd;                    // 0x54 **IDELAY CHD**
    uint32_t reserved_6[42];                // 0x58 - 0x100
    uint32_t fpga_ready;                    // 0x100 **FPGA ready**
    uint32_t reserved_7[959];               // 0x104 - 0x1000
    ext_trigger_t ext_trigger;              // 0x1000 **External trigger override**

} housekeeping_control_v2_t;

// For 250-12 board
typedef struct housekeeping_control_s_v3 {
    uint32_t id;                            // 0x0
    uint32_t dna_lo;                        // 0x4 **DNA part 1**
    uint32_t dna_hi;                        // 0x8 **DNA part 2**
    uint32_t digital_loop;                  // 0xC **Digital Loopback**
    uint32_t ex_cd_p;                       // 0x10 **Expansion connector direction P**
    uint32_t ex_cd_n;                       // 0x14 **Expansion connector direction N**
    uint32_t ex_co_p;                       // 0x18 **Expansion connector output P**
    uint32_t ex_co_n;                       // 0x1c **Expansion connector output N**
    uint32_t ex_ci_p;                       // 0x20 **Expansion connector input P**
    uint32_t ex_ci_n;                       // 0x24 **Expansion connector input N**
    uint32_t reserved_2;                    // 0x28
    uint32_t reserved_3;                    // 0x2C
    uint32_t led_control;                   // 0x30 **LED control**
    can_control_t can_control;              // 0x34 **CAN control**
    uint32_t reserved_4;                    // 0x38
    uint32_t reserved_5;                    // 0x3C
    pll_control_t pll_control;              // 0x40
    uint32_t idelay_reset;                  // 0x44 **IDELAY reset**
    uint32_t idelay_cha;                    // 0x48 **IDELAY CHA**
    uint32_t idelay_chb;                    // 0x4C **IDELAY CHB**
    uint32_t adc_spi_cw;                    // 0x50 **ADC SPI Control word**
    uint32_t adc_spi_wd;                    // 0x54 **ADC SPI Write data / start transfer**
    uint32_t adc_spi_rd;                    // 0x58 **ADC SPI Read data / Transfer busy**
    uint32_t reserved_6;                    // 0x5C
    uint32_t dac_spi_cw;                    // 0x60 **DAC SPI Control word**
    uint32_t dac_spi_wd;                    // 0x64 **DAC SPI Write data / start transfer**
    uint32_t dac_spi_rd;                    // 0x68 **DAC SPI Read data / Transfer busy**
    uint32_t reserved_7[37];                // 0x6C - 0x100
    uint32_t fpga_ready;                    // 0x100 **FPGA ready**
    uint32_t reserved_8[959];               // 0x104 - 0x1000
    ext_trigger_t ext_trigger;              // 0x1000 **External trigger override**

} housekeeping_control_v3_t;

static const uint32_t LED_CONTROL_MASK = 0xFF;
static const uint32_t DIGITAL_LOOP_MASK = 0x1;
static const uint32_t EX_CD_P_MASK = 0xFF;
static const uint32_t EX_CD_N_MASK = 0xFF;
static const uint32_t EX_CO_P_MASK = 0xFF;
static const uint32_t EX_CO_N_MASK = 0xFF;
static const uint32_t EX_CI_P_MASK = 0xFF;
static const uint32_t EX_CI_N_MASK = 0xFF;

int hk_EnableDigitalLoop(bool enable);
int house_SetPllControlEnable(bool enable);

volatile housekeeping_control_t *hk = NULL;

static int hk_Init(bool reset) {
    cmn_Map(HOUSEKEEPING_BASE_SIZE, HOUSEKEEPING_BASE_ADDR, (void**)&hk);

    if (rp_HPGetIsPLLControlEnableOrDefault()){
        if (reset) {
            house_SetPllControlEnable(false);
        }
    }
    return RP_OK;
}

static hk_version_t house_getHKVersion() {
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get board model\n");
    }

    switch (c)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
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
        case STEM_250_12_120:
            return HK_V3;
        default:
            fprintf(stderr,"[Error] Can't get board model\n");
    }
    return HK_V1;
}

static int hk_Release() {
    cmn_Unmap(HOUSEKEEPING_BASE_SIZE, (void**)&hk);
    hk = NULL;
    return RP_OK;
}

int house_GetPllControlEnable(bool *enable){
    if (rp_HPGetIsPLLControlEnableOrDefault()){
        hk_version_t ver = house_getHKVersion();
        switch (ver)
        {
            case HK_V2:{
                volatile housekeeping_control_v2_t *hk_v2 = (volatile housekeeping_control_v2_t*)hk;
                *enable = hk_v2->pll_control.enable;
                return RP_OK;
            }
            case HK_V3:{
                volatile housekeeping_control_v3_t *hk_v3 = (volatile housekeeping_control_v3_t*)hk;
                *enable = hk_v3->pll_control.enable;
                return RP_OK;
            }
            default:
                return RP_NOTS;
        }
    }
    return RP_NOTS;
}

int house_SetPllControlEnable(bool enable){
    if (rp_HPGetIsPLLControlEnableOrDefault()){
        hk_version_t ver = house_getHKVersion();
        switch (ver)
        {
            case HK_V2:{
                volatile housekeeping_control_v2_t *hk_v2 = (volatile housekeeping_control_v2_t*)hk;
                hk_v2->pll_control.enable = enable;
                return RP_OK;
            }
            case HK_V3:{
                volatile housekeeping_control_v3_t *hk_v3 = (volatile housekeeping_control_v3_t*)hk;
                hk_v3->pll_control.enable = enable;
                return RP_OK;
            }
            default:
                return RP_NOTS;
        }
    }
    return RP_NOTS;
}

int house_GetPllControlLocked(bool *status){
    if (rp_HPGetIsPLLControlEnableOrDefault()){
        hk_version_t ver = house_getHKVersion();
        switch (ver)
        {
            case HK_V2:{
                volatile housekeeping_control_v2_t *hk_v2 = (volatile housekeeping_control_v2_t*)hk;
                *status = hk_v2->pll_control.locked;
                return RP_OK;
            }
            case HK_V3:{
                volatile housekeeping_control_v3_t *hk_v3 = (volatile housekeeping_control_v3_t*)hk;
                *status = hk_v3->pll_control.locked;
                return RP_OK;
            }
            default:
                return RP_NOTS;
        }
    }
    return RP_NOTS;
}

int house_SetEnableDaisyChainSync(bool enable){
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            volatile housekeeping_control_v1_t *hk_v1 = (volatile housekeeping_control_v1_t*)hk;
            hk_v1->ext_trigger.enable = enable;
            return RP_OK;
        }
        case HK_V2:{
            volatile housekeeping_control_v2_t *hk_v2 = (volatile housekeeping_control_v2_t*)hk;
            hk_v2->ext_trigger.enable = enable;
            return RP_OK;
        }
        case HK_V3:{
            volatile housekeeping_control_v3_t *hk_v3 = (volatile housekeeping_control_v3_t*)hk;
            hk_v3->ext_trigger.enable = enable;
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
}

int house_GetEnableDaisyChainSync(bool *status){
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            volatile housekeeping_control_v1_t *hk_v1 = (volatile housekeeping_control_v1_t*)hk;
            *status = hk_v1->ext_trigger.enable;
            return RP_OK;
        }
        case HK_V2:{
            volatile housekeeping_control_v2_t *hk_v2 = (volatile housekeeping_control_v2_t*)hk;
            *status = hk_v2->ext_trigger.enable;
            return RP_OK;
        }
        case HK_V3:{
            volatile housekeeping_control_v3_t *hk_v3 = (volatile housekeeping_control_v3_t*)hk;
            *status = hk_v3->ext_trigger.enable;
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
}

int house_SetDpinEnableTrigOutput(bool enable){
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            volatile housekeeping_control_v1_t *hk_v1 = (volatile housekeeping_control_v1_t*)hk;
            hk_v1->ext_trigger.gpio_adc_dac = enable;
            return RP_OK;
        }
        case HK_V2:{
            volatile housekeeping_control_v2_t *hk_v2 = (volatile housekeeping_control_v2_t*)hk;
            hk_v2->ext_trigger.gpio_adc_dac = enable;
            return RP_OK;
        }
        case HK_V3:{
            volatile housekeeping_control_v3_t *hk_v3 = (volatile housekeeping_control_v3_t*)hk;
            hk_v3->ext_trigger.gpio_adc_dac = enable;
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
}

int house_GetDpinEnableTrigOutput(bool *enable){
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            volatile housekeeping_control_v1_t *hk_v1 = (volatile housekeeping_control_v1_t*)hk;
            *enable = hk_v1->ext_trigger.gpio_adc_dac;
            return RP_OK;
        }
        case HK_V2:{
            volatile housekeeping_control_v2_t *hk_v2 = (volatile housekeeping_control_v2_t*)hk;
            *enable = hk_v2->ext_trigger.gpio_adc_dac;
            return RP_OK;
        }
        case HK_V3:{
            volatile housekeeping_control_v3_t *hk_v3 = (volatile housekeeping_control_v3_t*)hk;
            *enable = hk_v3->ext_trigger.gpio_adc_dac;
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
}


int house_SetSourceTrigOutput(rp_outTiggerMode_t mode){
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            volatile housekeeping_control_v1_t *hk_v1 = (volatile housekeeping_control_v1_t*)hk;
            hk_v1->ext_trigger.out_selector = mode;
            return RP_OK;
        }
        case HK_V2:{
            volatile housekeeping_control_v2_t *hk_v2 = (volatile housekeeping_control_v2_t*)hk;
            hk_v2->ext_trigger.out_selector = mode;
            return RP_OK;
        }
        case HK_V3:{
            volatile housekeeping_control_v3_t *hk_v3 = (volatile housekeeping_control_v3_t*)hk;
            hk_v3->ext_trigger.out_selector = mode;
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
}

int house_GetSourceTrigOutput(rp_outTiggerMode_t *mode){
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            volatile housekeeping_control_v1_t *hk_v1 = (volatile housekeeping_control_v1_t*)hk;
            *mode = hk_v1->ext_trigger.out_selector;
            return RP_OK;
        }
        case HK_V2:{
            volatile housekeeping_control_v2_t *hk_v2 = (volatile housekeeping_control_v2_t*)hk;
            *mode = hk_v2->ext_trigger.out_selector;
            return RP_OK;
        }
        case HK_V3:{
            volatile housekeeping_control_v3_t *hk_v3 = (volatile housekeeping_control_v3_t*)hk;
            *mode = hk_v3->ext_trigger.out_selector;
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
}

int house_SetCANModeEnable(bool _enable){
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            volatile housekeeping_control_v1_t *hk_v1 = (volatile housekeeping_control_v1_t*)hk;
            hk_v1->can_control.enable = _enable;
            return RP_OK;
        }
        case HK_V2:{
            volatile housekeeping_control_v2_t *hk_v2 = (volatile housekeeping_control_v2_t*)hk;
            hk_v2->can_control.enable = _enable;
            return RP_OK;
        }
        case HK_V3:{
            volatile housekeeping_control_v3_t *hk_v3 = (volatile housekeeping_control_v3_t*)hk;
            hk_v3->can_control.enable = _enable;
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
}

int house_GetCANModeEnable(bool *_enable){
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            volatile housekeeping_control_v1_t *hk_v1 = (volatile housekeeping_control_v1_t*)hk;
            *_enable = hk_v1->can_control.enable;
            return RP_OK;
        }
        case HK_V2:{
            volatile housekeeping_control_v2_t *hk_v2 = (volatile housekeeping_control_v2_t*)hk;
            *_enable = hk_v2->can_control.enable;
            return RP_OK;
        }
        case HK_V3:{
            volatile housekeeping_control_v3_t *hk_v3 = (volatile housekeeping_control_v3_t*)hk;
            *_enable = hk_v3->can_control.enable;
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
}

#endif //__HOUSEKEEPING_H
