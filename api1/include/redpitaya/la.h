#ifndef LA_H
#define LA_H

#include <stdint.h>
#include <stdbool.h>

#include "redpitaya/util.h"
#include "redpitaya/uio.h"
#include "redpitaya/evn.h"
#include "redpitaya/acq.h"
#include "redpitaya/la_trg.h"

#define DWL 16

typedef struct {
    rp_evn_regset_t     evn;
    uint32_t            rsv0;     // reserved
    rp_acq_regset_t     acq;
    rp_la_trg_regset_t  trg;
    uint32_t            rsv1;     // reserved
    // decimation
    uint32_t            cfg_dec;  // decimation factor
} rp_la_regset_t;

typedef struct {
    rp_uio_t uio;
    volatile rp_la_regset_t  *regset;
    volatile int16_t         *buffer;
    rp_evn_t     evn;
    rp_acq_t     acq;
    rp_la_trg_t  trg;
    // sampling frequency
    double       FS;
    // table size
    size_t       buffer_size;
} rp_la_t;

int           rp_la_init        (rp_la_t *handle);
int           rp_la_release     (rp_la_t *handle);
int           rp_la_default     (rp_la_t *handle);
void          rp_la_print       (rp_la_t *handle);

int unsigned  rp_la_get_decimation   (rp_la_t *handle);
void          rp_la_set_decimation   (rp_la_t *handle, int unsigned value);
double        rp_la_get_sample_rate  (rp_la_t *handle);
double        rp_la_get_sample_period(rp_la_t *handle);
size_t        rp_la_get_pointer      (rp_la_t *handle);
size_t        rp_la_get_data         (rp_la_t *handle, float *data, size_t siz, size_t ptr);

#endif

