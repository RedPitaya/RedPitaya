#ifndef EVN_H
#define EVN_H

#include <stdint.h>
#include <stdbool.h>

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

inline void rp_evn_reset (rp_evn_regset_t *regset) {
    regset->ctl_sts = CTL_RST_MASK;
}

inline void rp_evn_start (rp_evn_regset_t *regset) {
    regset->ctl_sts = CTL_STR_MASK;
}

inline void rp_evn_stop (rp_evn_regset_t *regset) {
    regset->ctl_sts = CTL_STP_MASK;
}

inline void rp_evn_trigger (rp_evn_regset_t *regset) {
    regset->ctl_sts = CTL_TRG_MASK;
}

inline bool rp_evn_status_run (rp_evn_regset_t *regset) {
    return ((bool) (regset->ctl_sts & CTL_STR_MASK));
}

inline bool rp_evn_status_trigger (rp_evn_regset_t *regset) {
    return ((bool) (regset->ctl_sts & CTL_TRG_MASK));
}

inline uint32_t rp_evn_get_sync_src (rp_evn_regset_t *regset) {
    return (regset->cfg_evn);
}

inline void rp_evn_set_sync_src (rp_evn_regset_t *regset, uint32_t value) {
    regset->cfg_trg = value;
}

inline uint32_t rp_evn_get_trig_src (rp_evn_regset_t *regset) {
    return (regset->cfg_trg);
}

inline void rp_evn_set_trig_src (rp_evn_regset_t *regset, uint32_t value) {
    regset->cfg_trg = value;
}

#endif
