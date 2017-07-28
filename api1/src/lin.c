#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "redpitaya/fixp.h"
#include "redpitaya/lin.h"

void rp_lin_init(rp_lin_t *handle, volatile rp_lin_regset_t *regset, const fixp_t mul_t, const fixp_t sum_t) {
    handle->regset = regset;
    handle->mul_t = mul_t;
    handle->sum_t = sum_t;
}

void rp_lin_default (rp_lin_t *handle) {
    handle->regset->cfg_mul = 0;
    handle->regset->cfg_sum = 0;
}

void rp_lin_print (rp_lin_t *handle) {
    printf("lin.mul_t = %s\n", rp_util_fixp_print(handle->mul_t));
    printf("lin.sum_t = %s\n", rp_util_fixp_print(handle->sum_t));
    printf("lin.cfg_mul = %08x\n", handle->regset->cfg_mul);
    printf("lin.cfg_sum = %08x\n", handle->regset->cfg_sum);
}

float rp_lin_get_gain (rp_lin_t *handle) {
    return((float) handle->regset->cfg_mul / (float) fixp_unit(handle->mul_t));
}

int rp_lin_set_gain (rp_lin_t *handle, float value) {
    if ((fixp_min(handle->mul_t) <= value) || (value <= fixp_max(handle->mul_t))) {
        handle->regset->cfg_mul = (int32_t) (value * (float) fixp_unit(handle->mul_t));
        return(0);
    } else {
//        raise ValueError("Gain should be inside [-1,1] volts.")
        return(-1);
    }
}

float rp_lin_get_offset(rp_lin_t *handle) {
    return((float) handle->regset->cfg_sum / (float) fixp_max(handle->sum_t));
}

int rp_lin_set_offset(rp_lin_t *handle, float value) {
    if ((-1.0 <= value) || (value <= 1.0)) {
        handle->regset->cfg_sum = (int32_t) (value * (float) fixp_max(handle->sum_t));
        return(0);
    } else {
//        raise ValueError("Offset should be inside [-1,1] volts.")
        return(-1);
    }
}

