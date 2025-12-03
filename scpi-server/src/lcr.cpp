/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server generate SCPI commands implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "lcr.h"

#include "apiApp/lcrApp.h"
#include "common.h"
#include "rp.h"
#include "rp_hw-profiles.h"
#include "scpi/parser.h"
#include "scpi/units.h"

const scpi_choice_def_t scpi_lcr_shunt[] = {{"S10", RP_LCR_S_10},     {"S100", RP_LCR_S_100}, {"S1k", RP_LCR_S_1k}, {"S10k", RP_LCR_S_10k},
                                            {"S100k", RP_LCR_S_100k}, {"S1M", RP_LCR_S_1M},   SCPI_CHOICE_LIST_END};

const scpi_choice_def_t scpi_lcr_shunt_mode[] = {{"LCR_EXT", RP_LCR_S_EXTENSION}, {"CUSTOM", RP_LCR_S_CUSTOM}, SCPI_CHOICE_LIST_END};

const scpi_choice_def_t scpi_lcr_series_mode[] = {{"SERIES", 1}, {"PARALLEL", 0}, SCPI_CHOICE_LIST_END};

void stopLCR() {
    lcrApp_GenStop();
    lcrApp_LcrStop();
}

scpi_result_t RP_LCRStart(scpi_t* context) {
    stopAllThreads(context);
    auto result = lcrApp_LcrReset();
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed reset LCR api: %s", lcrApp_LcrGetError((lcr_error_t)result));
        return SCPI_RES_ERR;
    }
    result = lcrApp_LcrRun();
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed start LCR api: %s", lcrApp_LcrGetError((lcr_error_t)result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRStartGen(scpi_t* context) {
    auto result = lcrApp_GenRun();
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed start LCR generator api: %s", lcrApp_LcrGetError((lcr_error_t)result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRStop(scpi_t* context) {
    auto result = lcrApp_GenStop();
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed stop generator in LCR api: %s", lcrApp_LcrGetError((lcr_error_t)result));
        return SCPI_RES_ERR;
    }
    result = lcrApp_GenStop();
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed stop LCR service: %s", lcrApp_LcrGetError((lcr_error_t)result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRReset(scpi_t* context) {
    auto result = lcrApp_LcrReset();
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed reset LCR api: %s", lcrApp_LcrGetError((lcr_error_t)result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRMeasureQ(scpi_t* context) {
    lcr_main_data_t values;
    std::string ret_json;
    auto result = lcrApp_LcrCopyParams(&values);
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed get measure: %s", lcrApp_LcrGetError((lcr_error_t)result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    ret_json = "{";
    ret_json += "\"freq\":" + std::to_string(values.lcr_freq);
    ret_json += ",\"amplitude\":" + std::to_string(values.lcr_amplitude);
    ret_json += ",\"phase\":" + std::to_string(values.lcr_phase);
    ret_json += ",\"D\":" + std::to_string(values.lcr_D);
    ret_json += ",\"Q\":" + std::to_string(values.lcr_Q);
    ret_json += ",\"ESR\":" + std::to_string(values.lcr_ESR);
    ret_json += ",\"L\":" + std::to_string(values.lcr_L);
    ret_json += ",\"C\":" + std::to_string(values.lcr_C);
    ret_json += ",\"R\":" + std::to_string(values.lcr_R);
    ret_json += ",\"L_s\":" + std::to_string(values.lcr_L_s);
    ret_json += ",\"C_s\":" + std::to_string(values.lcr_C_s);
    ret_json += ",\"R_s\":" + std::to_string(values.lcr_R_s);
    ret_json += ",\"L_p\":" + std::to_string(values.lcr_L_p);
    ret_json += ",\"C_p\":" + std::to_string(values.lcr_C_p);
    ret_json += ",\"R_p\":" + std::to_string(values.lcr_R_p);
    ret_json += ",\"D_s\":" + std::to_string(values.lcr_D_s);
    ret_json += ",\"Q_s\":" + std::to_string(values.lcr_Q_s);
    ret_json += ",\"D_p\":" + std::to_string(values.lcr_D_p);
    ret_json += ",\"Q_p\":" + std::to_string(values.lcr_Q_p);
    ret_json += ",\"X_s\":" + std::to_string(values.lcr_X_s);
    ret_json += ",\"G_p\":" + std::to_string(values.lcr_G_p);
    ret_json += ",\"B_p\":" + std::to_string(values.lcr_B_p);
    ret_json += ",\"Y_abs\":" + std::to_string(values.lcr_Y_abs);
    ret_json += ",\"Phase_Y\":" + std::to_string(values.lcr_Phase_Y);
    ret_json += "}";
    SCPI_ResultMnemonic(context, ret_json.c_str());
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRFrequency(scpi_t* context) {
    scpi_number_t frequency;
    /* Parse first, FREQUENCY parameter */
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &frequency, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    auto result = lcrApp_LcrSetFrequency(frequency.content.value);
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed to set frequency: %s", lcrApp_LcrGetError((lcr_error_t)result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRFrequencyQ(scpi_t* context) {
    float frequency = 0;
    auto result = lcrApp_LcrGetFrequency(&frequency);
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed to get frequency: %s", lcrApp_LcrGetError((lcr_error_t)result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    /* Return data to client */
    SCPI_ResultFloat(context, frequency);
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRAmplitude(scpi_t* context) {
    scpi_number_t frequency;
    /* Parse first, FREQUENCY parameter */
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &frequency, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    auto result = lcrApp_LcrSetAmplitude(frequency.content.value);
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed to set amplitude: %s", lcrApp_LcrGetError((lcr_error_t)result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRAmplitudeQ(scpi_t* context) {
    float frequency = 0;
    auto result = lcrApp_LcrGetAmplitude(&frequency);
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed to get amplitude: %s", lcrApp_LcrGetError((lcr_error_t)result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    /* Return data to client */
    SCPI_ResultFloat(context, frequency);
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCROffset(scpi_t* context) {
    scpi_number_t frequency;
    /* Parse first, FREQUENCY parameter */
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &frequency, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    auto result = lcrApp_LcrSetOffset(frequency.content.value);
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed to set amplitude: %s", lcrApp_LcrGetError((lcr_error_t)result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCROffsetQ(scpi_t* context) {
    float frequency = 0;
    auto result = lcrApp_LcrGetOffset(&frequency);
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed to get amplitude: %s", lcrApp_LcrGetError((lcr_error_t)result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    /* Return data to client */
    SCPI_ResultFloat(context, frequency);
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRShunt(scpi_t* context) {
    int32_t mode = 0;
    if (!SCPI_ParamChoice(context, scpi_lcr_shunt, &mode, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing shunt parameter.");
        return SCPI_RES_ERR;
    }
    auto result = lcrApp_LcrSetShunt((lcr_shunt_t)mode);
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed to set shunt: %s", lcrApp_LcrGetError((lcr_error_t)result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRShuntQ(scpi_t* context) {
    const char* name = nullptr;
    lcr_shunt_t mode = RP_LCR_S_NOT_INIT;
    auto result = lcrApp_LcrGetShunt(&mode);
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed to get shunt: %s", lcrApp_LcrGetError((lcr_error_t)result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    if (!SCPI_ChoiceToName(scpi_lcr_shunt, (int)mode, &name)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to get shunt name.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, name);
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRCustomShunt(scpi_t* context) {
    scpi_number_t shunt;
    /* Parse first, FREQUENCY parameter */
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &shunt, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    auto result = lcrApp_LcrSetCustomShunt((int)shunt.content.value);
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed to set user shunt: %s", lcrApp_LcrGetError((lcr_error_t)result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRCustomShuntQ(scpi_t* context) {
    int shunt = 0;
    auto result = lcrApp_LcrGetCustomShunt(&shunt);
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed to get user shunt: %s", lcrApp_LcrGetError((lcr_error_t)result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    /* Return data to client */
    SCPI_ResultInt32(context, shunt);
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRShuntMode(scpi_t* context) {
    int32_t mode = 0;
    if (!SCPI_ParamChoice(context, scpi_lcr_shunt_mode, &mode, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing shunt mode parameter.");
        return SCPI_RES_ERR;
    }
    auto result = lcrApp_LcrSetShuntMode((lcr_shunt_mode_t)mode);
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed to set shunt: %s", lcrApp_LcrGetError((lcr_error_t)result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRShuntModeQ(scpi_t* context) {
    const char* name = nullptr;
    lcr_shunt_mode_t mode = RP_LCR_S_CUSTOM;
    auto result = lcrApp_LcrGetShuntMode(&mode);
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed to get shunt: %s", lcrApp_LcrGetError((lcr_error_t)result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    if (!SCPI_ChoiceToName(scpi_lcr_shunt_mode, (int)mode, &name)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to get shunt mode name.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, name);
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRShuntAuto(scpi_t* context) {
    bool state_c = false;
    /* Parse first, STATE argument */
    if (!SCPI_ParamBool(context, &state_c, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    auto result = lcrApp_LcrSetShuntIsAuto(state_c);
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRMeasSeries(scpi_t* context) {
    int32_t mode = 0;
    if (!SCPI_ParamChoice(context, scpi_lcr_series_mode, &mode, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    auto result = lcrApp_LcrSetMeasSeries((bool)mode);
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed to set shunt: %s", lcrApp_LcrGetError((lcr_error_t)result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRMeasSeriesQ(scpi_t* context) {
    const char* name = nullptr;
    bool mode = false;
    auto result = lcrApp_LcrGetMeasSeries(&mode);
    if (result != RP_LCR_OK) {
        RP_LOG_CRIT("Failed to get shunt: %s", lcrApp_LcrGetError((lcr_error_t)result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    if (!SCPI_ChoiceToName(scpi_lcr_series_mode, (int)mode, &name)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to get series name.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, name);
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LCRCheckExtensionModuleConnectioQ(scpi_t* context) {
    bool state = false;
    auto result = lcrApp_LcrIsModuleConnected(&state);
    if (result != RP_OK) {
        RP_LOG_CRIT("Failed to get ext module state: %s", lcrApp_LcrGetError((lcr_error_t)result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    SCPI_ResultBool(context, state);
    RP_LOG_INFO("%s", lcrApp_LcrGetError((lcr_error_t)result))
    return SCPI_RES_OK;
}