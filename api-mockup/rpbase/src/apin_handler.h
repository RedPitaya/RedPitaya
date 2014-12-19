/**
 * $Id: $
 *
 * @brief Red Pitaya library Analog Pin handler interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef APIN_HANDLER_H_
#define APIN_HANDLER_H_


#include "rp.h"

int apin_SetValue(rp_apin_t pin, float value);
int apin_GetValue(rp_apin_t pin, float* value);
int apin_SetValueRaw(rp_apin_t pin, uint32_t value);
int apin_GetValueRaw(rp_apin_t pin, uint32_t* value);
int apin_GetRange(rp_apin_t pin, float* min_val, float* max_val);

int toVolts(rp_apin_t pin, uint32_t value, float* returnValue);
int fromVolts(rp_apin_t pin, float value, uint32_t* returnValue);

#endif
