#include <stdbool.h>
#include <math.h>

#include "util.h"
#include "gen_out.h"

void rp_gen_out_init (rp_gen_out_t *handle, volatile rp_gen_out_regset_t *regset, const fixp_t mul_t, const fixp_t sum_t) {
    handle->regset = regset;
    handle->mul_t = mul_t;
    handle->sum_t = sum_t;
}

float    rp_gen_out_get_amplitude(rp_gen_out_t *handle) {
}

int      rp_gen_out_set_amplitude(rp_gen_out_t *handle, float value) {
    return(0);
}

float    rp_gen_out_get_offset   (rp_gen_out_t *handle) {
}

int      rp_gen_out_set_offset   (rp_gen_out_t *handle, float value) {
    return(0);
}

bool rp_gen_out_get_enable   (rp_gen_out_t *handle) {
}

void rp_gen_out_set_enable   (rp_gen_out_t *handle, bool value) {
}

