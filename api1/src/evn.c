#include <stdint.h>
#include <stdbool.h>

#include "evn.h"

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

uint32_t rp_evn_get_trig_src (rp_evn_regset_t *regset) {
    return (regset->cfg_trg);
}

inline void rp_evn_set_trig_src (rp_evn_regset_t *regset, uint32_t value) {
    regset->cfg_trg = value;
}

