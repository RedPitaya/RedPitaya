#ifndef LG_OUT_H
#define LG_OUT_H

#include <stdint.h>

#include "redpitaya/fixp.h"

#define DWM 14
#define DWS 14

typedef struct {
    uint32_t cfg_oen[2];  // output enable [0,1]
    uint32_t cfg_msk;     // mask
    uint32_t cfg_val;     // value/polarity
} rp_lg_out_regset_t;

typedef struct {
    volatile rp_lg_out_regset_t *regset;
    fixp_t dat_t;
} rp_lg_out_t;

void     rp_lg_out_init       (rp_lg_out_t *handle, volatile rp_lg_out_regset_t *regset, const fixp_t dat_t);
void     rp_lg_out_default    (rp_lg_out_t *handle);
void     rp_lg_out_print      (rp_lg_out_t *handle);

void     rp_lg_out_get_enable (rp_lg_out_t *handle, uint32_t *value[2]);
int      rp_lg_out_set_enable (rp_lg_out_t *handle, uint32_t  value[2]);
uint32_t rp_lg_out_get_mask   (rp_lg_out_t *handle);
int      rp_lg_out_set_mask   (rp_lg_out_t *handle, uint32_t value);
uint32_t rp_lg_out_get_value  (rp_lg_out_t *handle);
int      rp_lg_out_set_value  (rp_lg_out_t *handle, uint32_t value);

#endif

