/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server SCPI commands implementation
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
#include <stdlib.h>

#include "common.h"
#include "led.h"
#include "scpi/parser.h"
#include "rp_hw.h"

const scpi_choice_def_t scpi_LED_state[] = {
    {"OFF", 0},
    {"ON", 1},
    SCPI_CHOICE_LIST_END
};

scpi_result_t RP_LED_MMC(scpi_t *context) {    
    int32_t value;

    if (!SCPI_ParamChoice(context, scpi_LED_state, &value, true)) {
        RP_LOG(LOG_ERR, "*LED:MMC is missing first parameter.\n");
        return SCPI_RES_ERR;
    }

    int result = rp_SetLEDMMCState((bool)value);
    if (RP_HW_OK != result) {
        RP_LOG(LOG_ERR, "*LED:MMC Failed to set MMC led: %d\n", result);
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*LED:MMC Successfully set MMC led.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_LED_MMCQ(scpi_t *context) {
    const char *_name;

    bool value;
    int result = rp_GetLEDMMCState(&value);

    if (RP_OK != result) {
        RP_LOG(LOG_ERR, "*LED:MMC? Failed get LED state: %d", result);
        return SCPI_RES_ERR;
    }

    if(!SCPI_ChoiceToName(scpi_LED_state, (int32_t)value, &_name)){
        RP_LOG(LOG_ERR, "*LED:MMC? Failed to parse LED state.\n");
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, _name);

    
    RP_LOG(LOG_INFO, "*LED:MMC? Successfully returned MMC led state.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_LED_HB(scpi_t *context) {    
    int32_t value;

    if (!SCPI_ParamChoice(context, scpi_LED_state, &value, true)) {
        RP_LOG(LOG_ERR, "*LED:HB is missing first parameter.\n");
        return SCPI_RES_ERR;
    }

    int result = rp_SetLEDHeartBeatState((bool)value);
    if (RP_HW_OK != result) {
        RP_LOG(LOG_ERR, "*LED:HB Failed to set HB led: %d\n", result);
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*LED:HB Successfully set HB led.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_LED_HBQ(scpi_t *context) {
    const char *_name;

    bool value;
    int result = rp_GetLEDHeartBeatState(&value);

    if (RP_OK != result) {
        RP_LOG(LOG_ERR, "*LED:HB? Failed get LED state: %d", result);
        return SCPI_RES_ERR;
    }

    if(!SCPI_ChoiceToName(scpi_LED_state, (int32_t)value, &_name)){
        RP_LOG(LOG_ERR, "*LED:HB? Failed to parse LED state.\n");
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, _name);

    
    RP_LOG(LOG_INFO, "*LED:HB? Successfully returned HB led state.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_LED_ETH(scpi_t *context) {    
    int32_t value;

    if (!SCPI_ParamChoice(context, scpi_LED_state, &value, true)) {
        RP_LOG(LOG_ERR, "*LED:ETH is missing first parameter.\n");
        return SCPI_RES_ERR;
    }

    int result = rp_SetLEDEthState((bool)value);
    if (RP_HW_OK != result) {
        RP_LOG(LOG_ERR, "*LED:ETH Failed to set ETH led: %d\n", result);
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*LED:ETH Successfully set ETH led.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_LED_ETHQ(scpi_t *context) {
    const char *_name;

    bool value;
    int result = rp_GetLEDEthState(&value);

    if (RP_OK != result) {
        RP_LOG(LOG_ERR, "*LED:ETH? Failed get LED state: %d", result);
        return SCPI_RES_ERR;
    }

    if(!SCPI_ChoiceToName(scpi_LED_state, (int32_t)value, &_name)){
        RP_LOG(LOG_ERR, "*LED:ETH? Failed to parse LED state.\n");
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, _name);

    
    RP_LOG(LOG_INFO, "*LED:ETH? Successfully returned ETH led state.\n");
    return SCPI_RES_OK;
}
