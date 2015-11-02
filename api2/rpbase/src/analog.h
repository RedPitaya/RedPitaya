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
static const int ANALOG_MIXED_SIGNALS_BASE_ADDR = 0x40400000;
static const int ANALOG_MIXED_SIGNALS_BASE_SIZE = 0x30;

typedef struct {
    uint32_t pdm_cfg [4];
} analog_control_t;

static const uint32_t ANALOG_OUT_MASK            = 0xFF;
static const uint32_t ANALOG_IN_MASK             = 0xFFF;

static const float    ANALOG_IN_MAX_VAL          = 7.0;
static const float    ANALOG_IN_MIN_VAL          = 0.0;
static const uint32_t ANALOG_IN_MAX_VAL_INTEGER  = 0xFFF;

static const float    ANALOG_OUT_MAX_VAL         = 1.8;
static const float    ANALOG_OUT_MIN_VAL         = 0.0;
static const uint32_t ANALOG_OUT_MAX_VAL_INTEGER = 255;

int analog_Init();
int analog_Release();

#endif //__ANALOG_MIXED_SIGNALS_H
