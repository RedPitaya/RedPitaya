/**
 * $Id: worker.c 881 2013-12-16 05:37:34Z rp_jmenart $
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
#include "house_kp.h"
 #include "dsp.h"

pthread_t *rp_pwr_thread_handler_1 = NULL;
void *rp_pwr_worker_thread(void *args);
pthread_t *rp_pwr_thread_handler_2 = NULL;
void *rp_pwr_dsp_thread(void *args);

extern const int pwr_dft_harmonic_num;
extern const int c_dsp_sig_len;

pthread_mutex_t       rp_pwr_ctrl_mutex = PTHREAD_MUTEX_INITIALIZER;
rp_pwr_worker_state_t rp_pwr_ctrl;
rp_app_params_t       *rp_pwr_params = NULL;
int                   rp_pwr_params_dirty;
int                   rp_pwr_dsp_params_dirty;
int                   rp_pwr_params_fpga_update;

pthread_mutex_t       rp_pwr_sig_mutex = PTHREAD_MUTEX_INITIALIZER;
float               **rp_pwr_signals;
int                   rp_pwr_signals_dirty = 0;
int                   rp_pwr_sig_last_idx = 0;
float               **rp_tmp_signals; /* used for calculation, only from worker */

pthread_mutex_t   rp_pwr_dsp_sig_mutex = PTHREAD_MUTEX_INITIALIZER;
int               rp_pwr_dsp_sig_ready = 0;

/* Signals directly pointing at the FPGA mem space */
int                  *rp_fpga_cha_signal, *rp_fpga_chb_signal;

/* Internal structures */
/* Size = PWR_FPGA_SIG_LEN  */
int *rp_cha_buffer = NULL;
int *rp_chb_buffer = NULL;
double *rp_cha_in = NULL;
double *rp_chb_in = NULL;
double *rp_ch_hann = NULL;

/* Size = calculated for coherency  */
double *rp_cha_in_trunc = NULL;
double *rp_chb_in_trunc = NULL;
double *rp_cha_hann_trunc = NULL;
double *rp_chb_hann_trunc = NULL;

/* DSP out structures */
/* Size = pwr_dft_harmonic number*/
double *rp_dft_o_amp_U = NULL;
double *rp_dft_o_amp_I = NULL;
double *rp_dft_o_fi_U = NULL;
double *rp_dft_o_fi_I = NULL;

rp_pwr_harm_t *harmonics = NULL;

/* Calibration parameters read from EEPROM */
rp_calib_params_t *rp_calib_params = NULL;


/*----------------------------------------------------------------------------------*/
int rp_pwr_worker_init(rp_app_params_t *params, int params_len,
                       rp_calib_params_t *calib_params)
{
    int ret_val_1;
    int ret_val_2;

    rp_pwr_ctrl               = rp_pwr_idle_state;
    rp_pwr_params_dirty       = 0;
    rp_pwr_dsp_params_dirty   = 0;
    rp_pwr_params_fpga_update = 0;

    rp_copy_params(params, (rp_app_params_t **)&rp_pwr_params);

    rp_cleanup_signals(&rp_pwr_signals);
    if(rp_create_signals(&rp_pwr_signals) < 0)
        return -1;

    rp_cleanup_signals(&rp_tmp_signals);
    if(rp_create_signals(&rp_tmp_signals) < 0) {
        rp_cleanup_signals(&rp_pwr_signals);
        return -1;
    }

    if(pwr_fpga_init() < 0) {
        rp_cleanup_signals(&rp_pwr_signals);
        rp_cleanup_signals(&rp_tmp_signals);
        return -1;
    }
    
    if(house_init() < 0) {
		house_release();
		return -1;
	}
	
	rp_cha_buffer = (int *)malloc(sizeof(int) * PWR_FPGA_SIG_LEN);
    rp_chb_buffer = (int *)malloc(sizeof(int) * PWR_FPGA_SIG_LEN);
    rp_cha_in = (double *)malloc(sizeof(double) * PWR_FPGA_SIG_LEN);
    rp_chb_in = (double *)malloc(sizeof(double) * PWR_FPGA_SIG_LEN);
    rp_ch_hann = (double *)malloc(sizeof(double) * PWR_FPGA_SIG_LEN);
    
    rp_cha_in_trunc = (double *)malloc(sizeof(double) * PWR_FPGA_SIG_LEN);
    rp_chb_in_trunc = (double *)malloc(sizeof(double) * PWR_FPGA_SIG_LEN);
    
    rp_cha_hann_trunc = (double *)malloc(sizeof(double) * PWR_FPGA_SIG_LEN);
    rp_chb_hann_trunc = (double *)malloc(sizeof(double) * PWR_FPGA_SIG_LEN);
    
    rp_dft_o_amp_U = (double *)malloc(sizeof(double) * pwr_dft_harmonic_num);
    rp_dft_o_amp_I = (double *)malloc(sizeof(double) * pwr_dft_harmonic_num);
    rp_dft_o_fi_U = (double *)malloc(sizeof(double) * pwr_dft_harmonic_num);
    rp_dft_o_fi_I = (double *)malloc(sizeof(double) * pwr_dft_harmonic_num);
    
    harmonics = (rp_pwr_harm_t *)malloc(sizeof(rp_pwr_harm_t) * 40);
     
    if(!rp_cha_buffer || !rp_chb_buffer ||
       !rp_cha_in || !rp_chb_in || !rp_ch_hann || !rp_cha_in_trunc || 
       !rp_chb_in_trunc || !rp_cha_hann_trunc || !rp_chb_hann_trunc ||
       !rp_dft_o_amp_U || !rp_dft_o_amp_I || 
       !rp_dft_o_fi_U || !rp_dft_o_fi_I || !harmonics) {
        rp_pwr_worker_clean();
        return -1;
    }
    
    if(rp_pwr_dft_init() < 0) {
        rp_pwr_worker_clean();
        return -1;
    }

    rp_calib_params = calib_params;

    pwr_fpga_get_sig_ptr(&rp_fpga_cha_signal, &rp_fpga_chb_signal);

    rp_pwr_thread_handler_1 = (pthread_t *)malloc(sizeof(pthread_t));
    if(rp_pwr_thread_handler_1 == NULL) {
        rp_cleanup_signals(&rp_pwr_signals);
        rp_cleanup_signals(&rp_tmp_signals);
        return -1;
    }
    rp_pwr_thread_handler_2 = (pthread_t *)malloc(sizeof(pthread_t));
    if(rp_pwr_thread_handler_2 == NULL) {
        rp_cleanup_signals(&rp_pwr_signals);
        rp_cleanup_signals(&rp_tmp_signals);
        return -1;
    }
    
    ret_val_1= 
        pthread_create(rp_pwr_thread_handler_1, NULL, rp_pwr_worker_thread, NULL);
    ret_val_2= 
        pthread_create(rp_pwr_thread_handler_2, NULL, rp_pwr_dsp_thread, NULL);
    if(ret_val_1 != 0 || ret_val_2 !=0) {
        pwr_fpga_exit();

        rp_cleanup_signals(&rp_pwr_signals);
        rp_cleanup_signals(&rp_tmp_signals);
        fprintf(stderr, "pthread_create() failed: %s\n", 
                strerror(errno));
        return -1;
    }

    return 0;
}
/*----------------------------------------------------------------------------------*/
int rp_pwr_worker_clean(void)
{
    rp_cleanup_signals(&rp_pwr_signals);
    rp_cleanup_signals(&rp_tmp_signals);
    rp_pwr_hann_clean();
    rp_pwr_fft_clean();
    rp_pwr_dft_clean();
    
    if(rp_cha_buffer) {
        free(rp_cha_buffer);
        rp_cha_buffer = NULL;
    }
    if(rp_chb_buffer) {
        free(rp_chb_buffer);
        rp_chb_buffer = NULL;
    }
    if(rp_cha_in) {
        free(rp_cha_in);
        rp_cha_in = NULL;
    }
    if(rp_chb_in) {
        free(rp_chb_in);
        rp_chb_in = NULL;
    }
    if(rp_ch_hann) {
        free(rp_ch_hann);
        rp_ch_hann = NULL;
    }
    if(rp_cha_in_trunc) {
        free(rp_cha_in_trunc);
        rp_cha_in_trunc = NULL;
    }
    if(rp_chb_in_trunc) {
        free(rp_chb_in_trunc);
        rp_chb_in_trunc = NULL;
    }
    if(rp_cha_hann_trunc) {
        free(rp_cha_hann_trunc);
        rp_cha_hann_trunc = NULL;
    }
    if(rp_chb_hann_trunc) {
        free(rp_chb_hann_trunc);
        rp_chb_hann_trunc = NULL;
    }
    if(rp_dft_o_amp_U) {
        free(rp_dft_o_amp_U);
        rp_dft_o_amp_U = NULL;
    }
    if(rp_dft_o_amp_I) {
        free(rp_dft_o_amp_I);
        rp_dft_o_amp_I = NULL;
    }
    if(rp_dft_o_fi_U) {
        free(rp_dft_o_fi_U);
        rp_dft_o_fi_U = NULL;
    }
    if(rp_dft_o_fi_I) {
        free(rp_dft_o_fi_I);
        rp_dft_o_fi_I = NULL;
    }
    if(harmonics) {
        free(harmonics);
        harmonics = NULL;
    }

    return 0;
}

/*----------------------------------------------------------------------------------*/
int rp_pwr_worker_exit(void)
{
    int ret_val_1 = 0;
    int ret_val_2 = 0; 

    rp_pwr_worker_change_state(rp_pwr_quit_state);
    if(rp_pwr_thread_handler_1) {
        ret_val_1 = pthread_join(*rp_pwr_thread_handler_1, NULL);
        free(rp_pwr_thread_handler_1);
        rp_pwr_thread_handler_1 = NULL;
    }
    if(rp_pwr_thread_handler_2) {
        ret_val_2 = pthread_join(*rp_pwr_thread_handler_2, NULL);
        free(rp_pwr_thread_handler_2);
        rp_pwr_thread_handler_2 = NULL;
    }
    if(ret_val_1 != 0 || ret_val_2 != 0) {
        fprintf(stderr, "pthread_join() failed: %s\n", 
                strerror(errno));
    }
    
    unset_transducer_and_led();
    house_release();
    pwr_fpga_exit();
    rp_pwr_worker_clean();

    rp_clean_params(rp_pwr_params);

    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_pwr_worker_change_state(rp_pwr_worker_state_t new_state)
{
    if(new_state >= rp_pwr_nonexisting_state)
        return -1;
    pthread_mutex_lock(&rp_pwr_ctrl_mutex);
    rp_pwr_ctrl = new_state;
    pthread_mutex_unlock(&rp_pwr_ctrl_mutex);
    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_pwr_worker_get_state(rp_pwr_worker_state_t *state)
{
    pthread_mutex_lock(&rp_pwr_ctrl_mutex);
    *state = rp_pwr_ctrl;
    pthread_mutex_unlock(&rp_pwr_ctrl_mutex);
    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_pwr_worker_update_params(rp_app_params_t *params, int fpga_update)
{
    pthread_mutex_lock(&rp_pwr_ctrl_mutex);
    rp_copy_params(params, (rp_app_params_t **)&rp_pwr_params);
    rp_pwr_params_dirty       = 1;
    rp_pwr_dsp_params_dirty   = 1;
    rp_pwr_params_fpga_update = fpga_update;
    rp_pwr_params[PARAMS_NUM].name = NULL;
    rp_pwr_params[PARAMS_NUM].value = -1;

    pthread_mutex_unlock(&rp_pwr_ctrl_mutex);
    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_pwr_clean_signals(void)
{
    pthread_mutex_lock(&rp_pwr_sig_mutex);
    rp_pwr_signals_dirty = 0;
    pthread_mutex_unlock(&rp_pwr_sig_mutex);
    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_pwr_get_signals(float ***signals, int *sig_idx)
{
    float **s = *signals;
    pthread_mutex_lock(&rp_pwr_sig_mutex);
    if(rp_pwr_signals_dirty == 0) {
        *sig_idx = rp_pwr_sig_last_idx;
        pthread_mutex_unlock(&rp_pwr_sig_mutex);
        return -1;
    }

    memcpy(&s[0][0], &rp_pwr_signals[0][0], sizeof(float)*SIGNAL_LENGTH);
    memcpy(&s[1][0], &rp_pwr_signals[1][0], sizeof(float)*SIGNAL_LENGTH);
    memcpy(&s[2][0], &rp_pwr_signals[2][0], sizeof(float)*SIGNAL_LENGTH);

    *sig_idx = rp_pwr_sig_last_idx;

    rp_pwr_signals_dirty = 0;
    pthread_mutex_unlock(&rp_pwr_sig_mutex);
    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_pwr_set_signals(float **source, int index)
{
    pthread_mutex_lock(&rp_pwr_sig_mutex);

    memcpy(&rp_pwr_signals[0][0], &source[0][0], sizeof(float)*SIGNAL_LENGTH);
    memcpy(&rp_pwr_signals[1][0], &source[1][0], sizeof(float)*SIGNAL_LENGTH);
    memcpy(&rp_pwr_signals[2][0], &source[2][0], sizeof(float)*SIGNAL_LENGTH);
    rp_pwr_sig_last_idx = index;

    rp_pwr_signals_dirty = 1;
    pthread_mutex_unlock(&rp_pwr_sig_mutex);

    return 0;
}
/*----------------------------------------------------------------------------------*/
int rp_pwr_copy_buffer(double *cha, double *chb, int calib_dc_off_a, int calib_dc_off_b)
{
    int idx;
        
    int cnts_a = 0;
    int cnts_b = 0;    
    int m_a = 0;
    int m_b = 0;

    pthread_mutex_lock(&rp_pwr_dsp_sig_mutex); 
     
    for(idx = 0; idx < PWR_FPGA_SIG_LEN; idx++) {
 
        cnts_a = rp_cha_buffer[idx];
        cnts_b = rp_chb_buffer[idx];
        
         /* check sign */
        if(cnts_a & (1<<(c_pwr_fpga_adc_bits-1))) {
        /* negative number */
        m_a = -1 *((cnts_a ^ ((1<<c_pwr_fpga_adc_bits)-1)) + 1);
        } else {
        /* positive number */
        m_a = cnts_a;
        }
        /* adopt ADC count with calibrated DC offset */
        m_a += calib_dc_off_a;
        
         /* check sign */
        if(cnts_b & (1<<(c_pwr_fpga_adc_bits-1))) {
        /* negative number */
        m_b = -1 *((cnts_b ^ ((1<<c_pwr_fpga_adc_bits)-1)) + 1);
        } else {
        /* positive number */
        m_b = cnts_b;
        }
        /* adopt ADC count with calibrated DC offset */
        m_b += calib_dc_off_b;

        /*cast to double and asign to out array*/
        cha[idx] = (double)m_a;
        chb[idx] = (double)m_b;
        
        /*here we assign the buffer offset values, 
         *if it doesn get overwritten, we get zeros in the output*/
        rp_cha_buffer[idx] = -calib_dc_off_a;
        rp_chb_buffer[idx] = -calib_dc_off_b;
         
    }
    
    pthread_mutex_unlock(&rp_pwr_dsp_sig_mutex);
            
    return 0;
}

/*----------------------------------------------------------------------------------*/
int rp_pwr_set_ch_meas_data(rp_pwr_ch_meas_res_t u_meas, rp_pwr_ch_meas_res_t i_meas)
{
    rp_update_ch_meas_data(u_meas, i_meas);
    return 0;
}

/*----------------------------------------------------------------------------------*/
int rp_pwr_set_meas_data(rp_pwr_meas_res_t pwr_meas)
{
    rp_update_meas_data(pwr_meas);
    return 0;
}

/*----------------------------------------------------------------------------------*/
int rp_pwr_set_harmonics_data(rp_pwr_harm_t *harm_meas)
{
    rp_update_harmonics_data(&harm_meas[0]);
    return 0;
}


/*----------------------------------------------------------------------------------*/
void *rp_pwr_worker_thread(void *args)
{
    rp_pwr_worker_state_t old_state, state;
    rp_app_params_t      *curr_params = NULL;
    int                   fpga_update = 0;
    int                   time_range = 0;
    int                   dec_factor = 0;
    int                   dc_mode = 0; 
    int                   time_vect_update = 0;
    uint32_t              trig_source = 0;
    int                   params_dirty = 0;
    float                 volt_probe_att = 1;
    float                 curr_probe_fact = 1;

    /* Long acquisition special function */
    int long_acq = 0; /* long_acq if acq_time > 1 [s] */
    int long_acq_idx = 0;
    int long_acq_first_wr_ptr = 0;
    int long_acq_last_wr_ptr = 0;
    int long_acq_step = 0;
    int long_acq_init_trig_ptr;
    
    rp_pwr_meas_res_t    meas;
    rp_pwr_ch_meas_res_t chu_meas, chi_meas;
    float ch1_max_adc_v = 1, ch2_max_adc_v = 1;
    float max_adc_norm = pwr_fpga_calc_adc_max_v(rp_calib_params->fe_ch1_fs_g_lo);

    pthread_mutex_lock(&rp_pwr_ctrl_mutex);
    old_state = state = rp_pwr_ctrl;
    pthread_mutex_unlock(&rp_pwr_ctrl_mutex);


    while(1) {
        /* update states - we save also old state to see if we need to reset
         * FPGA 
         */
        old_state = state;
        pthread_mutex_lock(&rp_pwr_ctrl_mutex);
        state = rp_pwr_ctrl;
        if(rp_pwr_params_dirty) {
			pthread_mutex_lock(&rp_pwr_dsp_sig_mutex);
	        rp_pwr_dsp_sig_ready = 0;
            pthread_mutex_unlock(&rp_pwr_dsp_sig_mutex);
            rp_copy_params(rp_pwr_params, (rp_app_params_t **)&curr_params);
            fpga_update = rp_pwr_params_fpga_update;

            rp_pwr_params_dirty = 0;
            time_range = curr_params[TIME_RANGE_PARAM].value;
            dec_factor = pwr_fpga_cnv_time_range_to_dec(time_range);
            if (time_range == 6) {
				dc_mode = 1;
		    } else {
			    dc_mode = 0;
			}			
            time_vect_update = 1;

            volt_probe_att = voltage_probe_factor_select(curr_params[DIFF_PRB_ATT].value, 0);
            curr_probe_fact = probe_select_to_factor(curr_params[CURR_PROBE_SELECT].value, 0);

            uint32_t fe_fsg1 = (curr_params[GAIN_CH1].value == 0) ?
                    rp_calib_params->fe_ch1_fs_g_hi :
                    rp_calib_params->fe_ch1_fs_g_lo;
            ch1_max_adc_v =
                    pwr_fpga_calc_adc_max_v(fe_fsg1);

            uint32_t fe_fsg2 = (curr_params[GAIN_CH2].value == 0) ?
                    rp_calib_params->fe_ch2_fs_g_hi :
                    rp_calib_params->fe_ch2_fs_g_lo;
            ch2_max_adc_v =
                    pwr_fpga_calc_adc_max_v(fe_fsg2);
        }
        pthread_mutex_unlock(&rp_pwr_ctrl_mutex);

        /* request to stop worker thread, we will shut down */
        if(state == rp_pwr_quit_state) {
            rp_clean_params(curr_params);
            return 0;
        }

        if(state == rp_pwr_auto_set_state) {
            /* Auto-set algorithm was selected - run it */
            rp_pwr_auto_set(curr_params, ch1_max_adc_v, ch2_max_adc_v,
                            curr_params[GEN_DC_OFFS_1].value,
                            curr_params[GEN_DC_OFFS_2].value,
                            volt_probe_att,	curr_probe_fact,
                            curr_params[GAIN_CH1].value,
                            curr_params[GAIN_CH2].value, 			    
                            curr_params[EN_AVG_AT_DEC].value);
            /* Return calculated parameters to main module */
            rp_update_main_params(curr_params);
            continue;
        }
        if(fpga_update) {
            pwr_fpga_reset();
            if(pwr_fpga_update_params((curr_params[TRIG_MODE_PARAM].value == 0),
                                      curr_params[TRIG_SRC_PARAM].value, 
                                      curr_params[TRIG_EDGE_PARAM].value,
                                      /* Here we could use trigger, but it is safer
                                       * to use start GUI value (it was recalculated
                                       * correctly already in rp_pwr_main() so we
                                       * can use it and be sure that all signal 
                                       * (even if extended becuase of decimation
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
                                      volt_probe_att, curr_probe_fact,
                                      curr_params[GAIN_CH1].value,
                                      curr_params[GAIN_CH2].value,				      
                                      curr_params[EN_AVG_AT_DEC].value) < 0) {
                fprintf(stderr, "Setting of FPGA registers failed\n");
                rp_pwr_worker_change_state(rp_pwr_idle_state);
            }
            trig_source = pwr_fpga_cnv_trig_source(
                                     (curr_params[TRIG_MODE_PARAM].value == 0),
                                      curr_params[TRIG_SRC_PARAM].value,
                                      curr_params[TRIG_EDGE_PARAM].value);

            fpga_update = 0;
        }

        if(state == rp_pwr_idle_state) {
            usleep(10000);
            continue;
        }

        if(time_vect_update) {
			pthread_mutex_lock(&rp_pwr_dsp_sig_mutex);
	        rp_pwr_dsp_sig_ready = 0;
            pthread_mutex_unlock(&rp_pwr_dsp_sig_mutex);
			
            float unit_factor = 
                rp_pwr_get_time_unit_factor(curr_params[TIME_UNIT_PARAM].value);
            float t_acq = (curr_params[MAX_GUI_PARAM].value - 
                           curr_params[MIN_GUI_PARAM].value) / unit_factor;

            rp_pwr_prepare_time_vector((float **)&rp_tmp_signals[0], 
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
                rp_pwr_ch_meas_clear(&chu_meas);
                rp_pwr_ch_meas_clear(&chi_meas);
                pwr_fpga_get_wr_ptr(NULL, &long_acq_init_trig_ptr);
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
            pwr_fpga_reset();
            pwr_fpga_arm_trigger();
        
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
            pwr_fpga_set_trigger(trig_source);
        }

        /* start working */
        pthread_mutex_lock(&rp_pwr_ctrl_mutex);
        old_state = state = rp_pwr_ctrl;
        pthread_mutex_unlock(&rp_pwr_ctrl_mutex);
        if((state == rp_pwr_idle_state) || (state == rp_pwr_abort_state)) {
            continue;
        } else if(state == rp_pwr_quit_state) {
            break;
        }

        if(long_acq_idx == 0) {
            /* polling until data is ready */
            while(1) {
                pthread_mutex_lock(&rp_pwr_ctrl_mutex);
                state = rp_pwr_ctrl;
                params_dirty = rp_pwr_params_dirty;
                pthread_mutex_unlock(&rp_pwr_ctrl_mutex);
                /* change in state, abort polling */
                if((state != old_state) || params_dirty) {
                    break;
                }
                
                if(!long_acq && pwr_fpga_triggered()) {
                    /* for non-long acquisition wait for trigger */
                    break;
                } else if(long_acq) {
                    int trig_ptr, curr_ptr;
                    pwr_fpga_get_wr_ptr(&curr_ptr, &trig_ptr);
                    if((long_acq_init_trig_ptr != trig_ptr) || pwr_fpga_triggered()) {
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
                float smpl_period = c_pwr_fpga_smpl_period * dec_factor;
                int t_start_idx = 
                    round(curr_params[MIN_GUI_PARAM].value / smpl_period);
                float unit_factor = 
                    rp_pwr_get_time_unit_factor(
                                         curr_params[TIME_UNIT_PARAM].value);
                float t_acq = (curr_params[MAX_GUI_PARAM].value - 
                               curr_params[MIN_GUI_PARAM].value) / 
                    unit_factor;
                
                pwr_fpga_get_wr_ptr(&wr_ptr_curr, &wr_ptr_trig);
                long_acq_first_wr_ptr = wr_ptr_trig + t_start_idx - 3;
                if(long_acq_first_wr_ptr < 0)
                    long_acq_first_wr_ptr = 
                        PWR_FPGA_SIG_LEN + long_acq_first_wr_ptr;
                long_acq_last_wr_ptr  = long_acq_first_wr_ptr +
                    (t_acq / (c_pwr_fpga_smpl_period * dec_factor));
                long_acq_last_wr_ptr = long_acq_last_wr_ptr % PWR_FPGA_SIG_LEN;

                if(round((t_acq / (c_pwr_fpga_smpl_period * dec_factor) /
                         (SIGNAL_LENGTH-1))) < 0)
                    long_acq_step = 1;
                else
                    long_acq_step = 
                        round((t_acq / (c_pwr_fpga_smpl_period * dec_factor)) / 
                              (SIGNAL_LENGTH-1));

                rp_pwr_ch_meas_clear(&chu_meas);
                rp_pwr_ch_meas_clear(&chi_meas);
            }
             
            /* we are after trigger - so let's wait a while to collect some 
            * samples */
            usleep(long_acq_part_delay); /* Sleep for 200 [ms] */
        }

        pthread_mutex_lock(&rp_pwr_ctrl_mutex);
        state = rp_pwr_ctrl;
        params_dirty = rp_pwr_params_dirty;
        pthread_mutex_unlock(&rp_pwr_ctrl_mutex);

        if((state != old_state) || params_dirty)
            continue;

        if(!long_acq) {
            /* Triggered, decimate & convert the values */
            rp_pwr_ch_meas_clear(&chu_meas);
            rp_pwr_ch_meas_clear(&chi_meas);
            rp_pwr_decimate((float **)&rp_tmp_signals[1], &rp_fpga_cha_signal[0],
                            (float **)&rp_tmp_signals[2], &rp_fpga_chb_signal[0],
                            (float **)&rp_tmp_signals[0], dec_factor, 
                            curr_params[MIN_GUI_PARAM].value,
                            curr_params[MAX_GUI_PARAM].value,
                            curr_params[TIME_UNIT_PARAM].value, 
                            &chu_meas, &chi_meas, ch1_max_adc_v, ch2_max_adc_v,
                            curr_params[GEN_DC_OFFS_1].value,
                            curr_params[GEN_DC_OFFS_2].value,
                            volt_probe_att, curr_probe_fact);
        } else {
            long_acq_idx = rp_pwr_decimate_partial((float **)&rp_tmp_signals[1], 
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
                                             &chu_meas, &chi_meas,
                                             ch1_max_adc_v, ch2_max_adc_v,
                                             curr_params[GEN_DC_OFFS_1].value,
                                             curr_params[GEN_DC_OFFS_2].value,
                                             volt_probe_att, curr_probe_fact);

            /* Acquisition over, start one more! */
            if(long_acq_idx >= SIGNAL_LENGTH-1) {
                long_acq_idx = 0;
                
                pthread_mutex_lock(&rp_pwr_dsp_sig_mutex);
	            rp_pwr_dsp_sig_ready = 1;
                pthread_mutex_unlock(&rp_pwr_dsp_sig_mutex);

                pwr_fpga_get_wr_ptr(NULL, &long_acq_init_trig_ptr);

                if(state == rp_pwr_single_state) {
                    rp_pwr_worker_change_state(rp_pwr_idle_state);
                }
            }
            
        }

        /* check again for change of state */
        pthread_mutex_lock(&rp_pwr_ctrl_mutex);
        state = rp_pwr_ctrl;
        pthread_mutex_unlock(&rp_pwr_ctrl_mutex);

        /* We have acquisition - if we are in single put state machine
         * to idle */
        if((state == rp_pwr_single_state) && (!long_acq)) {
            rp_pwr_worker_change_state(rp_pwr_idle_state);
        }

        
        /* copy the results to the user buffer - if we are finished or not */
        if(!long_acq || long_acq_idx == 0) {
            /* Finish the measurement */
            rp_pwr_meas_avg_amp(&chu_meas, PWR_FPGA_SIG_LEN);
            rp_pwr_meas_avg_amp(&chi_meas, PWR_FPGA_SIG_LEN);
            
            rp_pwr_meas_convert(&chu_meas, ch1_max_adc_v, 
                                rp_calib_params->fe_ch1_dc_offs, volt_probe_att);
            rp_pwr_meas_convert(&chi_meas, ch2_max_adc_v, 
                                rp_calib_params->fe_ch2_dc_offs, curr_probe_fact);
            if (dc_mode == 1){
			   rp_pwr_meas_clear(&meas);
			   meas.p = chu_meas.avg * chi_meas.avg;
			   rp_pwr_set_meas_data(meas);	
			}
            
            rp_pwr_set_ch_meas_data(chu_meas, chi_meas);
            rp_pwr_set_signals(rp_tmp_signals, SIGNAL_LENGTH-1);
        } else {
            rp_pwr_set_signals(rp_tmp_signals, long_acq_idx);
        }
        /* do not loop too fast */
        usleep(10000);
    }

    rp_clean_params(curr_params);
    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_pwr_prepare_time_vector(float **out_signal, int dec_factor,
                               float t_start, float t_stop, int time_unit)
{
    float smpl_period = c_pwr_fpga_smpl_period * dec_factor;
    float t_step, t_curr;
    int   out_idx, in_idx;
    int   idx_step;
    int   t_unit_factor = rp_pwr_get_time_unit_factor(time_unit);;

    float *s = *out_signal;

    if(t_stop <= t_start) {
        t_start = 0;
        t_stop  = PWR_FPGA_SIG_LEN * smpl_period;
    }

    t_step = (t_stop - t_start) / (SIGNAL_LENGTH-1);
    idx_step = (int)(ceil(t_step/smpl_period));
    if(idx_step > 8)
        idx_step = 8;

    for(out_idx = 0, in_idx = 0, t_curr=t_start; out_idx < SIGNAL_LENGTH; 
        out_idx++, t_curr += t_step, in_idx += idx_step) {
        s[out_idx] = t_curr * t_unit_factor;
    }
    
    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_pwr_decimate(float **cha_signal, int *in_cha_signal,
                    float **chb_signal, int *in_chb_signal,
                    float **time_signal, int dec_factor, 
                    float t_start, float t_stop, int time_unit,
                    rp_pwr_ch_meas_res_t *ch1_meas, rp_pwr_ch_meas_res_t *ch2_meas,
                    float ch1_max_adc_v, float ch2_max_adc_v,
                    float ch1_user_dc_off, float ch2_user_dc_off,
                    float diff_probe_att, float probe_fact)
{
    int t_start_idx, t_stop_idx;
    float smpl_period = c_pwr_fpga_smpl_period * dec_factor;
    int   t_unit_factor = rp_pwr_get_time_unit_factor(time_unit);
    int t_step;
    int in_idx, out_idx, t_idx;
    int wr_ptr_curr, wr_ptr_trig;

    float *cha_s = *cha_signal;
    float *chb_s = *chb_signal;
    float *t = *time_signal;
    
    /* If illegal take whole frame */
    if(t_stop <= t_start) {
        t_start = 0;
        t_stop = (PWR_FPGA_SIG_LEN-1) * smpl_period;
    }
    
    /* convert time to samples */
    t_start_idx = round(t_start / smpl_period);
    t_stop_idx  = round(t_stop / smpl_period);

    if((((t_stop_idx-t_start_idx)/(float)(SIGNAL_LENGTH-1))) < 1)
        t_step = 1;
    else {
        /* ceil was used already in rp_pwr_main() for parameters, so we can easily
         * use round() here 
         */
        t_step = round((t_stop_idx-t_start_idx)/(float)(SIGNAL_LENGTH-1));
    }
    pwr_fpga_get_wr_ptr(&wr_ptr_curr, &wr_ptr_trig);
    in_idx = wr_ptr_trig + t_start_idx - 3;

    if(in_idx < 0) 
        in_idx = PWR_FPGA_SIG_LEN + in_idx;
    if(in_idx >= PWR_FPGA_SIG_LEN)
        in_idx = in_idx % PWR_FPGA_SIG_LEN;

    /* First perform measurements on non-decimated signal:
     *  - min, max - performed in the loop
     *  - avg, amp - performed after the loop
     *  - freq, period - performed in the next decimation loop
     */
    for(out_idx=0; out_idx < PWR_FPGA_SIG_LEN; out_idx++) {
		pthread_mutex_lock(&rp_pwr_dsp_sig_mutex);
		rp_pwr_dsp_sig_ready = 0;
		rp_cha_buffer[out_idx] = in_cha_signal[out_idx];
		rp_chb_buffer[out_idx] = in_chb_signal[out_idx];
		pthread_mutex_unlock(&rp_pwr_dsp_sig_mutex);
        rp_pwr_meas_min_max(ch1_meas, in_cha_signal[out_idx]);
        rp_pwr_meas_min_max(ch2_meas, in_chb_signal[out_idx]);
    }
    
    pthread_mutex_lock(&rp_pwr_dsp_sig_mutex);
	rp_pwr_dsp_sig_ready = 1;
    pthread_mutex_unlock(&rp_pwr_dsp_sig_mutex);

    

    for(out_idx=0, t_idx=0; out_idx < SIGNAL_LENGTH; 
        out_idx++, in_idx+=t_step, t_idx+=t_step) {
        /* Wrap the pointer */
        if(in_idx >= PWR_FPGA_SIG_LEN)
            in_idx = in_idx % PWR_FPGA_SIG_LEN;

        cha_s[out_idx] = pwr_fpga_cnv_cnt_to_v(in_cha_signal[in_idx], ch1_max_adc_v,
                                               rp_calib_params->fe_ch1_dc_offs,
                                               ch1_user_dc_off, diff_probe_att);

        chb_s[out_idx] = pwr_fpga_cnv_cnt_to_a(in_chb_signal[in_idx], ch2_max_adc_v,
                                               rp_calib_params->fe_ch2_dc_offs,
                                               ch2_user_dc_off, probe_fact);

        t[out_idx] = (t_start + (t_idx * smpl_period)) * t_unit_factor;

        /* A bug in FPGA? - Trig & write pointers not sample-accurate. */
        if ( (dec_factor > 64) && (out_idx == 1) ) {
            int i;
            for (i=0; i < out_idx; i++) {
                cha_s[i] = cha_s[out_idx];
                chb_s[i] = chb_s[out_idx];
            }
        }
    }

    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_pwr_decimate_partial(float **cha_out_signal, int *cha_in_signal, 
                            float **chb_out_signal, int *chb_in_signal,
                            float **time_out_signal, int *next_wr_ptr, 
                            int last_wr_ptr, int step_wr_ptr, int next_out_idx,
                            float t_start, int dec_factor, int time_unit,
                            rp_pwr_ch_meas_res_t *ch1_meas, 
                            rp_pwr_ch_meas_res_t *ch2_meas,
                            float ch1_max_adc_v, float ch2_max_adc_v,
                            float ch1_user_dc_off, float ch2_user_dc_off,
                            float diff_probe_att, float probe_fact)
{
    float *cha_out = *cha_out_signal;
    float *chb_out = *chb_out_signal;
    float *t_out   = *time_out_signal;
    int    in_idx = *next_wr_ptr;

    float smpl_period = c_pwr_fpga_smpl_period * dec_factor;
    int   t_unit_factor = rp_pwr_get_time_unit_factor(time_unit);

    int curr_ptr;
    /* check if we have reached currently acquired signals in FPGA */
    pwr_fpga_get_wr_ptr(&curr_ptr, NULL);

    for(; in_idx < curr_ptr; in_idx++) {
        if(in_idx >= PWR_FPGA_SIG_LEN)
            in_idx = in_idx % PWR_FPGA_SIG_LEN;
        pthread_mutex_lock(&rp_pwr_dsp_sig_mutex);
		rp_pwr_dsp_sig_ready = 0;
		rp_cha_buffer[in_idx] = cha_in_signal[in_idx];
		rp_chb_buffer[in_idx] = chb_in_signal[in_idx];
		pthread_mutex_unlock(&rp_pwr_dsp_sig_mutex);
        rp_pwr_meas_min_max(ch1_meas, cha_in_signal[in_idx]);
        rp_pwr_meas_min_max(ch2_meas, chb_in_signal[in_idx]);
    }

    in_idx = *next_wr_ptr;

    for(; (next_out_idx < SIGNAL_LENGTH); next_out_idx++, 
            in_idx += step_wr_ptr) {
        int curr_ptr;
        int diff_ptr;
        /* check if we have reached currently acquired signals in FPGA */
        pwr_fpga_get_wr_ptr(&curr_ptr, NULL);
        if(in_idx >= PWR_FPGA_SIG_LEN)
            in_idx = in_idx % PWR_FPGA_SIG_LEN;
        diff_ptr = (in_idx-curr_ptr);
        /* Check that we did not hit the curr ptr (and that pointer is not
         * wrapped 
         */
        if((in_idx >= curr_ptr) && (diff_ptr > 0) && (diff_ptr < 100))
            break;

        cha_out[next_out_idx] = 
            pwr_fpga_cnv_cnt_to_v(cha_in_signal[in_idx], ch1_max_adc_v,
                                  rp_calib_params->fe_ch1_dc_offs,
                                  ch1_user_dc_off, diff_probe_att);

        chb_out[next_out_idx] = 
            pwr_fpga_cnv_cnt_to_a(chb_in_signal[in_idx], ch2_max_adc_v,
                                  rp_calib_params->fe_ch1_dc_offs,
                                  ch2_user_dc_off, probe_fact);

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
int rp_pwr_get_time_unit_factor(int time_unit)
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
int rp_pwr_auto_set(rp_app_params_t *orig_params, 
                    float ch1_max_adc_v, float ch2_max_adc_v,
                    float ch1_user_dc_off, float ch2_user_dc_off,
                    float ch1_probe_att, float ch2_probe_att, int ch1_gain, 
                    int ch2_gain, int en_avg_at_dec)
{
    const int c_noise_thr = 10; /* noise threshold */
    rp_pwr_worker_state_t old_state, state;
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
        pthread_mutex_lock(&rp_pwr_ctrl_mutex);
        old_state = state = rp_pwr_ctrl;
        pthread_mutex_unlock(&rp_pwr_ctrl_mutex);

        pwr_fpga_reset();
        pwr_fpga_update_params(1, 0, 0, 0, 0, time_range, ch1_max_adc_v, ch2_max_adc_v,
                   rp_calib_params->fe_ch1_dc_offs,
                   0,
                   rp_calib_params->fe_ch2_dc_offs,
                   0,
                   ch1_probe_att, ch2_probe_att, ch1_gain, ch2_gain, 0);

        /* ARM & Trigger */
        pwr_fpga_arm_trigger();
        pwr_fpga_set_trigger(1);

        /* Wait for trigger to finish */
        while(1) {
            pthread_mutex_lock(&rp_pwr_ctrl_mutex);
            state = rp_pwr_ctrl;
            params_dirty = rp_pwr_params_dirty;
            pthread_mutex_unlock(&rp_pwr_ctrl_mutex);
            /* change in state, abort polling */
            if((state != old_state) || params_dirty) {
                return -1;
            }
            if(pwr_fpga_triggered()) {
                break;
            }
            usleep(500);
        }

        /* Get the signals - available at rp_fpga_chX_signal vectors */
        for(smpl_cnt = 0; smpl_cnt < PWR_FPGA_SIG_LEN; smpl_cnt++) {
            int cha_smpl = rp_fpga_cha_signal[smpl_cnt];
            int chb_smpl = rp_fpga_chb_signal[smpl_cnt];

            // TWO'S COMPLEMENT
            if(cha_smpl & (1<<(c_pwr_fpga_adc_bits-1)))
                cha_smpl = -1 * ((cha_smpl ^ ((1<<c_pwr_fpga_adc_bits)-1))+1);
            if(chb_smpl & (1<<(c_pwr_fpga_adc_bits-1)))
                chb_smpl = -1 * ((chb_smpl ^ ((1<<c_pwr_fpga_adc_bits)-1))+1);

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

        orig_params[MIN_Y_NORM_U].value = min_y / (float)(1 << (c_pwr_fpga_adc_bits - 1)); 
        orig_params[MAX_Y_NORM_U].value = max_y / (float)(1 << (c_pwr_fpga_adc_bits - 1));
        orig_params[MIN_Y_NORM_I].value = min_y / (float)(1 << (c_pwr_fpga_adc_bits - 1));
        orig_params[MAX_Y_NORM_I].value = max_y / (float)(1 << (c_pwr_fpga_adc_bits - 1));
        orig_params[MIN_Y_NORM_P].value = min_y;											//kr ene vrednosti, sam za probo
        orig_params[MAX_Y_NORM_P].value = max_y;

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
                        (float)(1<<(c_pwr_fpga_adc_bits-1))) ;

        int loc_max = INT_MIN;
        int loc_min = INT_MAX;

        /* Loop over time ranges and search for the best suiting one (Last range removed: too slow) */
        for(time_range = 0; time_range < 5; time_range++) {
            float period = 0;
            int trig_source = pwr_fpga_cnv_trig_source(0, trig_src_ch, 0);
            int *sig_data;

            pwr_fpga_reset();
            pwr_fpga_update_params(0, trig_src_ch, 0, 0, trig_level_v, time_range,
                                   ch1_max_adc_v, ch2_max_adc_v,
                                   rp_calib_params->fe_ch1_dc_offs,
                                   0,
                                   rp_calib_params->fe_ch2_dc_offs,
                                   0, 
                                   ch1_probe_att, ch2_probe_att, ch1_gain, ch2_gain, en_avg_at_dec);

            /* ARM & Trigger */
            pwr_fpga_arm_trigger();
            pwr_fpga_set_trigger(trig_source);

            /* Wait for trigger */
            /* Wait for trigger to finish */
            while(1) {
                pthread_mutex_lock(&rp_pwr_ctrl_mutex);
                state = rp_pwr_ctrl;
                params_dirty = rp_pwr_params_dirty;
                pthread_mutex_unlock(&rp_pwr_ctrl_mutex);

                /* change in state, abort polling */
                if((state != old_state) || params_dirty) {
                    return -1;
                }
                if(pwr_fpga_triggered()) {
                    break;
                }    
                usleep(500);
            }

            // Checking where acquisition starts
            pwr_fpga_get_wr_ptr(&wr_ptr_curr, &wr_ptr_trig);

            /* We have a signal in rp_fpga_chX_signal */
            sig_data = (channel == 0) ? rp_fpga_cha_signal : rp_fpga_chb_signal;
            int dec_factor = pwr_fpga_cnv_time_range_to_dec(time_range);


            rp_pwr_ch_meas_res_t meas;
            rp_pwr_ch_meas_clear(&meas);
            meas.min = min_y;
            meas.max = max_y;

            
            period = meas_period(&meas, sig_data, wr_ptr_trig, 
                                 dec_factor, &loc_min, &loc_max);
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
                                        (float)(1<<(c_pwr_fpga_adc_bits-1));

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

                orig_params[MIN_Y_NORM_U].value = min_y / (float)(1 << (c_pwr_fpga_adc_bits - 1));
                orig_params[MAX_Y_NORM_U].value = max_y / (float)(1 << (c_pwr_fpga_adc_bits - 1));

                // For POST response ...
                transform_to_iface_units(orig_params);
                return 0;
            }
            
            
        }
    }
    return -1;
}


/*----------------------------------------------------------------------------------*/
void *rp_pwr_dsp_thread(void *args)
{
    rp_pwr_worker_state_t state;
    rp_app_params_t      *dsp_params = NULL;
    int                   time_range = 0;
    int                   dec_factor = 0;
    int                   dc_mode = 0; 
    float                 volt_probe_att = 1;
	float                 curr_probe_fact = 1;
	int                   harm_num = 99;
	rp_pwr_meas_res_t     meas;
	float ch1_max_adc_v = 1, ch2_max_adc_v = 1;
	float ch1_user_dc_off = 0, ch2_user_dc_off = 0;
	int   sig_ready = 0;
		
	double bin_max_amp1;
	double bin_max_amp2;
	double bin_max_amp3;
	double bin_max_arg;
	int bin_max_num;
	double bin_max_ampU1;
	double bin_max_ampU2;
	double bin_max_ampU3;
	double bin_max_argU;
	int bin_max_numU;
	double bin_max_ampI1;
	double bin_max_ampI2;
	double bin_max_ampI3;
	double bin_max_argI;
	int bin_max_numI;
		
	int length_half = PWR_FPGA_SIG_LEN / 2;
	int n_x_half = 0;
	int n_x_start = 0;
	
	double freq_fact = 1;
	double d1 = 0;
	double d2U = 0, d2I = 0;
	int b1 = 0, b2 = 0;
	double f1 = 0, f2 = 0;
	double freq_2 = 0;
	double n_y = 0;
	int n_x = 0;
	int n_y2 = 0;
	int n_y2_start = 0;
	int sign;
	int sign_q = 0;
	
	double amp_fft_u_1 = 0;
	double amp_fft_i_1 = 0;
	double fft_fi_1 = 1;
	
	int i, j, ii;
	
	pthread_mutex_lock(&rp_pwr_ctrl_mutex);
    state = rp_pwr_ctrl;
    pthread_mutex_unlock(&rp_pwr_ctrl_mutex);

    while(1) {
		
        pthread_mutex_lock(&rp_pwr_ctrl_mutex);
        state = rp_pwr_ctrl;
        if(rp_pwr_dsp_params_dirty) {
            rp_copy_params(rp_pwr_params, (rp_app_params_t **)&dsp_params);

            rp_pwr_dsp_params_dirty = 0;
            time_range = dsp_params[TIME_RANGE_PARAM].value;
            dec_factor = pwr_fpga_cnv_time_range_to_dec(time_range);
            if (time_range == 6) {
				dc_mode = 1;
			} else {
				dc_mode = 0;
			}

            volt_probe_att = voltage_probe_factor_select(dsp_params[DIFF_PRB_ATT].value, 0);
            curr_probe_fact = probe_select_to_factor(dsp_params[CURR_PROBE_SELECT].value, 0);
            harm_num = (dsp_params[HARM_NUM].value);
            
            freq_fact = c_pwr_fpga_smpl_freq / dec_factor;
            
            uint32_t fe_fsg1 = (dsp_params[GAIN_CH1].value == 0) ?
                    rp_calib_params->fe_ch1_fs_g_hi :
                    rp_calib_params->fe_ch1_fs_g_lo;
            ch1_max_adc_v =
                    pwr_fpga_calc_adc_max_v(fe_fsg1);
                   
            ch1_user_dc_off = dsp_params[GEN_DC_OFFS_1].value;      

            uint32_t fe_fsg2 = (dsp_params[GAIN_CH2].value == 0) ?
                    rp_calib_params->fe_ch2_fs_g_hi :
                    rp_calib_params->fe_ch2_fs_g_lo;
            ch2_max_adc_v =
                    pwr_fpga_calc_adc_max_v(fe_fsg2);
                    
            ch2_user_dc_off = dsp_params[GEN_DC_OFFS_2].value;
        }
        pthread_mutex_unlock(&rp_pwr_ctrl_mutex);
        
        /* request to stop worker thread, we will shut down */
        if(state == rp_pwr_quit_state) {
            rp_clean_params(dsp_params);
            return 0;
        }

        if(state == rp_pwr_auto_set_state) {
            usleep(3*10e6);
            continue;
        }
        
        if(dc_mode == 1) {
			usleep(1e6);
            continue;
		}
		
        while(1) {
			
			pthread_mutex_lock(&rp_pwr_dsp_sig_mutex);
			sig_ready = rp_pwr_dsp_sig_ready;
			pthread_mutex_unlock(&rp_pwr_dsp_sig_mutex);
			
			if(sig_ready == 1) {
			    rp_pwr_copy_buffer(&rp_cha_in[0], &rp_chb_in[0],
                                   rp_calib_params->fe_ch1_dc_offs,
                                   rp_calib_params->fe_ch2_dc_offs);
                break;
		    }
		    
		    pthread_mutex_lock(&rp_pwr_ctrl_mutex);
            state = rp_pwr_ctrl;
            pthread_mutex_unlock(&rp_pwr_ctrl_mutex);
            if((state == rp_pwr_quit_state) || (state == rp_pwr_abort_state) ||
               (state == rp_pwr_auto_set_state) || rp_pwr_dsp_params_dirty) {
                break;
            }
		    usleep(70);
		    
	    }
	    
	    pthread_mutex_lock(&rp_pwr_ctrl_mutex);
        state = rp_pwr_ctrl;
        pthread_mutex_unlock(&rp_pwr_ctrl_mutex);
        if((state == rp_pwr_abort_state) ||
           (state == rp_pwr_auto_set_state) || rp_pwr_dsp_params_dirty) {
            continue;
        } else if(state == rp_pwr_quit_state) {
            break;
        }
		
        rp_pwr_hann_init(PWR_FPGA_SIG_LEN);                                                
        rp_pwr_hann_filter(&rp_cha_in[0], &rp_ch_hann[0], PWR_FPGA_SIG_LEN);
        rp_pwr_fft_init(PWR_FPGA_SIG_LEN);
        rp_pwr_fft(&rp_ch_hann[0], &bin_max_amp1, &bin_max_amp2, &bin_max_amp3,
                   &bin_max_arg, &bin_max_num, length_half);
 
        pthread_mutex_lock(&rp_pwr_ctrl_mutex);
        state = rp_pwr_ctrl;
        pthread_mutex_unlock(&rp_pwr_ctrl_mutex);
        if((state == rp_pwr_abort_state) ||
           (state == rp_pwr_auto_set_state) || rp_pwr_dsp_params_dirty) {
            continue;
        } else if(state == rp_pwr_quit_state) {
            break;
        }
                   
        if(bin_max_num > 1) {
            b1 = bin_max_num;
            d1 = rp_pwr_calc_d(bin_max_amp1, bin_max_amp2, bin_max_amp3);
            f1 = b1 + d1;
            n_y = floor(f1) * PWR_FPGA_SIG_LEN / f1;
            
            sign = 0;
            if((int)floor(n_y) % 2 == 0) {
				n_x = (int)floor(n_y);
				sign = 1;
			} else if((int)ceil(n_y) % 2 == 0) {
				n_x = (int)ceil(n_y);
				sign = 2;
			}

			
			ii=0;
			while(rp_pwr_is_fast_enough(n_x) < 1) {
				ii++;
				n_x += pow((-1), (ii+sign)) * (ii * 2);
			}
			
            n_y = (int)round(n_y);
			
            n_x_half = n_x / 2;
			n_x_start = PWR_FPGA_SIG_LEN - n_x;
        
            for(i=0; i<n_x; i++) {
	            rp_cha_in_trunc[i] = rp_cha_in[n_x_start + i];
	            rp_chb_in_trunc[i] = rp_chb_in[n_x_start + i];
	        }
	        
	        pthread_mutex_lock(&rp_pwr_ctrl_mutex);
            state = rp_pwr_ctrl;
            pthread_mutex_unlock(&rp_pwr_ctrl_mutex);
            if((state == rp_pwr_abort_state) ||
               (state == rp_pwr_auto_set_state) || rp_pwr_dsp_params_dirty) {
            continue;
            } else if(state == rp_pwr_quit_state) {
            break;
            }     
                  
            rp_pwr_hann_init(n_x);
            rp_pwr_fft_init(n_x);
            rp_pwr_hann_filter(&rp_cha_in_trunc[0], &rp_cha_hann_trunc[0], n_x);
            rp_pwr_hann_filter(&rp_chb_in_trunc[0], &rp_chb_hann_trunc[0], n_x);

            pthread_mutex_lock(&rp_pwr_ctrl_mutex);
            state = rp_pwr_ctrl;
            pthread_mutex_unlock(&rp_pwr_ctrl_mutex);
            if((state == rp_pwr_abort_state) ||
               (state == rp_pwr_auto_set_state) || rp_pwr_dsp_params_dirty) {
            continue;
            } else if(state == rp_pwr_quit_state) {
            break;
            }
 
            rp_pwr_fft(&rp_cha_hann_trunc[0], &bin_max_ampU1, &bin_max_ampU2, 
                       &bin_max_ampU3, &bin_max_argU, &bin_max_numU, n_x_half);

            rp_pwr_fft(&rp_chb_hann_trunc[0], &bin_max_ampI1, &bin_max_ampI2, 
                       &bin_max_ampI3, &bin_max_argI, &bin_max_numI, n_x_half);
                       
            pthread_mutex_lock(&rp_pwr_ctrl_mutex);
            state = rp_pwr_ctrl;
            pthread_mutex_unlock(&rp_pwr_ctrl_mutex);
            if((state == rp_pwr_abort_state) ||
               (state == rp_pwr_auto_set_state) || rp_pwr_dsp_params_dirty) {
            continue;
            } else if(state == rp_pwr_quit_state) {
            break;
            }          
                                                          
            b2 = bin_max_numU;
            d2U = rp_pwr_calc_d(bin_max_ampU1, bin_max_ampU2, bin_max_ampU3);
            f2 = b2 + d2U;
            
            n_y2 = (int)round(floor(f2) * n_x / f2); //skrajsamo buffer (se za eno periodo), za vecjo natancnost
            n_y2_start = n_x - n_y2;
            freq_2 = f2 * freq_fact / n_x;
            
            f2 = n_y2 * f2 / n_x; //popravimo relativno frekvenco se za napako zaokrozevanja st. samplov
            d2I = rp_pwr_calc_d(bin_max_ampI1, bin_max_ampI2, bin_max_ampI3);         
            amp_fft_u_1 = rp_pwr_calc_interpolated_amp(bin_max_ampU1, bin_max_ampU2, 
                                                       bin_max_ampU3, d2U);          
            amp_fft_i_1 = rp_pwr_calc_interpolated_amp(bin_max_ampI1, bin_max_ampI2, 
                                                       bin_max_ampI3, d2I);
            amp_fft_u_1 = amp_fft_u_1 / n_x;                                         
            amp_fft_i_1 = amp_fft_i_1 / n_x;
                                                          
            fft_fi_1 = bin_max_argI-bin_max_argU ;
            
            pthread_mutex_lock(&rp_pwr_ctrl_mutex);
            state = rp_pwr_ctrl;
            pthread_mutex_unlock(&rp_pwr_ctrl_mutex);
            if((state == rp_pwr_abort_state) ||
               (state == rp_pwr_auto_set_state) || rp_pwr_dsp_params_dirty) {
            continue;
            } else if(state == rp_pwr_quit_state) {
            break;
            }          
            rp_pwr_dft(&rp_cha_in_trunc[n_y2_start], &rp_chb_in_trunc[n_y2_start], 
					   n_y2, f2,
                       &rp_dft_o_amp_U[0], &rp_dft_o_amp_I[0], 
                       &rp_dft_o_fi_U[0], &rp_dft_o_fi_I[0]);

            rp_pwr_meas_clear(&meas);
            
            pthread_mutex_lock(&rp_pwr_ctrl_mutex);
            state = rp_pwr_ctrl;
            pthread_mutex_unlock(&rp_pwr_ctrl_mutex);
            if((state == rp_pwr_abort_state) ||
               (state == rp_pwr_auto_set_state) || rp_pwr_dsp_params_dirty) {
            continue;
            } else if(state == rp_pwr_quit_state) {
            break;
            }   
            
            sign_q = (sin(fft_fi_1) < 0) ? -1 : 1;        
            meas.cos_fi_fund = cos(fft_fi_1);
            meas.freq = freq_2;
        
            double v_h = 0;
            double i_h = 0;
            double sum_v_h = 0;
            double sum_i_h = 0;
            double p_h = 0;
            double q_h = 0;
            double v_temp = 0;
            double i_temp = 0;
            double h_fi_U = 0;
            double h_fi_I = 0;
            int k;
        
            for(j=0; j<pwr_dft_harmonic_num; j++) {
				
				v_temp = pwr_cnv_cnt_to_v(rp_dft_o_amp_U[j], ch1_max_adc_v,
                                          rp_calib_params->fe_ch1_dc_offs,
                                          ch1_user_dc_off, volt_probe_att);
		    	i_temp = pwr_cnv_cnt_to_a(rp_dft_o_amp_I[j], ch2_max_adc_v,
                                          rp_calib_params->fe_ch2_dc_offs,
                                          ch2_user_dc_off, curr_probe_fact);
				
				if (j<40) {
					
				    harmonics[j].U = v_temp / SQRT2;                                          
				    harmonics[j].I = i_temp / SQRT2;
				    				    
					h_fi_U = rp_dft_o_fi_U[j]-(j+1)*rp_dft_o_fi_U[0];
				   					   		   	 
				    if (h_fi_U <=  (-M_PI) ) {
						k = round(fabs(h_fi_U)/(2*M_PI));
                        h_fi_U = h_fi_U +(k*2*M_PI);
                    }
                    else if ( h_fi_U >= M_PI ) {
						k = round(fabs(h_fi_U)/(2*M_PI));
                        h_fi_U = h_fi_U -(k*2*M_PI) ;
                    }
                    else {
                         h_fi_U = h_fi_U;
                    }
                    
                    h_fi_I = rp_dft_o_fi_I[j] - (j+1)*rp_dft_o_fi_I[0];
                     
                    if (h_fi_I <=  (-M_PI) ) {
						k = round(fabs(h_fi_I)/(2*M_PI));
                        h_fi_I = h_fi_I +(k*2*M_PI);
                    }
                    else if ( h_fi_I >= M_PI ) {
						k = round(fabs(h_fi_I)/(2*M_PI));
                        h_fi_I = h_fi_I -(k*2*M_PI) ;
                    }
                    else {
                         h_fi_I = h_fi_I;
                    }
				    
				    harmonics[j].fiU = h_fi_U * 180 / M_PI; //from rad to degree
				    harmonics[j].fiI = h_fi_I * 180 / M_PI;
			    }
			    
			    if(j>=1) {
	    		   v_h += pow(v_temp, 2);
	    		   i_h += pow(i_temp, 2);
	    		   if(j==harm_num) {
					 sum_v_h = sqrt(v_h) / SQRT2;
					 sum_i_h = sqrt(i_h) / SQRT2;
				   }
                   p_h += (v_temp * i_temp * 
                           cos(rp_dft_o_fi_I[j]-rp_dft_o_fi_U[j])/2);
                   q_h += (v_temp * i_temp * 
                           sin(rp_dft_o_fi_I[j]-rp_dft_o_fi_U[j])/2);
                }
            }
            
			
            amp_fft_u_1 = pwr_cnv_cnt_to_v(amp_fft_u_1, ch1_max_adc_v,
                                           rp_calib_params->fe_ch1_dc_offs,
                                           ch1_user_dc_off, volt_probe_att);
            amp_fft_i_1 = pwr_cnv_cnt_to_a(amp_fft_i_1, ch2_max_adc_v,
                                           rp_calib_params->fe_ch2_dc_offs,
                                           ch2_user_dc_off, curr_probe_fact);
                                           
            meas.u1_fft = amp_fft_u_1 / SQRT2;
            meas.i1_fft = amp_fft_i_1 / SQRT2;
            meas.p1 = amp_fft_u_1 * amp_fft_i_1 * meas.cos_fi_fund/2;
            meas.q1 = amp_fft_u_1 * amp_fft_i_1 * sin(fft_fi_1)/2;
            meas.ph = p_h;
            meas.qh2 = q_h;
            meas.qh1 = meas.u1_fft * (sqrt(i_h) / SQRT2);
            meas.p = p_h + meas.p1;
            meas.q2 = q_h + meas.q1;
            meas.Uef = sqrt(pow(amp_fft_u_1, 2) + v_h) / SQRT2;
            meas.Ief = sqrt(pow(amp_fft_i_1, 2) + i_h) / SQRT2;
            meas.s = meas.Uef * meas.Ief;
            meas.q = sign_q * sqrt(pow(meas.s, 2) - pow(meas.p, 2));
            meas.sum_Uh = sum_v_h;
            meas.sum_Ih = sum_i_h;
            meas.thd_u = sum_v_h * 100 / meas.u1_fft;
            if (amp_fft_i_1 == 0) {
				meas.thd_i = 0;
			}  else {
				meas.thd_i = sum_i_h * 100 / meas.i1_fft;
			}
            meas.pf = (meas.p / meas.s);
            
            rp_pwr_set_harmonics_data(&harmonics[0]); 
            rp_pwr_set_meas_data(meas);
        
        } else {
		  pthread_mutex_lock(&rp_pwr_ctrl_mutex);
          state = rp_pwr_ctrl;
          pthread_mutex_unlock(&rp_pwr_ctrl_mutex);
          if(state == rp_pwr_idle_state) {
			  usleep(2e5);
              continue;
		  } else {
			rp_pwr_harmonics_clear(&harmonics[0]);
			rp_pwr_set_harmonics_data(&harmonics[0]);	
			rp_pwr_meas_clear(&meas);
			rp_pwr_set_meas_data(meas);
			usleep(5e6);
          continue;
		  }
        }
    }
    
    rp_clean_params(dsp_params);
    return 0;
}
/*----------------------------------------------------------------------------*/

float pwr_cnv_cnt_to_v(double cnts, float adc_max_v,
                       int calib_dc_off, float user_dc_off,
                       float diff_probe_att)
{
    float ret_val;

    ret_val =  (cnts * adc_max_v / 
                (float)(1<<(c_pwr_fpga_adc_bits-1)));

    /* and adopt the calculation with user specified DC offset */
    ret_val += user_dc_off;
    ret_val *= diff_probe_att;

    return ret_val;
}

/*----------------------------------------------------------------------------*/

float pwr_cnv_cnt_to_a(double cnts, float adc_max_v,
                       int calib_dc_off, float user_dc_off,
                       float k)
{
    float volts;
    float ampers;

    volts = (cnts * adc_max_v / (float)(1<<(c_pwr_fpga_adc_bits-1)));

    /* and adopt the calculation with user specified DC offset */
    volts += user_dc_off;
    
    /* calculate ampers from measured volts */
    ampers = volts * k;

    return ampers;
}

/*----------------------------------------------------------------------------------*/
int rp_pwr_ch_meas_clear(rp_pwr_ch_meas_res_t *ch_meas)
{
    ch_meas->min = 1e9;
    ch_meas->max = -1e9;
    ch_meas->amp = 0;
    ch_meas->avg = 0;

    return 0;
}

/*----------------------------------------------------------------------------------*/
int rp_pwr_meas_clear(rp_pwr_meas_res_t *pwr_meas)
{
	pwr_meas->p = 0;
	pwr_meas->q = 0;
	pwr_meas->s = 0;
	pwr_meas->p1 = 0;
    pwr_meas->q1 = 0;
    pwr_meas->ph = 0;
    pwr_meas->qh1 = 0;
    pwr_meas->qh2 = 0;
    pwr_meas->q2 = 0;
    pwr_meas->freq = 0;
    pwr_meas->cos_fi_fund = 1;
    pwr_meas->pf = 1;
    pwr_meas->Uef = 0;
    pwr_meas->Ief = 0;
    pwr_meas->u1_fft = 1e-9;
    pwr_meas->i1_fft = 1e-9;
    pwr_meas->sum_Uh = 0;
    pwr_meas->sum_Ih = 0;
    pwr_meas->thd_u = 0;
    pwr_meas->thd_i = 0;
    
    return 0;
}
/*----------------------------------------------------------------------------------*/
int rp_pwr_harmonics_clear(rp_pwr_harm_t *harm_meas)
{
	int i;
	for(i=0; i<40; i++) {
		
       harm_meas[i].U = 0;
       harm_meas[i].I = 0;
       harm_meas[i].fiU = 0;
       harm_meas[i].fiI = 0;
    }
    
    return 0;
}

/*----------------------------------------------------------------------------------*/
inline int rp_pwr_adc_sign(int in_data)
{
    int s_data = in_data;
    if(s_data & (1<<(c_pwr_fpga_adc_bits-1)))
        s_data = -1 * ((s_data ^ ((1<<c_pwr_fpga_adc_bits)-1)) + 1);
    return s_data;
}


/*----------------------------------------------------------------------------------*/
int rp_pwr_meas_min_max(rp_pwr_ch_meas_res_t *ch_meas, int sig_data)
{
    int s_data = rp_pwr_adc_sign(sig_data);

    if(ch_meas->min > s_data)
        ch_meas->min = s_data;
    if(ch_meas->max < s_data)
        ch_meas->max = s_data;

    ch_meas->avg += s_data;

    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_pwr_meas_avg_amp(rp_pwr_ch_meas_res_t *ch_meas, int avg_len)
{
    ch_meas->avg /= avg_len;
    ch_meas->amp = ch_meas->max - ch_meas->min;
    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_pwr_meas_period(rp_pwr_ch_meas_res_t *ch1_meas, rp_pwr_ch_meas_res_t *ch2_meas, 
                       int *in_cha_signal, int *in_chb_signal, int dec_factor)
{
    int wr_ptr_curr, wr_ptr_trig;

    // Checking where acquisition starts
    pwr_fpga_get_wr_ptr(&wr_ptr_curr, &wr_ptr_trig);

    int min, max; // Ignored for measurement panel calculations
    meas_period(ch1_meas, in_cha_signal, wr_ptr_trig, dec_factor, &min, &max);
    meas_period(ch2_meas, in_chb_signal, wr_ptr_trig, dec_factor, &min, &max);

    return 0;
}

/*----------------------------------------------------------------------------------*/
int meas_period(rp_pwr_ch_meas_res_t *meas, int *in_signal, int wr_ptr_trig, int dec_factor,
                int *min, int *max)
{
	float period = 0;
	
    const float c_meas_freq_thr = 100;
    const int c_meas_time_thr = PWR_FPGA_SIG_LEN / 8;
    const float c_min_period = 19.6e-9; // 51 MHz

    float thr1, thr2, cen;
    int state = 0;
    int trig_t[2] = { 0, 0 };
    int trig_cnt = 0;
    int ix, ix_corr;

    float acq_dur=(float)(PWR_FPGA_SIG_LEN)/((float) c_pwr_fpga_smpl_freq) * (float) dec_factor;

    cen = (meas->max + meas->min) / 2;

    thr1 = cen + 0.2 * (meas->min - cen);
    thr2 = cen + 0.2 * (meas->max - cen);

    *max = INT_MIN;
    *min = INT_MAX;

    for(ix = 0; ix < (PWR_FPGA_SIG_LEN); ix++) {
        ix_corr = ix + wr_ptr_trig;

        if (ix_corr >= PWR_FPGA_SIG_LEN) {
            ix_corr %= PWR_FPGA_SIG_LEN;
        }

        int sa = rp_pwr_adc_sign(in_signal[ix_corr]);

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
        period = (trig_t[1] - trig_t[0]) /
            ((float)c_pwr_fpga_smpl_freq * (trig_cnt - 1)) * dec_factor;
    }

    if( ((thr2 - thr1) < c_meas_freq_thr) ||
         (period * 3 >= acq_dur)    ||
         (period < c_min_period) )
    {
        period = 0;
    }

    return period;
}


/*----------------------------------------------------------------------------------*/
inline float rp_pwr_meas_cnv_cnt(float data, float adc_max_v, float k)
{
    return (k * (data * adc_max_v / (float)(1<<(c_pwr_fpga_adc_bits-1))));
}


/*----------------------------------------------------------------------------------*/
int rp_pwr_meas_convert(rp_pwr_ch_meas_res_t *ch_meas, float adc_max_v, 
                        int32_t cal_dc_offs, float factor)
{
    ch_meas->min = rp_pwr_meas_cnv_cnt(ch_meas->min+cal_dc_offs, 
                                       adc_max_v, factor);
    ch_meas->max = rp_pwr_meas_cnv_cnt(ch_meas->max+cal_dc_offs, 
                                       adc_max_v, factor);
    ch_meas->amp = rp_pwr_meas_cnv_cnt(ch_meas->amp, adc_max_v, factor);
    ch_meas->avg = rp_pwr_meas_cnv_cnt(ch_meas->avg+cal_dc_offs, 
                                       adc_max_v, factor);
                                       
    return 0;
}

int rp_pwr_is_fast_enough(int n)
{
	int N = 524;
	int nn;
	int i;
	
	while (n % 2 == 0) {
		n /= 2;
		if (n < N) {
			return 1;
		}
	}
	
	nn = (int)ceil(sqrt(n));
	for(i = 3; i < nn; i += 2) {
		while (n % i == 0) {
			n /= i;
			if(n < N) {
				return 1;
			}
		}
	}
	
	return 0;
}
	
