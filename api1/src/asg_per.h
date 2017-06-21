    // buffer parameters (fixed point number uM.F)
#ifndef ASG_PER_H
#define ASG_PER_H

#include <stdint.h>

typedef struct {
    uint32_t cfg_siz;  // size
    uint32_t cfg_off;  // offset
    uint32_t cfg_ste;  // step
} rp_asg_per_regset_t;

typedef struct {
    rp_uio_t uio;
    volatile rp_asg_per_regset_t *regset;
    // buffer parameters (fixed point number uM.F)
    int unsigned CWM;  // counter width - magnitude (fixed point integer)
    int unsigned CWF;  // counter width - fraction  (fixed point fraction)
    int unsigned CW ;
} rp_asg_per_t;

#endif

