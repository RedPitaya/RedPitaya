#ifndef OSC_H
#define OSC_H

#include <stdint.h>
#include <stdbool.h>

#include "redpitaya/util.h"
#include "redpitaya/uio.h"
#include "redpitaya/evn.h"
#include "redpitaya/acq.h"
#include "redpitaya/osc_trg.h"
#include "redpitaya/osc_fil.h"

#define DWO 16

typedef struct {
    rp_evn_regset_t     evn;
    uint32_t            rsv0;     // reserved
    rp_acq_regset_t     acq;
    rp_osc_trg_regset_t trg;
    uint32_t            rsv1;     // reserved
    // decimation
    uint32_t            cfg_dec;  // decimation factor
    uint32_t            cfg_shr;  // shift right
    uint32_t            cfg_avg;  // average enable
    rp_osc_fil_regset_t fil;
} rp_osc_regset_t;

typedef struct {
    rp_uio_t uio;
    volatile rp_osc_regset_t *regset;
    volatile int16_t         *buffer;
    rp_evn_t     evn;
    rp_acq_t     acq;
    rp_osc_trg_t trg;
    rp_osc_fil_t fil;
    // sampling frequency
    double       FS;
    // table size
    size_t       buffer_size;
    // data fixed point size
    fixp_t       dat_t;
    float        input_rangen;
    float       *input_ranges;
    float        input_range;
} rp_osc_t;

int           rp_osc_init        (rp_osc_t *handle, const int unsigned index);
int           rp_osc_release     (rp_osc_t *handle);
int           rp_osc_default     (rp_osc_t *handle);
void          rp_osc_print       (rp_osc_t *handle);

float         rp_osc_get_input_range  (rp_osc_t *handle);
void          rp_osc_set_input_range  (rp_osc_t *handle, float value);
int unsigned  rp_osc_get_decimation   (rp_osc_t *handle);
void          rp_osc_set_decimation   (rp_osc_t *handle, int unsigned value);
double        rp_osc_get_sample_rate  (rp_osc_t *handle);
double        rp_osc_get_sample_period(rp_osc_t *handle);
bool          rp_osc_get_average      (rp_osc_t *handle);
void          rp_osc_set_average      (rp_osc_t *handle, bool value);
size_t        rp_osc_get_pointer      (rp_osc_t *handle);
size_t        rp_osc_get_data         (rp_osc_t *handle, float *data, size_t siz, size_t ptr);

#endif

