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

#include <stdbool.h>
#include <stdint.h>
#include "common.h"
#include "rp.h"
#include "rp_hw-profiles.h"

// Base Housekeeping address
static const int HOUSEKEEPING_BASE_ADDR = 0x00000000;
static const int HOUSEKEEPING_BASE_SIZE = 0x2000;

// Housekeeping structure declaration

typedef struct {
    uint32_t enable : 1, : 3, refDetected : 1, : 3, locked : 1, : 23;
    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "enable", enable);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "refDetected", refDetected);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "locked", locked);
    };
} pll_control_t;

typedef union {
    uint32_t reg_full;
    pll_control_t reg;
} pll_control_u_t;

typedef struct {
    uint32_t enable : 1;
    uint32_t gpio_adc_dac : 1;
    uint32_t out_selector : 1;
    uint32_t : 29;
    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "Enable", enable);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "gpio_adc_dac", gpio_adc_dac);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "out_selector", out_selector);
    };
} ext_trigger_t;

typedef union {
    uint32_t reg_full;
    ext_trigger_t reg;
} ext_trigger_u_t;

typedef struct {
    uint32_t enable : 1;
    uint32_t : 31;
    void print() volatile { printRegBit(" - %-39s = 0x%08X (%d)\n", "Enable", enable); };
} can_control_t;

typedef union {
    uint32_t reg_full;
    can_control_t reg;
} can_control_u_t;

typedef enum { HK_V1, HK_V2, HK_V3, HK_V4 } hk_version_t;

typedef struct {
    uint32_t data[2048];
} housekeeping_control_t;

// Classic board
typedef struct housekeeping_control_s_v1 {
    uint32_t id;                  // 0x0
    uint32_t dna_lo;              // 0x4 **DNA part 1**
    uint32_t dna_hi;              // 0x8 **DNA part 2**
    uint32_t digital_loop;        // 0xC **Digital Loopback**
    uint32_t ex_cd_p;             // 0x10 **Expansion connector direction P**
    uint32_t ex_cd_n;             // 0x14 **Expansion connector direction N**
    uint32_t ex_co_p;             // 0x18 **Expansion connector output P**
    uint32_t ex_co_n;             // 0x1c **Expansion connector output N**
    uint32_t ex_ci_p;             // 0x20 **Expansion connector input P**
    uint32_t ex_ci_n;             // 0x24 **Expansion connector input N**
    uint32_t reserved_2;          // 0x28
    uint32_t reserved_3;          // 0x2C
    uint32_t led_control;         // 0x30 **LED control**
    can_control_u_t can_control;  // 0x34 **CAN control**
    uint32_t reserved_4[50];      // 0x38 - 0x100
    uint32_t fpga_ready;          // 0x100 **FPGA ready**
    uint32_t acq_clock_counter;   // 0x104 **ADC clock frequency meter**
    uint32_t reserved_5[958];     // 0x108 - 0x1000
    ext_trigger_u_t ext_trigger;  // 0x1000 **External trigger override**
} housekeeping_control_v1_t;

// For 125-14-4ch board
typedef struct housekeeping_control_s_v2 {
    uint32_t id;                  // 0x0
    uint32_t dna_lo;              // 0x4 **DNA part 1**
    uint32_t dna_hi;              // 0x8 **DNA part 2**
    uint32_t digital_loop;        // 0xC **Digital Loopback**
    uint32_t ex_cd_p;             // 0x10 **Expansion connector direction P**
    uint32_t ex_cd_n;             // 0x14 **Expansion connector direction N**
    uint32_t ex_co_p;             // 0x18 **Expansion connector output P**
    uint32_t ex_co_n;             // 0x1c **Expansion connector output N**
    uint32_t ex_ci_p;             // 0x20 **Expansion connector input P**
    uint32_t ex_ci_n;             // 0x24 **Expansion connector input N**
    uint32_t reserved_2;          // 0x28
    uint32_t reserved_3;          // 0x2C
    uint32_t led_control;         // 0x30 **LED control**
    can_control_u_t can_control;  // 0x34 **CAN control**
    uint32_t reserved_4;          // 0x38
    uint32_t reserved_5;          // 0x3C
    pll_control_u_t pll_control;  // 0x40
    uint32_t idelay_reset;        // 0x44 **IDELAY reset**
    uint32_t idelay_cha;          // 0x48 **IDELAY CHA**
    uint32_t idelay_chb;          // 0x4C **IDELAY CHB**
    uint32_t idelay_chc;          // 0x50 **IDELAY CHC**
    uint32_t idelay_chd;          // 0x54 **IDELAY CHD**
    uint32_t reserved_6[42];      // 0x58 - 0x100
    uint32_t fpga_ready;          // 0x100 **FPGA ready**
    uint32_t acq_clock_counter;   // 0x104 **ADC clock frequency meter**
    uint32_t reserved_7[958];     // 0x108 - 0x1000
    ext_trigger_u_t ext_trigger;  // 0x1000 **External trigger override**

} housekeeping_control_v2_t;

// For 250-12 board
typedef struct housekeeping_control_s_v3 {
    uint32_t id;                  // 0x0
    uint32_t dna_lo;              // 0x4 **DNA part 1**
    uint32_t dna_hi;              // 0x8 **DNA part 2**
    uint32_t digital_loop;        // 0xC **Digital Loopback**
    uint32_t ex_cd_p;             // 0x10 **Expansion connector direction P**
    uint32_t ex_cd_n;             // 0x14 **Expansion connector direction N**
    uint32_t ex_co_p;             // 0x18 **Expansion connector output P**
    uint32_t ex_co_n;             // 0x1c **Expansion connector output N**
    uint32_t ex_ci_p;             // 0x20 **Expansion connector input P**
    uint32_t ex_ci_n;             // 0x24 **Expansion connector input N**
    uint32_t reserved_2;          // 0x28
    uint32_t reserved_3;          // 0x2C
    uint32_t led_control;         // 0x30 **LED control**
    can_control_u_t can_control;  // 0x34 **CAN control**
    uint32_t reserved_4;          // 0x38
    uint32_t reserved_5;          // 0x3C
    pll_control_u_t pll_control;  // 0x40
    uint32_t idelay_reset;        // 0x44 **IDELAY reset**
    uint32_t idelay_cha;          // 0x48 **IDELAY CHA**
    uint32_t idelay_chb;          // 0x4C **IDELAY CHB**
    uint32_t adc_spi_cw;          // 0x50 **ADC SPI Control word**
    uint32_t adc_spi_wd;          // 0x54 **ADC SPI Write data / start transfer**
    uint32_t adc_spi_rd;          // 0x58 **ADC SPI Read data / Transfer busy**
    uint32_t reserved_6;          // 0x5C
    uint32_t dac_spi_cw;          // 0x60 **DAC SPI Control word**
    uint32_t dac_spi_wd;          // 0x64 **DAC SPI Write data / start transfer**
    uint32_t dac_spi_rd;          // 0x68 **DAC SPI Read data / Transfer busy**
    uint32_t reserved_7[37];      // 0x6C - 0x100
    uint32_t fpga_ready;          // 0x100 **FPGA ready**
    uint32_t acq_clock_counter;   // 0x104 **ADC clock frequency meter**
    uint32_t reserved_8[958];     // 0x104 - 0x1000
    ext_trigger_u_t ext_trigger;  // 0x1000 **External trigger override**

} housekeeping_control_v3_t;

// For Low Latency board
typedef struct housekeeping_control_s_v4 {
    uint32_t id;                  // 0x0
    uint32_t dna_lo;              // 0x4 **DNA part 1**
    uint32_t dna_hi;              // 0x8 **DNA part 2**
    uint32_t digital_loop;        // 0xC **Digital Loopback**
    uint32_t ex_cd_p;             // 0x10 **Expansion connector direction P**
    uint32_t ex_cd_n;             // 0x14 **Expansion connector direction N**
    uint32_t ex_co_p;             // 0x18 **Expansion connector output P**
    uint32_t ex_co_n;             // 0x1c **Expansion connector output N**
    uint32_t ex_ci_p;             // 0x20 **Expansion connector input P**
    uint32_t ex_ci_n;             // 0x24 **Expansion connector input N**
    uint32_t reserved_2;          // 0x28
    uint32_t reserved_3;          // 0x2C
    uint32_t led_control;         // 0x30 **LED control**
    can_control_u_t can_control;  // 0x34 **CAN control**
    uint32_t reserved_4;          // 0x38
    uint32_t reserved_5;          // 0x3C
    uint32_t idelay_control;      // 0x40
    uint32_t reserved_6;          // 0x44
    uint32_t reserved_7;          // 0x48
    uint32_t reserved_8;          // 0x4C
    uint32_t adc_spi_cw;          // 0x50 **ADC SPI Control word**
    uint32_t adc_spi_wd;          // 0x54 **ADC SPI Write data / start transfer**
    uint32_t adc_spi_rd;          // 0x58 **ADC SPI Read data / Transfer busy**
    uint32_t reserved_9[41];      // 0x5C - 0x100
    uint32_t fpga_ready;          // 0x100 **FPGA ready**
    uint32_t acq_clock_counter;   // 0x104 **ADC clock frequency meter**
} housekeeping_control_v4_t;

static const uint32_t LED_CONTROL_MASK = 0xFF;
static const uint32_t DIGITAL_LOOP_MASK = 0x1;
static const uint32_t EX_CD_P_MASK = 0xFF;
static const uint32_t EX_CD_N_MASK = 0xFF;
static const uint32_t EX_CO_P_MASK = 0xFF;
static const uint32_t EX_CO_N_MASK = 0xFF;
static const uint32_t EX_CI_P_MASK = 0xFF;
static const uint32_t EX_CI_N_MASK = 0xFF;

// int hk_EnableDigitalLoop(bool enable);

int hk_Init();

int hk_Reset();

int hk_Release();

volatile housekeeping_control_t* hk_GetHK();

int hk_printRegset();

hk_version_t house_getHKVersion();

int house_GetPllControlEnable(bool* enable);

int house_SetPllControlEnable(bool enable);

int house_GetPllControlLocked(bool* status);

int house_SetEnableDaisyChainSync(bool enable);

int house_GetEnableDaisyChainSync(bool* status);

int house_SetDpinEnableTrigOutput(bool enable);

int house_GetDpinEnableTrigOutput(bool* enable);

int house_SetSourceTrigOutput(rp_outTiggerMode_t mode);

int house_GetSourceTrigOutput(rp_outTiggerMode_t* mode);

int house_SetCANModeEnable(bool _enable);

int house_GetCANModeEnable(bool* _enable);

int house_SetPllControlEnable(bool enable);

#endif  //__HOUSEKEEPING_H
