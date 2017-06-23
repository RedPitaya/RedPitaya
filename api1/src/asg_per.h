#ifndef ASG_PER_H
#define ASG_PER_H

#include <stdint.h>

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
    int unsigned CWM;  // counter width - magnitude (fixed point integer)
    int unsigned CWF;  // counter width - fraction  (fixed point fraction)
    int unsigned CW ;
} rp_asg_per_t;

void     rp_asg_per_init (rp_asg_per_t *handle, volatile rp_asg_per_regset_t *regset, double FS, int unsigned buffer_size, int unsigned CWM, int unsigned CWF);

uint32_t rp_asg_per_get_table_size(rp_asg_per_t *handle);
int      rp_asg_per_set_table_size(rp_asg_per_t *handle, uint32_t value);
double   rp_asg_gen_get_frequency (rp_asg_per_t *handle);
int      rp_asg_gen_set_frequency (rp_asg_per_t *handle, double value);
double   rp_asg_gen_get_phase     (rp_asg_per_t *handle);
int      rp_asg_gen_set_phase     (rp_asg_per_t *handle, double value);

#endif

