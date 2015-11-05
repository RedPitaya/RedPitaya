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
#include "analog.h"
#include "housekeeping.h"
#include "oscilloscope.h"
#include "acq_handler.h"
#include "analog.h"
#include "calib.h"
#include "generate.h"

static char version[50];

/**
 * Global methods
 */

int rp_Init()
{
    cmn_Init();
    analog_Init();
    calib_Init();
	
    ECHECK(hk_Init());
    ECHECK(generate_Init());
    ECHECK(osc_Init());
    // TODO: Place other module initializations here

    // Set default configuration per handler
    ECHECK(rp_Reset());

    return RP_OK;
}

int rp_Release()
{
    analog_Release();
    calib_Release();
    ECHECK(osc_Release())
    ECHECK(generate_Release());
    ECHECK(hk_Release());
    // TODO: Place other module releasing here (in reverse order)
    cmn_Release();
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
        case RP_OK  :  return "OK";
        case RP_EOED:  return "Failed to Open EEPROM Device.";
        case RP_EOMD:  return "Failed to open memory device.";
        case RP_ECMD:  return "Failed to close memory device.";
        case RP_EMMD:  return "Failed to map memory device.";
        case RP_EUMD:  return "Failed to unmap memory device.";
        case RP_EOOR:  return "Value out of range.";
        case RP_ELID:  return "LED input direction is not valid.";
        case RP_EMRO:  return "Modifying read only filed is not allowed.";
        case RP_EWIP:  return "Writing to input pin is not valid.";
        case RP_EPN :  return "Invalid Pin number.";
        case RP_UIA :  return "Uninitialized Input Argument.";
        case RP_FCA :  return "Failed to Find Calibration Parameters.";
        case RP_RCA :  return "Failed to Read Calibration Parameters.";
        case RP_BTS :  return "Buffer too small";
        case RP_EIPV:  return "Invalid parameter value";
        case RP_EUF :  return "Unsupported Feature";
        case RP_ENN :  return "Data not normalized";
        case RP_EFOB:  return "Failed to open bus";
        case RP_EFCB:  return "Failed to close bus";
        case RP_EABA:  return "Failed to acquire bus access";
        case RP_EFRB:  return "Failed to read from the bus";
        case RP_EFWB:  return "Failed to write to the bus";
        default:       return "Unknown error";
    }
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

float rp_CmnCnvCntToV(uint32_t field_len, uint32_t cnts, float adc_max_v)
{
	return cmn_CnvCntToV(field_len, cnts, adc_max_v);
}

