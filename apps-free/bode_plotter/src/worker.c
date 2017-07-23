/**
 * $Id: worker.c 881 2013-12-16 05:37:34Z rp_jmenart $
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

#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>

#include "worker.h"
#include "fpga.h"

pthread_t *rp_osc_thread_handler = NULL;
void *rp_osc_worker_thread(void *args);


pthread_mutex_t       rp_osc_ctrl_mutex = PTHREAD_MUTEX_INITIALIZER;
rp_osc_worker_state_t rp_osc_ctrl;
rp_app_params_t       *rp_osc_params = NULL;
int                   rp_osc_params_dirty;
int                   rp_osc_params_fpga_update; //LukaG?

pthread_mutex_t       rp_osc_sig_mutex = PTHREAD_MUTEX_INITIALIZER;//Worker mutex
float               **rp_osc_signals;
int                   rp_osc_signals_dirty = 0;
int                   rp_osc_sig_last_idx = 0;
float               **rp_tmp_signals; /* used for calculation, only from worker */

/* Signals directly pointing at the FPGA mem space */
int                  *rp_fpga_cha_signal, *rp_fpga_chb_signal;

/* Calibration parameters read from EEPROM */
rp_calib_params_t *rp_calib_params = NULL;

int counter = 0;


/*----------------------------------------------------------------------------------*/
int rp_osc_worker_init(rp_app_params_t *params, int params_len,
                       rp_calib_params_t *calib_params)
{
    int ret_val;

    rp_osc_ctrl               = rp_osc_idle_state;
    rp_osc_params_dirty       = 0;
    rp_osc_params_fpga_update = 0;

    /* First copy of main params from main.c */
    rp_copy_params(params, (rp_app_params_t **)&rp_osc_params);

    /* First cleans up the params, case mem is already allocated */
    rp_cleanup_signals(&rp_osc_signals);
    /* Creates a double dimension vector with 3 values - s[i], where i is from 0 - 2 */
    if(rp_create_signals(&rp_osc_signals) < 0)
        return -1;
    /* Same for tmp_signals */
    rp_cleanup_signals(&rp_tmp_signals);
    if(rp_create_signals(&rp_tmp_signals) < 0) {
        rp_cleanup_signals(&rp_osc_signals);
        return -1;
    }

    /* cleans up FPGA memory buffer, if -1, we stop. */
    if(osc_fpga_init() < 0) {
        rp_cleanup_signals(&rp_osc_signals);
        rp_cleanup_signals(&rp_tmp_signals);
        return -1;
    }

    /* Calibration parameters */
    rp_calib_params = calib_params;

    /* Signal pointer directly to memory for channel A and B */
    osc_fpga_get_sig_ptr(&rp_fpga_cha_signal, &rp_fpga_chb_signal);

    /* Thread creation */
    rp_osc_thread_handler = (pthread_t *)malloc(sizeof(pthread_t));

    if(rp_osc_thread_handler == NULL) {
        rp_cleanup_signals(&rp_osc_signals);
        rp_cleanup_signals(&rp_tmp_signals);
        return -1;
    }

    ret_val = pthread_create(rp_osc_thread_handler, NULL, rp_osc_worker_thread, NULL);
    
    /* If thread creation failed */

    if(ret_val != 0) {
        osc_fpga_exit();

        rp_cleanup_signals(&rp_osc_signals);
        rp_cleanup_signals(&rp_tmp_signals);
        
        fprintf(stderr, "pthread_create() failed: %s\n", 
                strerror(errno));
        return -1;
    }

    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_osc_worker_exit(void)
{
    int ret_val = 0; 

    /* Change worker state to quite state */
    rp_osc_worker_change_state(rp_osc_quit_state);

    if(rp_osc_thread_handler) {
        ret_val = pthread_join(*rp_osc_thread_handler, NULL);
        /* Free thread memory space */
        free(rp_osc_thread_handler);

        rp_osc_thread_handler = NULL;
    }

    if(ret_val != 0) {
        fprintf(stderr, "pthread_join() failed: %s\n", 
                strerror(errno));
    }
    osc_fpga_exit();

    rp_cleanup_signals(&rp_osc_signals);
    rp_cleanup_signals(&rp_tmp_signals);

    rp_clean_params(rp_osc_params);

    return 0;
}


/*----------------------------------------------------------------------------------*/

/* This function just changes the worker state (To idle, for example)*/
int rp_osc_worker_change_state(rp_osc_worker_state_t new_state)
{
    if(new_state >= rp_osc_nonexisting_state)
        return -1;
    pthread_mutex_lock(&rp_osc_ctrl_mutex);
    rp_osc_ctrl = new_state;
    pthread_mutex_unlock(&rp_osc_ctrl_mutex);
    return 0;
}


/*----------------------------------------------------------------------------------*/

/* This functions gets the current worker state */
int rp_osc_worker_get_state(rp_osc_worker_state_t *state)
{
    pthread_mutex_lock(&rp_osc_ctrl_mutex);
    *state = rp_osc_ctrl;
    pthread_mutex_unlock(&rp_osc_ctrl_mutex);
    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_osc_worker_update_params(rp_app_params_t *params, int fpga_update)
{
    pthread_mutex_lock(&rp_osc_ctrl_mutex);
    rp_copy_params(params, (rp_app_params_t **)&rp_osc_params);
    rp_osc_params_dirty       = 1;

    /* LG - FPGA TODO */
    rp_osc_params_fpga_update = fpga_update;

    /* Name of idx at PARAMS_NUM is already NULL, why redefine it ? Error check -> Is it possible, to change the last param in worker? */
    rp_osc_params[PARAMS_NUM].name = NULL;
    rp_osc_params[PARAMS_NUM].value = -1;

    pthread_mutex_unlock(&rp_osc_ctrl_mutex);
    return 0;
}


/*----------------------------------------------------------------------------------*/

/* No new signals are needed */
int rp_osc_clean_signals(void)
{
    pthread_mutex_lock(&rp_osc_sig_mutex);
    rp_osc_signals_dirty = 0;
    pthread_mutex_unlock(&rp_osc_sig_mutex);
    return 0;
}


/*----------------------------------------------------------------------------------*/

/* Copies the mem location of rp_sc_signals to arg[0] -- signals */

int rp_osc_get_signals(float ***signals, int *sig_idx)
{
    float **s = *signals;
    pthread_mutex_lock(&rp_osc_sig_mutex);

    /* If new signals are avaliable */
    if(rp_osc_signals_dirty == 0) {
        *sig_idx = rp_osc_sig_last_idx;
        pthread_mutex_unlock(&rp_osc_sig_mutex);
        return -1;
    }

    memcpy(&s[0][0], &rp_osc_signals[0][0], sizeof(float)*((int)rp_get_params_bode(5)));
    memcpy(&s[1][0], &rp_osc_signals[1][0], sizeof(float)*((int)rp_get_params_bode(5)));
    memcpy(&s[2][0], &rp_osc_signals[2][0], sizeof(float)*((int)rp_get_params_bode(5)));

    /* Index of newly copied params */
    *sig_idx = rp_osc_sig_last_idx;

    rp_osc_signals_dirty = 0;
    pthread_mutex_unlock(&rp_osc_sig_mutex);
    return 0;
}


/*----------------------------------------------------------------------------------*/

/* Copies source to rp_osc_signals */
int rp_osc_set_signals(float **source, int index)
{
    
    pthread_mutex_lock(&rp_osc_sig_mutex);
    memcpy(&rp_osc_signals[0][0], &source[0][0], sizeof(float)*((int)rp_get_params_bode(5)));
    memcpy(&rp_osc_signals[1][0], &source[1][0], sizeof(float)*((int)rp_get_params_bode(5)));
    memcpy(&rp_osc_signals[2][0], &source[2][0], sizeof(float)*((int)rp_get_params_bode(5)));
    rp_osc_sig_last_idx = index;

    rp_osc_signals_dirty = 1;
    pthread_mutex_unlock(&rp_osc_sig_mutex);

    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_osc_set_meas_data(rp_osc_meas_res_t ch1_meas, rp_osc_meas_res_t ch2_meas)
{
    rp_update_meas_data(ch1_meas, ch2_meas);
    return 0;
}


/*----------------------------------------------------------------------------------*/

/* Main worker thread */
void *rp_osc_worker_thread(void *args)
{
    rp_osc_worker_state_t old_state, state;
    rp_app_params_t      *curr_params = NULL;
    int                   fpga_update = 0;
    int                   dec_factor = 0;
    int                   time_vect_update = 0;
    uint32_t              trig_source = 0;
    int                   params_dirty = 0;

    /* Long acquisition special function */
    int long_acq = 0; /* long_acq if acq_time > 1 [s] */
    int long_acq_idx = 0;
    int long_acq_first_wr_ptr = 0;
    int long_acq_last_wr_ptr = 0;
    int long_acq_step = 0;
    int long_acq_init_trig_ptr;

    rp_osc_meas_res_t ch1_meas, ch2_meas;
    float ch1_max_adc_v = 1, ch2_max_adc_v = 1;
    float max_adc_norm = osc_fpga_calc_adc_max_v(rp_calib_params->fe_ch1_fs_g_hi, 0);

    pthread_mutex_lock(&rp_osc_ctrl_mutex);
    old_state = state = rp_osc_ctrl;
    pthread_mutex_unlock(&rp_osc_ctrl_mutex);


    while(1) {
        /* update states - we also save old state to see if we need to reset
         * FPGA 
         */

        int start_measure = rp_get_params_bode(0);
        if(start_measure == 1){

            char command[100];
            float read_amp = rp_get_params_bode(1);
            float read_avg = rp_get_params_bode(2);
            float read_dc_bias = rp_get_params_bode(3);
            float read_start_freq = rp_get_params_bode(4);
            float read_counts = rp_get_params_bode(5);
            float read_scale = rp_get_params_bode(6);
            float read_end_freq = rp_get_params_bode(9);

            char amp[5];
            char avg[5];
            char dc_bias[5];
            char s_freq[20];
            char e_freq[20];
            char scale[5];
            char counts[5];

            snprintf(amp, 5, "%f", read_amp);
            snprintf(avg, 5, "%f", read_avg);
            snprintf(dc_bias, 5, "%f", read_dc_bias);
            snprintf(s_freq, 20, "%f", read_start_freq);
            snprintf(e_freq,20, "%f", read_end_freq);
            snprintf(scale, 5, "%f", read_scale);
            snprintf(counts, 5, "%f", read_counts);

            strcpy(command, "/opt/redpitaya/www/apps/bode_plotter/bode 1 ");
            
            strcat(command, amp);
            strcat(command, " ");

            strcat(command, dc_bias);
            strcat(command, " ");

            strcat(command, avg);
            strcat(command, " ");
            strcat(command, counts);
            strcat(command, " ");

            strcat(command, s_freq);
            strcat(command, " ");
            strcat(command, e_freq);
            strcat(command, " ");
            strcat(command, scale);
            
            system(command);

            rp_set_params_bode(0, 0);

        }

        old_state = state;
        pthread_mutex_lock(&rp_osc_ctrl_mutex);
        state = rp_osc_ctrl;

        /* If there are no new params */
        if(rp_osc_params_dirty) {
            rp_copy_params(rp_osc_params, (rp_app_params_t **)&curr_params);
            fpga_update = rp_osc_params_fpga_update;

            rp_osc_params_dirty = 0;

            dec_factor = 
                osc_fpga_cnv_time_range_to_dec(curr_params[TIME_RANGE_PARAM].value);
            time_vect_update = 1;


            uint32_t fe_fsg1 = (curr_params[GAIN_CH1].value == 0) ?
                    rp_calib_params->fe_ch1_fs_g_hi :
                    rp_calib_params->fe_ch1_fs_g_lo;
            ch1_max_adc_v =
                    osc_fpga_calc_adc_max_v(fe_fsg1, (int)curr_params[PRB_ATT_CH1].value);

            uint32_t fe_fsg2 = (curr_params[GAIN_CH2].value == 0) ?
                    rp_calib_params->fe_ch2_fs_g_hi :
                    rp_calib_params->fe_ch2_fs_g_lo;
            ch2_max_adc_v =
                    osc_fpga_calc_adc_max_v(fe_fsg2, (int)curr_params[PRB_ATT_CH2].value);
        }

        pthread_mutex_unlock(&rp_osc_ctrl_mutex);

        /* request to stop worker thread, we will shut down */
        if(state == rp_osc_quit_state) {
            rp_clean_params(curr_params);
            return 0;
        }

        if(state == rp_osc_auto_set_state) {
            /* Auto-set algorithm was selected - run it */
            rp_osc_auto_set(curr_params, ch1_max_adc_v, ch2_max_adc_v,
                            curr_params[GEN_DC_OFFS_1].value,
                            curr_params[GEN_DC_OFFS_2].value,
                            curr_params[PRB_ATT_CH1].value,
                            curr_params[PRB_ATT_CH2].value, 
                            curr_params[GAIN_CH1].value,
                            curr_params[GAIN_CH2].value, 			    
                            curr_params[EN_AVG_AT_DEC].value);
            /* Return calculated parameters to main module */
            rp_update_main_params(curr_params);
            /* while(1) - loop until break */
            continue;
        }
        if(fpga_update) {
            /* Reset write state machine? LG */
            osc_fpga_reset();
            if(osc_fpga_update_params((curr_params[TRIG_MODE_PARAM].value == 0),
                                      curr_params[TRIG_SRC_PARAM].value, 
                                      curr_params[TRIG_EDGE_PARAM].value,
                                      /* Here we could use trigger, but it is safer
                                       * to use start GUI value (it was recalculated
                                       * correctly already in rp_osc_main() so we
                                       * can use it and be sure that all signal 
                                       * (even if extended because of decimation
                                       * will be covered in the acquisition 
                                       */
                                      /*curr_params[TRIG_DLY_PARAM].value,*/
                                      curr_params[MIN_GUI_PARAM].value,
                                      curr_params[TRIG_LEVEL_PARAM].value,
                                      curr_params[TIME_RANGE_PARAM].value,
                                      max_adc_norm, max_adc_norm,
                                      rp_calib_params->fe_ch1_dc_offs,
                                      curr_params[GEN_DC_OFFS_1].value,
                                      rp_calib_params->fe_ch2_dc_offs,
                                      curr_params[GEN_DC_OFFS_2].value,
                                      curr_params[PRB_ATT_CH1].value,
                                      curr_params[PRB_ATT_CH2].value,
                                      curr_params[GAIN_CH1].value,
                                      curr_params[GAIN_CH2].value,				      
                                      curr_params[EN_AVG_AT_DEC].value) < 0) {
                fprintf(stderr, "Setting of FPGA registers failed\n");
                rp_osc_worker_change_state(rp_osc_idle_state);
            }
            trig_source = osc_fpga_cnv_trig_source(
                                     (curr_params[TRIG_MODE_PARAM].value == 0),
                                     curr_params[TRIG_SRC_PARAM].value,
                                     curr_params[TRIG_EDGE_PARAM].value);

            fpga_update = 0;
        }

        if(state == rp_osc_idle_state) {
            usleep(10000);
            continue;
        }
        /* Time vector update? */

        if(time_vect_update) {
            float unit_factor = 
                rp_osc_get_time_unit_factor(curr_params[TIME_UNIT_PARAM].value);
            float t_acq = (curr_params[MAX_GUI_PARAM].value - 
                           curr_params[MIN_GUI_PARAM].value) / unit_factor;


            rp_osc_prepare_time_vector((float **)&rp_tmp_signals[0], 
                                       dec_factor,
                                       curr_params[MIN_GUI_PARAM].value,
                                       curr_params[MAX_GUI_PARAM].value,
                                       curr_params[TIME_UNIT_PARAM].value);

            time_vect_update = 0;

            /* check if we have long acquisition - if yes the algorithm 
             * (wait for pre-defined time and return partial signal) */
            /* TODO: Make it programmable */
            if(t_acq >= 1.5) {
                long_acq = 1;
                /* Catch initial trigger - we will poll trigger pointer, 
                 * when it changes we will act like official 'trigger' 
                 * came
                 */
                rp_osc_meas_clear(&ch1_meas);
                rp_osc_meas_clear(&ch2_meas);
                osc_fpga_get_wr_ptr(NULL, &long_acq_init_trig_ptr);
            } else {
                long_acq_first_wr_ptr  = 0;
                long_acq_last_wr_ptr   = 0;
                long_acq = 0;
            }
            long_acq_idx = 0;
        }

        /* Start new acquisition only if it is the index 0 (new acquisition) */
        if(long_acq_idx == 0) {
            float time_delay = curr_params[TRIG_DLY_PARAM].value;
            /* Start the writting machine */
            osc_fpga_arm_trigger();
        
            /* Be sure to have enough time to load necessary history - wait */
            if(time_delay < 0) {
                /* time delay is always in seconds - convert to [us] and
                * sleep 
                */
                usleep(round(-1 * time_delay * 1e6));
            } else {
                if(curr_params[TIME_RANGE_PARAM].value > 4)
                    usleep(5000);
                else
                    usleep(1);
            }

            /* Start the trigger */
            osc_fpga_set_trigger(trig_source);
        }

        /* start working */
        pthread_mutex_lock(&rp_osc_ctrl_mutex);
        old_state = state = rp_osc_ctrl;
        pthread_mutex_unlock(&rp_osc_ctrl_mutex);
        if((state == rp_osc_idle_state) || (state == rp_osc_abort_state)) {
            continue;
        } else if(state == rp_osc_quit_state) {
            break;
        }

        if(long_acq_idx == 0) {
            /* polling until data is ready */
            while(1) {
                pthread_mutex_lock(&rp_osc_ctrl_mutex);
                state = rp_osc_ctrl;
                params_dirty = rp_osc_params_dirty;
                pthread_mutex_unlock(&rp_osc_ctrl_mutex);
                /* change in state, abort polling */
                if((state != old_state) || params_dirty) {
                    break;
                }
                
                if(!long_acq && osc_fpga_triggered()) {
                    /* for non-long acquisition wait for trigger */
                    break;
                } else if(long_acq) {
                    int trig_ptr, curr_ptr;
                    osc_fpga_get_wr_ptr(&curr_ptr, &trig_ptr);
                    if((long_acq_init_trig_ptr != trig_ptr) || osc_fpga_triggered()) {
                        /* FPGA wrote new trigger pointer - which means
                         * new trigger happened 
                         */
                        break;
                    }
                }
                usleep(1000);
            }
        }

        if((state != old_state) || params_dirty) {
            params_dirty = 0;
            continue;
        }
        if(long_acq) {
            /* Long acquisition - after trigger wait for a while to collect some
            * data
            */
            
            /* TODO: Make it programmable - Sleep for 200 [ms] */
            const int long_acq_part_delay = 1000 * 200; 
            if(long_acq_idx == 0) {
                /* Trigger - so let's arrange all the needed pointers & stuff */
                int wr_ptr_curr, wr_ptr_trig;
                float smpl_period = c_osc_fpga_smpl_period * dec_factor;
                int t_start_idx = 
                    round(curr_params[MIN_GUI_PARAM].value / smpl_period);
                float unit_factor = 
                    rp_osc_get_time_unit_factor(
                                         curr_params[TIME_UNIT_PARAM].value);
                float t_acq = (curr_params[MAX_GUI_PARAM].value - 
                               curr_params[MIN_GUI_PARAM].value) / 
                    unit_factor;
                
                osc_fpga_get_wr_ptr(&wr_ptr_curr, &wr_ptr_trig);
                long_acq_first_wr_ptr = wr_ptr_trig + t_start_idx - 3;
                if(long_acq_first_wr_ptr < 0)
                    long_acq_first_wr_ptr = 
                        OSC_FPGA_SIG_LEN + long_acq_first_wr_ptr;
                long_acq_last_wr_ptr  = long_acq_first_wr_ptr +
                    (t_acq / (c_osc_fpga_smpl_period * dec_factor));
                long_acq_last_wr_ptr = long_acq_last_wr_ptr % OSC_FPGA_SIG_LEN;

                if(round((t_acq / (c_osc_fpga_smpl_period * dec_factor) /
                         (((int)rp_get_params_bode(5))-1))) < 0)
                    long_acq_step = 1;
                else
                    long_acq_step = 
                        round((t_acq / (c_osc_fpga_smpl_period * dec_factor)) / 
                              (((int)rp_get_params_bode(5))-1));

                rp_osc_meas_clear(&ch1_meas);
                rp_osc_meas_clear(&ch2_meas);
            }
             
            /* we are after trigger - so let's wait a while to collect some 
            * samples */
            usleep(long_acq_part_delay); /* Sleep for 200 [ms] */
        }

        pthread_mutex_lock(&rp_osc_ctrl_mutex);
        state = rp_osc_ctrl;
        params_dirty = rp_osc_params_dirty;
        pthread_mutex_unlock(&rp_osc_ctrl_mutex);

        if((state != old_state) || params_dirty)
            continue;

        if(!long_acq) {
            /* Triggered, decimate & convert the values */
            rp_osc_meas_clear(&ch1_meas);
            rp_osc_meas_clear(&ch2_meas);
            bode_start_measure((float **)&rp_tmp_signals[1], &rp_fpga_cha_signal[0],
                            (float **)&rp_tmp_signals[2], &rp_fpga_chb_signal[0],
                            (float **)&rp_tmp_signals[0]);
        } else {
            long_acq_idx = rp_osc_decimate_partial((float **)&rp_tmp_signals[1], 
                                             &rp_fpga_cha_signal[0], 
                                             (float **)&rp_tmp_signals[2],
                                             &rp_fpga_chb_signal[0],
                                             (float **)&rp_tmp_signals[0],
                                             &long_acq_first_wr_ptr, 
                                             long_acq_last_wr_ptr,
                                             long_acq_step, long_acq_idx,
                                             curr_params[MIN_GUI_PARAM].value,
                                             dec_factor, 
                                             curr_params[TIME_UNIT_PARAM].value,
                                             &ch1_meas, &ch2_meas,
                                             ch1_max_adc_v, ch2_max_adc_v,
                                             curr_params[GEN_DC_OFFS_1].value,
                                             curr_params[GEN_DC_OFFS_2].value);

            /* Acquisition over, start one more! */
            if(long_acq_idx >= ((int)rp_get_params_bode(5))-1) {
                long_acq_idx = 0;

                osc_fpga_get_wr_ptr(NULL, &long_acq_init_trig_ptr);

                if(state == rp_osc_single_state) {
                    rp_osc_worker_change_state(rp_osc_idle_state);
                }
            }
            
        }

        /* check again for change of state */
        pthread_mutex_lock(&rp_osc_ctrl_mutex);
        state = rp_osc_ctrl;
        pthread_mutex_unlock(&rp_osc_ctrl_mutex);

        /* We have acquisition - if we are in single put state machine
         * to idle */
        if((state == rp_osc_single_state) && (!long_acq)) {
            rp_osc_worker_change_state(rp_osc_idle_state);
        }

       
       
        
        /* copy the results to the user buffer - if we are finished or not */
        if(!long_acq || long_acq_idx == 0) {
            /* Finish the measurement */
            rp_osc_meas_avg_amp(&ch1_meas, OSC_FPGA_SIG_LEN);
            rp_osc_meas_avg_amp(&ch2_meas, OSC_FPGA_SIG_LEN);
            
            rp_osc_meas_period(&ch1_meas, &ch2_meas, &rp_fpga_cha_signal[0], 
                               &rp_fpga_chb_signal[0], dec_factor);
            rp_osc_meas_convert(&ch1_meas, ch1_max_adc_v, rp_calib_params->fe_ch1_dc_offs);
            rp_osc_meas_convert(&ch2_meas, ch2_max_adc_v, rp_calib_params->fe_ch2_dc_offs);
            
            rp_osc_set_meas_data(ch1_meas, ch2_meas);
            rp_osc_set_signals(rp_tmp_signals, ((int)rp_get_params_bode(5))-1);
        } else {
            rp_osc_set_signals(rp_tmp_signals, long_acq_idx);
        }
        /* do not loop too fast */
        usleep(10000);
    }

    rp_clean_params(curr_params);
    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_osc_prepare_time_vector(float **out_signal, int dec_factor,
                               float t_start, float t_stop, int time_unit)
{
    float smpl_period = c_osc_fpga_smpl_period * dec_factor;
    float t_step, t_curr;
    int   out_idx, in_idx;
    int   idx_step;
    int   t_unit_factor = rp_osc_get_time_unit_factor(time_unit);;

    float *s = *out_signal;

    if(t_stop <= t_start) {
        t_start = 0;
        t_stop  = OSC_FPGA_SIG_LEN * smpl_period;
    }

    t_step = (t_stop - t_start) / (((int)rp_get_params_bode(5))-1);
    idx_step = (int)(ceil(t_step/smpl_period));
    if(idx_step > 8)
        idx_step = 8;

    for(out_idx = 0, in_idx = 0, t_curr=t_start; out_idx < ((int)rp_get_params_bode(5)); 
        out_idx++, t_curr += t_step, in_idx += idx_step) {
        s[out_idx] = t_curr * t_unit_factor;
    }
    
    return 0;
}


/*----------------------------------------------------------------------------------*/
int bode_start_measure(float **cha_signal, int *in_cha_signal,
                    float **chb_signal, int *in_chb_signal,
                    float **time_signal)
{
    int out_idx;

    float *cha_s = *cha_signal;
    float *chb_s = *chb_signal;
    float *t = *time_signal;
    float *frequency = *time_signal;

    
    //FILE *file_phase = fopen("/tmp/bode_data/data_phase", "r");

    

   
    
    /* We have data aquisition */
    if(rp_get_params_bode(0) != 0){

        for(out_idx = 0; out_idx < ((int)rp_get_params_bode(5)); out_idx++){
            t[out_idx] = out_idx;
            cha_s[out_idx] = 1;
            chb_s[out_idx] = -1;
        }

    }
     
    else{
        
        counter = 0;
        /* Opening files */
        FILE *file_frequency = fopen("/tmp/bode_data/data_frequency", "r");
        FILE *file_amplitude = fopen("/tmp/bode_data/data_amplitude", "r");
        FILE *file_phase = fopen("/tmp/bode_data/data_phase", "r");

        while(!feof(file_frequency)){
            fscanf(file_frequency, "%f", &frequency[counter]);
            counter++;
        }

        /* Allocation memory */
        float *amplitude = malloc(counter * sizeof(float));
        float *phase = malloc(counter * sizeof(float));

        int amp_counter = 0;
        while(!feof(file_amplitude)){
            fscanf(file_amplitude, "%f", &amplitude[amp_counter]);
            amp_counter++;
        }

        int p_counter = 0;
        while(!feof(file_phase)){
            fscanf(file_phase, "%f", &phase[p_counter]);
            p_counter++;
        }

        for(out_idx = 0; out_idx < counter; out_idx++){

            t[out_idx] = frequency[out_idx];
            
            if(rp_get_params_bode(8) == 0){
                cha_s[out_idx] = amplitude[out_idx];
            }else{
                cha_s[out_idx] = phase[out_idx];
            }
            
            chb_s[out_idx] = -1;
        }

        /* Closing files */
        fclose(file_frequency);
        fclose(file_amplitude);
        fclose(file_phase);
    }
    

    
    
    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_osc_decimate_partial(float **cha_out_signal, int *cha_in_signal, 
                            float **chb_out_signal, int *chb_in_signal,
                            float **time_out_signal, int *next_wr_ptr, 
                            int last_wr_ptr, int step_wr_ptr, int next_out_idx,
                            float t_start, int dec_factor, int time_unit,
                            rp_osc_meas_res_t *ch1_meas, 
                            rp_osc_meas_res_t *ch2_meas,
                            float ch1_max_adc_v, float ch2_max_adc_v,
                            float ch1_user_dc_off, float ch2_user_dc_off)
{
    float *cha_out = *cha_out_signal;
    float *chb_out = *chb_out_signal;
    float *t_out   = *time_out_signal;
    int    in_idx = *next_wr_ptr;

    float smpl_period = c_osc_fpga_smpl_period * dec_factor;
    int   t_unit_factor = rp_osc_get_time_unit_factor(time_unit);

    int curr_ptr;
    /* check if we have reached currently acquired signals in FPGA */
    osc_fpga_get_wr_ptr(&curr_ptr, NULL);

    for(; in_idx < curr_ptr; in_idx++) {
        if(in_idx >= OSC_FPGA_SIG_LEN)
            in_idx = in_idx % OSC_FPGA_SIG_LEN;
        rp_osc_meas_min_max(ch1_meas, cha_in_signal[in_idx]);
        rp_osc_meas_min_max(ch2_meas, chb_in_signal[in_idx]);
    }

    in_idx = *next_wr_ptr;

    for(; (next_out_idx < ((int)rp_get_params_bode(5))); next_out_idx++, 
            in_idx += step_wr_ptr) {
        int curr_ptr;
        int diff_ptr;
        /* check if we have reached currently acquired signals in FPGA */
        osc_fpga_get_wr_ptr(&curr_ptr, NULL);
        if(in_idx >= OSC_FPGA_SIG_LEN)
            in_idx = in_idx % OSC_FPGA_SIG_LEN;
        diff_ptr = (in_idx-curr_ptr);
        /* Check that we did not hit the curr ptr (and that pointer is not
         * wrapped 
         */
        if((in_idx >= curr_ptr) && (diff_ptr > 0) && (diff_ptr < 100))
            break;

        cha_out[next_out_idx] = 
            osc_fpga_cnv_cnt_to_v(cha_in_signal[in_idx], ch1_max_adc_v,
                                  rp_calib_params->fe_ch1_dc_offs,
                                  ch1_user_dc_off);

        chb_out[next_out_idx] = 
            osc_fpga_cnv_cnt_to_v(chb_in_signal[in_idx], ch2_max_adc_v,
                                  rp_calib_params->fe_ch1_dc_offs,
                                  ch2_user_dc_off);

        t_out[next_out_idx]   = 
            (t_start + ((next_out_idx*step_wr_ptr)*smpl_period))*t_unit_factor;

        /* A bug in FPGA? - Trig & write pointers not sample-accurate. */
        if ( (dec_factor > 64) && (next_out_idx == 2) ) {
             int i;
             for (i=0; i < next_out_idx; i++) {
                 cha_out[i] = cha_out[next_out_idx];
                 chb_out[i] = chb_out[next_out_idx];
             }
         }
    }

    *next_wr_ptr = in_idx;

    return next_out_idx;
}


/*----------------------------------------------------------------------------------*/
int rp_osc_get_time_unit_factor(int time_unit)
{
    int t_unit_factor;

    switch(time_unit) {
    case 0:
        /* [us] */
        t_unit_factor = 1e6;
        break;
    case 1:
        /* [ms] */
        t_unit_factor = 1e3;
        break;
    case 2:
    default:
        /* [s] */
        t_unit_factor = 1;
        break;
    }

    return t_unit_factor;
}


/*----------------------------------------------------------------------------------*/
int rp_osc_auto_set(rp_app_params_t *orig_params, 
                    float ch1_max_adc_v, float ch2_max_adc_v,
                    float ch1_user_dc_off, float ch2_user_dc_off,
                    int ch1_probe_att, int ch2_probe_att, int ch1_gain, int ch2_gain, int en_avg_at_dec)
{
    const int c_noise_thr = 500; /* noise threshold */
    rp_osc_worker_state_t old_state, state;
    int params_dirty;
    /* Min/maxes from both channel */
    int max_cha = INT_MIN;
    int max_chb = INT_MIN;
    int min_cha = INT_MAX;
    int min_chb = INT_MAX;
    /* Y axis deltas, 0 - ChA, 1 - Chb */
    int dy[2]     = { 0, 0 };
    int old_dy[2] = { 0, 0 };

    /* Channel to be used for auto-algorithm:
     * 0 - Channel A 
     * 1 - Channel B 
     */
    int channel; 
    int smpl_cnt;
    int iter;
    int time_range = 0;

    for (iter=0; iter < 10; iter++) {
        /* 10 auto-trigger acquisitions */
        pthread_mutex_lock(&rp_osc_ctrl_mutex);
        old_state = state = rp_osc_ctrl;
        pthread_mutex_unlock(&rp_osc_ctrl_mutex);

        osc_fpga_reset();
        osc_fpga_update_params(1, 0, 0, 0, 0, time_range, ch1_max_adc_v, ch2_max_adc_v,
                   rp_calib_params->fe_ch1_dc_offs,
                   0,
                   rp_calib_params->fe_ch2_dc_offs,
                   0,
                   ch1_probe_att, ch2_probe_att, ch1_gain, ch2_gain, 0);

        /* ARM & Trigger */
        osc_fpga_arm_trigger();
        osc_fpga_set_trigger(1);

        /* Wait for trigger to finish */
        while(1) {
            pthread_mutex_lock(&rp_osc_ctrl_mutex);
            state = rp_osc_ctrl;
            params_dirty = rp_osc_params_dirty;
            pthread_mutex_unlock(&rp_osc_ctrl_mutex);
            /* change in state, abort polling */
            if((state != old_state) || params_dirty) {
                return -1;
            }
            if(osc_fpga_triggered()) {
                break;
            }
            usleep(500);
        }

        /* Get the signals - available at rp_fpga_chX_signal vectors */
        for(smpl_cnt = 0; smpl_cnt < OSC_FPGA_SIG_LEN; smpl_cnt++) {
            int cha_smpl = rp_fpga_cha_signal[smpl_cnt];
            int chb_smpl = rp_fpga_chb_signal[smpl_cnt];

            // TWO'S COMPLEMENT
            if(cha_smpl & (1<<(c_osc_fpga_adc_bits-1)))
                cha_smpl = -1 * ((cha_smpl ^ ((1<<c_osc_fpga_adc_bits)-1))+1);
            if(chb_smpl & (1<<(c_osc_fpga_adc_bits-1)))
                chb_smpl = -1 * ((chb_smpl ^ ((1<<c_osc_fpga_adc_bits)-1))+1);

            cha_smpl = cha_smpl + rp_calib_params->fe_ch1_dc_offs;
            chb_smpl = chb_smpl + rp_calib_params->fe_ch2_dc_offs;

            /* Get min/maxes */
            max_cha = (max_cha < cha_smpl) ? cha_smpl : max_cha;
            min_cha = (min_cha > cha_smpl) ? cha_smpl : min_cha;
            max_chb = (max_chb < chb_smpl) ? chb_smpl : max_chb;
            min_chb = (min_chb > chb_smpl) ? chb_smpl : min_chb;
        }

        dy[0] = max_cha - min_cha;
        dy[1] = max_chb - min_chb;

        /* Calculate ratios of last amplitude increase for both channels */
        const float c_inc_thr = 1.1;
        float ratio[2] = { 1e6, 1e6 };
        int i;
        for(i = 0; i < 2; i++) {
            if (old_dy[i] != 0) {
                ratio[i] = (float)dy[i] / (float)old_dy[i];
            }
        }
        old_dy[0] = dy[0];
        old_dy[1] = dy[1];

        /* Signal amplitude found & stable? */
        if ( ((ratio[0] < c_inc_thr) && (dy[0] > c_noise_thr)) ||
             ((ratio[1] < c_inc_thr) && (dy[1] > c_noise_thr)) ) {
            /* This is meant for optimizing the time it takes for AUTO to complete,
             * and is a compromise between the speed and robustness of the AUTO
             * algorithm.
             * Since it is fast enough, it is safer to search for amplitude through
             * all the iterations instead of bailing out sooner.
             */
            //break;
        }

        /* Still searching? - Increase time range up to 130 ms (D = 1024). */
        if ((iter % 3) == 2) {
            time_range++;
            if (time_range > 3) {
                time_range = 3;
            }
        }
    }

    /* Check the Y axis amplitude on both channels and select the channel with 
     * the larger one.
     */
    channel = (dy[0] > dy[1]) ? 0 : 1;

    if(dy[channel] < c_noise_thr) {
        /* No signal detected, set the parameters to:
         * - no decimation (time range 130 [us])
         * - trigger mode - auto
         * - X axis - full, from 0 to 130 [us]
         * - Y axis - Min/Max + adding extra 200% to average
         */
        TRACE("AUTO: No signal detected.\n");
        int min_y, max_y, ave_y;

        orig_params[TRIG_MODE_PARAM].value  = 0;
        orig_params[MIN_GUI_PARAM].value    = 0;      
        orig_params[MAX_GUI_PARAM].value    = 130;
        orig_params[TIME_RANGE_PARAM].value = 0;
        orig_params[TRIG_SRC_PARAM].value   = 0;
        orig_params[TRIG_LEVEL_PARAM].value = 0;
        orig_params[AUTO_FLAG_PARAM].value  = 0;
        orig_params[TIME_UNIT_PARAM].value  = 0;
        orig_params[TRIG_DLY_PARAM].value   = 0;

        min_y = (min_cha < min_chb) ? min_cha : min_chb;
        max_y = (max_cha > max_chb) ? max_cha : max_chb;

        ave_y = (min_y + max_y) >> 1;
        min_y = (min_y - ave_y) * 2 + ave_y;
        max_y = (max_y - ave_y) * 2 + ave_y;

        orig_params[MIN_Y_NORM].value = min_y / (float)(1 << (c_osc_fpga_adc_bits - 1));
        orig_params[MAX_Y_NORM].value = max_y / (float)(1 << (c_osc_fpga_adc_bits - 1));

        // For POST response ...
        transform_to_iface_units(orig_params);
        return 0;

    } else {

        /* Signal above threshold - loop from lower to higher decimation */
        int min_y, max_y;
        int trig_src_ch;               /* Trigger level in samples, smpls_2 introduced for histeresis */
        int trig_level;
        float trig_level_v;            /* Trigger level in [V] */
        int time_range;                /* decimation from 0 to 5 */
        float max_adc_v;
        int calib_dc_off;

       int wr_ptr_curr, wr_ptr_trig;

        if(channel == 0) {
            min_y = min_cha;
            max_y = max_cha;
            trig_src_ch = 0;
            max_adc_v = ch1_max_adc_v;
            calib_dc_off = rp_calib_params->fe_ch1_dc_offs;
        } else {
            min_y = min_chb;
            max_y = max_chb;
            trig_src_ch = 1;
            max_adc_v = ch2_max_adc_v;
            calib_dc_off = rp_calib_params->fe_ch2_dc_offs;            
        }
        trig_level = (max_y + min_y) >> 1;
        TRACE("AUTO: trig level: %d\n", trig_level);
        TRACE("AUTO: min,max: %d  %d\n", min_y, max_y);

        trig_level_v = (((trig_level + calib_dc_off) * max_adc_v)/
                        (float)(1<<(c_osc_fpga_adc_bits-1))) ;

        int loc_max = INT_MIN;
        int loc_min = INT_MAX;

        /* Loop over time ranges and search for the best suiting one (Last range removed: too slow) */
        for(time_range = 0; time_range < 5; time_range++) {
            float period = 0;
            int trig_source = osc_fpga_cnv_trig_source(0, trig_src_ch, 0);
            int *sig_data;

            osc_fpga_reset();
            osc_fpga_update_params(0, trig_src_ch, 0, 0, trig_level_v, time_range,
                                   ch1_max_adc_v, ch2_max_adc_v,
                                   rp_calib_params->fe_ch1_dc_offs,
                                   0,
                                   rp_calib_params->fe_ch2_dc_offs,
                                   0, 
                                   ch1_probe_att, ch2_probe_att, ch1_gain, ch2_gain, en_avg_at_dec);

            /* ARM & Trigger */
            osc_fpga_arm_trigger();
            osc_fpga_set_trigger(trig_source);

            /* Wait for trigger */
            /* Wait for trigger to finish */
            while(1) {
                pthread_mutex_lock(&rp_osc_ctrl_mutex);
                state = rp_osc_ctrl;
                params_dirty = rp_osc_params_dirty;
                pthread_mutex_unlock(&rp_osc_ctrl_mutex);

                /* change in state, abort polling */
                if((state != old_state) || params_dirty) {
                    return -1;
                }
                if(osc_fpga_triggered()) {
                    break;
                }    
                usleep(500);
            }

            // Checking where acquisition starts
            osc_fpga_get_wr_ptr(&wr_ptr_curr, &wr_ptr_trig);

            /* We have a signal in rp_fpga_chX_signal */
            sig_data = (channel == 0) ? rp_fpga_cha_signal : rp_fpga_chb_signal;
            int dec_factor = osc_fpga_cnv_time_range_to_dec(time_range);

            rp_osc_meas_res_t meas;
            rp_osc_meas_clear(&meas);
            meas.min = min_y;
            meas.max = max_y;

            meas_period(&meas, sig_data, wr_ptr_trig, dec_factor, &loc_min, &loc_max);
            period = meas.period;
            TRACE("AUTO: period = %.6f\n", period);

            /* We have a winner - calculate the new parameters */
            if ((period > 0) || (time_range >= 4))
            {
                int ave_y, amp_y;
                int time_unit = 2;
                float t_unit_factor = 1; /* to convert to seconds */

                /* pick correct which time unit is selected */
                if((time_range == 0) || (time_range == 1)) {
                    time_unit     = 0;
                    t_unit_factor = 1e6;
                } else if((time_range == 2) || (time_range == 3)) {
                    time_unit     = 1;
                    t_unit_factor = 1e3;
                }
                
                orig_params[TRIG_MODE_PARAM].value  = 1; /* 'normal' */
                orig_params[TIME_RANGE_PARAM].value = time_range;
                orig_params[TRIG_SRC_PARAM].value   = trig_src_ch;
                orig_params[TRIG_LEVEL_PARAM].value = ((float)(loc_max + loc_min))/2 /
                                        (float)(1<<(c_osc_fpga_adc_bits-1));

                orig_params[MIN_GUI_PARAM].value    = 0;
                orig_params[TRIG_DLY_PARAM].value   = 0;

                if (period > 0) {
                    /* Period detected */
                    const float c_min_t_span = 1e-7;
                    if (period < c_min_t_span / 1.5) {
                        period = c_min_t_span / 1.5;
                    }
                    orig_params[MAX_GUI_PARAM].value =  period * 1.5 * t_unit_factor;
                } else {
                    /* Period not detected, which means it is longer than ~300 ms */
                    TRACE("AUTO: Signal period cannot be determined.\n");
                    /* Stretch to max 1/4 range. All slow signals should be still visible there */
                    orig_params[MAX_GUI_PARAM].value = 2.0;
                    orig_params[TIME_RANGE_PARAM].value = 5;
                }

                orig_params[TIME_UNIT_PARAM].value  = time_unit;
                orig_params[AUTO_FLAG_PARAM].value  = 0;

                min_y = (min_cha < min_chb) ? min_cha : min_chb;
                max_y = (max_cha > max_chb) ? max_cha : max_chb;

                if (loc_max > max_y)
                    max_y = loc_max;

                if (loc_min <  min_y)
                    min_y = loc_min;

                ave_y = (min_y + max_y) >> 1;
                amp_y = ((max_y - min_y) >> 1) * 1.2;
                min_y = ave_y - amp_y;
                max_y = ave_y + amp_y;

                orig_params[MIN_Y_NORM].value = min_y / (float)(1 << (c_osc_fpga_adc_bits - 1));
                orig_params[MAX_Y_NORM].value = max_y / (float)(1 << (c_osc_fpga_adc_bits - 1));

                // For POST response ...
                transform_to_iface_units(orig_params);
                return 0;
            }
        }
    }
    return -1;
}


/*----------------------------------------------------------------------------------*/
int rp_osc_meas_clear(rp_osc_meas_res_t *ch_meas)
{
    ch_meas->min = 1e9;
    ch_meas->max = -1e9;
    ch_meas->amp = 0;
    ch_meas->avg = 0;
    ch_meas->freq = 0;
    ch_meas->period = 0;

    return 0;
}


/*----------------------------------------------------------------------------------*/
inline int rp_osc_adc_sign(int in_data)
{
    int s_data = in_data;
    if(s_data & (1<<(c_osc_fpga_adc_bits-1)))
        s_data = -1 * ((s_data ^ ((1<<c_osc_fpga_adc_bits)-1)) + 1);
    return s_data;
}


/*----------------------------------------------------------------------------------*/
int rp_osc_meas_min_max(rp_osc_meas_res_t *ch_meas, int sig_data)
{
    int s_data = rp_osc_adc_sign(sig_data);

    if(ch_meas->min > s_data)
        ch_meas->min = s_data;
    if(ch_meas->max < s_data)
        ch_meas->max = s_data;

    ch_meas->avg += s_data;

    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_osc_meas_avg_amp(rp_osc_meas_res_t *ch_meas, int avg_len)
{
    ch_meas->avg /= avg_len;
    ch_meas->amp = ch_meas->max - ch_meas->min;
    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_osc_meas_period(rp_osc_meas_res_t *ch1_meas, rp_osc_meas_res_t *ch2_meas, 
                       int *in_cha_signal, int *in_chb_signal, int dec_factor)
{
    int wr_ptr_curr, wr_ptr_trig;

    // Checking where acquisition starts
    osc_fpga_get_wr_ptr(&wr_ptr_curr, &wr_ptr_trig);

    int min, max; // Ignored for measurement panel calculations
    meas_period(ch1_meas, in_cha_signal, wr_ptr_trig, dec_factor, &min, &max);
    meas_period(ch2_meas, in_chb_signal, wr_ptr_trig, dec_factor, &min, &max);

    return 0;
}

/*----------------------------------------------------------------------------------*/
int meas_period(rp_osc_meas_res_t *meas, int *in_signal, int wr_ptr_trig, int dec_factor,
                int *min, int *max)
{
    const float c_meas_freq_thr = 100;
    const int c_meas_time_thr = OSC_FPGA_SIG_LEN / 8;
    const float c_min_period = 19.6e-9; // 51 MHz

    float thr1, thr2, cen;
    int state = 0;
    int trig_t[2] = { 0, 0 };
    int trig_cnt = 0;
    int ix, ix_corr;

    float acq_dur=(float)(OSC_FPGA_SIG_LEN)/((float) c_osc_fpga_smpl_freq) * (float) dec_factor;

    cen = (meas->max + meas->min) / 2;

    thr1 = cen + 0.2 * (meas->min - cen);
    thr2 = cen + 0.2 * (meas->max - cen);

    meas->period = 0;
    *max = INT_MIN;
    *min = INT_MAX;

    for(ix = 0; ix < (OSC_FPGA_SIG_LEN); ix++) {
        ix_corr = ix + wr_ptr_trig;

        if (ix_corr >= OSC_FPGA_SIG_LEN) {
            ix_corr %= OSC_FPGA_SIG_LEN;
        }

        int sa = rp_osc_adc_sign(in_signal[ix_corr]);

        /* Another max, min calculation at lower rate to avoid evaluation errors on slower signals */
        if (sa > *max)
            *max = sa;
        if (sa < *min)
            *min = sa;

        /* Lower transitions */
        if((state == 0) && (ix_corr > 0) && (sa < thr1)) {
            state = 1;
        }

        /* Upper transitions - count them & store edge times. */
        if((state == 1) && (sa >= thr2) ) {
            state = 0;
            if (trig_cnt++ == 0) {
                trig_t[0] = ix;
            } else {
                trig_t[1] = ix;
            }
        }

        if ((trig_t[1] - trig_t[0]) > c_meas_time_thr) {
            break;
        }
    }

    /* Period calculation - taking into account at least meas_time_thr samples */
    if(trig_cnt >= 2) {
        meas->period = (trig_t[1] - trig_t[0]) /
            ((float)c_osc_fpga_smpl_freq * (trig_cnt - 1)) * dec_factor;
    }

    if( ((thr2 - thr1) < c_meas_freq_thr) ||
         (meas->period * 3 >= acq_dur)    ||
         (meas->period < c_min_period) )
    {
        meas->period = 0;
        meas->freq   = 0;
    } else {
        meas->freq = 1.0 / meas->period;
    }

    return 0;
}


/*----------------------------------------------------------------------------------*/
inline float rp_osc_meas_cnv_cnt(float data, float adc_max_v)
{
    return (data * adc_max_v / (float)(1<<(c_osc_fpga_adc_bits-1)));
}


/*----------------------------------------------------------------------------------*/
int rp_osc_meas_convert(rp_osc_meas_res_t *ch_meas, float adc_max_v, int32_t cal_dc_offs)
{
    ch_meas->min = rp_osc_meas_cnv_cnt(ch_meas->min+cal_dc_offs, adc_max_v);
    ch_meas->max = rp_osc_meas_cnv_cnt(ch_meas->max+cal_dc_offs, adc_max_v);
    ch_meas->amp = rp_osc_meas_cnv_cnt(ch_meas->amp, adc_max_v);
    ch_meas->avg = rp_osc_meas_cnv_cnt(ch_meas->avg+cal_dc_offs, adc_max_v);

    return 0;
}
