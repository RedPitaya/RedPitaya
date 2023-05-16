/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server apin SCPI commands implementation
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
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "common.h"
#include "api_cmd.h"
#include "scpi/parser.h"
#include "acquire.h"
#include "dpin.h"
#include "apin.h"

#include "generate.h"


const scpi_choice_def_t scpi_TRIG_O_mode[] = {
    {"ADC", 0},
    {"DAC", 1},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_DAISY_state[] = {
    {"OFF", 0},
    {"ON", 1},
    SCPI_CHOICE_LIST_END
};

scpi_result_t RP_InitAll(scpi_t *context){

    int result = rp_Init();

    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*RP:INIT Failed to initialize Red "
            "Pitaya modules: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*RP:INIT Successfully inizitalized Red Pitaya modules.");
    return SCPI_RES_OK;
}

scpi_result_t RP_ResetAll(scpi_t *context){

    int result = RP_AcqReset(context);

    if(result != SCPI_RES_OK){
        RP_LOG(context,LOG_ERR, "*RP:RST Failed to reset Red "
            "Pitaya ACQ modules: %s (%d)", rp_GetError(result),result);
        return SCPI_RES_ERR;
    }

    result = RP_AnalogPinReset(context);

    if(result != SCPI_RES_OK){
        RP_LOG(context,LOG_ERR, "*RP:RST Failed to reset Red "
            "Pitaya APIO modules: %s (%d)", rp_GetError(result),result);
        return SCPI_RES_ERR;
    }

    result = RP_DigitalPinReset(context);

    if(result != SCPI_RES_OK){
        RP_LOG(context,LOG_ERR, "*RP:RST Failed to reset Red "
            "Pitaya DPIO modules: %s (%d)", rp_GetError(result),result);
        return SCPI_RES_ERR;
    }

    result = RP_GenReset(context);

    if(result != SCPI_RES_OK){
        RP_LOG(context,LOG_ERR, "*RP:RST Failed to reset Red "
            "Pitaya Gen modules: %s (%d)", rp_GetError(result),result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*RP:RST Successfully reset Red Pitaya modules.");
    return SCPI_RES_OK;
}

scpi_result_t RP_ReleaseAll(scpi_t *context){

    int result = rp_Release();

    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*RP:RELEASE Failed to release Red "
            "Pitaya modules: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*RP:RELEASE Successfully released Red Pitaya modules.");
    return SCPI_RES_OK;
}


scpi_result_t RP_EnableDigLoop(scpi_t *context){

    int result = rp_EnableDigitalLoop(true);

    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*RP:DIG:LOop Failed to initialize Red Pitaya"
            " digital loop: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*RP:DIG:LOop Successfully initialize Red Pitaya"
        " digital loop.");

    return SCPI_RES_OK;
}


scpi_result_t RP_EnableDaisyChainSync(scpi_t *context){
    int32_t value;

    if (!SCPI_ParamChoice(context, scpi_DAISY_state, &value, true)) {
        RP_LOG(context,LOG_ERR, "*DAISY:ENable is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_SetEnableDaisyChainSync((bool)value);
    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*DAISY:ENable Failed to enabled mode: %d", result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*DAISY:ENable Successfully enabled mode.");
    return SCPI_RES_OK;
}

scpi_result_t RP_EnableDaisyChainSyncQ(scpi_t *context){
    const char *_name;

    bool value;
    int result = rp_GetEnableDaisyChainSync(&value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*DAISY:ENable? Failed get state: %d", result);
        return SCPI_RES_ERR;
    }

    if(!SCPI_ChoiceToName(scpi_DAISY_state, (int32_t)value, &_name)){
        RP_LOG(context,LOG_ERR, "*DAISY:ENable? Failed to parse state.");
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, _name);


    RP_LOG(context,LOG_INFO, "*DAISY:ENable? Successfully returned state.");
    return SCPI_RES_OK;
}

scpi_result_t RP_DpinEnableTrigOutput(scpi_t *context){
    int32_t value;

    if (!SCPI_ParamChoice(context, scpi_DAISY_state, &value, true)) {
        RP_LOG(context,LOG_ERR, "*DAISY:TRIG_O:ENable is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_SetDpinEnableTrigOutput((bool)value);
    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*DAISY:TRIG_O:ENable Failed to enabled mode: %d", result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*DAISY:TRIG_O:ENable Successfully enabled mode.");
    return SCPI_RES_OK;
}

scpi_result_t RP_DpinEnableTrigOutputQ(scpi_t *context){
    const char *_name;

    bool value;
    int result = rp_GetDpinEnableTrigOutput(&value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*DAISY:TRIG_O:ENable? Failed get state: %d", result);
        return SCPI_RES_ERR;
    }

    if(!SCPI_ChoiceToName(scpi_DAISY_state, (int32_t)value, &_name)){
        RP_LOG(context,LOG_ERR, "*DAISY:TRIG_O:ENable? Failed to parse state.");
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, _name);


    RP_LOG(context,LOG_INFO, "*DAISY:TRIG_O:ENable? Successfully returned state.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SourceTrigOutput(scpi_t *context){
    int32_t value;

    if (!SCPI_ParamChoice(context, scpi_TRIG_O_mode, &value, true)) {
        RP_LOG(context,LOG_ERR, "*DAISY:TRIG_O:SOUR is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_SetSourceTrigOutput((rp_outTiggerMode_t)value);
    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*DAISY:TRIG_O:SOUR Failed to set mode: %d", result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*DAISY:TRIG_O:SOUR Successfully set mode.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SourceTrigOutputQ(scpi_t *context){
    const char *_name;

    rp_outTiggerMode_t value;
    int result = rp_GetSourceTrigOutput(&value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*DAISY:TRIG_O:SOUR? Failed get state: %d", result);
        return SCPI_RES_ERR;
    }

    if(!SCPI_ChoiceToName(scpi_TRIG_O_mode, (int32_t)value, &_name)){
        RP_LOG(context,LOG_ERR, "*DAISY:TRIG_O:SOUR? Failed to parse state.");
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, _name);


    RP_LOG(context,LOG_INFO, "*DAISY:TRIG_O:SOUR? Successfully returned mode.");
    return SCPI_RES_OK;
}
