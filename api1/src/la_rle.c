#include <stdio.h>
#include <stdbool.h>

#include "redpitaya/la_rle.h"

void rp_la_rle_init (rp_la_rle_t *handle, volatile rp_la_rle_regset_t *regset) {
    handle->regset = regset;
}

void rp_la_rle_default (rp_la_rle_t *handle) {
    handle->regset->cfg_rle = 0;
}

void rp_la_rle_print (rp_la_rle_t *handle) {
    printf("la_rle.cfg_rle = %08x\n", handle->regset->cfg_rle);
    printf("la_rle.sts_cur = %08x\n", handle->regset->sts_cur);
    printf("la_rle.sts_lst = %08x\n", handle->regset->sts_lst);
}

bool rp_la_rle_get_rle (rp_la_rle_t *handle) {
    return((bool) handle->regset->cfg_rle);
}

int rp_la_rle_set_rle (rp_la_rle_t *handle, bool value) {
    handle->regset->cfg_rle = (uint32_t) value;
    return(0);
}

uint32_t rp_la_rle_get_counter_current (rp_la_rle_t *handle) {
    return(handle->regset->sts_cur);
}

uint32_t rp_la_rle_get_counter_last (rp_la_rle_t *handle) {
    return(handle->regset->sts_lst);
}

