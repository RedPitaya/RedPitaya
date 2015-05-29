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
#include "version.h"
#include "common.h"

static char version[50];

/**
* Global methods
*/

int rpApp_Init() {
    ECHECK_APP(rp_Init());
//    ECHECK_APP(cmn_Init());
    ECHECK_APP(osc_Init());
    // TODO: Place other module releasing here (in reverse order)

    ECHECK_APP(rpApp_Reset());

    return RP_OK;
}

int rpApp_Release() {
    ECHECK_APP(osc_Release());
    ECHECK_APP(rp_Release());
    // TODO: Place other module releasing here (in reverse order)

    return RP_OK;
}

int rpApp_Reset() {
    ECHECK_APP(rp_Reset())
    ECHECK_APP(osc_SetDefaultValues());
    // TODO: Place other module resetting here (in reverse order)

    return 0;
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

int rpApp_OscIsRunning(bool *running) {
    return osc_isRunning(running);
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

int rpApp_OscSetAmplitudeScale(rpApp_osc_source source, float scale) {
    return osc_setAmplitudeScale(source, scale);
}

int rpApp_OscGetAmplitudeScale(rpApp_osc_source source, float *scale) {
    return osc_getAmplitudeScale(source, scale);
}

int rpApp_OscSetAmplitudeOffset(rpApp_osc_source source, float offset) {
    return osc_setAmplitudeOffset(source, offset);
}

int rpApp_OscGetAmplitudeOffset(rpApp_osc_source source, float *offset) {
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

int rpApp_OscGetViewPos(float *positionRatio) {
    return osc_getViewPos(positionRatio);
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

int rpApp_OscGetInvViewData(rpApp_osc_source source, float *data, uint32_t size){
    return osc_getInvData(source, data, size);
}

int rpApp_OscSetViewSize(uint32_t size) {
    return osc_setViewSize(size);
}

int rpApp_OscGetViewSize(uint32_t *size) {
    return osc_getViewSize(size);
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
    return oscGetCursorDeltaAmplitude(source, cursor1, cursor2, value);
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