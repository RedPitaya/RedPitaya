#ifndef LA_MSK_H
#define LA_MSK_H

#include <stdint.h>

typedef struct {
    uint32_t cfg_msk;  // input mask
    uint32_t cfg_pol;  // input polarity
} rp_la_msk_regset_t;

typedef struct {
    volatile rp_la_msk_regset_t *regset;
} rp_la_msk_t;

void     rp_la_msk_init                (rp_la_msk_t *handle, volatile rp_la_msk_regset_t *regset);
void     rp_la_msk_default             (rp_la_msk_t *handle);
void     rp_la_msk_print               (rp_la_msk_t *handle);

uint32_t rp_la_msk_get_input_mask      (rp_la_msk_t *handle);
int      rp_la_msk_set_input_mask      (rp_la_msk_t *handle, uint32_t value);
uint32_t rp_la_msk_get_input_polarity  (rp_la_msk_t *handle);
int      rp_la_msk_set_input_polarity  (rp_la_msk_t *handle, uint32_t value);

#endif

