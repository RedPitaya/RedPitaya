#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "redpitaya/fixp.h"
#include "redpitaya/uio.h"
#include "redpitaya/evn.h"
#include "redpitaya/la.h"
#include "redpitaya/la_trg.h"
#include "redpitaya/la_rle.h"
#include "redpitaya/la_msk.h"

int rp_la_init (rp_la_t *handle) {
    static char path [] = "/dev/uio/la";

    // initialize constants
    handle->FS = 125000000.0;  // sampling frequency
    handle->buffer_size = 1<<14;

    int status = rp_uio_init (&handle->uio, path);
    if (status) {
        return (-1);
    }
    // map regset
    handle->regset = (rp_la_regset_t *) handle->uio.map[0].mem;
    // map buffer
    handle->buffer = (int16_t *) handle->uio.map[1].mem;

    // events
    rp_evn_init(&handle->evn, &handle->regset->evn);
    // acquire
    const fixp_t cnt_t = {.s = 0, .m = CW-1, .f = 0};
    rp_acq_init(&handle->acq, &handle->regset->acq, cnt_t);
    // trigger configuration
    rp_la_trg_init(&handle->trg, &handle->regset->trg);
    rp_la_rle_init(&handle->rle, &handle->regset->rle);
    rp_la_msk_init(&handle->msk, &handle->regset->msk);

    return(0);
}

int rp_la_release (rp_la_t *handle) {
    // reset hardware
    rp_evn_reset(&handle->evn);

    int status = rp_uio_release (&handle->uio);
    if (status) {
        return (-1);
    }

    return(0);
}

int rp_la_default (rp_la_t *handle) {
    // reset hardware
    rp_evn_reset(&handle->evn);
    // set acquire defaults
    rp_acq_default(&handle->acq);
    // set trigger defaults
    rp_la_trg_default(&handle->trg);
    rp_la_rle_default(&handle->rle);
    rp_la_msk_default(&handle->msk);
    // decimation
    handle->regset->cfg_dec = 0;

    return(0);
}

void rp_la_print (rp_la_t *handle) {
    printf("la.FS = %f\n", handle->FS);
    printf("la.buffer_size = %zu\n", handle->buffer_size);
    rp_evn_print(&handle->evn);
    rp_acq_print(&handle->acq);
    rp_la_trg_print(&handle->trg);
    rp_la_rle_print(&handle->rle);
    rp_la_msk_print(&handle->msk);
    printf("la.cfg_dec = %08x\n", handle->regset->cfg_dec);
}

int unsigned rp_la_get_decimation(rp_la_t *handle) {
    return(handle->regset->cfg_dec + 1);
}

void rp_la_set_decimation (rp_la_t *handle, int unsigned value) {
    // TODO check range
    handle->regset->cfg_dec = value - 1;
}

double rp_la_get_sample_rate(rp_la_t *handle) {
    return(handle->FS / (double) rp_la_get_decimation(handle));
}

double rp_la_get_sample_period(rp_la_t *handle) {
    return((double) rp_la_get_decimation(handle) / handle->FS);
}


size_t rp_la_get_pointer (rp_la_t *handle) {
    // mask out overflow bit and sum pre and post trigger counters
    size_t cnt = rp_acq_get_trigger_pre_status (&handle->acq)
               + rp_acq_get_trigger_post_status(&handle->acq);
    size_t adr = cnt % handle->buffer_size;
    return(adr);
}

size_t rp_la_get_data (rp_la_t *handle, float *data, size_t siz, size_t ptr) {
    // TODO
    if (ptr == 0) {
        ptr = rp_la_get_pointer(handle);
    }
//    size_t adr = (handle->buffer_size + ptr - siz) % handle->buffer_size;
    // TODO add loop
    // TODO: avoid making copy of entire array
//    table = np.roll(self.table, -ptr)
//    return (float) [-siz:] * (self.__input_range / self._DWr)
    return (0);
}

