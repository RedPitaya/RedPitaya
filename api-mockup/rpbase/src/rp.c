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
#include "dpin_handler.h"
#include "oscilloscope.h"
#include "acq_handler.h"
#include "analog_mixed_signals.h"
#include "apin_handler.h"
#include "health.h"
#include "health_handler.h"
#include "calib.h"
#include "generate.h"
#include "gen_handler.h"

static char version[50];

/**
 * Global methods
 */

int rp_Init()
{
    ECHECK(calib_Init());
    ECHECK(hk_Init());
    ECHECK(ams_Init());
    ECHECK(health_Init());
    ECHECK(generate_Init());
    ECHECK(osc_Init());
    // TODO: Place other module initializations here

    // Set default configuration per handler
    ECHECK(rp_Reset());

    return RP_OK;
}

int rp_Release()
{
    ECHECK(osc_Release())
    ECHECK(generate_Release());;
    ECHECK(health_Release());
    ECHECK(ams_Release());
    ECHECK(hk_Release());
    ECHECK(calib_Release());

    // TODO: Place other module releasing here (in reverse order)
    return RP_OK;
}

int rp_Reset()
{
    ECHECK(rp_DpinReset());
    ECHECK(rp_ApinReset());
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

rp_calib_params_t rp_GetCalibrationSettings()
{
    return calib_GetParams();
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
        default:
            return "Unknown error";
    }
}



/**
 * Digital Pin Input Output methods
 */

int rp_DpinReset() {
    return dpin_SetDefaultValues();
}

int rp_DpinSetDirection(rp_dpin_t pin, rp_pinDirection_t direction)
{
    return dpin_SetDirection(pin, direction);
}

int rp_DpinGetDirection(rp_dpin_t pin, rp_pinDirection_t* direction)
{
    return dpin_GetDirection(pin, direction);
}

int rp_DpinSetState(rp_dpin_t pin, rp_pinState_t state)
{
    return dpin_SetState(pin, state);
}

int rp_DpinGetState(rp_dpin_t pin, rp_pinState_t* state)
{
    return dpin_GetState(pin, state);
}

/**
 * Analog In Output methods
 */

int rp_ApinReset() {
    return apin_SetDefaultValues();
}

int rp_ApinSetValue(rp_apin_t pin, float value)
{
    return apin_SetValue(pin, value);
}

int rp_ApinGetValue(rp_apin_t pin, float* value)
{
    return apin_GetValue(pin, value);
}

int rp_ApinSetValueRaw(rp_apin_t pin, uint32_t value)
{
    return apin_SetValueRaw(pin, value);
}

int rp_ApinGetValueRaw(rp_apin_t pin, uint32_t* value)
{
    return apin_GetValueRaw(pin, value);
}

int rp_ApinGetRange(rp_apin_t pin, float* min_val,  float* max_val)
{
    return apin_GetRange(pin, min_val, max_val);
}


/**
 * Acquire methods
 */

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
 * Deep averaging acquire functions
 */

int rp_DeepAvgStart(){
    return acq_DeepAvgStart();
}

int rp_SetDeepAvgCount(uint32_t count){
    return acq_SetDeepAvgCount(count);
}

int rp_SetDeepAvgShift(uint32_t shift){
    return acq_SetDeepAvgShift(shift);
}

int rp_SetDeepDataSeqLen(uint32_t len){
    return acq_SetDeepDataSeqLen(len);
}

int rp_SetDeepAvgDebTim(uint32_t deb_t){
    return acq_SetDeepAvgDebTim(deb_t);
}

int rp_GetDeepAvgCount(uint32_t *count){
    return acq_GetDeepAvgCount(count);
}

int rp_GetDeepAvgShift(uint32_t *shift){
    return acq_GetDeepAvgShift(shift);
}

int rp_GetDeepDataSeqLen(uint32_t *len){
    return acq_GetDeepDataSeqLen(len);
}

int rp_GetDeepAvgDebTim(uint32_t *deb_t){
    return acq_GetDeepAvgDebTim(deb_t);
}


int rp_GetDeepAvgRawData(rp_channel_t channel, uint32_t *size, int16_t *buffer){
    return acq_GetDeepAvgDataRaw(channel, size, buffer);
}

/**
 * Health methods
 */

int rp_HealthGetValue(rp_health_t sensor, float* value)
{
    return health_GetValue(sensor, value);
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

int rp_GenAmp(rp_channel_t channel, float amplitude) {
    return gen_setAmplitude(channel, amplitude);
}

int rp_GenOffset(rp_channel_t channel, float offset) {
    return gen_Offset(channel, offset);
}

int rp_GenFreq(rp_channel_t channel, float frequency) {
    return gen_Frequency(channel, frequency);
}

int rp_GenPhase(rp_channel_t channel, float phase) {
    return gen_Phase(channel, phase);
}

int rp_GenWaveform(rp_channel_t channel, rp_waveform_t type) {
    return gen_Waveform(channel, type);
}

int rp_GenArbWaveform(rp_channel_t channel, float *waveform, uint32_t length) {
    return gen_ArbWaveform(channel, waveform, length);
}

int rp_GenDutyCycle(rp_channel_t channel, float ratio) {
    return gen_DutyCycle(channel, ratio);
}

int rp_GenMode(rp_channel_t channel, rp_gen_mode_t mode) {
    return gen_GenMode(channel, mode);
}

int rp_GenBurstCount(rp_channel_t channel, int num) {
    return gen_BurstCount(channel, num);
}

int rp_GenBurstRepetitions(rp_channel_t channel, int repetitions) {
    return gen_BurstRepetitions(channel, repetitions);
}

int rp_GenBurstPeriod(rp_channel_t channel, uint32_t period) {
    return gen_BurstPeriod(channel, period);
}

int rp_GenTriggerSource(rp_channel_t channel, rp_trig_src_t src) {
    return gen_TriggerSource(channel, src);
}

int rp_GenTrigger(int mask) {
    return gen_Trigger(mask);
}
