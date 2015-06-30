/**
 * $Id: worker.h 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Oscilloscope worker.
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
#include "calib.h"

typedef enum rp_osc_worker_state_e {
    rp_osc_idle_state = 0, /* do nothing */
    rp_osc_quit_state, /* shutdown worker */
    rp_osc_abort_state, /* abort current measurement */
    rp_osc_auto_state, /* auto mode acquisition */
    rp_osc_normal_state, /* normal mode */
    rp_osc_single_state, /* single acq., automatically goes to idle */
    rp_osc_auto_set_state, /* runs auto-set algorithm */
    rp_osc_nonexisting_state /* must be last */
} rp_osc_worker_state_t;

int rp_osc_worker_init(rp_app_params_t *params, int params_len,
                       rp_calib_params_t *calib_params);
int rp_osc_worker_exit(void);
int rp_osc_worker_change_state(rp_osc_worker_state_t new_state);
int rp_osc_worker_get_state(rp_osc_worker_state_t *state);
int rp_osc_worker_update_params(rp_app_params_t *params, int fpga_update);

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
/* Fills the output measuremenet data with last measurements
 */
int rp_osc_set_meas_data(rp_osc_meas_res_t ch1_meas, rp_osc_meas_res_t ch2_meas);

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
                    float t_start, float t_stop, int time_unit,
                    rp_osc_meas_res_t *ch1_meas, rp_osc_meas_res_t *ch2_meas,
                    float ch1_max_adc_v, float ch2_max_adc_v,
                    float ch1_user_dc_off, float ch2_user_dc_off);

int rp_osc_decimate_partial(float **cha_out_signal, int *cha_in_signal, 
                            float **chb_out_signal, int *chb_in_signal,
                            float **time_out_signal, int *next_wr_ptr, 
                            int last_wr_ptr, int step_wr_ptr, int next_out_idx,
                            float t_start, int dec_factor, int time_unit,
                            rp_osc_meas_res_t *ch1_meas,
                            rp_osc_meas_res_t *ch2_meas,
                            float ch1_max_adc_v, float ch2_max_adc_v,
                            float ch1_user_dc_off, float ch2_user_dc_off);

/* Auto-set algorithm */
int rp_osc_auto_set(rp_app_params_t *orig_params, 
                    float ch1_max_adc_v, float ch2_max_adc_v,
                    float ch1_user_dc_off, float ch2_user_dc_off,
                    int ch1_probe_att, int ch2_probe_att, int ch1_gain, int ch2_gain, int en_avg_at_dec);

/* helper function - returns the factor for time unit conversion */
int rp_osc_get_time_unit_factor(int time_unit);

/* helper function - clears the measurement structure */
int rp_osc_meas_clear(rp_osc_meas_res_t *ch_meas);
/* helper function - calculates min, max and accumulates average value */
int rp_osc_meas_min_max(rp_osc_meas_res_t *ch_meas, int sig_data);
/* helper function - calculates average and amplitude */
int rp_osc_meas_avg_amp(rp_osc_meas_res_t *ch_meas, int avg_len);
/* helper function - calculates period and frequency */
int rp_osc_meas_period(rp_osc_meas_res_t *ch1_meas, rp_osc_meas_res_t *ch2_meas, 
                       int *in_cha_signal, int *in_chb_signal, int dec_factor);
int meas_period(rp_osc_meas_res_t *meas, int *in_signal, int wr_ptr_trig, int dec_factor,
                int *min, int *max);
/* helper function - convert CNT to V for meas. data (min, max, amp, avg) */
int rp_osc_meas_convert(rp_osc_meas_res_t *ch_meas, float adc_max_v, int32_t cal_dc_offs);

#endif /* __WORKER_H*/
