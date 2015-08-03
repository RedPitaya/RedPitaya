/**
 * $Id: $
 *
 * @brief Red Pitaya library Identification handler implementation
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
#include <stdint.h>

#include "version.h"
#include "common.h"
#include "housekeeping.h"
#include "id_handler.h"

int id_GetID(uint32_t *id){
    return hk_GetID(id);
}

int id_GetDNA(uint64_t *dna){
    return hk_GetDNA(dna);
}

