#ifndef LA_RLE_H
#define LA_RLE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t cfg_rle;  // RLE mode
    uint32_t sts_cur;  // current counter
    uint32_t sts_lst;  // last    counter
} rp_la_rle_regset_t;

typedef struct {
    volatile rp_la_rle_regset_t *regset;
} rp_la_rle_t;

void     rp_la_rle_init                (rp_la_rle_t *handle, volatile rp_la_rle_regset_t *regset);
void     rp_la_rle_default             (rp_la_rle_t *handle);
void     rp_la_rle_print               (rp_la_rle_t *handle);

bool     rp_la_rle_get_rle             (rp_la_rle_t *handle);
int      rp_la_rle_set_rle             (rp_la_rle_t *handle, bool value);
uint32_t rp_la_rle_get_counter_current (rp_la_rle_t *handle);
uint32_t rp_la_rle_get_counter_last    (rp_la_rle_t *handle);

#endif

