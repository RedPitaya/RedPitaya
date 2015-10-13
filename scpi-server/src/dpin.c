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
#include "../../api/rpbase/src/common.h"
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
    {"DIO0_N",  16},
    {"DIO1_N",  17},
    {"DIO2_N",  18},
    {"DIO3_N",  19},
    {"DIO4_N",  20},
    {"DIO5_N",  21},
    {"DIO6_N",  21},
    {"DIO7_N",  22},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_RpDir[] = {
    {"IN",  0},
    {"OUT", 1},
    SCPI_CHOICE_LIST_END
};

scpi_result_t RP_DigitalPinReset(scpi_t *context) {
    int result = rp_DpinReset();

    if (RP_OK != result) {
        RP_ERR("DIG:RST Failed to reset Red Pitaya digital pins", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*DIG:RST Successfully reset Red Pitaya digital pins.");
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
        RP_ERR("*DIG:PIN is missing first parameter.", NULL);
        return SCPI_RES_ERR;
    }

    /* Parse second, BIT parameter */
    if(!SCPI_ParamUInt32(context, &bit, true)){
        RP_ERR("*DIG:PIN invalid second parameter", NULL);
        return SCPI_RES_ERR;
    }

    rp_dpin_t pin = pin_choice;

    /* Set API pin state */
    int result = rp_DpinSetState(pin, bit);

    if (RP_OK != result){
		RP_ERR("*DIG:PIN Failed to set pin state", rp_GetError(result));
		return SCPI_RES_ERR;
	}

	RP_INFO("*DIG:PIN Successfully set port value");
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
        RP_ERR("*DIG:PIN? is missing first parameter.", NULL);
        return SCPI_RES_ERR;
    }

    rp_dpin_t pin = pin_choice;

    /* Get pin state */
    rp_pinState_t state;
    int result = rp_DpinGetState(pin, &state);

    if (RP_OK != result){
        RP_ERR("*DIG:PIN? Failed to get pin state", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    /* Return PIN state to the client */
    SCPI_ResultInt32(context, state);

    RP_INFO("*DIG:PIN? Successfully returned port value");
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
        RP_ERR("*DIG:PIN:DIR is missing first parameter", NULL);
        return SCPI_RES_ERR;
    }

    /* Read second, PIN parameter */
    if(!SCPI_ParamChoice(context, scpi_RpDpin, &pin_choice, true)){
        RP_ERR("*DIG:PIN:DIR is missing second parameter.", NULL);
        return SCPI_RES_ERR;
    }

    /* Set api values */
    rp_pinDirection_t direction = dir_choice;
    rp_dpin_t pin               = pin_choice;

    // Now set the pin state
    int result = rp_DpinSetDirection(pin, direction);

    if (RP_OK != result){
        RP_ERR("*DIG:PIN:DIR Failed to set pin direction", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_INFO("*DIG:PIN:DIR Successfully set port direction.");
    return SCPI_RES_OK;
}
