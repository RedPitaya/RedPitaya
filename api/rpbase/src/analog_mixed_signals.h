/**
 * $Id: $
 *
 * @brief Red Pitaya library Analog Mixed Signals (AMS) module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __ANALOG_MIXED_SIGNALS_H
#define __ANALOG_MIXED_SIGNALS_H

// Base Analog Mixed Signals address
static const int ANALOG_MIXED_SIGNALS_BASE_ADDR = 0x00400000;
static const int ANALOG_MIXED_SIGNALS_BASE_SIZE = 0x30;

typedef struct analog_mixed_signals_control_s {
    uint32_t aif [4];
    uint32_t reserved[4];
    uint32_t dac [4];
} analog_mixed_signals_control_t;

static const uint32_t ANALOG_OUT_MASK            = 0xFF;
static const uint32_t ANALOG_OUT_BITS            = 16;
static const uint32_t ANALOG_IN_MASK             = 0xFFF;

static const float    ANALOG_IN_MAX_VAL          = 7.0;
static const float    ANALOG_IN_MIN_VAL          = 0.0;
static const uint32_t ANALOG_IN_MAX_VAL_INTEGER  = 0xFFF;
static const float    ANALOG_OUT_MAX_VAL         = 1.8;
static const float    ANALOG_OUT_MIN_VAL         = 0.0;
static const uint32_t ANALOG_OUT_MAX_VAL_INTEGER = 156;

static volatile analog_mixed_signals_control_t *ams = NULL;

static int ams_Init() {
    cmn_Map(ANALOG_MIXED_SIGNALS_BASE_SIZE, ANALOG_MIXED_SIGNALS_BASE_ADDR, (void**)&ams);
    return RP_OK;
}

static int ams_Release() {
    cmn_Unmap(ANALOG_MIXED_SIGNALS_BASE_SIZE, (void**)&ams);
    return RP_OK;
}

#endif //__ANALOG_MIXED_SIGNALS_H
