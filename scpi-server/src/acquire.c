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
#include "../scpi-parser/libscpi/inc/scpi/error.h"
#include "scpi/parser.h"

rp_scpi_acq_unit_t unit     = RP_SCPI_VOLTS;        // default value

/* These structures are a direct API mirror 
and should not be altered! */
const scpi_choice_def_t scpi_RpGain[] = {
    {"LV", 0},
    {"HV", 1},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_RpChannel[] = {
    {"RP_CH_1", 0},
    {"RP_CH_2", 1},
    SCPI_CHOICE_LIST_END
};

scpi_result_t RP_AcqSetDataFormat(scpi_t *context) {
    const char * param;
    size_t param_len;

    // read first parameter Format type (BIN, ASCII)
    if (!SCPI_ParamCharacters(context, &param, &param_len, true)) {
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

scpi_result_t RP_AcqDecimation(scpi_t *context) {
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

scpi_result_t RP_AcqDecimationQ(scpi_t *context) {
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

scpi_result_t RP_AcqSamplingRate(scpi_t *context) {
    const char * param;
    size_t param_len;
    char samplingRateStr[15];

    // read first parameter SAMPLING_RATE (125MHz,15_6MHz, 1_9MHz,103_8kHz, 15_2kHz, 1_9kHz)
    if (!SCPI_ParamCharacters(context, &param, &param_len, true)) {
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

scpi_result_t RP_AcqSamplingRateQ(scpi_t *context) {
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
    SCPI_ResultMnemonic(context, samplingRateString);

    syslog(LOG_INFO, "*ACQ:SRAT? Successfully returned sampling rate.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqSamplingRateHzQ(scpi_t *context) {
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
    SCPI_ResultMnemonic(context, &samplingRateString);

    syslog(LOG_INFO, "*ACQ:SRA:HZ? Successfully returned sampling rate in Hz.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAveraging(scpi_t *context) {
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

scpi_result_t RP_AcqAveragingQ(scpi_t *context) {
    // get averaging
    bool value;
    int result = rp_AcqGetAveraging(&value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*ACQ:AVG? Failed to get averaging: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, value ? "ON" : "OFF");

    syslog(LOG_INFO, "*ACQ:AVG? Successfully returned averaging.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerSrc(scpi_t *context) {
    const char * param;
    size_t param_len;

    char triggerSource[15];

    // read first parameter TRIGGER SOURCE (DISABLED,NOW,CH1_PE,CH1_NE,CH2_PE,CH2_NE,EXT_PE,EXT_NE,AWG_PE)
    if (!SCPI_ParamCharacters(context, &param, &param_len, false)) {
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

scpi_result_t RP_AcqTriggerQ(scpi_t *context) {
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
    SCPI_ResultMnemonic(context, sourceString);

    syslog(LOG_INFO, "*ACQ:TRIG:STAT? Successfully returned trigger.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerDelay(scpi_t *context) {
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

scpi_result_t RP_AcqTriggerDelayQ(scpi_t *context) {
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

scpi_result_t RP_AcqTriggerDelayNs(scpi_t *context) {
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

scpi_result_t RP_AcqTriggerDelayNsQ(scpi_t *context) {
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

//Todo: Custom error handling.
scpi_result_t RP_AcqGain(scpi_t *context) {

    int32_t ch_usr[1];
    const char *name;
    int32_t param;

    SCPI_CommandNumbers(context, ch_usr, 1);

    if((ch_usr[0] != 0) && (ch_usr[0] != 1)){
        RP_ERR("ACQ:SOUR#:GAIN Invalid channel number", &ch_usr[0]);
        return SCPI_RES_ERR;
    }

    /* Get param val */
    if(!SCPI_ParamChoice(context, scpi_RpGain, &param, true)){
        RP_ERR("ACQ:SOUR#:GAIN is missing first parameter.", NULL);
        return SCPI_RES_ERR;
    }

    /* Get param name */
    if(!SCPI_ChoiceToName(scpi_RpGain, param, &name)){
        RP_ERR("ACQ:SOUR#:GAIN is missing first parameter.", NULL);
        return SCPI_RES_ERR;
    }

    rp_pinState_t state = param;
    rp_channel_t channel = ch_usr[0];

    if(rp_AcqSetGain(channel, state)){
        RP_ERR("ACQ:SOUR#:GAIN Failed to set gain", name);
        return SCPI_RES_ERR;
    }

    RP_INFO("ACQ:SOUR#:GAIN Successfully set gain.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqGainQ(scpi_t *context){

    int32_t ch_usr[1];
    rp_pinState_t state;

    SCPI_CommandNumbers(context, ch_usr, 1);

    if(ch_usr[0] != 0 && ch_usr[0] != 1){
        RP_ERR("ACQ:SOUR#:GAIN? Invalid channel number!", NULL);
        return SCPI_RES_ERR;
    }

    rp_channel_t channel;
    scpi_getRpChannel(ch_usr[0], &channel);

    if(rp_AcqGetGain(channel, &state)){
        RP_ERR("ACQ:SOUR#:GAIN? Failed to get gain.", NULL);
        return SCPI_RES_ERR;
    }

    /* Return data to client */
    SCPI_ResultMnemonic(context, state == RP_HIGH ? "HV" : "LV");
    RP_INFO("ACQ:SOUR#:GAIN? Successfully returned gain data.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerLevel(scpi_t *context) {
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

scpi_result_t RP_AcqTriggerLevelQ(scpi_t *context) {
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

scpi_result_t RP_AcqWritePointerQ(scpi_t *context) {
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

scpi_result_t RP_AcqWritePointerAtTrigQ(scpi_t *context) {
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
    if (!SCPI_ParamCharacters(context,  &param, &param_len, false)) {
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

scpi_result_t RP_AcqDataPosQ(scpi_t *context) {
    
    uint32_t start, end;
    int32_t ch_usr[1];
    int result;

    /* Read channel */
    SCPI_CommandNumbers(context, ch_usr, 1);
    if((ch_usr[0] != 0) && (ch_usr[0] != 1)){
        RP_ERR("*ACQ:SOUR#:DATA:STA:END? Invalid channel number", &ch_usr[0]);
        return SCPI_RES_ERR;
    }

    rp_channel_t channel = ch_usr[0];

    /* Read START parameter */
    if(!SCPI_ParamUnsignedInt(context, &start, true)){
        RP_ERR("*ACQ:SOUR#:DATA:STA:END? Unable to read START parameter.", NULL);
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamUnsignedInt(context, &end, true)){
        RP_ERR("*ACQ:SOUR#:DATA:STA:END? Unable to read END parameter.", NULL);
        return SCPI_RES_ERR;
    }

    uint32_t size = end - start;
    if(unit == RP_SCPI_VOLTS){
        float buffer[size];
        result = rp_AcqGetDataPosV(channel, start, end, buffer, &size);
        
        if(result != RP_OK){
            RP_ERR("*ACQ:SOUR#:DATA:STA:END? Failed to get data in volts", rp_GetError(result));
            return SCPI_RES_ERR;
        }
        
        SCPI_ResultBufferFloat(context, buffer, size);

    }else{
        int16_t buffer[size];
        result = rp_AcqGetDataPosRaw(channel, start, end, buffer, &size);
        
        if(result != RP_OK){
            RP_ERR("*ACQ:SOUR#:DATA:STA:END? Failed to get raw data", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferInt16(context, buffer, size);
    }
    RP_INFO("*ACQ:SOUR#:DATA:STA:END? Successfully returned data to client.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDataQ(scpi_t *context) {

    uint32_t start, size;
    int32_t ch_usr[1];
    int result;

    SCPI_CommandNumbers(context, ch_usr, 1);

    if(ch_usr[0] != 0 && ch_usr[0] != 1){
        RP_ERR("*ACQ:SOUR#:DATA:STA:N? Invalid channel number.", NULL);
        return SCPI_RES_ERR;
    }

    rp_channel_t channel = ch_usr[0];

    /* Parse START parameter */
    if(!SCPI_ParamUnsignedInt(context, &start, true)){
        RP_ERR("*ACQ:SOUR<n>:DATA:STA:N? is missing START parameter.", NULL);
        return SCPI_RES_ERR;
    }

    /* Parse SIZE parameter */
    if(!SCPI_ParamUnsignedInt(context, &size, true)){
        RP_ERR("*ACQ:SOUR<n>:DATA:STA:N? is missing SIZE parameter.", NULL);
        return SCPI_RES_ERR;
    }

    uint32_t size_buff;
    rp_AcqGetBufSize(&size_buff);
    if(unit == RP_SCPI_VOLTS){
        float buffer[size_buff - start];
        result = rp_AcqGetDataV(channel, start, &size, buffer);
        if(result != RP_OK){
            RP_ERR("*ACQ:SOUR<n>:DATA:STA:N? Failed to get data in volts", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferFloat(context, buffer, size);

    }else{
        int16_t buffer[size_buff - start];
        result = rp_AcqGetDataRaw(channel, start, &size, buffer);

        if(result != RP_OK){
            RP_ERR("*ACQ:SOUR<n>:DATA:STA:N? Failed to get raw data", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferInt16(context, buffer, size);
    }

    RP_INFO("*ACQ:SOUR<n>:DATA:STA:N? Successfully returned data.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDataOldestAllQ(scpi_t *context) {
    
    uint32_t size;
    int32_t ch_usr[1];
    int result;

    SCPI_CommandNumbers(context, ch_usr, 1);

    if(ch_usr[0] != 0 && ch_usr[0] != 1){
        RP_ERR("*ACQ:SOUR#:DATA? Invalid channel number.", NULL);
        return SCPI_RES_ERR;
    }

    rp_channel_t channel = ch_usr[0];
    rp_AcqGetBufSize(&size);
    if(unit == RP_SCPI_VOLTS){
        float buffer[size];
        result = rp_AcqGetOldestDataV(channel, &size, buffer);

        if(result != RP_OK){
            RP_ERR("*ACQ:SOUR#:DATA? Failed to get data in volt.", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferFloat(context, buffer, size);

    }else{
        int16_t buffer[size];
        result = rp_AcqGetOldestDataRaw(channel, &size, buffer);
        if(result != RP_OK){
            RP_ERR("*ACQ:SOUR#:DATA? Failed to get raw data.", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferInt16(context, buffer, size);
    }

    RP_INFO("*ACQ:SOUR#:DATA? Successfully returned data.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqOldestDataQ(scpi_t *context) {
    
    uint32_t size;
    int32_t ch_usr[1];
    int result;

    SCPI_CommandNumbers(context, ch_usr, 1);
    if(ch_usr[0] != 0 && ch_usr[0] != 1){
        RP_ERR("*ACQ:SOUR#:DATA:OLD:N? Invalid channel number.", NULL);
        return SCPI_RES_ERR;
    }

    rp_channel_t channel = ch_usr[0];

    if(!SCPI_ParamUnsignedInt(context, &size, true)){
        RP_ERR("*ACQ:SOUR#:DATA:OLD:N? Missing SIZE parameter.", NULL);
        return SCPI_RES_ERR;
    }

    if(unit == RP_SCPI_VOLTS){
        float buffer[size];
        result = rp_AcqGetOldestDataV(channel, &size, buffer);

        if(result != RP_OK){
            RP_ERR("*ACQ:SOUR#:DATA:OLD:N? Failed to get data in volt", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferFloat(context, buffer, size);

    }else{
        int16_t buffer[size];
        result = rp_AcqGetOldestDataRaw(channel, &size, buffer);
        if(result != RP_OK){
            RP_ERR("*ACQ:SOUR#:DATA:OLD:N? Failed to get raw data.", NULL);
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferInt16(context, buffer, size);
    }

    RP_INFO("*ACQ:SOUR#:DATA:OLD:N? Successfully returned data to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqLatestDataQ(scpi_t *context) {
    
    uint32_t size;
    int32_t ch_usr[1];
    int result;

    SCPI_CommandNumbers(context, ch_usr, 1);

    if(ch_usr[0] != 0 && ch_usr[0] != 1){
        RP_ERR("*ACQ:SOUR#:DATA:OLD:N? Invalid channel number.", NULL);
        return SCPI_RES_ERR;
    }

    rp_channel_t channel = ch_usr[0];

    if (!SCPI_ParamUnsignedInt(context, &size, true)) {
        RP_ERR("*ACQ:SOUR<n>:DATA:LAT:N? Missing first parameter", NULL);
        return SCPI_RES_ERR;
    }

    if(unit == RP_SCPI_VOLTS){
        float buffer[size];
        result = rp_AcqGetLatestDataV(channel, &size, buffer);

        if(result != RP_OK){
            RP_ERR("*ACQ:SOUR<n>:DATA:LAT:N? Failed to get data in volt", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferFloat(context, buffer, size);
    }else{
        int16_t buffer[size];
        result = rp_AcqGetLatestDataRaw(channel, &size, buffer);

        if(result != RP_OK){
            RP_ERR("*ACQ:SOUR<n>:DATA:LAT:N? Failed to get raw data.", rp_GetError(result));
        }

        SCPI_ResultBufferInt16(context, buffer, size);
    }

    RP_INFO("*ACQ:SOUR<n>:DATA:LAT:N? Successfully returned data to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqBufferSizeQ(scpi_t *context) {
    uint32_t size;
    int result = rp_AcqGetBufSize(&size);

    if (RP_OK != result) {
        RP_ERR("*ACQ:BUF:SIZE? Failed to get buffer size", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt(context, size);
    RP_INFO("*ACQ:BUF:SIZE?? Successfully returned buffer size.");

    return SCPI_RES_OK;
}