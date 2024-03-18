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
#include <stdlib.h>
#include <pthread.h>

#include "common/version.h"
#include "common.h"
#include "housekeeping.h"
#include "oscilloscope.h"
#include "acq_handler.h"
#include "analog_mixed_signals.h"
#include "rp_hw-calib.h"
#include "generate.h"
#include "gen_handler.h"
#include "daisy.h"

#include "rp-i2c-mcp47x6-c.h"
#include "rp-i2c-max7311-c.h"

static char version[50];
int g_api_state = 0;
float g_ext_trig_trash = 0;

pthread_mutex_t rp_init_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Global methods
 */

int rp_Init()
{
    return rp_InitReset(true);
}

int rp_InitReset(bool reset)
{
    if (g_api_state) return RP_EOOR;
    pthread_mutex_lock(&rp_init_mutex);
    cmn_Init();

    rp_CalibInit();
    hk_Init(reset);
    ams_Init();
    daisy_Init();

    if (rp_HPIsFastDAC_PresentOrDefault()){
        generate_Init();
    }

    osc_Init(rp_HPGetFastADCChannelsCountOrDefault());

    // Set default configuration per handler
    if (reset){
        rp_Reset();
    }
    g_api_state = true;
    pthread_mutex_unlock(&rp_init_mutex);
    return RP_OK;
}

int rp_IsApiInit(){
    return g_api_state;
}

int rp_Release()
{
    pthread_mutex_lock(&rp_init_mutex);
    ECHECK_NO_RET(osc_Release())
    ECHECK_NO_RET(generate_Release())
    ECHECK_NO_RET(ams_Release())
    ECHECK_NO_RET(hk_Release())
    ECHECK_NO_RET(daisy_Release())
    ECHECK_NO_RET(cmn_Release())
    g_api_state = false;
    pthread_mutex_unlock(&rp_init_mutex);
    return RP_OK;
}

int rp_Reset()
{
    rp_DpinReset();
    rp_AOpinReset();

    if (rp_HPIsFastDAC_PresentOrDefault()){
        rp_GenReset();
    }

    rp_AcqReset();
    return 0;
}

const char* rp_GetVersion()
{
    sprintf(version, "%s (%s)", VERSION_STR, REVISION_STR);
    return version;
}

const char* rp_GetError(int errorCode) {
    switch (errorCode) {
        case RP_OK:    return "OK";
        case RP_EOED:  return "Failed to Open EEPROM Device.";
        case RP_EOMD:  return "Failed to open memory device.";
        case RP_ECMD:  return "Failed to close memory device.";
        case RP_EMMD:  return "Failed to map memory device.";
        case RP_EUMD:  return "Failed to unmap memory device.";
        case RP_EOOR:  return "Value out of range.";
        case RP_ELID:  return "LED input direction is not valid.";
        case RP_EMRO:  return "Modifying read only filed is not allowed.";
        case RP_EWIP:  return "Writing to input pin is not valid.";
        case RP_EPN:   return "Invalid Pin number.";
        case RP_UIA:   return "Uninitialized Input Argument.";
        case RP_FCA:   return "Failed to Find Calibration Parameters.";
        case RP_RCA:   return "Failed to Read Calibration Parameters.";
        case RP_BTS:   return "Buffer too small";
        case RP_EIPV:  return "Invalid parameter value";
        case RP_EUF:   return "Unsupported Feature";
        case RP_ENN:   return "Data not normalized";
        case RP_EFOB:  return "Failed to open bus";
        case RP_EFCB:  return "Failed to close bus";
        case RP_EABA:  return "Failed to acquire bus access";
        case RP_EFRB:  return "Failed to read from the bus";
        case RP_EFWB:  return "Failed to write to the bus";
        case RP_EMNC:  return "Extension module not connected";
        case RP_NOTS:  return "Command not supported ";
        default:       return "Unknown error";
    }
}


/**
 * Identification
 */

int rp_IdGetID(uint32_t *id) {
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            housekeeping_control_v1_t *hk_v1 = (housekeeping_control_v1_t*)hk;
            *id = ioread32(&hk_v1->id);
            return RP_OK;
        }
        case HK_V2:{
            housekeeping_control_v2_t *hk_v2 = (housekeeping_control_v2_t*)hk;
            *id = ioread32(&hk_v2->id);
            return RP_OK;
        }
        case HK_V3:{
            housekeeping_control_v3_t *hk_v3 = (housekeeping_control_v3_t*)hk;
            *id = ioread32(&hk_v3->id);
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
    return RP_NOTS;
}

int rp_IdGetDNA(uint64_t *dna) {
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            housekeeping_control_v1_t *hk_v1 = (housekeeping_control_v1_t*)hk;
            *dna = ((uint64_t) ioread32(&hk_v1->dna_hi) << 32)
                 | ((uint64_t) ioread32(&hk_v1->dna_lo) <<  0);
            return RP_OK;
        }
        case HK_V2:{
            housekeeping_control_v2_t *hk_v2 = (housekeeping_control_v2_t*)hk;
            *dna = ((uint64_t) ioread32(&hk_v2->dna_hi) << 32)
                 | ((uint64_t) ioread32(&hk_v2->dna_lo) <<  0);
            return RP_OK;
        }
        case HK_V3:{
            housekeeping_control_v3_t *hk_v3 = (housekeeping_control_v3_t*)hk;
            *dna = ((uint64_t) ioread32(&hk_v3->dna_hi) << 32)
                 | ((uint64_t) ioread32(&hk_v3->dna_lo) <<  0);
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
    return RP_NOTS;
}

/**
 * LED methods
 */

int rp_LEDSetState(uint32_t state) {
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            housekeeping_control_v1_t *hk_v1 = (housekeeping_control_v1_t*)hk;
            iowrite32(state, &hk_v1->led_control);
            return RP_OK;
        }
        case HK_V2:{
            housekeeping_control_v2_t *hk_v2 = (housekeeping_control_v2_t*)hk;
            iowrite32(state, &hk_v2->led_control);
            return RP_OK;
        }
        case HK_V3:{
            housekeeping_control_v3_t *hk_v3 = (housekeeping_control_v3_t*)hk;
            iowrite32(state, &hk_v3->led_control);
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
    return RP_NOTS;
}

int rp_LEDGetState(uint32_t *state) {
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            housekeeping_control_v1_t *hk_v1 = (housekeeping_control_v1_t*)hk;
            *state = ioread32(&hk_v1->led_control);
            return RP_OK;
        }
        case HK_V2:{
            housekeeping_control_v2_t *hk_v2 = (housekeeping_control_v2_t*)hk;
            *state = ioread32(&hk_v2->led_control);
            return RP_OK;
        }
        case HK_V3:{
            housekeeping_control_v3_t *hk_v3 = (housekeeping_control_v3_t*)hk;
            *state = ioread32(&hk_v3->led_control);
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
    return RP_NOTS;
}

/**
 * GPIO methods
 */

int rp_GPIOnSetDirection(uint32_t direction) {
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            housekeeping_control_v1_t *hk_v1 = (housekeeping_control_v1_t*)hk;
            iowrite32(direction, &hk_v1->ex_cd_n);
            return RP_OK;
        }
        case HK_V2:{
            housekeeping_control_v2_t *hk_v2 = (housekeeping_control_v2_t*)hk;
            iowrite32(direction, &hk_v2->ex_cd_n);
            return RP_OK;
        }
        case HK_V3:{
            housekeeping_control_v3_t *hk_v3 = (housekeeping_control_v3_t*)hk;
            iowrite32(direction, &hk_v3->ex_cd_n);
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
    return RP_NOTS;
}

int rp_GPIOnGetDirection(uint32_t *direction) {
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            housekeeping_control_v1_t *hk_v1 = (housekeeping_control_v1_t*)hk;
            *direction = ioread32(&hk_v1->ex_cd_n);
            return RP_OK;
        }
        case HK_V2:{
            housekeeping_control_v2_t *hk_v2 = (housekeeping_control_v2_t*)hk;
            *direction = ioread32(&hk_v2->ex_cd_n);
            return RP_OK;
        }
        case HK_V3:{
            housekeeping_control_v3_t *hk_v3 = (housekeeping_control_v3_t*)hk;
            *direction = ioread32(&hk_v3->ex_cd_n);
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
    return RP_NOTS;
}

int rp_GPIOnSetState(uint32_t state) {
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            housekeeping_control_v1_t *hk_v1 = (housekeeping_control_v1_t*)hk;
            iowrite32(state, &hk_v1->ex_co_n);
            return RP_OK;
        }
        case HK_V2:{
            housekeeping_control_v2_t *hk_v2 = (housekeeping_control_v2_t*)hk;
            iowrite32(state, &hk_v2->ex_co_n);
            return RP_OK;
        }
        case HK_V3:{
            housekeeping_control_v3_t *hk_v3 = (housekeeping_control_v3_t*)hk;
            iowrite32(state, &hk_v3->ex_co_n);
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
    return RP_NOTS;
}

int rp_GPIOnGetState(uint32_t *state) {
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            housekeeping_control_v1_t *hk_v1 = (housekeeping_control_v1_t*)hk;
            *state = ioread32(&hk_v1->ex_ci_n);
            return RP_OK;
        }
        case HK_V2:{
            housekeeping_control_v2_t *hk_v2 = (housekeeping_control_v2_t*)hk;
            *state = ioread32(&hk_v2->ex_ci_n);
            return RP_OK;
        }
        case HK_V3:{
            housekeeping_control_v3_t *hk_v3 = (housekeeping_control_v3_t*)hk;
            *state = ioread32(&hk_v3->ex_ci_n);
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
    return RP_NOTS;
}

int rp_GPIOpSetDirection(uint32_t direction) {
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            housekeeping_control_v1_t *hk_v1 = (housekeeping_control_v1_t*)hk;
            iowrite32(direction, &hk_v1->ex_cd_p);
            return RP_OK;
        }
        case HK_V2:{
            housekeeping_control_v2_t *hk_v2 = (housekeeping_control_v2_t*)hk;
            iowrite32(direction, &hk_v2->ex_cd_p);
            return RP_OK;
        }
        case HK_V3:{
            housekeeping_control_v3_t *hk_v3 = (housekeeping_control_v3_t*)hk;
            iowrite32(direction, &hk_v3->ex_cd_p);
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
    return RP_NOTS;
}

int rp_GPIOpGetDirection(uint32_t *direction) {
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            housekeeping_control_v1_t *hk_v1 = (housekeeping_control_v1_t*)hk;
            *direction = ioread32(&hk_v1->ex_cd_p);
            return RP_OK;
        }
        case HK_V2:{
            housekeeping_control_v2_t *hk_v2 = (housekeeping_control_v2_t*)hk;
            *direction = ioread32(&hk_v2->ex_cd_p);
            return RP_OK;
        }
        case HK_V3:{
            housekeeping_control_v3_t *hk_v3 = (housekeeping_control_v3_t*)hk;
            *direction = ioread32(&hk_v3->ex_cd_p);
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
    return RP_NOTS;
}

int rp_GPIOpSetState(uint32_t state) {
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            housekeeping_control_v1_t *hk_v1 = (housekeeping_control_v1_t*)hk;
            iowrite32(state, &hk_v1->ex_co_p);
            return RP_OK;
        }
        case HK_V2:{
            housekeeping_control_v2_t *hk_v2 = (housekeeping_control_v2_t*)hk;
            iowrite32(state, &hk_v2->ex_co_p);
            return RP_OK;
        }
        case HK_V3:{
            housekeeping_control_v3_t *hk_v3 = (housekeeping_control_v3_t*)hk;
            iowrite32(state, &hk_v3->ex_co_p);
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
    return RP_NOTS;
}

int rp_GPIOpGetState(uint32_t *state) {
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            housekeeping_control_v1_t *hk_v1 = (housekeeping_control_v1_t*)hk;
            *state = ioread32(&hk_v1->ex_ci_p);
            return RP_OK;
        }
        case HK_V2:{
            housekeeping_control_v2_t *hk_v2 = (housekeeping_control_v2_t*)hk;
            *state = ioread32(&hk_v2->ex_ci_p);
            return RP_OK;
        }
        case HK_V3:{
            housekeeping_control_v3_t *hk_v3 = (housekeeping_control_v3_t*)hk;
            *state = ioread32(&hk_v3->ex_ci_p);
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
    return RP_NOTS;
}

/**
 * Digital Pin Input Output methods
 */

int rp_DpinReset() {
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            housekeeping_control_v1_t *hk_v1 = (housekeeping_control_v1_t*)hk;
            iowrite32(0, &hk_v1->ex_cd_p);
            iowrite32(0, &hk_v1->ex_cd_n);
            iowrite32(0, &hk_v1->ex_co_p);
            iowrite32(0, &hk_v1->ex_co_n);
            iowrite32(0, &hk_v1->led_control);
            iowrite32(0, &hk_v1->digital_loop);
            return RP_OK;
        }
        case HK_V2:{
            housekeeping_control_v2_t *hk_v2 = (housekeeping_control_v2_t*)hk;
            iowrite32(0, &hk_v2->ex_cd_p);
            iowrite32(0, &hk_v2->ex_cd_n);
            iowrite32(0, &hk_v2->ex_co_p);
            iowrite32(0, &hk_v2->ex_co_n);
            iowrite32(0, &hk_v2->led_control);
            iowrite32(0, &hk_v2->digital_loop);
            return RP_OK;
        }
        case HK_V3:{
            housekeeping_control_v3_t *hk_v3 = (housekeeping_control_v3_t*)hk;
            iowrite32(0, &hk_v3->ex_cd_p);
            iowrite32(0, &hk_v3->ex_cd_n);
            iowrite32(0, &hk_v3->ex_co_p);
            iowrite32(0, &hk_v3->ex_co_n);
            iowrite32(0, &hk_v3->led_control);
            iowrite32(0, &hk_v3->digital_loop);
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
    return RP_NOTS;
}

int rp_DpinSetDirection(rp_dpin_t pin, rp_pinDirection_t direction) {
    uint32_t tmp;
    if (pin < RP_DIO0_P) {
        // LEDS
        if (direction == RP_OUT)  return RP_OK;
        else                      return RP_ELID;
    } else if (pin < RP_DIO0_N) {
        // DIO_P
        pin -= RP_DIO0_P;
        rp_GPIOpGetDirection(&tmp);
        rp_GPIOpSetDirection((tmp & ~(1 << pin)) | ((direction << pin) & (1 << pin)));
    } else {
        // DIO_N
        pin -= RP_DIO0_N;
        rp_GPIOnGetDirection(&tmp);
        rp_GPIOnSetDirection((tmp & ~(1 << pin)) | ((direction << pin) & (1 << pin)));
   }
    return RP_OK;
}

int rp_DpinGetDirection(rp_dpin_t pin, rp_pinDirection_t* direction) {
    uint32_t tmp;
    if (pin < RP_DIO0_P) {
        // LEDS
        *direction = RP_OUT;
    } else if (pin < RP_DIO0_N) {
        // DIO_P
        pin -= RP_DIO0_P;
        rp_GPIOpGetDirection(&tmp);
        *direction = (tmp >> pin) & 0x1;
    } else {
        // DIO_N
        pin -= RP_DIO0_N;
        rp_GPIOnGetDirection(&tmp);
        *direction = (tmp >> pin) & 0x1;
    }
    return RP_OK;
}

int rp_DpinSetState(rp_dpin_t pin, rp_pinState_t state) {
    uint32_t tmp;
    rp_pinDirection_t direction;
    rp_DpinGetDirection(pin, &direction);
    if (!direction) {
        return RP_EWIP;
    }
    if (pin < RP_DIO0_P) {
        // LEDS
        rp_LEDGetState(&tmp);
        rp_LEDSetState((tmp & ~(1 << pin)) | ((state << pin) & (1 << pin)));
    } else if (pin < RP_DIO0_N) {
        // DIO_P
        pin -= RP_DIO0_P;
        rp_GPIOpGetState(&tmp);
        rp_GPIOpSetState((tmp & ~(1 << pin)) | ((state << pin) & (1 << pin)));
    } else {
        // DIO_N
        pin -= RP_DIO0_N;
        rp_GPIOnGetState(&tmp);
        rp_GPIOnSetState((tmp & ~(1 << pin)) | ((state << pin) & (1 << pin)));
    }
    return RP_OK;
}

int rp_DpinGetState(rp_dpin_t pin, rp_pinState_t* state) {
    uint32_t tmp;
    if (pin < RP_DIO0_P) {
        // LEDS
        rp_LEDGetState(&tmp);
        *state = (tmp >> pin) & 0x1;
    } else if (pin < RP_DIO0_N) {
        // DIO_P
        pin -= RP_DIO0_P;
        rp_GPIOpGetState(&tmp);
        *state = (tmp >> pin) & 0x1;
    } else {
        // DIO_N
        pin -= RP_DIO0_N;
        rp_GPIOnGetState(&tmp);
        *state = (tmp >> pin) & 0x1;
    }
    return RP_OK;
}


/**
 * Digital loop
 */

int rp_EnableDigitalLoop(bool enable) {
    hk_version_t ver = house_getHKVersion();
    switch (ver)
    {
        case HK_V1:{
            housekeeping_control_v1_t *hk_v1 = (housekeeping_control_v1_t*)hk;
            iowrite32((uint32_t) enable, &hk_v1->digital_loop);
            return RP_OK;
        }
        case HK_V2:{
            housekeeping_control_v2_t *hk_v2 = (housekeeping_control_v2_t*)hk;
            iowrite32((uint32_t) enable, &hk_v2->digital_loop);
            return RP_OK;
        }
        case HK_V3:{
            housekeeping_control_v3_t *hk_v3 = (housekeeping_control_v3_t*)hk;
            iowrite32((uint32_t) enable, &hk_v3->digital_loop);
            return RP_OK;
        }
        default:
            return RP_NOTS;
    }
    return RP_NOTS;
}


/** @name Analog Inputs/Outputs
 */
///@{

int rp_ApinReset() {
    return rp_AOpinReset();
}

int rp_ApinGetValue(rp_apin_t pin, float* value, uint32_t* raw) {
    if (pin <= RP_AOUT3) {
        return rp_AOpinGetValue(pin-RP_AOUT0, value, raw);
    } else if (pin <= RP_AIN3) {
        return rp_AIpinGetValue(pin-RP_AIN0, value, raw);
    }
    return RP_EPN;
}

int rp_ApinGetValueRaw(rp_apin_t pin, uint32_t* value) {
    if (pin <= RP_AOUT3) {
        return rp_AOpinGetValueRaw(pin-RP_AOUT0, value);
    } else if (pin <= RP_AIN3) {
        return rp_AIpinGetValueRaw(pin-RP_AIN0, value);
    }
    return RP_EPN;
}

int rp_ApinSetValue(rp_apin_t pin, float value) {
    if (pin <= RP_AOUT3) {
        return rp_AOpinSetValue(pin-RP_AOUT0, value);
    }
    return RP_EPN;
}

int rp_ApinSetValueRaw(rp_apin_t pin, uint32_t value) {
    if (pin <= RP_AOUT3) {
        return rp_AOpinSetValueRaw(pin-RP_AOUT0, value);
    }
    return RP_EPN;
}

int rp_ApinGetRange(rp_apin_t pin, float* min_val, float* max_val) {
    rp_channel_calib_t ch = convertPINCh(pin);
    if (pin <= RP_AOUT3) {
        float fs = rp_HPGetSlowDACFullScaleOrDefault(ch);
        *min_val = rp_HPGetSlowDACIsSignedOrDefault(ch) ? -fs: 0;
        *max_val = fs;
        return RP_OK;
    }

    if (pin <= RP_AIN3) {
        float fs = rp_HPGetSlowADCFullScaleOrDefault(ch);
        *min_val = rp_HPGetSlowADCIsSignedOrDefault(ch) ? -fs: 0;
        *max_val = fs;
        return RP_OK;
    }
    return RP_EPN;
}


/**
 * Analog Inputs
 */

int rp_AIpinGetValueRaw(int unsigned pin, uint32_t* value) {
    FILE *fp;
    switch (pin) {
        case 0:  fp = fopen ("/sys/devices/soc0/axi/83c00000.xadc_wiz/iio:device1/in_voltage9_raw", "r");  break;
        case 1:  fp = fopen ("/sys/devices/soc0/axi/83c00000.xadc_wiz/iio:device1/in_voltage11_raw" , "r");  break;
        case 2:  fp = fopen ("/sys/devices/soc0/axi/83c00000.xadc_wiz/iio:device1/in_voltage10_raw", "r");  break;
        case 3:  fp = fopen ("/sys/devices/soc0/axi/83c00000.xadc_wiz/iio:device1/in_voltage8_raw", "r");  break;
        case 4:  fp = fopen ("/sys/devices/soc0/axi/83c00000.xadc_wiz/iio:device1/in_voltage12_raw", "r");  break;
        default:
            return RP_EPN;
    }
    int r = !fscanf (fp, "%u", value);
    fclose(fp);
    return r;
}

int rp_AIpinGetValue(int unsigned pin, float* value, uint32_t* raw) {
    uint32_t value_raw;
    int result = rp_AIpinGetValueRaw(pin, &value_raw);
    rp_channel_calib_t ch = convertPINCh(pin);

    float fs = 0;
    if (rp_HPGetSlowADCFullScale(ch,&fs) != RP_HP_OK){
        ERROR("Can't get slow ADC full scale");
        return RP_EOOR;
    }

    bool is_signed = false;
    if (rp_HPGetSlowADCIsSigned(ch,&is_signed) != RP_HP_OK){
        ERROR("Can't get slow ADC sign state");
        return RP_EOOR;
    }

    uint8_t bits = 0;
    if (rp_HPGetSlowADCBits(ch,&bits) != RP_HP_OK){
        ERROR("Can't get slow ADC bits");
        return RP_EOOR;
    }
    if (is_signed){
        *value = cmn_convertToVoltSigned(value_raw,bits,fs,1000,1000,0);
    }
    else{
        *value = cmn_convertToVoltUnsigned(value_raw,bits,fs,1000,1000,0);
    }
    *raw = value_raw;
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

    rp_channel_calib_t ch = convertPINCh(pin);

    uint8_t bits = 0;
    if (rp_HPGetSlowADCBits(ch,&bits) != RP_HP_OK){
        ERROR("Can't get slow DAC bits");
        return RP_EOOR;
    }
    uint32_t max_value = (1 << bits);
    uint32_t mask = max_value - 1;
    if (value >= max_value) {
        return RP_EOOR;
    }
    iowrite32((value & mask) << 16, &ams->dac[pin]);
    return RP_OK;
}

int rp_AOpinSetValue(int unsigned pin, float value) {
    rp_channel_calib_t ch = convertPINCh(pin);

    float fs = 0;
    if (rp_HPGetSlowDACFullScale(ch,&fs) != RP_HP_OK){
        ERROR("Can't get slow ADC full scale");
        return RP_EOOR;
    }

    bool is_signed = false;
    if (rp_HPGetSlowDACIsSigned(ch,&is_signed) != RP_HP_OK){
        ERROR("Can't get slow ADC sign state");
        return RP_EOOR;
    }

    uint8_t bits = 0;
    if (rp_HPGetSlowDACBits(ch,&bits) != RP_HP_OK){
        ERROR("Can't get slow ADC bits");
        return RP_EOOR;
    }

    uint32_t value_raw = cmn_convertToCnt(value,bits,fs,is_signed,1,0);
    return rp_AOpinSetValueRaw(pin, value_raw);
}

int rp_AOpinGetValueRaw(int unsigned pin, uint32_t* value) {
    if (pin >= 4) {
        return RP_EPN;
    }

    rp_channel_calib_t ch = convertPINCh(pin);

    uint8_t bits = 0;
    if (rp_HPGetSlowADCBits(ch,&bits) != RP_HP_OK){
        ERROR("Can't get slow DAC bits");
        return RP_EOOR;
    }
    uint32_t max_value = (1 << bits);
    uint32_t mask = max_value - 1;

    *value = (ioread32(&ams->dac[pin]) >> 16) & mask;
    return RP_OK;
}

int rp_AOpinGetValue(int unsigned pin, float* value, uint32_t* raw) {

    uint32_t value_raw;
    int result = rp_AOpinGetValueRaw(pin, &value_raw);
    rp_channel_calib_t ch = convertPINCh(pin);

    float fs = 0;
    if (rp_HPGetSlowDACFullScale(ch,&fs) != RP_HP_OK){
        ERROR("Can't get slow ADC full scale");
        return RP_EOOR;
    }

    bool is_signed = false;
    if (rp_HPGetSlowDACIsSigned(ch,&is_signed) != RP_HP_OK){
        ERROR("Can't get slow ADC sign state");
        return RP_EOOR;
    }

    uint8_t bits = 0;
    if (rp_HPGetSlowDACBits(ch,&bits) != RP_HP_OK){
        ERROR("Can't get slow ADC bits");
        return RP_EOOR;
    }
    if (is_signed){
        *value = cmn_convertToVoltSigned(value_raw,bits,fs,1000,1000,0);
    }
    else{
        *value = cmn_convertToVoltUnsigned(value_raw,bits,fs,1000,1000,0);
    }
    *raw = value_raw;
    return result;
}

int rp_AOpinGetRange(int unsigned pin, float* min_val,  float* max_val) {
    rp_channel_calib_t ch = convertPINCh(pin);
    if (pin <= RP_AOUT3) {
        float fs = rp_HPGetSlowDACFullScaleOrDefault(ch);
        *min_val = rp_HPGetSlowDACIsSignedOrDefault(ch) ? -fs: 0;
        *max_val = fs;
        return RP_OK;
    }
    return RP_EPN;
}


/**
 * Acquire methods
 */

int rp_AcqSetArmKeep(bool enable)
{
    return acq_SetArmKeep(enable);
}

int rp_AcqGetArmKeep(bool* state){
    return acq_GetArmKeep(state);
}

int rp_AcqGetBufferFillState(bool* state){
    return acq_GetBufferFillState(state);
}

int rp_AcqAxiGetBufferFillState(rp_channel_t channel, bool* state){
    if (!rp_HPGetIsDMAinv0_94OrDefault())
        return RP_NOTS;
    return acq_axi_GetBufferFillState(channel, state);
}

int rp_AcqSetDecimation(rp_acq_decimation_t decimation)
{
    return acq_SetDecimation(decimation);
}

int rp_AcqGetDecimation(rp_acq_decimation_t* decimation)
{
    return acq_GetDecimation(decimation);
}

int rp_AcqSetDecimationFactor(uint32_t decimation)
{
    return acq_SetDecimationFactor(decimation);
}

int rp_AcqAxiSetDecimationFactor(uint32_t decimation){
    if (!rp_HPGetIsDMAinv0_94OrDefault())
        return RP_NOTS;
    return acq_axi_SetDecimationFactor(decimation);
}

int rp_AcqAxiGetDecimationFactor(uint32_t *decimation){
    if (!rp_HPGetIsDMAinv0_94OrDefault())
        return RP_NOTS;
    return acq_axi_GetDecimationFactor(decimation);
}

int rp_AcqGetDecimationFactor(uint32_t* decimation)
{
    return acq_GetDecimationFactor(decimation);
}

int rp_AcqConvertFactorToDecimation(uint32_t factor,rp_acq_decimation_t* decimation){
    return acq_ConvertFactorToDecimation(factor,decimation);
}

int rp_AcqGetSamplingRateHz(float* sampling_rate)
{
    return acq_GetSamplingRateHz(sampling_rate);
}

int rp_AcqSetAveraging(bool enabled)
{
    return acq_SetAveraging(enabled);
}

int rp_AcqGetAveraging(bool *enabled)
{
    return acq_GetAveraging(enabled);
}

int rp_AcqSetTriggerSrc(rp_acq_trig_src_t source)
{
    return acq_SetTriggerSrc(source);
}

int rp_AcqGetTriggerSrc(rp_acq_trig_src_t* source)
{
    return acq_GetTriggerSrc(source);
}

int rp_AcqGetTriggerState(rp_acq_trig_state_t* state)
{
    return acq_GetTriggerState(state);
}

int rp_AcqSetTriggerDelay(int32_t decimated_data_num)
{
    return acq_SetTriggerDelay(decimated_data_num);
}

int rp_AcqSetTriggerDelayDirect(uint32_t decimated_data_num)
{
    return acq_SetTriggerDelayDirect(decimated_data_num);
}

int rp_AcqAxiSetTriggerDelay(rp_channel_t channel, int32_t decimated_data_num){
    if (!rp_HPGetIsDMAinv0_94OrDefault())
        return RP_NOTS;
    return acq_axi_SetTriggerDelay(channel, decimated_data_num);
}

int rp_AcqAxiGetTriggerDelay(rp_channel_t channel, int32_t *decimated_data_num){
    if (!rp_HPGetIsDMAinv0_94OrDefault())
        return RP_NOTS;
    return acq_axi_GetTriggerDelay(channel, decimated_data_num);
}

int rp_AcqGetTriggerDelay(int32_t* decimated_data_num)
{
    return acq_GetTriggerDelay(decimated_data_num);
}

int rp_AcqGetTriggerDelayDirect(uint32_t* decimated_data_num)
{
    return acq_GetTriggerDelayDirect(decimated_data_num);
}

int rp_AcqSetTriggerDelayNs(int64_t time_ns)
{
    return acq_SetTriggerDelayNs(time_ns);
}

int rp_AcqGetTriggerDelayNs(int64_t* time_ns)
{
    return acq_GetTriggerDelayNs(time_ns);
}

int rp_AcqSetTriggerDelayNsDirect(uint64_t time_ns)
{
    return acq_SetTriggerDelayNsDirect(time_ns);
}

int rp_AcqGetTriggerDelayNsDirect(uint64_t* time_ns)
{
    return acq_GetTriggerDelayNsDirect(time_ns);
}

int rp_AcqGetPreTriggerCounter(uint32_t* value) {
    return acq_GetPreTriggerCounter(value);
}

int rp_AcqGetGain(rp_channel_t channel, rp_pinState_t* state)
{
    return acq_GetGain(channel, state);
}

int rp_AcqGetGainV(rp_channel_t channel, float* voltage)
{
    return acq_GetGainV(channel, voltage);
}

int rp_AcqSetGain(rp_channel_t channel, rp_pinState_t state)
{
    return acq_SetGain(channel, state);
}

int rp_AcqGetTriggerLevel(rp_channel_trigger_t channel, float* voltage)
{
    return acq_GetTriggerLevel(channel,voltage);
}

int rp_AcqSetTriggerLevel(rp_channel_trigger_t channel, float voltage)
{
    return acq_SetTriggerLevel(channel, voltage);
}

int rp_AcqGetTriggerHyst(float* voltage)
{
    return acq_GetTriggerHyst(voltage);
}

int rp_AcqSetTriggerHyst(float voltage)
{
    return acq_SetTriggerHyst(voltage);
}

int rp_AcqGetWritePointer(uint32_t* pos)
{
    return acq_GetWritePointer(pos);
}

int rp_AcqAxiGetWritePointer(rp_channel_t channel, uint32_t* pos){
    if (!rp_HPGetIsDMAinv0_94OrDefault())
        return RP_NOTS;
    return acq_axi_GetWritePointer(channel, pos);
}

int rp_AcqGetWritePointerAtTrig(uint32_t* pos)
{
    return acq_GetWritePointerAtTrig(pos);
}

int rp_AcqAxiGetWritePointerAtTrig(rp_channel_t channel, uint32_t* pos){
    if (!rp_HPGetIsDMAinv0_94OrDefault())
        return RP_NOTS;
    return acq_axi_GetWritePointerAtTrig(channel, pos);
}

int rp_AcqStart()
{
    return acq_Start();
}

int rp_AcqStop()
{
    return acq_Stop();
}

int rp_AcqUnlockTrigger()
{
    return acq_SetUnlockTrigger();
}

int rp_AcqGetUnlockTrigger(bool *state){
    return acq_GetUnlockTrigger(state);
}

int rp_AcqReset()
{
    return acq_Reset();
}

int rp_AcqResetFpga()
{
    return acq_ResetFpga();
}

uint32_t rp_AcqGetNormalizedDataPos(uint32_t pos)
{
    return acq_GetNormalizedDataPos(pos);
}

int rp_AcqGetDataPosRaw(rp_channel_t channel, uint32_t start_pos, uint32_t end_pos, int16_t* buffer, uint32_t* buffer_size)
{
    return acq_GetDataPosRaw(channel, start_pos, end_pos, buffer, buffer_size);
}

int rp_AcqGetDataPosRawNP(rp_channel_t channel, uint32_t start_pos, uint32_t end_pos, int16_t* np_buffer, int buffer_size)
{
    uint32_t size = buffer_size;
    return acq_GetDataPosRaw(channel, start_pos, end_pos, np_buffer, &size);
}

int rp_AcqGetDataPosV(rp_channel_t channel, uint32_t start_pos, uint32_t end_pos, float* buffer, uint32_t* buffer_size)
{
    return acq_GetDataPosV(channel, start_pos, end_pos, buffer, buffer_size);
}

int rp_AcqGetDataPosVNP(rp_channel_t channel, uint32_t start_pos, uint32_t end_pos, float* np_buffer, int buffer_size){
    uint32_t size = buffer_size;
    return acq_GetDataPosV(channel, start_pos, end_pos, np_buffer, &size);
}

int rp_AcqAxiGetMemoryRegion(uint32_t *_start,uint32_t *_size){
    if (!rp_HPGetIsDMAinv0_94OrDefault())
        return RP_NOTS;
    return acq_axi_GetMemoryRegion(_start,_size);
}

int rp_AcqAxiEnable(rp_channel_t channel, bool enable){
    if (!rp_HPGetIsDMAinv0_94OrDefault())
        return RP_NOTS;
    return acq_axi_Enable(channel, enable);
}

int rp_AcqGetDataRaw(rp_channel_t channel,  uint32_t pos, uint32_t* size, int16_t* buffer)
{
    return acq_GetDataRaw(channel, pos, size, buffer,false);
}

int rp_AcqGetDataRawNP(rp_channel_t channel,  uint32_t pos, int16_t* np_buffer, int size)
{
    uint32_t usize = size;
    return acq_GetDataRaw(channel, pos, &usize, np_buffer,false);
}

int rp_AcqGetDataRawWithCalib(rp_channel_t channel,  uint32_t pos, uint32_t* size, int16_t* buffer){
    return acq_GetDataRaw(channel, pos, size, buffer,true);
}

int rp_AcqGetDataRawWithCalibNP(rp_channel_t channel,  uint32_t pos, int16_t* np_buffer, int buffer_size){
    uint32_t size = buffer_size;
    return acq_GetDataRaw(channel, pos, &size, np_buffer,true);
}

int rp_AcqAxiGetDataRaw(rp_channel_t channel,  uint32_t pos, uint32_t* size, int16_t* buffer){
    if (!rp_HPGetIsDMAinv0_94OrDefault())
        return RP_NOTS;
    return acq_axi_GetDataRaw(channel, pos, size, buffer);
}

int rp_AcqAxiGetDataRawNP(rp_channel_t channel,  uint32_t pos, int16_t* np_buffer, int size){
    if (!rp_HPGetIsDMAinv0_94OrDefault())
        return RP_NOTS;
    uint32_t usize = size;
    return acq_axi_GetDataRaw(channel, pos, &usize, np_buffer);
}

int rp_AcqGetData(uint32_t pos, buffers_t *out)
{
    return acq_GetData(pos, out);
}

int rp_AcqGetOldestDataRaw(rp_channel_t channel, uint32_t* size, int16_t* buffer)
{
    return acq_GetOldestDataRaw(channel, size, buffer);
}

int rp_AcqGetOldestDataRawNP(rp_channel_t channel, int16_t* buff, int size)
{
    uint32_t usize = size;
    return acq_GetOldestDataRaw(channel, &usize, buff);
}

int rp_AcqGetLatestDataRaw(rp_channel_t channel, uint32_t* size, int16_t* buffer)
{
    return acq_GetLatestDataRaw(channel, size, buffer);
}

int rp_AcqGetLatestDataRawNP(rp_channel_t channel, int16_t* np_buffer, int size)
{
    uint32_t usize = size;
    return acq_GetLatestDataRaw(channel, &usize,np_buffer);
}

int rp_AcqGetDataV(rp_channel_t channel, uint32_t pos, uint32_t* size, float* buffer)
{
    return acq_GetDataV(channel, pos, size, buffer);
}

int rp_AcqGetDataVNP(rp_channel_t channel, uint32_t pos, float* np_buffer, int size)
{
    uint32_t usize = size;
    return acq_GetDataV(channel, pos, &usize, np_buffer);
}

int rp_AcqAxiGetDataV(rp_channel_t channel, uint32_t pos, uint32_t* size, float* buffer){
    if (!rp_HPGetIsDMAinv0_94OrDefault())
        return RP_NOTS;
    return acq_axi_GetDataV(channel, pos, size, buffer);
}

int rp_AcqAxiGetDataVNP(rp_channel_t channel, uint32_t pos, float* np_buffer, int size){
    if (!rp_HPGetIsDMAinv0_94OrDefault())
        return RP_NOTS;
    uint32_t usize = size;
    return acq_axi_GetDataV(channel, pos, &usize, np_buffer);
}

int rp_AcqGetOldestDataV(rp_channel_t channel, uint32_t* size, float* buffer)
{
    return acq_GetOldestDataV(channel, size, buffer);
}

int rp_AcqGetOldestDataVNP(rp_channel_t channel, float* np_buffer, int size)
{
    uint32_t usize = size;
    return acq_GetOldestDataV(channel, &usize, np_buffer);
}

int rp_AcqGetLatestDataV(rp_channel_t channel, uint32_t* size, float* buffer)
{
    return acq_GetLatestDataV(channel, size, buffer);
}

int rp_AcqGetLatestDataVNP(rp_channel_t channel, float* np_buffer, int size)
{
    uint32_t usize = size;
    return acq_GetLatestDataV(channel, &usize, np_buffer);
}

int rp_AcqGetBufSize(uint32_t *size) {
    return acq_GetBufferSize(size);
}

int rp_AcqAxiSetBufferSamples(rp_channel_t channel, uint32_t address, uint32_t samples) {
    if (!rp_HPGetIsDMAinv0_94OrDefault())
        return RP_NOTS;
    return acq_axi_SetBufferSamples(channel, address, samples);
}

int rp_AcqAxiSetBufferBytes(rp_channel_t channel, uint32_t address, uint32_t size) {
    if (!rp_HPGetIsDMAinv0_94OrDefault())
        return RP_NOTS;
    return acq_axi_SetBufferBytes(channel, address, size);
}

int rp_AcqSetAC_DC(rp_channel_t channel,rp_acq_ac_dc_mode_t mode){
    if (!rp_HPGetFastADCIsAC_DCOrDefault())
        return RP_NOTS;
    return acq_SetAC_DC(channel,mode);
}

int rp_AcqGetAC_DC(rp_channel_t channel,rp_acq_ac_dc_mode_t *status){
    if (!rp_HPGetFastADCIsAC_DCOrDefault())
        return RP_NOTS;
    return acq_GetAC_DC(channel,status);
}

int rp_AcqUpdateAcqFilter(rp_channel_t channel){
    if (!rp_HPGetFastADCIsFilterPresentOrDefault())
        return RP_NOTS;
    return acq_UpdateAcqFilter(channel);
}

int rp_AcqGetFilterCalibValue(rp_channel_t channel,uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp){
    if (!rp_HPGetFastADCIsFilterPresentOrDefault())
        return RP_NOTS;
    return acq_GetFilterCalibValue( channel,coef_aa, coef_bb, coef_kk, coef_pp);
}

/**
* Generate methods
*/

int rp_GenBurstLastValue(rp_channel_t channel, float amlitude){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setBurstLastValue(channel,amlitude);
}

int rp_GenGetBurstLastValue(rp_channel_t channel, float *amlitude){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getBurstLastValue(channel,amlitude);
}

int rp_GenSetInitGenValue(rp_channel_t channel, float amlitude){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setInitGenValue(channel,amlitude);
}

int rp_GenGetInitGenValue(rp_channel_t channel, float *amlitude){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getInitGenValue(channel,amlitude);
}

int rp_GenReset() {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_SetDefaultValues();
}

int rp_GenOutDisable(rp_channel_t channel) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_Disable(channel);
}

int rp_GenOutEnable(rp_channel_t channel) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_Enable(channel);
}

int rp_GenOutIsEnabled(rp_channel_t channel, bool *value) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_IsEnable(channel, value);
}

int rp_GenAmp(rp_channel_t channel, float amplitude) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setAmplitude(channel, amplitude);
}

int rp_GenGetAmp(rp_channel_t channel, float *amplitude) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getAmplitude(channel, amplitude);
}

int rp_GenOffset(rp_channel_t channel, float offset) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setOffset(channel, offset);
}

int rp_GenGetOffset(rp_channel_t channel, float *offset) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getOffset(channel, offset);
}

int rp_GenFreq(rp_channel_t channel, float frequency) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setFrequency(channel, frequency);
}

int rp_GenFreqDirect(rp_channel_t channel, float frequency){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setFrequencyDirect(channel, frequency);
}

int rp_GenGetFreq(rp_channel_t channel, float *frequency) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getFrequency(channel, frequency);
}

int rp_GenSweepStartFreq(rp_channel_t channel, float frequency){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setSweepStartFrequency(channel,frequency);
}

int rp_GenGetSweepStartFreq(rp_channel_t channel, float *frequency){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getSweepStartFrequency(channel,frequency);
}

int rp_GenSweepEndFreq(rp_channel_t channel, float frequency){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setSweepEndFrequency(channel,frequency);
}

int rp_GenGetSweepEndFreq(rp_channel_t channel, float *frequency){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getSweepEndFrequency(channel,frequency);
}

int rp_GenPhase(rp_channel_t channel, float phase) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setPhase(channel, phase);
}

int rp_GenGetPhase(rp_channel_t channel, float *phase) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getPhase(channel, phase);
}

int rp_GenWaveform(rp_channel_t channel, rp_waveform_t type) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setWaveform(channel, type);
}

int rp_GenGetWaveform(rp_channel_t channel, rp_waveform_t *type) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getWaveform(channel, type);
}

int rp_GenSweepMode(rp_channel_t channel, rp_gen_sweep_mode_t mode){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setSweepMode(channel,mode);
}

int rp_GenGetSweepMode(rp_channel_t channel, rp_gen_sweep_mode_t *mode){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getSweepMode(channel,mode);
}

int rp_GenSweepDir(rp_channel_t channel, rp_gen_sweep_dir_t mode){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setSweepDir(channel,mode);
}

int rp_GenGetSweepDir(rp_channel_t channel, rp_gen_sweep_dir_t *mode){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getSweepDir(channel,mode);
}

int rp_GenArbWaveform(rp_channel_t channel, float *waveform, int size) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setArbWaveform(channel, waveform, (uint32_t)size);
}

int rp_GenArbWaveformNP(rp_channel_t channel, float *waveform, int size) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setArbWaveform(channel, waveform, (uint32_t)size);
}

int rp_GenGetArbWaveform(rp_channel_t channel, float *waveform, int size, uint32_t *size_out) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    *size_out = size;
    return gen_getArbWaveform(channel, waveform, size_out);
}

int rp_GenGetArbWaveformNP(rp_channel_t channel, float *waveform, int size, uint32_t *size_out) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    *size_out = size;
    return gen_getArbWaveform(channel, waveform, size_out);
}

int rp_GenDutyCycle(rp_channel_t channel, float ratio) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setDutyCycle(channel, ratio);
}

int rp_GenRiseTime(rp_channel_t channel, float time) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setRiseTime(channel, time);
}

int rp_GenGetRiseTime(rp_channel_t channel, float *time){
if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getRiseTime(channel, time);
}

int rp_GenFallTime(rp_channel_t channel, float time) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setFallTime(channel, time);
}

int rp_GenGetFallTime(rp_channel_t channel, float *time){
if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getFallTime(channel, time);
}

int rp_GenGetDutyCycle(rp_channel_t channel, float *ratio) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getDutyCycle(channel, ratio);
}

int rp_GenMode(rp_channel_t channel, rp_gen_mode_t mode) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setGenMode(channel, mode);
}

int rp_GenGetMode(rp_channel_t channel, rp_gen_mode_t *mode) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getGenMode(channel, mode);
}

int rp_GenBurstCount(rp_channel_t channel, int num) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setBurstCount(channel, num);
}

int rp_GenGetBurstCount(rp_channel_t channel, int *num) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getBurstCount(channel, num);
}

int rp_GenBurstRepetitions(rp_channel_t channel, int repetitions) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setBurstRepetitions(channel, repetitions);
}

int rp_GenGetBurstRepetitions(rp_channel_t channel, int *repetitions) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getBurstRepetitions(channel, repetitions);
}

int rp_GenBurstPeriod(rp_channel_t channel, uint32_t period) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setBurstPeriod(channel, period);
}

int rp_GenGetBurstPeriod(rp_channel_t channel, uint32_t *period) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getBurstPeriod(channel, period);
}

int rp_GenTriggerSource(rp_channel_t channel, rp_trig_src_t src) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_setTriggerSource(channel, src);
}

int rp_GenGetTriggerSource(rp_channel_t channel, rp_trig_src_t *src) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_getTriggerSource(channel, src);
}

int rp_GenTriggerOnly(rp_channel_t channel){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_TriggerOnly(channel);
}

int rp_GenSynchronise() {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_TriggerSync();
}

int rp_GenResetTrigger(rp_channel_t channel){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_Trigger(channel);
}

int rp_GenResetChannelSM(rp_channel_t channel){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_ResetChannelSM(channel);
}

int rp_GenOutEnableSync(bool enable){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_EnableSync(enable);
}


int rp_SetEnableTempProtection(rp_channel_t channel, bool enable){
    return gen_setEnableTempProtection(channel,enable);
}

int rp_GetEnableTempProtection(rp_channel_t channel, bool *enable){
    return gen_getEnableTempProtection(channel,enable);
}

int rp_SetLatchTempAlarm(rp_channel_t channel, bool status){
    return gen_setLatchTempAlarm(channel,status);
}

int rp_GetLatchTempAlarm(rp_channel_t channel, bool *status){
    return gen_getLatchTempAlarm(channel,status);
}

int rp_GetRuntimeTempAlarm(rp_channel_t channel, bool *status){
    return gen_getRuntimeTempAlarm(channel,status);
}

int rp_GetPllControlEnable(bool *enable){
    return house_GetPllControlEnable(enable);
}

int rp_SetPllControlEnable(bool enable){

    return house_SetPllControlEnable(enable);
}

int rp_GetPllControlLocked(bool *status){
    return house_GetPllControlLocked(status);
}

int rp_GenSetGainOut(rp_channel_t channel,rp_gen_gain_t mode){
    return gen_setGainOut(channel,mode);
}

int rp_GenGetGainOut(rp_channel_t channel,rp_gen_gain_t *status){
    return gen_getGainOut(channel,status);
}


int rp_SetEnableDaisyChainTrigSync(bool enable){
    return house_SetEnableDaisyChainSync(enable);
}

int rp_GetEnableDaisyChainTrigSync(bool *status){
    return house_GetEnableDaisyChainSync(status);
}

int rp_SetEnableDiasyChainClockSync(bool enable){

    if (!rp_HPGetIsDaisyChainClockAvailableOrDefault())
        return RP_NOTS;

    int ret = daisy_SetTXEnable(enable);
    if (ret != RP_OK){
        return ret;
    }

    ret = daisy_SetRXEnable(enable);

    if (ret != RP_OK){
        return ret;
    }

    return ret;
}

int rp_GetEnableDiasyChainClockSync(bool *state){

    if (!rp_HPGetIsDaisyChainClockAvailableOrDefault())
        return RP_NOTS;

    bool stx,srx;

    int ret = daisy_GetTXEnable(&stx);
    if (ret != RP_OK){
        return ret;
    }

    ret = daisy_GetRXEnable(&srx);

    if (ret != RP_OK){
        return ret;
    }

    *state = stx & srx;

    return ret;
}

int rp_SetDpinEnableTrigOutput(bool enable){
    return house_SetDpinEnableTrigOutput(enable);
}

int rp_GetDpinEnableTrigOutput(bool *state){
    return house_GetDpinEnableTrigOutput(state);
}

int rp_SetSourceTrigOutput(rp_outTiggerMode_t mode){
    return house_SetSourceTrigOutput(mode);
}

int rp_GetSourceTrigOutput(rp_outTiggerMode_t *mode){
    return house_GetSourceTrigOutput(mode);
}

int rp_SetCANModeEnable(bool _enable){
    return house_SetCANModeEnable(_enable);
}

int rp_GetCANModeEnable(bool *_enable){
    return house_GetCANModeEnable(_enable);
}

int rp_AcqSetExtTriggerDebouncerUs(double value){
    return acq_SetExtTriggerDebouncerUs(value);
}

int rp_AcqGetExtTriggerDebouncerUs(double *value){
    return acq_GetExtTriggerDebouncerUs(value);
}

int rp_GenSetExtTriggerDebouncerUs(double value){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_SetExtTriggerDebouncerUs(value);
}

int rp_GenGetExtTriggerDebouncerUs(double *value){
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return RP_NOTS;
    return gen_GetExtTriggerDebouncerUs(value);
}

int rp_GenSetLoadMode(rp_channel_t channel, rp_gen_load_mode_t mode){
    if (!rp_HPGetIsDAC50OhmModeOrDefault())
        return RP_NOTS;
    return gen_setLoadMode(channel,mode);
}

int rp_GenGetLoadMode(rp_channel_t channel, rp_gen_load_mode_t *mode){
    if (!rp_HPGetIsDAC50OhmModeOrDefault())
        return RP_NOTS;
    return gen_getLoadMode(channel,mode);
}

int rp_EnableDebugReg(){
    cmn_enableDebugReg();
    return RP_OK;
}

buffers_t* rp_createBuffer(uint8_t maxChannels,uint32_t length,bool initInt16, bool initDouble, bool initFloat){
    if (maxChannels > 4) {
        ERROR("The number of channels is more than allowed");
        return NULL;
    }

    buffers_t * b = malloc(sizeof(buffers_t));
    if (b == NULL) return NULL;
    b->channels = maxChannels;
    b->size = length;
    b->use_calib_for_raw = false;
    b->use_calib_for_volts = true;

    bool NeedFree = false;
    for(int i = 0 ; i < 4; i++){
        b->ch_d[i] = NULL;
        b->ch_f[i] = NULL;
        b->ch_i[i] = NULL;
        if (initInt16 && i < maxChannels){
            b->ch_i[i] = malloc(length * sizeof(int16_t));
            if (b->ch_i[i] == NULL){
                NeedFree = true;
                break;
            }
        }
        if (initDouble && i < maxChannels){
            b->ch_d[i] = malloc(length * sizeof(double));
            if (b->ch_d[i] == NULL){
                NeedFree = true;
                break;
            }
        }
        if (initFloat && i < maxChannels){
            b->ch_f[i] = malloc(length * sizeof(float));
            if (b->ch_f[i] == NULL){
                NeedFree = true;
                break;
            }
        }
    }

    if (NeedFree){
        rp_deleteBuffer(b);
        free(b);
        return NULL;
    }
    return b;
}

void rp_deleteBuffer(buffers_t *_in_buffer){
    for(int i = 0 ; i < 4; i++){
        free(_in_buffer->ch_d[i]);
        free(_in_buffer->ch_f[i]);
        free(_in_buffer->ch_i[i]);
    }
    _in_buffer->size = 0;
}

int rp_SetExternalTriggerLevel(float value){
    if (rp_HPGetIsExternalTriggerLevelPresentOrDefault()){
        float fullScale = rp_HPGetIsExternalTriggerFullScalePresentOrDefault();
        bool is_signed = rp_HPGetIsExternalTriggerIsSignedOrDefault();
        float min = (is_signed ? -fullScale : 0);
        if (value <  min || value > fullScale){
            ERROR("Value out of range %f. Min %f Max %f",value,min,fullScale)
            return RP_EOOR;
        }
        int ret = rp_setExtTriggerLevel(value);
        switch(ret){
            case RP_I2C_EOOR: return RP_EOOR;
            case RP_I2C_EFRB: return RP_EFRB;
            case RP_I2C_EFWB: return RP_EFWB;
            case RP_I2C_OK: {
                g_ext_trig_trash = value;
                return RP_OK;
            }
            default:
                return RP_EOOR;
        }
    }
    ERROR("Unsupported");
    return RP_NOTS;
}

int rp_GetExternalTriggerLevel(float *value){
    if (rp_HPGetIsExternalTriggerLevelPresentOrDefault()){
        *value = g_ext_trig_trash;
        return RP_OK;
    }
    ERROR("Unsupported");
    return RP_NOTS;
}


