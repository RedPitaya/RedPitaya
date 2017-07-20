#ifndef OSC_FIL_H
#define OSC_FIL_H

#include <stdint.h>

typedef struct {
    int32_t aa;  // AA coeficient
    int32_t bb;  // BB coeficient
    int32_t kk;  // KK coeficient
    int32_t pp;  // PP coeficient
} rp_osc_fil_coeficients_t;

typedef struct {
    uint32_t cfg_byp;  // multiplier (amplitude)
     int32_t cfg_faa;  // AA coeficient
     int32_t cfg_fbb;  // BB coeficient
     int32_t cfg_fkk;  // KK coeficient
    uint32_t cfg_fpp;  // PP coeficient
} rp_osc_fil_regset_t;

typedef struct {
    volatile rp_osc_fil_regset_t *regset;
    rp_osc_fil_coeficients_t filters[2];
} rp_osc_fil_t;

void     rp_osc_fil_init         (rp_osc_fil_t *handle, volatile rp_osc_fil_regset_t *regset);
void     rp_osc_fil_default      (rp_osc_fil_t *handle);
void     rp_osc_fil_print        (rp_osc_fil_t *handle);

bool     rp_osc_fil_get_filter_bypass      (rp_osc_fil_t *handle);
int      rp_osc_fil_set_filter_bypass      (rp_osc_fil_t *handle, bool value);
int      rp_osc_fil_get_filter_coeficients (rp_osc_fil_t *handle, rp_osc_fil_coeficients_t *value);
int      rp_osc_fil_set_filter_coeficients (rp_osc_fil_t *handle, rp_osc_fil_coeficients_t *value);

#endif

