/**
 * $Id: $
 *
 * @brief Red Pitaya library Identification handler interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef ID_HANDLER_H_
#define ID_HANDLER_H_

#include <stdint.h>
#include <stdbool.h>
#include "rp.h"

int id_GetID(uint32_t *id);
int id_GetDNA(uint64_t *dna);

#endif /* ID_HANDLER_H_ */
