/**
 * $Id: $
 *
 * @brief Red Pitaya library API interface implementation
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
#include <stdint.h>

#include "common.h"
#include "analog.h"

static volatile analog_control_t *ams = NULL;

int analog_Init() {
    cmn_Map(ANALOG_MIXED_SIGNALS_BASE_SIZE, ANALOG_MIXED_SIGNALS_BASE_ADDR, (void**)&ams);
    return RP_OK;
}

int analog_Release() {
    cmn_Unmap(ANALOG_MIXED_SIGNALS_BASE_SIZE, (void**)&ams);
    return RP_OK;
}

/** @name Analog Inputs/Outputs
 */
///@{

int rp_ApinReset() {
    return rp_AOpinReset();
}

int rp_ApinGetValue(rp_apin_t pin, float* value) {
    if (pin <= RP_AIN3) {
        rp_AIpinGetValue(pin-RP_AIN0, value);
    } else if (pin <= RP_AOUT3) {
        rp_AOpinGetValue(pin-RP_AOUT0, value);
    } else {
        return RP_EPN;
    }
    return RP_OK;
}

int rp_ApinGetValueRaw(rp_apin_t pin, uint32_t* value) {
    if (pin <= RP_AIN3) {
        rp_AIpinGetValueRaw(pin-RP_AIN0, value);
    } else if (pin <= RP_AOUT3) {
        rp_AOpinGetValueRaw(pin-RP_AOUT0, value);
    } else {
        return RP_EPN;
    }
    return RP_OK;
}

int rp_ApinSetValue(rp_apin_t pin, float value) {
    if (pin <= RP_AIN3) {
        return RP_EPN;
    } else if (pin <= RP_AOUT3) {
        rp_AOpinSetValue(pin-RP_AOUT0, value);
    } else {
        return RP_EPN;
    }
    return RP_OK;
}

int rp_ApinSetValueRaw(rp_apin_t pin, uint32_t value) {
    if (pin <= RP_AIN3) {
        return RP_EPN;
    } else if (pin <= RP_AOUT3) {
        rp_AOpinSetValueRaw(pin-RP_AOUT0, value);
    } else {
        return RP_EPN;
    }
    return RP_OK;
}

int rp_ApinGetRange(rp_apin_t pin, float* min_val, float* max_val) {
    if (pin <= RP_AIN3) {
        *min_val = ANALOG_IN_MIN_VAL;
        *max_val = ANALOG_IN_MAX_VAL;
    } else if (pin <= RP_AOUT3) {
        *min_val = ANALOG_OUT_MIN_VAL;
        *max_val = ANALOG_OUT_MAX_VAL;
    } else {
        return RP_EPN;
    }
    return RP_OK;
}


/**
 * Analog Inputs
 */

int rp_AIpinGetValueRaw(int unsigned pin, uint32_t* value) {
    FILE *fp;
    switch (pin) {
        case 0:  fp = fopen ("/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/in_voltage11_raw", "r");  break;
        case 1:  fp = fopen ("/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/in_voltage9_raw", "r");   break;
        case 2:  fp = fopen ("/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/in_voltage10_raw", "r");  break;
        case 3:  fp = fopen ("/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/in_voltage12_raw", "r");  break;
        default:
            return RP_EPN;
    }
    int r = !fscanf (fp, "%d", value);
    fclose(fp);
    return r;
}

int rp_AIpinGetValue(int unsigned pin, float* value) {
    uint32_t value_raw;
    int result = rp_AIpinGetValueRaw(pin, &value_raw);
    *value = (((float)value_raw / ANALOG_IN_MAX_VAL_INTEGER) * (ANALOG_IN_MAX_VAL - ANALOG_IN_MIN_VAL)) + ANALOG_IN_MIN_VAL;
    return result;
}


/**
 * Analog Outputs
 */

int rp_AOpinReset() {
    for (int unsigned pin=0; pin<4; pin++) {
        rp_AOpinSetValueRaw(pin, 0);
    }
    return RP_OK;
}

int rp_AOpinSetValueRaw(int unsigned pin, uint32_t value) {
    if (pin >= 4) {
        return RP_EPN;
    }
    if (value > ANALOG_OUT_MAX_VAL_INTEGER) {
        return RP_EOOR;
    }
    iowrite32(value & ANALOG_OUT_MASK, &ams->pdm_cfg[pin]);
    return RP_OK;
}

int rp_AOpinSetValue(int unsigned pin, float value) {
    uint32_t value_raw = (uint32_t) (((value - ANALOG_OUT_MIN_VAL) / (ANALOG_OUT_MAX_VAL - ANALOG_OUT_MIN_VAL)) * ANALOG_OUT_MAX_VAL_INTEGER);
    return rp_AOpinSetValueRaw(pin, value_raw);
}

int rp_AOpinGetValueRaw(int unsigned pin, uint32_t* value) {
    if (pin >= 4) {
        return RP_EPN;
    }
    *value = ioread32(&ams->pdm_cfg[pin]) & ANALOG_OUT_MASK;
    return RP_OK;
}

int rp_AOpinGetValue(int unsigned pin, float* value) {
    uint32_t value_raw;
    int result = rp_AOpinGetValueRaw(pin, &value_raw);
    *value = (((float)value_raw / ANALOG_OUT_MAX_VAL_INTEGER) * (ANALOG_OUT_MAX_VAL - ANALOG_OUT_MIN_VAL)) + ANALOG_OUT_MIN_VAL;
    return result;
}

int rp_AOpinGetRange(int unsigned pin, float* min_val,  float* max_val) {
    *min_val = ANALOG_OUT_MIN_VAL;
    *max_val = ANALOG_OUT_MAX_VAL;
    return RP_OK;
}
