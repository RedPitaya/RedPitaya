#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "redpitaya/fixp.h"
#include "redpitaya/gen_out.h"

void rp_gen_out_init(rp_gen_out_t *handle, volatile rp_gen_out_regset_t *regset, const fixp_t mul_t, const fixp_t sum_t) {
    handle->regset = regset;
    handle->mul_t = mul_t;
    handle->sum_t = sum_t;
}

void rp_gen_out_default(rp_gen_out_t *handle) {
    handle->regset->cfg_mul = fixp_unit(handle->mul_t);
    handle->regset->cfg_sum = 0;
    handle->regset->cfg_ena = 0;
}

void rp_gen_out_print(rp_gen_out_t *handle) {
    printf("mul_t = %s\n", rp_util_fixp_print(handle->mul_t));
    printf("sum_t = %s\n", rp_util_fixp_print(handle->sum_t));
    printf("gen_out.cfg_mul = %08x\n", handle->regset->cfg_mul);
    printf("gen_out.cfg_sum = %08x\n", handle->regset->cfg_sum);
    printf("gen_out.cfg_ena = %08x\n", handle->regset->cfg_ena);
}

float rp_gen_out_get_amplitude(rp_gen_out_t *handle) {
    return((float) handle->regset->cfg_mul / (float) fixp_unit(handle->mul_t));
}

int rp_gen_out_set_amplitude(rp_gen_out_t *handle, float value) {
    if ((-1.0 <= value) || (value <= 1.0)) {
        handle->regset->cfg_mul = (int32_t) (value * (float) fixp_unit(handle->mul_t));
        return(0);
    } else {
//        raise ValueError("Output amplitude should be inside [-1,1] volts.")
        return(-1);
    }
}

float rp_gen_out_get_offset(rp_gen_out_t *handle) {
    return((float) handle->regset->cfg_sum / (float) fixp_max(handle->sum_t));
}

int rp_gen_out_set_offset(rp_gen_out_t *handle, float value) {
    if ((-1.0 <= value) || (value <= 1.0)) {
        handle->regset->cfg_sum = (int32_t) (value * (float) fixp_max(handle->sum_t));
        return(0);
    } else {
//        raise ValueError("Output offset should be inside [-1,1] volts.")
        return(-1);
    }
}

bool rp_gen_out_get_enable(rp_gen_out_t *handle) {
    return((bool) handle->regset->cfg_ena);
}

void rp_gen_out_set_enable(rp_gen_out_t *handle, bool value) {
    handle->regset->cfg_ena = value;
}

