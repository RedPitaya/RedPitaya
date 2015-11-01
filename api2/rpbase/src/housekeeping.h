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

// Base Housekeeping address
static const int HOUSEKEEPING_BASE_ADDR = 0x40000000;
static const int HOUSEKEEPING_BASE_SIZE = 0x30;

static const uint32_t RP_CALIB_DWM = 16;
static const uint32_t RP_CALIB_DWS = 14;

// calibration gain offset pair
typedef struct {
    uint32_t mul;  // fixed point   signed  s1.14 (RP_CALIB_DWM bits)
    uint32_t sum;  // fixed point unsigned  u1.13 (RP_CALIB_DWS bits)
} hw_calib_pair_t;

// Housekeeping structure declaration
typedef struct housekeeping_control_s {
    uint32_t id;            // 0x00
    uint32_t dna_lo;        // 0x04
    uint32_t dna_hi;        // 0x08
    uint32_t digital_loop;  // 0x0c
    uint32_t ex_cd_p;       // 0x10
    uint32_t ex_cd_n;       // 0x14
    uint32_t ex_co_p;       // 0x18
    uint32_t ex_co_n;       // 0x1c
    uint32_t ex_ci_p;       // 0x20
    uint32_t ex_ci_n;       // 0x24
    uint32_t reserved_2;    // 0x28
    uint32_t reserved_3;    // 0x2c
    uint32_t led_control;   // 0x30
    hw_calib_pair_t adc [2];
    hw_calib_pair_t dac [2];
} housekeeping_control_t;

static const uint32_t LED_CONTROL_MASK = 0xFF;
static const uint32_t DIGITAL_LOOP_MASK = 0x1;
static const uint32_t GPIO_MASK = 0xFF;

int hk_EnableDigitalLoop(bool enable);

static volatile housekeeping_control_t *hk = NULL;

static int hk_Init() {
    ECHECK(cmn_Map(HOUSEKEEPING_BASE_SIZE, HOUSEKEEPING_BASE_ADDR, (void**)&hk));
    return RP_OK;
}

static int hk_Release() {
    ECHECK(cmn_Unmap(HOUSEKEEPING_BASE_SIZE, (void**)&hk));
    return RP_OK;
}

#endif //__HOUSEKEEPING_H
