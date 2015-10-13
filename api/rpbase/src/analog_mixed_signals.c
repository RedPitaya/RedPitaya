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


#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "analog_mixed_signals.h"


static volatile analog_mixed_signals_control_t *ams = NULL;


int ams_Init()
{
//    ECHECK(cmn_Init());
    ECHECK(cmn_Map(ANALOG_MIXED_SIGNALS_BASE_SIZE, ANALOG_MIXED_SIGNALS_BASE_ADDR, (void**)&ams));
    return RP_OK;
}

int ams_Release()
{
    ECHECK(cmn_Unmap(ANALOG_MIXED_SIGNALS_BASE_SIZE, (void**)&ams));
//    ECHECK(cmn_Release());
    return RP_OK;
}

int ams_SetValueDAC0(uint32_t value)
{
    if (value > ANALOG_OUT_MAX_VAL_INTEGER) {
        return RP_EOOR;
    }
    return cmn_SetShiftedValue(&ams->dac0, value, ANALOG_OUT_MASK, ANALOG_OUT_BITS);
    return RP_OK;
}

int ams_SetValueDAC1(uint32_t value)
{
    if (value > ANALOG_OUT_MAX_VAL_INTEGER) {
        return RP_EOOR;
    }
    return cmn_SetShiftedValue(&ams->dac1, value, ANALOG_OUT_MASK, ANALOG_OUT_BITS);
    return RP_OK;
}

int ams_SetValueDAC2(uint32_t value)
{
    if (value > ANALOG_OUT_MAX_VAL_INTEGER) {
        return RP_EOOR;
    }
    return cmn_SetShiftedValue(&ams->dac2, value, ANALOG_OUT_MASK, ANALOG_OUT_BITS);
    return RP_OK;
}

int ams_SetValueDAC3(uint32_t value)
{
    if (value > ANALOG_OUT_MAX_VAL_INTEGER) {
        return RP_EOOR;
    }
    return cmn_SetShiftedValue(&ams->dac3, value, ANALOG_OUT_MASK, ANALOG_OUT_BITS);
    return RP_OK;
}

int ams_GetValueADC0(uint32_t* value)
{
    FILE *fp = fopen ("/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/in_voltage11_raw", "r");
    int r = !fscanf (fp, "%d", value);
    return r;
}

int ams_GetValueADC1(uint32_t* value)
{
    FILE *fp = fopen ("/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/in_voltage9_raw", "r");
    int r = !fscanf (fp, "%d", value);
    return r;
}

int ams_GetValueADC2(uint32_t* value)
{
    FILE *fp = fopen ("/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/in_voltage10_raw", "r");
    int r = !fscanf (fp, "%d", value);
    return r;
}

int ams_GetValueADC3(uint32_t* value)
{
    FILE *fp = fopen ("/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/in_voltage12_raw", "r");
    int r = !fscanf (fp, "%d", value);
    return r;
}

int ams_GetValueDAC0(uint32_t* value)
{
    return cmn_GetShiftedValue(&ams->dac0, value, ANALOG_IN_MASK, ANALOG_OUT_BITS);
}

int ams_GetValueDAC1(uint32_t* value)
{
    return cmn_GetShiftedValue(&ams->dac1, value, ANALOG_IN_MASK, ANALOG_OUT_BITS);
}

int ams_GetValueDAC2(uint32_t* value)
{
    return cmn_GetShiftedValue(&ams->dac2, value, ANALOG_IN_MASK, ANALOG_OUT_BITS);
}

int ams_GetValueDAC3(uint32_t* value)
{
    return cmn_GetShiftedValue(&ams->dac3, value, ANALOG_IN_MASK, ANALOG_OUT_BITS);
}

int ams_GetRangeInput(float *min_val, float *max_val, uint32_t *int_max_val) {
    *min_val = ANALOG_IN_MIN_VAL;
    *max_val = ANALOG_IN_MAX_VAL;
    *int_max_val = ANALOG_IN_MAX_VAL_INTEGER;
    return RP_OK;
}

int ams_GetRangeOutput(float *min_val, float *max_val, uint32_t *int_max_val) {
    *min_val = ANALOG_OUT_MIN_VAL;
    *max_val = ANALOG_OUT_MAX_VAL;
    *int_max_val = ANALOG_OUT_MAX_VAL_INTEGER;
    return RP_OK;
}
