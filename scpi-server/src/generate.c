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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "generate.h"
#include "../../api/rpbase/src/generate.h"

#include "common.h"
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

const scpi_choice_def_t scpi_RpGenTrig[] = {
    {"INT",     1},
    {"EXT_PE",  2},
    {"EXT_NE",  3},
    {"GATED",   4},
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

scpi_result_t RP_GenBurstCount(scpi_t *context) {
    
    rp_channel_t channel;
    int result, count;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:BURS:STAT Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamInt32(context, &count, true)){
        RP_ERR("*SOUR#:BURS:STAT Failed to parse first parameter", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenBurstCount(channel, count);
    if(result != RP_OK){
        RP_ERR("*SOUR#:BURS:STAT Failed to set count parameter", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*SOUR#:BURS:STAT Successfully set generate burst count.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstCountQ(scpi_t *context) {

    rp_channel_t channel;
    int result, count;
    
    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:BURS:STAT? Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetBurstCount(channel, &count);
    if(result != RP_OK){
        RP_ERR("*SOUR#:BURS:STAT? Failed to get generate burst count.", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultInt32(context, count);

    RP_INFO("*SOUR#:BURS:STAT? Successfully returned generate burst count value to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstRepetitions(scpi_t *context) {
    
    rp_channel_t channel;
    int result, repetitions;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:BURS:NOR Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamInt32(context, &repetitions, true)){
        RP_ERR("*SOUR#:BURS:NOR Failed to parse first parameter", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenBurstRepetitions(channel, repetitions);
    if(result != RP_OK){
        RP_ERR("*SOUR#:BURS:NOR Failed to set generate burst repetitions", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*SOUR#:BURS:NOR Successfully set generate repetitions");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstRepetitionsQ(scpi_t *context) {
    
    rp_channel_t channel;
    int result, repetitions;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:BURS:NOR? Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetBurstRepetitions(channel, &repetitions);
    if(result != RP_OK){
        RP_ERR("*SOUR#:BURS:NOR Failed to get generate repetitions", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultInt32(context, repetitions);

    RP_INFO("*SOUR#:BURS:NOR Successfully returned generate repetitions value to client");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstPeriod(scpi_t *context) {
    
    rp_channel_t channel;
    int result;
    uint32_t period;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:BURS:INT:PER Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamUInt32(context, &period, true)){
        RP_ERR("*SOUR#:BURS:INT:PER Failed to parse first parameter", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenBurstPeriod(channel, period);
    if(result != RP_OK){
        RP_ERR("*SOUR#:BURS:INT:PER Failed to get generate burst period", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*SOUR#:BURS:INT:PER Successfully set generate burst period.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstPeriodQ(scpi_t *context) {
    
    rp_channel_t channel;
    int result;
    uint32_t period;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:BURS:INT:PER Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetBurstPeriod(channel, &period);
    if(result != RP_OK){
        RP_ERR("*SOUR#:BURS:INT:PER Failed to get generate burst period", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, period, 10);

    RP_INFO("*SOUR#:BURS:INT:PER Successfully returned generate burst period value to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenTriggerSource(scpi_t *context) {
        
    rp_channel_t channel;
    int32_t trig_choice;
    int result;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:TRIG:SOUR Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamChoice(context, scpi_RpGenTrig, &trig_choice, true)){
        RP_ERR("*SOUR#:TRIG:SOUR Failed to parse first parameter", NULL);
        return SCPI_RES_ERR;
    }

    rp_trig_src_t trig_src = trig_choice;
    result = rp_GenTriggerSource(channel, trig_src);
    if(result != RP_OK){
        RP_ERR("*SOUR#:TRIG:SOUR Failed to set generate"
        " trigger source", rp_GetError(result));

        return SCPI_RES_ERR;
    }

    RP_INFO("*SOUR#:TRIG:SOUR Successfully set generate trigger source.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenTriggerSourceQ(scpi_t *context) {
    
    rp_channel_t channel;
    int result;
    const char *trig_name;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:TRIG:SOUR? Invalid channel number", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    rp_trig_src_t trig_src;
    result = rp_GenGetTriggerSource(channel, &trig_src);

    int32_t trig_n = trig_src;

    if(!SCPI_ChoiceToName(scpi_RpGenTrig, trig_n, &trig_name)){
        RP_ERR("*SOUR#:TRIG:SOUR? Failed to parse trigger name", NULL);
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, trig_name);

    RP_INFO("*SOUR#:TRIG:SOUR? Successfully returend generate trigger"
        "status to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenTrigger(scpi_t *context) {
    
    rp_channel_t channel;
    int result;

    result = RP_ParseChArgv(context, &channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:TRIG:IMM Invalid channel number", NULL);
        return SCPI_RES_ERR;
    }

    result = rp_GenTrigger(channel);
    if(result != RP_OK){
        RP_ERR("*SOUR#:TRIG:IMM Failed to set immediate trigger", NULL);
        return SCPI_RES_ERR;
    }

    RP_INFO("*SOUR#:TRIG:IMM Successfully set immediate trigger")
    return SCPI_RES_OK;
}