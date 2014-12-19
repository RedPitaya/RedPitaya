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

int ams_GetRangeInput(float *min_val, float *max_val);
int ams_GetRangeOutput(float *min_val, float *max_val);

#endif //__ANALOG_MIXED_SIGNALS_H
