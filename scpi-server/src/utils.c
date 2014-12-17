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

#include <strings.h>

#include "utils.h"


/**
 * Converts from String into rp_dpin_t...
 * @param pinStr
 * @param rpPin
 * @return
 */
int getRpPin(const char* pinStr, rp_dpin_t *rpPin) {
	if(strcasecmp(pinStr, "LED0") == 0) {
		*rpPin = RP_LED0;
	}
	else if(strcasecmp(pinStr, "LED1") == 0) {
		*rpPin = RP_LED1;
	}
	else if(strcasecmp(pinStr, "LED2") == 0) {
		*rpPin = RP_LED2;
	}
	else if(strcasecmp(pinStr, "LED3") == 0) {
		*rpPin = RP_LED3;
	}
	else if(strcasecmp(pinStr, "LED4") == 0) {
		*rpPin = RP_LED4;
	}
	else if(strcasecmp(pinStr, "LED5") == 0) {
		*rpPin = RP_LED5;
	}
	else if(strcasecmp(pinStr, "LED6") == 0) {
		*rpPin = RP_LED6;
	}
	else if(strcasecmp(pinStr, "LED7") == 0) {
		*rpPin = RP_LED7;
	}

	else if(strcasecmp(pinStr, "DIO0_P") == 0) {
		*rpPin = RP_DIO0_P;
	}
	else if(strcasecmp(pinStr, "DIO1_P") == 0) {
		*rpPin = RP_DIO1_P;
	}
	else if(strcasecmp(pinStr, "DIO2_P") == 0) {
		*rpPin = RP_DIO2_P;
	}
	else if(strcasecmp(pinStr, "DIO3_P") == 0) {
		*rpPin = RP_DIO3_P;
	}
	else if(strcasecmp(pinStr, "DIO4_P") == 0) {
		*rpPin = RP_DIO4_P;
	}
	else if(strcasecmp(pinStr, "DIO5_P") == 0) {
		*rpPin = RP_DIO5_P;
	}
	else if(strcasecmp(pinStr, "DIO6_P") == 0) {
		*rpPin = RP_DIO6_P;
	}
	else if(strcasecmp(pinStr, "DIO7_P") == 0) {
		*rpPin = RP_DIO7_P;
	}

	else if(strcasecmp(pinStr, "DIO0_N") == 0) {
		*rpPin = RP_DIO0_N;
	}
	else if(strcasecmp(pinStr, "DIO1_N") == 0) {
		*rpPin = RP_DIO1_N;
	}
	else if(strcasecmp(pinStr, "DIO2_N") == 0) {
		*rpPin = RP_DIO2_N;
	}
	else if(strcasecmp(pinStr, "DIO3_N") == 0) {
		*rpPin = RP_DIO3_N;
	}
	else if(strcasecmp(pinStr, "DIO4_N") == 0) {
		*rpPin = RP_DIO4_N;
	}
	else if(strcasecmp(pinStr, "DIO5_N") == 0) {
		*rpPin = RP_DIO5_N;
	}
	else if(strcasecmp(pinStr, "DIO6_N") == 0) {
		*rpPin = RP_DIO6_N;
	}
	else if(strcasecmp(pinStr, "DIO7_N") == 0) {
		*rpPin = RP_DIO7_N;
	}
	else {
		return 1; // ERROR
	}

	return 0; // OK
}
