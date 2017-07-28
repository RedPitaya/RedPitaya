#include <stdio.h>

#include "redpitaya/fixp.h"
#include "redpitaya/la_trg.h"

void rp_la_trg_init (rp_la_trg_t *handle, volatile rp_la_trg_regset_t *regset) {
    handle->regset = regset;
}

void rp_la_trg_default (rp_la_trg_t *handle) {
    handle->regset->cfg_msk = 0;
    handle->regset->cfg_val = 0;
    handle->regset->cfg_pos = 0;
    handle->regset->cfg_neg = 0;
}

void rp_la_trg_print (rp_la_trg_t *handle) {
    printf("asg_per.cfg_msk = %08x\n", handle->regset->cfg_msk);
    printf("asg_per.cfg_val = %08x\n", handle->regset->cfg_val);
    printf("asg_per.cfg_pos = %08x\n", handle->regset->cfg_pos);
    printf("asg_per.cfg_neg = %08x\n", handle->regset->cfg_neg);
}

uint32_t rp_la_trg_get_mask (rp_la_trg_t *handle) {
    return(handle->regset->cfg_msk);
}

int rp_la_trg_set_mask (rp_la_trg_t *handle, uint32_t value) {
    handle->regset->cfg_msk = value;
    return(0);
}

uint32_t rp_la_trg_get_value (rp_la_trg_t *handle) {
    return(handle->regset->cfg_val);
}

int rp_la_trg_set_value (rp_la_trg_t *handle, uint32_t value) {
    handle->regset->cfg_val = value;
    return(0);
}

uint32_t rp_la_trg_get_edge_pos (rp_la_trg_t *handle) {
    return(handle->regset->cfg_pos);
}

int rp_la_trg_set_edge_pos (rp_la_trg_t *handle, uint32_t value) {
    handle->regset->cfg_pos = value;
    return(0);
}

uint32_t rp_la_trg_get_edge_neg (rp_la_trg_t *handle) {
    return(handle->regset->cfg_neg);
}

int rp_la_trg_set_edge_neg (rp_la_trg_t *handle, uint32_t value) {
    handle->regset->cfg_neg = value;
    return(0);
}
