#ifndef ASG_PER_H
#define ASG_PER_H

#include <stdint.h>

#include "redpitaya/util.h"

#define CWM 14
#define CWF 16

typedef struct {
    uint32_t cfg_siz;  // size
    uint32_t cfg_off;  // offset
    uint32_t cfg_ste;  // step
} rp_asg_per_regset_t;

typedef struct {
    volatile rp_asg_per_regset_t *regset;
    double FS;
    int unsigned buffer_size;
    // buffer parameters (fixed point number uM.F)
    fixp_t cnt_t;
} rp_asg_per_t;

void     rp_asg_per_init          (rp_asg_per_t *handle, volatile rp_asg_per_regset_t *regset, double FS, int unsigned buffer_size, const fixp_t cnt_t);
void     rp_asg_per_default       (rp_asg_per_t *handle);
void     rp_asg_per_print         (rp_asg_per_t *handle);

uint32_t rp_asg_per_get_table_size(rp_asg_per_t *handle);
int      rp_asg_per_set_table_size(rp_asg_per_t *handle, uint32_t value);
double   rp_asg_gen_get_frequency (rp_asg_per_t *handle);
int      rp_asg_gen_set_frequency (rp_asg_per_t *handle, double value);
double   rp_asg_gen_get_phase     (rp_asg_per_t *handle);
int      rp_asg_gen_set_phase     (rp_asg_per_t *handle, double value);

#endif

