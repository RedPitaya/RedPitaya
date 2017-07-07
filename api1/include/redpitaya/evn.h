#ifndef EVN_H
#define EVN_H

#include <stdint.h>
#include <stdbool.h>

// control register masks
#define CTL_TRG_MASK  (1<<3)  // sw trigger bit (sw trigger must be enabled)
#define CTL_STP_MASK  (1<<2)  // stop/abort; returns 1 when stopped
#define CTL_STR_MASK  (1<<1)  // start
#define CTL_RST_MASK  (1<<0)  // reset state machine so that it is in known state

typedef struct {
    uint32_t ctl_sts;  // control/status register
    uint32_t cfg_evn;  // software event source select
    uint32_t cfg_trg;  // hardware trigger mask
} rp_evn_regset_t;

typedef struct {
    volatile rp_evn_regset_t *regset;
} rp_evn_t;

void     rp_evn_init           (rp_evn_t *handle, volatile rp_evn_regset_t *regset);
void     rp_evn_print          (rp_evn_t *handle);

void     rp_evn_reset          (rp_evn_t *handle);
void     rp_evn_start          (rp_evn_t *handle);
void     rp_evn_stop           (rp_evn_t *handle);
void     rp_evn_trigger        (rp_evn_t *handle);

bool     rp_evn_status_run     (rp_evn_t *handle);
bool     rp_evn_status_trigger (rp_evn_t *handle);

uint32_t rp_evn_get_sync_src   (rp_evn_t *handle);
void     rp_evn_set_sync_src   (rp_evn_t *handle, uint32_t value);

uint32_t rp_evn_get_trig_src   (rp_evn_t *handle);
void     rp_evn_set_trig_src   (rp_evn_t *handle, uint32_t value);

#endif
