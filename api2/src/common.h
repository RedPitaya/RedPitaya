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

#include <stdint.h>
#include <stdbool.h>

#include "rp2.h"

// unmasked IO read/write (p - pointer, v - value)
#define ioread32(p) (*(volatile uint32_t *)(p))
#define iowrite32(v,p) (*(volatile uint32_t *)(p) = (v))

#define RP_TRG_LGA_PAT_MASK 	(1<<5)   ///< logic analyzer pattern trigger
#define RP_TRG_LGA_SWE_MASK 	(1<<6)   ///< logic analyzer software trigger
#define RP_TRG_DIG_GEN_SWE_MASK (1<<7) ///< digital generator software trigger
#define RP_TRG_ALL 0xffffffff

/** Global trigger mask */
typedef struct {
    uint32_t msk;     ///<  global trigger mask
} rp_global_trig_regset_t;

#endif /* COMMON_H_ */
