/**
* $Id: $
*
* @brief Red Pitaya lapplication ibrary API interface implementation
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

#include "rpApp.h"
#include "osciloscopeApp.h"
#include "spectrometerApp.h"
#include "version.h"
#include "common.h"

static char version[50];

/**
* Global methods
*/

int rpApp_Init() {
    int ret = rp_Init();
    if (ret != RP_OK){
        return ret;
    }

    osc_Init();

    ret = rpApp_Reset();
    if (ret != RP_OK){
        rpApp_Release();
        return ret;
    }

    return RP_OK;
}

int rpApp_OscRunMainThread(){
    return osc_RunMainThread();
}

int rpApp_Release() {
    osc_stop();
    osc_Release();
    ECHECK(rp_Release());
    return RP_OK;
}

int rpApp_Reset() {
    rp_Reset();
    ECHECK(osc_SetDefaultValues());
    return RP_OK;
}

const char *rpApp_GetVersion() {
    sprintf(version, "%s (%s)", VERSION_STR, REVISION_STR);
    return version;
}

const char* rpApp_GetError(int errorCode) {
    if (errorCode < 100) {
        return rp_GetError(errorCode);
    }
    switch (errorCode) {
        case RP_APP_EST:
            return "Failed to Start a Thread.";
        case RP_APP_ENS:
            return "No signal found.";
        case RP_EAA:
            return "Failed to allocate array.";
        case RP_APP_ECP:
            return "Failed to calculate period.";
        default:
            return "Unknown error";
    }
}

int rpApp_OscRun() {
    return osc_run();
}

int rpApp_OscStop() {
    return osc_stop();
}

int rpApp_OscReset() {
    return osc_reset();
}

int rpApp_OscSingle() {
    return osc_single();
}

int rpApp_OscAutoScale() {
    return osc_autoScale();
}

int rpApp_OscGetAutoScale(bool *_state) {
    return osc_getAutoScale(_state);
}

int rpApp_OscRefreshViewData() {
    return osc_refreshViewData();
}

int rpApp_OscIsRunning(bool *running) {
    return osc_isRunning(running);
}

int rpApp_OscIsTriggered() {
    return osc_isTriggered();
}

int rpApp_OscSetTimeScale(float scale) {
    return osc_setTimeScale(scale);
}

int rpApp_OscGetTimeScale(float *scale) {
    return osc_getTimeScale(scale);
}

int rpApp_OscSetTimeOffset(float offset) {
    return osc_setTimeOffset(offset);
}

int rpApp_OscGetTimeOffset(float *offset) {
    return osc_getTimeOffset(offset);
}

int rpApp_OscSetProbeAtt(rp_channel_t channel, float att) {
    return osc_setProbeAtt(channel, att);
}

int rpApp_OscGetProbeAtt(rp_channel_t channel, float *att) {
    return osc_getProbeAtt(channel, att);
}

int rpApp_OscSetInputGain(rp_channel_t channel, rpApp_osc_in_gain_t gain) {
    return osc_setInputGain(channel, gain);
}

int rpApp_OscGetInputGain(rp_channel_t channel, rpApp_osc_in_gain_t *gain) {
    return osc_getInputGain(channel, gain);
}

int rpApp_OscSetAmplitudeScale(rpApp_osc_source source, double scale) {
    return osc_setAmplitudeScale(source, scale);
}

int rpApp_OscGetAmplitudeScale(rpApp_osc_source source, double *scale) {
    return osc_getAmplitudeScale(source, scale);
}

int rpApp_OscSetAmplitudeOffset(rpApp_osc_source source, double offset) {
    return osc_setAmplitudeOffset(source, offset);
}

int rpApp_OscGetAmplitudeOffset(rpApp_osc_source source, double *offset) {
    return osc_getAmplitudeOffset(source, offset);
}

int rpApp_OscSetTriggerSource(rpApp_osc_trig_source_t triggerSource) {
    return osc_setTriggerSource(triggerSource);
}

int rpApp_OscGetTriggerSource(rpApp_osc_trig_source_t *triggerSource) {
    return osc_getTriggerSource(triggerSource);
}

int rpApp_OscSetTriggerSlope(rpApp_osc_trig_slope_t slope) {
    return osc_setTriggerSlope(slope);
}

int rpApp_OscGetTriggerSlope(rpApp_osc_trig_slope_t *slope) {
    return osc_getTriggerSlope(slope);
}

int rpApp_OscSetTriggerLevel(float level) {
    return osc_setTriggerLevel(level);
}

int rpApp_OscGetTriggerLevel(float *level) {
    return osc_getTriggerLevel(level);
}

int rpApp_OscSetInverted(rpApp_osc_source source, bool inverted) {
    return osc_setInverted(source, inverted);
}
int rpApp_OscIsInverted(rpApp_osc_source source, bool *inverted) {
    return osc_isInverted(source, inverted);
}

int rpApp_OscGetViewPart(float *ratio) {
    return osc_getViewPart(ratio);
}

int rpApp_OscSetTriggerSweep(rpApp_osc_trig_sweep_t mode) {
    return osc_setTriggerSweep(mode);
}

int rpApp_OscGetTriggerSweep(rpApp_osc_trig_sweep_t *mode) {
    return osc_getTriggerSweep(mode);
}

int rpApp_OscGetViewData(rpApp_osc_source source, float *data, uint32_t size) {
    return osc_getData(source, data, size);
}

int rpApp_OscGetRawData(rp_channel_t source, uint16_t *data, uint32_t size) {
    return osc_getRawData(source, data, size);
}

int rpApp_OscGetExportedData(rpApp_osc_source source, rpApp_osc_exportMode mode, bool normalize, float *data, uint32_t *size){
    return osc_getExportedData(source,mode,normalize,data,size);
}

int rpApp_OscSetViewSize(uint32_t size) {
    return osc_setViewSize(size);
}

int rpApp_OscGetViewSize(uint32_t *size) {
    return osc_getViewSize(size);
}

int rpApp_OscGetViewLimits(uint32_t* start, uint32_t* end) {
    return osc_getViewLimits(start, end);
}

int rpApp_OscMeasureVpp(rpApp_osc_source source, float *Vpp) {
    return osc_measureVpp(source, Vpp);
}

int rpApp_OscMeasureMeanVoltage(rpApp_osc_source source, float *meanVoltage) {
    return osc_measureMeanVoltage(source, meanVoltage);
}

int rpApp_OscMeasureAmplitudeMax(rpApp_osc_source source, float *Vmax) {
    return osc_measureMaxVoltage(source, Vmax);
}

int rpApp_OscMeasureAmplitudeMin(rpApp_osc_source source, float *Vmin) {
    return osc_measureMinVoltage(source, Vmin);
}

int rpApp_OscMeasureFrequency(rpApp_osc_source source, float *frequency) {
    return osc_measureFrequency(source, frequency);
}

int rpApp_OscMeasurePeriod(rpApp_osc_source source, float *period) {
    return osc_measurePeriod(source, period);
}

int rpApp_OscMeasureDutyCycle(rpApp_osc_source source, float *dutyCycle) {
    return osc_measureDutyCycle(source, dutyCycle);
}

int rpApp_OscMeasureRootMeanSquare(rpApp_osc_source source, float *rms) {
    return osc_measureRootMeanSquare(source, rms);
}

int rpApp_OscGetCursorVoltage(rpApp_osc_source source, uint32_t cursor, float *value) {
    return osc_getCursorVoltage(source, cursor, value);
}

int rpApp_OscGetCursorTime(uint32_t cursor, float *value) {
    return osc_getCursorTime(cursor, value);
}

int rpApp_OscGetCursorDeltaTime(uint32_t cursor1, uint32_t cursor2, float *value) {
    return osc_getCursorDeltaTime(cursor1, cursor2, value);
}

int rpApp_OscGetCursorDeltaAmplitude(rpApp_osc_source source, uint32_t cursor1, uint32_t cursor2, float *value) {
    return osc_GetCursorDeltaAmplitude(source, cursor1, cursor2, value);
}

int rpApp_OscGetCursorDeltaFrequency(uint32_t cursor1, uint32_t cursor2, float *value) {
    return osc_getCursorDeltaFrequency(cursor1, cursor2, value);
}

int rpApp_OscSetMathOperation(rpApp_osc_math_oper_t operation) {
    return osc_setMathOperation(operation);
}

int rpApp_OscGetMathOperation(rpApp_osc_math_oper_t *operation) {
    return osc_getMathOperation(operation);
}

int rpApp_OscSetMathSources(rp_channel_t source1, rp_channel_t source2) {
    return osc_setMathSources(source1, source2);
}

int rpApp_OscGetMathSources(rp_channel_t *source1, rp_channel_t *source2) {
    return osc_getMathSources(source1, source2);
}

int rpApp_OscScaleMath(){
    return osc_scaleMath();
}

int rpApp_OscSetSmoothMode(rp_channel_t _channel, rpApp_osc_interpolationMode _mode){
    return osc_SetSmoothMode(_channel,_mode);
}

int rpApp_OscGetSmoothMode(rp_channel_t _channel, rpApp_osc_interpolationMode *_mode){
    return osc_GetSmoothMode(_channel,_mode);
}


// SPECTRUM


int rpApp_SpecRun() {
	return spec_run();
}

int rpApp_SpecStop(void) {
	return spec_stop();
}

int rpApp_SpecRunning(void) {
	return spec_running();
}

int rpApp_SpecReset() {
	return spec_reset();
}

int rpApp_SpecGetViewData(float** signals, size_t size) {
	return spec_getViewData(signals, size);
}

int rpApp_SpecGetPeakPower(rp_channel_t channel, float* power) {
	return spec_getPeakPower(channel, power);
}

int rpApp_SpecGetPeakFreq(rp_channel_t channel, float* freq) {
	return spec_getPeakFreq(channel, freq);
}

int rpApp_SpecSetFreqRange(float _freq_min, float freq) {
	return spec_setFreqRange(_freq_min, freq);
}

int rpApp_SpecGetADCFreq(){
    return spec_getGetADCFreq();
}

int rpApp_SpecGetViewSize(size_t* size)
{
	spec_getViewSize(size);
	return RP_OK;
}

int rpApp_SpecGetFreqMin(float* freq)
{
	return spec_getFreqMin(freq);
}

int rpApp_SpecGetFreqMax(float* freq)
{
	return spec_getFreqMax(freq);
}

int rpApp_SpecSetFreqMin(float)
{
	return RP_OK;
}

int rpApp_SpecSetFreqMax(float freq)
{
	return spec_setFreqRange(0, freq);
}

int rpApp_SpecGetFpgaFreq(float* freq)
{
	return spec_getFpgaFreq(freq);
}

int rpApp_SpecSetADCBufferSize(int size){
    return spec_setADCBufferSize(size);
}

int rpApp_SpecGetADCBufferSize(){
    return spec_getADCBufferSize();
}

int rpApp_OscMeasureMaxValue(rpApp_osc_source source, float *Max) {
    return osc_measureMin(source, Max);
}

int rpApp_OscMeasureMinValue(rpApp_osc_source source, float *Min) {
    return osc_measureMin(source, Min);
}

int rpApp_SpecSetWindow(rp_dsp_api::window_mode_t mode){
    return spec_setWindow(mode);
}

int rpApp_SpecGetWindow(rp_dsp_api::window_mode_t *mode){
    return spec_getWindow(mode);
}

int rpApp_SpecSetRemoveDC(int state){
    return spec_setRemoveDC(state);
}

int rpApp_SpecGetRemoveDC(){
    return spec_getRemoveDC();
}

int rpApp_SpecGetMode(rp_dsp_api::mode_t *mode){
    return spec_getVoltMode(mode);
}

int rpApp_SpecSetMode(rp_dsp_api::mode_t mode){
    return spec_setVoltMode(mode);
}

int rpApp_SpecSetImpedance(double value){
    return spec_setImpedance(value);
}

int rpApp_SpecGetImpedance(double *value){
    return spec_getImpedance(value);
}
