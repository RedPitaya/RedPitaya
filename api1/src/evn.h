#ifndef __RP_EVN_H__
#define __RP_EVN_H__

// control register masks
#define CTL_TRG_MASK  (1<<3)  // sw trigger bit (sw trigger must be enabled)
#define CTL_STP_MASK  (1<<2)  // stop/abort; returns 1 when stopped
#define CTL_STR_MASK  (1<<1)  // start
#define CTL_RST_MASK  (1<<0)  // reset state machine so that it is in known state

typedef volatile struct {
    uint32_t ctl_sts;  // control/status register
    uint32_t cfg_evn;  // software event source select
    uint32_t cfg_trg;  // hardware trigger mask
} rp_evn_regset_t;


#endif
