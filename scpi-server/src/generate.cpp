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
//#include "../../api/src/generate.h"

#include "rp.h"
#include "rp_hw-profiles.h"
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
    {"DC",          5},
    {"PWM",         6},
    {"ARBITRARY",   7},
    {"DC_NEG",      8},
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
        RP_LOG(context,LOG_ERR, "*GEN:RST Failed to reset Red "
            "Pitaya generate: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*GEN:RST Successfully reset Red "
        "Pitaya generate module.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenSync(scpi_t *context) {
    int result = rp_GenSynchronise();
    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*GEN:SYNC Failed to sync Red "
            "Pitaya generate: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*GEN:SYNC Successfully sync Red "
        "Pitaya generate module.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenSyncState(scpi_t *context) {

    int result;
    bool state_c;

    /* Parse first, STATE argument */
    if(!SCPI_ParamBool(context, &state_c, true)){
        RP_LOG(context,LOG_ERR, "*OUTPUT:STATE Missing first parameter.");
        return SCPI_RES_ERR;
    }

    result = rp_GenOutEnableSync(state_c);

    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*OUTPUT:STATE Failed to enable generate: %s",
            rp_GetError(result));

        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*OUTPUT:STATE Successfully enabled generate output.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenState(scpi_t *context) {

    int result;
    rp_channel_t channel;
    bool state_c;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    /* Parse first, STATE argument */
    if(!SCPI_ParamBool(context, &state_c, true)){
        RP_LOG(context,LOG_ERR, "*OUTPUT#:STATE Missing first parameter.");
        return SCPI_RES_ERR;
    }
    if (state_c){
        result = rp_GenOutEnable(channel);
    }else{
        result = rp_GenOutDisable(channel);
    }

    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*OUTPUT#:STATE Failed to enable generate: %s",
            rp_GetError(result));

        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*OUTPUT#:STATE Successfully enabled generate output.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenStateQ(scpi_t *context){

    bool enabled;
    int result;
    rp_channel_t channel;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenOutIsEnabled(channel, &enabled);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*OUTPUT#:STATE Failed to get generate "
            "state: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, enabled);

    RP_LOG(context,LOG_INFO, "*OUTPUT#:STATE Successfully returned generate state.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenFrequency(scpi_t *context){

    scpi_number_t frequency;
    rp_channel_t channel;
    int result;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    /* Parse first, FREQUENCY parameter */
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &frequency, true)) {
        RP_LOG(context,LOG_ERR, "*SOUR#:FREQ:FIX Missing first parameter.");
        return SCPI_RES_ERR;
    }

    result = rp_GenFreq(channel, frequency.value);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:FREQ:FIX Failed to set frequency: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SOUR#:FREQ:FIX Successfully set frequency.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenFrequencyQ(scpi_t *context) {

    float frequency;
    rp_channel_t channel;
    int result;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetFreq(channel, &frequency);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*OUTPUT#:STATE Failed to get frequency: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    /* Return data to client */
    SCPI_ResultDouble(context, frequency);

    RP_LOG(context,LOG_INFO, "*OUTPUT#:STATE Successfully returned frequency value to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenWaveForm(scpi_t *context) {

    rp_channel_t channel;
    int32_t wave_form;
    int result;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    /* Read WAVEFORM parameter */
    if(!SCPI_ParamChoice(context, scpi_RpWForm, &wave_form, true)){
        RP_LOG(context,LOG_ERR, "*SOUR#:FUNC Missing first parameter.");
        return SCPI_RES_ERR;
    }

    rp_waveform_t wf = (rp_waveform_t)wave_form;
    result = rp_GenWaveform(channel, wf);

    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:FUNC Failed to set generate wave form: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SOUR#:FUNC Successfully set generate waveform.");
    return SCPI_RES_OK;
}


scpi_result_t RP_GenWaveFormQ(scpi_t *context) {

    const char *wf_name;
    rp_channel_t channel;
    rp_waveform_t wave_form;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if (rp_GenGetWaveform(channel, &wave_form) != RP_OK){
        return SCPI_RES_ERR;
    }
    int32_t wf = wave_form;

    if(!SCPI_ChoiceToName(scpi_RpWForm, wf, &wf_name)){
        RP_LOG(context,LOG_ERR, "*SOUR#:FUNC? Failed to get wave form.");
        return SCPI_RES_ERR;
    }

    /* Return result to client */
    SCPI_ResultMnemonic(context, wf_name);

    RP_LOG(context,LOG_INFO, "*SOUR#:FUNC? Successfully returned generate wave form to client.");
    return SCPI_RES_OK;
}



scpi_result_t RP_GenPhase(scpi_t *context) {

    rp_channel_t channel;
    scpi_number_t phase;
    int result;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &phase, true)) {
        RP_LOG(context,LOG_ERR, "*SOUR#:PHAS Failed to parse first argument.");
        return SCPI_RES_ERR;
    }

    result = rp_GenPhase(channel, phase.value);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:PHAS Failed to set generate "
            "phase: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SOUR#:PHAS Successfully set generate phase.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenPhaseQ(scpi_t *context) {

    rp_channel_t channel;
    int result;
    float phase;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetPhase(channel, &phase);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:PHAS? Failed to get "
            "generate phase: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    SCPI_ResultFloat(context, phase);

    RP_LOG(context,LOG_INFO, "*SOUR#:PHAS? Successfully returned "
        "generate phase value to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenDutyCycle(scpi_t *context) {

    rp_channel_t channel;
    float duty_cycle;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamFloat(context, &duty_cycle, true)){
        RP_LOG(context,LOG_ERR, "*SOUR#:DCYC Failed to parse first argument");
        return SCPI_RES_ERR;
    }

    if (rp_GenDutyCycle(channel, duty_cycle) != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:DCYC Failed to set generate duty cycle");
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SOUR#:DCYC Successfully set generate duty cycle.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenDutyCycleQ(scpi_t *context) {

    rp_channel_t channel;
    float duty_cycle;
    int result;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetDutyCycle(channel, &duty_cycle);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:DCYC? Failed to get "
            "generate duty cycle: %s", rp_GetError(result));

        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, duty_cycle);

    RP_LOG(context,LOG_INFO, "*SOUR#:DCYC Successfully "
        "returned generate duty cycle value to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenArbitraryWaveForm(scpi_t *context) {

    rp_channel_t channel;
    float buffer[DAC_BUFFER_SIZE];
    uint32_t size;
    int result;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamBufferFloat(context, buffer, &size, true)){
        RP_LOG(context,LOG_ERR, "*SOUR#:TRAC:DATA:DATA Failed to "
            "arbitrary waveform data parameter.");
        return SCPI_RES_ERR;
    }

    result = rp_GenArbWaveform(channel, buffer, size);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:TRAC:DATA:DATA Failed to "
            "set arbitrary waveform data: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SOUR#:TRAC:DATA:DATA Successfully set arbitrary waveform data.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenArbitraryWaveFormQ(scpi_t *context) {

    rp_channel_t channel;
    float buffer[DAC_BUFFER_SIZE];
    uint32_t size;
    int result;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetArbWaveform(channel, buffer, &size);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:TRAC:DATA:DATA? Failed to "
            "get arbitrary waveform data: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultBufferFloat(context, buffer, size);

    RP_LOG(context,LOG_INFO, "*SOUR#:TRAC:DATA:DATA? Successfully "
        "returned arbitrary waveform data to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenGenerateMode(scpi_t *context) {

    rp_channel_t channel;
    int32_t usr_mode;
    int result;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamChoice(context, scpi_RpGenMode, &usr_mode, true)){
        RP_LOG(context,LOG_ERR, "*SOUR#:BURS:STAT Failed to parse first parameter.");
        return SCPI_RES_ERR;
    }

    rp_gen_mode_t mode = (rp_gen_mode_t)usr_mode;
    result = rp_GenMode(channel, mode);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:BURS:STAT Failed to get generate "
            "mode: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SOUR#:BURS:STAT Successfully set generate mode.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenGenerateModeQ(scpi_t *context) {

    rp_channel_t channel;
    int result;
    const char *gen_mode;
    rp_gen_mode_t mode;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetMode(channel, &mode);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:BURS:STAT? Failed to get generate "
            "mode: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    int32_t i_mode = mode;

    if(!SCPI_ChoiceToName(scpi_RpGenMode, i_mode, &gen_mode)){
        RP_LOG(context,LOG_ERR, "*SOUR#:BURS:STAT? Invalid generate mode.");
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, gen_mode);

    RP_LOG(context,LOG_INFO, "*SOUR#:BURS:STAT? Successfully returned "
        "generate mode status to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstCount(scpi_t *context) {

    rp_channel_t channel;
    int result, count;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamInt32(context, &count, true)){
        RP_LOG(context,LOG_ERR, "*SOUR#:BURS:NCYC Failed to parse "
            "first parameter.");
        return SCPI_RES_ERR;
    }

    result = rp_GenBurstCount(channel, count);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:BURS:NCYC Failed to set "
            "count parameter: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SOUR#:BURS:NCYC Successfully set generate burst count.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstCountQ(scpi_t *context) {

    rp_channel_t channel;
    int result, count;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetBurstCount(channel, &count);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:BURS:NCYC? Failed to get generate "
            "burst count: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultInt32(context, count);

    RP_LOG(context,LOG_INFO, "*SOUR#:BURS:NCYC? Successfully returned generate "
        "burst count value to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstRepetitions(scpi_t *context) {

    rp_channel_t channel;
    int result, repetitions;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamInt32(context, &repetitions, true)){
        RP_LOG(context,LOG_ERR, "*SOUR#:BURS:NOR Failed to parse "
            "first parameter.");
        return SCPI_RES_ERR;
    }

    result = rp_GenBurstRepetitions(channel, repetitions);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:BURS:NOR Failed to set "
            "generate burst repetitions: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SOUR#:BURS:NOR Successfully set generate repetitions.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstRepetitionsQ(scpi_t *context) {

    rp_channel_t channel;
    int result, repetitions;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetBurstRepetitions(channel, &repetitions);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:BURS:NOR Failed to get "
            "generate repetitions: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultInt32(context, repetitions);

    RP_LOG(context,LOG_INFO, "*SOUR#:BURS:NOR Successfully returned "
        "generate repetitions value to client.");

    return SCPI_RES_OK;
}


scpi_result_t RP_GenBurstPeriod(scpi_t *context) {

    rp_channel_t channel;
    int result;
    uint32_t period;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamUInt32(context, &period, true)){
        RP_LOG(context,LOG_ERR,"*SOUR#:BURS:INT:PER Failed to "
            "parse first parameter.");
        return SCPI_RES_ERR;
    }

    result = rp_GenBurstPeriod(channel, period);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:BURS:INT:PER Failed to get "
            "generate burst period: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SOUR#:BURS:INT:PER Successfully set generate burst period.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstPeriodQ(scpi_t *context) {

    rp_channel_t channel;
    int result;
    uint32_t period;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetBurstPeriod(channel, &period);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:BURS:INT:PER Failed to get "
            "generate burst period: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, period, 10);

    RP_LOG(context,LOG_INFO, "*SOUR#:BURS:INT:PER Successfully returned "
        "generate burst period value to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstLastValue(scpi_t *context) {

    rp_channel_t channel;
    int result;
    float value;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamFloat(context, &value, true)){
        RP_LOG(context,LOG_ERR,"*SOUR#:BURS:LastValue Failed to "
            "parse first parameter.");
        return SCPI_RES_ERR;
    }

    result = rp_GenBurstLastValue(channel, value);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:BURS:LastValue Failed to set "
            "generate burst last value: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SOUR#:BURS:LastValue Successfully set generate burst last value.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstLastValueQ(scpi_t *context) {

    rp_channel_t channel;
    int result;
    float value;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetBurstLastValue(channel, &value);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:BURS:LastValue? Failed to get "
            "generate burst period: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, value);

    RP_LOG(context,LOG_INFO, "*SOUR#:BURS:LastValue? Successfully returned "
        "generate burst last value to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenInitValue(scpi_t *context) {

    rp_channel_t channel;
    int result;
    float value;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamFloat(context, &value, true)){
        RP_LOG(context,LOG_ERR,"*SOUR#:InitValue Failed to "
            "parse first parameter.");
        return SCPI_RES_ERR;
    }

    result = rp_GenSetInitGenValue(channel, value);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:InitValue Failed to set "
            "generate burst last value: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SOUR#:InitValue Successfully set generate burst last value.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenInitValueQ(scpi_t *context) {

    rp_channel_t channel;
    int result;
    float value;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetInitGenValue(channel, &value);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:InitValue? Failed to get "
            "generate burst period: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, value);

    RP_LOG(context,LOG_INFO, "*SOUR#:InitValue? Successfully returned "
        "generate burst last value to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenTriggerSource(scpi_t *context) {

    rp_channel_t channel;
    int32_t trig_choice;
    int result;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamChoice(context, scpi_RpGenTrig, &trig_choice, true)){
        RP_LOG(context,LOG_ERR, "*SOUR#:TRIG:SOUR Failed to parse first parameter.");
        return SCPI_RES_ERR;
    }

    rp_trig_src_t trig_src = (rp_trig_src_t)trig_choice;

    result = rp_GenTriggerSource(channel, trig_src);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:TRIG:SOUR Failed to set generate"
        " trigger source: %s", rp_GetError(result));

        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SOUR#:TRIG:SOUR Successfully set generate trigger source.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenTriggerSourceQ(scpi_t *context) {

    rp_channel_t channel;
    const char *trig_name;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    rp_trig_src_t trig_src;
    if (rp_GenGetTriggerSource(channel, &trig_src) != RP_OK){
        return SCPI_RES_ERR;
    }

    int32_t trig_n = trig_src;
    if(!SCPI_ChoiceToName(scpi_RpGenTrig, trig_n, &trig_name)){
        RP_LOG(context,LOG_ERR, "*SOUR#:TRIG:SOUR? Failed to parse trigger name.");
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, trig_name);

    RP_LOG(context,LOG_INFO, "*SOUR#:TRIG:SOUR? Successfully returend"
    " generate trigger status to client.");

    return SCPI_RES_OK;
}

scpi_result_t RP_GenTrigger(scpi_t *context) {

    rp_channel_t channel;
    int result;
    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenResetTrigger(channel);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:TRIG:INT Failed to set immediate "
            "trigger: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SOUR#:TRIG:INT Successfully set immediate trigger.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenTriggerBoth(scpi_t *context) {
    int result;
    result = rp_GenSynchronise();
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR:TRIG:INT Failed to set immediate "
            "trigger: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SOUR:TRIG:INT Successfully set immediate trigger.");
    return SCPI_RES_OK;
}


scpi_result_t RP_GenAmplitude(scpi_t *context) {

    rp_channel_t channel;
    scpi_number_t amplitude;
    rp_gen_gain_t gain;
    int result;
    float offset;
    float amp;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &amplitude, true)) {
        RP_LOG(context,LOG_ERR, "*SOUR#:VOLT Failed to parse first parameter.");
        return SCPI_RES_ERR;
    }

    result = rp_GenGetOffset(channel, &offset);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:VOLT Failed to get offset: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetGainOut(channel, &gain);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:VOLT Failed to get gain out: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    amp = amplitude.value;

    if (rp_HPGetIsGainDACx5OrDefault()){
        if (gain == RP_GAIN_5X)
            offset *= 5;

        if (fabs(offset) + fabs(amp) > 1.0) {
            gain = RP_GAIN_5X;
        }else{
            gain = RP_GAIN_1X;
        }
    }else{
        if (gain == RP_GAIN_5X){
            RP_LOG(context,LOG_ERR, "*SOUR#:VOLT Failed. Wrong gain out: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }

    result = rp_GenAmp(channel, amp / (gain == RP_GAIN_5X ? 5.0 : 1.0));
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:VOLT Failed to set amplitude: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenOffset(channel, offset / (gain == RP_GAIN_5X ? 5.0 : 1.0));
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:VOLT Failed to set offset: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    if (rp_HPGetIsGainDACx5OrDefault()){
        result = rp_GenSetGainOut(channel, gain);
        if(result != RP_OK){
            RP_LOG(context,LOG_ERR, "*SOUR#:VOLT Failed to set gain out: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }

    RP_LOG(context,LOG_INFO, "*SOUR#:VOLT Successfully set amplitude.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenAmplitudeQ(scpi_t *context) {

    rp_channel_t channel;
    rp_gen_gain_t gain;
    float amplitude;
    int result;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetAmp(channel, &amplitude);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:VOLT? Failed to set amplitude: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetGainOut(channel, &gain);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:VOLT? Failed to get gain out: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    if (rp_HPGetIsGainDACx5OrDefault()){
        if (gain == RP_GAIN_5X)
            amplitude *= 5.0;
    }else{
        if (gain == RP_GAIN_5X){
            RP_LOG(context,LOG_ERR, "*SOUR#:VOLT? Failed. Wrong gain out: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }


    SCPI_ResultFloat(context, amplitude);

    RP_LOG(context,LOG_INFO, "*SOUR#:VOLT? Successfully returned amplitude value to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenOffset(scpi_t *context) {

    rp_channel_t channel;
    scpi_number_t offset;
    rp_gen_gain_t gain;
    float offs;
    float amp;
    int result;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &offset, true)) {
        RP_LOG(context,LOG_ERR, "*SOUR#:VOLT:OFFS Failed to parse parameter.");
        return SCPI_RES_ERR;
    }

    result = rp_GenGetAmp(channel, &amp);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:VOLT:OFFS Failed to get amplitude: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetGainOut(channel, &gain);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:VOLT:OFFS Failed to get gain out: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    offs = offset.value;

    if (rp_HPGetIsGainDACx5OrDefault()){
        if (gain == RP_GAIN_5X) amp *= 5;

        if (fabs(offs) + fabs(amp) > 1.0) {
            gain = RP_GAIN_5X;
        }else{
            gain = RP_GAIN_1X;
        }
    }else{
        if (gain == RP_GAIN_5X){
            RP_LOG(context,LOG_ERR, "*SOUR#:VOLT:OFFS Failed. Wrong gain out: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }


    result = rp_GenAmp(channel, amp / (gain == RP_GAIN_5X ? 5.0 : 1.0));
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:VOLT:OFFS Failed to set amplitude: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenOffset(channel, offs / (gain == RP_GAIN_5X ? 5.0 : 1.0));
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:VOLT:OFFS Failed to set offset: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    if (rp_HPGetIsGainDACx5OrDefault()){
        result = rp_GenSetGainOut(channel, gain);
        if(result != RP_OK){
            RP_LOG(context,LOG_ERR, "*SOUR#:VOLT:OFFS Failed to set gain out: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }

    RP_LOG(context,LOG_INFO, "*SOUR#:VOLT:OFFS Successfully set generate offset value.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenOffsetQ(scpi_t *context) {

    rp_channel_t channel;
    rp_gen_gain_t gain;
    float offset;
    int result;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    result = rp_GenGetOffset(channel, &offset);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:VOLT:OFFS? Failed to get "
            "generate offset: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetGainOut(channel, &gain);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*SOUR#:VOLT:OFFS? Failed to get gain out: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    if (rp_HPGetIsGainDACx5OrDefault()){
        if (gain == RP_GAIN_5X)
            offset *= 5;
    }else{
        if (gain == RP_GAIN_5X){
            RP_LOG(context,LOG_ERR, "*SOUR#:VOLT:OFFS? Failed. Wrong gain out: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }

    SCPI_ResultFloat(context, offset);

    RP_LOG(context,LOG_INFO, "*SOUR#:VOLT:OFFS? Successfully returned offset to the client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenExtTriggerDebouncerUs(scpi_t *context) {
    scpi_number_t value;

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        RP_LOG(context,LOG_ERR, "*SOUR:TRIG:EXT:DEBouncerUs is missing first parameter.");
        return SCPI_RES_ERR;
    }

    // Now set threshold
    int result = 0;
    result = rp_GenSetExtTriggerDebouncerUs((double) value.value);
    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*SOUR:TRIG:EXT:DEBouncerUs Failed to set: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }


    RP_LOG(context,LOG_INFO, "*SOUR:TRIG:EXT:DEBouncerUs Successfully set value.");
    return SCPI_RES_OK;
}

scpi_result_t RP_GenExtTriggerDebouncerUsQ(scpi_t *context) {
    double value;
    int result = rp_GenGetExtTriggerDebouncerUs(&value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*SOUR:TRIG:EXT:DEBouncerUs? Failed to get: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultDouble(context, value);

    RP_LOG(context,LOG_INFO, "*SOUR:TRIG:EXT:DEBouncerUs? Successfully returned value.");
    return SCPI_RES_OK;
}