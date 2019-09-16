/**
 * $Id$
 *
 * @brief Red Pitaya Oscilloscope worker module.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *         Ales Bardorfer <ales.bardorfer@redpitaya.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 * This module is multi-threaded and is using synchronization with mutexes
 * from POSIX thread library. For these topics please visit:
 * http://en.wikipedia.org/wiki/POSIX_Threads
 */

#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

#include "worker.h"
#include "fpga_osc.h"

/**
 * GENERAL DESCRIPTION:
 *
 * The worker module is linking point between main Oscilloscope module and FPGA
 * module. 
 *
 * It is running continuously in separate thread and controls FPGA OSC module. If
 * the oscilloscope is running in auto or manual mode it collects results, 
 * process them and put them to main module, where they are available for the 
 * client. In the opposite direction main module is setting new parameters which 
 * are used by worker by each new iteration of the worker thread loop. If 
 * parameters change during measurement, this is aborted and worker updates new 
 * parameters and starts new measurement.
 * Third party of the data is control of the worker thread loop - main or worker
 * module can change the state of the loop. Possible states are defined in
 * rp_osc_worker_state_e enumeration.
 *
 * Signal and parameters structures between worker and main module are protected
 * with mutexes - which means only one can access them at the same time. With 
 * this the consistency of output (signal) or input (parameters) data is assured.
 * 
 * Besides interfacing main and FPGA module big task of worker module is also 
 * processing the result, this includes:
 *  - preparing time vector, based on current settings
 *  - decimating the signal length from FPGA buffer length (OSC_FPGA_SIG_LEN) to
 *    output signal length (SIGNAL_LENGTH defined in main_osc.h). 
 *  - converting signal from ADC samples to voltage in [V].
 */

/** POSIX thread handler */
pthread_t rp_osc_thread_handler;
/** Worker thread function declaration */
void *rp_osc_worker_thread(void *args);

/** pthread mutex used to protect parameters and related variables */
pthread_mutex_t       rp_osc_ctrl_mutex = PTHREAD_MUTEX_INITIALIZER;
/** Worker thread state holder */
rp_osc_worker_state_t rp_osc_ctrl;
/** Worker module copy of parameters (copied from main parameters) */
rp_osc_params_t       rp_osc_params[PARAMS_NUM];
/** Signalizer if new parameters were copied */
int                   rp_osc_params_dirty;
/** Signalizer if worker thread loop needs to update FPGA registers */
int                   rp_osc_params_fpga_update;

/** pthread mutex used to protect signal structure and related variables */
pthread_mutex_t       rp_osc_sig_mutex = PTHREAD_MUTEX_INITIALIZER;
/** Output signals holder */
float               **rp_osc_signals;
/** Output signals signalizer - it indicates to main whether new signals are 
 *  available or not */
int                   rp_osc_signals_dirty = 0;
/** Output signal index indicator - used in long acquisitions, it indicates 
 *  to main module if the returning signal is full or partial
 */
int                   rp_osc_sig_last_idx = 0;
/** Working signal holder - this is used in working thread loop for the 
 *  calculation of the final result */
float               **rp_tmp_signals; /* used for calculation from worker */

/** Pointers to the FPGA input signal buffers for Channel A and B */
int                  *rp_fpga_cha_signal, *rp_fpga_chb_signal;

/** @brief Initializes worker module
 *
 * This function starts new worker thread, initializes internal structures 
 * (signals, state, ...) and initializes FPGA module.
 *
 * @retval -1 Failure
 * @retval 0 Success
*/
int rp_osc_worker_init(void)
{
    int ret_val;

    rp_osc_ctrl               = rp_osc_idle_state;
    rp_osc_params_dirty       = 0;
    rp_osc_params_fpga_update = 0;

    /* Create output signal structure */
    rp_cleanup_signals(&rp_osc_signals);
    if(rp_create_signals(&rp_osc_signals) < 0)
        return -1;

    /* Create working signal structure */
    rp_cleanup_signals(&rp_tmp_signals);
    if(rp_create_signals(&rp_tmp_signals) < 0) {
        rp_cleanup_signals(&rp_osc_signals);
        return -1;
    }

    /* FPGA module initialization */
    if(osc_fpga_init() < 0) {
        rp_cleanup_signals(&rp_osc_signals);
        rp_cleanup_signals(&rp_tmp_signals);
        return -1;
    }

    /* Initializing the pointers for the FPGA input signal buffers */
    osc_fpga_get_sig_ptr(&rp_fpga_cha_signal, &rp_fpga_chb_signal);

    /* Creating worker thread */
    ret_val = 
        pthread_create(&rp_osc_thread_handler, NULL, rp_osc_worker_thread, NULL);
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

/** @brief Cleans up worker module.
 *
 * This function stops the working thread (sending quit state to it) and waits
 * until it is shutdown. After that it cleans up FPGA module and internal 
 * structures.
 * After this function is called any access to worker module are forbidden.
 *
 * @retval 0 Always returns 0
*/
int rp_osc_worker_exit(void)
{
    int ret_val;

    rp_osc_worker_change_state(rp_osc_quit_state);
    ret_val = pthread_join(rp_osc_thread_handler, NULL);
    if(ret_val != 0) {
        fprintf(stderr, "pthread_join() failed: %s\n", 
                strerror(errno));
    }
    osc_fpga_exit();

    rp_cleanup_signals(&rp_osc_signals);
    rp_cleanup_signals(&rp_tmp_signals);

    return 0;
}

/** @brief Changes the worker state
 *
 * This function is used to change the worker thread state. When new state is
 * requested, it is not effective immediately. It is changed only when thread
 * loop checks and accepts it (for example - if new state is requested during
 * signal processing - the worker thread loop will detect change in the state
 * only after it finishes the processing.
 *
 * @param [in] new_state New requested state
 *
 * @retval -1 Failure
 * @retval 0 Success
 **/
int rp_osc_worker_change_state(rp_osc_worker_state_t new_state)
{
    if(new_state >= rp_osc_nonexisting_state)
        return -1;
    pthread_mutex_lock(&rp_osc_ctrl_mutex);
    rp_osc_ctrl = new_state;
    pthread_mutex_unlock(&rp_osc_ctrl_mutex);
    return 0;
}

/** @brief Updates the worker copy of the parameters 
 *
 * This function is used to update the parameters in worker. These parameters are
 * not effective immediately. They will be used by worker thread loop after 
 * current operation is finished.
 * 
 * @param [in] params New parameters
 * @param [in] fpga_update Flag is FPGA needs to be updated.
 *
 * @retval 0 Always returns 0
 **/
int rp_osc_worker_update_params(rp_osc_params_t *params, int fpga_update)
{
    pthread_mutex_lock(&rp_osc_ctrl_mutex);
    memcpy(&rp_osc_params, params, sizeof(rp_osc_params_t)*PARAMS_NUM);
    rp_osc_params_dirty       = 1;
    rp_osc_params_fpga_update = fpga_update;
    pthread_mutex_unlock(&rp_osc_ctrl_mutex);
    return 0;
}

/** @brief Marks output signals as clean.
 *
 * This function marks output signals as clean (already transmitted). This 
 * function can be used if client demands exactly one measurement - for example
 * single mode. In this case main module can clean signal from previous 
 * measurements and can be sure that new signal will be with desired parameters.
 *
 * @retval 0 Always returns 0
*/
int rp_osc_clean_signals(void)
{
    pthread_mutex_lock(&rp_osc_sig_mutex);
    rp_osc_signals_dirty = 0;
    pthread_mutex_unlock(&rp_osc_sig_mutex);
    return 0;
}

/** @brief Returns last output signals.
 *
 * This function returns last valid output signal. With return value it indicates
 * whether signals were already transmitted (are so called 'clean') or were not 
 * yet transmitted to the main module (so called 'dirty' signals).
 * 
 * @param [out] signals Copy of last valid signals.
 * @param [out] sig_idx Last valid index inside of signal (used in long 
 *                      acquisition operation)
 *
 * @retval -1 Clean signal (already read measurement) was returned
 * @retval 0 New signal (new measurement) was returned
*/
int rp_osc_get_signals(float ***signals, int *sig_idx)
{
    float **s = *signals;
    pthread_mutex_lock(&rp_osc_sig_mutex);
    if(rp_osc_signals_dirty == 0) {
        *sig_idx = rp_osc_sig_last_idx;
        pthread_mutex_unlock(&rp_osc_sig_mutex);
        return -1;
    }

    memcpy(&s[0][0], &rp_osc_signals[0][0], sizeof(float)*SIGNAL_LENGTH);
    memcpy(&s[1][0], &rp_osc_signals[1][0], sizeof(float)*SIGNAL_LENGTH);
    memcpy(&s[2][0], &rp_osc_signals[2][0], sizeof(float)*SIGNAL_LENGTH);

    *sig_idx = rp_osc_sig_last_idx;

    rp_osc_signals_dirty = 0;
    pthread_mutex_unlock(&rp_osc_sig_mutex);
    return 0;
}

/** @brief Internal function - copies newly computed signals to output holder
 *
 * This internal function copies newly computed signals from working signal 
 * holder to the output signal holder. It makes new measurement available for 
 * main module to pull them out.
 *
 * @param [in] source New signals, which needs to be copied to output signal 
 *                    holder.
 * @param [in] index Last written index in the signals (used for long acq. 
 *                   operation).
 *
 * @retval 0 Always returns 0
*/
int rp_osc_set_signals(float **source, int index)
{
    pthread_mutex_lock(&rp_osc_sig_mutex);

    memcpy(&rp_osc_signals[0][0], &source[0][0], sizeof(float)*SIGNAL_LENGTH);
    memcpy(&rp_osc_signals[1][0], &source[1][0], sizeof(float)*SIGNAL_LENGTH);
    memcpy(&rp_osc_signals[2][0], &source[2][0], sizeof(float)*SIGNAL_LENGTH);
    rp_osc_sig_last_idx = index;

    rp_osc_signals_dirty = 1;
    pthread_mutex_unlock(&rp_osc_sig_mutex);

    return 0;
}

/** @brief Main worker thread function.
 *
 * This is main worker thread function which implements continuous loop. This
 * loop is controlled with the state variable. This function should not be called
 * directly but with POSIX thread functions.
 * 
 * @param [in] args Optinal arguments as defined by pthread API functions.
 *
*/
void *rp_osc_worker_thread(void *args)
{
    rp_osc_worker_state_t old_state, state;
    rp_osc_params_t       curr_params[PARAMS_NUM];
    int                   fpga_update = 0;
    int                   dec_factor = 0;
    int                   time_vect_update = 0;
    uint32_t              trig_source = 0;
    int                   params_dirty = 0;

   

    pthread_mutex_lock(&rp_osc_ctrl_mutex);
    old_state = state = rp_osc_ctrl;
    pthread_mutex_unlock(&rp_osc_ctrl_mutex);

    /* Continuous thread loop (exited only with 'quit' state) */
    while(1) {
        /* update states - we save also old state to see if we need to reset
         * FPGA 
         */
        old_state = state;
        pthread_mutex_lock(&rp_osc_ctrl_mutex);
        state = rp_osc_ctrl;
        if(rp_osc_params_dirty) {
            memcpy(&curr_params, &rp_osc_params, 
                   sizeof(rp_osc_params_t)*PARAMS_NUM);
            fpga_update = rp_osc_params_fpga_update;

            rp_osc_params_dirty = 0;
            dec_factor = 
                osc_fpga_cnv_time_range_to_dec(curr_params[TIME_RANGE_PARAM].value);
            time_vect_update = 1;
        }
        pthread_mutex_unlock(&rp_osc_ctrl_mutex);

        /* request to stop worker thread, we will shut down */

        if(state == rp_osc_quit_state) {
            return 0;
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
                                      curr_params[EQUAL_FILT_PARAM].value,
                                      curr_params[SHAPE_FILT_PARAM].value,
                                      curr_params[GAIN1_PARAM].value,
                                      curr_params[GAIN2_PARAM].value) < 0) {
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

            rp_osc_prepare_time_vector((float **)&rp_tmp_signals[0], 
                                       dec_factor,
                                       curr_params[MIN_GUI_PARAM].value,
                                       curr_params[MAX_GUI_PARAM].value,
                                       curr_params[TIME_UNIT_PARAM].value);

            time_vect_update = 0;

         
	    
	   
        }

       
        
        /* Start new acquisition only if it is the index 0 (new acquisition) */
      
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
        

        /* start working */
        pthread_mutex_lock(&rp_osc_ctrl_mutex);
        old_state = state = rp_osc_ctrl;
        pthread_mutex_unlock(&rp_osc_ctrl_mutex);
        if((state == rp_osc_idle_state) || (state == rp_osc_abort_state)) {
            continue;
        } else if(state == rp_osc_quit_state) {
            break;
        }

       
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
                
                if(osc_fpga_triggered()) {
                    
                    break;
                } 
                usleep(1000);
            }
        

        if((state != old_state) || params_dirty) {
            params_dirty = 0;
            continue;
        }
     

        pthread_mutex_lock(&rp_osc_ctrl_mutex);
        state = rp_osc_ctrl;
        params_dirty = rp_osc_params_dirty;
        pthread_mutex_unlock(&rp_osc_ctrl_mutex);

        if((state != old_state) || params_dirty)
            continue;

	
	
		
        
            /* Triggered, decimate & convert the values */
            rp_osc_decimate((float **)&rp_tmp_signals[1], &rp_fpga_cha_signal[0],
                            (float **)&rp_tmp_signals[2], &rp_fpga_chb_signal[0],
                            (float **)&rp_tmp_signals[0], dec_factor, 
                            curr_params[MIN_GUI_PARAM].value,
                            curr_params[MAX_GUI_PARAM].value,
                            curr_params[TIME_UNIT_PARAM].value);
        

        /* check again for change of state */
        pthread_mutex_lock(&rp_osc_ctrl_mutex);
        state = rp_osc_ctrl;
        pthread_mutex_unlock(&rp_osc_ctrl_mutex);

        /* We have acquisition - if we are in single put state machine
         * to idle */
        if((state == rp_osc_single_state) ) {

            rp_osc_worker_change_state(rp_osc_idle_state);
        }

      
            rp_osc_set_signals(rp_tmp_signals, SIGNAL_LENGTH-1);
        
        /* do not loop too fast */
        usleep(10000);
    }

    return 0;
}

/** @brief Prepares new time vector.
 * 
 * Prepares new time vector based on currently used parameters.
 *
 * @param [out] out_signal New output time signal, from t_start to t_stop
 * @param [in] dec_factor Decimation factor programmed in FPGA.
 * @param [in] t_start Starting time defined by main module and client.
 * @param [in] t_stop Stop time defined by main module and client.
 * @param [in] time_unit Time units used with current parameters.
 *
 * @retval 0 Always returns 0
*/
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

/** @brief Main processing function used to decimate and convert data signals.
 *
 * This function takes as argument raw FPGA signal and performs decimation and
 * conversion from ADC samples to voltage.
 * Output signals are of length SIGNAL_LENGTH (defined in main_osc.h), input 
 * signals are of length OSC_FPGA_SIG_LEN (defined in fpga_osc.h).
 *
 * @param [out] cha_signal - processed output signal for Channel A
 * @param [in] in_cha_signal - input raw signal for Channel A
 * @param [out] chb_signal - processed output signal for Channel B
 * @param [in] in_chb_signal - input raw signal for Channel B
 * @param [out] time_signal - processed output time vector signal 
 * @param [in] dec_factor - decimation factor parameter set by main or client
 * @param [in] t_start - starting time of requested acquisition
 * @param [in] t_stop - stopping time of requested acquisition
 * @param [in] time_unit - time units currently used
 *
 * @retval 0 Always returns 0.
*/
int rp_osc_decimate(float **cha_signal, int *in_cha_signal,
                    float **chb_signal, int *in_chb_signal,
                    float **time_signal, int dec_factor, 
                    float t_start, float t_stop, int time_unit)
{
    int t_start_idx, t_stop_idx;
    float smpl_period = c_osc_fpga_smpl_period * dec_factor;
    int   t_unit_factor = rp_osc_get_time_unit_factor(time_unit);
    int t_step;
    int in_idx, out_idx, t_idx;
    int wr_ptr_curr, wr_ptr_trig;

    float *cha_s = *cha_signal;
    float *chb_s = *chb_signal;
    float *t = *time_signal;
            
    /* If illegal take whole frame */
    if(t_stop <= t_start) {
        t_start = 0;
        t_stop = (OSC_FPGA_SIG_LEN-1) * smpl_period;
    }
    
    /* convert time to samples */
    t_start_idx = round(t_start / smpl_period);
    t_stop_idx  = round(t_stop / smpl_period);

    if((((t_stop_idx-t_start_idx)/(float)(SIGNAL_LENGTH-1))) < 1)
        t_step = 1;
    else {
        /* ceil was used already in rp_osc_main() for parameters, so we can easily
         * use round() here 
         */
        t_step = round((t_stop_idx-t_start_idx)/(float)(SIGNAL_LENGTH-1));
    }
    osc_fpga_get_wr_ptr(&wr_ptr_curr, &wr_ptr_trig);
    
    
   
    
    if (dec_factor==1)      
      in_idx = wr_ptr_trig + t_start_idx + 3;  
    else
      in_idx = wr_ptr_trig + t_start_idx + 1;  // There it additional FPGA delay when decimation enabled.      
      
    if (dec_factor>8192)   
      in_idx = wr_ptr_trig + t_start_idx + 0; 
    
    
    if(in_idx < 0) 
        in_idx = OSC_FPGA_SIG_LEN + in_idx;
    if(in_idx >= OSC_FPGA_SIG_LEN)
        in_idx = in_idx % OSC_FPGA_SIG_LEN;

    
    
    
    for(out_idx=0, t_idx=0; out_idx < SIGNAL_LENGTH; 
        out_idx++, in_idx+=t_step, t_idx+=t_step) {
        /* Wrap the pointer */
        if(in_idx >= OSC_FPGA_SIG_LEN)
            in_idx = in_idx % OSC_FPGA_SIG_LEN;

        cha_s[out_idx] = osc_fpga_cnv_cnt_to_v(in_cha_signal[in_idx]);
        chb_s[out_idx] = osc_fpga_cnv_cnt_to_v(in_chb_signal[in_idx]);
        t[out_idx] = (t_start + (t_idx * smpl_period)) * t_unit_factor;
        
	
    
	  
	}

    return 0;
}

/** @brief Processing function used 
 *
 * This function takes as argument raw FPGA signal and performs decimation and
 * conversion from ADC samples to voltage.
 * Output signals are of length SIGNAL_LENGTH (defined in main_osc.h), input 
 * signals are of length OSC_FPGA_SIG_LEN (defined in fpga_osc.h).


 * @param [out] cha_out_signal - processed output signal for Channel A
 * @param [in] cha_in_signal - input raw signal for Channel A
 * @param [out] chb_out_signal - processed output signal for Channel B
 * @param [in] chb_in_signal - input raw signal for Channel B
 * @param [out] time_out_signal - processed output time vector signal 
 * @param [inout] next_wr_ptr - write pointer used for FPGA signal readout 
 *                              (it is updated during decimation and returned, so
 *                              worker thread can wait for next samples)
 * @param [in] last_wr_ptr - Last expected write pointer (when next_wr_ptr 
 *                           reaches this value full signal is acquired)
 * @param [in] next_out_idx - Starting index for output signals 
 * @param [in] dec_factor - decimation factor parameter set by main or client
 * @param [in] t_start - starting time of requested acquisition
 * @param [in] t_stop - stopping time of requested acquisition
 * @param [in] time_unit - time units currently used
 *
 * @retval last_out_idx Function returns last sample written to output signal.
*/
int rp_osc_decimate_partial(float **cha_out_signal, int *cha_in_signal, 
                            float **chb_out_signal, int *chb_in_signal,
                            float **time_out_signal, int *next_wr_ptr, 
                            int last_wr_ptr, int step_wr_ptr, int next_out_idx,
                            float t_start, int dec_factor, int time_unit)
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

    for(; (next_out_idx < SIGNAL_LENGTH); next_out_idx++, 
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

        cha_out[next_out_idx] = osc_fpga_cnv_cnt_to_v(cha_in_signal[in_idx]);
        chb_out[next_out_idx] = osc_fpga_cnv_cnt_to_v(chb_in_signal[in_idx]);
        t_out[next_out_idx]   = 
            (t_start + ((next_out_idx*step_wr_ptr)*smpl_period))*t_unit_factor;

    }

    *next_wr_ptr = in_idx;

    return next_out_idx;
}

/** @brief Helper function which converts time_unit to time factor.
 *
 * This function converts time unit used by Oscilloscope application to unit 
 * factor used to calculate time to [s].
 * 
 * @param [in] time_unit Currently used time unit
 * 
 */
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
