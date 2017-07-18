#ifndef LA_TRG_H
#define LA_TRG_H

#include <stdint.h>

#include "redpitaya/util.h"

enum edge {pos, neg};

typedef struct {
    uint32_t cfg_msk;  // comparator mask
    uint32_t cfg_val;  // comparator value
    uint32_t cfg_pos;  // edge positive
    uint32_t cfg_neg;  // edge negative
} rp_la_trg_regset_t;

typedef struct {
    volatile rp_la_trg_regset_t *regset;
} rp_la_trg_t;

void     rp_la_trg_init                 (rp_la_trg_t *handle, volatile rp_la_trg_regset_t *regset);
void     rp_la_trg_default              (rp_la_trg_t *handle);
void     rp_la_trg_print                (rp_la_trg_t *handle);

uint32_t rp_la_trg_get_mask             (rp_la_trg_t *handle);
int      rp_la_trg_set_mask             (rp_la_trg_t *handle, uint32_t value);
uint32_t rp_la_trg_get_value            (rp_la_trg_t *handle);
int      rp_la_trg_set_value            (rp_la_trg_t *handle, uint32_t value);
uint32_t rp_la_trg_get_edge_pos         (rp_la_trg_t *handle);
int      rp_la_trg_set_edge_pos         (rp_la_trg_t *handle, uint32_t value);
uint32_t rp_la_trg_get_edge_neg         (rp_la_trg_t *handle);
int      rp_la_trg_set_edge_neg         (rp_la_trg_t *handle, uint32_t value);

#endif

