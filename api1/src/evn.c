#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "evn.h"

void rp_evn_init (rp_evn_t *handle, volatile rp_evn_regset_t *regset) {
    handle->regset = regset;
}

void rp_evn_print (rp_evn_t *handle) {
    printf ("evn.ctl_sts = %08x\n", handle->regset->ctl_sts);
    printf ("evn.cfg_evn = %08x\n", handle->regset->cfg_evn);
    printf ("evn.cfg_trg = %08x\n", handle->regset->cfg_trg);
}

void rp_evn_reset (rp_evn_t *handle) {
    handle->regset->ctl_sts = CTL_RST_MASK;
}

void rp_evn_start (rp_evn_t *handle) {
    handle->regset->ctl_sts = CTL_STR_MASK;
}

void rp_evn_stop (rp_evn_t *handle) {
    handle->regset->ctl_sts = CTL_STP_MASK;
}

void rp_evn_trigger (rp_evn_t *handle) {
    handle->regset->ctl_sts = CTL_TRG_MASK;
}

bool rp_evn_status_run (rp_evn_t *handle) {
    return ((bool) (handle->regset->ctl_sts & CTL_STR_MASK));
}

bool rp_evn_status_trigger (rp_evn_t *handle) {
    return ((bool) (handle->regset->ctl_sts & CTL_TRG_MASK));
}

uint32_t rp_evn_get_sync_src (rp_evn_t *handle) {
    return (handle->regset->cfg_evn);
}

void rp_evn_set_sync_src (rp_evn_t *handle, uint32_t value) {
    handle->regset->cfg_trg = value;
}

uint32_t rp_evn_get_trig_src (rp_evn_t *handle) {
    return (handle->regset->cfg_trg);
}

void rp_evn_set_trig_src (rp_evn_t *handle, uint32_t value) {
    handle->regset->cfg_trg = value;
}

