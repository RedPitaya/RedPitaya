/** @file worker.c
 *
 * $Id: worker.c 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Oscilloscope Worker.
 * @author Jure Menart <juremenart@gmail.com>
 * @copyright Red Pitaya  http://www.redpitaya.com
 */

#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>

#include "worker.h"
#include "fpga.h"

pthread_t *rp_osc_thread_handler = NULL;
void *rp_osc_worker_thread(void *args);

pthread_mutex_t       rp_osc_ctrl_mutex = PTHREAD_MUTEX_INITIALIZER;
rp_osc_worker_state_t rp_osc_ctrl;
rp_app_params_t       *rp_osc_params = NULL;
int                   rp_osc_params_dirty;
int                   rp_osc_params_fpga_update;

pthread_mutex_t       rp_osc_sig_mutex = PTHREAD_MUTEX_INITIALIZER;
float               **rp_osc_signals;
int                   rp_osc_signals_dirty = 0;
int                   rp_osc_sig_last_idx = 0;
float               **rp_tmp_signals; /* used for calculation, only from worker */

/* Signals directly pointing at the FPGA mem space */
int                  *rp_fpga_cha_signal, *rp_fpga_chb_signal;

/* Calibration parameters read from EEPROM */
rp_calib_params_t *rp_calib_params = NULL;
int counter = 0;
int measure_counter = 0;
int steps_counter = 0;
int measure_method = 0;

/*----------------------------------------------------------------------------------*/
int rp_osc_worker_init(rp_app_params_t *params, int params_len,
                       rp_calib_params_t *calib_params)
{
    int ret_val;

    rp_osc_ctrl               = rp_osc_idle_state;
    rp_osc_params_dirty       = 0;
    rp_osc_params_fpga_update = 0;

    rp_copy_params(params, (rp_app_params_t **)&rp_osc_params);

    rp_cleanup_signals(&rp_osc_signals);
    if(rp_create_signals(&rp_osc_signals) < 0)
        return -1;

    rp_cleanup_signals(&rp_tmp_signals);
    if(rp_create_signals(&rp_tmp_signals) < 0) {
        rp_cleanup_signals(&rp_osc_signals);
        return -1;
    }

    if(osc_fpga_init() < 0) {
        rp_cleanup_signals(&rp_osc_signals);
        rp_cleanup_signals(&rp_tmp_signals);
        return -1;
    }

    rp_calib_params = calib_params;

    osc_fpga_get_sig_ptr(&rp_fpga_cha_signal, &rp_fpga_chb_signal);

    rp_osc_thread_handler = (pthread_t *)malloc(sizeof(pthread_t));
    if(rp_osc_thread_handler == NULL) {
        rp_cleanup_signals(&rp_osc_signals);
        rp_cleanup_signals(&rp_tmp_signals);
        return -1;
    }
    ret_val = 
        pthread_create(rp_osc_thread_handler, NULL, rp_osc_worker_thread, NULL);
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

    rp_osc_worker_change_state(rp_osc_quit_state);
    if(rp_osc_thread_handler) {
        ret_val = pthread_join(*rp_osc_thread_handler, NULL);
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
    rp_osc_params_fpga_update = fpga_update;
    rp_osc_params[PARAMS_NUM].name = NULL;
    rp_osc_params[PARAMS_NUM].value = -1;

    pthread_mutex_unlock(&rp_osc_ctrl_mutex);
    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_osc_clean_signals(void)
{
    pthread_mutex_lock(&rp_osc_sig_mutex);
    rp_osc_signals_dirty = 0;
    pthread_mutex_unlock(&rp_osc_sig_mutex);
    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_osc_get_signals(float ***signals, int *sig_idx)
{
    float **s = *signals;
    pthread_mutex_lock(&rp_osc_sig_mutex);
    if(rp_osc_signals_dirty == 0) {
        *sig_idx = rp_osc_sig_last_idx;
        pthread_mutex_unlock(&rp_osc_sig_mutex);
        return -1;
    }

    memcpy(&s[0][0], &rp_osc_signals[0][0], sizeof(float)*((int)rp_get_params_lcr(1)));
    memcpy(&s[1][0], &rp_osc_signals[1][0], sizeof(float)*((int)rp_get_params_lcr(1)));
    memcpy(&s[2][0], &rp_osc_signals[2][0], sizeof(float)*((int)rp_get_params_lcr(1)));

    *sig_idx = rp_osc_sig_last_idx;

    rp_osc_signals_dirty = 0;
    pthread_mutex_unlock(&rp_osc_sig_mutex);
    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_osc_set_signals(float **source, int index)
{
    pthread_mutex_lock(&rp_osc_sig_mutex);

    memcpy(&rp_osc_signals[0][0], &source[0][0], sizeof(float)*((int)rp_get_params_lcr(1)));
    memcpy(&rp_osc_signals[1][0], &source[1][0], sizeof(float)*((int)rp_get_params_lcr(1)));
    memcpy(&rp_osc_signals[2][0], &source[2][0], sizeof(float)*((int)rp_get_params_lcr(1)));
    rp_osc_sig_last_idx = index;

    rp_osc_signals_dirty = 1;
    pthread_mutex_unlock(&rp_osc_sig_mutex);

    return 0;
}


/*----------------------------------------------------------------------------------*/
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
    float ch1_max_adc_v = 1, ch2_max_adc_v = 1;
    rp_osc_meas_res_t ch1_meas, ch2_meas;

    pthread_mutex_lock(&rp_osc_ctrl_mutex);
    old_state = state = rp_osc_ctrl;
    pthread_mutex_unlock(&rp_osc_ctrl_mutex);

    while(1) {
        /* update states - we save also old state to see if we need to reset
         * FPGA 
         */
        
        old_state = state;
        pthread_mutex_lock(&rp_osc_ctrl_mutex);
        state = rp_osc_ctrl;
        if(rp_osc_params_dirty) {
            rp_copy_params(rp_osc_params, (rp_app_params_t **)&curr_params);
            fpga_update = rp_osc_params_fpga_update;

            rp_osc_params_dirty = 0;
            dec_factor = 
                osc_fpga_cnv_time_range_to_dec(curr_params[TIME_RANGE_PARAM].value);
            time_vect_update = 1;

            if(curr_params[GAIN_CH1].value == 0) {
                ch1_max_adc_v = 
                    osc_fpga_calc_adc_max_v(rp_calib_params->fe_ch1_fs_g_hi,
                                            (int)curr_params[PRB_ATT_CH1].value);
            } else {
                ch1_max_adc_v = 
                    osc_fpga_calc_adc_max_v(rp_calib_params->fe_ch1_fs_g_lo,
                                            (int)curr_params[PRB_ATT_CH1].value);
            }

            if(curr_params[GAIN_CH2].value == 0) {
                ch2_max_adc_v = 
                    osc_fpga_calc_adc_max_v(rp_calib_params->fe_ch2_fs_g_hi,
                                            (int)curr_params[PRB_ATT_CH2].value);
            } else {
                ch2_max_adc_v = 
                    osc_fpga_calc_adc_max_v(rp_calib_params->fe_ch2_fs_g_lo,
                                            (int)curr_params[PRB_ATT_CH2].value);
            }

        }
        pthread_mutex_unlock(&rp_osc_ctrl_mutex);


        float save_data = rp_get_params_lcr(16);

        /* Check if we want to save param data to a file */
        if(save_data == 1){

            char f_command[100];

            /* If the file doesn't exists yet */
            if(fopen("/opt/redpitaya/www/apps/impedance_analyzer/lcr_param_data", "w") == NULL){
                /* Create a normal file, as txt is a just an interpreter */
                strcpy(f_command, "touch /opt/redpitaya/www/apps/impedance_analyzer/lcr_param_data");
                system(f_command);
            }

            /* Open file for writing */
            FILE *data = fopen("/opt/redpitaya/www/apps/impedance_analyzer/lcr_param_data", "w");

            /* Write parameter data */
            fprintf(data, "%.1f\n", rp_get_params_lcr(2));
            fprintf(data, "%.1f\n", rp_get_params_lcr(3));
            fprintf(data, "%.1f\n", rp_get_params_lcr(4));
            fprintf(data, "%.1f\n", rp_get_params_lcr(5));
            fprintf(data, "%.1f\n", rp_get_params_lcr(6));
            fprintf(data, "%.1f\n", rp_get_params_lcr(7));
            fprintf(data, "%.1f\n", rp_get_params_lcr(8));
            fprintf(data, "%.1f\n", rp_get_params_lcr(9));
            fprintf(data, "%.1f\n", rp_get_params_lcr(10));
            fprintf(data, "%.1f\n", rp_get_params_lcr(11));
            fprintf(data, "%.1f\n", rp_get_params_lcr(15));

            /* Set lcr_save_data to 0, so we don't access this IF everytime */
            rp_set_params_lcr(2,0);
            fclose(data);

        }

        /* Load parameters if user input is true */
        if(rp_load_data(save_data) == -1){
            printf("Loading parameters failed!\n");
        }

        /* Start lcr measurment */
        float measure_option = rp_get_params_lcr(0);
        /* Set max command lenght */
        char command[100];
        /* Frequency sweep */
        if(measure_option == 1){


            float lcr_steps = rp_get_params_lcr(1);
            float lcr_amp = rp_get_params_lcr(2);
            float lcr_avg = rp_get_params_lcr(3);
            float lcr_dc_bias = rp_get_params_lcr(4);
            float lcr_r_shunt = rp_get_params_lcr(5);
            float start_freq = rp_get_params_lcr(6);
            float end_freq = rp_get_params_lcr(7);
            float lcr_Scale = rp_get_params_lcr(8);
            float lcr_load_re = rp_get_params_lcr(9);
            float lcr_load_im = rp_get_params_lcr(10);
            float lcr_calibration = rp_get_params_lcr(11);

            char steps[10];
            char sF[20];
            char eF[20];
            char amp[20];
            char avg[20];
            char dc_bias[20];
            char r_shunt[20];
            char scale[20];
            char re[10];
            char im[10];
            char calib[1];

            snprintf(steps, 10, "%f", lcr_steps);
            printf("STEPS: %s\n", steps);
            snprintf(sF, 20, "%f", start_freq);
            snprintf(eF, 20, "%f", end_freq);
            snprintf(amp, 20, "%f", lcr_amp);
            snprintf(avg, 20, "%f", lcr_avg);
            snprintf(dc_bias, 20, "%f", lcr_dc_bias);
            snprintf(r_shunt, 20, "%f", lcr_r_shunt);
            snprintf(scale, 20, "%f", lcr_Scale);
            snprintf(re, 10, "%f", lcr_load_re);
            snprintf(im, 10, "%f", lcr_load_im);
            snprintf(calib, 1, "%f", lcr_calibration);
            
            strcpy(command, "/opt/redpitaya/www/apps/impedance_analyzer/lcr 1 ");
            strcat(command, amp);
            strcat(command, " ");

            strcat(command, dc_bias);
            strcat(command, " ");

            strcat(command, r_shunt);
            strcat(command, " ");

            strcat(command, avg);

            strcat(command, " 0 ");

            strcat(command, re);
            strcat(command, " ");
            strcat(command, im);
            strcat(command, " ");
            strcat(command, steps);
            strcat(command, " 1 ");

            strcat(command, sF);
            strcat(command, " ");
            strcat(command, eF);
            strcat(command, " ");
            strcat(command, scale);
            strcat(command, " 0");

            system(command);
            measure_method = 2;
            rp_set_params_lcr(0, 0);


        /* Measurment sweep */
        }else if(measure_option == 2){

            float lcr_counts = rp_get_params_lcr(1);
            float lcr_amp = rp_get_params_lcr(2);
            float lcr_avg = rp_get_params_lcr(3);
            float lcr_dc_bias = rp_get_params_lcr(4);
            float lcr_r_shunt = rp_get_params_lcr(5);
            float start_freq = rp_get_params_lcr(6);
            float end_freq = rp_get_params_lcr(7);
            float lcr_Scale = rp_get_params_lcr(8);
            float lcr_load_re = rp_get_params_lcr(9);
            float lcr_load_im = rp_get_params_lcr(10);
            float lcr_calibration = rp_get_params_lcr(11);


            char ms_counts[20];
            char sF[20];
            char eF[20];
            char amp[20];
            char avg[20];
            char dc_bias[20];
            char r_shunt[20];
            char scale[20];
            char re[10];
            char im[10];
            char calib[1];


            snprintf(ms_counts, 20, "%f", lcr_counts);
            snprintf(sF, 20, "%f", start_freq);
            snprintf(eF, 20, "%f", end_freq);
            snprintf(amp, 20, "%f", lcr_amp);
            snprintf(avg, 20, "%f", lcr_avg);
            snprintf(dc_bias, 20, "%f", lcr_dc_bias);
            snprintf(r_shunt, 20, "%f", lcr_r_shunt);
            snprintf(scale, 20, "%f", lcr_Scale);
            snprintf(re, 10, "%f", lcr_load_re);
            snprintf(im, 10, "%f", lcr_load_im);
            snprintf(calib, 1, "%f", lcr_calibration);

            char command[100];
            
            strcpy(command, "/opt/redpitaya/www/apps/impedance_analyzer/lcr 1 "); 
            
            strcat(command, amp);
            strcat(command, " ");

            strcat(command, dc_bias);
            strcat(command, " ");

            strcat(command, r_shunt);
            strcat(command, " ");
            
            strcat(command, avg);
            strcat(command, " 0 ");

            strcat(command, re);
            strcat(command, " ");
            strcat(command, im);
            strcat(command, " ");

            strcat(command, ms_counts);
            strcat(command, " 0 ");

            strcat(command, sF);
            strcat(command, " ");
            strcat(command, " 1000 ");
            strcat(command, scale);
            strcat(command, " 0");
            
            system(command);
            measure_method = 1; // MS
            rp_set_params_lcr(0, 0);
            
        }
        
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
            continue;
        }
        if(fpga_update) {
            osc_fpga_reset();
            if(osc_fpga_update_params((curr_params[TRIG_MODE_PARAM].value == 0),
                                      curr_params[TRIG_SRC_PARAM].value, 
                                      curr_params[TRIG_EDGE_PARAM].value,
                                      /* Here we could use trigger, but it is safer
                                       * to use start GUI value (it was recalculated
                                       * correctly already in rp_osc_main() so we
                                       * can use it and be sure that all signal 
                                       * (even if extended becuase of decimation
                                       * will be covered in the acquisition 
                                       */
                                      /*curr_params[TRIG_DLY_PARAM].value,*/
                                      curr_params[MIN_GUI_PARAM].value,
                                      curr_params[TRIG_LEVEL_PARAM].value,
                                      curr_params[TIME_RANGE_PARAM].value,
                                      ch1_max_adc_v, ch2_max_adc_v,
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
                         (((int)rp_get_params_lcr(1))-1))) < 0)
                    long_acq_step = 1;
                else
                    long_acq_step = 
                        round((t_acq / (c_osc_fpga_smpl_period * dec_factor)) / 
                              (((int)rp_get_params_lcr(1))-1));

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

            lcr_start_Measure((float **)&rp_tmp_signals[1], &rp_fpga_cha_signal[0],
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
            if(long_acq_idx >= ((int)rp_get_params_lcr(1))-1) {
                long_acq_idx = 0;
                osc_fpga_get_wr_ptr(NULL, &long_acq_init_trig_ptr);

                
                rp_osc_worker_change_state(rp_osc_idle_state);
                
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
            
            rp_osc_set_signals(rp_tmp_signals, ((int)rp_get_params_lcr(1))-1);
        } else {
            rp_osc_set_signals(rp_tmp_signals, long_acq_idx);
        }
        /* do not loop too fast */
        usleep(1000);
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

    t_step = (t_stop - t_start) / (((int)rp_get_params_lcr(1))-1);
    idx_step = (int)(ceil(t_step/smpl_period));
    if(idx_step > 8)
        idx_step = 8;

    for(out_idx = 0, in_idx = 0, t_curr=t_start; out_idx < ((int)rp_get_params_lcr(1)); 
        out_idx++, t_curr += t_step, in_idx += idx_step) {
        s[out_idx] = t_curr * t_unit_factor;
    }
    
    return 0;
}


/*----------------------------------------------------------------------------------*/
int lcr_start_Measure(float **cha_signal, int *in_cha_signal,
                    float **chb_signal, int *in_chb_signal,
                    float **time_signal)
{
    /* Output signal index */
    int out_idx;

    /* Raw A and B channel signal */
    float *cha_s = *cha_signal;
    float *chb_s = *chb_signal;

    /* rp_tmp_signal[0] for X-coordinate set to frequency */
    float *t = *time_signal;
    float *frequency = *time_signal;

    /* If we are in first boot, start_measure will always be set to -1 
     * rp_get_params_lcr(0):
     * ret_val: -1 --> First boot 
     *           0 --> No measurment
     *           1 --> Frequency sweep
     *           2 --> Measurment sweep  */

    if(rp_get_params_lcr(0) == -1){


        for(out_idx=0; out_idx < counter; out_idx++) {
            cha_s[out_idx] = 0;
            chb_s[out_idx] = 0;
            t[out_idx] = out_idx;
        }
    }else{


        /* Opening files */
        FILE *file_frequency = fopen("/tmp/lcr_data/data_frequency", "r");
        FILE *file_phase = fopen("/tmp/lcr_data/data_phase", "r");
        FILE *file_amplitude = fopen("/tmp/lcr_data/data_amplitude", "r");
        FILE *file_Y_abs = fopen("/tmp/lcr_data/data_Y_abs", "r");
        FILE *file_PhaseY = fopen("/tmp/lcr_data/data_phaseY", "r");
        FILE *file_R_s = fopen("/tmp/lcr_data/data_R_s", "r");
        FILE *file_X_s = fopen("/tmp/lcr_data/data_X_s", "r");
        FILE *file_G_p = fopen("/tmp/lcr_data/data_G_p", "r");
        FILE *file_B_p = fopen("/tmp/lcr_data/data_B_p", "r");
        FILE *file_C_s = fopen("/tmp/lcr_data/data_C_s", "r");
        FILE *file_C_p = fopen("/tmp/lcr_data/data_C_p", "r");
        FILE *file_L_s = fopen("/tmp/lcr_data/data_L_s", "r");
        FILE *file_L_p = fopen("/tmp/lcr_data/data_L_p", "r");
        FILE *file_R_p = fopen("/tmp/lcr_data/data_R_p", "r");
        FILE *file_Q = fopen("/tmp/lcr_data/data_Q", "r");
        FILE *file_D = fopen("/tmp/lcr_data/data_D", "r");
        

        while(!feof(file_frequency)){
            fscanf(file_frequency, "%f", &frequency[counter]);
            counter++;
        }

        

        /* Allocationg memory. TODO: Free up memory after use */
        float *phase = malloc(counter * sizeof(float));
        float *amplitude = malloc(counter * sizeof(float));
        float *Y_abs = malloc(counter * sizeof(float));
        float *R_s = malloc(counter * sizeof(float));
        float *phaseY = malloc(counter * sizeof(float));
        float *X_s = malloc(counter * sizeof(float));
        float *G_p = malloc(counter * sizeof(float));
        float *B_p = malloc(counter * sizeof(float));
        float *C_s = malloc(counter * sizeof(float));
        float *C_p = malloc(counter * sizeof(float));
        float *L_s = malloc(counter * sizeof(float));
        float *L_p = malloc(counter * sizeof(float));
        float *R_p = malloc(counter * sizeof(float));
        float *Q = malloc(counter * sizeof(float));
        float *D = malloc(counter * sizeof(float));

        /* Reading data from files into mem-allocated variables */
        int B_p_counter = 0;
        while(!feof(file_B_p)){
            fscanf(file_B_p, "%f", &B_p[B_p_counter]);
            B_p_counter++;
        }

        int C_s_counter = 0;
        while(!feof(file_C_s)){
            fscanf(file_C_s, "%f", &C_s[C_s_counter]);
            C_s_counter++;
        }

        int C_p_counter = 0;
        while(!feof(file_C_p)){
            fscanf(file_C_p, "%f", &C_p[C_p_counter]);
            C_p_counter++;
        }

        int L_s_counter = 0;
        while(!feof(file_L_s)){
            fscanf(file_L_s, "%f", &L_s[L_s_counter]);
            L_s_counter++;
        }

        int L_p_counter = 0;
        while(!feof(file_L_p)){
            fscanf(file_L_p, "%f", &L_p[L_p_counter]);
            L_p_counter++;
        }

        int R_p_counter = 0;
        while(!feof(file_R_p)){
            fscanf(file_R_p, "%f", &R_p[R_p_counter]);
            R_p_counter++;
        }

        int q_counter = 0;
        while(!feof(file_Q)){
            fscanf(file_Q, "%f", &Q[q_counter]);
            q_counter++;
        }

        int d_counter = 0;
        while(!feof(file_D)){
            fscanf(file_D, "%f", &D[d_counter]);
            d_counter++;
        }
        
        int p_counter = 0;
        while(!feof(file_phase)){
            fscanf(file_phase, "%f", &phase[p_counter]);
            p_counter++;
        }

        int a_counter = 0;
        while(!feof(file_amplitude)){
            fscanf(file_amplitude, "%f", &amplitude[a_counter]);
            a_counter++;
        }
        
        int y_abs_counter = 0;
        while(!feof(file_Y_abs)){
            fscanf(file_Y_abs, "%f", &Y_abs[y_abs_counter]);
            y_abs_counter++;
        }

        
        int phaseY_counter = 0;
        while(!feof(file_PhaseY)){
            fscanf(file_PhaseY, "%f", &phaseY[phaseY_counter]);
            phaseY_counter++;
        }
        
        int R_s_counter = 0;
        while(!feof(file_R_s)){
            fscanf(file_R_s, "%f", &R_s[R_s_counter]);
            R_s_counter++;
        }
        
        int X_s_counter = 0;
        while(!feof(file_X_s)){
            fscanf(file_X_s, "%f", &X_s[X_s_counter]);
            X_s_counter++;
        }

        int G_p_counter = 0;
        while(!feof(file_G_p)){
            fscanf(file_G_p, "%f", &G_p[G_p_counter]);
            G_p_counter++;
        }
        
        /* Closing all the files */
        fclose(file_frequency);
        fclose(file_amplitude);
        fclose(file_phase);
        fclose(file_Y_abs);
        fclose(file_PhaseY);
        fclose(file_R_s);
        fclose(file_X_s);
        fclose(file_G_p);
        fclose(file_B_p);
        fclose(file_C_s);
        fclose(file_C_p);
        fclose(file_L_s);
        fclose(file_L_p);
        fclose(file_R_p);
        fclose(file_Q);
        fclose(file_D);
        
        for(out_idx=0; out_idx < counter; out_idx++) {

            /* Data check: TODO add switch statment for a niftier code */

            float scale = rp_get_params_lcr(15);
            if      (scale ==  0) cha_s[out_idx] = amplitude[out_idx];
            else if (scale ==  1) cha_s[out_idx] = phase[out_idx]; 
            else if (scale ==  2) cha_s[out_idx] = Y_abs[out_idx];
            else if (scale ==  3) cha_s[out_idx] = phaseY[out_idx];
            else if (scale ==  4) cha_s[out_idx] = R_s[out_idx];
            else if (scale ==  5) cha_s[out_idx] = R_p[out_idx];
            else if (scale ==  6) cha_s[out_idx] = X_s[out_idx];
            else if (scale ==  7) cha_s[out_idx] = G_p[out_idx];
            else if (scale ==  8) cha_s[out_idx] = B_p[out_idx];
            else if (scale ==  9) cha_s[out_idx] = C_s[out_idx];
            else if (scale == 10) cha_s[out_idx] = C_p[out_idx];
            else if (scale == 11) cha_s[out_idx] = L_s[out_idx];
            else if (scale == 12) cha_s[out_idx] = L_p[out_idx];
            else if (scale == 13) cha_s[out_idx] = Q[out_idx];
            else if (scale == 14) cha_s[out_idx] = D[out_idx];
   
            chb_s[out_idx] = 0;

            /* Measurment sweep */
            if(measure_method == 1){
                t[out_idx] = out_idx;
        
            /* Frequency sweep */
            }else if(measure_method == 2){
                t[out_idx] = frequency[out_idx];
            }
        }     
        
    }
    
    steps_counter = out_idx;
    counter = 0;

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
    }

    in_idx = *next_wr_ptr;

    for(; (next_out_idx < ((int)rp_get_params_lcr(1))); next_out_idx++, 
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
    int dy[2]; 
    /* Channel to be used for auto-algorithm:
     * 0 - Channel A 
     * 1 - Channel B 
     */
    int channel; 
    int i, smpl_cnt;
    int iter;

    for (iter=0; iter < 10; iter++) {
        /* 10 auto-trigger acquisitions */
        pthread_mutex_lock(&rp_osc_ctrl_mutex);
        old_state = state = rp_osc_ctrl;
        pthread_mutex_unlock(&rp_osc_ctrl_mutex);

        osc_fpga_reset();
        osc_fpga_update_params(1, 0, 0, 0, 0, 0, ch1_max_adc_v, ch2_max_adc_v,
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

            cha_smpl=cha_smpl+rp_calib_params->fe_ch1_dc_offs;
            chb_smpl=chb_smpl+rp_calib_params->fe_ch2_dc_offs;

            /* Get min/maxes */
            max_cha = (max_cha < cha_smpl) ? cha_smpl : max_cha;
            min_cha = (min_cha > cha_smpl) ? cha_smpl : min_cha;
            max_chb = (max_chb < chb_smpl) ? chb_smpl : max_chb;
            min_chb = (min_chb > chb_smpl) ? chb_smpl : min_chb;
        }
    }
    /* Check the Y axis amplitude on both channels and select the channel with 
     * the larger one.
     */
    dy[0] = max_cha - min_cha;
    dy[1] = max_chb - min_chb;

    if(dy[0] > dy[1])
        channel = 0;
    else
        channel = 1;

    if(dy[channel] < c_noise_thr) {
        /* No signal detected, set the parameters to:
         * - no decimation (time range 130 [us])
         * - trigger mode - auto
         * - X axis - full, from 0 to 130 [us]
         * - Y axis - Min/Max + adding extra 200% to average
         */
        int min_y, max_y, ave_y;
        int max_adc_v = ch1_max_adc_v;

        if(channel == 1)
            max_adc_v = ch2_max_adc_v;

        orig_params[TRIG_MODE_PARAM].value  = 0;
        orig_params[MIN_GUI_PARAM].value    = 0;      
        orig_params[MAX_GUI_PARAM].value    = 130;
        orig_params[TIME_RANGE_PARAM].value = 0;
        orig_params[TRIG_SRC_PARAM].value   = 0;
        orig_params[TRIG_LEVEL_PARAM].value = 0;
        orig_params[AUTO_FLAG_PARAM].value  = 0;
        orig_params[TIME_UNIT_PARAM].value  = 0;

        min_y = (min_cha < min_chb) ? min_cha : min_chb;
        max_y = (max_cha > max_chb) ? max_cha : max_chb;

        ave_y = (min_y + max_y)>>1;
        min_y = (min_y-ave_y)*2+ave_y;
        max_y = (max_y-ave_y)*2+ave_y;

        orig_params[MIN_Y_PARAM].value = (min_y * max_adc_v)/
            (float)(1<<(c_osc_fpga_adc_bits-1));
        orig_params[MAX_Y_PARAM].value = (max_y * max_adc_v)/
            (float)(1<<(c_osc_fpga_adc_bits-1));
        return 0;
    } else {
        int min_y, max_y;              /* Signal above threshold - loop from lower to higher decimation */
        int trig_src_ch;               /* Trigger level in samples, smpls_2 introduced for histeresis */
        int trig_level, trig_level_2;
        float trig_level_v;            /* Trigger level in [V] */
        int time_range;                /* decimation from 0 to 5 */
        float max_adc_v;
        int calib_dc_off;

       int wr_ptr_curr, wr_ptr_trig, ix_corr;

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
        trig_level = (max_y+min_y)>>1;
        trig_level_2 = (trig_level+max_y)>>1;

        trig_level_v = (((trig_level + calib_dc_off) * max_adc_v)/
                        (float)(1<<(c_osc_fpga_adc_bits-1))) ;

        int loc_max=INT_MIN;
        int loc_min=INT_MAX;

        /* Loop over time ranges and search for the best suiting one (Last range removed: too slow) */
        for(time_range = 0; time_range < 5; time_range++) {
            int trig_cnt = 0;
            float period = 0;
            int trig_time[3] = { -1, -1, -1 };
            int trig_source = osc_fpga_cnv_trig_source(0, trig_src_ch, 0);
            int *sig_data;
            int trig_state = 0;

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
            if(channel == 0)
                sig_data = rp_fpga_cha_signal;
            else
                sig_data = rp_fpga_chb_signal;

        
        
        
            /* Count trigger events */
            for(i = 1; i < OSC_FPGA_SIG_LEN; i++) {
                ix_corr=i+wr_ptr_trig;

                if (ix_corr>=(OSC_FPGA_SIG_LEN))
                    ix_corr=ix_corr-OSC_FPGA_SIG_LEN;
                int sig_smpl[2];
                sig_smpl[1] = sig_data[ix_corr];
                sig_smpl[0] = sig_data[ix_corr-1];

                if(sig_smpl[0] & (1<<(c_osc_fpga_adc_bits-1)))
                    sig_smpl[0] = 
                        -1*((sig_smpl[0] ^ ((1<<c_osc_fpga_adc_bits)-1))+1);
                if(sig_smpl[1] & (1<<(c_osc_fpga_adc_bits-1)))
                    sig_smpl[1] = 
                        -1*((sig_smpl[1] ^ ((1<<c_osc_fpga_adc_bits)-1))+1);

                
                // Another max, min calculation at lower rate to avoid evaluation errors on slower signals
                if (sig_smpl[0]>loc_max)
                    loc_max=sig_smpl[0];

                if (sig_smpl[0]<loc_min)
                    loc_min=sig_smpl[0];

                /* Check for trigger condition 1 (trig_level) */
                if((sig_smpl[0]<trig_level) && (sig_smpl[1]>=trig_level)) {
                    trig_state = 1;
                }

                /* Check for threshold condition 2 (trig_level_2) */
                if((trig_state == 1) && (sig_smpl[0]<trig_level_2) && 
                   (sig_smpl[1]>=trig_level_2)) {

                    int dec_factor = 
                        osc_fpga_cnv_time_range_to_dec(time_range);

                    trig_time[trig_cnt] = i;
                    trig_cnt = trig_cnt + 1;
                    trig_state = 0;

                    if(trig_cnt >= 3) {
                        period=((trig_time[1]-trig_time[0])/
                                (float)c_osc_fpga_smpl_freq*dec_factor);
                        break;
                    }
                }
            }

            /* We have a winner - calculate the new parameters */
            if ((period>0) || (time_range>=4))
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
                orig_params[TRIG_LEVEL_PARAM].value = trig_level_v;

                orig_params[MIN_GUI_PARAM].value    = 0;
                if(period < 0.1)
                {
                    orig_params[MAX_GUI_PARAM].value =  period * 1.5 * t_unit_factor;
                }
                else {
                     // 0.5 s fits into 1s TimeRange
                    orig_params[MAX_GUI_PARAM].value  = 0.5 * 1.5 * t_unit_factor;
                }

                orig_params[TIME_UNIT_PARAM].value  = time_unit;
                orig_params[AUTO_FLAG_PARAM].value  = 0;

                min_y = (min_cha < min_chb) ? min_cha : min_chb;
                max_y = (max_cha > max_chb) ? max_cha : max_chb;

                if (loc_max> max_y)
                  max_y=loc_max;

                if (loc_min< min_y)
                  min_y=loc_min;

                ave_y = (min_y + max_y)>>1;
                amp_y = ((max_y - min_y) >> 1) * 1.2;
                min_y = ave_y - amp_y;
                max_y = ave_y + amp_y;

                orig_params[MIN_Y_PARAM].value = (min_y * max_adc_v)/
                    (float)(1<<(c_osc_fpga_adc_bits-1));
                orig_params[MAX_Y_PARAM].value = (max_y * max_adc_v)/
                    (float)(1<<(c_osc_fpga_adc_bits-1));
        
        orig_params[TRIG_LEVEL_PARAM].value =((float)(loc_max+loc_min))/2* max_adc_v/
            (float)(1<<(c_osc_fpga_adc_bits-1));    

            
                return 0;
            }
           
        }
    }
    return -1;
}



/*----------------------------------------------------------------------------------*/
int rp_osc_adc_sign(int in_data)
{
    int s_data = in_data;
    if(s_data & (1<<(c_osc_fpga_adc_bits-1)))
        s_data = -1 * ((s_data ^ ((1<<c_osc_fpga_adc_bits)-1)) + 1);
    return s_data;
}

int rp_load_data(float save_data){
    /* If the user wants to load all the data */
    if(save_data == 2){
        float val = 0;
        
        if(fopen("/opt/redpitaya/www/apps/impedance_analyzer/lcr_param_data", "r") == NULL){
            printf("Parameter data failure!\n");
            return -1;
        }

        /* Read data */
        FILE *data = fopen("/opt/redpitaya/www/apps/impedance_analyzer/lcr_param_data", "r");

        int counter = 4;
        while(!feof(data)){
            val = 0;
            /* Read data into val */
            fscanf(data, "%f", &val);
            /* Set according parameters with value val */
            rp_set_params_lcr(counter, val);
            counter++;
        }
        rp_set_params_lcr(2, 3);
        fclose(data);

    }
    return 0;
}

