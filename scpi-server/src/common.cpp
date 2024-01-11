/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server utils module implementation
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
#include <stdarg.h>
#include <string>
#include <math.h>
#include <time.h>

#include "common.h"
#include "error.h"

const scpi_choice_def_t scpi_RpLogMode[] = {
    {"OFF", RP_SCPI_LOG_OFF},
    {"CONSOLE", RP_SCPI_LOG_CONSOLE},
    {"SYSLOG", RP_SCPI_LOG_SYSLOG},
    SCPI_CHOICE_LIST_END
};

rp_scpi_log g_logMode = RP_SCPI_LOG_OFF;


/* Parse channel */
int RP_ParseChArgvADC(scpi_t *context, rp_channel_t *channel){

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

int RP_ParseChArgvDAC(scpi_t *context, rp_channel_t *channel){

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

scpi_result_t RP_SetLogMode(scpi_t *context) {

    int32_t choice;

    /* Read UNITS parameters */
    if(!SCPI_ParamChoice(context, scpi_RpLogMode, &choice, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    /* Set global units for acq scpi */
    g_logMode = (rp_scpi_log)choice;

    RP_LOG_INFO("Successfully set scpi units.");
    return SCPI_RES_OK;
}

rp_scpi_log getLogMode(){
    return g_logMode;
}

auto rp_Log(scpi_t *context,int mode, int rp_err_code, const char * format, ...) -> void{
    va_list args;
    va_start (args, format);
    char logMsg[255];
    vsnprintf(logMsg,255,format,args);
    va_end (args);

    if (getLogMode() == RP_SCPI_LOG_SYSLOG)
        syslog(mode, "%s", logMsg);
    if (getLogMode() == RP_SCPI_LOG_CONSOLE)
        fprintf(stdout, "%s\n", logMsg);

    if (mode <= LOG_ERR && context){
        rp_error_t err;
        err.baseCode = mode < LOG_ERR ? RP_ERR_CODE_FATAL : RP_ERR_CODE;
        err.errorCode = rp_err_code;
        err.msg = logMsg;
        rp_errorPush(context,err);
    }
}

auto scpi_Log(scpi_t *context,int mode, int err_code, const char * format, ...) -> void{
    va_list args;
    va_start (args, format);
    char logMsg[255];
    vsnprintf(logMsg,255,format,args);
    va_end (args);

    if (getLogMode() == RP_SCPI_LOG_SYSLOG)
        syslog(mode, "%s", logMsg);
    if (getLogMode() == RP_SCPI_LOG_CONSOLE)
        fprintf(stdout, "%s\n", logMsg);

    if (mode <= LOG_ERR && context){
        rp_error_t err;
        err.baseCode = 0;
        err.errorCode = err_code;
        err.msg = logMsg;
        rp_errorPush(context,err);
    }
}

uint8_t getADCChannels(scpi_t *context){
    uint8_t c = 0;
    auto result = rp_HPGetFastADCChannelsCount(&c);
    if (result != RP_HP_OK){
        RP_LOG_CRIT("Can't get fast ADC channels count");
    }
    return c;
}

uint8_t getDACChannels(scpi_t *context){
    uint8_t c = 0;
    auto result = rp_HPGetFastDACChannelsCount(&c);
    if (result != RP_HP_OK){
        RP_LOG_CRIT("Can't get fast DAC channels count");
    }
    return c;
}

uint32_t getDACRate(scpi_t *context){
    uint32_t c = 0;
    auto result = rp_HPGetBaseFastDACSpeedHz(&c);
    if (result != RP_HP_OK){
        RP_LOG_CRIT("Can't get fast DAC channels count");
    }
    return c;
}

uint32_t getADCRate(scpi_t *context){
    uint32_t c = 0;
    auto result = rp_HPGetBaseFastADCSpeedHz(&c);
    if (result != RP_HP_OK){
        RP_LOG_CRIT("Can't get fast ADC channels count");
    }
    return c;
}

rp_HPeModels_t getModel(scpi_t *context){
    rp_HPeModels_t c = STEM_125_14_v1_0;
    auto result = rp_HPGetModel(&c);
    if (result != RP_HP_OK){
        RP_LOG_CRIT("Can't get board model");
    }
    return c;
}

scpi_result_t RP_Time(scpi_t *context){
    uint32_t hh, mm, ss;

    if(!SCPI_ParamUInt32(context, &hh, true)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Unable to read HOURS parameter.")
        return SCPI_RES_ERR;
    }else{
        if (hh > 23){
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Invalid value for the HOURS. Parameter must be between 0 and 23.")
            return SCPI_RES_ERR;
        }
    }

    if(!SCPI_ParamUInt32(context, &mm, true)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Unable to read MINUTES parameter.")
        return SCPI_RES_ERR;
    }else{
        if (mm > 59){
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Invalid value for the MINUTES. Parameter must be between 0 and 59.")
            return SCPI_RES_ERR;
        }
    }

    if(!SCPI_ParamUInt32(context, &ss, true)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Unable to read SECONDS parameter.")
        return SCPI_RES_ERR;
    }else{
        if (ss > 59){
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Invalid value for the SECONDS. Parameter must be between 0 and 59.")
            return SCPI_RES_ERR;
        }
    }


    struct timespec t_time;
    if (clock_gettime (CLOCK_REALTIME, & t_time)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Error getting current time.");
        return SCPI_RES_ERR;
    }
    time_t t_t = (time_t)t_time.tv_sec;
    struct tm *time = gmtime(&t_t);

    time->tm_hour = hh;
    time->tm_min  = mm;
    time->tm_sec  = ss;

    time_t t = mktime(time);
    if (t != (time_t)(-1)){
        struct timespec new_time = {t,0};
        if (clock_settime(CLOCK_REALTIME, &new_time)){
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Error setting new time.");
            return SCPI_RES_ERR;
        }
    }else{
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"New time conversion error.");
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("Successfully set time.");
    return SCPI_RES_OK;
}

scpi_result_t RP_TimeQ(scpi_t *context){

    struct timespec t_time;
    if (clock_gettime (CLOCK_REALTIME, & t_time)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Error getting current time.");
        return SCPI_RES_ERR;
    }

    struct tm *time = gmtime(&t_time.tv_sec);

    char buff[10];
    sprintf(buff,"%02d:%02d:%02d",time->tm_hour,time->tm_min,time->tm_sec);
    // Return back result
    SCPI_ResultMnemonic(context, buff);
    RP_LOG_INFO("Successfully returned time.");
    return SCPI_RES_OK;
}


scpi_result_t RP_Date(scpi_t *context){
    uint32_t year, m, d;

    if(!SCPI_ParamUInt32(context, &year, true)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Unable to read YEAR parameter.");
        return SCPI_RES_ERR;
    }else{
        if (year < 1900){
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Invalid value for the YEAR. The value must be greater 1900.");
            return SCPI_RES_ERR;
        }
    }

    if(!SCPI_ParamUInt32(context, &m, true)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Unable to read MONTH parameter.");
        return SCPI_RES_ERR;
    }else{
        if (m < 1 || m > 12){
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Invalid value for the MONTH. Parameter must be between 1 and 12.");
            return SCPI_RES_ERR;
        }
    }

    if(!SCPI_ParamUInt32(context, &d, true)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Unable to read DAY parameter.");
        return SCPI_RES_ERR;
    }else{
        if (d < 1 || d > 31){
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Invalid value for the DAY. Parameter must be between 1 and 31.");
            return SCPI_RES_ERR;
        }
    }


    struct timespec t_time;
    if (clock_gettime (CLOCK_REALTIME, & t_time)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Error getting current date.");
        return SCPI_RES_ERR;
    }
    time_t t_t = (time_t)t_time.tv_sec;
    struct tm *time = gmtime(&t_t);

    time->tm_year = year - 1900;
    time->tm_mon  = m - 1;
    time->tm_mday = d;

    time_t t = mktime(time);
    if (t != (time_t)(-1)){
        struct timespec new_time = {t,0};
        if (clock_settime(CLOCK_REALTIME, &new_time)){
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Error setting new date.");
            return SCPI_RES_ERR;
        }
    }else{
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"New date conversion error.");
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("Successfully set date.");
    return SCPI_RES_OK;
}

scpi_result_t RP_DateQ(scpi_t *context){

    struct timespec t_time;
    if (clock_gettime (CLOCK_REALTIME, & t_time)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Error getting current date.");
        return SCPI_RES_ERR;
    }

    struct tm *time = gmtime(&t_time.tv_sec);

    char buff[40];
    sprintf(buff,"%d-%02d-%02d",time->tm_year + 1900,time->tm_mon + 1,time->tm_mday);
    // Return back result
    SCPI_ResultMnemonic(context, buff);

    RP_LOG_INFO("Successfully returned date.");
    return SCPI_RES_OK;
}

scpi_result_t RP_BoardID(scpi_t *context){
    rp_HPeModels_t model;
    auto result = rp_HPGetModel(&model);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get board model: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    SCPI_ResultUInt32Base(context, (uint32_t)model, 10);

    RP_LOG_INFO("Successfully returned board model.");
    return SCPI_RES_OK;
}

scpi_result_t RP_BoardName(scpi_t *context){
    char *boardName;
    auto result = rp_HPGetModelName(&boardName);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get board name: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, boardName);

    RP_LOG_INFO("Successfully returned board name.");
    return SCPI_RES_OK;
}

auto getCmdName(scpi_t *context) -> const char *{
    size_t buff_len = 100;
    static char buff[100];
    buff_len = context->param_list.cmd_raw.length <  buff_len ? context->param_list.cmd_raw.length :  buff_len;
    strncpy(buff, context->param_list.cmd_raw.data, buff_len);
    buff[buff_len] = '\0';
    return buff;
}