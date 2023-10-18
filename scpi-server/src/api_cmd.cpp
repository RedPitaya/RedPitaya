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
    auto result = rp_Init();
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to initialize Red Pitaya modules: %s", rp_GetError(result))
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_ResetAll(scpi_t *context){
    auto result = RP_AcqReset(context);
    if(result != SCPI_RES_OK){
        RP_LOG_CRIT("Failed to reset Red Pitaya ACQ modules: %s", rp_GetError(result))
        return SCPI_RES_ERR;
    }

    result = RP_AnalogPinReset(context);

    if(result != SCPI_RES_OK){
        RP_LOG_CRIT("Failed to reset Red Pitaya APIO modules: %s", rp_GetError(result))
        return SCPI_RES_ERR;
    }

    result = RP_DigitalPinReset(context);

    if(result != SCPI_RES_OK){
        RP_LOG_CRIT("Failed to reset Red Pitaya DPIO modules: %s",rp_GetError(result))
        return SCPI_RES_ERR;
    }

    result = RP_GenReset(context);

    if(result != SCPI_RES_OK){
        RP_LOG_CRIT("Failed to reset Red Pitaya Gen modules: %s",rp_GetError(result))
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_ReleaseAll(scpi_t *context){   
    auto result = rp_Release();
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to release Red Pitaya modules: %s", rp_GetError(result))
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}


scpi_result_t RP_EnableDigLoop(scpi_t *context){   
    auto result = rp_EnableDigitalLoop(true);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to initialize Red Pitaya digital loop: %s", rp_GetError(result))
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}


scpi_result_t RP_EnableDaisyChainTrigSync(scpi_t *context){
    int32_t value;

    if (!SCPI_ParamChoice(context, scpi_DAISY_state, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_SetEnableDaisyChainTrigSync((bool)value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to enabled mode: %s",rp_GetError(result))
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_EnableDaisyChainTrigSyncQ(scpi_t *context){
    const char *_name;

    bool value;
    auto result = rp_GetEnableDaisyChainTrigSync(&value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed get state: %d", result)
        return SCPI_RES_ERR;
    }

    if(!SCPI_ChoiceToName(scpi_DAISY_state, (int32_t)value, &_name)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Failed to parse state.")
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, _name);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_EnableDaisyChainClockSync(scpi_t *context){
    int32_t value;

    if (!SCPI_ParamChoice(context, scpi_DAISY_state, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.")
        return SCPI_RES_ERR;
    }

    auto result = rp_SetEnableDiasyChainClockSync((bool)value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to enabled mode: %s",rp_GetError(result))
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_EnableDaisyChainClockSyncQ(scpi_t *context){
    const char *_name;

    bool value;
    auto result = rp_GetEnableDiasyChainClockSync(&value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed get state: %s",rp_GetError(result))
        return SCPI_RES_ERR;
    }

    if(!SCPI_ChoiceToName(scpi_DAISY_state, (int32_t)value, &_name)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Failed to parse state.")
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, _name);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_DpinEnableTrigOutput(scpi_t *context){
    int32_t value;

    if (!SCPI_ParamChoice(context, scpi_DAISY_state, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.")
        return SCPI_RES_ERR;
    }

    auto result = rp_SetDpinEnableTrigOutput((bool)value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to enabled mode: %s",rp_GetError(result))
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_DpinEnableTrigOutputQ(scpi_t *context){
    const char *_name;

    bool value;
    auto result = rp_GetDpinEnableTrigOutput(&value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed get state: %s",rp_GetError(result))
        return SCPI_RES_ERR;
    }

    if(!SCPI_ChoiceToName(scpi_DAISY_state, (int32_t)value, &_name)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Failed to parse state.")
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, _name);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SourceTrigOutput(scpi_t *context){
    int32_t value;

    if (!SCPI_ParamChoice(context, scpi_TRIG_O_mode, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"missing first parameter.")
        return SCPI_RES_ERR;
    }

    auto result = rp_SetSourceTrigOutput((rp_outTiggerMode_t)value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set mode: %s",rp_GetError(result))
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SourceTrigOutputQ(scpi_t *context){
    const char *_name;

    rp_outTiggerMode_t value;
    auto result = rp_GetSourceTrigOutput(&value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed get state %s",rp_GetError(result))
        return SCPI_RES_ERR;
    }

    if(!SCPI_ChoiceToName(scpi_TRIG_O_mode, (int32_t)value, &_name)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Failed to parse trigger mode.")
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, _name);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}
