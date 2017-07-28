#include <stdio.h>
#include <stdbool.h>

#include "redpitaya/fixp.h"
#include "redpitaya/osc_fil.h"

void rp_osc_fil_init (rp_osc_fil_t *handle, volatile rp_osc_fil_regset_t *regset) {
    handle->regset = regset;
//    handle->filters = { {0x7D93, 0x437C7, 0xd9999a, 0x2666),
//                        {0x4C5F, 0x2F38B, 0xd9999a, 0x2666} };
}

void rp_osc_fil_default (rp_osc_fil_t *handle) {
    handle->regset->cfg_byp = 0;
    handle->regset->cfg_faa = 0;  // TODO: is this really the default?
    handle->regset->cfg_fbb = 0;  // TODO: is this really the default?
    handle->regset->cfg_fkk = 0;  // TODO: is this really the default?
    handle->regset->cfg_fpp = 0;  // TODO: is this really the default?
}

void rp_osc_fil_print (rp_osc_fil_t *handle) {
    printf("osc_fil.cfg_byp = %08x\n", handle->regset->cfg_byp);
    printf("osc_fil.cfg_faa = %08x\n", handle->regset->cfg_faa);
    printf("osc_fil.cfg_fbb = %08x\n", handle->regset->cfg_fbb);
    printf("osc_fil.cfg_fkk = %08x\n", handle->regset->cfg_fkk);
    printf("osc_fil.cfg_fpp = %08x\n", handle->regset->cfg_fpp);
}

bool rp_osc_fil_get_filter_bypass (rp_osc_fil_t *handle) {
    return (bool) handle->regset->cfg_byp;
}

int rp_osc_fil_set_filter_bypass (rp_osc_fil_t *handle, bool value) {
    handle->regset->cfg_byp = (uint32_t) value;
    return(0);
}

int rp_osc_fil_get_filter_coeficients (rp_osc_fil_t *handle, rp_osc_fil_coeficients_t *value) {
    value->aa = handle->regset->cfg_faa;
    value->bb = handle->regset->cfg_fbb;
    value->kk = handle->regset->cfg_fkk;
    value->pp = handle->regset->cfg_fpp;
    return(0);
}

int rp_osc_fil_set_filter_coeficients (rp_osc_fil_t *handle, rp_osc_fil_coeficients_t *value) {
    handle->regset->cfg_faa = value->aa;
    handle->regset->cfg_fbb = value->bb;
    handle->regset->cfg_fkk = value->kk;
    handle->regset->cfg_fpp = value->pp;
    return(0);
}

