/**
 * $Id: $
 *
 * @brief Red Pitaya library Health handler interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef HEALTH_HANDLER_H_
#define HEALTH_HANDLER_H_


#include "rp.h"
#include "health.h"

int health_GetValue(rp_health_t sensor, float *value);

#endif
