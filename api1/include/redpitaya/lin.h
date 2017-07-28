#ifndef LIN_H
#define LIN_H

#include <stdint.h>

// single module regset structure
typedef struct {
    int32_t cfg_mul;  // multiplication
    int32_t cfg_sum;  // summation
} rp_lin_regset_t;

typedef struct {
    volatile rp_lin_regset_t *regset;
    fixp_t mul_t;
    fixp_t sum_t;
} rp_lin_t;

void     rp_lin_init   (rp_lin_t *handle, volatile rp_lin_regset_t *regset, const fixp_t mul_t, const fixp_t sum_t);
void     rp_lin_default(rp_lin_t *handle);
void     rp_lin_print  (rp_lin_t *handle);
    
float    rp_lin_chn_get_gain  (rp_lin_t *handle);
int      rp_lin_chn_set_gain  (rp_lin_t *handle, float value);
float    rp_lin_chn_get_offset(rp_lin_t *handle);
int      rp_lin_chn_set_offset(rp_lin_t *handle, float value);

#endif

