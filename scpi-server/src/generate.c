/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server generate SCPI commands implementation
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
#include "generate.h"
#include "../../api/rpbase/src/generate.h"

#include "utils.h"
#include "scpi/parser.h"

/* These structures are a direct API mirror 
and should not be altered! */
const scpi_choice_def_t scpi_RpWForm[] = {
    {"SINE",        0},
    {"SQUARE",      1},
    {"TRIANGLE",    2},
    {"SAWU",        3},
    {"SAWD",        4},
    {"PWM",         5},
    {"ARBITRARY",   6},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_RpGenMode[] = {
    {"OFF",  0},
    {"ON",   1},
    SCPI_CHOICE_LIST_END
};

scpi_result_t RP_GenReset(scpi_t *context) {
    int result = rp_GenReset();
    if (RP_OK != result) {
        RP_ERR("*GEN:RST Failed to reset Red Pitaya generate", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*GEN:RST Successfully");

    return SCPI_RES_OK;
}

scpi_result_t RP_GenState(scpi_t *context) {
    
    int result;
    rp_channel_t channel;
    bool state_c;

    /* Get channel number */
    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*OUTPUT#:STATE Invalid channel number", 
            rp_GetError(result));
        
        return SCPI_RES_ERR;
    }

    /* Parse first, STATE argument */
    if(!SCPI_ParamBool(context, &state_c, true)){
        RP_ERR("*OUTPUT#:STATE Missing first parameter", NULL);
        return SCPI_RES_ERR;
    }

    result = rp_GenOutIsEnabled(channel, &state_c);
    if(result != RP_OK){
        RP_ERR("*OUTPUT#:STATE Failed to enable generate", 
            rp_GetError(result));

        return SCPI_RES_ERR;
    }

    RP_INFO("*OUTPUT#:STATE Successfully enabled generate output.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenStateQ(scpi_t *context){

    bool enabled;
    int result;
    rp_channel_t channel;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*OUTPUT#:STATE Invalid channel number", NULL);
        return SCPI_RES_ERR;
    }

    result = rp_GenOutIsEnabled(channel, &enabled);
    if(result != RP_OK){
        RP_ERR("*OUTPUT#:STATE Failed to get generate state", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, enabled);

    RP_INFO("*OUTPUT#:STATE Successfully returned generate state");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenFrequency(scpi_t *context){

    double frequency;
    rp_channel_t channel;
    int result;

    /* Get channel number */
    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*OUR#:FREQ:FIX Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    /* Parse first, FREQUENCY parameter */
    if(!SCPI_ParamDouble(context, &frequency, true)){
        RP_ERR("*OUR#:FREQ:FIX Missing first parameter", NULL);
        return SCPI_RES_ERR;
    }

    result = rp_GenFreq(channel, frequency);
    if(result != RP_OK){
        RP_ERR("*OUR#:FREQ:FIX Failed to set frequency", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*OUR#:FREQ:FIX Successfully set frequency");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenFrequencyQ(scpi_t *context) {
    
    float frequency;
    rp_channel_t channel;
    int result;

    /* Get channel number */
    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*OUTPUT#:STATE Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetFreq(channel, &frequency);
    if(result != RP_OK){
        RP_ERR("*OUTPUT#:STATE Failed to get frequency", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    /* Return data to client */
    SCPI_ResultDouble(context, frequency);

    RP_INFO("*OUTPUT#:STATE Successfully returned frequency value to client.");
    return SCPI_RES_OK;
}    

scpi_result_t RP_GenWaveForm(scpi_t *context) {
    
    rp_channel_t channel;
    int32_t wave_form;
    int result;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("SOUR#:FUNC Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    /* Read WAVEFORM parameter */
    if(!SCPI_ParamChoice(context, scpi_RpWForm, &wave_form, true)){
        RP_ERR("*SOUR#:FUNC Missing first parameter", NULL);
        return SCPI_RES_ERR;
    }    

    rp_waveform_t wf = wave_form;
    result = rp_GenWaveform(channel, wf);

    if(result != RP_OK){
        RP_ERR("*SOUR#:FUNC Failed to set generate wave form.", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*SOUR#:FUNC Successfully set generate waveform.");
    return SCPI_RES_OK;
}


scpi_result_t RP_GenWaveFormQ(scpi_t *context) {
    
    const char *wf_name; 
    rp_channel_t channel;
    rp_waveform_t wave_form;
    int result;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:FUNC? Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetWaveform(channel, &wave_form);
    int32_t wf = wave_form;

    if(!SCPI_ChoiceToName(scpi_RpWForm, wf, &wf_name)){
        RP_ERR("*SOUR#:FUNC? Failed to get wave form", NULL);
        return SCPI_RES_ERR;
    }

    /* Return result to client */
    SCPI_ResultMnemonic(context, wf_name);

    RP_INFO("*SOUR#:FUNC? Successfully returned generate wave form to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenAmplitude(scpi_t *context) {
    
    rp_channel_t channel;
    float amplitude;
    int result;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:VOLT Invalid channel number", NULL);
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamFloat(context, &amplitude, true)){
        RP_ERR("*SOUR#:VOLT Failed to parse first parameter", NULL);
        return SCPI_RES_ERR;
    }

    result = rp_GenAmp(channel, amplitude);
    if(result != RP_OK){
        RP_ERR("*SOUR#:VOLT Failed to set amplitude", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*SOUR#:VOLT Successfully set amplitude.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenAmplitudeQ(scpi_t *context) {
   
    rp_channel_t channel;
    float amplitude;
    int result;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:VOLT? Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetAmp(channel, &amplitude);
    if(result != RP_OK){
        RP_ERR("*SOUR#:VOLT? Failed to set amplitude", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultDouble(context, amplitude);

    RP_INFO("*SOUR#:VOLT? Successfully returned amplitude value to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenOffset(scpi_t *context) {
    
    rp_channel_t channel;
    float offset;
    int result;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:VOLT:OFFS Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamFloat(context, &offset, true)){
        RP_ERR("*SOUR#:VOLT:OFFS Failed to parse parameter.", NULL);
        return SCPI_RES_ERR;
    }

    result = rp_GenOffset(channel, offset);
    if(result != RP_OK){
        RP_ERR("*SOUR#:VOLT:OFFS Failed to set offset.", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*SOUR#:VOLT:OFFS Successfully set generate offset value.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenOffsetQ(scpi_t *context) {
    
    rp_channel_t channel;
    float offset;
    int result;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:VOLT:OFFS? Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetOffset(channel, &offset);
    if(result != RP_OK){
        RP_ERR("*SOUR#:VOLT:OFFS? Failed to get generate offset", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, offset);
    return SCPI_RES_OK;
}

scpi_result_t RP_GenPhase(scpi_t *context) {
    
    rp_channel_t channel;
    float phase;
    int result;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:PHAS Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamFloat(context, &phase, true)){
        RP_ERR("*SOUR#:PHAS Failed to parse first argument", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenPhase(channel, phase);
    if(result != RP_OK){
        RP_ERR("*SOUR#:PHAS Failed to set generate phase.", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*SOUR#:PHAS Successfully set generate phase");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenPhaseQ(scpi_t *context) {
    
    rp_channel_t channel;
    int result;
    float phase;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:PHAS? Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetPhase(channel, &phase);
    if(result != RP_OK){
        RP_ERR("*SOUR#:PHAS? Failed to get generate phase", rp_GetError(result));
        return SCPI_RES_ERR;
    } 
    SCPI_ResultFloat(context, phase);

    RP_INFO("*SOUR#:PHAS? Successfully returned generate phase value to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenDutyCycle(scpi_t *context) {
    
    rp_channel_t channel;
    float duty_cycle;
    int result;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:DCYC", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamFloat(context, &duty_cycle, true)){
        RP_ERR("*SOUR#:DCYC Failed to parse first argument", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenDutyCycle(channel, duty_cycle);
    if(result != RP_OK){
        RP_ERR("*SOUR#:DCYC Failed to set generate duty cycle", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*SOUR#:DCYC Successfully set generate duty cycle.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenDutyCycleQ(scpi_t *context) {
    
    rp_channel_t channel;
    float duty_cycle;
    int result;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:DCYC? Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetDutyCycle(channel, &duty_cycle);
    if(result != RP_OK){
        RP_ERR("*SOUR#:DCYC? Failed to get generate duty cycle", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, duty_cycle);

    RP_INFO("*SOUR#:DCYC Successfully returned generate duty cycle  value to client");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenArbitraryWaveForm(scpi_t *context) {
    
    rp_channel_t channel;
    float buffer[BUFFER_LENGTH];
    uint32_t size;
    int result;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:TRAC:DATA:DATA Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamBufferFloat(context, buffer, &size, true)){
        RP_ERR("*SOUR#:TRAC:DATA:DATA Failed to arbitrary waveform data parameter.", NULL);
        return SCPI_RES_ERR;
    }

    result = rp_GenArbWaveform(channel, buffer, size);
    if(result != RP_OK){
        RP_ERR("*SOUR#:TRAC:DATA:DATA Failed to set arbitrary waveform data.", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*SOUR#:TRAC:DATA:DATA Successfully set arbitrary waveform data.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenArbitraryWaveFormQ(scpi_t *context) {
    
    rp_channel_t channel;
    float buffer[BUFFER_LENGTH];
    uint32_t size;
    int result;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:TRAC:DATA:DATA? Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetArbWaveform(channel, buffer, &size);
    if(result != RP_OK){
        RP_ERR("*SOUR#:TRAC:DATA:DATA? Failed to get arbitrary waveform data", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultBufferFloat(context, buffer, size);

    RP_INFO("*SOUR#:TRAC:DATA:DATA? Successfully returned arbitrary waveform data to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenGenerateMode(scpi_t *context) {
    
    rp_channel_t channel;
    int32_t usr_mode;
    int result;
    
    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:BURS:STAT Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamChoice(context, scpi_RpGenMode, &usr_mode, true)){
        RP_ERR("*SOUR#:BURS:STAT Failed to parse first parameter", NULL);
        return SCPI_RES_ERR;
    }

    rp_gen_mode_t mode = usr_mode;
    result = rp_GenMode(channel, mode);
    if(result != RP_OK){
        RP_ERR("*SOUR#:BURS:STAT Failed to get generate mode", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*SOUR#:BURS:STAT Successfully set generate mode.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenGenerateModeQ(scpi_t *context) {
    
    rp_channel_t channel;
    int result;
    const char *gen_mode;
    rp_gen_mode_t mode;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:BURS:STAT? Invalid channel name", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetMode(channel, &mode);
    if(result != RP_OK){
        RP_ERR("*SOUR#:BURS:STAT? Failed to get generate mode", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    int32_t i_mode = mode;

    if(!SCPI_ChoiceToName(scpi_RpGenMode, i_mode, &gen_mode)){
        RP_ERR("*SOUR#:BURS:STAT? Invalid genera*SOUR#:BURS:STAT?te mode", NULL);
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, gen_mode);

    RP_INFO("*SOUR#:BURS:STAT? Successfully returned generate mode status to client.");
    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenChannel1BurstCount(scpi_t *context) {
    return RP_GenSetBurstCount(RP_CH_1, context);
}

enum _scpi_result_t RP_GenChannel2BurstCount(scpi_t *context) {
    return RP_GenSetBurstCount(RP_CH_2, context);
}

enum _scpi_result_t RP_GenChannel1BurstCountQ(scpi_t *context) {
    return RP_GenGetBurstCount(RP_CH_1, context);
}

enum _scpi_result_t RP_GenChannel2BurstCountQ(scpi_t *context) {
    return RP_GenGetBurstCount(RP_CH_2, context);
}

scpi_result_t RP_GenChannel1BurstRepetitions(scpi_t *context) {
    return RP_GenSetBurstRepetitions(RP_CH_1, context);
}

scpi_result_t RP_GenChannel2BurstRepetitions(scpi_t *context) {
    return RP_GenSetBurstRepetitions(RP_CH_2, context);
}

scpi_result_t RP_GenChannel1BurstRepetitionsQ(scpi_t *context) {
    return RP_GenGetBurstRepetitions(RP_CH_1, context);
}

scpi_result_t RP_GenChannel2BurstRepetitionsQ(scpi_t *context) {
    return RP_GenGetBurstRepetitions(RP_CH_2, context);
}

scpi_result_t RP_GenChannel1BurstPeriod(scpi_t *context) {
    return RP_GenSetBurstPeriod(RP_CH_1, context);
}

scpi_result_t RP_GenChannel2BurstPeriod(scpi_t *context) {
    return RP_GenSetBurstPeriod(RP_CH_2, context);
}

scpi_result_t RP_GenChannel1BurstPeriodQ(scpi_t *context) {
    return RP_GenGetBurstPeriod(RP_CH_1, context);
}

scpi_result_t RP_GenChannel2BurstPeriodQ(scpi_t *context) {
    return RP_GenGetBurstPeriod(RP_CH_2, context);
}

enum _scpi_result_t RP_GenChannel1TriggerSource(scpi_t *context) {
    return RP_GenSetTriggerSource(RP_CH_1, context);
}

enum _scpi_result_t RP_GenChannel2TriggerSource(scpi_t *context) {
    return RP_GenSetTriggerSource(RP_CH_2, context);
}

enum _scpi_result_t RP_GenChannel1TriggerSourceQ(scpi_t *context) {
    return RP_GenGetTriggerSource(RP_CH_1, context);
}

enum _scpi_result_t RP_GenChannel2TriggerSourceQ(scpi_t *context) {
    return RP_GenGetTriggerSource(RP_CH_2, context);
}

enum _scpi_result_t RP_GenChannel1Trigger(scpi_t *context) {
    return RP_GenSetTrigger(1, context);
}

enum _scpi_result_t RP_GenChannel2Trigger(scpi_t *context) {
    return RP_GenSetTrigger(2, context);
}

enum _scpi_result_t RP_GenChannelAllTrigger(scpi_t *context) {
    return RP_GenSetTrigger(3, context);
}

enum _scpi_result_t RP_GenSetOffset(rp_channel_t channel, scpi_t *context) {
    double value;
    // read first parameter OFFSET (value in V)
    if (!SCPI_ParamDouble(context, &value, true)) {
        syslog(LOG_ERR, "*SOUR<n>:VOLT:OFFS is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenOffset(channel, (float) value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:VOLT:OFFS Failed to set offset: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:VOLT:OFFS Successfully set offset to %.2f V.", value);

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenGetOffset(rp_channel_t channel, scpi_t *context) {
    float value;
    int result = rp_GenGetOffset(channel, &value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:VOLT:OFFS? Failed to get offset: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultDouble(context, value);

    syslog(LOG_INFO, "*SOUR<n>:VOLT:OFFS? Successfully returned offset %.2fV to client.", value);

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetPhase(rp_channel_t channel, scpi_t *context) {
    double value;
    // read first parameter PHASE (value in degrees)
    if (!SCPI_ParamDouble(context, &value, true)) {
        syslog(LOG_ERR, "*SOUR<n>:PHAS is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenPhase(channel, (float) value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:PHAS Failed to set phase: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:PHAS Successfully set phase to %.2f deg.", value);

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenGetPhase(rp_channel_t channel, scpi_t *context) {
    float value;
    int result = rp_GenGetPhase(channel, &value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:PHAS? Failed to get phase: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultDouble(context, value);

    syslog(LOG_INFO, "*SOUR<n>:PHAS? Successfully returned phase %.2fdeg to client.", value);

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetDutyCycle(rp_channel_t channel, scpi_t *context) {
    double value;
    // read first parameter DUTY CYCLE (value in percentage)
    if (!SCPI_ParamDouble(context, &value, true)) {
        syslog(LOG_ERR, "*SOUR<n>:DCYC is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenDutyCycle(channel, (float) (value/100));

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:DCYC Failed to set duty cycle: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:DCYC Successfully set duty cycle to %.2f.", value);

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenGetDutyCycle(rp_channel_t channel, scpi_t *context) {
    float value;
    int result = rp_GenGetDutyCycle(channel, &value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:DCYC? Failed to get duty cycle: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultDouble(context, value*100);

    syslog(LOG_INFO, "*SOUR<n>:DCYC? Successfully returned duty cycle %.2f to client.", value);

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetArbitraryWaveForm(rp_channel_t channel, scpi_t *context) {
    float buffer[BUFFER_LENGTH];
    uint32_t size;
    // read first parameter ARBITRARY WAVEFORM (float array form -1 to 1)
    if (!SCPI_ParamBufferFloat(context, buffer, &size, true)) {
        syslog(LOG_ERR, "*SOUR<n>:TRAC:DATA:DATA is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenArbWaveform(channel, buffer, size);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:TRAC:DATA:DATA Failed to set arbitrary waveform: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:TRAC:DATA:DATA Successfully set arbitrary waveform");

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenGetArbitraryWaveForm(rp_channel_t channel, scpi_t *context) {
    uint32_t length;
    float data[BUFFER_LENGTH];
    int result = rp_GenGetArbWaveform(channel, data, &length);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:TRAC:DATA:DATA? Failed to get arbitrary waveform: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultBufferFloat(context, data, length);
    syslog(LOG_INFO, "*SOUR<n>:TRAC:DATA:DATA? Successfully returned arbitrary wave form to client.");

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetGenerateMode(rp_channel_t channel, scpi_t *context) {
    bool burst;
    // read first parameter BURST MODE (ON, OFF)
    if (!SCPI_ParamBool(context, &burst, true)) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:STAT is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result;
    if (burst) {
        result = rp_GenMode(channel, RP_GEN_MODE_BURST);
    }
    else {
        result = rp_GenMode(channel, RP_GEN_MODE_CONTINUOUS);
    }

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:STAT Failed to set generate mode: %s",rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:BURS:STAT Successfully set generate mode");
    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenGetGenerateMode(rp_channel_t channel, scpi_t *context) {
    
    char value;
    rp_gen_mode_t mode;
    int result = rp_GenGetMode(channel, &mode);
    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:STAT? Failed to get generate mode: %s",rp_GetError(result));
        return SCPI_RES_ERR;
    }

    if(getRpStateIntegerString(mode, &value)){
        syslog(LOG_ERR, "*SOUR<n>:BURS:STAT? Invalid mode.");
        return SCPI_RES_ERR;
    }

    /* Return generate mode status back to the client */
    SCPI_ResultMnemonic(context, &value);
    syslog(LOG_INFO, "*SOUR<n>:BURS:STAT? Successfully returned %s status to client.", &value);

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetBurstCount(rp_channel_t channel, scpi_t *context) {
    const char * param;
    size_t param_len;
    char string[15];

    // read first parameter NUMBER OF CYCLES (integer value)
    if (!SCPI_ParamCharacters(context, &param, &param_len, true)) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:NCYC is missing first parameter.");
        return SCPI_RES_ERR;
    }
    strncpy(string, param, param_len);
    string[param_len] = '\0';

    // Convert String to int

    int32_t value;
    if (getRpInfinityInteger(string, &value)) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:NCYC parameter cycles is invalid.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenBurstCount(channel, value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:NCYC Failed to set burst count: %s",rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:BURS:NCYC Successfully set burst count");

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenGetBurstCount(rp_channel_t channel, scpi_t *context) {
    int32_t value;
    int result = rp_GenGetBurstCount(channel, &value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:NCYC? Failed to get burst count: %s",rp_GetError(result));
        return SCPI_RES_ERR;
    }

    char string[50];
    if (getRpInfinityIntegerString(value, string)) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:NCYC? failed to convert to string.");
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, string);

    syslog(LOG_INFO, "*SOUR<n>:BURS:NCYC? Successfully returned burst count %s to client.", &string[0]);
    return SCPI_RES_OK;
}


scpi_result_t RP_GenSetBurstRepetitions(rp_channel_t channel, scpi_t *context) {
    const char * param;
    size_t param_len;
    char string[15];

    // read first parameter NUMBER OF REPETITIONS (integer value)
    if (!SCPI_ParamCharacters(context, &param, &param_len, true)) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:NOR is missing first parameter.");
        return SCPI_RES_ERR;
    }
    strncpy(string, param, param_len);
    string[param_len] = '\0';

    // Convert String to int
    int32_t value;
    if (getRpInfinityInteger(string, &value)) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:NOR parameter cycles is invalid.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenBurstRepetitions(channel, value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:NOR Failed to set burst repetitions: %s",rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:BURS:NOR Successfully set burst repetitions");

    return SCPI_RES_OK;
}

scpi_result_t RP_GenGetBurstRepetitions(rp_channel_t channel, scpi_t *context) {
    int32_t value;
    int result = rp_GenGetBurstRepetitions(channel, &value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:NOR? Failed to get burst repetitions: %s",rp_GetError(result));
        return SCPI_RES_ERR;
    }

    char string[50];
    if (getRpInfinityIntegerString(value, string)) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:NOR? failed to converto to string.");
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, string);

    syslog(LOG_INFO, "*SOUR<n>:BURS:NOR? Successfully returned burst repetitions %s to client.", &string[0]);
    return SCPI_RES_OK;
}

scpi_result_t RP_GenSetBurstPeriod(rp_channel_t channel, scpi_t *context) {
    uint32_t value;

    // read first parameter PERIOD TIME (unsigned integer value)
    if (!SCPI_ParamUInt32(context, &value, true)) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:INT:PER is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenBurstPeriod(channel, value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:INT:PER Failed to set burst period: %s",rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:BURS:INT:PER Successfully set burst period");

    return SCPI_RES_OK;
}

scpi_result_t RP_GenGetBurstPeriod(rp_channel_t channel, scpi_t *context) {
    uint32_t value;
    int result = rp_GenGetBurstPeriod(channel, &value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:INT:PER? Failed to get burst period: %s",rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);

    syslog(LOG_INFO, "*SOUR<n>:BURS:INT:PER? Successfully returned burst period %d to client.",  value);

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetTriggerSource(rp_channel_t channel, scpi_t *context) {
    const char * param;
    size_t param_len;
    char triggerSourceString[15];

    // read first parameter TRIGGER SOURCE (EXT_PE, EXT_NE, INT, GATED)
    if (!SCPI_ParamCharacters(context, &param, &param_len, true)) {
        syslog(LOG_ERR, "*SOUR<n>:TRIG:SOUR is missing first parameter.");
        return SCPI_RES_ERR;
    }
    strncpy(triggerSourceString, param, param_len);
    triggerSourceString[param_len] = '\0';

    // Convert triggerSource to rp_trig_src_t
    rp_trig_src_t triggerSource;
    if (getRpGenTriggerSource(triggerSourceString, &triggerSource)) {
        syslog(LOG_ERR, "*SOUR<n>:TRIG:SOUR parameter triggerSource is invalid.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenTriggerSource(channel, triggerSource);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:TRIG:SOUR Failed to set trigger source: %s",rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:TRIG:SOUR Successfully set trigger source");

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenGetTriggerSource(rp_channel_t channel, scpi_t *context) {
    rp_trig_src_t triggerSource;
    int result = rp_GenGetTriggerSource(channel, &triggerSource);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:TRIG:SOUR? Failed to get trigger source: %s",rp_GetError(result));
        return SCPI_RES_ERR;
    }

    char string[50];
    if (getRpGenTriggerSourceString(triggerSource, string)) {
        syslog(LOG_ERR, "*SOUR<n>:TRIG:SOUR? failed to convert to string.");
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, string);

    syslog(LOG_INFO, "*SOUR<n>:TRIG:SOUR? Successfully returned trigger source to client.");

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetTrigger(int channel, scpi_t *context) {
    int result = rp_GenTrigger(channel);

    if (RP_OK != result) {
        syslog(LOG_ERR, "%s Failed to set triggert: %s", channel==3 ? "TRIG:IMM" : "SOUR<n>:TRIG:IMM", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*%s Successfully set trigger", channel==3 ? "TRIG:IMM" : "SOUR<n>:TRIG:IMM");

    return SCPI_RES_OK;
}
