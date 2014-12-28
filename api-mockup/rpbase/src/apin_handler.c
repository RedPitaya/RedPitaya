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


#include <stdint.h>

#include "common.h"
#include "analog_mixed_signals.h"
#include "apin_handler.h"


int apin_SetValue(rp_apin_t pin, float value)
{
    uint32_t value_raw;
    int result = fromVolts(pin, value, &value_raw);
    if (result == RP_OK) {
        return apin_SetValueRaw(pin, value_raw);
    } else {
        return result;
    }
}

int apin_GetValue(rp_apin_t pin, float* value)
{
    uint32_t value_raw;
    int result = apin_GetValueRaw(pin, &value_raw);
    if (result == RP_OK) {
        return toVolts(pin, value_raw, value);
    } else {
        return result;
    }
}

int apin_SetValueRaw(rp_apin_t pin, uint32_t value)
{
    switch (pin) {
        case RP_AOUT0:
            return ams_SetValueDAC0(value);
        case RP_AOUT1:
            return ams_SetValueDAC1(value);
        case RP_AOUT2:
            return ams_SetValueDAC2(value);
        case RP_AOUT3:
            return ams_SetValueDAC3(value);
        default:
            return RP_EPN;
    }
}

int apin_GetValueRaw(rp_apin_t pin, uint32_t* value)
{
    switch (pin) {
        case RP_AIN0:
            return ams_GetValueADC0(value);
        case RP_AIN1:
            return ams_GetValueADC1(value);
        case RP_AIN2:
            return ams_GetValueADC2(value);
        case RP_AIN3:
            return ams_GetValueADC3(value);
        case RP_AOUT0:
            return ams_GetValueDAC0(value);
        case RP_AOUT1:
            return ams_GetValueDAC1(value);
        case RP_AOUT2:
            return ams_GetValueDAC2(value);
        case RP_AOUT3:
            return ams_GetValueDAC3(value);
        default:
            return RP_EPN;
    }
}

int apin_GetRange(rp_apin_t pin, float* min_val, float* max_val)
{
    uint32_t unused;
    return apin_GetRangeWithInt(pin, min_val, max_val, &unused);
}

int apin_GetRangeWithInt(rp_apin_t pin, float *min_val, float *max_val, uint32_t *int_max_val) {
    switch (pin) {
        case RP_AIN0:
        case RP_AIN1:
        case RP_AIN2:
        case RP_AIN3:
            return ams_GetRangeInput(min_val, max_val, int_max_val);
        case RP_AOUT0:
        case RP_AOUT1:
        case RP_AOUT2:
        case RP_AOUT3:
            return ams_GetRangeOutput(min_val, max_val, int_max_val);
        default:
            return RP_EPN;
    }
}

int toVolts(rp_apin_t pin, uint32_t value, float* returnValue)
{
    float min, max;
    uint32_t max_int;
    int result = apin_GetRangeWithInt(pin, &min, &max, &max_int);
    if (result == RP_OK) {
        *returnValue = (((float)value / max_int) * (max - min)) + min;
        return RP_OK;
    } else {
        return result;
    }
}

int fromVolts(rp_apin_t pin, float value, uint32_t* returnValue)
{
    float min, max;
    uint32_t max_int;
    int result = apin_GetRangeWithInt(pin, &min, &max, &max_int);
    if (result == RP_OK) {
        *returnValue = (uint32_t) (((value - min) / (max - min)) * max_int);
        return RP_OK;
    } else {
        return result;
    }
}
