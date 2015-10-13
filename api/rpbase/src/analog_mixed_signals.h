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

typedef struct analog_mixed_signals_control_s {
    uint32_t aif0;
    uint32_t aif1;
    uint32_t aif2;
    uint32_t aif3;
    uint32_t reserved[4];
    uint32_t dac0;
    uint32_t dac1;
    uint32_t dac2;
    uint32_t dac3;
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


int ams_Init();
int ams_Release();

int ams_SetValueDAC0(uint32_t value);
int ams_SetValueDAC1(uint32_t value);
int ams_SetValueDAC2(uint32_t value);
int ams_SetValueDAC3(uint32_t value);

int ams_GetValueADC0(uint32_t* value);
int ams_GetValueADC1(uint32_t* value);
int ams_GetValueADC2(uint32_t* value);
int ams_GetValueADC3(uint32_t* value);
int ams_GetValueDAC0(uint32_t* value);
int ams_GetValueDAC1(uint32_t* value);
int ams_GetValueDAC2(uint32_t* value);
int ams_GetValueDAC3(uint32_t* value);

int ams_GetRangeInput(float *min_val, float *max_val, uint32_t *int_max_val);
int ams_GetRangeOutput(float *min_val, float *max_val, uint32_t *int_max_val);

#endif //__ANALOG_MIXED_SIGNALS_H
