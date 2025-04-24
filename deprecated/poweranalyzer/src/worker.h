/**
 * $Id: worker.h 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Power analyzer worker.
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

#define SQRT2 sqrt(2)
#define M_PI 3.14159265358979323846

typedef enum rp_pwr_worker_state_e {
    rp_pwr_idle_state = 0, /* do nothing */
    rp_pwr_quit_state, /* shutdown worker */
    rp_pwr_abort_state, /* abort current measurement */
    rp_pwr_auto_state, /* auto mode acquisition */
    rp_pwr_normal_state, /* normal mode */
    rp_pwr_single_state, /* single acq., automatically goes to idle */
    rp_pwr_auto_set_state, /* runs auto-set algorithm */
    rp_pwr_nonexisting_state /* must be last */
} rp_pwr_worker_state_t;

int rp_pwr_worker_init(rp_app_params_t *params, int params_len,
                       rp_calib_params_t *calib_params);
int rp_pwr_worker_clean(void);                      
int rp_pwr_worker_exit(void);
int rp_pwr_worker_change_state(rp_pwr_worker_state_t new_state);
int rp_pwr_worker_get_state(rp_pwr_worker_state_t *state);
int rp_pwr_worker_update_params(rp_app_params_t *params, int fpga_update);

/* removes 'dirty' flags */
int rp_pwr_clean_signals(void);
/* Returns:
 *  0 - new signals (dirty signal) are copied to the output 
 *  1 - no new signals available (dirty signal was not set - we need to wait)
 */
int rp_pwr_get_signals(float ***signals, int *sig_idx);
/* Fills the output signal structure from temp one after calculation is done 
 * and marks it dirty 
 */
int rp_pwr_set_signals(float **source, int index);
int rp_pwr_copy_buffer(double *cha, double *chb, 
                       int calib_dc_off_a, int calib_dc_off_b);
/* Fills the output measuremenet data with last measurements
 */
int rp_pwr_set_ch_meas_data(rp_pwr_ch_meas_res_t u_meas, 
                            rp_pwr_ch_meas_res_t i_meas);
int rp_pwr_set_meas_data(rp_pwr_meas_res_t pwr_meas);
int rp_pwr_set_harmonics_data(rp_pwr_harm_t *harm_meas);
/* Prepares time vector (only where there is a need for it) */
int rp_pwr_prepare_time_vector(float **out_signal, int dec_factor,
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
int rp_pwr_decimate(float **cha_signal, int *in_cha_signal,
                    float **chb_signal, int *in_chb_signal,
                    float **time_signal, int dec_factor, 
                    float t_start, float t_stop, int time_unit,
                    rp_pwr_ch_meas_res_t *ch1_meas, 
                    rp_pwr_ch_meas_res_t *ch2_meas,
                    float ch1_max_adc_v, float ch2_max_adc_v,
                    float ch1_user_dc_off, float ch2_user_dc_off,
                    float diff_probe_att, float probe_fact);

int rp_pwr_decimate_partial(float **cha_out_signal, int *cha_in_signal, 
                            float **chb_out_signal, int *chb_in_signal,
                            float **time_out_signal, int *next_wr_ptr, 
                            int last_wr_ptr, int step_wr_ptr, int next_out_idx,
                            float t_start, int dec_factor, int time_unit,
                            rp_pwr_ch_meas_res_t *ch1_meas,
                            rp_pwr_ch_meas_res_t *ch2_meas,
                            float ch1_max_adc_v, float ch2_max_adc_v,
                            float ch1_user_dc_off, float ch2_user_dc_off,
                            float diff_probe_att, float probe_fact);

/* Auto-set algorithm */
int rp_pwr_auto_set(rp_app_params_t *orig_params, 
                    float ch1_max_adc_v, float ch2_max_adc_v,
                    float ch1_user_dc_off, float ch2_user_dc_off,
                    float ch1_probe_att, float ch2_probe_att, int ch1_gain, 
                    int ch2_gain, int en_avg_at_dec);

/* helper function - returns the factor for time unit conversion */
int rp_pwr_get_time_unit_factor(int time_unit);

float pwr_cnv_cnt_to_v(double cnts, float adc_max_v,
                       int calib_dc_off, float user_dc_off,
                       float diff_probe_att);
                       
float pwr_cnv_cnt_to_a(double cnts, float adc_max_v,
                       int calib_dc_off, float user_dc_off,
                       float k);
                    
/* helper function - clears the measurement structure */
int rp_pwr_ch_meas_clear(rp_pwr_ch_meas_res_t *ch_meas);
int rp_pwr_meas_clear(rp_pwr_meas_res_t *pwr_meas);
int rp_pwr_harmonics_clear(rp_pwr_harm_t *harm_meas);
/* helper function - calculates min, max and accumulates average value */
int rp_pwr_meas_min_max(rp_pwr_ch_meas_res_t *ch_meas, int sig_data);
/* helper function - calculates average and amplitude */
int rp_pwr_meas_avg_amp(rp_pwr_ch_meas_res_t *ch_meas, int avg_len);
/* helper function - calculates period and frequency */
int rp_pwr_meas_period(rp_pwr_ch_meas_res_t *ch1_meas, rp_pwr_ch_meas_res_t *ch2_meas, 
                       int *in_cha_signal, int *in_chb_signal, int dec_factor);
int meas_period(rp_pwr_ch_meas_res_t *meas, int *in_signal, int wr_ptr_trig, int dec_factor,
                int *min, int *max);
/* helper function - convert CNT to V for meas. data (min, max, amp, avg) */
int rp_pwr_meas_convert(rp_pwr_ch_meas_res_t *ch_meas, float adc_max_v, 
                        int32_t cal_dc_offs, float factor);
                        
int rp_pwr_is_fast_enough(int n);

#endif /* __WORKER_H*/
