/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server acquire SCPI commands implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "../3rdparty/libs/scpi-parser/libscpi/inc/scpi/parser.h"

rp_scpi_acq_unit_t unit     = RP_SCPI_VOLTS;        // default value

scpi_result_t RP_AcqSetDataFormat(scpi_t *context) {
    const char * param;
    size_t param_len;

    // read first parameter Format type (BIN, ASCII)
    if (!SCPI_ParamString(context, &param, &param_len, true)) {
        syslog(LOG_ERR, "*ACQ:DATA:FORMAT is missing first parameter.");
        return SCPI_RES_ERR;
    }

    if (strncasecmp(param, "BIN", param_len) == 0) {
        context->binary_output = true;
        syslog(LOG_INFO, "*ACQ:DATA:FORMAT set to BIN");
    }
    else if (strncasecmp(param, "ASCII", param_len) == 0) {
        context->binary_output = false;
        syslog(LOG_INFO, "*ACQ:DATA:FORMAT set to ASCII");
    }
    else {
        syslog(LOG_ERR, "*ACQ:DATA:FORMAT wrong argument value");
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


scpi_result_t RP_AcqStart(scpi_t *context) {
    int result = rp_AcqStart();

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:START Failed: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*ACQ:START Successful.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqStop(scpi_t *context) {
    int result = rp_AcqStop();

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:STOP Failed: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*ACQ:STOP Successful.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqReset(scpi_t *context) {
    int result = rp_AcqReset();

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:RST Failed: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    unit = RP_SCPI_VOLTS;
    context->binary_output = false;

    syslog(LOG_INFO, "*ACQ:RST Successful.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqSetDecimation(scpi_t *context) {
    int value;

    // read first parameter DECIMATION (1,8,64,1024,8192,65536)
    if (!SCPI_ParamInt(context, &value, false)) {
        syslog(LOG_ERR, "*ACQ:DEC is missing first parameter.");
        return SCPI_RES_ERR;
    }

    // Convert decimation to rp_acq_decimation_t
    rp_acq_decimation_t decimation;
    if (getRpDecimation(value, &decimation)) {
        syslog(LOG_ERR, "*ACQ:DEC parameter decimation is invalid.");
        return SCPI_RES_ERR;
    }

    // Now set the decimation
    int result = rp_AcqSetDecimation(decimation);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:DEC Failed to set decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*ACQ:DEC Successfully set decimation to %d.", value);

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqGetDecimation(scpi_t *context) {
    // Get decimation
    rp_acq_decimation_t decimation;
    int result = rp_AcqGetDecimation(&decimation);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:DEC? Failed to get decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Convert decimation to int
    int value;
    if (RP_OK != getRpDecimationInt(decimation, &value)) {
        syslog(LOG_ERR, "*ACQ:DEC? Failed to convert decimation to integer: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultDouble(context, value);

    syslog(LOG_INFO, "*ACQ:DEC? Successfully returned decimation.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqSetSamplingRate(scpi_t *context) {
    const char * param;
    size_t param_len;
    char samplingRateStr[15];

    // read first parameter SAMPLING_RATE (125MHz,15_6MHz, 1_9MHz,103_8kHz, 15_2kHz, 1_9kHz)
    if (!SCPI_ParamString(context, &param, &param_len, true)) {
        syslog(LOG_ERR, "*ACQ:SRAT is missing first parameter.");
        return SCPI_RES_ERR;
    }
    strncpy(samplingRateStr, param, param_len);
    samplingRateStr[param_len] = '\0';

    // Convert samplingRate to rp_acq_sampling_rate_t
    rp_acq_sampling_rate_t samplingRate;
    if (getRpSamplingRate(samplingRateStr, &samplingRate)) {
        syslog(LOG_ERR, "*ACQ:SRAT parameter sampling rate is invalid.");
        return SCPI_RES_ERR;
    }

    // Now set the sampling rate
    int result = rp_AcqSetSamplingRate(samplingRate);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:SRAT Failed to set sampling rate: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*ACQ:SRAT Successfully set sampling rate to %s.", samplingRateStr);

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqGetSamplingRate(scpi_t *context) {
    rp_acq_sampling_rate_t samplingRate;
    int result = rp_AcqGetSamplingRate(&samplingRate);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:SRAT? Failed to get sampling rate: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // convert sampling rate to string
    char samplingRateString[15];
    if (RP_OK != getRpSamplingRateString(samplingRate, samplingRateString)) {
        syslog(LOG_ERR, "*ACQ:SRAT? Failed to convert sampling rate to string: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultString(context, samplingRateString);

    syslog(LOG_INFO, "*ACQ:SRAT? Successfully returned sampling rate.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqGetSamplingRateHz(scpi_t *context) {
    // get sampling rate
    float samplingRate;
    int result = rp_AcqGetSamplingRateHz(&samplingRate);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:SRA:HZ? Failed to get sampling rate in Hz: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back string result
    char samplingRateString;
    sprintf(&samplingRateString, "%.0f Hz", samplingRate);

    //Return string in form "<Value> Hz"
    SCPI_ResultString(context, &samplingRateString);

    syslog(LOG_INFO, "*ACQ:SRA:HZ? Successfully returned sampling rate in Hz.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqSetAveraging(scpi_t *context) {
    scpi_bool_t value;

    // read first parameter AVERAGING (OFF,ON)
    if (!SCPI_ParamBool(context, &value, false)) {
        syslog(LOG_ERR, "*ACQ:AVGT is missing first parameter.");
        return SCPI_RES_ERR;
    }

    // Now set the averaging
    int result = rp_AcqSetAveraging(value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:AVGT Failed to set averaging: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*ACQ:AVG Successfully set averaging to %s.", value ? "ON" : "OFF");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqGetAveraging(scpi_t *context) {
    // get averaging
    bool value;
    int result = rp_AcqGetAveraging(&value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:AVG? Failed to get averaging: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultString(context, value ? "ON" : "OFF");

    syslog(LOG_INFO, "*ACQ:AVG? Successfully returned averaging.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqSetTriggerSrc(scpi_t *context) {
    const char * param;
    size_t param_len;

    char triggerSource[15];

    // read first parameter TRIGGER SOURCE (DISABLED,NOW,CH1_PE,CH1_NE,CH2_PE,CH2_NE,EXT_PE,EXT_NE,AWG_PE)
    if (!SCPI_ParamString(context, &param, &param_len, false)) {
        syslog(LOG_ERR, "*ACQ:TRIG is missing first parameter.");
        return SCPI_RES_ERR;
    }
    else {
        strncpy(triggerSource, param, param_len);
        triggerSource[param_len] = '\0';
    }

    rp_acq_trig_src_t source;
    if (getRpTriggerSource(triggerSource, &source)) {
        syslog(LOG_ERR, "*ACQ:TRIG parameter trigger source is invalid.");
        return SCPI_RES_ERR;
    }

    // Now set the trigger source
    int result = rp_AcqSetTriggerSrc(source);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:TRIG Failed to set trigger source: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*ACQ:TRIG Successfully set trigger source to %s.", triggerSource);

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqGetTrigger(scpi_t *context) {
    // get trigger source
    rp_acq_trig_src_t source;
    int result = rp_AcqGetTriggerSrc(&source);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:TRIG:STAT? Failed to get trigger: %s", rp_GetError(result));
        source = RP_TRIG_SRC_NOW;   // Some value not equal to DISABLE -> function return "WAIT"
    }

    char sourceString[15];
    if (getRpTriggerSourceString(source, sourceString)) {
        syslog(LOG_ERR, "*ACQ:TRIG:STAT? Failed to convert result to string: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultString(context, sourceString);

    syslog(LOG_INFO, "*ACQ:TRIG:STAT? Successfully returned trigger.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqSetTriggerDelay(scpi_t *context) {
    int32_t triggerDelay;

    // read first parameter TRIGGER DELAY (value in samples)
    if (!SCPI_ParamInt(context, &triggerDelay, false)) {
        triggerDelay = 0;
    }

    // Now set the trigger delay
    int result = rp_AcqSetTriggerDelay(triggerDelay);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:TRIG:DLY Failed to set trigger delay: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*ACQ:TRIG:DLY Successfully set trigger delay to %d.", triggerDelay);

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqGetTriggerDelay(scpi_t *context) {
    // get trigger delay
    int32_t value;
    int result = rp_AcqGetTriggerDelay(&value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:TRIG:DLY? Failed to get trigger delay: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultInt(context, value);

    syslog(LOG_INFO, "*ACQ:TRIG:DLY? Successfully returned trigger delay.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqSetTriggerDelayNs(scpi_t *context) {
    int64_t triggerDelay;

    // read first parameter TRIGGER DELAY ns (value in ns)
    if (!SCPI_ParamLong(context, &triggerDelay, false)) {
        triggerDelay = 0;
    }

    // Now set the trigger delay in ns
    int result = rp_AcqSetTriggerDelayNs(triggerDelay);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:TRIG:DLY:NS Failed to set trigger delay in ns: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*ACQ:TRIG:DLY:NS Successfully set trigger delay to %ld ns.", (signed long)triggerDelay);

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqGetTriggerDelayNs(scpi_t *context) {
    // get trigger delay ns
    int64_t value;
    int result = rp_AcqGetTriggerDelayNs(&value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:TRIG:DLY:NS? Failed to get trigger delay in ns: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultLong(context, value);

    syslog(LOG_INFO, "*ACQ:TRIG:DLY:NS? Successfully returned trigger delay in ns.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqSetChannel1Gain(scpi_t *context) {
    return RP_AcqSetGain(RP_CH_1, context);
}

scpi_result_t RP_AcqSetChannel2Gain(scpi_t *context) {
    return RP_AcqSetGain(RP_CH_2, context);
}

scpi_result_t RP_AcqGetChannel1Gain(scpi_t *context) {
    return RP_AcqGetGain(RP_CH_1, context);
}

scpi_result_t RP_AcqGetChannel2Gain(scpi_t *context) {
    return RP_AcqGetGain(RP_CH_2, context);
}

scpi_result_t RP_AcqSetTriggerLevel(scpi_t *context) {
    double value;

    // read first parameter TRIGGER LEVEL (value in mV)
    if (!SCPI_ParamDouble(context, &value, true)) {
        syslog(LOG_ERR, "*ACQ:TRIG:LEV is missing first parameter.");
        return SCPI_RES_ERR;
    }
    value = value / 1000.0;     // convert to to volts

    // Now set threshold
    int result = rp_AcqSetTriggerLevel((float) value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:TRIG:LEV Failed to set trigger level: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*ACQ:TRIG:LEV Successfully set trigger level %.2f.", value);

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqGetTriggerLevel(scpi_t *context) {
    float value;
    int result = rp_AcqGetTriggerLevel(&value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:TRIG:LEV? Failed to get trigger level: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    value = value * 1000;       // convert to milli volts
    // Return back result
    SCPI_ResultDouble(context, value);

    syslog(LOG_INFO, "*ACQ:TRIG:LEV? Successfully returned trigger level.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqGetWritePointer(scpi_t *context) {
    // get write pointer
    uint32_t value;
    int result = rp_AcqGetWritePointer(&value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:WPOS? Failed to get writer position: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt(context, value);

    syslog(LOG_INFO, "*ACQ:WPOS? Successfully returned writer position.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqGetWritePointerAtTrig(scpi_t *context) {
    // get write pointer at trigger
    uint32_t value;
    int result = rp_AcqGetWritePointerAtTrig(&value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:TPOS? Failed to get writer position at trigger: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt(context, value);

    syslog(LOG_INFO, "*ACQ:TPOS? Successfully returned writer position at trigger.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqScpiDataUnits(scpi_t *context) {
    const char * param;
    size_t param_len;

    char unitString[10];

    // read first parameter UNITS (RAW, VOLTS)
    if (!SCPI_ParamString(context,  &param, &param_len, false)) {
        syslog(LOG_ERR, "*ACQ:DATA:UNITSis missing first parameter.");
        return SCPI_RES_ERR;
    }
    else {
        strncpy(unitString, param, param_len);
        unitString[param_len] = '\0';
    }

    int result = getRpUnit(unitString, &unit);
    if (result != RP_OK) {
        syslog(LOG_ERR, "*ACQ:DATA:UNITS Failed to convert unit from string: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*ACQ:DATA:UNITS Successfully set unit to %s.", unitString);

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqGetChanel1DataPos(scpi_t *context) {
    return RP_AcqGetDataPos(RP_CH_1, context);
}

scpi_result_t RP_AcqGetChanel2DataPos(scpi_t *context) {
    return RP_AcqGetDataPos(RP_CH_2, context);
}

scpi_result_t RP_AcqGetChanel1Data(scpi_t *context) {
    return RP_AcqGetData(RP_CH_1, context);
}

scpi_result_t RP_AcqGetChanel2Data(scpi_t *context) {
    return RP_AcqGetData(RP_CH_2, context);
}

scpi_result_t RP_AcqGetChanel1OldestData(scpi_t *context) {
    return RP_AcqGetOldestData(RP_CH_1, context);
}

scpi_result_t RP_AcqGetChanel2OldestData(scpi_t *context) {
    return RP_AcqGetOldestData(RP_CH_2, context);
}

scpi_result_t RP_AcqGetChanel1LatestData(scpi_t *context) {
    return RP_AcqGetLatestData(RP_CH_1, context);
}

scpi_result_t RP_AcqGetChanel2LatestData(scpi_t *context) {
    return RP_AcqGetLatestData(RP_CH_2, context);
}

scpi_result_t RP_AcqGetChanel1OldestDataAll(scpi_t *context) {
    return RP_AcqGetOldestDataAll(RP_CH_1, context);
}

scpi_result_t RP_AcqGetChanel2OldestDataAll(scpi_t *context) {
    return RP_AcqGetOldestDataAll(RP_CH_2, context);
}

scpi_result_t RP_AcqGetBufferSize(scpi_t *context) {
    uint32_t size;
    int result = rp_AcqGetBufSize(&size);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:BUF:SIZE? Failed to get buffer size: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt(context, size);

    syslog(LOG_INFO, "*ACQ:BUF:SIZE?? Successfully returned buffer size.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqSetGain(rp_channel_t channel, scpi_t *context) {
    const char * param;
    size_t param_len;

    char gainString[15];

    // read first parameter GAIN (LV,HV)
    if (!SCPI_ParamString(context, &param, &param_len, false)) {
        syslog(LOG_ERR, "*ACQ:SOUR<n>:GAIN is missing first parameter.");
        return SCPI_RES_ERR;
    }
    else {
        strncpy(gainString, param, param_len);
        gainString[param_len] = '\0';
    }

    // Convert gain to rp_pinState_t
    rp_pinState_t state;

    if (getRpGain(gainString, &state)) {
        syslog(LOG_ERR, "*ACQ:SOUR<n>:GAIN parameter gain is invalid.");
        return SCPI_RES_ERR;
    }

    // Now set gain
    int result = rp_AcqSetGain(channel, state);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:SOUR<n>:GAIN Failed to set gain: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*ACQ:SOUR<n>:GAIN Successfully set gain %s.", gainString);

    return SCPI_RES_OK;
}


scpi_result_t RP_AcqGetGain(rp_channel_t channel, scpi_t *context) {
    rp_pinState_t state;
    int result = rp_AcqGetGain(channel, &state);
    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA:LAT:N? Failed to get latest data: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultString(context, state == RP_HIGH ? "HV" : "LV");

    syslog(LOG_INFO, "*AACQ:SOUR<n>:DATA:STA:END? Successfully returned  latest data.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqGetLatestData(rp_channel_t channel, scpi_t *context) {
    uint32_t size;
    // read first parameter SIZE
    if (!SCPI_ParamUInt(context, &size, true)) {
        syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA:LAT:N? is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result;
    if (unit == RP_SCPI_VOLTS) {
        float buffer[size];
        result = rp_AcqGetLatestDataV(channel, &size, buffer);

        if (RP_OK != result) {
            syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA:LAT:N? Failed to get latest data: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        // Return back result
        SCPI_ResultBufferFloat(context, buffer, size);
    }
    else {
        int16_t buffer[size];
        result = rp_AcqGetLatestDataRaw(channel, &size, buffer);

        if (RP_OK != result) {
            syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA:LAT:N? Failed to get latest data: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        // Return back result
        SCPI_ResultBufferInt16(context, buffer, size);
    }

    syslog(LOG_INFO, "*ACQ:SOUR<n>:DATA:LAT:N? Successfully returned latest data.");

    return SCPI_RES_OK;
}


scpi_result_t RP_AcqGetData(rp_channel_t channel, scpi_t *context) {
    uint32_t start, size;
    // read first parameter START POSITION
    if (!SCPI_ParamUInt(context, &start, true)) {
        syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA:STA:N? is missing first parameter.");
        return SCPI_RES_ERR;
    }

    // read second parameter SIZE
    if (!SCPI_ParamUInt(context, &size, true)) {
        syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA:STA:N? is missing second parameter.");
        return SCPI_RES_ERR;
    }

    int result;
    if (unit == RP_SCPI_VOLTS) {
        float buffer[size];
        result = rp_AcqGetDataV(channel, start, &size, buffer);

        if (RP_OK != result) {
            syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA:STA:N? Failed to get data in volts: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        // Return back result
        SCPI_ResultBufferFloat(context, buffer, size);
    }
    else {
        int16_t buffer[size];
        result = rp_AcqGetDataRaw(channel, start, &size, buffer);

        if (RP_OK != result) {
            syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA:STA:N? Failed to get raw data: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        // Return back result
        SCPI_ResultBufferInt16(context, buffer, size);
    }

    syslog(LOG_INFO, "ACQ:SOUR<n>:DATA:STA:N? Successfully returned data.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqGetOldestData(rp_channel_t channel, scpi_t *context) {
    uint32_t size;
    // read first parameter SIZE
    if (!SCPI_ParamUInt(context, &size, true)) {
        syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA:OLD:N? is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result;
    if (unit == RP_SCPI_VOLTS) {
        float buffer[size];
        result = rp_AcqGetOldestDataV(channel, &size, buffer);

        if (RP_OK != result) {
            syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA:OLD:N? Failed to get oldest data: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        // Return back result
        SCPI_ResultBufferFloat(context, buffer, size);
    }
    else {
        int16_t buffer[size];
        result = rp_AcqGetOldestDataRaw(channel, &size, buffer);

        if (RP_OK != result) {
            syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA:OLD:N? Failed to get oldest data: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        // Return back result
        SCPI_ResultBufferInt16(context, buffer, size);
    }

    syslog(LOG_INFO, "*ACQ:SOUR<n>:DATA:OLD:N? Successfully returned oldest data.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqGetDataPos(rp_channel_t channel, scpi_t *context) {
    uint32_t start, end;
    // read first parameter START POSITION
    if (!SCPI_ParamUInt(context, &start, true)) {
        syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA:STA:END? is missing first parameter.");
        return SCPI_RES_ERR;
    }

    // read second parameter END POSITION
    if (!SCPI_ParamUInt(context, &end, true)) {
        syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA:STA:END? is missing second parameter.");
        return SCPI_RES_ERR;
    }

    int result;
    uint32_t size;
    rp_AcqGetBufSize(&size);

    if (unit == RP_SCPI_VOLTS) {
        float buffer[size];
        result = rp_AcqGetDataPosV(channel, start, end, buffer, &size);

        if (RP_OK != result) {
            syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA:STA:END? Failed to get data at position: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        // Return back result
        SCPI_ResultBufferFloat(context, buffer, size);
    }
    else {
        int16_t buffer[size];
        result = rp_AcqGetDataPosRaw(channel, start, end, buffer, &size);

        if (RP_OK != result) {
            syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA:STA:END? Failed to get data at position: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        // Return back result
        SCPI_ResultBufferInt16(context, buffer, size);
    }

    syslog(LOG_INFO, "*AACQ:SOUR<n>:DATA:STA:END? Successfully returned data at position.");
    return SCPI_RES_OK;
}


scpi_result_t RP_AcqGetOldestDataAll(rp_channel_t channel, scpi_t *context) {
    int result;
    uint32_t size;
    result = rp_AcqGetBufSize(&size);
    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA? Failed to get buffer size: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    if (unit == RP_SCPI_VOLTS) {
        float buffer[size];
        result = rp_AcqGetOldestDataV(channel, &size, buffer);

        if (RP_OK != result) {
            syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA? Failed to get all oldest data: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        // Return back result
        SCPI_ResultBufferFloat(context, buffer, size);
    }
    else {
        int16_t buffer[size];
        result = rp_AcqGetOldestDataRaw(channel, &size, buffer);

        if (RP_OK != result) {
            syslog(LOG_ERR, "*ACQ:SOUR<n>:DATA? Failed to get all oldest data: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        // Return back result
        SCPI_ResultBufferInt16(context, buffer, size);
    }

    syslog(LOG_INFO, "*ACQ:SOUR<n>:DATA? Successfully returned all oldest data.");

    return SCPI_RES_OK;
}
