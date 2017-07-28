#include <stdio.h>
#include <stdbool.h>

#include "redpitaya/la_msk.h"

void rp_la_msk_init (rp_la_msk_t *handle, volatile rp_la_msk_regset_t *regset) {
    handle->regset = regset;
}

void rp_la_msk_default (rp_la_msk_t *handle) {
    handle->regset->cfg_msk = 0;
    handle->regset->cfg_pol = 0;
}

void rp_la_msk_print (rp_la_msk_t *handle) {
    printf("la_msk.cfg_msk = %08x\n", handle->regset->cfg_msk);
    printf("la_msk.cfg_pol = %08x\n", handle->regset->cfg_pol);
}

int      rp_la_msk_set_input_mask      (rp_la_msk_t *handle, uint32_t value);
uint32_t rp_la_msk_get_input_polarity  (rp_la_msk_t *handle);


uint32_t rp_la_msk_get_input_mask (rp_la_msk_t *handle) {
    return(handle->regset->cfg_msk);
}

int rp_la_msk_set_input_mask (rp_la_msk_t *handle, uint32_t value) {
    handle->regset->cfg_msk = value;
    return(0);
}

uint32_t rp_la_msk_get_input_polarity (rp_la_msk_t *handle) {
    return(handle->regset->cfg_pol);
}

int rp_la_msk_set_input_polarity (rp_la_msk_t *handle, uint32_t value) {
    handle->regset->cfg_pol = value;
    return(0);
}

