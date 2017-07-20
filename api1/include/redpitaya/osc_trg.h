#ifndef OSC_TRG_H
#define OSC_TRG_H

#include <stdint.h>

#include "redpitaya/util.h"

enum edge {pos, neg};

typedef struct {
     int32_t cfg_neg;  // negative level
     int32_t cfg_pos;  // positive level
    uint32_t cfg_edg;  // edge (0-pos, 1-neg)
} rp_osc_trg_regset_t;

typedef struct {
    volatile rp_osc_trg_regset_t *regset;
    // fixed point data type
    fixp_t dat_t;
} rp_osc_trg_t;

void     rp_osc_trg_init                 (rp_osc_trg_t *handle, volatile rp_osc_trg_regset_t *regset, const fixp_t dat_t);
void     rp_osc_trg_default              (rp_osc_trg_t *handle);
void     rp_osc_trg_print                (rp_osc_trg_t *handle);

int32_t  rp_osc_trg_get_level_pos        (rp_osc_trg_t *handle);
int      rp_osc_trg_set_level_pos        (rp_osc_trg_t *handle,  int32_t value);
int32_t  rp_osc_trg_get_level_neg        (rp_osc_trg_t *handle);
int      rp_osc_trg_set_level_neg        (rp_osc_trg_t *handle,  int32_t value);
uint32_t rp_osc_trg_get_edge             (rp_osc_trg_t *handle);
int      rp_osc_trg_set_edge             (rp_osc_trg_t *handle, uint32_t value);

#endif

