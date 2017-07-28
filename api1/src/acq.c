#include <stdio.h>

#include "redpitaya/fixp.h"
#include "redpitaya/acq.h"

void rp_acq_init (rp_acq_t *handle, volatile rp_acq_regset_t *regset, const fixp_t cnt_t) {
    handle->regset = regset;
    handle->cnt_t = cnt_t;
}

void rp_acq_default (rp_acq_t *handle) {
    handle->regset->cfg_pre = 0;
    handle->regset->cfg_pst = 0;
}

void rp_acq_print (rp_acq_t *handle) {
    printf("acq.cnt_t = %s\n", rp_util_fixp_print(handle->cnt_t));
    printf("acq.cfg_pre = %08x\n", handle->regset->cfg_pre);
    printf("acq.cfg_pst = %08x\n", handle->regset->cfg_pst);
    printf("acq.sts_pre = %08x\n", handle->regset->sts_pre);
    printf("acq.sts_pst = %08x\n", handle->regset->sts_pst);
}

uint32_t rp_acq_get_trigger_pre (rp_acq_t *handle) {
    return(handle->regset->cfg_pre);
}

int rp_acq_set_trigger_pre (rp_acq_t *handle, uint32_t value) {
    if (value <= fixp_max(handle->cnt_t)) {
        handle->regset->cfg_pre = value;
        return(0);
    } else {
//      raise ValueError("Pre trigger delay should be positive and less or equal to {}.".format(self._CWr))
        return(-1);
    }
}

uint32_t rp_acq_get_trigger_post (rp_acq_t *handle) {
    return(handle->regset->cfg_pst);
}

int rp_acq_set_trigger_post (rp_acq_t *handle, uint32_t value) {
    if (value <= fixp_max(handle->cnt_t)) {
        handle->regset->cfg_pst = value;
        return(0);
    } else {
//      raise ValueError("Post trigger delay should be positive and less or equal to {}.".format(self._CWr))
        return(-1);
    }
}

uint32_t rp_acq_get_trigger_pre_status (rp_acq_t *handle) {
    return(handle->regset->sts_pre & fixp_mask(handle->cnt_t));
}

uint32_t rp_acq_get_trigger_post_status (rp_acq_t *handle) {
    return(handle->regset->sts_pst & fixp_mask(handle->cnt_t));
}

