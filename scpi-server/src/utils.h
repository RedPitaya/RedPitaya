/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server utils module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "rp.h"

#define SET_OK(cont) \
    	SCPI_ResultString(cont, "OK"); \
    	return SCPI_RES_OK;

int getRpDpin(const char* pinStr, rp_dpin_t *rpPin);
int getRpDirection(const char *dirStr, rp_pinDirection_t *direction);
int getRpApin(const char *pinStr, rp_apin_t *rpPin);

#endif /* UTILS_H_ */
