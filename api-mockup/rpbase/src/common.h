/**
 * $Id: $
 *
 * @brief Red Pitaya library common module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "rp.h"

#define ECHECK(x) { \
  int retval = (x); \
  if (retval != RP_OK) { \
    fprintf(stderr, "Runtime error: %s returned %d at %s:%d", #x, retval, __FILE__, __LINE__); \
    return retval; \
  } \
}

#define SET_BITS(x,b) ((x) |= (b))
#define UNSET_BITS(x,b) ((x) &= ~(b))
#define SET_VALUE(x,b) ((x) = (b))
#define ARE_BITS_SET(x,b) (((x) & (b)) == (b))

#define VALIDATE_BITS(b,m) { \
	if (((b) & ~(m)) != 0) return RP_EOOR; \
}



int cmn_Init();
int cmn_Release();

int cmn_Map(size_t size, size_t offset, void** mapped);
int cmn_Unmap(size_t size, void** mapped);

int cmn_SetBits(volatile uint32_t* field, uint32_t bits, uint32_t mask);
int cmn_UnsetBits(volatile uint32_t* field, uint32_t bits, uint32_t mask);
int cmn_SetBitsValue(volatile uint32_t* field, uint32_t value, uint32_t mask, uint32_t bitsToSet);
int cmn_GetValue(volatile uint32_t* field, uint32_t* value, uint32_t mask);
int cmn_GetBitsValue(volatile uint32_t* field, uint32_t* value, uint32_t mask, uint32_t bitsToSetShift);
int cmn_AreBitsSet(uint32_t field, uint32_t bits, uint32_t mask, bool* result);

#endif /* COMMON_H_ */
