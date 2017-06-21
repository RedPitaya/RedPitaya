#ifndef GEN_H
#define GEN_H

#include <stdint.h>

#include "uio.h"
#include "evn.h"

typedef struct {
    rp_evn_regset_t     evn;
    uint32_t            rsv0;     // reserved
    uint32_t            cfg_bmd;  // mode [1:0] = [inf, ben]
//    rp_asg_per_regset_t per;
//    rp_asg_bst_regset_t bst;
    uint32_t            rsv1[2];  // reserved
//    rp_gen_out_regset_t out;
} rp_gen_regset_t;

typedef struct {
    rp_uio_t uio;
    volatile rp_gen_regset_t *regset;
    volatile int16_t         *table;
    // sampling frequency
    double       FS;
    // linear addition multiplication register width
    int unsigned DW ;  // data width - streaming sample
    int unsigned DWM;  // data width - linear gain multiplier
    int unsigned DWS;  // data width - linear offset summand
    // buffer parameters (fixed point number uM.F)
    int unsigned CWM;  // counter width - magnitude (fixed point integer)
    int unsigned CWF;  // counter width - fraction  (fixed point fraction)
    int unsigned CW ;
    // buffer counter ranges
    int unsigned buffer_size; // table size
    // burst counter parameters
    int unsigned CWR;  // counter width - burst data repetition
    int unsigned CWL;  // counter width - burst period length
    int unsigned CWN;  // counter width - burst period number
} rp_gen_t;

int rp_gen_init    (rp_gen_t *handle, const int unsigned index);
int rp_gen_release (rp_gen_t *handle);
    
void rp_gen_reset   (rp_gen_t *handle);
void rp_gen_start   (rp_gen_t *handle);
void rp_gen_stop    (rp_gen_t *handle);
void rp_gen_trigger (rp_gen_t *handle);

int rp_gen_set_enable(rp_gen_t *handle, bool value);

#endif

