#include <math.h>

#include "util.h"
#include "asg_per.h"

void rp_asg_per_init (rp_asg_per_t *handle, volatile rp_asg_per_regset_t *regset, double FS, int unsigned buffer_size, const fixp_t cnt_t) {
    handle->regset = regset;
    handle->FS = FS;
    handle->buffer_size = buffer_size;
    handle->cnt_t = cnt_t;
}

uint32_t rp_asg_per_get_table_size(rp_asg_per_t *handle) {
    return((handle->regset->cfg_siz + 1) >> handle->cnt_t.f);
}

int rp_asg_per_set_table_size(rp_asg_per_t *handle, uint32_t value) {
    if (value <= handle->buffer_size) {
        handle->regset->cfg_siz = (value << handle->cnt_t.f) - 1;
        return(0);
    } else {
//        raise ValueError("Waveform table size should not excede buffer size. buffer_size = {}".format(self.buffer_size))
        return(-1);
    }
}

double rp_asg_gen_get_frequency(rp_asg_per_t *handle) {
    // uint32_t might not be enough to hold register value + 1
    double siz = handle->regset->cfg_siz + 1;
    double ste = handle->regset->cfg_ste + 1;
    return (ste / siz * handle->FS);
}

int rp_asg_gen_set_frequency(rp_asg_per_t *handle, double value) {
    if (value < handle->FS) {
        double siz = handle->regset->cfg_siz + 1;
        handle->regset->cfg_ste = (uint32_t) (siz * (value / handle->FS)) - 1;
        return(0);
    } else {
//            raise ValueError("Frequency should be less then half the sample rate. f < FS/2 = {} Hz".format(self.FS/2))
      return(-1);
    }
}

double rp_asg_gen_get_phase(rp_asg_per_t *handle) {
    // uint32_t might not be enough to hold register value + 1
    double siz = handle->regset->cfg_siz + 1;
    double off = handle->regset->cfg_off;
    return (off / siz * 360.0);
}

int rp_asg_gen_set_phase(rp_asg_per_t *handle, double value) {
    double siz = handle->regset->cfg_siz + 1;
    handle->regset->cfg_off = (uint32_t) (siz * fmod(value, 360.0) / 360.0);
    return(0);
}

