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
#include <math.h>

#include "generate.h"
#include "../../api/rpbase/src/generate.h"

#include "common.h"
#include "scpi/parser.h"
#include "scpi/units.h"

/* These structures are a direct API mirror 
and should not be altered! */
const scpi_choice_def_t scpi_RpWForm[] = {
    {"SINE",        0},
    {"SQUARE",      1},
    {"TRIANGLE",    2},
    {"SAWU",        3},
    {"SAWD",        4},
    {"PWM",         5},
    {"DC",          6},
    {"ARBITRARY",   7},
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
    {"CONTINUOUS",  0},
    {"BURST",       1},
    {"STREAM",      2},
    SCPI_CHOICE_LIST_END
};

scpi_result_t RP_GenReset(scpi_t *context) {
    int result = rp_GenReset();
    if (RP_OK != result) {
        RP_LOG(LOG_ERR, "*GEN:RST Failed to reset Red "
            "Pitaya generate: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*GEN:RST Successfully reset Red "
        "Pitaya generate module.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenState(scpi_t *context) {
    
    int result;
    rp_channel_t channel;
    bool state_c;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    /* Parse first, STATE argument */
    if(!SCPI_ParamBool(context, &state_c, true)){
        RP_LOG(LOG_ERR, "*OUTPUT#:STATE Missing first parameter.\n");
        return SCPI_RES_ERR;
    }

    state_c ? (result = rp_GenOutEnable(channel)) :
        (result = rp_GenOutDisable(channel));

    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*OUTPUT#:STATE Failed to enable generate: %s\n", 
            rp_GetError(result));

        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*OUTPUT#:STATE Successfully enabled generate output.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenStateQ(scpi_t *context){

    bool enabled;
    int result;
    rp_channel_t channel;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenOutIsEnabled(channel, &enabled);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*OUTPUT#:STATE Failed to get generate "
            "state: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, enabled);

    RP_LOG(LOG_INFO, "*OUTPUT#:STATE Successfully returned generate state.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenFrequency(scpi_t *context){

    scpi_number_t frequency;
    rp_channel_t channel;
    int result;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    /* Parse first, FREQUENCY parameter */
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &frequency, true)) {
        RP_LOG(LOG_ERR, "*OUR#:FREQ:FIX Missing first parameter.\n");
        return SCPI_RES_ERR;
    }

    result = rp_GenFreq(channel, frequency.value);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*OUR#:FREQ:FIX Failed to set frequency: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*OUR#:FREQ:FIX Successfully set frequency.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenFrequencyQ(scpi_t *context) {
    
    float frequency;
    rp_channel_t channel;
    int result;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetFreq(channel, &frequency);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*OUTPUT#:STATE Failed to get frequency: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    /* Return data to client */
    SCPI_ResultDouble(context, frequency);

    RP_LOG(LOG_INFO, "*OUTPUT#:STATE Successfully returned frequency value to client.\n");
    return SCPI_RES_OK;
}    

scpi_result_t RP_GenWaveForm(scpi_t *context) {
    
    rp_channel_t channel;
    int32_t wave_form;
    int result;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    /* Read WAVEFORM parameter */
    if(!SCPI_ParamChoice(context, scpi_RpWForm, &wave_form, true)){
        RP_LOG(LOG_ERR, "*SOUR#:FUNC Missing first parameter.\n");
        return SCPI_RES_ERR;
    }    

    rp_waveform_t wf = wave_form;
    result = rp_GenWaveform(channel, wf);

    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:FUNC Failed to set generate wave form: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*SOUR#:FUNC Successfully set generate waveform.\n");
    return SCPI_RES_OK;
}


scpi_result_t RP_GenWaveFormQ(scpi_t *context) {
    
    const char *wf_name; 
    rp_channel_t channel;
    rp_waveform_t wave_form;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if (rp_GenGetWaveform(channel, &wave_form) != RP_OK){
        return SCPI_RES_ERR;
    }
    int32_t wf = wave_form;

    if(!SCPI_ChoiceToName(scpi_RpWForm, wf, &wf_name)){
        RP_LOG(LOG_ERR, "*SOUR#:FUNC? Failed to get wave form.\n");
        return SCPI_RES_ERR;
    }

    /* Return result to client */
    SCPI_ResultMnemonic(context, wf_name);

    RP_LOG(LOG_INFO, "*SOUR#:FUNC? Successfully returned generate wave form to client.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenAmplitude(scpi_t *context) {
    
    rp_channel_t channel;
    scpi_number_t amplitude;
    int result;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &amplitude, true)) {
        RP_LOG(LOG_ERR, "*SOUR#:VOLT Failed to parse first parameter.\n");
        return SCPI_RES_ERR;
    }

    result = rp_GenAmp(channel, amplitude.value);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:VOLT Failed to set amplitude: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*SOUR#:VOLT Successfully set amplitude.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenAmplitudeQ(scpi_t *context) {
   
    rp_channel_t channel;
    float amplitude;
    int result;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetAmp(channel, &amplitude);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:VOLT? Failed to set amplitude: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultDouble(context, amplitude);

    RP_LOG(LOG_INFO, "*SOUR#:VOLT? Successfully returned amplitude value to client.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenOffset(scpi_t *context) {
    
    rp_channel_t channel;
    scpi_number_t offset;
    int result;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &offset, true)) {
        RP_LOG(LOG_ERR, "*SOUR#:VOLT:OFFS Failed to parse parameter.\n");
        return SCPI_RES_ERR;
    }

    result = rp_GenOffset(channel, offset.value);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:VOLT:OFFS Failed to set offset: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*SOUR#:VOLT:OFFS Successfully set generate offset value.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenOffsetQ(scpi_t *context) {
    
    rp_channel_t channel;
    float offset;
    int result;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetOffset(channel, &offset);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:VOLT:OFFS? Failed to get "
            "generate offset: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, offset);

    RP_LOG(LOG_INFO, "*SOUR#:VOLT:OFFS? Successfully returned offset to the client.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenPhase(scpi_t *context) {
    
    rp_channel_t channel;
    scpi_number_t phase;
    int result;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &phase, true)) {
        RP_LOG(LOG_ERR, "*SOUR#:PHAS Failed to parse first argument.\n");
        return SCPI_RES_ERR;
    }

    result = rp_GenPhase(channel, phase.value/(2*M_PI)*360);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:PHAS Failed to set generate "
            "phase: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*SOUR#:PHAS Successfully set generate phase.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenPhaseQ(scpi_t *context) {
    
    rp_channel_t channel;
    int result;
    float phase;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetPhase(channel, &phase);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:PHAS? Failed to get "
            "generate phase: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    } 
    SCPI_ResultFloat(context, phase);

    RP_LOG(LOG_INFO, "*SOUR#:PHAS? Successfully returned "
        "generate phase value to client.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenDutyCycle(scpi_t *context) {
    
    rp_channel_t channel;
    float duty_cycle;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamFloat(context, &duty_cycle, true)){
        RP_LOG(LOG_ERR, "*SOUR#:DCYC Failed to parse first argument\n");
        return SCPI_RES_ERR;
    }

    if (rp_GenDutyCycle(channel, duty_cycle) != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:DCYC Failed to set generate duty cycle\n");
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*SOUR#:DCYC Successfully set generate duty cycle.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenDutyCycleQ(scpi_t *context) {
    
    rp_channel_t channel;
    float duty_cycle;
    int result;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetDutyCycle(channel, &duty_cycle);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:DCYC? Failed to get "
            "generate duty cycle: %s\n", rp_GetError(result));

        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, duty_cycle);

    RP_LOG(LOG_INFO, "*SOUR#:DCYC Successfully "
        "returned generate duty cycle value to client.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenArbitraryWaveForm(scpi_t *context) {
    
    rp_channel_t channel;
    float buffer[BUFFER_LENGTH];
    uint32_t size;
    int result;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamBufferFloat(context, buffer, &size, true)){
        RP_LOG(LOG_ERR, "*SOUR#:TRAC:DATA:DATA Failed to "
            "arbitrary waveform data parameter.\n");
        return SCPI_RES_ERR;
    }

    result = rp_GenArbWaveform(channel, buffer, size);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:TRAC:DATA:DATA Failed to "
            "set arbitrary waveform data: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*SOUR#:TRAC:DATA:DATA Successfully set arbitrary waveform data.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenArbitraryWaveFormQ(scpi_t *context) {
    
    rp_channel_t channel;
    float buffer[BUFFER_LENGTH];
    uint32_t size;
    int result;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetArbWaveform(channel, buffer, &size);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:TRAC:DATA:DATA? Failed to "
            "get arbitrary waveform data: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultBufferFloat(context, buffer, size);

    RP_LOG(LOG_INFO, "*SOUR#:TRAC:DATA:DATA? Successfully "
        "returned arbitrary waveform data to client.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenGenerateMode(scpi_t *context) {
    
    rp_channel_t channel;
    int32_t usr_mode;
    int result;
    
    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamChoice(context, scpi_RpGenMode, &usr_mode, true)){
        RP_LOG(LOG_ERR, "*SOUR#:BURS:STAT Failed to parse first parameter.\n");
        return SCPI_RES_ERR;
    }

    rp_gen_mode_t mode = usr_mode;
    result = rp_GenMode(channel, mode);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:BURS:STAT Failed to get generate "
            "mode: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*SOUR#:BURS:STAT Successfully set generate mode.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenGenerateModeQ(scpi_t *context) {
    
    rp_channel_t channel;
    int result;
    const char *gen_mode;
    rp_gen_mode_t mode;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetMode(channel, &mode);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:BURS:STAT? Failed to get generate "
            "mode: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    int32_t i_mode = mode;

    if(!SCPI_ChoiceToName(scpi_RpGenMode, i_mode, &gen_mode)){
        RP_LOG(LOG_ERR, "*SOUR#:BURS:STAT? Invalid generate mode.\n");
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, gen_mode);

    RP_LOG(LOG_INFO, "*SOUR#:BURS:STAT? Successfully returned "
        "generate mode status to client.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstCount(scpi_t *context) {
    
    rp_channel_t channel;
    int result, count;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamInt32(context, &count, true)){
        RP_LOG(LOG_ERR, "*SOUR#:BURS:STAT Failed to parse "
            "first parameter.\n");
        return SCPI_RES_ERR;
    }

    result = rp_GenBurstCount(channel, count);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:BURS:STAT Failed to set "
            "count parameter: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*SOUR#:BURS:STAT Successfully set generate burst count.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstCountQ(scpi_t *context) {

    rp_channel_t channel;
    int result, count;
    
    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetBurstCount(channel, &count);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:BURS:STAT? Failed to get generate "
            "burst count: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultInt32(context, count);

    RP_LOG(LOG_INFO, "*SOUR#:BURS:STAT? Successfully returned generate "
        "burst count value to client.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstRepetitions(scpi_t *context) {
    
    rp_channel_t channel;
    int result, repetitions;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamInt32(context, &repetitions, true)){
        RP_LOG(LOG_ERR, "*SOUR#:BURS:NOR Failed to parse "
            "first parameter.\n");
        return SCPI_RES_ERR;
    }

    result = rp_GenBurstRepetitions(channel, repetitions);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:BURS:NOR Failed to set "
            "generate burst repetitions: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*SOUR#:BURS:NOR Successfully set generate repetitions.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstRepetitionsQ(scpi_t *context) {
    
    rp_channel_t channel;
    int result, repetitions;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetBurstRepetitions(channel, &repetitions);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:BURS:NOR Failed to get "
            "generate repetitions: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultInt32(context, repetitions);

    RP_LOG(LOG_INFO, "*SOUR#:BURS:NOR Successfully returned "
        "generate repetitions value to client.\n");

    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstPeriod(scpi_t *context) {
    
    rp_channel_t channel;
    int result;
    uint32_t period;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamUInt32(context, &period, true)){
        RP_LOG(LOG_ERR,"*SOUR#:BURS:INT:PER Failed to "
            "parse first parameter.\n");
        return SCPI_RES_ERR;
    }

    result = rp_GenBurstPeriod(channel, period);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:BURS:INT:PER Failed to get "
            "generate burst period: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*SOUR#:BURS:INT:PER Successfully set generate burst period.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstPeriodQ(scpi_t *context) {
    
    rp_channel_t channel;
    int result;
    uint32_t period;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetBurstPeriod(channel, &period);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:BURS:INT:PER Failed to get "
            "generate burst period: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, period, 10);

    RP_LOG(LOG_INFO, "*SOUR#:BURS:INT:PER Successfully returned "
        "generate burst period value to client.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenTriggerSource(scpi_t *context) {
        
    rp_channel_t channel;
    int32_t trig_choice;
    int result;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamChoice(context, scpi_RpGenTrig, &trig_choice, true)){
        RP_LOG(LOG_ERR, "*SOUR#:TRIG:SOUR Failed to parse first parameter.\n");
        return SCPI_RES_ERR;
    }

    rp_trig_src_t trig_src = trig_choice;
    result = rp_GenTriggerSource(channel, trig_src);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:TRIG:SOUR Failed to set generate"
        " trigger source: %s\n", rp_GetError(result));

        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*SOUR#:TRIG:SOUR Successfully set generate trigger source.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenTriggerSourceQ(scpi_t *context) {
    
    rp_channel_t channel;
    const char *trig_name;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    rp_trig_src_t trig_src;
    if (rp_GenGetTriggerSource(channel, &trig_src) != RP_OK){
        return SCPI_RES_ERR;
    }

    int32_t trig_n = trig_src;

    if(!SCPI_ChoiceToName(scpi_RpGenTrig, trig_n, &trig_name)){
        RP_LOG(LOG_ERR, "*SOUR#:TRIG:SOUR? Failed to parse trigger name.\n");
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, trig_name);

    RP_LOG(LOG_INFO, "*SOUR#:TRIG:SOUR? Successfully returend"
    " generate trigger status to client.\n");

    return SCPI_RES_OK;
}

scpi_result_t RP_GenTrigger(scpi_t *context) {
    
    rp_channel_t channel;
    int result;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenTrigger(channel);
    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*SOUR#:TRIG:IMM Failed to set immediate "
            "trigger: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*SOUR#:TRIG:IMM Successfully set immediate trigger.\n");
    return SCPI_RES_OK;
}
