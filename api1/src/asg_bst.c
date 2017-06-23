#include "util.h"
#include "asg_bst.h"

void rp_asg_bst_init (rp_asg_bst_t *handle, volatile rp_asg_bst_regset_t *regset, double FS, int unsigned buffer_size, int unsigned CWR, int unsigned CWL, int unsigned CWN) {
    handle->regset = regset;
    handle->FS = FS;
    handle->buffer_size = buffer_size;
    handle->CWR = CWR;
    handle->CWL = CWL;
    handle->CWN = CWN;
}

uint32_t rp_asg_bst_get_data_repetitions (rp_asg_bst_t *handle) {
    return(handle->regset->cfg_bdr + 1);
}

int rp_asg_bst_set_data_repetitions (rp_asg_bst_t *handle, uint32_t value) {
    if (value <= fixp_num(handle->CWR)) {
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
    if (value <= fixp_num(handle->CWL)) {
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
    if (value <= fixp_num(handle->CWN)) {
        handle->regset->cfg_bpn = value - 1;
        return(0);
    } else {
//      raise ValueError("Burst period number should be less or equal to {}.".format(self._CWNr))
        return(-1);
    }
}

