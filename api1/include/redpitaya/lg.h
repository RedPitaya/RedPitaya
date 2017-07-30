#ifndef LG_H
#define LG_H

#include <stdint.h>
#include <stdbool.h>

#include "redpitaya/uio.h"
#include "redpitaya/evn.h"
#include "redpitaya/asg_per.h"
#include "redpitaya/asg_bst.h"
#include "redpitaya/lg_out.h"

#define DW 16

typedef struct {
    rp_evn_regset_t     evn;
    uint32_t            rsv0;     // reserved
    uint32_t            cfg_bmd;  // mode [1] = [inf]
    rp_asg_bst_regset_t bst;
    uint32_t            rsv1[2];  // reserved
    rp_lg_out_regset_t  out;
} rp_lg_regset_t;

typedef struct {
    rp_uio_t uio;
    volatile rp_lg_regset_t *regset;
    // TODO recode RTL for 16 memory access
    //volatile int16_t         *table;
    volatile int32_t         *table;
    rp_evn_t     evn;
    rp_asg_bst_t bst;
    rp_lg_out_t  out;
    // sampling frequency
    double       FS;
    // table size
    int unsigned buffer_size;
    // data fixed point size
    fixp_t dat_t;
    // waveform status (used by API consumer)
    int32_t      tag;
    float        opt;
} rp_lg_t;

int          rp_lg_init        (rp_lg_t *handle);
int          rp_lg_release     (rp_lg_t *handle);
int          rp_lg_default     (rp_lg_t *handle);
void         rp_lg_print       (rp_lg_t *handle);

int          rp_lg_get_waveform(rp_lg_t *handle, uint16_t *waveform, int unsigned *len);
int          rp_lg_set_waveform(rp_lg_t *handle, uint16_t *waveform, int unsigned  len);
rp_lg_mode_t rp_lg_get_mode    (rp_lg_t *handle);
void         rp_lg_set_mode    (rp_lg_t *handle, rp_lg_mode_t value);

#endif

