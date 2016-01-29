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

#include "redpitaya/rp2.h"

extern const char c_dummy_dev[10];

// unmasked IO read/write (p - pointer, v - value)
#define ioread32(p) (*(volatile uint32_t *)(p))
#define iowrite32(v,p) (*(volatile uint32_t *)(p) = (v))

// open/close UIO device
int common_Open(char *dev, rp_handle_uio_t *handle);
int common_Close(rp_handle_uio_t *handle);

//** trigger mask bits */
#define RP_TRG_EXT_PE_PAT_MASK   (1<<13)   ///< external trigger positive edge
#define RP_TRG_EXT_NE_PAT_MASK   (1<<12)   ///< external trigger negative edge

#define RP_TRG_GEN1_PER_MASK     (1<<11)   ///< triggers at beginning of each gen1. period
#define RP_TRG_GEN2_PER_MASK     (1<<10)   ///< triggers at beginning of each gen2. period

#define RP_TRG_GEN1_SWE_MASK     (1<<9)   ///< generator1 sw trigger
#define RP_TRG_GEN2_SWE_MASK     (1<<8)   ///< generator2 sw trigger

#define RP_TRG_ACQ1_EDGE_MASK    (1<<7)   ///< edge triggering acq1
#define RP_TRG_ACQ2_EDGE_MASK    (1<<6)   ///< edge triggering acq2

#define RP_TRG_ACQ1_SWE_MASK     (1<<5)   ///< acq. sw trigger
#define RP_TRG_ACQ2_SWE_MASK     (1<<4)   ///< acq. sw trigger

#define RP_TRG_DGEN_PER_MASK     (1<<3)   ///< triggers at beginning of gen. period
#define RP_TRG_DGEN_SWE_MASK     (1<<2)   ///< gen. sw trigger

#define RP_TRG_LOA_PAT_MASK      (1<<1)   ///< logic analyzer pattern trigger
#define RP_TRG_LOA_SWE_MASK      (1<<0)   ///< logic analyzer software trigger

#define RP_TRG_ALL_MASK         0xffffffff

/** Global trigger mask */
typedef struct {
    uint32_t msk;     ///<  global trigger mask
} rp_global_trig_regset_t;

/** control register masks  */
#define RP_CTL_RST_MASK (1<<0) ///< 1 - reset state machine so that it is in known state
#define RP_CTL_STA_MASK (1<<1) ///< 1 - ACQ: starts acq.; 0 - when acq. is stopped
#define RP_CTL_STO_MASK (1<<2) ///< 1 - ACQ: stops acq. (also used to about the acq.)
#define RP_CTL_SWT_MASK (1<<3) ///< 1 - sw trigger bit (sw trigger must be enabled)

/** Control registers */
typedef struct {
    uint32_t ctl;
} rp_ctl_regset_t;

int FpgaRegDump(uint32_t a_addr, uint32_t * a_data, uint32_t a_len);

#endif /* COMMON_H_ */
