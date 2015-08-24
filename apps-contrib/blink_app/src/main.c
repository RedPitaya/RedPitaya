/** @file main.c
 *
 * $Id: main.c 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Main module.
 * @author Jure Menart <juremenart@gmail.com>
 * @copyright Red Pitaya  http://www.redpitaya.com
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

#include "main.h"
#include "version.h"
#include "worker.h"
#include "fpga.h"
#include "house_kp.h"


/* Describe app. parameters with some info/limitations */
pthread_mutex_t rp_main_params_mutex = PTHREAD_MUTEX_INITIALIZER;
static rp_app_params_t rp_main_params[PARAMS_NUM+1] = {
    { /* min_gui_time   */ 
        //"xmin", -1000000, 1, 0, -10000000, +10000000 },
        "xmin", 0, 1, 0, -10000000, +10000000 },
    { /* max_gui_time   */ 
        //"xmax", +1000000, 1, 0, -10000000, +10000000 },
        "xmax", 131, 1, 0, -10000000, +10000000 },
    { /* trig_mode:
       *    0 - auto
       *    1 - normal
       *    2 - single  */
        "trig_mode", 0, 1, 0,         0,         2 },
    { /* trig_source:
       *    0 - ChA
       *    1 - ChB
       *    2 - ext.    */
        "trig_source", 0, 1, 0,         0,         2 },
    { /* trig_edge:
       *    0 - rising
       *    1 - falling */
        "trig_edge", 0, 1, 0,         0,         1 },
    { /* trig_delay     */
        "trig_delay", 0, 1, 1, -10000000, +10000000 },
    { /* trig_level     */
        "trig_level", 0, 1, 0,     -1000,     +1000 },
    { /* single_button:
       *    0 - ignore 
       *    1 - trigger */
        "single_btn", 0, 1, 0,         0,         1 },
    { /* time_range:
       *  decimation:
       *    0 - 1x
       *    1 - 8x
       *    2 - 64x
       *    3 - 1kx
       *    4 - 8kx
       *    5 - 65kx   */
        "time_range", 0, 1, 1,         0,         5 },
    { /* time_unit_used:
       *    0 - [us]
       *    1 - [ms]
       *    2 - [s]     */
        "time_units", 0, 0, 1,         0,         2 },
    { /* en_avg_at_dec:
           *    0 - disable
           *    1 - enable */
        "en_avg_at_dec", 0, 1, 0,      0,         1 },
    { /* auto_flag: 
       * Puts the controller to auto mode - the algorithm which detects input
       * signal and changes the parameters to most fit the input:
       *    0 - normal operation
       *    1 - auto button pressed */
        "auto_flag", 0, 1, 1, 0, 1 },
    { /* min_y, max_y - Controller defined Y range when using auto-set or after
       * gain change y range */
        "min_y", 0, 0, 0, -1000, +1000 },
    { /* min_y, max_y - Controller defined Y range when using auto-set or after
       * gain change y range */
        "max_y", 0, 0, 0, -1000, +1000 },
    { /* forcex_flag: 
       * Server sets this flag when X axis time units change
       * Client checks this flag, when set the server's xmin:xmax define the visualization range 
       *    0 - normal operation
       *    1 - Server forces xmin, xmax  */
        "forcex_flag", 0, 0, 0, 0, 1 }, 
      /* Measurement parameters for both channels. All are read-only and they
       * are calculated on FPGA buffer (non decimated in SW):
       * min, max [V] - minimum and maximum value in the buffer (non-decimated)
       * amp [Vpp] - amplitude = maximum - minum
       * avg [V] - average value
       * freq [MHz] - frequency of the signal (if any, otherwise NaN)
       * period [s] - period of the signal (if any, otherwise NaN)
       **/
    {  "meas_min_ch1", 0, 0, 1, -1000, +1000 },
    {  "meas_max_ch1", 0, 0, 1, +1000, -1000 },
    {  "meas_amp_ch1", 0, 0, 1, +1000, -1000 },
    {  "meas_avg_ch1", 0, 0, 1, +1000, -1000 },
    {  "meas_freq_ch1", 0, 0, 1, 0, 1e9 },
    {  "meas_per_ch1", 0, 0, 1, 0, 1e9 },
    {  "meas_min_ch2", 0, 0, 1, -1000, +1000 },
    {  "meas_max_ch2", 0, 0, 1, +1000, -1000 },
    {  "meas_amp_ch2", 0, 0, 1, +1000, -1000 },
    {  "meas_avg_ch2", 0, 0, 1, +1000, -1000 },
    {  "meas_freq_ch2", 0, 0, 1, 0, 1e9 },
    {  "meas_per_ch2", 0, 0, 1, 0, 1e9 },
    { /* prb_att_ch1 - User probe attenuation setting for channel 1:
       *    0 - 1x
       *    1 - 10x */
        "prb_att_ch1", 0, 1, 0, 0, 1 },
    { /* gain_ch1 - User jumper gain setting for channel 1:
       *    0 - high gain (0.6 [V] Full-scale)
       *    1 - low gain (15 [V] Full-scale) */
        "gain_ch1", 0, 1, 0, 0, 1 },
    { /* prb_att_ch2 - User probe attenuation setting for channel 2:
       *    0 - 1x
       *    1 - 10x */
        "prb_att_ch2", 0, 1, 0, 0, 1 },
    { /* gain_ch2 - User jumper gain setting for channel 2:
       *    0 - high gain (0.6 [V] Full-scale)
       *    1 - low gain (15 [V] Full-scale) */
        "gain_ch2", 0, 1, 0, 0, 1 },
    { /* gui_reset_y_range - Maximum voltage range [Vpp] with current settings
       * This parameter is calculated by application and is read-only for
       * client.
       */
        "gui_reset_y_range", 28, 0, 1, 0, 2000 },
    { /* prepare_wave - GUI notification for CPU to download waveform:
       *    0 - normal operation (no download)
       *    1 - button for download presses - this flag is self-clearing when
       *        download is finished */
        "prepare_wave", 0, 0, 0, 0, 1 },

    /* ------------------------------------ END OF APP PARAMS ------------------------------------------------*/
    
    { /* Start blink*/
      "rp_start_blink", 0, 1, 0, 0, 3},

    { /* Blink diode parameter  */
      "rp_led", 1, 1, 0, 0, 7 },
    
    { /* Must be last! */
        NULL, 0.0, -1, -1, 0.0, 0.0 }     
};
/* params initialized */
static int params_init = 0;

rp_calib_params_t rp_main_calib_params;
float rp_main_ch1_max_adc_v;
float rp_main_ch2_max_adc_v;

int forcex_state=0;
float forced_xmin=0;
float forced_xmax=0;
float forced_units=0;
float forced_delay=0;



const char *rp_app_desc(void)
{
    return (const char *)"Red Pitaya osciloscope application.\n";
}

int rp_app_init(void)
{
    
    fprintf(stderr, "Loading scope version %s-%s.\n", VERSION_STR, REVISION_STR);

    house_init();

    rp_default_calib_params(&rp_main_calib_params);
    if(rp_read_calib_params(&rp_main_calib_params) < 0) {
        fprintf(stderr, "rp_read_calib_params() failed, using default"
                " parameters\n");
    }
    if(rp_osc_worker_init(&rp_main_params[0], PARAMS_NUM, 
                          &rp_main_calib_params) < 0) {
        return -1;
    }

    if(rp_main_params[GAIN_CH1].value == 0) {
        rp_main_ch1_max_adc_v = 
            osc_fpga_calc_adc_max_v(rp_main_calib_params.fe_ch1_fs_g_hi, 
                                    rp_main_params[PRB_ATT_CH1].value);
    } else {
        rp_main_ch1_max_adc_v = 
            osc_fpga_calc_adc_max_v(rp_main_calib_params.fe_ch1_fs_g_lo, 
                                    rp_main_params[PRB_ATT_CH1].value);
    }
    if(rp_main_params[GAIN_CH2].value == 0) {
        rp_main_ch2_max_adc_v = 
            osc_fpga_calc_adc_max_v(rp_main_calib_params.fe_ch2_fs_g_hi, 
                                    rp_main_params[PRB_ATT_CH2].value);
    } else {
        rp_main_ch2_max_adc_v = 
            osc_fpga_calc_adc_max_v(rp_main_calib_params.fe_ch2_fs_g_lo, 
                                    rp_main_params[PRB_ATT_CH2].value);
    }

    rp_set_params(&rp_main_params[0], PARAMS_NUM);

    return 0;
}

int rp_app_exit(void)
{
    fprintf(stderr, "Unloading scope version %s-%s.\n", VERSION_STR, REVISION_STR);

    rp_osc_worker_exit();

    return 0;
}



int rp_set_params(rp_app_params_t *p, int len)
{
    int i;
    int fpga_update = 1;
    int params_change = 0;
    

    if(len > PARAMS_NUM) {
        fprintf(stderr, "Too much parameters, max=%d\n", PARAMS_NUM);
        return -1;
    }

    pthread_mutex_lock(&rp_main_params_mutex);
    for(i = 0; i < len || p[i].name != NULL; i++) {
        int p_idx = -1;
        int j = 0;
        /* Search for correct parameter name in defined parameters */
        while(rp_main_params[j].name != NULL) {
            int p_strlen = strlen(p[i].name);

            if(p_strlen != strlen(rp_main_params[j].name)) {
                j++;
                continue;
            }
            if(!strncmp(p[i].name, rp_main_params[j].name, p_strlen)) {
                p_idx = j;
                break;
            }
            j++;
        }

        if(p_idx == -1) {
            fprintf(stderr, "Parameter %s not found, ignoring it\n", p[i].name);
            continue;
        }

        if(rp_main_params[p_idx].read_only)
            continue;

        if(rp_main_params[p_idx].value != p[i].value) {
            if(rp_main_params[p_idx].fpga_update)
                fpga_update = 1;
        }
        if(rp_main_params[p_idx].min_val > p[i].value) {
            fprintf(stderr, "Incorrect parameters value: %f (min:%f), "
                    " correcting it\n", p[i].value, rp_main_params[p_idx].min_val);
            p[i].value = rp_main_params[p_idx].min_val;
        } else if(rp_main_params[p_idx].max_val < p[i].value) {
            fprintf(stderr, "Incorrect parameters value: %f (max:%f), "
                    " correcting it\n", p[i].value, rp_main_params[p_idx].max_val);
            p[i].value = rp_main_params[p_idx].max_val;
        }
        rp_main_params[p_idx].value = p[i].value;
    }
    pthread_mutex_unlock(&rp_main_params_mutex);
    

    
    if(params_change || (params_init == 0)) {


        /* First do health check and then send it to the worker! */
        int mode = rp_main_params[TRIG_MODE_PARAM].value;
        int time_range = rp_main_params[TIME_RANGE_PARAM].value;
        int time_unit = 2;
        /* Get info from FPGA module about clocks/decimation, ...*/
        int dec_factor = osc_fpga_cnv_time_range_to_dec(time_range);
        float smpl_period = c_osc_fpga_smpl_period * dec_factor;
        /* t_delay - trigger delay in seconds */
        float t_delay = rp_main_params[TRIG_DLY_PARAM].value;
        float t_unit_factor = 1; /* to convert to seconds */

        TRACE("After T: %d, %dx\n", time_range, dec_factor);
        /* Our time window with current settings:
         *   - time_delay is added later, when we check if it is correct 
         *     setting 
         */
        float t_min = 0;
        float t_max = ((OSC_FPGA_SIG_LEN-1) * smpl_period);

        params_init = 1;
        /* in time units time_unit, needs to be converted */
        float t_start = rp_main_params[MIN_GUI_PARAM].value;
        float t_stop  = rp_main_params[MAX_GUI_PARAM].value;
        int t_start_idx;
        int t_stop_idx;
        int t_step_idx = 0;
        float ch1_max_adc_v, ch2_max_adc_v;
        float ch1_delta, ch2_delta;

        /* If auto-set algorithm was requested do not set other parameters */
        if(rp_main_params[AUTO_FLAG_PARAM].value == 1) {
            rp_osc_clean_signals();
            rp_osc_worker_change_state(rp_osc_auto_set_state);
            /* AUTO_FLAG_PARAM is cleared when Auto-set algorithm finishes */
            /*            rp_main_params[AUTO_FLAG_PARAM].value = 0;*/
            
            /* Wait for auto-set algorithm to finish or timeout */
            int timeout = 10000000; // [us]
            const int step = 50000; // [us]
            rp_osc_worker_state_t state;
            while (timeout > 0) {
         
                rp_osc_worker_get_state(&state);
                if (state != rp_osc_auto_set_state) {
                    break;
                }
                
                usleep(step);
                timeout -= step;
            }

            if (timeout <= 0) {
                fprintf(stderr, "AUTO: Timeout waiting for AUTO-set algorithm to finish.\n");
            }

            return 0;
        }

        /* if AUTO reset trigger delay */
        if(mode == 0) 
            t_delay = 0;

        if(dec_factor < 0) {
            fprintf(stderr, "Incorrect time range: %d\n", time_range);
            return -1;
        }

        /* pick correct which time unit is selected */
        if((time_range == 0) || (time_range == 1)) {
            time_unit     = 0;
            t_unit_factor = 1e6;
        } else if((time_range == 2) || (time_range == 3)) {
            time_unit     = 1;
            t_unit_factor = 1e3;
        } 

        TRACE("After T: time_unit = %d\n", time_unit);

        rp_main_params[TIME_UNIT_PARAM].value = time_unit;

        /* Check if trigger delay in correct range, otherwise correct it
         * Correct trigger delay is:
         *  t_delay >= -t_max
         *  t_delay <= OSC_FPGA_MAX_TRIG_DELAY
         */
        if(t_delay < -t_max) {
            t_delay = -t_max;
        } else if(t_delay > (OSC_FPGA_TRIG_DLY_MASK * smpl_period)) {
            t_delay = OSC_FPGA_TRIG_DLY_MASK * smpl_period;
        } else {
            t_delay = round(t_delay / smpl_period) * smpl_period;
        }
        t_min = t_min + t_delay;
        t_max = t_max + t_delay;
        rp_main_params[TRIG_DLY_PARAM].value = t_delay;

        /* convert to seconds */
        t_start = t_start / t_unit_factor;
        t_stop  = t_stop  / t_unit_factor;

        /* Select correct time window with this settings:
         * time window is defined from:
         *  ([ 0 - 16k ] * smpl_period) + trig_delay */
        /* round to correct/possible values - convert to nearest index
         * and back 
         */
        t_start_idx = round(t_start / smpl_period);
        t_stop_idx  = round(t_stop / smpl_period);

        t_start = (t_start_idx * smpl_period);
        t_stop  = (t_stop_idx * smpl_period);

        if(t_start < t_min) 
            t_start = t_min;
        if(t_stop > t_max)
            t_stop = t_max;
        if(t_stop <= t_start )
            t_stop = t_max;

        /* Correct the window according to possible decimations - always
         * provide at least the data demanded by the user (ceil() instead
         * of round())
         */
        t_start_idx = round(t_start / smpl_period);
        t_stop_idx  = round(t_stop / smpl_period);

        if((((t_stop_idx-t_start_idx)/(float)(SIGNAL_LENGTH-1))) < 1)
            t_step_idx = 1;
        else {
            t_step_idx = ceil((t_stop_idx-t_start_idx)/(float)(SIGNAL_LENGTH-1));
            int max_step = OSC_FPGA_SIG_LEN/SIGNAL_LENGTH;
            if(t_step_idx > max_step)
                t_step_idx = max_step;
        }

        t_stop = t_start + SIGNAL_LENGTH * t_step_idx * smpl_period;

        /* write back and convert to set units */
        rp_main_params[MIN_GUI_PARAM].value = t_start;
        rp_main_params[MAX_GUI_PARAM].value = t_stop;

        /* Calculate new gui_reset_y_range */
        if(rp_main_params[GAIN_CH1].value == 0) {
            ch1_max_adc_v = 
                osc_fpga_calc_adc_max_v(rp_main_calib_params.fe_ch1_fs_g_hi, 
                                        rp_main_params[PRB_ATT_CH1].value);
        } else {
            ch1_max_adc_v = 
                osc_fpga_calc_adc_max_v(rp_main_calib_params.fe_ch1_fs_g_lo, 
                                        rp_main_params[PRB_ATT_CH1].value);
        }
        if(rp_main_params[GAIN_CH2].value == 0) {
            ch2_max_adc_v = 
                osc_fpga_calc_adc_max_v(rp_main_calib_params.fe_ch2_fs_g_hi, 
                                        rp_main_params[PRB_ATT_CH2].value);
        } else {
            ch2_max_adc_v = 
                osc_fpga_calc_adc_max_v(rp_main_calib_params.fe_ch2_fs_g_lo, 
                                        rp_main_params[PRB_ATT_CH2].value);
        }

        if(ch1_max_adc_v > ch2_max_adc_v)
            rp_main_params[GUI_RST_Y_RANGE].value = 2.0 * ch1_max_adc_v;
        else
            rp_main_params[GUI_RST_Y_RANGE].value = 2.0 * ch2_max_adc_v;

        /* Re-calculate output parameters */
        ch1_delta = (ch1_max_adc_v / rp_main_ch1_max_adc_v);
        ch2_delta = (ch2_max_adc_v / rp_main_ch2_max_adc_v);

        rp_main_ch1_max_adc_v = ch1_max_adc_v;
        rp_main_ch2_max_adc_v = ch2_max_adc_v;

        if(ch1_delta > ch2_delta) {
            rp_main_params[MIN_Y_PARAM].value *= ch1_delta;
            rp_main_params[MAX_Y_PARAM].value *= ch1_delta;
        } else {
            rp_main_params[MIN_Y_PARAM].value *= ch2_delta;
            rp_main_params[MAX_Y_PARAM].value *= ch2_delta;
        }

        if((int)rp_main_params[TRIG_SRC_PARAM].value == 0) {
            /* Trigger selected on Channel 1 */
            rp_main_params[TRIG_LEVEL_PARAM].value *= ch1_delta;
        } else {
            /* Trigger selected on Channel 2 */
            rp_main_params[TRIG_LEVEL_PARAM].value *= ch2_delta;
        }

        rp_osc_worker_update_params((rp_app_params_t *)&rp_main_params[0], 
                                    fpga_update);

        /* check if we need to change state */
        switch(mode) {
        case 0:
            /* auto */
            rp_osc_worker_change_state(rp_osc_auto_state);
            break;
        case 1:
            /* normal */
            rp_osc_worker_change_state(rp_osc_normal_state);
            break;
        case 2:
            /* single - clear last ok buffer */
            rp_osc_worker_change_state(rp_osc_idle_state);
            rp_osc_clean_signals();
            break;
        default:
            return -1;
        }

        if(rp_main_params[SINGLE_BUT_PARAM].value == 1) {
            rp_main_params[SINGLE_BUT_PARAM].value = 0;
            rp_osc_clean_signals();
            rp_osc_worker_change_state(rp_osc_single_state);
        }

    }

    return 0;
}

/* Returned vector must be free'd externally! */
int rp_get_params(rp_app_params_t **p)
{
    rp_app_params_t *p_copy = NULL;
    int t_unit_factor;
    int i;

    p_copy = (rp_app_params_t *)malloc((PARAMS_NUM+1) * sizeof(rp_app_params_t));
    if(p_copy == NULL)
        return -1;

    pthread_mutex_lock(&rp_main_params_mutex);
    t_unit_factor =  
        rp_osc_get_time_unit_factor(rp_main_params[TIME_UNIT_PARAM].value);

    for(i = 0; i < PARAMS_NUM; i++) {
        int p_strlen = strlen(rp_main_params[i].name);
        p_copy[i].name = (char *)malloc(p_strlen+1);
        strncpy((char *)&p_copy[i].name[0], &rp_main_params[i].name[0], 
                p_strlen);
        p_copy[i].name[p_strlen]='\0';

        p_copy[i].value       = rp_main_params[i].value;
        p_copy[i].fpga_update = rp_main_params[i].fpga_update;
        p_copy[i].read_only   = rp_main_params[i].read_only;
        p_copy[i].min_val     = rp_main_params[i].min_val;
        p_copy[i].max_val     = rp_main_params[i].max_val;
    }
    pthread_mutex_unlock(&rp_main_params_mutex);
    p_copy[PARAMS_NUM].name = NULL;

    p_copy[MIN_GUI_PARAM].value = p_copy[MIN_GUI_PARAM].value * t_unit_factor;
    p_copy[MAX_GUI_PARAM].value = p_copy[MAX_GUI_PARAM].value * t_unit_factor;

    *p = p_copy;
    return PARAMS_NUM;
}

int rp_get_signals(float ***s, int *sig_num, int *sig_len)
{
    int ret_val;
    int sig_idx;

    if(*s == NULL)
        return -1;

    *sig_num = SIGNALS_NUM;
    *sig_len = SIGNAL_LENGTH;

    ret_val = rp_osc_get_signals(s, &sig_idx);
    
    if(ret_val == 0){
      printf("Starting OK\n");
    }
   //Not finished signal
    if((ret_val != -1) && sig_idx != SIGNAL_LENGTH-1) {
        return -2;
    }
    
    //Old signal
    if(ret_val < 0) {
        return -1;
    }
    
    return 0;
}

int rp_create_signals(float ***a_signals)
{
    int i;
    float **s;

    s = (float **)malloc(SIGNALS_NUM * sizeof(float *));
    if(s == NULL) {
        return -1;
    }
    for(i = 0; i < SIGNALS_NUM; i++)
        s[i] = NULL;

    for(i = 0; i < SIGNALS_NUM; i++) {
        s[i] = (float *)malloc(SIGNAL_LENGTH * sizeof(float));
        if(s[i] == NULL) {
            rp_cleanup_signals(a_signals);
            return -1;
        }
        memset(&s[i][0], 0, SIGNAL_LENGTH * sizeof(float));
    }
    *a_signals = s;

    return 0;
}

void rp_cleanup_signals(float ***a_signals)
{
    int i;
    float **s = *a_signals;

    if(s) {
        for(i = 0; i < SIGNAL_LENGTH; i++) {
            if(s[i]) {
                free(s[i]);
                s[i] = NULL;
            }
        }
        free(s);
        *a_signals = NULL;
    }
}

/*----------------------------------------------------------------------------------*/
/**
 * @brief Make a copy of Application parameters
 *
 * Function copies actual Application parameters to the specified destination
 * buffer. This action was intended to prepare two parameter instances, where the first
 * one can be further modified from the user side, while the second one is processed by
 * the worker thread.
 * In case the destination buffer is not allocated yet, it is allocated internally and must
 * be freed outside of the function scope by calling rp_clean_params() function. Note that
 * if function returns failure, the destination buffer could be partially allocated and must
 * be freed in the same way.
 * If the specified destination buffer is already allocated, it is assumed the number of table
 * entries is the same as in the source table. No special check is made internally if this is really
 * the case.
 *
 * @param[in]   src  Source application parameters
 * @param[out]  dst  Destination application parameters
 * @retval      -1   Failure, error message is output on standard error
 * @retval      0    Successful operation
 */
int rp_copy_params(rp_app_params_t *src, rp_app_params_t **dst)
{
    rp_app_params_t *p_new = *dst;
    int i, num_params;

    /* check arguments */
    if (src == NULL) {
        fprintf(stderr, "Internal error, the source Application parameters are not specified.\n");
        return -1;
    }

    /* check if destination buffer is allocated or not */
    if(p_new == NULL) {
        i = 0;

        /* retrieve the number of source parameters */
        num_params=0;
        while(src[i++].name != NULL)
            num_params++;

        /* allocate array of parameter entries, parameter names must be allocated separately */
        p_new = (rp_app_params_t *)malloc(sizeof(rp_app_params_t) * (num_params+1));
        if(p_new == NULL) {
            fprintf(stderr, "Memory problem, the destination buffer could not be allocated.\n");
            return -1;
        }

        /* scan source parameters, allocate memory space for parameter names and copy values */
        i = 0;
        while(src[i].name != NULL) {
            p_new[i].name = (char *)malloc(strlen(src[i].name)+1);
            if(p_new[i].name == NULL)
                return -1;

            strncpy(p_new[i].name, src[i].name, strlen(src[i].name));
            p_new[i].name[strlen(src[i].name)]='\0';
            p_new[i].value = src[i].value;
            i++;
        }

        /* mark last one */
        p_new[num_params].name = NULL;
        p_new[num_params].value = -1;

    } else {
        /* destination buffer is already allocated, just copy values */
        i = 0;
        while(src[i].name != NULL) {
            p_new[i].value = src[i].value;
            i++;
        }
        
    }
    
    if(rp_main_params[RP_START_BLINK].value == 1){

      set_led((rp_main_params[RP_LED].value));
      rp_main_params[RP_START_BLINK].value = 0; 
    
    }else if(rp_main_params[RP_START_BLINK].value == 2){
      
      unset_led(rp_main_params[RP_LED].value);
      rp_main_params[RP_START_BLINK].value = 0;
    
    }
    

    *dst = p_new;
    return 0;
}


/*----------------------------------------------------------------------------------*/
/**
 * @brief Deallocate the specified buffer of Application parameters
 *
 * Function is used to deallocate the specified buffers, which were previously
 * allocated by calling rp_copy_params() function.
 *
 * @param[in]   params  Application parameters to be deallocated
 * @retval      0       Success, never fails
 */

int rp_clean_params(rp_app_params_t *params)
{
    int i = 0;
    /* cleanup params structure */
    if(params) {
        while(params[i].name != NULL) {
            if(params[i].name)
                free(params[i].name);
            params[i].name = NULL;
            i++;
        }
        free(params);
        params = NULL;
    }
    return 0;
}

int rp_update_main_params(rp_app_params_t *params)
{
    int i = 0;
    if(params == NULL)
        return -1;

    pthread_mutex_lock(&rp_main_params_mutex);
    while(params[i].name != NULL) {
        rp_main_params[i].value = params[i].value;
        i++;
    }
    pthread_mutex_unlock(&rp_main_params_mutex);
    params_init = 0;
    rp_set_params(&rp_main_params[0], PARAMS_NUM);

    return 0;
}


