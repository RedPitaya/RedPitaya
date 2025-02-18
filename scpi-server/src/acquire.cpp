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

#include "scpi-parser-ext.h"
#include "scpi/parser.h"
#include "scpi/units.h"

rp_scpi_acq_unit_t unit = RP_SCPI_VOLTS;  // default value

const scpi_choice_def_t scpi_RpAcqTrigRequest[] = {{"PRE_TRIG", 1}, {"POST_TRIG", 2}, {"PRE_POST_TRIG", 3}, SCPI_CHOICE_LIST_END};

scpi_result_t RP_AcqSplitTrigger(scpi_t* context) {
    bool state_c;
    /* Parse first, STATE argument */
    if (!SCPI_ParamBool(context, &state_c, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    // Now set the decimation
    auto result = rp_AcqSetSplitTrigger(state_c);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set split trigger: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqSplitTriggerQ(scpi_t* context) {

    bool enabled;

    auto result = rp_AcqGetSplitTrigger(&enabled);
    if (result != RP_OK) {
        RP_LOG_CRIT("Failed to get split trigger: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, enabled);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDataFormat(scpi_t* context) {
    const char* param;
    size_t param_len;

    // read first parameter Format type (BIN, ASCII)
    if (!SCPI_ParamCharacters(context, &param, &param_len, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    if (strncasecmp(param, "BIN", param_len) == 0) {
        user_context_t* uc = (user_context_t*)context->user_context;
        uc->binary_format = true;
        RP_LOG_INFO("Set to BIN");
    } else if (strncasecmp(param, "ASCII", param_len) == 0) {
        user_context_t* uc = (user_context_t*)context->user_context;
        uc->binary_format = false;
        RP_LOG_INFO("Set to ASCII");
    } else {
        RP_LOG_INFO("Wrong argument value");
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDataFormatQ(scpi_t* context) {
    user_context_t* uc = (user_context_t*)context->user_context;
    if (uc->binary_format) {
        SCPI_ResultMnemonic(context, "BIN");
    } else {
        SCPI_ResultMnemonic(context, "ASCII");
    }
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqStart(scpi_t* context) {
    auto result = rp_AcqStart();
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to start Red Pitaya acquire: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqStartCh(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    auto result = rp_AcqStartCh(channel);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to start Red Pitaya acquire: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqStop(scpi_t* context) {
    auto result = rp_AcqStop();
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to stop Red Pitaya acquisition: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t rp_AcqStopCh(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    auto result = rp_AcqStopCh(channel);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to stop Red Pitaya acquisition: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqReset(scpi_t* context) {
    auto result = rp_AcqReset();
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to reset Red Pitaya acquire: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    unit = RP_SCPI_VOLTS;
    user_context_t* uc = (user_context_t*)context->user_context;
    uc->binary_format = false;
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqResetCh(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    auto result = rp_AcqResetCh(channel);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to reset Red Pitaya acquire: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    unit = RP_SCPI_VOLTS;
    user_context_t* uc = (user_context_t*)context->user_context;
    uc->binary_format = false;
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDecimation(scpi_t* context) {

    uint32_t value;

    /* Read DECIMATION parameter */
    if (!SCPI_ParamUInt32(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    // Convert decimation to rp_acq_decimation_t
    rp_acq_decimation_t decimation;
    if (rp_AcqConvertFactorToDecimation(value, &decimation)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Parameter decimation is invalid.");
        return SCPI_RES_ERR;
    }

    // Now set the decimation
    auto result = rp_AcqSetDecimation(decimation);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDecimationCh(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    uint32_t value;

    /* Read DECIMATION parameter */
    if (!SCPI_ParamUInt32(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    // Convert decimation to rp_acq_decimation_t
    rp_acq_decimation_t decimation;
    if (rp_AcqConvertFactorToDecimation(value, &decimation)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Parameter decimation is invalid.");
        return SCPI_RES_ERR;
    }

    // Now set the decimation
    auto result = rp_AcqSetDecimationCh(channel, decimation);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDecimationQ(scpi_t* context) {
    // Get decimation
    rp_acq_decimation_t decimation;
    auto result = rp_AcqGetDecimation(&decimation);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Convert decimation to int
    auto value = (uint32_t)decimation;

    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDecimationChQ(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    // Get decimation
    rp_acq_decimation_t decimation;
    auto result = rp_AcqGetDecimationCh(channel, &decimation);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Convert decimation to int
    auto value = (uint32_t)decimation;

    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDecimationFactor(scpi_t* context) {

    uint32_t value;

    /* Read DECIMATION parameter */
    if (!SCPI_ParamUInt32(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    // Now set the decimation
    auto result = rp_AcqSetDecimationFactor(value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set decimation factor: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDecimationFactorCh(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    uint32_t value;

    /* Read DECIMATION parameter */
    if (!SCPI_ParamUInt32(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    // Now set the decimation
    auto result = rp_AcqSetDecimationFactorCh(channel, value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set decimation factor: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDecimationFactorQ(scpi_t* context) {
    // Get decimation
    u_int32_t decimation;
    auto result = rp_AcqGetDecimationFactor(&decimation);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get decimation factor: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, decimation, 10);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDecimationFactorChQ(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    // Get decimation
    u_int32_t decimation;
    auto result = rp_AcqGetDecimationFactorCh(channel, &decimation);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get decimation factor: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, decimation, 10);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqSamplingRateHzQ(scpi_t* context) {

    // get sampling rate
    float samplingRate;
    auto result = rp_AcqGetSamplingRateHz(&samplingRate);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get sampling rate in Hz: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back string result
    char samplingRateString[20];
    sprintf(samplingRateString, "%9.0f Hz", samplingRate);

    //Return string in form "<Value> Hz"
    SCPI_ResultMnemonic(context, samplingRateString);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqSamplingRateHzChQ(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    // get sampling rate
    float samplingRate;
    auto result = rp_AcqGetSamplingRateHzCh(channel, &samplingRate);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get sampling rate in Hz: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back string result
    char samplingRateString[20];
    sprintf(samplingRateString, "%9.0f Hz", samplingRate);

    //Return string in form "<Value> Hz"
    SCPI_ResultMnemonic(context, samplingRateString);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAveraging(scpi_t* context) {

    scpi_bool_t value;

    // read first parameter AVERAGING (OFF,ON)
    if (!SCPI_ParamBool(context, &value, false)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_AcqSetAveraging(value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set averaging: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAveragingCh(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    scpi_bool_t value;

    // read first parameter AVERAGING (OFF,ON)
    if (!SCPI_ParamBool(context, &value, false)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_AcqSetAveragingCh(channel, value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set averaging: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAveragingQ(scpi_t* context) {
    // get averaging
    bool value;
    auto result = rp_AcqGetAveraging(&value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get averaging: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, value ? "ON" : "OFF");
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAveragingChQ(scpi_t* context) {
    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    // get averaging
    bool value;
    auto result = rp_AcqGetAveragingCh(channel, &value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get averaging: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, value ? "ON" : "OFF");
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerSrc(scpi_t* context) {

    int32_t trig_src;

    /* Read TRIGGER SOURCE parameter */
    if (!SCPI_ParamChoice(context, scpi_RpTrigSrc, &trig_src, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    rp_acq_trig_src_t source = (rp_acq_trig_src_t)trig_src;

    // Now set the trigger source
    int result = rp_AcqSetTriggerSrc(source);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set trigger source: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerSrcCh(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    int32_t trig_src;

    /* Read TRIGGER SOURCE parameter */
    if (!SCPI_ParamChoice(context, scpi_RpTrigSrc, &trig_src, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    rp_acq_trig_src_t source = (rp_acq_trig_src_t)trig_src;

    // Now set the trigger source
    int result = rp_AcqSetTriggerSrcCh(channel, source);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set trigger source: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerStateQ(scpi_t* context) {

    const char* trig_name;
    // get trigger source
    rp_acq_trig_state_t state;
    auto result = rp_AcqGetTriggerState(&state);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get trigger: %s", rp_GetError(result));
        state = RP_TRIG_STATE_WAITING;
    }

    if (!SCPI_ChoiceToName(scpi_RpTrigStat, state, &trig_name)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to parse trigger source.")
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, trig_name);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerStateChQ(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    const char* trig_name;
    // get trigger source
    rp_acq_trig_state_t state;
    auto result = rp_AcqGetTriggerStateCh(channel, &state);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get trigger: %s", rp_GetError(result));
        state = RP_TRIG_STATE_WAITING;  // Some value not equal to DISABLE -> function return "WAIT"
    }

    if (!SCPI_ChoiceToName(scpi_RpTrigStat, state, &trig_name)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to parse trigger source.")
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, trig_name);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerDelay(scpi_t* context) {

    int32_t triggerDelay;

    // read first parameter TRIGGER DELAY (value in samples)
    if (!SCPI_ParamInt32(context, &triggerDelay, false)) {
        triggerDelay = 0;
    }

    // Now set the trigger delay
    auto result = rp_AcqSetTriggerDelay(triggerDelay);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set trigger delay: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerDelayCh(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    int32_t triggerDelay;

    // read first parameter TRIGGER DELAY (value in samples)
    if (!SCPI_ParamInt32(context, &triggerDelay, false)) {
        triggerDelay = 0;
    }

    // Now set the trigger delay
    auto result = rp_AcqSetTriggerDelayCh(channel, triggerDelay);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set trigger delay: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerDelayQ(scpi_t* context) {

    // get trigger delay
    int32_t value;
    auto result = rp_AcqGetTriggerDelay(&value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get trigger delay: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultInt32(context, value);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerDelayChQ(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    // get trigger delay
    int32_t value;
    auto result = rp_AcqGetTriggerDelayCh(channel, &value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get trigger delay: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultInt32(context, value);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerDelayNs(scpi_t* context) {
    int64_t triggerDelay;

    // read first parameter TRIGGER DELAY ns (value in ns)
    if (!SCPI_ParamInt64(context, &triggerDelay, false)) {
        triggerDelay = 0;
    }

    // Now set the trigger delay in ns
    auto result = rp_AcqSetTriggerDelayNs(triggerDelay);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set trigger delay in ns: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerDelayNsCh(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    int64_t triggerDelay;

    // read first parameter TRIGGER DELAY ns (value in ns)
    if (!SCPI_ParamInt64(context, &triggerDelay, false)) {
        triggerDelay = 0;
    }

    // Now set the trigger delay in ns
    auto result = rp_AcqSetTriggerDelayNsCh(channel, triggerDelay);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set trigger delay in ns: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerDelayNsQ(scpi_t* context) {
    // get trigger delay ns
    int64_t value;
    int result = rp_AcqGetTriggerDelayNs(&value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get trigger delay: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultInt32(context, value);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerDelayNsChQ(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    // get trigger delay ns
    int64_t value;
    int result = rp_AcqGetTriggerDelayNsCh(channel, &value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get trigger delay: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultInt32(context, value);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerHyst(scpi_t* context) {
    float voltage;

    if (!SCPI_ParamFloat(context, &voltage, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_AcqSetTriggerHyst(voltage);
    if (result != RP_OK) {
        RP_LOG_CRIT("Failed to set trigger hysteresis: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerHystQ(scpi_t* context) {
    float voltage;
    auto result = rp_AcqGetTriggerHyst(&voltage);
    if (result != RP_OK) {
        RP_LOG_CRIT("Failed to get trigger hysteresis: %s", rp_GetError(result));

        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, voltage);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerFillQ(scpi_t* context) {
    bool fillRes;
    auto result = rp_AcqGetBufferFillState(&fillRes);
    if (result != RP_OK) {
        RP_LOG_CRIT("Failed to get trigger fill state: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultInt32(context, fillRes);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerFillChQ(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    bool fillRes;
    auto result = rp_AcqGetBufferFillStateCh(channel, &fillRes);
    if (result != RP_OK) {
        RP_LOG_CRIT("Failed to get trigger fill state: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultInt32(context, fillRes);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

//Todo: Custom error handling.
scpi_result_t RP_AcqGain(scpi_t* context) {

    const char* name;
    int32_t param;

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    /* Get param val */
    if (!SCPI_ParamChoice(context, scpi_RpGain, &param, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    /* Get param name */
    if (!SCPI_ChoiceToName(scpi_RpGain, param, &name)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Error convert parameter.");
        return SCPI_RES_ERR;
    }

    rp_pinState_t state = (rp_pinState_t)param;
    auto result = rp_AcqSetGain(channel, state);
    if (result != RP_OK) {
        RP_LOG_CRIT("Failed to set gain: %s", &name[0]);
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqGainQ(scpi_t* context) {

    rp_pinState_t state;
    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    auto result = rp_AcqGetGain(channel, &state);
    if (result != RP_OK) {
        RP_LOG_CRIT("Failed to get gain: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    /* Return data to client */
    SCPI_ResultMnemonic(context, state == RP_HIGH ? "HV" : "LV");
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerLevel(scpi_t* context) {
    scpi_number_t value;

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    uint8_t channels = getADCChannels(context);
    // Now set threshold
    auto result = 0;
    if (channels >= 1) {
        result = rp_AcqSetTriggerLevel(RP_T_CH_1, (float)value.content.value);
        if (RP_OK != result) {
            RP_LOG_CRIT("Failed to set CH1 trigger level: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }

    if (channels >= 2) {
        result = rp_AcqSetTriggerLevel(RP_T_CH_2, (float)value.content.value);
        if (RP_OK != result) {
            RP_LOG_CRIT("Failed to set CH2 trigger level: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }

    if (channels >= 3) {
        result = rp_AcqSetTriggerLevel(RP_T_CH_3, (float)value.content.value);
        if (RP_OK != result) {
            RP_LOG_CRIT("Failed to set CH3 trigger level: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }

    if (channels >= 4) {
        result = rp_AcqSetTriggerLevel(RP_T_CH_4, (float)value.content.value);
        if (RP_OK != result) {
            RP_LOG_CRIT("Failed to set CH4 trigger level: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerLevelCh(scpi_t* context) {
    scpi_number_t value;

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_AcqSetTriggerLevel((rp_channel_trigger_t)channel, (float)value.content.value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set trigger level: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerLevelQ(scpi_t* context) {
    float value;
    auto result = rp_AcqGetTriggerLevel(RP_T_CH_1, &value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get trigger level: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultFloat(context, value);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerLevelChQ(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    float value;
    auto result = rp_AcqGetTriggerLevel((rp_channel_trigger_t)channel, &value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get trigger level: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultFloat(context, value);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqWritePointerQ(scpi_t* context) {
    // get write pointer
    uint32_t value;
    auto result = rp_AcqGetWritePointer(&value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get writer position: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqWritePointerChQ(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    // get write pointer
    uint32_t value;
    auto result = rp_AcqGetWritePointerCh(channel, &value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get writer position: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqWritePointerAtTrigQ(scpi_t* context) {
    // get write pointer at trigger
    uint32_t value;
    auto result = rp_AcqGetWritePointerAtTrig(&value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get writer position at trigger: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqWritePointerAtTrigChQ(scpi_t* context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    // get write pointer at trigger
    uint32_t value;
    auto result = rp_AcqGetWritePointerAtTrigCh(channel, &value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get writer position at trigger: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqScpiDataUnits(scpi_t* context) {

    int32_t choice;

    /* Read UNITS parameters */
    if (!SCPI_ParamChoice(context, scpi_RpUnits, &choice, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    /* Set global units for acq scpi */
    unit = (rp_scpi_acq_unit_t)choice;
    RP_LOG_INFO("%s", rp_GetError(RP_OK))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqScpiDataUnitsQ(scpi_t* context) {

    const char* units;

    if (!SCPI_ChoiceToName(scpi_RpUnits, unit, &units)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to get data units.")
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, units);
    RP_LOG_INFO("%s", rp_GetError(RP_OK))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDataPosQ(scpi_t* context) {

    uint32_t start, end;
    auto result = 0;

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    /* Read START parameter */
    if (!SCPI_ParamUInt32(context, &start, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Unable to read START parameter.");
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &end, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Unable to read END parameter.");
        return SCPI_RES_ERR;
    }

    uint32_t size_buff;
    bool error = false;

    rp_AcqGetBufSize(&size_buff);

    uint32_t size = ((end + size_buff) - start) % size_buff + 1;
    if (unit == RP_SCPI_VOLTS) {
        float* buffer = nullptr;
        try {
            buffer = new float[size];
        } catch (const std::bad_alloc&) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed allocate buffer")
            return SCPI_RES_ERR;
        };

        result = rp_AcqGetDataPosV(channel, start, end, buffer, &size);

        if (result != RP_OK) {
            RP_LOG_CRIT("Failed to get data in volts: %s", rp_GetError(result));
            delete[] buffer;
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferFloat(context, buffer, size, &error);
        delete[] buffer;

        if (error) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to send data");
            return SCPI_RES_ERR;
        }
    } else {
        int16_t* buffer = nullptr;
        try {
            buffer = new int16_t[size];
        } catch (const std::bad_alloc&) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed allocate buffer")
            return SCPI_RES_ERR;
        };

        result = rp_AcqGetDataPosRaw(channel, start, end, buffer, &size);

        if (result != RP_OK) {
            RP_LOG_CRIT("Failed to get raw data: %s", rp_GetError(result));
            delete[] buffer;
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferInt16(context, buffer, size, &error);
        delete[] buffer;

        if (error) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to send data");
            return SCPI_RES_ERR;
        }
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDataQ(scpi_t* context) {

    uint32_t start, size;
    int result = 0;

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    /* Parse START parameter */
    if (!SCPI_ParamUInt32(context, &start, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing START parameter.");
        return SCPI_RES_ERR;
    }

    /* Parse SIZE parameter */
    if (!SCPI_ParamUInt32(context, &size, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing SIZE parameter.");
        return SCPI_RES_ERR;
    }

    uint32_t size_buff;
    bool error = false;

    rp_AcqGetBufSize(&size_buff);
    if (unit == RP_SCPI_VOLTS) {
        float* buffer = nullptr;
        try {
            buffer = new float[size];
        } catch (std::bad_alloc&) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed allocate buffer")
            return SCPI_RES_ERR;
        };
        result = rp_AcqGetDataV(channel, start, &size, buffer);
        if (result != RP_OK) {
            RP_LOG_CRIT("Failed to get data in volts: %s", rp_GetError(result));
            delete[] buffer;
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferFloat(context, buffer, size, &error);
        delete[] buffer;
        if (error) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to send data");
            return SCPI_RES_ERR;
        }
    } else {
        int16_t* buffer = nullptr;
        try {
            buffer = new int16_t[size];
        } catch (std::bad_alloc&) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed allocate buffer")
            return SCPI_RES_ERR;
        };
        result = rp_AcqGetDataRaw(channel, start, &size, buffer);

        if (result != RP_OK) {
            RP_LOG_CRIT("Failed to get raw data: %s", rp_GetError(result));
            delete[] buffer;
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferInt16(context, buffer, size, &error);
        delete[] buffer;
        if (error) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to send data");
            return SCPI_RES_ERR;
        }
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqDataOldestAllQ(scpi_t* context) {

    uint32_t size = 0;
    bool error = false;
    auto result = 0;

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    rp_AcqGetBufSize(&size);
    if (unit == RP_SCPI_VOLTS) {
        float buffer[size];
        result = rp_AcqGetOldestDataV(channel, &size, buffer);

        if (result != RP_OK) {
            RP_LOG_CRIT("Failed to get data in volt: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
        SCPI_ResultBufferFloat(context, buffer, size, &error);
        if (error) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to send data");
            return SCPI_RES_ERR;
        }
    } else {
        int16_t buffer[size];
        result = rp_AcqGetOldestDataRaw(channel, &size, buffer);
        if (result != RP_OK) {
            RP_LOG_CRIT("Failed to get raw data: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }
        SCPI_ResultBufferInt16(context, buffer, size, &error);
        if (error) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to send data");
            return SCPI_RES_ERR;
        }
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqOldestDataQ(scpi_t* context) {

    uint32_t size = 0;
    bool error = false;
    auto result = 0;

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &size, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing SIZE parameter.");
        return SCPI_RES_ERR;
    }

    if (unit == RP_SCPI_VOLTS) {
        float buffer[size];
        result = rp_AcqGetOldestDataV(channel, &size, buffer);

        if (result != RP_OK) {
            RP_LOG_CRIT("Failed to get data in volt: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferFloat(context, buffer, size, &error);

        if (error) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to send data");
            return SCPI_RES_ERR;
        }

    } else {
        int16_t buffer[size];
        result = rp_AcqGetOldestDataRaw(channel, &size, buffer);
        if (result != RP_OK) {
            RP_LOG_CRIT("Failed to get raw data: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferInt16(context, buffer, size, &error);

        if (error) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to send data");
            return SCPI_RES_ERR;
        }
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqLatestDataQ(scpi_t* context) {

    uint32_t size = 0;
    auto result = 0;
    bool error = false;

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &size, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    if (unit == RP_SCPI_VOLTS) {
        float buffer[size];
        result = rp_AcqGetLatestDataV(channel, &size, buffer);

        if (result != RP_OK) {
            RP_LOG_CRIT("Failed to get data in volt: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferFloat(context, buffer, size, &error);

        if (error) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to send data");
            return SCPI_RES_ERR;
        }

    } else {
        int16_t buffer[size];
        result = rp_AcqGetLatestDataRaw(channel, &size, buffer);

        if (result != RP_OK) {
            RP_LOG_CRIT("Failed to get raw data: %s", rp_GetError(result));
        }

        SCPI_ResultBufferInt16(context, buffer, size, &error);

        if (error) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to send data");
            return SCPI_RES_ERR;
        }
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqTriggerDataQ(scpi_t* context) {

    uint32_t count;
    int result;

    rp_channel_t channel;
    int32_t trig_request;
    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }
    /* Parse count samples parameter */
    if (!SCPI_ParamUInt32(context, &count, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing Size parameter.");
        return SCPI_RES_ERR;
    }

    /* Parse MODE parameter */
    if (!SCPI_ParamChoice(context, scpi_RpAcqTrigRequest, &trig_request, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing trigger mode parameter.");
        return SCPI_RES_ERR;
    }

    uint32_t trig_pos;

    if (rp_HPGetFastADCIsSplitTriggerOrDefault()) {
        result = rp_AcqGetWritePointerAtTrigCh(channel, &trig_pos);
    } else {
        result = rp_AcqGetWritePointerAtTrig(&trig_pos);
    }

    if (result != RP_OK) {
        RP_LOG_CRIT("Failed to get trigger position: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    uint32_t size_buff;
    rp_AcqGetBufSize(&size_buff);

    uint32_t data_start = 0;
    uint32_t data_size = 0;
    bool error = false;

    switch (trig_request) {
        case 1:  // Pre trigger mode
            data_start = (ADC_BUFFER_SIZE + trig_pos - count) % ADC_BUFFER_SIZE;
            data_size = count + 1;
            break;
        case 2:  // Post trigger mode
            data_start = trig_pos;
            data_size = count + 1;
            break;
        case 3:  // Pre-Post trigger mode
            data_start = (ADC_BUFFER_SIZE + trig_pos - count) % ADC_BUFFER_SIZE;
            data_size = count * 2 + 1;
            break;

        default:
            RP_LOG_CRIT("Undefined trigger mode: %d", trig_request);
            return SCPI_RES_ERR;
    }
    if (unit == RP_SCPI_VOLTS) {
        float* buffer = nullptr;
        try {
            buffer = new float[data_size];
        } catch (std::bad_alloc&) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed allocate buffer")
            return SCPI_RES_ERR;
        };
        result = rp_AcqGetDataV(channel, data_start, &data_size, buffer);
        if (result != RP_OK) {
            RP_LOG_CRIT("Failed to get data in volts: %s", rp_GetError(result));
            delete[] buffer;
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferFloat(context, buffer, data_size, &error);
        delete[] buffer;

        if (error) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to send data");
            return SCPI_RES_ERR;
        }

    } else {
        int16_t* buffer = nullptr;
        try {
            buffer = new int16_t[data_size];
        } catch (std::bad_alloc&) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed allocate buffer")
            return SCPI_RES_ERR;
        };
        result = rp_AcqGetDataRaw(channel, data_start, &data_size, buffer);

        if (result != RP_OK) {
            RP_LOG_CRIT("Failed to get raw data: %s", rp_GetError(result));
            delete[] buffer;
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferInt16(context, buffer, data_size, &error);
        delete[] buffer;

        if (error) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to send data");
            return SCPI_RES_ERR;
        }
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqBufferSizeQ(scpi_t* context) {
    uint32_t size;
    auto result = rp_AcqGetBufSize(&size);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get buffer size: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, size, 10);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAC_DC(scpi_t* context) {
    const char* name;
    int32_t param;

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    /* Get param val */
    if (!SCPI_ParamChoice(context, scpi_RpAC_DC, &param, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.")
        return SCPI_RES_ERR;
    }

    /* Get param name */
    if (!SCPI_ChoiceToName(scpi_RpAC_DC, param, &name)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Error convert parameter.")
        return SCPI_RES_ERR;
    }

    rp_acq_ac_dc_mode_t state = (rp_acq_ac_dc_mode_t)param;
    auto result = rp_AcqSetAC_DC(channel, state);
    if (result != RP_OK) {
        RP_LOG_CRIT("Failed to set AC/DC mode: %s", &name[0]);
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAC_DCQ(scpi_t* context) {
    rp_acq_ac_dc_mode_t state;
    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }

    auto result = rp_AcqGetAC_DC(channel, &state);
    if (result != RP_OK) {
        RP_LOG_CRIT("Failed to get gain: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    /* Return data to client */
    SCPI_ResultMnemonic(context, state == RP_DC ? "DC" : "AC");
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_ExtTriggerDebouncerUs(scpi_t* context) {
    scpi_number_t value;

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_AcqSetExtTriggerDebouncerUs((double)value.content.value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_ExtTriggerDebouncerUsQ(scpi_t* context) {
    double value;
    auto result = rp_AcqGetExtTriggerDebouncerUs(&value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    SCPI_ResultDouble(context, value);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}