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
#include <new>

#include "acquire.h"
#include "common.h"

#include "scpi/parser.h"
#include "scpi/units.h"


rp_scpi_acq_unit_t unit     = RP_SCPI_VOLTS;        // default value


scpi_result_t RP_AcqSetDataFormat(scpi_t *context) {
    const char * param;
    size_t param_len;

    // read first parameter Format type (BIN, ASCII)
    if (!SCPI_ParamCharacters(context, &param, &param_len, true)) {
        RP_LOG(context,LOG_ERR, "*ACQ:DATA:FORMAT is missing first parameter.");
        return SCPI_RES_ERR;
    }

    if (strncasecmp(param, "BIN", param_len) == 0) {
        context->binary_output = true;
        RP_LOG(context,LOG_INFO, "*ACQ:DATA:FORMAT set to BIN");
    }
    else if (strncasecmp(param, "ASCII", param_len) == 0) {
        context->binary_output = false;
        RP_LOG(context,LOG_INFO, "*ACQ:DATA:FORMAT set to ASCII");
    }
    else {
        RP_LOG(context,LOG_ERR, "*ACQ:DATA:FORMAT wrong argument value");
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


scpi_result_t RP_AcqStart(scpi_t *context) {
    int result = rp_AcqStart();

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:START Failed to start Red Pitaya acquire: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*ACQ:START Successful started Red Pitaya acquire.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqStop(scpi_t *context) {
    int result = rp_AcqStop();

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:STOP Failed to stop Red Pitaya acquisition: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*ACQ:STOP Successful stopped Red Pitaya acquire.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqReset(scpi_t *context) {
    int result = rp_AcqReset();

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:RST Failed to reset Red Pitaya acquire: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    unit = RP_SCPI_VOLTS;
    context->binary_output = false;

    RP_LOG(context,LOG_INFO, "*ACQ:RST Successful reset Red Pitaya acquire.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDecimation(scpi_t *context) {

    uint32_t value;

    /* Read DECIMATION parameter */
    if (!SCPI_ParamUInt32(context, &value, true)) {
        RP_LOG(context,LOG_ERR, "*ACQ:DEC is missing first parameter.");
        return SCPI_RES_ERR;
    }

    // Convert decimation to rp_acq_decimation_t
    rp_acq_decimation_t decimation;
    if (rp_AcqConvertFactorToDecimation(value, &decimation)) {
        syslog(LOG_ERR, "*ACQ:DEC parameter decimation is invalid.");
        return SCPI_RES_ERR;
    }

    // Now set the decimation
    int result = rp_AcqSetDecimation(decimation);
    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:DEC Failed to set decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*ACQ:DEC Successfully set decimation.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDecimationQ(scpi_t *context) {
    // Get decimation
    rp_acq_decimation_t decimation;
    int result = rp_AcqGetDecimation(&decimation);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:DEC? Failed to get decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Convert decimation to int
    uint32_t value = (uint32_t)decimation;

    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);

    RP_LOG(context,LOG_INFO, "*ACQ:DEC? Successfully returned decimation.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqSamplingRateHzQ(scpi_t *context) {

    // get sampling rate
    float samplingRate;
    int result = rp_AcqGetSamplingRateHz(&samplingRate);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:SRA:HZ? Failed to get sampling rate in Hz: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back string result
    char samplingRateString[13];
    sprintf(samplingRateString, "%9.0f Hz", samplingRate);

    //Return string in form "<Value> Hz"
    SCPI_ResultMnemonic(context, samplingRateString);

    RP_LOG(context,LOG_INFO, "*ACQ:SRA:HZ? Successfully returned sampling rate in Hz.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAveraging(scpi_t *context) {

    scpi_bool_t value;

    // read first parameter AVERAGING (OFF,ON)
    if (!SCPI_ParamBool(context, &value, false)) {
        RP_LOG(context,LOG_ERR, "*ACQ:AVGT is missing first parameter.");
        return SCPI_RES_ERR;
    }

    // Now set the averaging
    int result = rp_AcqSetAveraging(value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:AVGT Failed to set averaging: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*ACQ:AVG Successfully set averaging.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAveragingQ(scpi_t *context) {
    // get averaging
    bool value;
    int result = rp_AcqGetAveraging(&value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:AVG? Failed to get averaging: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, value ? "ON" : "OFF");

    RP_LOG(context,LOG_INFO, "*ACQ:AVG? Successfully returned averaging.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerSrc(scpi_t *context) {

    int32_t trig_src;

    /* Read TRIGGER SOURCE parameter */
    if (!SCPI_ParamChoice(context, scpi_RpTrigSrc, &trig_src, true)) {
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG is missing first parameter.");
        return SCPI_RES_ERR;
    }

    rp_acq_trig_src_t source = (rp_acq_trig_src_t)trig_src;

    // Now set the trigger source
    int result = rp_AcqSetTriggerSrc(source);
    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG Failed to set trigger source: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*ACQ:TRIG Successfully set trigger source.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerSrcQ(scpi_t *context) {

    const char *trig_name;
    // get trigger source
    rp_acq_trig_src_t source;
    int result = rp_AcqGetTriggerSrc(&source);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:STAT? Failed to get trigger: %s", rp_GetError(result));
        source = RP_TRIG_SRC_NOW;   // Some value not equal to DISABLE -> function return "WAIT"
    }

    if(!SCPI_ChoiceToName(scpi_RpTrigStat, source, &trig_name)){
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:STAT? Failed to parse trigger source.");
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, trig_name);

    RP_LOG(context,LOG_INFO, "*ACQ:TRIG:STAT? Successfully returned trigger.");
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
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:DLY Failed to set trigger delay: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*ACQ:TRIG:DLY Successfully set trigger delay.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerDelayQ(scpi_t *context) {
    // get trigger delay
    int32_t value;
    int result = rp_AcqGetTriggerDelay(&value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:DLY? Failed to get trigger delay: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultInt32(context, value);

    RP_LOG(context,LOG_INFO, "*ACQ:TRIG:DLY? Successfully returned trigger delay.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerDelayNs(scpi_t *context) {
    int64_t triggerDelay;

    // read first parameter TRIGGER DELAY ns (value in ns)
    if (!SCPI_ParamInt64(context, &triggerDelay, false)) {
        triggerDelay = 0;
    }

    // Now set the trigger delay in ns
    int result = rp_AcqSetTriggerDelayNs(triggerDelay);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:DLY:NS Failed to set trigger delay in ns: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*ACQ:TRIG:DLY:NS Successfully set trigger delay.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerDelayNsQ(scpi_t *context) {
    // get trigger delay ns
    int64_t value;
    int result = rp_AcqGetTriggerDelayNs(&value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:DLY:NS? Failed to get trigger delay: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultInt32(context, value);

    RP_LOG(context,LOG_INFO, "*ACQ:TRIG:DLY:NS? Successfully returned trigger delay in ns.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerHyst(scpi_t *context){

    int result;
    float voltage;

    if(!SCPI_ParamFloat(context, &voltage, true)){
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:HYST Missing first parameter.");
        return SCPI_RES_ERR;
    }

    result = rp_AcqSetTriggerHyst(voltage);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:HYST Failed to set trigger "
            "hysteresis: %s", rp_GetError(result));

        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*ACQ:TRIG:HYST Successfully set trigger hysteresis.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerHystQ(scpi_t *context){

    int result;
    float voltage;

    result = rp_AcqGetTriggerHyst(&voltage);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:HYST Failed to get trigger "
            "hysteresis: %s", rp_GetError(result));

        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, voltage);

    RP_LOG(context,LOG_INFO, "*ACQ:TRIG:HYST Successfully returned "
        "hysteresis value to client.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerFillQ(scpi_t *context){

    int result;
    bool fillRes;

    result = rp_AcqGetBufferFillState(&fillRes);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:FILL Failed to get trigger "
            "fill state: %s", rp_GetError(result));

        return SCPI_RES_ERR;
    }
    SCPI_ResultInt32(context, fillRes);

    RP_LOG(context,LOG_INFO, "*ACQ:TRIG:FILL Successfully returned "
        "fill state value to client.");

    return SCPI_RES_OK;
}

//Todo: Custom error handling.
scpi_result_t RP_AcqGain(scpi_t *context) {

    const char *name;
    int32_t param;

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    /* Get param val */
    if(!SCPI_ParamChoice(context, scpi_RpGain, &param, true)){
        RP_LOG(context,LOG_ERR, "ACQ:SOUR#:GAIN is missing first parameter.");
        return SCPI_RES_ERR;
    }

    /* Get param name */
    if(!SCPI_ChoiceToName(scpi_RpGain, param, &name)){
        RP_LOG(context,LOG_ERR, "ACQ:SOUR#:GAIN is missing first parameter.");
        return SCPI_RES_ERR;
    }

    rp_pinState_t state = (rp_pinState_t)param;

    if(rp_AcqSetGain(channel, state)){
        RP_LOG(context,LOG_ERR, "ACQ:SOUR#:GAIN Failed to set gain: %s", &name[0]);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "ACQ:SOUR#:GAIN Successfully set gain.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqGainQ(scpi_t *context){

    rp_pinState_t state;
    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    int result = rp_AcqGetGain(channel, &state);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "ACQ:SOUR#:GAIN? Failed to get gain: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    /* Return data to client */
    SCPI_ResultMnemonic(context, state == RP_HIGH ? "HV" : "LV");

    RP_LOG(context,LOG_INFO, "ACQ:SOUR#:GAIN? Successfully returned gain data.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerLevel(scpi_t *context) {
    scpi_number_t value;

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:LEV is missing first parameter.");
        return SCPI_RES_ERR;
    }

    uint8_t channels = getADCChannels(context);
    // Now set threshold
    int result = 0;
    if (channels >= 1){
        result = rp_AcqSetTriggerLevel(RP_T_CH_1, (float) value.value);
        if (RP_OK != result) {
            RP_LOG(context,LOG_ERR, "*ACQ:TRIG:LEV [Ch1] Failed to set trigger level: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }

    if (channels >= 2){
        result = rp_AcqSetTriggerLevel(RP_T_CH_2, (float) value.value);
        if (RP_OK != result) {
            RP_LOG(context,LOG_ERR, "*ACQ:TRIG:LEV [Ch2] Failed to set trigger level: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }

    if (channels >= 3){
        result = rp_AcqSetTriggerLevel(RP_T_CH_3, (float) value.value);
        if (RP_OK != result) {
            RP_LOG(context,LOG_ERR, "*ACQ:TRIG:LEV [Ch3] Failed to set trigger level: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }

    if (channels >= 4){
        result = rp_AcqSetTriggerLevel(RP_T_CH_4, (float) value.value);
        if (RP_OK != result) {
            RP_LOG(context,LOG_ERR, "*ACQ:TRIG:LEV [Ch4] Failed to set trigger level: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }


    RP_LOG(context,LOG_INFO, "*ACQ:TRIG:LEV Successfully set trigger level.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerLevelQ(scpi_t *context) {
    float value;
    int result = rp_AcqGetTriggerLevel(RP_T_CH_1,&value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:LEV? Failed to get "
            "trigger level: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    value = value;
    // Return back result
    SCPI_ResultDouble(context, value);

    RP_LOG(context,LOG_INFO, "*ACQ:TRIG:LEV? Successfully returned trigger level.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqWritePointerQ(scpi_t *context) {
    // get write pointer
    uint32_t value;
    int result = rp_AcqGetWritePointer(&value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:WPOS? Failed to get writer "
            "position: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);

    RP_LOG(context,LOG_INFO, "*ACQ:WPOS? Successfully returned writer position.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqWritePointerAtTrigQ(scpi_t *context) {
    // get write pointer at trigger
    uint32_t value;
    int result = rp_AcqGetWritePointerAtTrig(&value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:TPOS? Failed to get writer position at trigger: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);

    RP_LOG(context,LOG_INFO, "*ACQ:TPOS? Successfully returned writer position at trigger.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqScpiDataUnits(scpi_t *context) {

    int32_t choice;

    /* Read UNITS parameters */
    if(!SCPI_ParamChoice(context, scpi_RpUnits, &choice, true)){
        RP_LOG(context,LOG_ERR, "*ACQ:DATA:UNITS Missing first parameter.");
        return SCPI_RES_ERR;
    }

    /* Set global units for acq scpi */
    unit = (rp_scpi_acq_unit_t)choice;

    RP_LOG(context,LOG_INFO, "*ACQ:DATA:UNITS Successfully set scpi units.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqScpiDataUnitsQ(scpi_t *context){

    const char *units;

    if(!SCPI_ChoiceToName(scpi_RpUnits, unit, &units)){
        RP_LOG(context,LOG_ERR, "*ACQ:DATA:UNITS? Failed to get data units.");
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, units);

    RP_LOG(context,LOG_INFO, "*ACQ:DATA:UNITS? Successfully returned data to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDataPosQ(scpi_t *context) {

    uint32_t start, end;
    int result;

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    /* Read START parameter */
    if(!SCPI_ParamUInt32(context, &start, true)){
        RP_LOG(context,LOG_ERR, "*ACQ:SOUR#:DATA:STA:END? Unable to read START parameter.");
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamUInt32(context, &end, true)){
        RP_LOG(context,LOG_ERR, "*ACQ:SOUR#:DATA:STA:END? Unable to read END parameter.");
        return SCPI_RES_ERR;
    }
    
    uint32_t size_buff;
    rp_AcqGetBufSize(&size_buff);

    uint32_t size = ((end + size_buff)  - start) % size_buff + 1;
    if(unit == RP_SCPI_VOLTS){
        float *buffer = nullptr;
        try{
            buffer = new float[size];
        }catch(const std::bad_alloc &)
        {
            RP_LOG(context,LOG_ERR, "*ACQ:SOUR#:DATA:STA:END? Failed allocate buffer");
            return SCPI_RES_ERR;
        };

        result = rp_AcqGetDataPosV(channel, start, end, buffer, &size);

        if(result != RP_OK){
            RP_LOG(context,LOG_ERR, "*ACQ:SOUR#:DATA:STA:END? Failed to get data in volts: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferFloat(context, buffer, size);
        delete[] buffer;
    }else{
        int16_t *buffer = nullptr;
        try{
            buffer = new int16_t[size];
        }catch(const std::bad_alloc &)
        {
            RP_LOG(context,LOG_ERR, "*ACQ:SOUR#:DATA:STA:END? Failed allocate buffer");
            return SCPI_RES_ERR;
        };

        result = rp_AcqGetDataPosRaw(channel, start, end, buffer, &size);

        if(result != RP_OK){
            RP_LOG(context,LOG_ERR, "*ACQ:SOUR#:DATA:STA:END? Failed to get raw data: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferInt16(context, buffer, size);
        delete[] buffer;
    }

    RP_LOG(context,LOG_INFO, "*ACQ:SOUR#:DATA:STA:END? Successfully returned data to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDataQ(scpi_t *context) {

    uint32_t start, size;
    int result;

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    /* Parse START parameter */
    if(!SCPI_ParamUInt32(context, &start, true)){
        RP_LOG(context,LOG_ERR, "*ACQ:SOUR<n>:DATA:STA:N? is missing START parameter.");
        return SCPI_RES_ERR;
    }

    /* Parse SIZE parameter */
    if(!SCPI_ParamUInt32(context, &size, true)){
        RP_LOG(context,LOG_INFO, "*ACQ:SOUR<n>:DATA:STA:N? is missing SIZE parameter.");
        return SCPI_RES_ERR;
    }

    uint32_t size_buff;
    rp_AcqGetBufSize(&size_buff);
    if(unit == RP_SCPI_VOLTS){
        float *buffer = nullptr;
        try{
            buffer = new float[size];
        }catch(std::bad_alloc &)
        {
            RP_LOG(context,LOG_ERR, "*ACQ:SOUR<n>:DATA:STA:N? Failed allocate buffer");
            return SCPI_RES_ERR;
        };
        result = rp_AcqGetDataV(channel, start, &size, buffer);
        if(result != RP_OK){
            RP_LOG(context,LOG_ERR, "*ACQ:SOUR<n>:DATA:STA:N? Failed to get "
            "data in volts: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferFloat(context, buffer, size);
        delete[] buffer;

    }else{
        int16_t *buffer = nullptr;
        try{
            buffer = new int16_t[size];
        }catch(std::bad_alloc &)
        {
            RP_LOG(context,LOG_ERR, "*ACQ:SOUR<n>:DATA:STA:N? Failed allocate buffer");
            return SCPI_RES_ERR;
        };
        result = rp_AcqGetDataRaw(channel, start, &size, buffer);

        if(result != RP_OK){
            RP_LOG(context,LOG_ERR, "*ACQ:SOUR<n>:DATA:STA:N? Failed to get raw data: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferInt16(context, buffer, size);
        delete[] buffer;
    }

    RP_LOG(context,LOG_INFO, "*ACQ:SOUR<n>:DATA:STA:N? Successfully returned data.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDataOldestAllQ(scpi_t *context) {

    uint32_t size;
    int result;

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    rp_AcqGetBufSize(&size);
    if(unit == RP_SCPI_VOLTS){
        float buffer[size];
        result = rp_AcqGetOldestDataV(channel, &size, buffer);

        if(result != RP_OK){
            RP_LOG(context,LOG_ERR, "*ACQ:SOUR#:DATA? Failed to get data in volt: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferFloat(context, buffer, size);

    }else{
        int16_t buffer[size];
        result = rp_AcqGetOldestDataRaw(channel, &size, buffer);
        if(result != RP_OK){
            RP_LOG(context,LOG_ERR, "*ACQ:SOUR#:DATA? Failed to get raw data: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferInt16(context, buffer, size);
    }

    RP_LOG(context,LOG_INFO, "*ACQ:SOUR#:DATA? Successfully returned data.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqOldestDataQ(scpi_t *context) {

    uint32_t size;
    int result;

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamUInt32(context, &size, true)){
        RP_LOG(context,LOG_ERR, "*ACQ:SOUR#:DATA:OLD:N? Missing SIZE parameter.");
        return SCPI_RES_ERR;
    }

    if(unit == RP_SCPI_VOLTS){
        float buffer[size];
        result = rp_AcqGetOldestDataV(channel, &size, buffer);

        if(result != RP_OK){
            RP_LOG(context,LOG_ERR, "*ACQ:SOUR#:DATA:OLD:N? Failed to get data in "
                "volt: %s", rp_GetError(result));

            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferFloat(context, buffer, size);

    }else{
        int16_t buffer[size];
        result = rp_AcqGetOldestDataRaw(channel, &size, buffer);
        if(result != RP_OK){
            RP_LOG(context,LOG_ERR, "*ACQ:SOUR#:DATA:OLD:N? Failed to get raw data: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferInt16(context, buffer, size);
    }

    RP_LOG(context,LOG_INFO, "*ACQ:SOUR#:DATA:OLD:N? Successfully returned data to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqLatestDataQ(scpi_t *context) {

    uint32_t size;
    int result;

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &size, true)) {
        RP_LOG(context,LOG_ERR, "*ACQ:SOUR<n>:DATA:LAT:N? Missing first parameter.");
        return SCPI_RES_ERR;
    }

    if(unit == RP_SCPI_VOLTS){
        float buffer[size];
        result = rp_AcqGetLatestDataV(channel, &size, buffer);

        if(result != RP_OK){
            RP_LOG(context,LOG_INFO, "*ACQ:SOUR<n>:DATA:LAT:N? Failed to "
                " get data in volt: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferFloat(context, buffer, size);
    }else{
        int16_t buffer[size];
        result = rp_AcqGetLatestDataRaw(channel, &size, buffer);

        if(result != RP_OK){
            RP_LOG(context,LOG_ERR, "*ACQ:SOUR<n>:DATA:LAT:N? Failed to "
                "get raw data: %s", rp_GetError(result));
        }

        SCPI_ResultBufferInt16(context, buffer, size);
    }

    RP_LOG(context,LOG_INFO, "*ACQ:SOUR<n>:DATA:LAT:N? Successfully returned data to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqBufferSizeQ(scpi_t *context) {
    uint32_t size;
    int result = rp_AcqGetBufSize(&size);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:BUF:SIZE? Failed to get buffer size: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, size, 10);

    RP_LOG(context,LOG_INFO, "*ACQ:BUF:SIZE?? Successfully returned buffer size.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAC_DC(scpi_t * context){
    const char *name;
    int32_t param;

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    /* Get param val */
    if(!SCPI_ParamChoice(context, scpi_RpAC_DC, &param, true)){
        RP_LOG(context,LOG_ERR, "ACQ:SOUR#:COUP is missing first parameter.");
        return SCPI_RES_ERR;
    }

    /* Get param name */
    if(!SCPI_ChoiceToName(scpi_RpAC_DC, param, &name)){
        RP_LOG(context,LOG_ERR, "ACQ:SOUR#:COUP is missing first parameter.");
        return SCPI_RES_ERR;
    }

    rp_acq_ac_dc_mode_t state = (rp_acq_ac_dc_mode_t)param;

    if(rp_AcqSetAC_DC(channel, state)){
        RP_LOG(context,LOG_ERR, "ACQ:SOUR#:COUP Failed to set AC/DC mode: %s", &name[0]);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "ACQ:SOUR#:COUP Successfully set AC/DC mode.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAC_DCQ(scpi_t * context){
    rp_acq_ac_dc_mode_t state;
    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    int result = rp_AcqGetAC_DC(channel, &state);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "ACQ:SOUR#:COUP? Failed to get gain: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    /* Return data to client */
    SCPI_ResultMnemonic(context, state == RP_DC ? "DC" : "AC");

    RP_LOG(context,LOG_INFO, "ACQ:SOUR#:COUP? Successfully returned gain data.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqExtTriggerLevel(scpi_t *context) {
    scpi_number_t value;

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:LEV is missing first parameter.");
        return SCPI_RES_ERR;
    }

    // Now set threshold
    int result = 0;
    result = rp_AcqSetTriggerLevel(RP_T_CH_EXT, (float) value.value);
    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:EXT:LEV Failed to set trigger level: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }


    RP_LOG(context,LOG_INFO, "*ACQ:TRIG:EXT:LEV Successfully set trigger level.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqExtTriggerLevelQ(scpi_t *context) {
    float value;
    int result = rp_AcqGetTriggerLevel(RP_T_CH_EXT,&value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:EXT:LEV? Failed to get "
            "trigger level: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultDouble(context, value);

    RP_LOG(context,LOG_INFO, "*ACQ:TRIG:EXT:LEV? Successfully returned trigger level.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqExtTriggerDebouncerUs(scpi_t *context) {
    scpi_number_t value;

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:EXT:DEBouncerUs is missing first parameter.");
        return SCPI_RES_ERR;
    }

    // Now set threshold
    int result = 0;
    result = rp_AcqSetExtTriggerDebouncerUs((double) value.value);
    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:EXT:DEBouncerUs Failed to set: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }


    RP_LOG(context,LOG_INFO, "*ACQ:TRIG:EXT:DEBouncerUs Successfully set value.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqExtTriggerDebouncerUsQ(scpi_t *context) {
    double value;
    int result = rp_AcqGetExtTriggerDebouncerUs(&value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:TRIG:EXT:DEBouncerUs? Failed to get: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultDouble(context, value);

    RP_LOG(context,LOG_INFO, "*ACQ:TRIG:EXT:DEBouncerUs? Successfully returned value.");
    return SCPI_RES_OK;
}