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
#include "sweep.h"
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

const scpi_choice_def_t scpi_RpGenLoad[] = {
    {"INF",         0},
    {"L50",         1},
    SCPI_CHOICE_LIST_END
};

scpi_result_t RP_GenReset(scpi_t *context) {
    RP_GenSweepDefault(context);
    auto result = rp_GenReset();
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to reset Red Pitaya generate: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenSync(scpi_t *context) {
    auto result = rp_GenSynchronise();
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to sync Red Pitaya generate:: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenSyncState(scpi_t *context) {

    bool state_c;

    /* Parse first, STATE argument */
    if(!SCPI_ParamBool(context, &state_c, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_GenOutEnableSync(state_c);

    if(result != RP_OK){
        RP_LOG_CRIT("Failed to enable generate: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenState(scpi_t *context) {

    auto result = 0;
    rp_channel_t channel;
    bool state_c;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    /* Parse first, STATE argument */
    if(!SCPI_ParamBool(context, &state_c, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }
    if (state_c){
        result = rp_GenOutEnable(channel);
    }else{
        result = rp_GenOutDisable(channel);
    }

    if(result != RP_OK){
        RP_LOG_CRIT("Failed to enable generate: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenStateQ(scpi_t *context){

    bool enabled;
    rp_channel_t channel;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result = rp_GenOutIsEnabled(channel, &enabled);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get generate state: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, enabled);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenFrequency(scpi_t *context){

    scpi_number_t frequency;
    rp_channel_t channel;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    /* Parse first, FREQUENCY parameter */
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &frequency, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_GenFreq(channel, frequency.content.value);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set frequency: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenFrequencyQ(scpi_t *context) {

    float frequency;
    rp_channel_t channel;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result = rp_GenGetFreq(channel, &frequency);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get frequency: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    /* Return data to client */
    SCPI_ResultFloat(context, frequency);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenFrequencyDirect(scpi_t *context){

    scpi_number_t frequency;
    rp_channel_t channel;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    /* Parse first, FREQUENCY parameter */
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &frequency, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_GenFreqDirect(channel, frequency.content.value);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set frequency: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenWaveForm(scpi_t *context) {

    rp_channel_t channel;
    int32_t wave_form;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    /* Read WAVEFORM parameter */
    if(!SCPI_ParamChoice(context, scpi_RpWForm, &wave_form, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    rp_waveform_t wf = (rp_waveform_t)wave_form;
    auto result = rp_GenWaveform(channel, wf);

    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set generate wave form: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
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
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Failed to get wave form.")
        return SCPI_RES_ERR;
    }

    /* Return result to client */
    SCPI_ResultMnemonic(context, wf_name);
    RP_LOG_INFO("%s",rp_GetError(RP_OK))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenPhase(scpi_t *context) {

    rp_channel_t channel;
    scpi_number_t phase;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &phase, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_GenPhase(channel, phase.content.value);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set generate phase. %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenPhaseQ(scpi_t *context) {

    rp_channel_t channel;
    float phase;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result = rp_GenGetPhase(channel, &phase);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get generate phase: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    SCPI_ResultFloat(context, phase);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenDutyCycle(scpi_t *context) {

    rp_channel_t channel;
    float duty_cycle;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamFloat(context, &duty_cycle, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }
    auto result = rp_GenDutyCycle(channel, duty_cycle);
    if (result != RP_OK){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Failed to set generate duty cycle");
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenDutyCycleQ(scpi_t *context) {

    rp_channel_t channel;
    float duty_cycle;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result = rp_GenGetDutyCycle(channel, &duty_cycle);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get generate duty cycle: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, duty_cycle);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenArbitraryWaveForm(scpi_t *context) {

    rp_channel_t channel;
    float buffer[DAC_BUFFER_SIZE];
    uint32_t size;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamBufferFloat(context, buffer, &size, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Failed to arbitrary waveform data parameter.");
        return SCPI_RES_ERR;
    }
    auto result = rp_GenArbWaveform(channel, buffer, size);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set arbitrary waveform data: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenArbitraryWaveFormQ(scpi_t *context) {

    rp_channel_t channel;
    float buffer[DAC_BUFFER_SIZE];
    uint32_t size = DAC_BUFFER_SIZE;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    uint32_t ret_size = 0;
    auto result = rp_GenGetArbWaveform(channel, buffer, size, &ret_size);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get arbitrary waveform data: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultBufferFloat(context, buffer, ret_size);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenGenerateMode(scpi_t *context) {

    rp_channel_t channel;
    int32_t usr_mode;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamChoice(context, scpi_RpGenMode, &usr_mode, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    rp_gen_mode_t mode = (rp_gen_mode_t)usr_mode;
    auto result = rp_GenMode(channel, mode);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set generate mode: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenGenerateModeQ(scpi_t *context) {

    rp_channel_t channel;
    const char *gen_mode;
    rp_gen_mode_t mode;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result = rp_GenGetMode(channel, &mode);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get generate mode: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    int32_t i_mode = mode;

    if(!SCPI_ChoiceToName(scpi_RpGenMode, i_mode, &gen_mode)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Invalid generate mode.")
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, gen_mode);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstCount(scpi_t *context) {

    rp_channel_t channel;
    int count;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamInt32(context, &count, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_GenBurstCount(channel, count);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set burst count parameter: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstCountQ(scpi_t *context) {

    rp_channel_t channel;
    int count;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result = rp_GenGetBurstCount(channel, &count);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get burst count parameter: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultInt32(context, count);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstRepetitions(scpi_t *context) {

    rp_channel_t channel;
    int repetitions;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamInt32(context, &repetitions, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_GenBurstRepetitions(channel, repetitions);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set burst repetitions: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstRepetitionsQ(scpi_t *context) {

    rp_channel_t channel;
    int repetitions;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result = rp_GenGetBurstRepetitions(channel, &repetitions);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get burst repetitions: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultInt32(context, repetitions);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}


scpi_result_t RP_GenBurstPeriod(scpi_t *context) {

    rp_channel_t channel;
    uint32_t period;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamUInt32(context, &period, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_GenBurstPeriod(channel, period);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set burst period: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstPeriodQ(scpi_t *context) {

    rp_channel_t channel;
    uint32_t period;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result = rp_GenGetBurstPeriod(channel, &period);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get burst period: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, period, 10);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstLastValue(scpi_t *context) {

    rp_channel_t channel;
    float value;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamFloat(context, &value, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_GenBurstLastValue(channel, value);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set generate burst last value: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenBurstLastValueQ(scpi_t *context) {

    rp_channel_t channel;
    float value;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result = rp_GenGetBurstLastValue(channel, &value);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get generate burst last value: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, value);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenInitValue(scpi_t *context) {

    rp_channel_t channel;
    float value;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamFloat(context, &value, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_GenSetInitGenValue(channel, value);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set generate burst init value: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenInitValueQ(scpi_t *context) {

    rp_channel_t channel;
    float value;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result = rp_GenGetInitGenValue(channel, &value);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get generate burst init value: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, value);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenTriggerSource(scpi_t *context) {

    rp_channel_t channel;
    int32_t trig_choice;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamChoice(context, scpi_RpGenTrig, &trig_choice, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    rp_trig_src_t trig_src = (rp_trig_src_t)trig_choice;

    auto result = rp_GenTriggerSource(channel, trig_src);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set generate trigger source: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenTriggerSourceQ(scpi_t *context) {

    rp_channel_t channel;
    const char *trig_name;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    rp_trig_src_t trig_src;
    auto result = rp_GenGetTriggerSource(channel, &trig_src);
    if (result != RP_OK){
        return SCPI_RES_ERR;
    }

    int32_t trig_n = trig_src;
    if(!SCPI_ChoiceToName(scpi_RpGenTrig, trig_n, &trig_name)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, trig_name);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenTrigger(scpi_t *context) {

    rp_channel_t channel;
    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result = rp_GenResetTrigger(channel);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set trigger: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenTriggerBoth(scpi_t *context) {
    auto result = rp_GenSynchronise();
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set trigger: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}


scpi_result_t RP_GenAmplitude(scpi_t *context) {

    rp_channel_t channel;
    scpi_number_t amplitude;
    rp_gen_gain_t gain;
    float offset;
    float amp;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &amplitude, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_GenGetOffset(channel, &offset);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get offset: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetGainOut(channel, &gain);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get gain out: %s", rp_GetError(result));
       return SCPI_RES_ERR;
    }

    amp = amplitude.content.value;

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
            RP_LOG_CRIT("Failed. Wrong gain out: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }

    result = rp_GenAmp(channel, amp / (gain == RP_GAIN_5X ? 5.0 : 1.0));
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set amplitude: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenOffset(channel, offset / (gain == RP_GAIN_5X ? 5.0 : 1.0));
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set offset: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    if (rp_HPGetIsGainDACx5OrDefault()){
        result = rp_GenSetGainOut(channel, gain);
        if(result != RP_OK){
            RP_LOG_CRIT("Failed to set gain out: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenAmplitudeQ(scpi_t *context) {

    rp_channel_t channel;
    rp_gen_gain_t gain;
    float amplitude;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result = rp_GenGetAmp(channel, &amplitude);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get amplitude: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetGainOut(channel, &gain);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get gain out: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    if (rp_HPGetIsGainDACx5OrDefault()){
        if (gain == RP_GAIN_5X)
            amplitude *= 5.0;
    }else{
        if (gain == RP_GAIN_5X){
            RP_LOG_CRIT("Failed. Wrong gain out: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }
    SCPI_ResultFloat(context, amplitude);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenOffset(scpi_t *context) {

    rp_channel_t channel;
    scpi_number_t offset;
    rp_gen_gain_t gain;
    float offs;
    float amp;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &offset, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_GenGetAmp(channel, &amp);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get amplitude: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetGainOut(channel, &gain);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get gain out: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    offs = offset.content.value;

    if (rp_HPGetIsGainDACx5OrDefault()){
        if (gain == RP_GAIN_5X) amp *= 5;

        if (fabs(offs) + fabs(amp) > 1.0) {
            gain = RP_GAIN_5X;
        }else{
            gain = RP_GAIN_1X;
        }
    }else{
        if (gain == RP_GAIN_5X){
            RP_LOG_CRIT("Failed. Wrong gain out: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }


    result = rp_GenAmp(channel, amp / (gain == RP_GAIN_5X ? 5.0 : 1.0));
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set amplitude: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenOffset(channel, offs / (gain == RP_GAIN_5X ? 5.0 : 1.0));
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set offset: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    if (rp_HPGetIsGainDACx5OrDefault()){
        result = rp_GenSetGainOut(channel, gain);
        if(result != RP_OK){
            RP_LOG_CRIT("Failed to set gain out: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenOffsetQ(scpi_t *context) {

    rp_channel_t channel;
    rp_gen_gain_t gain;
    float offset;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result = rp_GenGetOffset(channel, &offset);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get generate offset: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = rp_GenGetGainOut(channel, &gain);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get gain out: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    if (rp_HPGetIsGainDACx5OrDefault()){
        if (gain == RP_GAIN_5X)
            offset *= 5;
    }else{
        if (gain == RP_GAIN_5X){
            RP_LOG_CRIT("Failed. Wrong gain out: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }

    SCPI_ResultFloat(context, offset);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenExtTriggerDebouncerUs(scpi_t *context) {
    scpi_number_t value;

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_GenSetExtTriggerDebouncerUs((double) value.content.value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set debouncer: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenExtTriggerDebouncerUsQ(scpi_t *context) {
    double value;
    auto result = rp_GenGetExtTriggerDebouncerUs(&value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get debouncer: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultDouble(context, value);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenLoad(scpi_t * context){
    rp_channel_t channel;
    int32_t usr_mode;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ParamChoice(context, scpi_RpGenLoad, &usr_mode, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    rp_gen_load_mode_t mode = (rp_gen_load_mode_t)usr_mode;
    auto result = rp_GenSetLoadMode(channel, mode);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set generate mode: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenLoadQ(scpi_t * context){
    rp_channel_t channel;
    const char *gen_mode;
    rp_gen_load_mode_t mode;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result = rp_GenGetLoadMode(channel, &mode);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get generate load: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    int32_t i_mode = mode;

    if(!SCPI_ChoiceToName(scpi_RpGenLoad, i_mode, &gen_mode)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Invalid generate load.")
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, gen_mode);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}
