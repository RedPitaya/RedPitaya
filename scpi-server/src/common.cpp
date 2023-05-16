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
    SCPI_CommandNumbers(context, ch_usr, 1, SCPI_CMD_NUM);
    if (!((ch_usr[0] > 0) && (ch_usr[0] <= getADCChannels(context)))) {
        RP_LOG(context,LOG_ERR, "ERROR: Invalid channel number: %.*s", 50, context->param_list.cmd_raw.data);
        return RP_EOOR;
    }
    *channel = (rp_channel_t)(ch_usr[0] - 1);

    return RP_OK;
}

int RP_ParseChArgvDAC(scpi_t *context, rp_channel_t *channel){

    int32_t ch_usr[1];
    SCPI_CommandNumbers(context, ch_usr, 1, SCPI_CMD_NUM);
    if (!((ch_usr[0] > 0) && (ch_usr[0] <= getDACChannels(context)))) {
        RP_LOG(context,LOG_ERR, "ERROR: Invalid channel number: %.*s", 50, context->param_list.cmd_raw.data);
        return RP_EOOR;
    }
    *channel = (rp_channel_t)(ch_usr[0] - 1);

    return RP_OK;
}

scpi_result_t RP_SetLogMode(scpi_t *context) {

    int32_t choice;

    /* Read UNITS parameters */
    if(!SCPI_ParamChoice(context, scpi_RpLogMode, &choice, true)){
        RP_LOG(context,LOG_ERR, "*RP:LOGmode Missing first parameter.");
        return SCPI_RES_ERR;
    }

    /* Set global units for acq scpi */
    g_logMode = (rp_scpi_log)choice;

    RP_LOG(context,LOG_INFO, "*RP:LOGmode Successfully set scpi units.");
    return SCPI_RES_OK;
}

rp_scpi_log getLogMode(){
    return g_logMode;
}

void RP_LOG(scpi_t *context, int mode, const char * format,...){
    va_list args;
    va_start (args, format);
    char logMsg[255];
    vsprintf(logMsg,format,args);
    va_end (args);

    if (getLogMode() == RP_SCPI_LOG_SYSLOG)
        syslog(mode, "%s", logMsg);
    if (getLogMode() == RP_SCPI_LOG_CONSOLE)
        fprintf(stdout, "%s", logMsg);

    if (mode == LOG_ERR && context){
        rp_errorPush(context,logMsg);
    }


}

uint8_t getADCChannels(scpi_t *context){
    uint8_t c = 0;
    if (rp_HPGetFastADCChannelsCount(&c) != RP_HP_OK){
        RP_LOG(context,LOG_ERR,"[Error] Can't get fast ADC channels count");
    }
    return c;
}

uint8_t getDACChannels(scpi_t *context){
    uint8_t c = 0;

    if (rp_HPGetFastDACChannelsCount(&c) != RP_HP_OK){
        RP_LOG(context,LOG_ERR,"[Error] Can't get fast DAC channels count");
    }
    return c;
}

uint32_t getDACRate(scpi_t *context){
    uint32_t c = 0;
    if (rp_HPGetBaseFastDACSpeedHz(&c) != RP_HP_OK){
        RP_LOG(context,LOG_ERR,"[Error] Can't get fast DAC channels count");
    }
    return c;
}

uint32_t getADCRate(scpi_t *context){
    uint32_t c = 0;
    if (rp_HPGetBaseFastADCSpeedHz(&c) != RP_HP_OK){
        RP_LOG(context,LOG_ERR,"[Error] Can't get fast ADC channels count");
    }
    return c;
}

rp_HPeModels_t getModel(scpi_t *context){
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK){
        RP_LOG(context,LOG_ERR,"[Error] Can't get board model");
    }
    return c;
}

scpi_result_t RP_Time(scpi_t *context){
    uint32_t hh, mm, ss;

    if(!SCPI_ParamUInt32(context, &hh, true)){
        RP_LOG(context,LOG_ERR, "*SYSTem:TIME Unable to read HOURS parameter.");
        return SCPI_RES_ERR;
    }else{
        if (hh > 23){
            RP_LOG(context,LOG_ERR, "*SYSTem:TIME Invalid value for the HOURS. Parameter must be between 0 and 23.");
            return SCPI_RES_ERR;
        }
    }

    if(!SCPI_ParamUInt32(context, &mm, true)){
        RP_LOG(context,LOG_ERR, "*SYSTem:TIME Unable to read MINUTES parameter.");
        return SCPI_RES_ERR;
    }else{
        if (mm > 59){
            RP_LOG(context,LOG_ERR, "*SYSTem:TIME Invalid value for the MINUTES. Parameter must be between 0 and 23.");
            return SCPI_RES_ERR;
        }
    }

    if(!SCPI_ParamUInt32(context, &ss, true)){
        RP_LOG(context,LOG_ERR, "*SYSTem:TIME Unable to read SECONDS parameter.");
        return SCPI_RES_ERR;
    }else{
        if (ss > 59){
            RP_LOG(context,LOG_ERR, "*SYSTem:TIME Invalid value for the SECONDS. Parameter must be between 0 and 23.");
            return SCPI_RES_ERR;
        }
    }


    struct timespec t_time;
    if (clock_gettime (CLOCK_REALTIME, & t_time)){
        RP_LOG(context,LOG_ERR, "*SYSTem:TIME Error getting current time.");
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
        RP_LOG(context,LOG_ERR, "*SYSTem:TIME Error setting new time.");
            return SCPI_RES_ERR;
        }
    }else{
        RP_LOG(context,LOG_ERR, "*SYSTem:TIME New time conversion error.");
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SYSTem:TIME Successfully set time.");
    return SCPI_RES_OK;
}

scpi_result_t RP_TimeQ(scpi_t *context){

    struct timespec t_time;
    if (clock_gettime (CLOCK_REALTIME, & t_time)){
        RP_LOG(context,LOG_ERR, "*SYSTem:TIME? Error getting current time.");
        return SCPI_RES_ERR;
    }

    struct tm *time = gmtime(&t_time.tv_sec);

    char buff[10];
    sprintf(buff,"%d,%d,%d",time->tm_hour,time->tm_min,time->tm_sec);
    // Return back result
    SCPI_ResultMnemonic(context, buff);


    RP_LOG(context,LOG_INFO, "*SYSTem:TIME? Successfully returned time.");
    return SCPI_RES_OK;
}


scpi_result_t RP_Date(scpi_t *context){
    uint32_t year, m, d;

    if(!SCPI_ParamUInt32(context, &year, true)){
        RP_LOG(context,LOG_ERR, "*SYSTem:DATE Unable to read YEAR parameter.");
        return SCPI_RES_ERR;
    }else{
        if (year < 1900){
            RP_LOG(context,LOG_ERR, "*SYSTem:DATE Invalid value for the YEAR. The value must be greater 1900.");
            return SCPI_RES_ERR;
        }
    }

    if(!SCPI_ParamUInt32(context, &m, true)){
        RP_LOG(context,LOG_ERR, "*SYSTem:DATE Unable to read MONTH parameter.");
        return SCPI_RES_ERR;
    }else{
        if (m < 1 || m > 12){
            RP_LOG(context,LOG_ERR, "*SYSTem:DATE Invalid value for the MONTH. Parameter must be between 1 and 12.");
            return SCPI_RES_ERR;
        }
    }

    if(!SCPI_ParamUInt32(context, &d, true)){
        RP_LOG(context,LOG_ERR, "*SYSTem:DATE Unable to read DAY parameter.");
        return SCPI_RES_ERR;
    }else{
        if (d < 1 || d > 31){
            RP_LOG(context,LOG_ERR, "*SYSTem:DATE Invalid value for the DAY. Parameter must be between 1 and 31.");
            return SCPI_RES_ERR;
        }
    }


    struct timespec t_time;
    if (clock_gettime (CLOCK_REALTIME, & t_time)){
        RP_LOG(context,LOG_ERR, "*SYSTem:DATE Error getting current date.");
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
            RP_LOG(context,LOG_ERR, "*SYSTem:DATE Error setting new date.");
            return SCPI_RES_ERR;
        }
    }else{
        RP_LOG(context,LOG_ERR, "*SYSTem:DATE New date conversion error.");
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SYSTem:DATE Successfully set date.");
    return SCPI_RES_OK;
}

scpi_result_t RP_DateQ(scpi_t *context){

    struct timespec t_time;
    if (clock_gettime (CLOCK_REALTIME, & t_time)){
        RP_LOG(context,LOG_ERR, "*SYSTem:DATE? Error getting current date.");
        return SCPI_RES_ERR;
    }

    struct tm *time = gmtime(&t_time.tv_sec);

    char buff[40];
    sprintf(buff,"%d,%d,%d",time->tm_year + 1900,time->tm_mon + 1,time->tm_mday);
    // Return back result
    SCPI_ResultMnemonic(context, buff);


    RP_LOG(context,LOG_INFO, "*SYSTem:DATE? Successfully returned date.");
    return SCPI_RES_OK;
}