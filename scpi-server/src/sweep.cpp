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

#include "sweep.h"

#include "rp.h"
#include "rp_hw-profiles.h"
#include "common/rp_sweep.h"
#include "common.h"
#include "scpi/parser.h"
#include "scpi/units.h"

rp_sweep_api::CSweepController g_sweepController;


const scpi_choice_def_t scpi_sweep_mode[] = {
    {"LINEAR", 0},
    {"LOG", 1},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_sweep_dir[] = {
    {"NORMAL", 0},
    {"UP_DOWN", 1},
    SCPI_CHOICE_LIST_END
};

void stopSweep(){
    g_sweepController.stop();
}


scpi_result_t RP_GenSweepReset(scpi_t *context) {
    g_sweepController.resetAll();
    RP_LOG_INFO("%s",rp_GetError(RP_OK))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenSweepPause(scpi_t *context) {

    bool state_c;
    /* Parse first, STATE argument */
    if(!SCPI_ParamBool(context, &state_c, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }
    g_sweepController.pause(state_c);
    RP_LOG_INFO("%s",rp_GetError(RP_OK))
    return SCPI_RES_OK;
}


scpi_result_t RP_GenSweepState(scpi_t *context) {

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
        g_sweepController.run();
    }

    result = g_sweepController.genSweep(channel,state_c);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set sweep state: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    if (g_sweepController.isAllDisabled()){
        g_sweepController.stop();
    }

    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenSweepStateQ(scpi_t *context){

    bool enabled;
    rp_channel_t channel;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result = g_sweepController.isGen(channel, &enabled);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get sweep state: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, enabled);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}


scpi_result_t RP_GenSweepFreqStart(scpi_t *context){

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

    auto result = g_sweepController.setStartFreq(channel, frequency.content.value);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set start frequency: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenSweepFreqStartQ(scpi_t *context) {

    float frequency;
    rp_channel_t channel;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result =  g_sweepController.getStartFreq(channel, &frequency);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get start frequency: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    /* Return data to client */
    SCPI_ResultFloat(context, frequency);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}


scpi_result_t RP_GenSweepFreqStop(scpi_t *context){

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

    auto result = g_sweepController.setStopFreq(channel, frequency.content.value);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set stop frequency: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenSweepFreqStopQ(scpi_t *context) {

    float frequency;
    rp_channel_t channel;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result =  g_sweepController.getStopFreq(channel, &frequency);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get stop frequency: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    /* Return data to client */
    SCPI_ResultFloat(context, frequency);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenSweepTime(scpi_t *context){

    scpi_number_t time;
    rp_channel_t channel;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    /* Parse first parameter */
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &time, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = g_sweepController.setTime(channel, time.content.value);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set time: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenSweepTimeQ(scpi_t *context) {

    int time;
    rp_channel_t channel;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    auto result =  g_sweepController.getTime(channel, &time);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get time: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    /* Return data to client */
    SCPI_ResultInt32(context, time);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}


scpi_result_t RP_GenSweepMode(scpi_t *context) {

    rp_channel_t channel;
    int32_t mode;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    if(!SCPI_ParamChoice(context, scpi_sweep_mode, &mode, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = g_sweepController.setMode(channel, (rp_gen_sweep_mode_t)mode );

    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set sweep mode: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}


scpi_result_t RP_GenSweepModeQ(scpi_t *context) {

    const char *name;
    rp_channel_t channel;
    rp_gen_sweep_mode_t mode;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if (g_sweepController.getMode(channel, &mode) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ChoiceToName(scpi_sweep_mode, (int)mode, &name)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Failed to get sweep mode.")
        return SCPI_RES_ERR;
    }

    /* Return result to client */
    SCPI_ResultMnemonic(context, name);
    RP_LOG_INFO("%s",rp_GetError(RP_OK))
    return SCPI_RES_OK;
}



scpi_result_t RP_GenSweepDir(scpi_t *context) {

    rp_channel_t channel;
    int32_t mode;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    if(!SCPI_ParamChoice(context, scpi_sweep_dir, &mode, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = g_sweepController.setDir(channel, (rp_gen_sweep_dir_t)mode );

    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set sweep direction: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}


scpi_result_t RP_GenSweepDirQ(scpi_t *context) {

    const char *name;
    rp_channel_t channel;
    rp_gen_sweep_dir_t mode;

    if (RP_ParseChArgvDAC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    if (g_sweepController.getDir(channel, &mode) != RP_OK){
        return SCPI_RES_ERR;
    }

    if(!SCPI_ChoiceToName(scpi_sweep_dir, (int)mode, &name)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Failed to get sweep direction.")
        return SCPI_RES_ERR;
    }

    /* Return result to client */
    SCPI_ResultMnemonic(context, name);
    RP_LOG_INFO("%s",rp_GetError(RP_OK))
    return SCPI_RES_OK;
}


