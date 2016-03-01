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

#include "version.h"
#include "common.h"
#include "housekeeping.h"
#include "oscilloscope.h"
#include "acq_handler.h"
#include "analog_mixed_signals.h"
#include "calib.h"
#include "generate.h"
#include "gen_handler.h"

static char version[50];

/**
 * Global methods
 */

int rp_Init()
{
    ECHECK(cmn_Init());
	
    ECHECK(calib_Init());
    ECHECK(hk_Init());
    ECHECK(ams_Init());
    ECHECK(generate_Init());
    ECHECK(osc_Init());
    // TODO: Place other module initializations here

    // Set default configuration per handler
    ECHECK(rp_Reset());

    return RP_OK;
}

int rp_CalibInit()
{
    ECHECK(calib_Init());

    return RP_OK;
}

int rp_Release()
{
    ECHECK(osc_Release())
    ECHECK(generate_Release());
    ECHECK(ams_Release());
    ECHECK(hk_Release());
    ECHECK(calib_Release());
    ECHECK(cmn_Release());
    // TODO: Place other module releasing here (in reverse order)
    return RP_OK;
}

int rp_Reset()
{
    ECHECK(rp_DpinReset());
    ECHECK(rp_AOpinReset());
    ECHECK(rp_GenReset());
    ECHECK(rp_AcqReset());
    // TODO: Place other module resetting here (in reverse order)
    return 0;
}

const char* rp_GetVersion()
{
    sprintf(version, "%s (%s)", VERSION_STR, REVISION_STR);
    return version;
}

const char* rp_GetError(int errorCode) {
    switch (errorCode) {
        case RP_OK:
            return "OK";
        case RP_EOED:
            return "Failed to Open EEPROM Device.";
        case RP_EOMD:
            return "Failed to open memory device.";
        case RP_ECMD:
            return "Failed to close memory device.";
        case RP_EMMD:
            return "Failed to map memory device.";
        case RP_EUMD:
            return "Failed to unmap memory device.";
        case RP_EOOR:
            return "Value out of range.";
        case RP_ELID:
            return "LED input direction is not valid.";
        case RP_EMRO:
            return "Modifying read only filed is not allowed.";
        case RP_EWIP:
            return "Writing to input pin is not valid.";
        case RP_EPN:
            return "Invalid Pin number.";
        case RP_UIA:
            return "Uninitialized Input Argument.";
        case RP_FCA:
            return "Failed to Find Calibration Parameters.";
        case RP_RCA:
            return "Failed to Read Calibration Parameters.";
        case RP_BTS:
            return "Buffer too small";
        case RP_EIPV:
            return "Invalid parameter value";
        case RP_EUF:
            return "Unsupported Feature";
        case RP_ENN:
            return "Data not normalized";
        case RP_EFOB:
            return "Failed to open bus";
        case RP_EFCB:
            return "Failed to close bus";
        case RP_EABA:
            return "Failed to acquire bus access";
        case RP_EFRB:
            return "Failed to read from the bus";
        case RP_EFWB:
            return "Failed to write to the bus";
        default:
            return "Unknown error";
    }
}

/**
 * Calibrate methods
 */

rp_calib_params_t rp_GetCalibrationSettings()
{
    return calib_GetParams();
}

int rp_CalibrateFrontEndOffset(rp_channel_t channel, rp_pinState_t gain, rp_calib_params_t* out_params) {
    return calib_SetFrontEndOffset(channel, gain, out_params);
}

int rp_CalibrateFrontEndScaleLV(rp_channel_t channel, float referentialVoltage, rp_calib_params_t* out_params) {
    return calib_SetFrontEndScaleLV(channel, referentialVoltage, out_params);
}

int rp_CalibrateFrontEndScaleHV(rp_channel_t channel, float referentialVoltage, rp_calib_params_t* out_params) {
    return calib_SetFrontEndScaleHV(channel, referentialVoltage, out_params);
}

int rp_CalibrateBackEndOffset(rp_channel_t channel) {
    return calib_SetBackEndOffset(channel);
}

int rp_CalibrateBackEndScale(rp_channel_t channel) {
    return calib_SetBackEndScale(channel);
}

int rp_CalibrateBackEnd(rp_channel_t channel, rp_calib_params_t* out_params) {
    return calib_CalibrateBackEnd(channel, out_params);
}

int rp_CalibrationReset() {
    return calib_Reset();
}

int rp_CalibrationSetCachedParams() {
    return calib_setCachedParams();
}

int rp_CalibrationWriteParams(rp_calib_params_t calib_params) {
    return calib_WriteParams(calib_params);
}

/**
 * Identification
 */

int rp_IdGetID(uint32_t *id) {
    *id = ioread32(&hk->id);
    return RP_OK;
}

int rp_IdGetDNA(uint64_t *dna) {
    *dna = ((uint64_t) ioread32(&hk->dna_hi) << 32)
         | ((uint64_t) ioread32(&hk->dna_lo) <<  0);
    return RP_OK;
}

/**
 * LED methods
 */

int rp_LEDSetState(uint32_t state) {
    iowrite32(state, &hk->led_control);
    return RP_OK;
}

int rp_LEDGetState(uint32_t *state) {
    *state = ioread32(&hk->led_control);
    return RP_OK;
}

/**
 * GPIO methods
 */

int rp_GPIOnSetDirection(uint32_t direction) {
    iowrite32(direction, &hk->ex_cd_n);
    return RP_OK;
}

int rp_GPIOnGetDirection(uint32_t *direction) {
    *direction = ioread32(&hk->ex_cd_n);
    return RP_OK;
}

int rp_GPIOnSetState(uint32_t state) {
    iowrite32(state, &hk->ex_co_n);
    return RP_OK;
}

int rp_GPIOnGetState(uint32_t *state) {
    *state = ioread32(&hk->ex_ci_n);
    return RP_OK;
}

int rp_GPIOpSetDirection(uint32_t direction) {
    iowrite32(direction, &hk->ex_cd_p);
    return RP_OK;
}

int rp_GPIOpGetDirection(uint32_t *direction) {
    *direction = ioread32(&hk->ex_cd_p);
    return RP_OK;
}

int rp_GPIOpSetState(uint32_t state) {
    iowrite32(state, &hk->ex_co_p);
    return RP_OK;
}

int rp_GPIOpGetState(uint32_t *state) {
    *state = ioread32(&hk->ex_ci_p);
    return RP_OK;
}

/**
 * Digital Pin Input Output methods
 */

int rp_DpinReset() {
    iowrite32(0, &hk->ex_cd_p);
    iowrite32(0, &hk->ex_cd_n);
    iowrite32(0, &hk->ex_co_p);
    iowrite32(0, &hk->ex_co_n);
    iowrite32(0, &hk->led_control);
    iowrite32(0, &hk->digital_loop);
    return RP_OK;
}

int rp_DpinSetDirection(rp_dpin_t pin, rp_pinDirection_t direction) {
    uint32_t tmp;
    if (pin < RP_DIO0_P) {
        // LEDS
        return RP_ELID;
    } else if (pin < RP_DIO0_N) {
        // DIO_P
        pin -= RP_DIO0_P;
        tmp = ioread32(&hk->ex_cd_p);
        iowrite32((tmp & ~(1 << pin)) | ((direction << pin) & (1 << pin)), &hk->ex_cd_p);
    } else {
        // DIO_N
        pin -= RP_DIO0_N;
        tmp = ioread32(&hk->ex_cd_n);
        iowrite32((tmp & ~(1 << pin)) | ((direction << pin) & (1 << pin)), &hk->ex_cd_n);
    }
    return RP_OK;
}

int rp_DpinGetDirection(rp_dpin_t pin, rp_pinDirection_t* direction) {
    if (pin < RP_DIO0_P) {
        // LEDS
        *direction = RP_OUT;
    } else if (pin < RP_DIO0_N) {
        // DIO_P
        pin -= RP_DIO0_P;
        *direction = (ioread32(&hk->ex_cd_p) >> pin) & 0x1;
    } else {
        // DIO_N
        pin -= RP_DIO0_N;
        *direction = (ioread32(&hk->ex_cd_n) >> pin) & 0x1;
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
        tmp = ioread32(&hk->led_control);
        iowrite32((tmp & ~(1 << pin)) | ((state << pin) & (1 << pin)), &hk->led_control);
    } else if (pin < RP_DIO0_N) {
        // DIO_P
        pin -= RP_DIO0_P;
        tmp = ioread32(&hk->ex_co_p);
        iowrite32((tmp & ~(1 << pin)) | ((state << pin) & (1 << pin)), &hk->ex_co_p);
    } else {
        // DIO_N
        pin -= RP_DIO0_N;
        tmp = ioread32(&hk->ex_co_n);
        iowrite32((tmp & ~(1 << pin)) | ((state << pin) & (1 << pin)), &hk->ex_co_n);
    }
    return RP_OK;
}

int rp_DpinGetState(rp_dpin_t pin, rp_pinState_t* state) {
    if (pin < RP_DIO0_P) {
        // LEDS
        *state = (ioread32(&hk->led_control) >> pin) & 0x1;
    } else if (pin < RP_DIO0_N) {
        // DIO_P
        pin -= RP_DIO0_P;
        *state = (ioread32(&hk->ex_ci_p) >> pin) & 0x1;
    } else {
        // DIO_N
        pin -= RP_DIO0_N;
        *state = (ioread32(&hk->ex_ci_n) >> pin) & 0x1;
    }
    return RP_OK;
}


/**
 * Digital loop
 */

int rp_EnableDigitalLoop(bool enable) {
    iowrite32((uint32_t) enable, &hk->digital_loop);
    return RP_OK;
}


/** @name Analog Inputs/Outputs
 */
///@{

int rp_ApinReset() {
    return rp_AOpinReset();
}

int rp_ApinGetValue(rp_apin_t pin, float* value) {
    if (pin <= RP_AOUT3) {
        rp_AOpinGetValue(pin-RP_AOUT0, value);
    } else if (pin <= RP_AIN3) {
        rp_AIpinGetValue(pin-RP_AIN0, value);
    } else {
        return RP_EPN;
    }
    return RP_OK;
}

int rp_ApinGetValueRaw(rp_apin_t pin, uint32_t* value) {
    if (pin <= RP_AOUT3) {
        rp_AOpinGetValueRaw(pin-RP_AOUT0, value);
    } else if (pin <= RP_AIN3) {
        rp_AIpinGetValueRaw(pin-RP_AIN0, value);
    } else {
        return RP_EPN;
    }
    return RP_OK;
}

int rp_ApinSetValue(rp_apin_t pin, float value) {
    if (pin <= RP_AOUT3) {
        rp_AOpinSetValue(pin-RP_AOUT0, value);
    } else if (pin <= RP_AIN3) {
        return RP_EPN;
    } else {
        return RP_EPN;
    }
    return RP_OK;
}

int rp_ApinSetValueRaw(rp_apin_t pin, uint32_t value) {
    if (pin <= RP_AOUT3) {
        rp_AOpinSetValueRaw(pin-RP_AOUT0, value);
    } else if (pin <= RP_AIN3) {
        return RP_EPN;
    } else {
        return RP_EPN;
    }
    return RP_OK;
}

int rp_ApinGetRange(rp_apin_t pin, float* min_val, float* max_val) {
    if (pin <= RP_AOUT3) {
        *min_val = ANALOG_OUT_MIN_VAL;
        *max_val = ANALOG_OUT_MAX_VAL;
    } else if (pin <= RP_AIN3) {
        *min_val = ANALOG_IN_MIN_VAL;
        *max_val = ANALOG_IN_MAX_VAL;
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
    iowrite32((value & ANALOG_OUT_MASK) << ANALOG_OUT_BITS, &ams->dac[pin]);
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
    *value = (ioread32(&ams->dac[pin]) >> ANALOG_OUT_BITS) & ANALOG_OUT_MASK;
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


/**
 * Acquire methods
 */

int rp_AcqSetArmKeep(bool enable)
{
    return acq_SetArmKeep(enable);
}

int rp_AcqSetDecimation(rp_acq_decimation_t decimation)
{
    return acq_SetDecimation(decimation);
}

int rp_AcqGetDecimation(rp_acq_decimation_t* decimation)
{
    return acq_GetDecimation(decimation);
}

int rp_AcqGetDecimationFactor(uint32_t* decimation)
{
    return acq_GetDecimationFactor(decimation);
}

int rp_AcqSetSamplingRate(rp_acq_sampling_rate_t sampling_rate)
{
    return acq_SetSamplingRate(sampling_rate);
}

int rp_AcqGetSamplingRate(rp_acq_sampling_rate_t* sampling_rate)
{
    return acq_GetSamplingRate(sampling_rate);
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
    return acq_SetTriggerDelay(decimated_data_num, false);
}

int rp_AcqGetTriggerDelay(int32_t* decimated_data_num)
{
    return acq_GetTriggerDelay(decimated_data_num);
}

int rp_AcqSetTriggerDelayNs(int64_t time_ns)
{
    return acq_SetTriggerDelayNs(time_ns, false);
}

int rp_AcqGetTriggerDelayNs(int64_t* time_ns)
{
    return acq_GetTriggerDelayNs(time_ns);
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

int rp_AcqGetTriggerLevel(float* voltage)
{
    return acq_GetTriggerLevel(voltage);
}

int rp_AcqSetTriggerLevel(float voltage)
{
    return acq_SetTriggerLevel(voltage);
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

int rp_AcqGetWritePointerAtTrig(uint32_t* pos)
{
    return acq_GetWritePointerAtTrig(pos);
}

int rp_AcqStart()
{
    return acq_Start();
}

int rp_AcqStop()
{
    return acq_Stop();
}
int rp_AcqReset()
{
    return acq_Reset();
}

uint32_t rp_AcqGetNormalizedDataPos(uint32_t pos)
{
    return acq_GetNormalizedDataPos(pos);
}

int rp_AcqGetDataPosRaw(rp_channel_t channel, uint32_t start_pos, uint32_t end_pos, int16_t* buffer, uint32_t* buffer_size)
{
    return acq_GetDataPosRaw(channel, start_pos, end_pos, buffer, buffer_size);
}

int rp_AcqGetDataPosV(rp_channel_t channel, uint32_t start_pos, uint32_t end_pos, float* buffer, uint32_t* buffer_size)
{
    return acq_GetDataPosV(channel, start_pos, end_pos, buffer, buffer_size);
}

int rp_AcqGetDataRaw(rp_channel_t channel,  uint32_t pos, uint32_t* size, int16_t* buffer)
{
    return acq_GetDataRaw(channel, pos, size, buffer);
}

int rp_AcqGetDataRawV2(uint32_t pos, uint32_t* size, uint16_t* buffer, uint16_t* buffer2)
{
    return acq_GetDataRawV2(pos, size, buffer, buffer2);
}

int rp_AcqGetOldestDataRaw(rp_channel_t channel, uint32_t* size, int16_t* buffer)
{
    return acq_GetOldestDataRaw(channel, size, buffer);
}

int rp_AcqGetLatestDataRaw(rp_channel_t channel, uint32_t* size, int16_t* buffer)
{
    return acq_GetLatestDataRaw(channel, size, buffer);
}

int rp_AcqGetDataV(rp_channel_t channel, uint32_t pos, uint32_t* size, float* buffer)
{
    return acq_GetDataV(channel, pos, size, buffer);
}

int rp_AcqGetDataV2(uint32_t pos, uint32_t* size, float* buffer1, float* buffer2)
{
    return acq_GetDataV2(pos, size, buffer1, buffer2);
}

int rp_AcqGetOldestDataV(rp_channel_t channel, uint32_t* size, float* buffer)
{
    return acq_GetOldestDataV(channel, size, buffer);
}

int rp_AcqGetLatestDataV(rp_channel_t channel, uint32_t* size, float* buffer)
{
    return acq_GetLatestDataV(channel, size, buffer);
}

int rp_AcqGetBufSize(uint32_t *size) {
    return acq_GetBufferSize(size);
}

/**
* Generate methods
*/

int rp_GenReset() {
    return gen_SetDefaultValues();
}

int rp_GenOutDisable(rp_channel_t channel) {
    return gen_Disable(channel);
}

int rp_GenOutEnable(rp_channel_t channel) {
    return gen_Enable(channel);
}

int rp_GenOutIsEnabled(rp_channel_t channel, bool *value) {
    return gen_IsEnable(channel, value);
}

int rp_GenAmp(rp_channel_t channel, float amplitude) {
    return gen_setAmplitude(channel, amplitude);
}

int rp_GenGetAmp(rp_channel_t channel, float *amplitude) {
    return gen_getAmplitude(channel, amplitude);
}

int rp_GenOffset(rp_channel_t channel, float offset) {
    return gen_setOffset(channel, offset);
}

int rp_GenGetOffset(rp_channel_t channel, float *offset) {
    return gen_getOffset(channel, offset);
}

int rp_GenFreq(rp_channel_t channel, float frequency) {
    return gen_setFrequency(channel, frequency);
}

int rp_GenGetFreq(rp_channel_t channel, float *frequency) {
    return gen_getFrequency(channel, frequency);
}

int rp_GenPhase(rp_channel_t channel, float phase) {
    return gen_setPhase(channel, phase);
}

int rp_GenGetPhase(rp_channel_t channel, float *phase) {
    return gen_getPhase(channel, phase);
}

int rp_GenWaveform(rp_channel_t channel, rp_waveform_t type) {
    return gen_setWaveform(channel, type);
}

int rp_GenGetWaveform(rp_channel_t channel, rp_waveform_t *type) {
    return gen_getWaveform(channel, type);
}

int rp_GenArbWaveform(rp_channel_t channel, float *waveform, uint32_t length) {
    return gen_setArbWaveform(channel, waveform, length);
}

int rp_GenGetArbWaveform(rp_channel_t channel, float *waveform, uint32_t *length) {
    return gen_getArbWaveform(channel, waveform, length);
}

int rp_GenDutyCycle(rp_channel_t channel, float ratio) {
    return gen_setDutyCycle(channel, ratio);
}

int rp_GenGetDutyCycle(rp_channel_t channel, float *ratio) {
    return gen_getDutyCycle(channel, ratio);
}

int rp_GenMode(rp_channel_t channel, rp_gen_mode_t mode) {
    return gen_setGenMode(channel, mode);
}

int rp_GenGetMode(rp_channel_t channel, rp_gen_mode_t *mode) {
    return gen_getGenMode(channel, mode);
}

int rp_GenBurstCount(rp_channel_t channel, int num) {
    return gen_setBurstCount(channel, num);
}

int rp_GenGetBurstCount(rp_channel_t channel, int *num) {
    return gen_getBurstCount(channel, num);
}

int rp_GenBurstRepetitions(rp_channel_t channel, int repetitions) {
    return gen_setBurstRepetitions(channel, repetitions);
}

int rp_GenGetBurstRepetitions(rp_channel_t channel, int *repetitions) {
    return gen_getBurstRepetitions(channel, repetitions);
}

int rp_GenBurstPeriod(rp_channel_t channel, uint32_t period) {
    return gen_setBurstPeriod(channel, period);
}

int rp_GenGetBurstPeriod(rp_channel_t channel, uint32_t *period) {
    return gen_getBurstPeriod(channel, period);
}

int rp_GenTriggerSource(rp_channel_t channel, rp_trig_src_t src) {
    return gen_setTriggerSource(channel, src);
}

int rp_GenGetTriggerSource(rp_channel_t channel, rp_trig_src_t *src) {
    return gen_getTriggerSource(channel, src);
}

int rp_GenTrigger(uint32_t channel) {
    return gen_Trigger(channel);
}

float rp_CmnCnvCntToV(uint32_t field_len, uint32_t cnts, float adc_max_v, uint32_t calibScale, int calib_dc_off, float user_dc_off)
{
	return cmn_CnvCntToV(field_len, cnts, adc_max_v, calibScale, calib_dc_off, user_dc_off);
}

