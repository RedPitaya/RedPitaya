#ifndef GEN_OUT_H
#define GEN_OUT_H

#include <stdint.h>

#include "util.h"

#define DWM 14
#define DWS 14

typedef struct {
     int32_t cfg_mul;  // multiplier (amplitude)
     int32_t cfg_sum;  // adder (offset)
    uint32_t cfg_ena;  // output enable
} rp_gen_out_regset_t;

typedef struct {
    volatile rp_gen_out_regset_t *regset;
    // linear addition multiplication register fixed point type
    fixp_t mul_t;  // linear gain multiplier
    fixp_t sum_t;  // linear offset summand
} rp_gen_out_t;

void     rp_gen_out_init         (rp_gen_out_t *handle, volatile rp_gen_out_regset_t *regset, const fixp_t mul_t, const fixp_t sum_t);
void     rp_gen_out_default      (rp_gen_out_t *handle);
void     rp_gen_out_print        (rp_gen_out_t *handle);

float    rp_gen_out_get_amplitude(rp_gen_out_t *handle);
int      rp_gen_out_set_amplitude(rp_gen_out_t *handle, float value);
float    rp_gen_out_get_offset   (rp_gen_out_t *handle);
int      rp_gen_out_set_offset   (rp_gen_out_t *handle, float value);
bool     rp_gen_out_get_enable   (rp_gen_out_t *handle);
void     rp_gen_out_set_enable   (rp_gen_out_t *handle, bool value);

#endif

