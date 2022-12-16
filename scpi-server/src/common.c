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
#include "common.h"

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
    if (!((ch_usr[0] > 0) && (ch_usr[0] <= getADCChannels()))) {
        RP_LOG(LOG_ERR, "ERROR: Invalid channel number: %.*s\n", 50, context->param_list.cmd_raw.data);
        return RP_EOOR;
    }
    *channel = ch_usr[0] - 1;

    return RP_OK;
}

int RP_ParseChArgvDAC(scpi_t *context, rp_channel_t *channel){

    int32_t ch_usr[1];
    SCPI_CommandNumbers(context, ch_usr, 1, SCPI_CMD_NUM);
    if (!((ch_usr[0] > 0) && (ch_usr[0] <= getDACChannels()))) {
        RP_LOG(LOG_ERR, "ERROR: Invalid channel number: %.*s\n", 50, context->param_list.cmd_raw.data);
        return RP_EOOR;
    }
    *channel = ch_usr[0] - 1;

    return RP_OK;
}

scpi_result_t RP_SetLogMode(scpi_t *context) {

    int32_t choice;

    /* Read UNITS parameters */
    if(!SCPI_ParamChoice(context, scpi_RpLogMode, &choice, true)){
        RP_LOG(LOG_ERR, "*RP:LOGmode Missing first parameter.\n");
        return SCPI_RES_ERR;
    }

    /* Set global units for acq scpi */
    g_logMode = choice;

    RP_LOG(LOG_INFO, "*RP:LOGmode Successfully set scpi units.\n");
    return SCPI_RES_OK;
}

rp_scpi_log getLogMode(){
    return g_logMode;
}

void RP_LOG(int mode, const char * format,...){
    va_list args;
    va_start (args, format);
    if (getLogMode() == RP_SCPI_LOG_SYSLOG)
        syslog(mode, format, args);
    if (getLogMode() == RP_SCPI_LOG_CONSOLE)
        vfprintf(stdout, format, args);
    va_end (args);
}

uint8_t getADCChannels(){
    uint8_t c = 0;
    if (rp_HPGetFastADCChannelsCount(&c) != RP_HP_OK){
        RP_LOG(LOG_WARNING,"[Error] Can't get fast ADC channels count\n");
    }
    return c;
}

uint8_t getDACChannels(){
    uint8_t c = 0;

    if (rp_HPGetFastDACChannelsCount(&c) != RP_HP_OK){
        RP_LOG(LOG_WARNING,"[Error] Can't get fast DAC channels count\n");
    }
    return c;
}

uint32_t getDACRate(){
    uint32_t c = 0;
    if (rp_HPGetBaseFastDACSpeedHz(&c) != RP_HP_OK){
        RP_LOG(LOG_WARNING,"[Error] Can't get fast DAC channels count\n");
    }
    return c;
}

uint32_t getADCRate(){
    uint32_t c = 0;
    if (rp_HPGetBaseFastADCSpeedHz(&c) != RP_HP_OK){
        RP_LOG(LOG_WARNING,"[Error] Can't get fast ADC channels count\n");
    }
    return c;
}

rp_HPeModels_t getModel(){
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK){
        RP_LOG(LOG_WARNING,"[Error] Can't get board model\n");
    }
    return c;
}