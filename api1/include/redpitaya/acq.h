#ifndef ACQ_H
#define ACQ_H

#include <stdint.h>

#include "redpitaya/util.h"

#define CW 31

typedef struct {
    uint32_t cfg_pre;  // configuration pre  trigger
    uint32_t cfg_pst;  // configuration post trigger
    uint32_t sts_pre;  // status pre  trigger
    uint32_t sts_pst;  // status post trigger
} rp_acq_regset_t;

typedef struct {
    volatile rp_acq_regset_t *regset;
    // counter fixed point types
    fixp_t cnt_t;
} rp_acq_t;

void     rp_acq_init                 (rp_acq_t *handle, volatile rp_acq_regset_t *regset, const fixp_t cnt_t);
void     rp_acq_default              (rp_acq_t *handle);
void     rp_acq_print                (rp_acq_t *handle);

uint32_t rp_acq_get_trigger_pre      (rp_acq_t *handle);
int      rp_acq_set_trigger_pre      (rp_acq_t *handle, uint32_t value);
uint32_t rp_acq_get_trigger_post     (rp_acq_t *handle);
int      rp_acq_set_trigger_post     (rp_acq_t *handle, uint32_t value);
uint32_t rp_acq_get_trigger_pre_status  (rp_acq_t *handle);
uint32_t rp_acq_get_trigger_post_status (rp_acq_t *handle);

#endif

