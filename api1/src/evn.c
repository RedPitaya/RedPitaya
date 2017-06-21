#include <stdint.h>
#include <stdbool.h>

#include "evn.h"

extern void     rp_evn_reset          (rp_evn_regset_t *regset);
extern void     rp_evn_start          (rp_evn_regset_t *regset);
extern void     rp_evn_stop           (rp_evn_regset_t *regset);
extern void     rp_evn_trigger        (rp_evn_regset_t *regset);

extern bool     rp_evn_status_run     (rp_evn_regset_t *regset);
extern bool     rp_evn_status_trigger (rp_evn_regset_t *regset);

extern uint32_t rp_evn_get_sync_src   (rp_evn_regset_t *regset);
extern void     rp_evn_set_sync_src   (rp_evn_regset_t *regset, uint32_t value);

extern uint32_t rp_evn_get_trig_src   (rp_evn_regset_t *regset);
extern void     rp_evn_set_trig_src   (rp_evn_regset_t *regset, uint32_t value);

