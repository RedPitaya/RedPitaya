#include <stdio.h>

#include "redpitaya/fixp.h"
#include "redpitaya/osc_trg.h"

void rp_osc_trg_init (rp_osc_trg_t *handle, volatile rp_osc_trg_regset_t *regset, const fixp_t dat_t) {
    handle->regset = regset;
    handle->dat_t = dat_t;
}

void rp_osc_trg_default (rp_osc_trg_t *handle) {
    handle->regset->cfg_pos = 0;
    handle->regset->cfg_neg = 0;
    handle->regset->cfg_edg = 0;
}

void rp_osc_trg_print (rp_osc_trg_t *handle) {
    printf("asg_per.dat_t = %s\n", rp_util_fixp_print(handle->dat_t));
    printf("asg_per.cfg_pos = %08x\n", handle->regset->cfg_pos);
    printf("asg_per.cfg_neg = %08x\n", handle->regset->cfg_neg);
    printf("asg_per.cfg_edg = %08x\n", handle->regset->cfg_edg);
}

int32_t rp_osc_trg_get_level_pos (rp_osc_trg_t *handle) {
    return(handle->regset->cfg_pos);
}

int rp_osc_trg_set_level_pos (rp_osc_trg_t *handle, int32_t value) {
    if ( (fixp_min(handle->dat_t) <= value) && (value <= fixp_max(handle->dat_t)) ) {
        handle->regset->cfg_pos = value;
        return(0);
    } else {
//      raise ValueError("Trigger positive level should be inside [{},{}]".format(self.input_range))
        return(-1);
    }
}

int32_t rp_osc_trg_get_level_neg (rp_osc_trg_t *handle) {
    return(handle->regset->cfg_neg);
}

int rp_osc_trg_set_level_neg (rp_osc_trg_t *handle, int32_t value) {
    if ( (fixp_min(handle->dat_t) <= value) && (value <= fixp_max(handle->dat_t)) ) {
        handle->regset->cfg_neg = value;
        return(0);
    } else {
//      raise ValueError("Trigger negative level should be inside [{},{}]".format(self.input_range))
        return(-1);
    }
}

uint32_t rp_osc_trg_get_edge (rp_osc_trg_t *handle) {
    return(handle->regset->cfg_edg);
}

int rp_osc_trg_set_edge (rp_osc_trg_t *handle, uint32_t value) {
    if ( (value == pos) || (value == neg) ) {
        handle->regset->cfg_edg = value;
        return(0);
    } else {
//      raise ValueError("Trigger edge should be one of {}".format(list(self._edges.keys())))
        return(-1);
    }
}

