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

#include "redpitaya/rp1.h"

// unmasked IO read/write (p - pointer, v - value)
#define ioread32(p) (*(volatile uint32_t *)(p))
#define iowrite32(v,p) (*(volatile uint32_t *)(p) = (v))

#define SET_BITS(x,b) ((x) |= (b))
#define UNSET_BITS(x,b) ((x) &= ~(b))
#define SET_VALUE(x,b) ((x) = (b))
#define ARE_BITS_SET(x,b) (((x) & (b)) == (b))

#define VALIDATE_BITS(b,m) { \
        if (((b) & ~(m)) != 0) return RP_EOOR; \
}

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define FLOAT_EPS 0.00001f

#define GAIN_V(gain) (gain ? 20.0 : 1.0)

/* @brief Number of ADC acquisition bits. */
#define ADC_BITS     14
#define ADC_BITS_MSK (1<<ADC_BITS)-1
#define ADC_BITS_MAX ( (1<<(ADC_BITS-1))-1)
#define ADC_BITS_MIN (-(1<<(ADC_BITS-1))  )

/* @brief Number of DAC generator bits. */
#define DAC_BITS     14
#define DAC_BITS_MSK (1<<ADC_BITS)-1
#define DAC_BITS_MAX ( (1<<(ADC_BITS-1))-1)
#define DAC_BITS_MIN (-(1<<(ADC_BITS-1))  )

int cmn_Init();
int cmn_Release();

int cmn_Map(size_t size, size_t offset, void** mapped);
int cmn_Unmap(size_t size, void** mapped);

int cmn_SetBits(volatile uint32_t* field, uint32_t bits, uint32_t mask);
int cmn_UnsetBits(volatile uint32_t* field, uint32_t bits, uint32_t mask);
int cmn_AreBitsSet(volatile uint32_t field, uint32_t bits, uint32_t mask, bool* result);

#endif /* COMMON_H_ */
