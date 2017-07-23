/**
 * @brief Red Pitaya Frequency Response Analyzer worker.
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

#include "worker.h"
#include "fpga.h"
#include "dsp.h"
#include "fpga_awg.h"


/** AWG buffer length [samples]*/
#define NN (16*1024)

#define BUFFS 64

/** AWG data buffer */
int32_t ch1_data[NN*BUFFS] ;

double scale[BUFFS];

/** AWG FPGA parameters */
typedef struct {
    int32_t  offsgain;   ///< AWG offset & gain.
    uint32_t wrap;       ///< AWG buffer wrap value.
    uint32_t step;       ///< AWG step interval.
} awg_param_t;

/* Forward declarations */
void synthesize_fra_sig(int ampl,  int kstart, int kstep, int II,
                        int32_t *data, int buffoffs, double *scale,
                        awg_param_t *params);

void write_data_fpga(uint32_t ch, int enable, int trigger,
                    const int32_t *data, int buffoffs, const awg_param_t *awg, int step);




pthread_t *rp_spectr_thread_handler = NULL;
void *rp_spectr_worker_thread(void *args);

/* Signals directly pointing at the FPGA mem space */
int                  *rp_fpga_cha_signal, *rp_fpga_chb_signal;

int cal_ready1 = 1;
int cal_ready2 = 1;
int synth_ready = 0;

double tmpdouble;

const int dacamp = 8000;
const int k1 = 0;
const int II = 64;  //Number of spectral components inspected per acquisition
const int JJ = 8;   //Number of acquisitions (64 is max available by synthetic signal buffer)
                    //II*JJ;  should be < SPECTR_OUT_SIG_LEN  (to fit output buffer)
const int kstp=16;  //Should be set to (16k/2)/(II*JJ) in order to get to fs/2 at fs/16k minimum steps

/* Internal structures */
/* Size = SPECTR_FPGA_SIG_LEN  */
double *rp_cha_in = NULL;
double *rp_chb_in = NULL;

double *rp_cha_resp = NULL;
double *rp_chb_resp = NULL;

double *rp_cha_resp_cal = NULL;
double *rp_chb_resp_cal = NULL;

/* Output 3 x SPECTR_OUT_SIG signals - used internally for calculation */
float               **rp_tmp_signals = NULL;

/* Parameters & signals communicating with 'external world' */
pthread_mutex_t       rp_spectr_ctrl_mutex = PTHREAD_MUTEX_INITIALIZER;
rp_spectr_worker_state_t rp_spectr_ctrl;
rp_app_params_t       rp_spectr_params[PARAMS_NUM];
int                   rp_spectr_params_dirty;
int                   rp_spectr_params_fpga_update;

pthread_mutex_t        rp_spectr_sig_mutex = PTHREAD_MUTEX_INITIALIZER;
float                **rp_spectr_signals = NULL;
int                    rp_spectr_signals_dirty = 0;

int rp_spectr_worker_init(void)
{
    int ret_val;

    rp_spectr_ctrl               = rp_spectr_idle_state;
    rp_spectr_params_dirty       = 1;
    rp_spectr_params_fpga_update = 1;

    rp_cleanup_signals(&rp_spectr_signals);
    if(rp_create_signals(&rp_spectr_signals) < 0)
        return -1;

    rp_cleanup_signals(&rp_tmp_signals);

    if(rp_create_signals(&rp_tmp_signals) < 0) {
        rp_cleanup_signals(&rp_spectr_signals);
        return -1;
    }

    rp_cha_in = (double *)malloc(sizeof(double) * SPECTR_FPGA_SIG_LEN);
    rp_chb_in = (double *)malloc(sizeof(double) * SPECTR_FPGA_SIG_LEN);

    rp_cha_resp = (double *)malloc(sizeof(double) * SPECTR_OUT_SIG_LEN);
    rp_chb_resp = (double *)malloc(sizeof(double) * SPECTR_OUT_SIG_LEN);

    rp_cha_resp_cal = (double *)malloc(sizeof(double) * SPECTR_OUT_SIG_LEN);
    rp_chb_resp_cal = (double *)malloc(sizeof(double) * SPECTR_OUT_SIG_LEN);

    if(!rp_cha_resp ||  !rp_chb_resp || !rp_cha_resp_cal || !rp_chb_resp_cal || !rp_cha_in || !rp_chb_in) {
        rp_spectr_worker_clean();
        return -1;
    }

    if(spectr_fpga_init() < 0) {
        rp_spectr_worker_clean();
        return -1;
    }

    if(rp_spectr_fft_init() < 0) {
        rp_spectr_worker_clean();
        return -1;
    }
    spectr_fpga_get_sig_ptr(&rp_fpga_cha_signal, &rp_fpga_chb_signal);

    rp_spectr_thread_handler = (pthread_t *)malloc(sizeof(pthread_t));
    if(rp_spectr_thread_handler == NULL) {
        rp_cleanup_signals(&rp_spectr_signals);
        rp_cleanup_signals(&rp_tmp_signals);
        return -1;
    }

    ret_val = 
        pthread_create(rp_spectr_thread_handler, NULL, 
                       rp_spectr_worker_thread, NULL);
    if(ret_val != 0) {
        spectr_fpga_exit();

        rp_cleanup_signals(&rp_spectr_signals);
        rp_cleanup_signals(&rp_tmp_signals);
        fprintf(stderr, "pthread_create() failed: %s\n", 
                strerror(errno));
        return -1;
    }

    return 0;
}

int rp_spectr_worker_clean(void)
{
    spectr_fpga_exit();

    rp_cleanup_signals(&rp_spectr_signals);
    rp_cleanup_signals(&rp_tmp_signals);

    rp_spectr_fft_clean();

    if(rp_cha_in) {
        free(rp_cha_in);
        rp_cha_in = NULL;
    }
    if(rp_chb_in) {
        free(rp_chb_in);
        rp_chb_in = NULL;
    }

    if(rp_cha_resp) {
        free(rp_cha_resp);
        rp_cha_resp = NULL;
    }
    if(rp_chb_resp) {
        free(rp_chb_resp);
        rp_chb_resp = NULL;
    }
    if(rp_cha_resp_cal) {
        free(rp_cha_resp_cal);
        rp_cha_resp_cal = NULL;
    }
    if(rp_chb_resp_cal) {
        free(rp_chb_resp_cal);
        rp_chb_resp_cal = NULL;
    }

    return 0;
}

int rp_spectr_worker_exit(void)
{
    int ret_val = 0;

    rp_spectr_worker_change_state(rp_spectr_quit_state);
    if(rp_spectr_thread_handler) {
        ret_val = pthread_join(*rp_spectr_thread_handler, NULL);
        free(rp_spectr_thread_handler);
        rp_spectr_thread_handler = NULL;
    }
    if(ret_val != 0) {
        fprintf(stderr, "pthread_join() failed: %s\n", 
                strerror(errno));
    }
    rp_spectr_worker_clean();
    return 0;
}

int rp_spectr_worker_change_state(rp_spectr_worker_state_t new_state)
{
    if(new_state >= rp_spectr_nonexisting_state)
        return -1;
    pthread_mutex_lock(&rp_spectr_ctrl_mutex);
    rp_spectr_ctrl = new_state;
    pthread_mutex_unlock(&rp_spectr_ctrl_mutex);
    return 0;
}

int rp_spectr_worker_update_params(rp_app_params_t *params, int fpga_update)
{
    pthread_mutex_lock(&rp_spectr_ctrl_mutex);
    memcpy(&rp_spectr_params, params, sizeof(rp_app_params_t)*PARAMS_NUM);
    rp_spectr_params_dirty       = 1;
    rp_spectr_params_fpga_update = fpga_update;
    pthread_mutex_unlock(&rp_spectr_ctrl_mutex);
    return 0;
}

int rp_spectr_clean_signals(void)
{
    pthread_mutex_lock(&rp_spectr_sig_mutex);
    rp_spectr_signals_dirty = 0;
    pthread_mutex_unlock(&rp_spectr_sig_mutex);
    return 0;
}


int rp_spectr_get_signals(float ***signals)
{
    float **s = *signals;
    pthread_mutex_lock(&rp_spectr_sig_mutex);
    if(rp_spectr_signals_dirty == 0) {
        pthread_mutex_unlock(&rp_spectr_sig_mutex);
        return -1;
    }

    memcpy(&s[0][0], &rp_spectr_signals[0][0], sizeof(float)*SPECTR_OUT_SIG_LEN);
    memcpy(&s[1][0], &rp_spectr_signals[1][0], sizeof(float)*SPECTR_OUT_SIG_LEN);
    memcpy(&s[2][0], &rp_spectr_signals[2][0], sizeof(float)*SPECTR_OUT_SIG_LEN);

    rp_spectr_signals_dirty = 0;

    pthread_mutex_unlock(&rp_spectr_sig_mutex);

    return 0;
}

int rp_spectr_set_signals(float **source)
{
    pthread_mutex_lock(&rp_spectr_sig_mutex);

    memcpy(&rp_spectr_signals[0][0], &source[0][0], sizeof(float)*SPECTR_OUT_SIG_LEN);
    memcpy(&rp_spectr_signals[1][0], &source[1][0], sizeof(float)*SPECTR_OUT_SIG_LEN);
    memcpy(&rp_spectr_signals[2][0], &source[2][0], sizeof(float)*SPECTR_OUT_SIG_LEN);

    rp_spectr_signals_dirty = 1;

    pthread_mutex_unlock(&rp_spectr_sig_mutex);

    return 0;
}

void *rp_spectr_worker_thread(void *args)
{
    rp_spectr_worker_state_t old_state, state;
    rp_app_params_t          curr_params[PARAMS_NUM];
    int                      fpga_update = 1;
    int                      params_dirty = 1;

    // TODO: Name these
    int jj_state = 0;
    int iix, iix2;
    int cal_butt1_old, cal_butt2_old;
    int start_state = 1;
    awg_param_t awg_par;

    pthread_mutex_lock(&rp_spectr_ctrl_mutex);
    old_state = state = rp_spectr_ctrl;
    pthread_mutex_unlock(&rp_spectr_ctrl_mutex);

    while(1) {
        /* update states - we save also old state to see if we need to reset
         * FPGA 
         */
        old_state = state;
        pthread_mutex_lock(&rp_spectr_ctrl_mutex);
        state = rp_spectr_ctrl;
        if(rp_spectr_params_dirty) {
            memcpy(&curr_params, &rp_spectr_params, 
                   sizeof(rp_app_params_t)*PARAMS_NUM);
            fpga_update = rp_spectr_params_fpga_update;
            rp_spectr_params_dirty = 0;
        }
        pthread_mutex_unlock(&rp_spectr_ctrl_mutex);

        /* request to stop worker thread, we will shut down */
        if(state == rp_spectr_quit_state) {
            return 0;
        }

        if(fpga_update) {
            spectr_fpga_reset();
            //cal_ready1 = 0; // Resets calibration
            //cal_ready2 = 0; // Resets calibration
            jj_state = 0;

            if(spectr_fpga_update_params(0, 0, 0, 0, 0, 
                    (int)curr_params[FREQ_RANGE_PARAM].value, 0) < 0) {
                rp_spectr_worker_change_state(rp_spectr_auto_state);
            }

            fpga_update = 0;
        }

        if(state == rp_spectr_idle_state) {
            usleep(10000);
            continue;
        }

        /* Prepare AWG data buffer  */
        // Buffer prepared only once in order to save CPU resources
        if (synth_ready == 0) {
            // Before preparing buffer visualize some constant signal
            rp_resp_init_sigs(&rp_tmp_signals[0], (float **)&rp_tmp_signals[1], (float **)&rp_tmp_signals[2]);
            rp_spectr_set_signals(rp_tmp_signals);

            for (iix2 = 0; iix2 < JJ; iix2++) {
                synthesize_fra_sig(dacamp,  iix2*II*kstp, kstp, II, ch1_data, iix2*NN, &tmpdouble, &awg_par);
                scale[iix2] = tmpdouble;
            }
            synth_ready = 1;
        }

        if (start_state == 0) {
            if (round(curr_params[EN_CAL_1].value) != cal_butt1_old) {
                cal_ready1 = 0;
            }
            if (round(curr_params[EN_CAL_2].value) != cal_butt2_old) {
                cal_ready2 = 0;
            }
        } else {
            start_state = 0;
            // The calibration consists in coping the current response data to reference cal. vectors
            for (iix = 0; iix < SPECTR_OUT_SIG_LEN; iix++) {
                rp_cha_resp_cal[iix] = 56e6;
                rp_chb_resp_cal[iix] = 56e6;
            }
        }

        cal_butt1_old = round(curr_params[EN_CAL_1].value);
        cal_butt2_old = round(curr_params[EN_CAL_2].value);

        // TODO: Implement signal generator part as a separate module

        /* Write the data to the FPGA and set FPGA AWG state machine for current pattern*/
        write_data_fpga(0, 1, 0,
                ch1_data, jj_state*NN,
                &awg_par, round(65536 /
                        ((float)spectr_fpga_cnv_freq_range_to_dec(curr_params[FREQ_RANGE_PARAM].value))));

        write_data_fpga(1, 1, 0,
                ch1_data, jj_state*NN,
                &awg_par, round(65536 /
                        ((float)spectr_fpga_cnv_freq_range_to_dec(curr_params[FREQ_RANGE_PARAM].value))));

        /* Start the writing machine: DATA ACQUISITION */
        spectr_fpga_arm_trigger();
        
        usleep(10);

        spectr_fpga_set_trigger(1);

        /* start working */
        pthread_mutex_lock(&rp_spectr_ctrl_mutex);
        old_state = state = rp_spectr_ctrl;
        pthread_mutex_unlock(&rp_spectr_ctrl_mutex);
        if((state == rp_spectr_idle_state) || (state == rp_spectr_abort_state)) {
            continue;
        } else if(state == rp_spectr_quit_state) {
            break;
        }

        /* polling until data is ready */
        while(1) {
            pthread_mutex_lock(&rp_spectr_ctrl_mutex);
            state = rp_spectr_ctrl;
            params_dirty = rp_spectr_params_dirty;
            pthread_mutex_unlock(&rp_spectr_ctrl_mutex);
            /* change in state, abort polling */
            if((state != old_state) || params_dirty) {
                break;
            }
                
            if(spectr_fpga_triggered()) {
                break;
            }
        }

        if((state != old_state) || params_dirty) {
            params_dirty = 0;
            continue;
        }

        /* retrieve data and process it*/
        spectr_fpga_get_signal(&rp_cha_in, &rp_chb_in);

        /* Calculate response at frequency components for each excitation pattern*/
        rp_resp_calc(&rp_cha_in[0], &rp_chb_in[0], jj_state*II, scale[jj_state], kstp, II, (double **)&rp_cha_resp, (double **)&rp_chb_resp);

        // Continue acquiring and processing until all the pattern sequence completed
        if (jj_state < JJ) {
            jj_state++;
        } else {
            // Response characterization completed
            jj_state = 0;

            /* Perform calibration at startup or parameter update.
             * TODO: add a GUI Calibration button for individual channels
             */
            if (cal_ready1 == 0)
            {																		// Provide calibration vector initialization since calibration
                cal_ready1 = 1;													    // executed on demand
                // The calibration consists in coping the current response data to reference cal. vectors
                for (iix = 0; iix < SPECTR_OUT_SIG_LEN; iix++) {
                    rp_cha_resp_cal[iix] = rp_cha_resp[iix];
                }
            }
            /* Perform calibration at startup or parameter update
             * TODO: add a GUI Calibration button for individual channels
             */
            if (cal_ready2 == 0)
            {																		// Provide calibration vector initialization since calibration
                cal_ready2 = 1;													    // executed on demand
                // The calibration consists in coping the current response data to reverence cal. vectors
                for (iix = 0; iix < SPECTR_OUT_SIG_LEN; iix++) {
                    rp_chb_resp_cal[iix] = rp_chb_resp[iix];
                }
            }

            // Calculate frequency vector for the frequencies where response measured
            rp_resp_prepare_freq_vector(&rp_tmp_signals[0],
                    c_spectr_fpga_smpl_freq,
                    curr_params[FREQ_RANGE_PARAM].value, II, JJ, k1, kstp);


            // Calculate frequency response in dB and implement calibration
            rp_resp_cnv_to_dB(&rp_cha_resp[0], &rp_chb_resp[0],
                    &rp_cha_resp_cal[0], &rp_chb_resp_cal[0],
                    (float **)&rp_tmp_signals[1],
                    (float **)&rp_tmp_signals[2], II*JJ);

            rp_spectr_set_signals(rp_tmp_signals);

            usleep(10000);

        }
    }

    return 0;
}


void synthesize_fra_sig(int ampl,  int kstart, int kstep, int II,
                       int32_t *data, int buffoffs, double *scale,
                       awg_param_t *awg)
{
    uint32_t ix, jx;
    double ddata[NN];
    double maxabs = 0;

    // Various locally used constants - HW specific parameters
    const int dcoffs = -155;

    awg->offsgain = (dcoffs << 16) + 0x1fff;
    awg->step = round(65536 * 1);
    awg->wrap = round(65536 * NN-1);

    // Fill data[] with appropriate buffer samples
    for (jx = 0; jx < II; jx++) {
        for (ix = 0; ix < NN; ix++) {
            if (jx == 0) {
                ddata[ix] = cos(2 * M_PI * (double)ix / (double)NN *
                        (double)(kstart + kstep * jx));
            } else {
                ddata[ix] = ddata[ix] + cos(2 * M_PI * (double)ix / (double)NN *
                        (double)(kstart + kstep * jx));
            }
        }
    }

	// Checking maximum
	for (ix = 0; ix < NN; ix++) {
		if (abs(ddata[ix]) > maxabs)
			maxabs = abs(ddata[ix]);
	}

	*scale = maxabs;

	// Normalization
	for (ix = 0; ix < NN; ix++) {
		if (ddata[ix] < 0)
			 //TODO: Remove, not necessary in C/C++.
			data[ix+buffoffs] = round(ddata[ix] * ((double)ampl) / maxabs) + (1 << 14);
		else
			data[ix+buffoffs] = round(ddata[ix] * ((double)ampl) / maxabs);
	}

}


/**
 * Write synthesized data[] to FPGA buffer.
 *
 * @param ch    Channel number [0, 1].
 * @param data  AWG data to write to FPGA.
 * @param awg   AWG paramters to write to FPGA.
 */
void write_data_fpga(uint32_t ch, int enable, int trigger, const int32_t *data, int buffoffs,
                     const awg_param_t *awg, int step)
{
   int ixx;

   if (ch == 0) {

       set_gen(0, 0x000041);
       set_gen(1, awg->offsgain);
       set_gen(2, awg->wrap);
       set_gen(4, step);

       for(ixx = 0; ixx < NN; ixx++)
           set_gen(ixx + 0x4000, data[ixx + buffoffs]);

       set_gen(0, 0x110011);

   } else {

       set_gen(0, 0x410000);
       set_gen(9, awg->offsgain);
       set_gen(10, awg->wrap);
       set_gen(12, step);

       for(ixx = 0; ixx < NN; ixx++)
           set_gen(ixx + 0x8000, data[ixx + buffoffs]);

       set_gen(0, 0x110011);
   }
}
