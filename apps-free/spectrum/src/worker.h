/**
 * $Id: worker.h 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Spectrum Analyzer worker.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __WORKER_H
#define __WORKER_H

#include "main.h"

typedef enum rp_spectr_worker_state_e {
    rp_spectr_idle_state = 0, /* do nothing */
    rp_spectr_quit_state, /* shutdown worker */
    rp_spectr_abort_state, /* abort current measurement */
    rp_spectr_auto_state, /* auto mode acquisition */
    rp_spectr_nonexisting_state /* must be last */
} rp_spectr_worker_state_t;

/* Worker results (not signal but calculated peaks and jpeg index */
typedef struct rp_spectr_worker_res_s {
    int   jpg_idx;
    float peak_pw_cha;
    float peak_pw_freq_cha;
    float peak_pw_chb;
    float peak_pw_freq_chb;
} rp_spectr_worker_res_t;

int rp_spectr_worker_init(void);
int rp_spectr_worker_clean(void);
int rp_spectr_worker_exit(void);
int rp_spectr_worker_change_state(rp_spectr_worker_state_t new_state);
int rp_spectr_worker_update_params(rp_app_params_t *params, int fpga_update);

/* removes 'dirty' flags */
int rp_spectr_clean_signals(void);
/* Cleans up temporary directory (JPGs) */
int rp_spectr_clean_tmpdir(const char *dir);

/* Returns:
 *  0 - new signals (dirty signal) are copied to the output 
 *  1 - no new signals available (dirty signal was not set - we need to wait)
 */
int rp_spectr_get_signals(float ***signals, rp_spectr_worker_res_t *result);

/* Fills the output signal structure from temp one after calculation is done 
 * and marks it dirty 
 */
int rp_spectr_set_signals(float **source, rp_spectr_worker_res_t result);

#endif /* __WORKER_H*/
