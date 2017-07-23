/**
 * $Id: main.c 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Oscilloscope main module.
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
#include "calib.h"
#include "generate.h"
#include "pid.h"

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
    { /* trig_level : Trigger level, expressed in normalized 1V  */
        "trig_level", 0, 1, 0,     -2,     +2 },
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
        "auto_flag", 0, 1, 0, 0, 1 },
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
    { /* gen_DC_offs_1 - DC offset for channel 1 expressed in [V] requested by 
       * GUI */
        "gen_DC_offs_1", 0, 1, 0, -100, 100 },
    { /* gen_DC_offs_2 - DC offset for channel 2 expressed in [V] requested by 
       * GUI */
        "gen_DC_offs_2", 0, 1, 0, -100, 100 },
    { /* gui_xmin - Xmin as specified by GUI - not rounded to sampling engine quanta. */
        "gui_xmin",      0, 0, 1, -10000000, +10000000 },
    { /* gui_xmax - Xmax as specified by GUI - not rounded to sampling engine quanta. */
        "gui_xmax",    131, 0, 1, -10000000, +10000000 },
    { /* min_y_norm, max_y_norm - Normalized controller defined Y range when using auto-set */
        "min_y_norm", 0, 0, 0, -1000, +1000 },
    { /* min_y_norm, max_y_norm - Normalized controller defined Y range when using auto-set */
        "max_y_norm", 0, 0, 0, -1000, +1000 },
    { /* gen_DC_norm_1 - DC offset for channel 1 expressed in normalized 1V */
        "gen_DC_norm_1", 0, 1, 0, -100, 100 },
    { /* gen_DC_norm_2 - DC offset for channel 2 expressed in normalized 1V */
        "gen_DC_norm_2", 0, 1, 0, -100, 100 },
    { /* scale_ch1 - Jumper & probe attenuation dependent Y scaling factor for Channel 1 */
        "scale_ch1", 0, 0, 1, -1000, 1000 },
    { /* scale_ch2 - Jumper & probe attenuation dependent Y scaling factor for Channel 2 */
        "scale_ch2", 0, 0, 1, -1000, 1000 },

    /* BODE PLOTTER PARAMETERS FROM HERE ON (VERSION 1.0 - TODO CLEAR REST OF PARAMETERS) */
    { /* General bode plotter flag:
       *   -1 - First boot -> No measurment yet made.
       *    0 - Negative -> We have data aquisition, but we aren't doing any more measurments
       *    1 - Start measurment */
        "bode_start_measure", -1, 1, 0, -1, 1},
    { /* Amplitude parameter:
       *    Max value: +1
       *    Min value: +0 */
        "bode_amp", 0, 1, 0, 0, 2},
    { /* Averaging parameter:
       *    Max value: +1000
       *    Min value: +1 */
        "bode_avg", 1, 1, 0, 1, 1000},
    { /* DC_bias parameter:
       *    Max value: +1
       *    Min value: +0 */
        "bode_dc_bias", 0, 1, 0, 0, 2},
    { /* Start frequency parameter:
       *    Max value: +1000000
       *    Min value: +0 */
        "bode_start_f", 0, 1, 0, 0, 1000000},
    { /* Start frequency parameter:
       *    Max value: +1000000
       *    Min value: +0 */
        "bode_end_f", 1, 1, 0, 0, 62000000},
    { /* Bode counts:
       *    Max value: +1000000
       *    Min value: +0 */
        "bode_counts", 1024, 1, 0, 0, 1024},
    { /* Parameter for selecting data plot:
       *    0 - Amplitude
       *    1 - Phase */
        "plot_data", 0, 1, 0, 0, 1},
    { /* Scale type parameter:
       *    0 - Linear
       *    1 - Logarithmic */
        "bode_scale_type", 0, 1, 0, 0, 1},
    { /* Scale type parameter:
       *    0 - Linear
       *    1 - Logarithmic */
        "bode_data_plot", 0, 1, 0, 0, 1},

    /********************************************************/
    /* Arbitrary Waveform Generator parameters from here on */
    /********************************************************/

    { /* gen_trig_mod_ch1 - Selects the trigger mode for channel 1:
       *    0 - continuous
       *    1 - single 
       *    2 - external */
        "gen_trig_mod_ch1", 0, 1, 0, 0, 2 },
    { /* gen_sig_type_ch1 - Selects the type of signal for channel 1:
       *    0 - sine
       *    1 - square
       *    2 - triangle
       *    3 - from file */
        "gen_sig_type_ch1", 0, 1, 0, 0, 3 },
    { /* gen_enable_ch1 - Enables/disable signal generation on channel 1:
       *    0 - Channel 1 disabled
       *    1 - Channel 1 enabled */
        "gen_enable_ch1", 0, 1, 0, 0, 1 },
    { /* gen_single_ch1 - Fire single trigger on generator channel 1:
       *    0 - Do not fire single trigger
       *    1 - Fire single trigger */
        "gen_single_ch1", 0, 1, 0, 0, 1 },
    { /* gen_sig_amp_ch1 - Amplitude for Channel 1 in [Vpp] */
        "gen_sig_amp_ch1", 0, 1, 0, 0, 2.0 },
    { /* gen_sig_freq_ch1 - Frequency for Channel 1 in [Hz] */
        "gen_sig_freq_ch1", 1000, 1, 0, 0, 50e6 },
    { /* gen_sig_dcoff_ch1 - DC offset applied to the signal in [V] */
        "gen_sig_dcoff_ch1", 0, 1, 0, -1, 1 },
    { /* gen_trig_mod_ch2 - Selects the trigger mode for channel 2:
       *    0 - continuous
       *    1 - single 
       *    2 - external */
        "gen_trig_mod_ch2", 0, 1, 0, 0, 2 },
    { /* gen_sig_type_ch2 - Selects the type of signal for channel 2:
       *    0 - sine
       *    1 - square
       *    2 - triangle
       *    3 - from file */
        "gen_sig_type_ch2", 0, 1, 0, 0, 3 },
    { /* gen_enable_ch2 - Enables/disable signal generation on channel 2:
       *    0 - channel 2 disabled
       *    1 - channel 2 enabled */
        "gen_enable_ch2", 0, 1, 0, 0, 1 },
    { /* gen_single_ch2 - Fire single trigger on generator channel 2:
       *    0 - Do not fire single trigger
       *    1 - Fire single trigger */
        "gen_single_ch2", 0, 1, 0, 0, 1 },
    { /* gen_sig_amp_ch2 - Amplitude for channel 2 in [Vpp] */
        "gen_sig_amp_ch2", 0, 1, 0, 0, 2.0 },
    { /* gen_sig_freq_ch2 - Frequency for channel 2 in [Hz] */
        "gen_sig_freq_ch2", 1000, 1, 0, 0.2, 50e6 },
    { /* gen_sig_dcoff_ch2 - DC offset applied to the signal in [V] */
        "gen_sig_dcoff_ch2", 0, 1, 0, -1, 1 },
    { /* gen_awg_refresh - Refresh AWG data from (uploaded) file.
       *     0 - Do not refresh
       *     1 - Refresh Channel 1
       *     2 - Refresh Channel 2
       */
        "gen_awg_refresh",   0, 0, 0, 0, 2 },

    /******************************************/
    /* PID Controller parameters from here on */
    /******************************************/

    { /* pid_NN_enable - Enables/closes or disables/open PID NN loop:
       *    0 - PID disabled (open loop)
       *    1 - PID enabled (closed loop)    */
        "pid_11_enable", 0, 1, 0, 0, 1 },
    { /* pid_NN_rst - Reset PID NN integrator:
        *    0 - Do not reset integrator
        *    1 - Reset integrator            */
        "pid_11_rst", 0, 1, 0, 0, 1 },
    { /* pid_NN_sp - PID NN set-point in [ADC] counts. */
        "pid_11_sp",  0, 1, 0, -8192, 8191 },
    { /* pid_NN_kp - PID NN proportional gain Kp in [ADC] counts. */
        "pid_11_kp",  0, 1, 0, -8192, 8191 },
    { /* pid_NN_ki - PID NN integral gain     Ki in [ADC] counts. */
        "pid_11_ki",  0, 1, 0, -8192, 8191 },
    { /* pid_NN_kd - PID NN derivative gain   Kd in [ADC] counts. */
        "pid_11_kd",  0, 1, 0, -8192, 8191 },

    { /* pid_NN_enable - Enables/closes or disables/open PID NN loop:
       *    0 - PID disabled (open loop)
       *    1 - PID enabled (closed loop)    */
        "pid_12_enable", 0, 1, 0, 0, 1 },
    { /* pid_NN_rst - Reset PID NN integrator:
        *    0 - Do not reset integrator
        *    1 - Reset integrator            */
        "pid_12_rst", 0, 1, 0, 0, 1 },
    { /* pid_NN_sp - PID NN set-point in [ADC] counts. */
        "pid_12_sp",  0, 1, 0, -8192, 8191 },
    { /* pid_NN_kp - PID NN proportional gain Kp in [ADC] counts. */
        "pid_12_kp",  0, 1, 0, -8192, 8191 },
    { /* pid_NN_ki - PID NN integral gain     Ki in [ADC] counts. */
        "pid_12_ki",  0, 1, 0, -8192, 8191 },
    { /* pid_NN_kd - PID NN derivative gain   Kd in [ADC] counts. */
        "pid_12_kd",  0, 1, 0, -8192, 8191 },

    { /* pid_NN_enable - Enables/closes or disables/open PID NN loop:
       *    0 - PID disabled (open loop)
       *    1 - PID enabled (closed loop)    */
        "pid_21_enable", 0, 1, 0, 0, 1 },
    { /* pid_NN_rst - Reset PID NN integrator:
        *    0 - Do not reset integrator
        *    1 - Reset integrator            */
        "pid_21_rst", 0, 1, 0, 0, 1 },
    { /* pid_NN_sp - PID NN set-point in [ADC] counts. */
        "pid_21_sp",  0, 1, 0, -8192, 8191 },
    { /* pid_NN_kp - PID NN proportional gain Kp in [ADC] counts. */
        "pid_21_kp",  0, 1, 0, -8192, 8191 },
    { /* pid_NN_ki - PID NN integral gain     Ki in [ADC] counts. */
        "pid_21_ki",  0, 1, 0, -8192, 8191 },
    { /* pid_NN_kd - PID NN derivative gain   Kd in [ADC] counts. */
        "pid_21_kd",  0, 1, 0, -8192, 8191 },

    { /* pid_NN_enable - Enables/closes or disables/open PID NN loop:
       *    0 - PID disabled (open loop)
       *    1 - PID enabled (closed loop)    */
        "pid_22_enable", 0, 1, 0, 0, 1 },
    { /* pid_NN_rst - Reset PID NN integrator:
        *    0 - Do not reset integrator
        *    1 - Reset integrator            */
        "pid_22_rst", 0, 1, 0, 0, 1 },
    { /* pid_NN_sp - PID NN set-point in [ADC] counts. */
        "pid_22_sp",  0, 1, 0, -8192, 8191 },
    { /* pid_NN_kp - PID NN proportional gain Kp in [ADC] counts. */
        "pid_22_kp",  0, 1, 0, -8192, 8191 },
    { /* pid_NN_ki - PID NN integral gain     Ki in [ADC] counts. */
        "pid_22_ki",  0, 1, 0, -8192, 8191 },
    { /* pid_NN_kd - PID NN derivative gain   Kd in [ADC] counts. */
        "pid_22_kd",  0, 1, 0, -8192, 8191 },

    { /* Must be last! */
        NULL, 0.0, -1, -1, 0.0, 0.0 }     
};
/* params initialized */
static int params_init = 0;

/* AUTO set algorithm in progress flag */
int auto_in_progress = 0;

rp_calib_params_t rp_main_calib_params;

int forcex_state = 0;
float forced_xmin = 0;
float forced_xmax = 0;
float forced_units = 0;
float forced_delay = 0;


const char *rp_app_desc(void)
{
    return (const char *)"Red Pitaya osciloscope application.\n";
}

int rp_app_init(void)
{
    fprintf(stderr, "Loading scope (with gen+pid extensions) version %s-%s.\n", VERSION_STR, REVISION_STR);

    rp_default_calib_params(&rp_main_calib_params);
    if(rp_read_calib_params(&rp_main_calib_params) < 0) {
        fprintf(stderr, "rp_read_calib_params() failed, using default"
                " parameters\n");
    }
    if(rp_osc_worker_init(&rp_main_params[0], PARAMS_NUM, 
                          &rp_main_calib_params) < 0) {
        return -1;
    }
    if(generate_init(&rp_main_calib_params) < 0) {
        return -1;
    }
    if(pid_init() < 0) {
        return -1;
    }

    rp_set_params(&rp_main_params[0], PARAMS_NUM);


    return 0;
}

int rp_app_exit(void)
{
    fprintf(stderr, "Unloading scope (with gen+pid extensions) version %s-%s.\n", VERSION_STR, REVISION_STR);

    rp_osc_worker_exit();
    generate_exit();
    pid_exit();

    return 0;
}

int time_range_to_time_unit(int range)
{
    int unit = 2;
    
    switch (range) {
    case 0:
    case 1:
        unit = 0;
        break;
    case 2:
    case 3:
        unit = 1;
        break;
    default:
        unit = 2;
    }

    return unit;
}

/* Find a suitable FPGA decimation factor and trigger delay,
 * based on xmin & xmax zoom conntrols
 */
int transform_acq_params(rp_app_params_t *p)
{
    TRACE("%s()\n", __FUNCTION__);

    int ret = 0;
    int i;

    /* Skip the transform in case auto-set is in progress */
    if ( (p[AUTO_FLAG_PARAM].value == 1) || (auto_in_progress == 1)) {
        return ret;
    }

    double xmin = p[MIN_GUI_PARAM].value;
    double xmax = p[MAX_GUI_PARAM].value;

    float ratio;

    int reset_zoom = 0;

    int time_unit = p[TIME_UNIT_PARAM].value;
    float t_unit_factor = pow(10, 3*(2 - time_unit));

    /* When exactly this pair is provided by client, Reset Zoom is requested. */
    if ((xmax == 1.0e6) && (xmin == -1.0e6)) {
        reset_zoom = 1;
    }

    /* Server ForceX state */
    p[FORCEX_FLAG_PARAM].value = (float) forcex_state;

    /* Difference (expressed as ratio) between forced values and GUI state */
    if ((xmax - xmin) != 0) {
        ratio = fabs(forced_xmax - forced_xmin) / fabs(xmax - xmin);
    } else {
        ratio = 0.0;
    }

    /* Make it always between 0 and 1   (0: very different, 1 equal) */
    if (ratio > 1) {
        ratio = 1.0 / ratio;
    }

    /* Stop forcing if factor 33 of difference or less */
    if (ratio > 0.03) {
        p[FORCEX_FLAG_PARAM].value  = 0;
        forcex_state = 0;
    }

    /* Contver GUI values to seconds */
    xmin /= t_unit_factor;
    xmax /= t_unit_factor;

    TRACE("TR: Xmin, Xmax: %10.8f, %10.8f\n", xmin, xmax);

    int time_unit_gui = time_unit;

    int dec;
    double rdec;

    /* Calculate the suitable FPGA decimation setting that optimally covers the GUI time frame */
    if (p[TRIG_MODE_PARAM].value == 0) {
        /* Autotriggering mode => acquisition starts at time t = 0 */
        rdec = (xmax - 0) * c_osc_fpga_smpl_freq / OSC_FPGA_SIG_LEN;
    } else {
        double rxmax = (xmax < 0) ? 0 : xmax;
        rdec = (rxmax - xmin) * c_osc_fpga_smpl_freq / OSC_FPGA_SIG_LEN;
    }

    /* Find optimal decimation setting */
    for (i = 0; i < 6; i++) {
        dec = osc_fpga_cnv_time_range_to_dec(i);
        if (dec >= rdec) {
            break;
        }
    }
    if (i > 5)
        i = 5;

    /* Apply decimation parameter (time range), but not when forcing GUI client
     * or during reset zoom.
     */
    if ((forcex_state == 0) && (reset_zoom == 0)) {
        p[TIME_RANGE_PARAM].value = i;
    }

    TRACE("TR: Dcimation: %6.2f -> %dx\n", rdec, dec);

    /* New time_unit & factor */
    time_unit = time_range_to_time_unit(p[TIME_RANGE_PARAM].value);
    t_unit_factor = pow(10, 3*(2 - time_unit));

    /* Update time unit Min and Max, but not if GUI hasn't responded to "forceX" command. */
    if (forcex_state == 0) {
        p[MIN_GUI_PARAM].value = xmin * t_unit_factor;
        p[MAX_GUI_PARAM].value = xmax * t_unit_factor;
        p[GUI_XMIN].value = p[MIN_GUI_PARAM].value;
        p[GUI_XMAX].value = p[MAX_GUI_PARAM].value;
        p[TIME_UNIT_PARAM].value = time_unit;
    } else {
        p[MIN_GUI_PARAM].value = forced_xmin;
        p[MAX_GUI_PARAM].value = forced_xmax;
        p[GUI_XMIN].value = p[MIN_GUI_PARAM].value;
        p[GUI_XMAX].value = p[MAX_GUI_PARAM].value;
        p[TIME_UNIT_PARAM].value = forced_units;
    }

    /* If time units have changed by server: client MUST configure x axis
     * (ForceX is set for this purpose by server) to p[MIN_GUI_PARAM].value,
     * expressed in new units.
     */

    TRACE("TR: New xmin, xmax [unit]: %6.2f  %6.2f [%d]\n",
            p[MIN_GUI_PARAM].value,
            p[MAX_GUI_PARAM].value,
            (int)p[TIME_UNIT_PARAM].value);

    int64_t t_delay;

    /* Calculate necessary trigger delay expressed in FPGA decimated cycles */
    if (p[TRIG_MODE_PARAM].value == 0) {
        /* Autotriggering mode => acquisition starts at time t = 0 */
        t_delay= OSC_FPGA_SIG_LEN ;
    } else {
        t_delay= OSC_FPGA_SIG_LEN + (xmin * c_osc_fpga_smpl_freq / dec);
    }

    /* Trigger delay limitations/saturation */
    const int64_t c_max_t_delay = ((int64_t)1 << 32) - 1;
    if (t_delay < 0)
        t_delay = 0;
    if (t_delay > c_max_t_delay)
        t_delay = c_max_t_delay;

    /* Trigger delay (reconverted in seconds) updated ONLY if client has responded to
     * last forceX command.
     */
    if (forcex_state == 0) {
        p[TRIG_DLY_PARAM].value = ((t_delay - OSC_FPGA_SIG_LEN) * dec / c_osc_fpga_smpl_freq);
    } else {
        p[TRIG_DLY_PARAM].value = forced_delay;
    }

    /* Server issues a forceX command when time units change wrt. GUI (client) units */
    if ((time_unit != time_unit_gui)) {
        p[FORCEX_FLAG_PARAM].value = 1.0;
        forcex_state = 1;

        /* Other settings frozen until GUI recovers */
        forced_xmin = p[MIN_GUI_PARAM].value;
        forced_xmax = p[MAX_GUI_PARAM].value;
        forced_units = p[TIME_UNIT_PARAM].value;
        forced_delay = p[TRIG_DLY_PARAM].value;
    }

    /* When client issues a zoom reset, a particular ForceX command with
     * the initial 0 - 130 us time range.
     */
    if (reset_zoom == 1) {
        p[FORCEX_FLAG_PARAM].value  = 1.0;
        forcex_state = 1;

        forced_xmin = 0.0;
        forced_xmax = 130.0;
        forced_units = 0.0;
        forced_delay = 0;

        p[MIN_GUI_PARAM].value = forced_xmin;
        p[MAX_GUI_PARAM].value = forced_xmax;
        p[GUI_XMIN].value = p[MIN_GUI_PARAM].value;
        p[GUI_XMAX].value = p[MAX_GUI_PARAM].value;
        p[TIME_UNIT_PARAM].value = forced_units;
        p[TRIG_DLY_PARAM].value = forced_delay;
        p[TIME_RANGE_PARAM].value = 0;
    }

    TRACE("TR: Trigger delay: %.6f\n", p[TRIG_DLY_PARAM].value);

    return ret;
}

void get_scales(rp_app_params_t *p, float *scale1, float *scale2, float *maxv) {

    /* Max ADC for Ch1, Ch2, both combined, normalized & selected */
    uint32_t fe_fsg1 = (p[GAIN_CH1].value == 0) ?
            rp_main_calib_params.fe_ch1_fs_g_hi :
            rp_main_calib_params.fe_ch1_fs_g_lo;
    float ch1_max_adc_v =
            osc_fpga_calc_adc_max_v(fe_fsg1, p[PRB_ATT_CH1].value);

    uint32_t fe_fsg2 = (p[GAIN_CH2].value == 0) ?
            rp_main_calib_params.fe_ch2_fs_g_hi :
            rp_main_calib_params.fe_ch2_fs_g_lo;
    float ch2_max_adc_v =
            osc_fpga_calc_adc_max_v(fe_fsg2, p[PRB_ATT_CH2].value);

    float max_adc_norm = osc_fpga_calc_adc_max_v(rp_main_calib_params.fe_ch1_fs_g_hi, 0);

    *scale1 = ch1_max_adc_v / max_adc_norm;
    *scale2 = ch2_max_adc_v / max_adc_norm;
    *maxv = (ch1_max_adc_v > ch2_max_adc_v) ?
             ch1_max_adc_v : ch2_max_adc_v;
}

void transform_to_iface_units(rp_app_params_t *p)
{
    float scale, scale1, scale2, maxv;
    get_scales(p, &scale1, &scale2, &maxv);
    scale = (scale1 > scale2) ? scale1 : scale2;

    /* Re-calculate output parameters */
    p[GUI_RST_Y_RANGE].value = 2.0 * maxv;

    p[MIN_Y_PARAM].value = p[MIN_Y_NORM].value * scale;
    p[MAX_Y_PARAM].value = p[MAX_Y_NORM].value * scale;

    p[GEN_DC_OFFS_1].value = p[GEN_DC_NORM_1].value * scale1;
    p[GEN_DC_OFFS_2].value = p[GEN_DC_NORM_2].value * scale2;

    p[SCALE_CH1].value = scale1;
    p[SCALE_CH2].value = scale2;
}

void transform_from_iface_units(rp_app_params_t *p)
{
    float scale1, scale2, maxv;
    get_scales(p, &scale1, &scale2, &maxv);

    /* Re-calculate input parameters */
    p[GEN_DC_NORM_1].value = p[GEN_DC_OFFS_1].value / scale1;
    p[GEN_DC_NORM_2].value = p[GEN_DC_OFFS_2].value / scale2;
}

int rp_set_params(rp_app_params_t *p, int len)
{
    int i;
    int fpga_update = 1;
    int params_change = 0;
    int awg_params_change = 0;
    int pid_params_change = 0;
    
    TRACE("%s()\n", __FUNCTION__);

    if(len > PARAMS_NUM) {
        fprintf(stderr, "Too many parameters, max=%d\n", PARAMS_NUM);
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
            if(p_idx < PARAMS_AWG_PARAMS) 
                params_change = 1;
            if ( (p_idx >= PARAMS_AWG_PARAMS) && (p_idx < PARAMS_PID_PARAMS) )
                awg_params_change = 1;
            if(p_idx >= PARAMS_PID_PARAMS)
                pid_params_change = 1;
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
    transform_from_iface_units(&rp_main_params[0]);
    pthread_mutex_unlock(&rp_main_params_mutex);
    

    /* Set parameters in HW/FPGA only if they have changed */
    if(params_change || (params_init == 0)) {

        pthread_mutex_lock(&rp_main_params_mutex);
        /* Xmin & Xmax public copy to be served to clients */
        rp_main_params[GUI_XMIN].value = p[MIN_GUI_PARAM].value;
        rp_main_params[GUI_XMAX].value = p[MAX_GUI_PARAM].value;
        transform_acq_params(rp_main_params);
        pthread_mutex_unlock(&rp_main_params_mutex);

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

        /* Our time window with current settings:
         *   - time_delay is added later, when we check if it is correct 
         *     setting 
         */
        float t_min = 0;
        float t_max = ((OSC_FPGA_SIG_LEN-1) * smpl_period);
        float t_max_minus = ((OSC_FPGA_SIG_LEN-6) * smpl_period);

        params_init = 1;
        /* in time units time_unit, needs to be converted */
        float t_start = rp_main_params[MIN_GUI_PARAM].value;
        float t_stop  = rp_main_params[MAX_GUI_PARAM].value;
        int t_start_idx;
        int t_stop_idx;
        int t_step_idx = 0;

        /* If auto-set algorithm was requested do not set other parameters */
        if(rp_main_params[AUTO_FLAG_PARAM].value == 1) {
            auto_in_progress = 1;
            forcex_state = 0;

            rp_osc_clean_signals();
            rp_osc_worker_change_state(rp_osc_auto_set_state);
            /* AUTO_FLAG_PARAM is cleared when Auto-set algorithm finishes */
            
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

            auto_in_progress = 0;

            return 0;
        }

        /* If AUTO trigger mode, reset trigger delay */
        if(mode == 0) 
            t_delay = 0;

        if(dec_factor < 0) {
            fprintf(stderr, "Incorrect time range: %d\n", time_range);
            return -1;
        }

        /* Pick time unit and unit factor corresponding to current time range. */
        if((time_range == 0) || (time_range == 1)) {
            time_unit     = 0;
            t_unit_factor = 1e6;
        } else if((time_range == 2) || (time_range == 3)) {
            time_unit     = 1;
            t_unit_factor = 1e3;
        }

        rp_main_params[TIME_UNIT_PARAM].value = time_unit;
        TRACE("PC: time_(R,U) = (%d, %d)\n", time_range, time_unit);

        /* Check if trigger delay in correct range, otherwise correct it
         * Correct trigger delay is:
         *  t_delay >= -t_max_minus
         *  t_delay <= OSC_FPGA_MAX_TRIG_DELAY
         */
        if(t_delay < -t_max_minus) {
            t_delay = -t_max_minus;
        } else if(t_delay > (OSC_FPGA_TRIG_DLY_MASK * smpl_period)) {
            t_delay = OSC_FPGA_TRIG_DLY_MASK * smpl_period;
        } else {
            t_delay = round(t_delay / smpl_period) * smpl_period;
        }
        t_min = t_min + t_delay;
        t_max = t_max + t_delay;
        rp_main_params[TRIG_DLY_PARAM].value = t_delay;

        /* Convert to seconds */
        t_start = t_start / t_unit_factor;
        t_stop  = t_stop  / t_unit_factor;
        TRACE("PC: t_stop = %.9f\n", t_stop);

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

        if((((t_stop_idx-t_start_idx)/(float)(((int)rp_get_params_bode(5))-1))) >= 1) {
            t_step_idx = ceil((t_stop_idx-t_start_idx)/(float)(((int)rp_get_params_bode(5))-1));
            int max_step = OSC_FPGA_SIG_LEN/((int)rp_get_params_bode(5));
            if(t_step_idx > max_step)
                t_step_idx = max_step;

            t_stop = t_start + ((int)rp_get_params_bode(5)) * t_step_idx * smpl_period;
        }

        TRACE("PC: t_stop (rounded) = %.9f\n", t_stop);

        /* write back and convert to set units */
        rp_main_params[MIN_GUI_PARAM].value = t_start;
        rp_main_params[MAX_GUI_PARAM].value = t_stop;

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

    if(awg_params_change) {

        /* Correct frequencies if needed */
        rp_main_params[GEN_SIG_FREQ_CH1].value = 
            rp_gen_limit_freq(rp_main_params[GEN_SIG_FREQ_CH1].value,
                              rp_main_params[GEN_SIG_TYPE_CH1].value);
        rp_main_params[GEN_SIG_FREQ_CH2].value = 
            rp_gen_limit_freq(rp_main_params[GEN_SIG_FREQ_CH2].value,
                              rp_main_params[GEN_SIG_TYPE_CH2].value);
        if(generate_update(&rp_main_params[0]) < 0) {
            return -1;
        }
    }

    if (pid_params_change) {
        if(pid_update(&rp_main_params[0]) < 0) {
            return -1;
        }
    }

    return 0;
}

/* Returned vector must be free'd externally! */
int rp_get_params(rp_app_params_t **p)
{
    rp_app_params_t *p_copy = NULL;
    int i;

    p_copy = (rp_app_params_t *)malloc((PARAMS_NUM+1) * sizeof(rp_app_params_t));
    if(p_copy == NULL)
        return -1;

    pthread_mutex_lock(&rp_main_params_mutex);
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

    /* Return the original public Xmin & Xmax to client (not the internally modified ones). */
    p_copy[MIN_GUI_PARAM].value = p_copy[GUI_XMIN].value;
    p_copy[MAX_GUI_PARAM].value = p_copy[GUI_XMAX].value;

    transform_to_iface_units(p_copy);

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
    *sig_len = ((int)rp_get_params_bode(5));

    ret_val = rp_osc_get_signals(s, &sig_idx);

    /* Not finished signal */
    if((ret_val != -1) && sig_idx != ((int)rp_get_params_bode(5))-1) {
        return -2;
    }
    /* Old signal */
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
        s[i] = (float *)malloc(((int)rp_get_params_bode(5)) * sizeof(float));
        if(s[i] == NULL) {
            rp_cleanup_signals(a_signals);
            return -1;
        }
        memset(&s[i][0], 0, ((int)rp_get_params_bode(5)) * sizeof(float));
    }
    *a_signals = s;

    return 0;
}

void rp_cleanup_signals(float ***a_signals)
{
    int i;
    float **s = *a_signals;

    if(s) {
        for(i = 0; i < SIGNALS_NUM; i++) {
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

int rp_update_meas_data(rp_osc_meas_res_t ch1_meas, rp_osc_meas_res_t ch2_meas)
{
    pthread_mutex_lock(&rp_main_params_mutex);
    rp_main_params[MEAS_MIN_CH1].value = ch1_meas.min;
    rp_main_params[MEAS_MAX_CH1].value = ch1_meas.max;
    rp_main_params[MEAS_AMP_CH1].value = ch1_meas.amp;
    rp_main_params[MEAS_AVG_CH1].value = ch1_meas.avg;
    rp_main_params[MEAS_FREQ_CH1].value = ch1_meas.freq;
    rp_main_params[MEAS_PER_CH1].value = ch1_meas.period;

    rp_main_params[MEAS_MIN_CH2].value = ch2_meas.min;
    rp_main_params[MEAS_MAX_CH2].value = ch2_meas.max;
    rp_main_params[MEAS_AMP_CH2].value = ch2_meas.amp;
    rp_main_params[MEAS_AVG_CH2].value = ch2_meas.avg;
    rp_main_params[MEAS_FREQ_CH2].value = ch2_meas.freq;
    rp_main_params[MEAS_PER_CH2].value = ch2_meas.period;

    pthread_mutex_unlock(&rp_main_params_mutex);
    return 0;
}

float rp_gen_limit_freq(float freq, float gen_type)
{
    int type = (int)gen_type;

    if(freq < 0) {
        freq = 0;
    } else {
        switch(type) {
        case 0:
            /* Sine */
            if(freq > 50e6)
                freq = 50e6;
            break;
        case 1:
            /* Square */
            if(freq > 20e6)
                freq = 20e6;
            break;
        case 2:
            /* Triangle */
            if(freq > 25e6)
                freq = 25e6;
            break;
        }
    }

    return freq;
}

float rp_get_params_bode(int pos){
  switch(pos){
    case 0:
      return rp_main_params[BODE_S_MEASURE].value;
    case 1:
      return rp_main_params[BODE_GEN_AMP].value;
    case 2:
      return rp_main_params[BODE_GEN_AVG].value;
    case 3:
      return rp_main_params[BODE_GEN_DC_BIAS].value;
    case 4:
      return rp_main_params[BODE_STAR_FREQ].value;
    case 5:
      return rp_main_params[BODE_COUNTS].value;
    case 6:
      return rp_main_params[DATA_PLOT].value;
    case 7:
      return rp_main_params[BODE_SCALE_TYPE].value;
    case 8:
      return rp_main_params[BODE_DATA_PLOT].value;
    case 9:
      return rp_main_params[BODE_END_FREQ].value;
    default:
      return -1;
  }
}

void rp_set_params_bode(int pos, float val){
  switch(pos){
    case 0:
      rp_main_params[BODE_S_MEASURE].value = val;
      break;
  }
}