/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server utils module implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <string>

#include "common.h"
#include "common/rp_sweep.h"
#include "error.h"
#include "lcr.h"
#include "scpi/parser.h"
#include "scpi/units.h"

const scpi_choice_def_t scpi_RpLogMode[] = {{"OFF", RP_SCPI_LOG_OFF}, {"CONSOLE", RP_SCPI_LOG_CONSOLE}, {"SYSLOG", RP_SCPI_LOG_SYSLOG}, SCPI_CHOICE_LIST_END};

rp_scpi_log g_logMode = RP_SCPI_LOG_OFF;
bool g_retByError = false;

/* Parse channel */
int RP_ParseChArgvADC(scpi_t* context, rp_channel_t* channel) {

    int32_t ch_usr[1];
    int result = 0;
    SCPI_CommandNumbers(context, ch_usr, 1, SCPI_CMD_NUM);
    if (!((ch_usr[0] > 0) && (ch_usr[0] <= getADCChannels(context)))) {
        RP_LOG_CRIT("Invalid channel number");
        return RP_EOOR;
    }
    *channel = (rp_channel_t)(ch_usr[0] - 1);

    return RP_OK;
}

int RP_ParseChArgvDAC(scpi_t* context, rp_channel_t* channel) {

    int32_t ch_usr[1];
    int result = 0;
    SCPI_CommandNumbers(context, ch_usr, 1, SCPI_CMD_NUM);
    if (!((ch_usr[0] > 0) && (ch_usr[0] <= getDACChannels(context)))) {
        RP_LOG_CRIT("Invalid channel number");
        return RP_EOOR;
    }
    *channel = (rp_channel_t)(ch_usr[0] - 1);

    return RP_OK;
}

scpi_result_t RP_SetLogMode(scpi_t* context) {

    int32_t choice;

    /* Read UNITS parameters */
    if (!SCPI_ParamChoice(context, scpi_RpLogMode, &choice, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    /* Set global units for acq scpi */
    g_logMode = (rp_scpi_log)choice;

    RP_LOG_INFO("Successfully set log mode %d.", g_logMode);
    return SCPI_RES_OK;
}

scpi_result_t RP_SetRetOnError(scpi_t* context) {

    scpi_bool_t value = FALSE;

    if (!SCPI_ParamBool(context, &value, false)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    g_retByError = value;

    RP_LOG_INFO("Successfully set return on error %d.", g_retByError);
    return SCPI_RES_OK;
}

bool getRetOnError() {
    return g_retByError;
}

rp_scpi_log getLogMode() {
    return g_logMode;
}

auto rp_Log(scpi_t* context, int mode, int rp_err_code, const char* format, ...) -> void {
    va_list args;
    va_start(args, format);
    char logMsg[255];
    vsnprintf(logMsg, 255, format, args);
    va_end(args);

    if (getLogMode() == RP_SCPI_LOG_SYSLOG)
        syslog(mode, "%s", logMsg);
    if (getLogMode() == RP_SCPI_LOG_CONSOLE)
        fprintf(stdout, "%s\n", logMsg);

    if (mode <= LOG_ERR && context) {
        rp_error_t err;
        err.baseCode = mode < LOG_ERR ? RP_ERR_CODE_FATAL : RP_ERR_CODE;
        err.errorCode = rp_err_code;
        err.msg = logMsg;
        rp_errorPush(context, err);
    }
}

auto scpi_Log(scpi_t* context, int mode, int err_code, const char* format, ...) -> void {
    va_list args;
    va_start(args, format);
    char logMsg[255];
    vsnprintf(logMsg, 255, format, args);
    va_end(args);

    if (getLogMode() == RP_SCPI_LOG_SYSLOG)
        syslog(mode, "%s", logMsg);
    if (getLogMode() == RP_SCPI_LOG_CONSOLE)
        fprintf(stdout, "%s\n", logMsg);

    if (mode <= LOG_ERR && context) {
        rp_error_t err;
        err.baseCode = 0;
        err.errorCode = err_code;
        err.msg = logMsg;
        rp_errorPush(context, err);
    }
}

uint8_t getADCChannels(scpi_t* context) {
    uint8_t c = 0;
    auto result = rp_HPGetFastADCChannelsCount(&c);
    if (result != RP_HP_OK) {
        RP_LOG_CRIT("Can't get fast ADC channels count");
    }
    return c;
}

uint8_t getDACChannels(scpi_t* context) {
    uint8_t c = 0;
    auto result = rp_HPGetFastDACChannelsCount(&c);
    if (result != RP_HP_OK) {
        RP_LOG_CRIT("Can't get fast DAC channels count");
    }
    return c;
}

uint32_t getDACRate(scpi_t* context) {
    uint32_t c = 0;
    auto result = rp_HPGetBaseFastDACSpeedHz(&c);
    if (result != RP_HP_OK) {
        RP_LOG_CRIT("Can't get fast DAC channels count");
    }
    return c;
}

uint32_t getADCRate(scpi_t* context) {
    uint32_t c = 0;
    auto result = rp_HPGetBaseFastADCSpeedHz(&c);
    if (result != RP_HP_OK) {
        RP_LOG_CRIT("Can't get fast ADC channels count");
    }
    return c;
}

rp_HPeModels_t getModel(scpi_t* context) {
    rp_HPeModels_t c = STEM_125_14_v1_0;
    auto result = rp_HPGetModel(&c);
    if (result != RP_HP_OK) {
        RP_LOG_CRIT("Can't get board model");
    }
    return c;
}

scpi_result_t RP_Time(scpi_t* context) {
    uint32_t hh, mm, ss;
    hh = UINT32_MAX;
    mm = UINT32_MAX;
    ss = UINT32_MAX;
    const char* buffer;
    size_t len = 0;

    if (SCPI_ParamCharacters(context, &buffer, &len, true)) {
        if (len > 0) {
            sscanf(buffer, "%d:%d:%d", &hh, &mm, &ss);

            if (hh > 23) {
                SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Invalid value for the HOURS. Parameter must be between 0 and 23.")
                return SCPI_RES_ERR;
            }

            if (mm > 59) {
                SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Invalid value for the MINUTES. Parameter must be between 0 and 59.")
                return SCPI_RES_ERR;
            }

            if (ss > 59) {
                SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Invalid value for the SECONDS. Parameter must be between 0 and 59.")
                return SCPI_RES_ERR;
            }
        }
    } else {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Unable to read parameter.")
        return SCPI_RES_ERR;
    }

    time_t rawtime;
    time(&rawtime);
    struct tm* time = localtime(&rawtime);

    time->tm_hour = hh;
    time->tm_min = mm;
    time->tm_sec = ss;

    time_t t = mktime(time);
    if (t != (time_t)(-1)) {
        struct timeval new_time = {t, 0};
        if (settimeofday(&new_time, nullptr) == -1) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Error setting new time.");
            return SCPI_RES_ERR;
        }
    } else {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "New time conversion error.");
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("Successfully set time.");
    return SCPI_RES_OK;
}

scpi_result_t RP_TimeQ(scpi_t* context) {

    time_t rawtime;
    time(&rawtime);
    struct tm* time = localtime(&rawtime);

    char buff[10];
    sprintf(buff, "%02d:%02d:%02d", time->tm_hour, time->tm_min, time->tm_sec);
    // Return back result
    SCPI_ResultMnemonic(context, buff);
    RP_LOG_INFO("Successfully returned time.");
    return SCPI_RES_OK;
}

scpi_result_t RP_Date(scpi_t* context) {
    uint32_t y, m, d;
    y = UINT32_MAX;
    m = UINT32_MAX;
    d = UINT32_MAX;
    const char* buffer;
    size_t len = 0;

    if (SCPI_ParamCharacters(context, &buffer, &len, true)) {
        if (len > 0) {
            sscanf(buffer, "%d-%d-%d", &y, &m, &d);

            if (y < 1900 || y > 3000) {
                SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Invalid value for the YEAR. The value must be between 1900 - 3000.");
                return SCPI_RES_ERR;
            }

            if (m < 1 || m > 12) {
                SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Invalid value for the MONTH. Parameter must be between 1 and 12.");
                return SCPI_RES_ERR;
            }

            if (d < 1 || d > 31) {
                SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Invalid value for the DAY. Parameter must be between 1 and 31.");
                return SCPI_RES_ERR;
            }
        }
    } else {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Unable to read parameter.")
        return SCPI_RES_ERR;
    }

    time_t rawtime;
    time(&rawtime);
    struct tm* time = localtime(&rawtime);

    time->tm_year = y - 1900;
    time->tm_mon = m - 1;
    time->tm_mday = d;

    time_t t = mktime(time);
    if (t != (time_t)(-1)) {
        struct timeval new_time = {t, 0};
        if (settimeofday(&new_time, nullptr) == -1) {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Error setting new date.");
            return SCPI_RES_ERR;
        }
    } else {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "New date conversion error.");
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("Successfully set date.");
    return SCPI_RES_OK;
}

scpi_result_t RP_DateQ(scpi_t* context) {

    time_t rawtime;
    time(&rawtime);
    struct tm* time = localtime(&rawtime);

    char buff[40];
    sprintf(buff, "%d-%02d-%02d", time->tm_year + 1900, time->tm_mon + 1, time->tm_mday);
    // Return back result
    SCPI_ResultMnemonic(context, buff);

    RP_LOG_INFO("Successfully returned date.");
    return SCPI_RES_OK;
}

scpi_result_t RP_BoardID(scpi_t* context) {
    rp_HPeModels_t model;
    auto result = rp_HPGetModel(&model);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get board model: %s", rp_GetError(result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    SCPI_ResultUInt32Base(context, (uint32_t)model, 10);
    RP_LOG_INFO("Successfully returned board model.");
    return SCPI_RES_OK;
}

scpi_result_t RP_BoardName(scpi_t* context) {
    char* boardName = nullptr;
    auto result = rp_HPGetModelName(&boardName);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get board name: %s", rp_GetError(result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, boardName);
    RP_LOG_INFO("Successfully returned board name.");
    return SCPI_RES_OK;
}

auto getCmdName(scpi_t* context) -> const char* {
    size_t buff_len = 100;
    static char buff[100];
    buff_len = context->param_list.cmd_raw.length < buff_len ? context->param_list.cmd_raw.length : buff_len;
    strncpy(buff, context->param_list.cmd_raw.data, buff_len);
    buff[buff_len] = '\0';
    return buff;
}

scpi_result_t RP_ExtTriggerLevel(scpi_t* context) {
    scpi_number_t value;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    int result = 0;
    result = rp_SetExternalTriggerLevel((float)value.content.value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set trigger level: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_ExtTriggerLevelQ(scpi_t* context) {
    float value = 0;
    auto result = rp_GetExternalTriggerLevel(&value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get trigger level: %s", rp_GetError(result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    SCPI_ResultFloat(context, value);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

void requestSendNewLine(scpi_t* context) {
    if (context) {
        context->first_output = FALSE;
    }
}

void stopAllThreads(scpi_t* context) {
    rp_sweep_api::rp_SWStop();
    RP_LOG_INFO("%s", "Stopping the sweep generation service")
    stopLCR();
    RP_LOG_INFO("%s", "Stopping the lcr service")
}
