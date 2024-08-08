/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server dpin SCPI commands implementation
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

#include "common.h"
#include "dpin.h"
#include "scpi/parser.h"


/* Param choice definition - Must remain the same! */
const scpi_choice_def_t scpi_RpDpin[] = {
    {"LED0",    0},
    {"LED1",    1},
    {"LED2",    2},
    {"LED3",    3},
    {"LED4",    4},
    {"LED5",    5},
    {"LED6",    6},
    {"LED7",    7},
    {"DIO0_P",  8},
    {"DIO1_P",  9},
    {"DIO2_P",  10},
    {"DIO3_P",  11},
    {"DIO4_P",  12},
    {"DIO5_P",  13},
    {"DIO6_P",  14},
    {"DIO7_P",  15},
    {"DIO8_P",  16},
    {"DIO9_P",  17},
    {"DIO10_P", 18},
    {"DIO0_N",  19},
    {"DIO1_N",  20},
    {"DIO2_N",  21},
    {"DIO3_N",  22},
    {"DIO4_N",  23},
    {"DIO5_N",  24},
    {"DIO6_N",  25},
    {"DIO7_N",  26},
    {"DIO8_N",  27},
    {"DIO9_N",  28},
    {"DIO10_N", 29},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_RpDir[] = {
    {"IN",  0},
    {"OUT", 1},
    SCPI_CHOICE_LIST_END
};

scpi_result_t RP_DigitalPinReset(scpi_t *context) {
    auto result = rp_DpinReset();

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to reset Red Pitaya digital pins: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

/**
 *  Sets Digital Pin to state High/Low
 * @param context SCPI context
 * @return success or failure
 */
scpi_result_t RP_DigitalPinState(scpi_t * context) {

    int32_t pin_choice;
    uint32_t bit;

    /* Parse first, PIN parameter */
    if(!SCPI_ParamChoice(context, scpi_RpDpin, &pin_choice, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    /* Parse second, BIT parameter */
    if(!SCPI_ParamUInt32(context, &bit, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Invalid second parameter");
        return SCPI_RES_ERR;
    }

    rp_dpin_t pin = (rp_dpin_t)pin_choice;

    /* Set API pin state */
    auto result = rp_DpinSetState(pin, (rp_pinState_t)bit);

    if (RP_OK != result){
		RP_LOG_CRIT("Failed to set pin state: %s", rp_GetError(result));
		return SCPI_RES_ERR;
	}
    RP_LOG_INFO("%s",rp_GetError(result))
	return SCPI_RES_OK;
}

/**
 * Returns Digital Pin state to SCPI context
 * @param context SCPI context
 * @return success or failure
 */
scpi_result_t RP_DigitalPinStateQ(scpi_t * context) {

    int32_t pin_choice;

    /* Read PIN parameter */
    if(!SCPI_ParamChoice(context, scpi_RpDpin, &pin_choice, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    rp_dpin_t pin = (rp_dpin_t)pin_choice;

    /* Get pin state */
    rp_pinState_t state;
    auto result = rp_DpinGetState(pin, &state);

    if (RP_OK != result){
        RP_LOG_CRIT("Failed to get pin state: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    /* Return PIN state to the client */
    SCPI_ResultInt32(context, state);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

/**
* Sets Digital Pin direction to state Output/Input
* @param context SCPI context
* @return success or failure
*/
scpi_result_t RP_DigitalPinDirection(scpi_t * context) {

    int32_t dir_choice, pin_choice;

    /* Read first, DIRECTION parameter */
    if(!SCPI_ParamChoice(context, scpi_RpDir, &dir_choice, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    /* Read second, PIN parameter */
    if(!SCPI_ParamChoice(context, scpi_RpDpin, &pin_choice, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Invalid second parameter");
        return SCPI_RES_ERR;
    }

    /* Set api values */
    rp_pinDirection_t direction = (rp_pinDirection_t)dir_choice;
    rp_dpin_t pin               = (rp_dpin_t)pin_choice;

    // Now set the pin state
    auto result = rp_DpinSetDirection(pin, direction);

    if (RP_OK != result){
        RP_LOG_CRIT("Failed to set pin direction: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_DigitalPinDirectionQ(scpi_t *context){

    int32_t usr_pin;
    int result;
    const char *dir_n;

    if(!SCPI_ParamChoice(context, scpi_RpDpin, &usr_pin, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    rp_dpin_t pin = (rp_dpin_t)usr_pin;
    rp_pinDirection_t direction;

    result = rp_DpinGetDirection(pin, &direction);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to returned pin direction");
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("GET DIRECTION: %d", direction);

    if(!SCPI_ChoiceToName(scpi_RpDir, direction, &dir_n)){
        RP_LOG_CRIT("Failed to parse direction.");
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, dir_n);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}
