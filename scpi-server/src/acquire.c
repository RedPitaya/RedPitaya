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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "../scpi-parser/libscpi/inc/scpi/error.h"
#include "scpi/parser.h"

rp_scpi_acq_unit_t unit     = RP_SCPI_VOLTS;        // default value

/* These structures are a direct API mirror 
and should not be altered! */
const scpi_choice_def_t scpi_RpUnits[] = {
    {"VOLTS", 0},
    {"RAW", 1},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_RpGain[] = {
    {"LV", 0},
    {"HV", 1},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_RpDec[] = {
    {"D_1",     1},
    {"D_8",     2},
    {"D_64",    3},
    {"D_1024",  4},
    {"D_8192",  5},
    {"D_65536", 6},
};

const scpi_choice_def_t scpi_RpSmpRate[] = {
    {"S_125MHz",   0}, //!< Sample rate 125Msps; Buffer time length 131us; Decimation 1
    {"S_15_6MHz",  1}, //!< Sample rate 15.625Msps; Buffer time length 1.048ms; Decimation 8
    {"S_1_9MHz",   2}, //!< Sample rate 1.953Msps; Buffer time length 8.388ms; Decimation 64
    {"S_103_8kHz", 3}, //!< Sample rate 122.070ksps; Buffer time length 134.2ms; Decimation 1024
    {"S_15_2kHz",  4}, //!< Sample rate 15.258ksps; Buffer time length 1.073s; Decimation 8192
    {"S_1_9kHz",   5},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_RpTrigSrc[] = {
    {"DISABLED",    0},
    {"NOW",         1},
    {"CH1_PE",      2},
    {"CH1_NE",      3},
    {"CH2_PE",      4},
    {"CH2_NE",      5},
    {"EXT_PE",      6},
    {"EXT_NE",      7},
    {"AWG_PE",      8},
    {"AWG_NE",      9},
    SCPI_CHOICE_LIST_END
};

scpi_result_t RP_AcqSetDataFormat(scpi_t *context) {
    const char * param;
    size_t param_len;

    // read first parameter Format type (BIN, ASCII)
    if (!SCPI_ParamCharacters(context, &param, &param_len, true)) {
        RP_ERR("*ACQ:DATA:FORMAT is missing first parameter.", NULL);
        return SCPI_RES_ERR;
    }

    if (strncasecmp(param, "BIN", param_len) == 0) {
        context->binary_output = true;
        RP_INFO("*ACQ:DATA:FORMAT set to BIN");
    }
    else if (strncasecmp(param, "ASCII", param_len) == 0) {
        context->binary_output = false;
        RP_INFO("*ACQ:DATA:FORMAT set to ASCII");
    }
    else {
        RP_ERR("*ACQ:DATA:FORMAT wrong argument value", NULL);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


scpi_result_t RP_AcqStart(scpi_t *context) {
    int result = rp_AcqStart();

    if (RP_OK != result) {
        RP_ERR("*ACQ:START Failed", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*ACQ:START Successful started Red Pitaya acquire.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqStop(scpi_t *context) {
    int result = rp_AcqStop();

    if (RP_OK != result) {
        RP_ERR("*ACQ:STOP Failed", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_ERR("*ACQ:STOP Successful stopped Red Pitaya acquire.", NULL);
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqReset(scpi_t *context) {
    int result = rp_AcqReset();

    if (RP_OK != result) {
        RP_ERR("*ACQ:RST Failed", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    unit = RP_SCPI_VOLTS;
    context->binary_output = false;

    RP_INFO("*ACQ:RST Successful reset  Red Pitaya acquire.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDecimation(scpi_t *context) {
    
    int32_t choice;

    /* Read DECIMATION parameter */
    if (!SCPI_ParamChoice(context, scpi_RpDec, &choice, true)) {
        RP_ERR("*ACQ:DEC is missing first parameter.", NULL);
        return SCPI_RES_ERR;
    }

    // Convert decimation to rp_acq_decimation_t
    rp_acq_decimation_t decimation = choice;

    // Now set the decimation
    int result = rp_AcqSetDecimation(decimation);
    if (RP_OK != result) {
        RP_ERR("*ACQ:DEC Failed to set decimation", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*ACQ:DEC Successfully set decimation.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDecimationQ(scpi_t *context) {
    
    const char *dec_name;

    // Get decimation
    rp_acq_decimation_t decimation;
    int result = rp_AcqGetDecimation(&decimation);

    if (RP_OK != result) {
        RP_ERR("*ACQ:DEC? Failed to get decimation", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    /* Parse decimation to choice */
    if(!SCPI_ChoiceToName(scpi_RpDec, decimation, &dec_name)){
        RP_ERR("*ACQ:DEC? Failed to get decimation.", NULL);
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, dec_name);

    RP_INFO("*ACQ:DEC? Successfully returned decimation.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqSamplingRate(scpi_t *context) {
    
    int32_t choice;

    if(!SCPI_ParamChoice(context, scpi_RpSmpRate, &choice, true)){
        RP_ERR("*ACQ:SRAT Missing SAMPLE RATE parameter.", NULL);
        return SCPI_RES_ERR;
    }

    rp_acq_sampling_rate_t  samplingRate = choice;
    int result = rp_AcqSetSamplingRate(samplingRate);

    if (RP_OK != result) {
        RP_ERR("*ACQ:SRAT Failed to set sampling rate", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*ACQ:SRAT Successfully set sampling rate.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqSamplingRateQ(scpi_t *context) {
    
    const char *smp_name;
    rp_acq_sampling_rate_t samplingRate;
    int result = rp_AcqGetSamplingRate(&samplingRate);

    if (RP_OK != result) {
        RP_ERR("*ACQ:SRAT? Failed to get sampling rate", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    /* Parse sampling rate to choice */
    if(!SCPI_ChoiceToName(scpi_RpSmpRate, samplingRate, &smp_name)){
        RP_ERR("*ACQ:SRAT? Failed to get sampling rate.", NULL);
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, smp_name);
    RP_INFO("*ACQ:SRAT? Successfully returned sampling rate.");

    return SCPI_RES_OK;
}

//TODO: Redundand function?
scpi_result_t RP_AcqSamplingRateHzQ(scpi_t *context) {
    
    // get sampling rate
    float samplingRate;
    int result = rp_AcqGetSamplingRateHz(&samplingRate);

    if (RP_OK != result) {
        RP_ERR("*ACQ:SRA:HZ? Failed to get sampling rate in Hz", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back string result
    char samplingRateString;
    sprintf(&samplingRateString, "%.0f Hz", samplingRate);

    //Return string in form "<Value> Hz"
    SCPI_ResultMnemonic(context, &samplingRateString);

    RP_INFO("*ACQ:SRA:HZ? Successfully returned sampling rate in Hz.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAveraging(scpi_t *context) {
    scpi_bool_t value;

    // read first parameter AVERAGING (OFF,ON)
    if (!SCPI_ParamBool(context, &value, false)) {
        RP_ERR("*ACQ:AVGT is missing first parameter.", NULL);
        return SCPI_RES_ERR;
    }

    // Now set the averaging
    int result = rp_AcqSetAveraging(value);

    if (RP_OK != result) {
        RP_ERR("*ACQ:AVGT Failed to set averaging", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*ACQ:AVG Successfully set averaging.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAveragingQ(scpi_t *context) {
    // get averaging
    bool value;
    int result = rp_AcqGetAveraging(&value);

    if (RP_OK != result) {
        RP_ERR("*ACQ:AVG? Failed to get averaging", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, value ? "ON" : "OFF");

    RP_INFO("*ACQ:AVG? Successfully returned averaging.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerSrc(scpi_t *context) {
    
    int32_t trig_src;

    /* Read TRIGGER SOURCE parameter */
    if (!SCPI_ParamChoice(context, scpi_RpTrigSrc, &trig_src, true)) {
        RP_ERR("*ACQ:TRIG is missing first parameter.", NULL);
        return SCPI_RES_ERR;
    }

    rp_acq_trig_src_t source = trig_src;

    // Now set the trigger source
    int result = rp_AcqSetTriggerSrc(source);
    if (RP_OK != result) {
        RP_ERR("*ACQ:TRIG Failed to set trigger source", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*ACQ:TRIG Successfully set trigger source.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerQ(scpi_t *context) {
    
    const char *trig_name;
    // get trigger source
    rp_acq_trig_src_t source;
    int result = rp_AcqGetTriggerSrc(&source);

    if (RP_OK != result) {
        RP_ERR("*ACQ:TRIG:STAT? Failed to get trigger", rp_GetError(result));
        source = RP_TRIG_SRC_NOW;   // Some value not equal to DISABLE -> function return "WAIT"
    }

    if(!SCPI_ChoiceToName(scpi_RpTrigSrc, source, &trig_name)){
        RP_ERR("*ACQ:TRIG:STAT? Failed to parse trigger source.", NULL);
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, trig_name);

    RP_INFO("*ACQ:TRIG:STAT? Successfully returned trigger.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerDelay(scpi_t *context) {
    int32_t triggerDelay;

    // read first parameter TRIGGER DELAY (value in samples)
    if (!SCPI_ParamInt32(context, &triggerDelay, false)) {
        triggerDelay = 0;
    }

    // Now set the trigger delay
    int result = rp_AcqSetTriggerDelay(triggerDelay);

    if (RP_OK != result) {
        RP_ERR("*ACQ:TRIG:DLY Failed to set trigger delay", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*ACQ:TRIG:DLY Successfully set trigger delay.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerDelayQ(scpi_t *context) {
    // get trigger delay
    int32_t value;
    int result = rp_AcqGetTriggerDelay(&value);

    if (RP_OK != result) {
        RP_ERR("*ACQ:TRIG:DLY? Failed to get trigger delay", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultInt32(context, value);

    RP_INFO("*ACQ:TRIG:DLY? Successfully returned trigger delay.");

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
        RP_ERR("*ACQ:TRIG:DLY:NS Failed to set trigger delay in ns", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*ACQ:TRIG:DLY:NS Successfully set trigger delay.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerDelayNsQ(scpi_t *context) {
    // get trigger delay ns
    int64_t value;
    int result = rp_AcqGetTriggerDelayNs(&value);

    if (RP_OK != result) {
        RP_ERR("*ACQ:TRIG:DLY:NS? Failed to get trigger delay", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultLong(context, value);

    RP_INFO("*ACQ:TRIG:DLY:NS? Successfully returned trigger delay in ns.");

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

    rp_channel_t channel = ch_usr[0];

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
        RP_ERR("*ACQ:TRIG:LEV Failed to set trigger level", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*ACQ:TRIG:LEV Successfully set trigger level.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerLevelQ(scpi_t *context) {
    float value;
    int result = rp_AcqGetTriggerLevel(&value);

    if (RP_OK != result) {
        RP_ERR("*ACQ:TRIG:LEV? Failed to get trigger level", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    value = value * 1000;       // convert to milli volts
    // Return back result
    SCPI_ResultDouble(context, value);

    RP_INFO("*ACQ:TRIG:LEV? Successfully returned trigger level.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqWritePointerQ(scpi_t *context) {
    // get write pointer
    uint32_t value;
    int result = rp_AcqGetWritePointer(&value);

    if (RP_OK != result) {
        RP_ERR("*ACQ:WPOS? Failed to get writer position", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt(context, value);

    RP_INFO("*ACQ:WPOS? Successfully returned writer position.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqWritePointerAtTrigQ(scpi_t *context) {
    // get write pointer at trigger
    uint32_t value;
    int result = rp_AcqGetWritePointerAtTrig(&value);

    if (RP_OK != result) {
        RP_ERR("*ACQ:TPOS? Failed to get writer position at trigger", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt(context, value);

    RP_INFO("*ACQ:TPOS? Successfully returned writer position at trigger.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqScpiDataUnits(scpi_t *context) {
    
    int32_t choice;

    /* Read UNITS parameters */
    if(!SCPI_ParamChoice(context, scpi_RpUnits, &choice, true)){
        RP_ERR("*ACQ:DATA:UNITS Missing first parameter.", NULL);
        return SCPI_RES_ERR;
    }

    /* Set global units for acq scpi */
    unit = choice;

    RP_INFO("*ACQ:DATA:UNITS Successfully set scpi units.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqScpiDataUnitsQ(scpi_t *context){

    const char *units;

    if(!SCPI_ChoiceToName(scpi_RpUnits, unit, &units)){
        RP_ERR("*ACQ:DATA:UNITS? Failed to get data units.", NULL);
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, units);
    RP_INFO("*ACQ:DATA:UNITS? Successfully returned data to client.");
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
    if(!SCPI_ParamUInt32(context, &start, true)){
        RP_ERR("*ACQ:SOUR#:DATA:STA:END? Unable to read START parameter.", NULL);
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamUInt32(context, &end, true)){
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
    if(!SCPI_ParamUInt32(context, &start, true)){
        RP_ERR("*ACQ:SOUR<n>:DATA:STA:N? is missing START parameter.", NULL);
        return SCPI_RES_ERR;
    }

    /* Parse SIZE parameter */
    if(!SCPI_ParamUInt32(context, &size, true)){
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

    if(!SCPI_ParamUInt32(context, &size, true)){
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

    if (!SCPI_ParamUInt32(context, &size, true)) {
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
