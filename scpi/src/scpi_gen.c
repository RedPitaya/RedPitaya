/**
 * @brief Red Pitaya Scpi server generate SCPI commands implementation
 * @Author Red Pitaya
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "scpi_gen.h"
#include "redpitaya/rp1.h"

#include "common.h"
#include "scpi/parser.h"
#include "scpi/units.h"

/* These structures are a direct API mirror 
and should not be altered! */
const scpi_choice_def_t scpi_RpWForm[] = {
    {"SINusoid", 0},
    {"SQUare",   1},
    {"TRIangle", 2},
    {"USER",     7},
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
    if (rp_GenReset() != 0) {
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_enable(scpi_t *context) {
    int unsigned channel;
    bool value;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    if(!SCPI_ParamBool(context, &value, true)){
        return SCPI_RES_ERR;
    }
    rp_gen_out_set_enable(&gen[channel].out, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_enable(scpi_t *context){
    int unsigned channel;
    bool value;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    value = rp_gen_out_set_enable(&gen[channel].out);
    SCPI_ResultBool(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_frequency(scpi_t *context) {
    int unsigned channel;
    scpi_number_t value;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (rp_asg_gen_set_frequency(&gen[channel].per, value.content.value) != 0) {
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_frequency(scpi_t *context) {
    int unsigned channel;
    double value;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    value = rp_asg_gen_get_frequency(&gen[channel].per);
    SCPI_ResultDouble(context, value);
    return SCPI_RES_OK;
}    

scpi_result_t rpscpi_gen_set_phase(scpi_t *context) {
    int unsigned channel;
    scpi_number_t value;
    
    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (rp_asg_gen_set_phase(&gen[channel].per, value.content.value) != 0) {
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_phase(scpi_t *context) {
    int unsigned channel;
    double value;
    
    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    value = rpscpi_gen_get_phase(&gen[channel].per);
    SCPI_ResultDouble(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_amplitude(scpi_t *context) {
    int unsigned channel;
    scpi_number_t value;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    // special values are not allowed
    if (value.special) {
        return SCPI_RES_ERR;
    } else {
        if (rp_gen_out_set_amplitude(&gen[channel].out, (float) value.content.value)) {
            return SCPI_RES_ERR;
        }
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_amplitude(scpi_t *context) {
    int unsigned channel;
    float value;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    value rp_gen_out_get_amplitude(&gen[channel].out);
    SCPI_ResultFloat(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_offset(scpi_t *context) {
    int unsigned channel;
    scpi_number_t value;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    // special values are not allowed
    if (value.special) {
        return SCPI_RES_ERR;
    } else {
        if (rp_gen_out_set_offset(&gen[channel].out, (float) value.content.value)) {
            return SCPI_RES_ERR;
        }
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_offset(scpi_t *context) {
    int unsigned channel;
    float value;

    if (RP_ParseChArgv(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    value rp_gen_out_get_offset(&gen[channel].out);
    SCPI_ResultFloat(context, value);
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
