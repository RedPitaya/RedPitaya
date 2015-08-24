/** @file worker.h
 *
 * $Id: main_osc.h 881 2013-12-16 05:37:34Z rp_jmenart $
 * 
 * @brief Red Pitaya Oscilloscope worker module.
 * 
 * @Author Jure Menart <juremenart@gmail.com>
 *
 * This is C header file. It includes various defines, structure definitions and
 * function declarations for main oscilloscope module. Please visit:
 * http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language and principle used herein.
 * 
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __WORKER_H
#define __WORKER_H

#include "main_osc.h"

/** @defgroup worker_h worker_h
 * @{
 */

/** @brief Worker state enumenartion
 *
 * Enumeration used by main and worker modules to set workers module operation.
 */
typedef enum rp_osc_worker_state_e {
    /** do nothing, idling */
    rp_osc_idle_state = 0, 
    /** requests an shutdown of the worker thread */
    rp_osc_quit_state,
    /* abort current measurement */
    rp_osc_abort_state,
    /* auto mode acquisition - continuous measurements without trigger */ 
    rp_osc_auto_state, 
    /* normal mode - continuous measurements with trigger */
    rp_osc_normal_state,
    /* single mode - one measurement with trigger then go to idle state */
    rp_osc_single_state,
    /* non existing state - just to define the end of enumeration - must be
     * always last in the enumeration */
    rp_osc_nonexisting_state
} rp_osc_worker_state_t;

/** @} */

int rp_osc_worker_init(void);
int rp_osc_worker_exit(void);
int rp_osc_worker_change_state(rp_osc_worker_state_t new_state);
int rp_osc_worker_update_params(rp_osc_params_t *params, int fpga_update);

/* removes 'dirty' flags */
int rp_osc_clean_signals(void);
/* Returns:
 *  0 - new signals (dirty signal) are copied to the output 
 *  1 - no new signals available (dirty signal was not set - we need to wait)
 */
int rp_osc_get_signals(float ***signals, int *sig_idx);
/* Fills the output signal structure from temp one after calculation is done 
 * and marks it dirty 
 */
int rp_osc_set_signals(float **source, int index);

/* Prepares time vector (only where there is a need for it) */
int rp_osc_prepare_time_vector(float **out_signal, int dec_factor,
                               float t_start, float t_stop, int time_unit);

/* 16k -> 2k decimation
 * out_signal - needs to be already allocated, 2k float samples
 * in_signal  - 16k signal 
 * dec_factor - set in FPGA
 * t_start    - user set start time
 * t_stop     - user set stop time
 * TODO: Remove time vector generation from these functions, it should
 * be created at the beginning
 */
int rp_osc_decimate(float **cha_signal, int *in_cha_signal,
                    float **chb_signal, int *in_chb_signal,
                    float **time_signal, int dec_factor, 
                    float t_start, float t_stop, int time_unit);

int rp_osc_decimate_partial(float **cha_out_signal, int *cha_in_signal, 
                            float **chb_out_signal, int *chb_in_signal,
                            float **time_out_signal, int *next_wr_ptr, 
                            int last_wr_ptr, int step_wr_ptr, int next_out_idx,
                            float t_start, int dec_factor, int time_unit);


/* helper function - returns the factor for time unit conversion */
int rp_osc_get_time_unit_factor(int time_unit);

#endif /* __WORKER_H*/
