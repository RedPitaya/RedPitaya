#ifndef GEN_H
#define GEN_H

#include <stdint.h>
#include <stdbool.h>

#include "redpitaya/util.h"
#include "redpitaya/uio.h"
#include "redpitaya/evn.h"
#include "redpitaya/asg_per.h"
#include "redpitaya/asg_bst.h"
#include "redpitaya/gen_out.h"

#define DW 14

typedef enum rp_gen_mode_t {
    CONTINUOUS = 0x0,
    FINITE     = 0x1,
    INFINITE   = 0x3
} rp_gen_mode_t;

typedef struct {
    rp_evn_regset_t     evn;
    uint32_t            rsv0;     // reserved
    uint32_t            cfg_bmd;  // mode [1:0] = [inf, ben]
    rp_asg_per_regset_t per;
    rp_asg_bst_regset_t bst;
    uint32_t            rsv1[2];  // reserved
    rp_gen_out_regset_t out;
} rp_gen_regset_t;

typedef struct {
    rp_uio_t uio;
    volatile rp_gen_regset_t *regset;
    // TODO recode RTL for 16 memory access
    //volatile int16_t         *table;
    volatile int32_t         *table;
    rp_evn_t     evn;
    rp_asg_per_t per;
    rp_asg_bst_t bst;
    rp_gen_out_t out;
    // sampling frequency
    double       FS;
    // table size
    int unsigned buffer_size;
    // data fixed point size
    fixp_t dat_t;
    // waveform status (used by API consumer)
    int32_t     tag;
    float       opt;
} rp_gen_t;

int           rp_gen_init        (rp_gen_t *handle, const int unsigned index);
int           rp_gen_release     (rp_gen_t *handle);
int           rp_gen_default     (rp_gen_t *handle);
void          rp_gen_print       (rp_gen_t *handle);

int           rp_gen_get_waveform(rp_gen_t *handle, float *waveform, int unsigned *len);
int           rp_gen_set_waveform(rp_gen_t *handle, float *waveform, int unsigned  len);
rp_gen_mode_t rp_gen_get_mode    (rp_gen_t *handle);
void          rp_gen_set_mode    (rp_gen_t *handle, rp_gen_mode_t value);

#endif

