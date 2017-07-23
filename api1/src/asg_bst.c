#include <stdio.h>

#include "redpitaya/util.h"
#include "redpitaya/asg_bst.h"

void rp_asg_bst_init (rp_asg_bst_t *handle, volatile rp_asg_bst_regset_t *regset, double FS, int unsigned buffer_size, const fixp_t bdr_t, const fixp_t bpl_t, const fixp_t bpn_t) {
    handle->regset = regset;
    handle->FS = FS;
    handle->buffer_size = buffer_size;
    handle->bdr_t = bdr_t;
    handle->bpl_t = bpl_t;
    handle->bpn_t = bpn_t;
}

void rp_asg_bst_default (rp_asg_bst_t *handle) {
    handle->regset->cfg_bdr = 0;
    handle->regset->cfg_bdl = 0;
    handle->regset->cfg_bpl = 0;
    handle->regset->cfg_bpn = 0;
}

void rp_asg_bst_print (rp_asg_bst_t *handle) {
    printf("asg_per.bdr_t = %s\n", rp_util_fixp_print(handle->bdr_t));
    printf("asg_per.bpl_t = %s\n", rp_util_fixp_print(handle->bpl_t));
    printf("asg_per.bpn_t = %s\n", rp_util_fixp_print(handle->bpn_t));
    printf("asg_per.cfg_bdr = %08x\n", handle->regset->cfg_bdr);
    printf("asg_per.cfg_bdl = %08x\n", handle->regset->cfg_bdl);
    printf("asg_per.cfg_bpl = %08x\n", handle->regset->cfg_bpl);
    printf("asg_per.cfg_bpn = %08x\n", handle->regset->cfg_bpn);
}

uint32_t rp_asg_bst_get_data_repetitions (rp_asg_bst_t *handle) {
    return(handle->regset->cfg_bdr + 1);
}

int rp_asg_bst_set_data_repetitions (rp_asg_bst_t *handle, uint32_t value) {
    if (value <= fixp_num(handle->bdr_t)) {
        handle->regset->cfg_bdr = value - 1;
        return(0);
    } else {
//      raise ValueError("Burst data repetitions should be less or equal to {}.".format(self._CWRr))
        return(-1);
    }
}

uint32_t rp_asg_bst_get_data_length (rp_asg_bst_t *handle) {
    return(handle->regset->cfg_bdl + 1);
}

int rp_asg_bst_set_data_length (rp_asg_bst_t *handle, uint32_t value) {
    if (value <= handle->buffer_size) {
        handle->regset->cfg_bdl = value - 1;
        return(0);
    } else {
//      raise ValueError("Burst data length should be less or equal to {}.".format(self.buffer_size))
        return(-1);
    }
}

// NOTE: this function has an overflow issue
uint32_t rp_asg_bst_get_period_length (rp_asg_bst_t *handle) {
    return(handle->regset->cfg_bpl + 1);
}

// NOTE: this function has an overflow issue
int rp_asg_bst_set_period_length (rp_asg_bst_t *handle, uint32_t value) {
    if (value <= fixp_num(handle->bpl_t)) {
        handle->regset->cfg_bpl = value - 1;
        return(0);
    } else {
//      raise ValueError("Burst period length should be less or equal to {}.".format(self._CWLr))
        return(-1);
    }
}

uint32_t rp_asg_bst_get_period_number (rp_asg_bst_t *handle) {
    return(handle->regset->cfg_bpn + 1);
}

int rp_asg_bst_set_period_number (rp_asg_bst_t *handle, uint32_t value) {
    if (value <= fixp_num(handle->bpn_t)) {
        handle->regset->cfg_bpn = value - 1;
        return(0);
    } else {
//      raise ValueError("Burst period number should be less or equal to {}.".format(self._CWNr))
        return(-1);
    }
}

