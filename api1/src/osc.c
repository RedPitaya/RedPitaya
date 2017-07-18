#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "redpitaya/util.h"
#include "redpitaya/uio.h"
#include "redpitaya/evn.h"
#include "redpitaya/osc.h"
#include "redpitaya/osc_trg.h"

int rp_osc_init (rp_osc_t *handle, const int unsigned index) {
    static char path_osc [] = "/dev/uio/osc";
    size_t len = strlen(path_osc)+1+1;
    char path[len];

    // initialize constants
    handle->FS = 125000000.0;  // sampling frequency
    handle->buffer_size = 1<<14;
    handle->dat_t = (fixp_t) {.s = 1, .m = 0, .f = DWO-1};

    // TODO: generalize this code
    handle->input_rangen = 2;
    handle->input_ranges = malloc(handle->input_rangen * sizeof(float));
    handle->input_ranges [0] =  1.0;
    handle->input_ranges [1] = 20.0;

    // a single character is reserved for the index
    // so indexes above 9 are an error
    if (index>9) {
        fprintf(stderr, "OSC: failed device index decimal representation is limited to 1 digit.\n");
        return (-1);
    }
    snprintf(path, len, "%s%u", path_osc, index);
    
    int status = rp_uio_init (&handle->uio, path);
    if (status) {
        return (-1);
    }
    // map regset
    handle->regset = (rp_osc_regset_t *) handle->uio.map[0].mem;
    // map buffer
    handle->buffer = (int16_t *) handle->uio.map[1].mem;

    // events
    rp_evn_init(&handle->evn, &handle->regset->evn);
    // acquire
    const fixp_t cnt_t = {.s = 0, .m = CW-1, .f = 0};
    rp_acq_init(&handle->acq, &handle->regset->acq, cnt_t);
    // trigger configuration
    rp_osc_trg_init(&handle->trg, &handle->regset->trg, handle->dat_t);
    // filter configuration
    rp_osc_fil_init(&handle->fil, &handle->regset->fil);

    return(0);
}

int rp_osc_release (rp_osc_t *handle) {
    free(handle->input_ranges);
    // reset hardware
    rp_evn_reset(&handle->evn);

    int status = rp_uio_release (&handle->uio);
    if (status) {
        return (-1);
    }

    return(0);
}

int rp_osc_default (rp_osc_t *handle) {
    // reset hardware
    rp_evn_reset(&handle->evn);
    // set acquire defaults
    rp_acq_default(&handle->acq);
    // set trigger defaults
    rp_osc_trg_default(&handle->trg);
    // decimation
    handle->regset->cfg_dec = 0;
    handle->regset->cfg_shr = 0;
    handle->regset->cfg_avg = 0;
    // set filter defaults
    rp_osc_fil_default(&handle->fil);

    return(0);
}

void rp_osc_print (rp_osc_t *handle) {
    printf("osc.FS = %f\n", handle->FS);
    printf("osc.buffer_size = %lu\n", handle->buffer_size);
    printf("osc.dat_t = %s\n", rp_util_fixp_print(handle->dat_t));
    rp_evn_print(&handle->evn);
    rp_acq_print(&handle->acq);
    rp_osc_trg_print(&handle->trg);
    printf("osc.cfg_dec = %08x\n", handle->regset->cfg_dec);
    printf("osc.cfg_shr = %08x\n", handle->regset->cfg_shr);
    printf("osc.cfg_avg = %08x\n", handle->regset->cfg_avg);
    rp_osc_fil_print(&handle->fil);
}

float rp_osc_get_input_range(rp_osc_t *handle) {
    return (handle->input_range);
}

void rp_osc_set_input_range(rp_osc_t *handle, float value) {
    bool match = false;
    int unsigned index = 0;
    for (int unsigned i=0; i<handle->input_rangen; i++) {
        if (handle->input_ranges[i] == value) {
            match = true;
            index = i;
        }
    }
    if (match) {
        handle->input_range = value;
        rp_osc_fil_set_filter_coeficients(&handle->fil, &handle->fil.filters[index]);
    } else {
//        raise ValueError("Input range can be one of {} volts.".format(self.ranges))
    }
}

int unsigned rp_osc_get_decimation(rp_osc_t *handle) {
    return(handle->regset->cfg_dec + 1);
}

void rp_osc_set_decimation (rp_osc_t *handle, int unsigned value) {
    // TODO check range
    handle->regset->cfg_dec = value - 1;
}

double rp_osc_get_sample_rate(rp_osc_t *handle) {
    return(handle->FS / (double) rp_osc_get_decimation(handle));
}

double rp_osc_get_sample_period(rp_osc_t *handle) {
    return((double) rp_osc_get_decimation(handle) / handle->FS);
}

bool rp_osc_get_average (rp_osc_t *handle) {
    return((bool) handle->regset->cfg_avg);
}

void rp_osc_set_average (rp_osc_t *handle, bool value) {
    // TODO check range, for non 2**n decimation factors,
    // scaling should be applied in addition to shift
    handle->regset->cfg_avg = (uint32_t) value;
    handle->regset->cfg_shr = ceil(log2(rp_osc_get_decimation(handle)));
}

size_t rp_osc_get_pointer (rp_osc_t *handle) {
    // mask out overflow bit and sum pre and post trigger counters
    size_t cnt = rp_acq_get_trigger_pre_status (&handle->acq)
               + rp_acq_get_trigger_post_status(&handle->acq);
    size_t adr = cnt % handle->buffer_size;
    return(adr);
}

size_t rp_osc_get_data (rp_osc_t *handle, float *data, size_t siz, size_t ptr) {
        // TODO
        if (ptr == 0) {
            ptr = rp_osc_get_pointer(handle);
        }
//        size_t adr = (handle->buffer_size + ptr - siz) % handle->buffer_size;
        // TODO add loop
        // TODO: avoid making copy of entire array
//        table = np.roll(self.table, -ptr)
//        return (float) [-siz:] * (self.__input_range / self._DWr)
        return (0);
}
