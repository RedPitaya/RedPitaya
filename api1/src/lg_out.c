#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "redpitaya/fixp.h"
#include "redpitaya/lg_out.h"

void rp_lg_out_init(rp_lg_out_t *handle, volatile rp_lg_out_regset_t *regset, const fixp_t dat_t) {
    handle->regset = regset;
    handle->dat_t = dat_t;
}

void rp_lg_out_default(rp_lg_out_t *handle) {
    for (int unsigned i=0; i<2; i++)
        handle->regset->cfg_oen[i] = 0;
    handle->regset->cfg_msk    = 0;
    handle->regset->cfg_val    = 0;
}

void rp_lg_out_print(rp_lg_out_t *handle) {
    printf("dat_t = %s\n", rp_util_fixp_print(handle->dat_t));
    for (int unsigned i=0; i<2; i++)
        printf("lg_out.cfg_oen[%u] = %08x\n", i, handle->regset->cfg_oen[i]);
    printf("lg_out.cfg_msk = %08x\n", handle->regset->cfg_msk);
    printf("lg_out.cfg_val = %08x\n", handle->regset->cfg_val);
}

void rp_lg_out_get_enable(rp_lg_out_t *handle, uint32_t *value[2]) {
    for (int unsigned i=0; i<2; i++)
        *value[i] = handle->regset->cfg_oen[i];
}

int rp_lg_out_set_enable(rp_lg_out_t *handle, uint32_t value[2]) {
    for (int unsigned i=0; i<2; i++) {
        if (value[i] & ~fixp_mask(handle->dat_t)) {
            handle->regset->cfg_oen[i] = value[i];
        } else {
//            raise ValueError(".")
            return(-1);
        }
    }
    return(0);
}

uint32_t rp_lg_out_get_mask(rp_lg_out_t *handle) {
    return(handle->regset->cfg_msk);
}

int rp_lg_out_set_mask(rp_lg_out_t *handle, uint32_t value) {
    if (value & ~fixp_mask(handle->dat_t)) {
        handle->regset->cfg_msk = value;
    } else {
//        raise ValueError(".")
        return(-1);
    }
    return(0);
}

uint32_t rp_lg_out_get_value(rp_lg_out_t *handle) {
    return(handle->regset->cfg_val);
}

int rp_lg_out_set_value(rp_lg_out_t *handle, uint32_t value) {
    if (value & ~fixp_mask(handle->dat_t)) {
        handle->regset->cfg_val = value;
    } else {
//        raise ValueError(".")
        return(-1);
    }
    return(0);
}

