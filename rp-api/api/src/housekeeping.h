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
#include "rp_cross.h"
// Base Housekeeping address
static const int HOUSEKEEPING_BASE_ADDR = 0x00000000;
#ifdef Z20_250_12
    static const int HOUSEKEEPING_BASE_SIZE = 0x44;
#else
    static const int HOUSEKEEPING_BASE_SIZE = 0x34;
#endif

// Housekeeping structure declaration
typedef struct pll_control_s {
    uint32_t enable: 1, : 3, refDetected: 1, : 3, locked: 1;
} pll_control_t;

typedef struct housekeeping_control_s {
    uint32_t id;
    uint32_t dna_lo;
    uint32_t dna_hi;
    uint32_t digital_loop;
    uint32_t ex_cd_p;
    uint32_t ex_cd_n;
    uint32_t ex_co_p;
    uint32_t ex_co_n;
    uint32_t ex_ci_p;
    uint32_t ex_ci_n;
    uint32_t reserved_2;
    uint32_t reserved_3;
    uint32_t led_control;
#ifdef Z20_250_12
    uint32_t reserved_4;
    uint32_t reserved_5;
    uint32_t reserved_6;
    pll_control_t pll_control;
#endif
} housekeeping_control_t;


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

static volatile housekeeping_control_t *hk = NULL;

static int hk_Init(bool reset) {
    cmn_Map(HOUSEKEEPING_BASE_SIZE, HOUSEKEEPING_BASE_ADDR, (void**)&hk);
    if (reset) {
        house_SetPllControlEnable(false);
    }
    return RP_OK;
}

static int hk_Release() {
    cmn_Unmap(HOUSEKEEPING_BASE_SIZE, (void**)&hk);
    return RP_OK;
}

int house_GetPllControlEnable(bool *enable){
#ifdef Z20_250_12
    *enable = hk->pll_control.enable;
    return RP_OK;
#else
    return RP_NOTS;
#endif
}

int house_SetPllControlEnable(bool enable){
#ifdef Z20_250_12
    hk->pll_control.enable = enable;
    return RP_OK;
#else
    return RP_NOTS;
#endif
}

int house_GetPllControlLocked(bool *status){
#ifdef Z20_250_12
    *status = hk->pll_control.locked;
    return RP_OK;
#else
    return RP_NOTS;
#endif
}

#endif //__HOUSEKEEPING_H
