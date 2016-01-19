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

typedef struct {
  char          *name;
  char          *dev;
  int            fd;
  volatile void *regset;
           void *context;
} rp_handle_uio_t;

#include "redpitaya/rp.h"

// unmasked IO read/write (p - pointer, v - value)
#define ioread32(p) (*(volatile uint32_t *)(p))
#define iowrite32(v,p) (*(volatile uint32_t *)(p) = (v))

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define FLOAT_EPS 0.00001f

#define FULL_SCALE_NORM     20.0    // V

int cmn_Init();
int cmn_Release();

int cmn_Map(size_t size, size_t offset, void** mapped);
int cmn_Unmap(size_t size, void** mapped);

#endif /* COMMON_H_ */
