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

// Housekeeping structure declaration
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
} housekeeping_control_t;


static const uint32_t LED_CONTROL_MASK = 0xFF;
static const uint32_t DIGITAL_LOOP_MASK = 0x1;
static const uint32_t EX_CD_P_MASK = 0xFF;
static const uint32_t EX_CD_N_MASK = 0xFF;
static const uint32_t EX_CO_P_MASK = 0xFF;
static const uint32_t EX_CO_N_MASK = 0xFF;
static const uint32_t EX_CI_P_MASK = 0xFF;
static const uint32_t EX_CI_N_MASK = 0xFF;


int hk_Init();
int hk_Release();

int hk_GetID(uint32_t *id);
int hk_GetDNA(uint64_t *dna);

int hk_SetLedBits(uint32_t bits);
int hk_UnsetLedBits(uint32_t bits);
int hk_AreLedBitsSet(uint32_t bits, bool* result);

int hk_SetExCdPBits(uint32_t bits);
int hk_UnsetExCdPBits(uint32_t bits);
int hk_AreExCdPBitsSet(uint32_t bits, bool* result);

int hk_SetExCdNBits(uint32_t bits);
int hk_UnsetExCdNBits(uint32_t bits);
int hk_AreExCdNBitsSet(uint32_t bits, bool* result);

int hk_SetExCoPBits(uint32_t bits);
int hk_UnsetExCoPBits(uint32_t bits);
int hk_AreExCoPBitsSet(uint32_t bits, bool* result);

int hk_SetExCoNBits(uint32_t bits);
int hk_UnsetExCoNBits(uint32_t bits);
int hk_AreExCoNBitsSet(uint32_t bits, bool* result);

int hk_AreExCiPBitsSet(uint32_t bits, bool* result);

int hk_AreExCiNBitsSet(uint32_t bits, bool* result);

int hk_EnableDigitalLoop(bool enable);

#endif //__HOUSEKEEPING_H
