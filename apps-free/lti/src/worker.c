/**
 * $Id: 
 *
 * @brief Red Pitaya LTI workbench worker.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 * @Author Dashpi <dashpi46@gmail.com>
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
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "worker.h"
#include "fpga_lti.h"
#include "generate_basic.h"
#include "dsp.h"



pthread_t *rp_lti_thread_handler = NULL;
void *rp_lti_worker_thread(void *args);

/* Signals directly pointing at the FPGA mem space */
int                  *rp_fpga_cha_signal, *rp_fpga_chb_signal;
int                  *rp_fpga_cha_gen, *rp_fpga_chb_gen;

/* Internal structures */
/* Size = LTI_FPGA_SIG_LEN  */
double *rp_cha_in = NULL;
double *rp_chb_in = NULL;

double *rp_dsp_state_a = NULL;
double *rp_dsp_state_b = NULL;

double *rp_dsp_par_a = NULL;
double *rp_dsp_par_b = NULL;


/* DSP structures */
/* size = c_dsp_sig_len */
double *rp_cha_fft = NULL;
double *rp_chb_fft = NULL;

/* Output 3 x LTI_OUT_SIG signals - used internally for calculation */
float               **rp_tmp_signals = NULL;

/* Parameters & signals communicating with 'external world' */
pthread_mutex_t       rp_lti_ctrl_mutex = PTHREAD_MUTEX_INITIALIZER;
rp_lti_worker_state_t rp_lti_ctrl;
rp_app_params_t       rp_lti_params[PARAMS_NUM];
int                   rp_lti_params_dirty;
int                   rp_lti_params_fpga_update;

pthread_mutex_t        rp_lti_sig_mutex = PTHREAD_MUTEX_INITIALIZER;
float                **rp_lti_signals = NULL;

int                    rp_lti_signals_dirty = 0;

int rp_lti_worker_init(void)
{
    int ret_val;

    rp_lti_ctrl               = rp_lti_idle_state;
    rp_lti_params_dirty       = 1;
    rp_lti_params_fpga_update = 1;

    

    rp_cleanup_signals(&rp_lti_signals);
    if(rp_create_signals(&rp_lti_signals) < 0)
        return -1;

    rp_cleanup_signals(&rp_tmp_signals);

    if(rp_create_signals(&rp_tmp_signals) < 0) {
        rp_cleanup_signals(&rp_lti_signals);
        return -1;
    }

    //Input acquisition data
    rp_cha_in = (double *)malloc(sizeof(double) * LTI_FPGA_SIG_LEN);
    rp_chb_in = (double *)malloc(sizeof(double) * LTI_FPGA_SIG_LEN);
    
    //DSP states
    rp_dsp_state_a = (double *)malloc(sizeof(double) * LTI_DSP_STATES);
    rp_dsp_state_b = (double *)malloc(sizeof(double) * LTI_DSP_STATES);

    //DSP parameters
    rp_dsp_par_a = (double *)malloc(sizeof(double) * LTI_DSP_PARAMS);
    rp_dsp_par_b = (double *)malloc(sizeof(double) * LTI_DSP_PARAMS);    
    
    
    //FFT data (currently not used)
    rp_cha_fft = (double *)malloc(sizeof(double) * c_dsp_sig_len);
    rp_chb_fft = (double *)malloc(sizeof(double) * c_dsp_sig_len);
    
    
    
    if(!rp_cha_in || !rp_chb_in || !rp_dsp_state_a || !rp_dsp_state_b || !rp_dsp_par_a || !rp_dsp_par_b || !rp_cha_fft || !rp_chb_fft) {
        rp_lti_worker_clean();
        return -1;
    }

    

    if(lti_fpga_init() < 0) {
        rp_lti_worker_clean();
        return -1;
    }


    if(rp_lti_fft_init() < 0) {
        rp_lti_worker_clean();
        return -1;
    }


    

    lti_fpga_get_sig_ptr(&rp_fpga_cha_signal, &rp_fpga_chb_signal);

    rp_lti_thread_handler = (pthread_t *)malloc(sizeof(pthread_t));
    if(rp_lti_thread_handler == NULL) {
        rp_cleanup_signals(&rp_lti_signals);
        rp_cleanup_signals(&rp_tmp_signals);
        return -1;
    }

    ret_val = 
        pthread_create(rp_lti_thread_handler, NULL, 
                       rp_lti_worker_thread, NULL);
    if(ret_val != 0) {
        lti_fpga_exit();

        rp_cleanup_signals(&rp_lti_signals);
        rp_cleanup_signals(&rp_tmp_signals);
        fprintf(stderr, "pthread_create() failed: %s\n", 
                strerror(errno));
        return -1;
    }

    return 0;
}

int rp_lti_worker_clean(void)
{
    lti_fpga_exit();
    rp_cleanup_signals(&rp_lti_signals);
    rp_cleanup_signals(&rp_tmp_signals);
    rp_lti_fft_clean();


    
    if(rp_cha_in) {
        free(rp_cha_in);
        rp_cha_in = NULL;
    }
    if(rp_chb_in) {
        free(rp_chb_in);
        rp_chb_in = NULL;
    }
    if(rp_dsp_state_a) {
        free(rp_dsp_state_a);
        rp_dsp_state_a = NULL;
    }
    if(rp_dsp_state_b) {
        free(rp_dsp_state_b);
        rp_dsp_state_b = NULL;
    }
    if(rp_dsp_par_a) {
        free(rp_dsp_par_a);
        rp_dsp_par_a = NULL;
    }
    if(rp_dsp_par_b) {
        free(rp_dsp_par_b);
        rp_dsp_par_b = NULL;
    }    
    if(rp_cha_fft) {
        free(rp_cha_fft);
        rp_cha_fft = NULL;
    }
    if(rp_chb_fft) {
        free(rp_chb_fft);
        rp_chb_fft = NULL;
    }

    return 0;
}

int rp_lti_worker_exit(void)
{
    int ret_val = 0;

    rp_lti_worker_change_state(rp_lti_quit_state);
    if(rp_lti_thread_handler) {
        ret_val = pthread_join(*rp_lti_thread_handler, NULL);
        free(rp_lti_thread_handler);
        rp_lti_thread_handler = NULL;
    }
    if(ret_val != 0) {
        fprintf(stderr, "pthread_join() failed: %s\n", 
                strerror(errno));
    }
    rp_lti_worker_clean();
    
    return 0;
}

int rp_lti_worker_change_state(rp_lti_worker_state_t new_state)
{
    if(new_state >= rp_lti_nonexisting_state)
        return -1;
    pthread_mutex_lock(&rp_lti_ctrl_mutex);
    rp_lti_ctrl = new_state;
    pthread_mutex_unlock(&rp_lti_ctrl_mutex);
    return 0;
}

int rp_lti_worker_update_params(rp_app_params_t *params, int fpga_update)
{
    pthread_mutex_lock(&rp_lti_ctrl_mutex);
    memcpy(&rp_lti_params, params, sizeof(rp_app_params_t)*PARAMS_NUM);
    rp_lti_params_dirty       = 1;    
    rp_lti_params_fpga_update = fpga_update;
    pthread_mutex_unlock(&rp_lti_ctrl_mutex);
    return 0;
}

int rp_lti_clean_signals(void)
{
    pthread_mutex_lock(&rp_lti_sig_mutex);
    rp_lti_signals_dirty = 0;
    pthread_mutex_unlock(&rp_lti_sig_mutex);
    return 0;
}



int rp_lti_get_signals(float ***signals)
{
    float **s = *signals;
    pthread_mutex_lock(&rp_lti_sig_mutex);
    if(rp_lti_signals_dirty == 0) {
        pthread_mutex_unlock(&rp_lti_sig_mutex);
        return -1;
    }

    memcpy(&s[0][0], &rp_lti_signals[0][0], sizeof(float)*LTI_OUT_SIG_LEN);
    memcpy(&s[1][0], &rp_lti_signals[1][0], sizeof(float)*LTI_OUT_SIG_LEN);
    memcpy(&s[2][0], &rp_lti_signals[2][0], sizeof(float)*LTI_OUT_SIG_LEN);

    rp_lti_signals_dirty = 0;

    
    

    pthread_mutex_unlock(&rp_lti_sig_mutex);
    return 0;
}

int rp_lti_set_signals(float **source)
{
    pthread_mutex_lock(&rp_lti_sig_mutex);
    memcpy(&rp_lti_signals[0][0], &source[0][0], sizeof(float)*LTI_OUT_SIG_LEN);
    memcpy(&rp_lti_signals[1][0], &source[1][0], sizeof(float)*LTI_OUT_SIG_LEN);
    memcpy(&rp_lti_signals[2][0], &source[2][0], sizeof(float)*LTI_OUT_SIG_LEN);

    rp_lti_signals_dirty = 1;

    
 

    pthread_mutex_unlock(&rp_lti_sig_mutex);

    return 0;
}

void *rp_lti_worker_thread(void *args)
{
    rp_lti_worker_state_t  state;
    rp_app_params_t          curr_params[PARAMS_NUM];
    int                      fpga_update = 1;

    
    int                      armed=0;
    int                      gen_start_ptr,rp_dsp_loc_ptr;
    int                      indx;
    
    float                    curr_dec;
    


    pthread_mutex_lock(&rp_lti_ctrl_mutex);
    state = rp_lti_ctrl;
    pthread_mutex_unlock(&rp_lti_ctrl_mutex);

    while(1) {
        /* update states - we save also old state to see if we need to reset
         * FPGA 
         */
        
        pthread_mutex_lock(&rp_lti_ctrl_mutex);
        state = rp_lti_ctrl;
        if(rp_lti_params_dirty) {
	  
            memcpy(&curr_params, &rp_lti_params, 
                   sizeof(rp_app_params_t)*PARAMS_NUM);
            fpga_update = rp_lti_params_fpga_update;
            rp_lti_params_dirty = 0;
	    
	    armed=0; // Repeat ARM sequence when user changes parameters
	    
        }
        pthread_mutex_unlock(&rp_lti_ctrl_mutex);

        /* request to stop worker thread, we will shut down */
        if(state == rp_lti_quit_state) {
            return 0;
        }

        if(fpga_update) {

	    
	  
            lti_fpga_reset();
            if(lti_fpga_update_params(0, 0, 0, 0, 0, 
                               (int)curr_params[FREQ_RANGE_PARAM].value,
                               curr_params[EN_AVG_AT_DEC].value) < 0) {
                rp_lti_worker_change_state(rp_lti_auto_state);
            }

            fpga_update = 0;
            
            
        }

        if(state == rp_lti_idle_state) {
	  
            usleep(100);
            continue;
        }

        if (curr_params[FREQ_RANGE_PARAM].value>=3)  // Other acquisition decimation ranges to be supported by FPGA LTI system implementation (currenty NOT AVAILABLE)
	{
	     
	 
 	if (armed==0)  // When acquisition "NOT ARMED", start circular buffer first
	{
	
	  
	/* Start the writting machine*/
        lti_fpga_arm_trigger();	      
        armed=1;  // Arm just the first time
        
        // Check input buffer sampling rate
        curr_dec=lti_fpga_cnv_freq_range_to_dec(curr_params[FREQ_RANGE_PARAM].value);
        
        
        // Start signal generator: the output samples will be continuosly updated by the DSP algorithm
	
        dir_gen_set(0, 0, 0xc0);	     // Disable signal on ch1
	dir_gen_set(0, 1, 8191);            // set Scale parameter
	dir_gen_set(0, 2, 0);               // set DC Offset parameter	 (TODO: add calibration term here (check calgenscope))
	dir_gen_set(0, 3, round(65536 * 16*1024 - 1));               //set Wrap parameter	(Full 16k buffer wrapping)
	dir_gen_set(0, 4, 0);               // set TimeOffs parameter	
	dir_gen_set(0, 5, round(65536.0/curr_dec)); // set Step parameter in order to synchronize INPUT and OUTPUT 16k buffers 	
	
	// Remember acquisition buffer pointer when signal generator started (to resolve phase relation between input and output)	
	lti_fpga_get_wr_ptr(&gen_start_ptr, NULL);
	
	// From this point on signal generator buffer and acquisition buffer locked (same frequency)
	dir_gen_set(0, 0, 0x11);	     // Start signal generator on ch1	     
        
	// Initialize the DSP pointer with the location corresponding to generator start
	rp_dsp_loc_ptr=gen_start_ptr;
	
	// Fetch pointers to generator buffer
	lti_fpga_get_gen_ptr(&rp_fpga_cha_gen, &rp_fpga_chb_gen);
	
	
	// Clear states
	
	for (indx=0; indx<128; indx++) {
	
	rp_dsp_state_a[indx]=0;
	rp_dsp_state_b[indx]=0;	
	
	}
	
	
	
	  // Read LTI coeffs. (6th order IIR, DSP function (lti_fpga_online_dsp) supports up tu 64th order)
	  //  			Possible upgrade: LTI parameters passeed through file upload
	
	rp_dsp_par_a[0]=curr_params[LTI_B0].value; 
	rp_dsp_par_a[1]=curr_params[LTI_B1].value;
	rp_dsp_par_a[2]=curr_params[LTI_B2].value;
	rp_dsp_par_a[3]=curr_params[LTI_B3].value;
	rp_dsp_par_a[4]=curr_params[LTI_B4].value;
	rp_dsp_par_a[5]=curr_params[LTI_B5].value;

	rp_dsp_par_a[64]=6;         // order (a0 is implicitly = 1)
	rp_dsp_par_a[65]=curr_params[LTI_A1].value; 
	rp_dsp_par_a[66]=curr_params[LTI_A2].value; 
	rp_dsp_par_a[67]=curr_params[LTI_A3].value; 
	rp_dsp_par_a[68]=curr_params[LTI_A4].value; 
	rp_dsp_par_a[69]=curr_params[LTI_A5].value; 	  
	
	
	rp_lti_prepare_freq_vector(&rp_tmp_signals[0], 
                                      c_lti_fpga_smpl_freq,
                                      curr_params[FREQ_RANGE_PARAM].value);

	// Calculate LTI response
        rp_lti_calc_fresp((float **)&rp_tmp_signals[1], 
                             (float **)&rp_tmp_signals[2],
			      &rp_dsp_par_a, &rp_dsp_par_b,
                             curr_params[FREQ_RANGE_PARAM].value);	
	
	}
	
	// CALL DSP ALGORITHM:
	//  - retrieve input ADC samples buffered after last algorithm call (rp_dsp_loc_ptr)
	//  - apply DSP according to previous DSP state and parameters (rp_dsp_state_a, rp_dsp_par_a)
	//  - applies results as output DAC samples (rp_fpga_cha_gen) 
	
	// returns ADC input samples (rp_cha_in) 
	     
	   
	rp_dsp_loc_ptr=lti_fpga_online_dsp(&rp_cha_in, &rp_chb_in, round(3000.0*1024.0/curr_dec), 
					    &rp_dsp_state_a, &rp_dsp_state_b, 
				            &rp_dsp_par_a, &rp_dsp_par_b, 
				           rp_dsp_loc_ptr, 
				           &rp_fpga_cha_gen, &rp_fpga_chb_gen);
		
	
	// Experimentally a safe generator delay is 3000 locs at 122 kHz
	
	
	
	} 
	
	else
	{    // DO nothing
	  
     
	}


        /* Copy the signals to the output part 
         */
	
	
         rp_lti_set_signals(rp_tmp_signals);
	 usleep(100);
	
	

        
    }

    return 0;
}

