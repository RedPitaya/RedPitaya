#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "redpitaya/fixp.h"
#include "redpitaya/uio.h"
#include "redpitaya/evn.h"
#include "redpitaya/lg.h"
#include "redpitaya/asg_per.h"
#include "redpitaya/asg_bst.h"
#include "redpitaya/lg_out.h"

int rp_lg_init (rp_lg_t *handle) {
    static char path [] = "/dev/uio/lg";
    size_t len = strlen(path_lg)+1+1;

    // initialize constants
    handle->FS = 125000000.0;  // sampling frequency
    handle->buffer_size = 1<<14;
    handle->dat_t = (fixp_t) {.s = 0, .m = 16};

    int status = rp_uio_init (&handle->uio, path);
    if (status) {
        return (-1);
    }
    // map regset
    handle->regset = (rp_lg_regset_t *) handle->uio.map[0].mem;
    // map table
    // TODO recode RTL for 16 memory access
    //handle->table = (int16_t *) handle->uio.map[1].mem;
    handle->table = (int32_t *) handle->uio.map[1].mem;

    // events
    rp_evn_init(&handle->evn, &handle->regset->evn);
    // burst mode
    const fixp_t bdr_t = {.s = 0, .m = CWR, .f = 0};
    const fixp_t bpl_t = {.s = 0, .m = CWL, .f = 0};
    const fixp_t bpn_t = {.s = 0, .m = CWN, .f = 0};
    rp_asg_bst_init(&handle->bst, &handle->regset->bst, handle->FS, handle->buffer_size, bdr_t, bpl_t, bpn_t);
    // output configuration
    const fixp_t mul_t = {.s = 1, .m = 1, .f = DWM-2};
    const fixp_t sum_t = {.s = 1, .m = 0, .f = DWS-1};
    rp_lg_out_init(&handle->out, &handle->regset->out, mul_t, sum_t);

    return(0);
}

int rp_lg_release (rp_lg_t *handle) {
    // disable output
    rp_lg_out_set_enable(&handle->out, false);
    // reset hardware
    rp_evn_reset(&handle->evn);

    int status = rp_uio_release (&handle->uio);
    if (status) {
        return (-1);
    }

    return(0);
}

int rp_lg_default (rp_lg_t *handle) {
    // reset hardware
    rp_evn_reset(&handle->evn);
    // set mode to continuous
    rp_lg_set_mode(handle, FINITE);
    // the waveform table is not cleared
    // set burst mode defaults
    rp_asg_bst_default(&handle->bst);
    // set output defaults
    rp_lg_out_default(&handle->out);

    return(0);
}

void rp_lg_print (rp_lg_t *handle) {
    printf("lg.FS = %f\n", handle->FS);
    printf("lg.buffer_size = %u\n", handle->buffer_size);
    printf("lg.dat_t = %s\n", rp_util_fixp_print(handle->dat_t));
    rp_evn_print(&handle->evn);
    printf("lg.cfg_bmd = %08x\n", handle->regset->cfg_bmd);
    rp_asg_bst_print(&handle->bst);
    rp_lg_out_print(&handle->out);
}

static inline int unsigned rp_lg_get_length(rp_lg_t *handle) {
    if (rp_lg_get_mode(handle) == CONTINUOUS) {
        return(rp_asg_per_get_table_size(&handle->per));
    } else {
        return(rp_asg_bst_get_data_length(&handle->bst));
    }
    return(0);
}

int rp_lg_get_waveform(rp_lg_t *handle, int16_t *waveform, int unsigned *len) {
    if (len == 0) {
        *len = rp_lg_get_length(handle);
    }
    if (*len <= handle->buffer_size) {
        for (int unsigned i=0; i<*len; i++) {
            waveform [i] = (int16_t) handle->table [i];
        }
        return(0);
    } else {
        return(-1);
    }
}

int rp_lg_set_waveform(rp_lg_t *handle, int16_t *waveform, const int unsigned len) {
    if (len <= handle->buffer_size) {
        for (int unsigned i=0; i<len; i++) {
            // TODO recode RTL for 16 memory access
            //handle->table [i] = (int16_t) waveform [i];
            handle->table [i] = (int32_t) waveform [i];
        }
        return(0);
    } else {
        return(-1);
    }
}

rp_lg_mode_t rp_lg_get_mode(rp_lg_t *handle) {
    return((rp_lg_mode_t) handle->regset->cfg_bmd);
}

void rp_lg_set_mode(rp_lg_t *handle, rp_lg_mode_t value) {
    handle->regset->cfg_bmd = (uint32_t) value;
}

