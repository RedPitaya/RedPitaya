/**
 * $Id: $
 *
 * @brief Red Pitaya library Digital Pin handler interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef DPIN_HANDLER_H_
#define DPIN_HANDLER_H_

#include <stdint.h>
#include <stdbool.h>
#include "rp.h"

int dpin_SetDirection(rp_dpin_t pin, rp_pinDirection_t direction);
int dpin_GetDirection(rp_dpin_t pin, rp_pinDirection_t* direction);
int dpin_SetState(rp_dpin_t pin, rp_pinState_t state);
int dpin_GetState(rp_dpin_t pin, rp_pinState_t* state);

#endif /* DPIN_HANDLER_H_ */
