/**
 * $Id: main.c 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Power analyzer main module.
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
#include <time.h>

#include "main.h"
#include "version.h"
#include "worker.h"
#include "fpga.h"
#include "calib.h"
#include "house_kp.h" 

/* Describe app. parameters with some info/limitations */
pthread_mutex_t rp_main_params_mutex = PTHREAD_MUTEX_INITIALIZER;
static rp_app_params_t rp_main_params[PARAMS_NUM+1] = {
    { /* min_gui_time   */ 
        //"xmin", -1000000, 1, 0, -10000000, +10000000 },
        "xmin", 0, 1, 0, -10000000, +10000000 },
    { /* max_gui_time   */ 
        //"xmax", +1000000, 1, 0, -10000000, +10000000 },
        "xmax", 134.2, 1, 0, -10000000, +10000000 },
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
        "trig_level", 0, 1, 0,     -10000,     +10000 },
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
       *    5 - 65kx 
       *    6 - 1x DC*/
        "time_range", 3, 1, 0,         0,         6 },
    { /* time_unit_used:
       *    0 - [us]
       *    1 - [ms]
       *    2 - [s]     */
        "time_units", 1, 0, 1,         0,         2 },
    { /* en_avg_at_dec:
           *    0 - disable
           *    1 - enable */
        "en_avg_at_dec", 1, 1, 0,      0,         1 },
    { /* auto_flag: 
       * Puts the controller to auto mode - the algorithm which detects input
       * signal and changes the parameters to most fit the input:
       *    0 - normal operation
       *    1 - auto button pressed */
        "auto_flag", 0, 1, 0, 0, 1 },
    { /* min_y, max_y - Controller defined Y range when using auto-set or after
       * gain change y range */
        "min_y_U", 0, 0, 0, -1500, +1500 },
    { /* min_y, max_y - Controller defined Y range when using auto-set or after
       * gain change y range */
        "max_y_U", 0, 0, 0, -1500, +1500 },
    { /* min_y, max_y - Controller defined Y range when using auto-set or after
       * gain change y range */
        "min_y_I", 0, 0, 0, -1000, +1000 },
    { /* min_y, max_y - Controller defined Y range when using auto-set or after
       * gain change y range */
        "max_y_I", 0, 0, 0, -1000, +1000 },
    { /* min_y, max_y - Controller defined Y range when using auto-set or after
       * gain change y range */
        "min_y_P", 0, 0, 0, -1000000, +1000000 },
    { /* min_y, max_y - Controller defined Y range when using auto-set or after
       * gain change y range */
        "max_y_P", 0, 0, 0, -1000000, +1000000 },
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
    {  "meas_min_U", 0, 0, 1, -1e9, +1e9 },
    {  "meas_max_U", 0, 0, 1, +1e9, -1e9 },
    {  "meas_amp_U", 0, 0, 1, +1e9, -1e9 },
    {  "meas_avg_U", 0, 0, 1, +1e9, -1e9 },
    {  "meas_min_I", 0, 0, 1, -1000, +1000 },
    {  "meas_max_I", 0, 0, 1, +1000, -1000 },
    {  "meas_amp_I", 0, 0, 1, +1000, -1000 },
    {  "meas_avg_I", 0, 0, 1, +1000, -1000 },
    {  "meas_U_ef", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I_ef", 0, 0, 1, -1000, +1000 },
    {  "meas_p", 0, 0, 1, -1e9, +1e9 },
    {  "meas_q", 0, 0, 1, -1e9, +1e9 },
    {  "meas_s", 0, 0, 1, -1e9, +1e9 },
    {  "meas_p_1", 0, 0, 1, -1e9, +1e9 },
    {  "meas_q_1", 0, 0, 1, -1e9, +1e9 },
    {  "meas_p_h", 0, 0, 1, -1e9, +1e9 },
    {  "meas_q_h1", 0, 0, 1, -1e9, +1e9 },
    {  "meas_q_h2", 0, 0, 1, -1e9, +1e9 },
    {  "meas_q_2", 0, 0, 1, -1e9, +1e9 },
    {  "meas_freq", 0, 0, 1, 0, +1e9 },
    {  "meas_cos_fi_1", 0, 0, 1, -1, +1 },
    {  "meas_pf", 0, 0, 1, -1, +1 },
    {  "meas_fft_u_1", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fft_i_1", 0, 0, 1, -1e9, +1e9 },
    {  "meas_sum_u_h", 0, 0, 1, -1e9, +1e9 },
    {  "meas_sum_i_h", 0, 0, 1, -1e9, +1e9 },
    {  "meas_thd_u", 0, 0, 1, -1e9, +1e9 },
    {  "meas_thd_i", 0, 0, 1, -1e9, +1e9 },
    { /* diff_prb_att - User diff. probe attenuation setting for voltage:
       *    0 - 20x
       *    1 - 200x 
       *    2 - other */
        "diff_prb_att", 0, 1, 0, 0, 2 },
    { /* gain_ch1 - User jumper gain setting for channel 1:
       *    0 - high gain (0.6 [V] Full-scale)
       *    1 - low gain (15 [V] Full-scale) */
        "gain_ch1", 1, 1, 0, 0, 1 },
    { /* curr_probe_select - User current transducer selection:
       *    0 - none
       *    1 - 300 A
       *    2 - 150 A 
       *    3 - 100 A
       * 	4 - 25 A */
        "curr_probe_select", 0, 1, 0, 0, 4 },
    { /* gain_ch2 - User jumper gain setting for channel 2:
       *    0 - high gain (0.6 [V] Full-scale)
       *    1 - low gain (15 [V] Full-scale) */
        "gain_ch2", 1, 1, 0, 0, 1 },
    { /* gui_reset_y_range_U - Maximum voltage range [Vpp] with current settings
       * This parameter is calculated by application and is read-only for
       * client.
       */
        "gui_reset_y_range_U", 0, 0, 1, 0, 50000 },
    { /* gui_reset_y_range_I - Maximum current range [Ipp] with current settings
       * This parameter is calculated by application and is read-only for
       * client.
       */
        "gui_reset_y_range_I", 0, 0, 1, 0, 10000 },
    { /* gui_reset_y_range - Maximum power range [Ppp] with current settings
       * This parameter is calculated by application and is read-only for
       * client.
       */
        "gui_reset_y_range_P", 0, 0, 1,0, 1e9 },
    { /* gen_DC_offs_1 - DC offset for channel 1 expressed in [V] requested by 
       * GUI */
        "gen_DC_offs_1", 0, 1, 0, -100, 100 },
    { /* gen_DC_offs_2 - DC offset for channel 2 expressed in [V] requested by 
       * GUI */
        "gen_DC_offs_2", 0, 1, 0, -100, 100 },
    { /* gui_xmin - Xmin as specified by GUI - not rounded to sampling engine quanta. */
        "gui_xmin",      0, 0, 1, -10000000, +10000000 },
    { /* gui_xmax - Xmax as specified by GUI - not rounded to sampling engine quanta. */
        "gui_xmax",    134.2, 0, 1, -10000000, +10000000 },
    { /* min_y_norm, max_y_norm - Normalized controller defined Y range when using auto-set */
        "min_y_norm_U", 0, 0, 0, -1000000, +1000000 },
    { /* min_y_norm, max_y_norm - Normalized controller defined Y range when using auto-set */
        "max_y_norm_U", 0, 0, 0, -1000000, +1000000 },
    { /* min_y_norm, max_y_norm - Normalized controller defined Y range when using auto-set */
        "min_y_norm_I", 0, 0, 0, -1000, +1000 },
    { /* min_y_norm, max_y_norm - Normalized controller defined Y range when using auto-set */
        "max_y_norm_I", 0, 0, 0, -1000, +1000 },
    { /* min_y_norm, max_y_norm - Normalized controller defined Y range when using auto-set */
        "min_y_norm_P", 0, 0, 0, -1e9, +1e9 },
    { /* min_y_norm, max_y_norm - Normalized controller defined Y range when using auto-set */
        "max_y_norm_P", 0, 0, 0, -1e9, +1e9 },
    { /* gen_DC_norm_1 - DC offset for channel 1 expressed in normalized 1V */
        "gen_DC_norm_1", 0, 1, 0, -100, 100 },
    { /* gen_DC_norm_2 - DC offset for channel 2 expressed in normalized 1V */
        "gen_DC_norm_2", 0, 1, 0, -100, 100 },
    { /* scale_ch1 - Jumper & probe attenuation dependent Y scaling factor for Channel 1 (U) */
        "scale_ch1", 1, 0, 0, -1000, 1000 },
    { /* scale_ch2 - Jumper & probe factor dependent Y scaling factor for Channel 2 (I) */
        "scale_ch2", 1, 0, 0, -1000, 1000 }, 
    { /* number of harmonics for thd calculation */
		"harm_num", 99, 0, 0, 2, 99 },
    { /* start writing params to file flag */  
		"write_params", 0, 0, 0, 0, 1 },
    /*harmonics from here on*/
    {  "meas_U1", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U2", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U3", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U4", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U5", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U6", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U7", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U8", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U9", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U10", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U11", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U12", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U13", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U14", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U15", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U16", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U17", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U18", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U19", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U20", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U21", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U22", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U23", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U24", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U25", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U26", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U27", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U28", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U29", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U30", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U31", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U32", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U33", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U34", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U35", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U36", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U37", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U38", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U39", 0, 0, 1, -1e9, +1e9 },
    {  "meas_U40", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I1", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I2", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I3", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I4", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I5", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I6", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I7", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I8", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I9", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I10", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I11", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I12", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I13", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I14", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I15", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I16", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I17", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I18", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I19", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I20", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I21", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I22", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I23", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I24", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I25", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I26", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I27", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I28", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I29", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I30", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I31", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I32", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I33", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I34", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I35", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I36", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I37", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I38", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I39", 0, 0, 1, -1e9, +1e9 },
    {  "meas_I40", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU2", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU3", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU4", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU5", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU6", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU7", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU8", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU9", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU10", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU11", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU12", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU13", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU14", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU15", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU16", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU17", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU18", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU19", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU20", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU21", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU22", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU23", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU24", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU25", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU26", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU27", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU28", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU29", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU30", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU31", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU32", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU33", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU34", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU35", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU36", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU37", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU38", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU39", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiU40", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI2", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI3", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI4", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI5", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI6", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI7", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI8", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI9", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI10", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI11", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI12", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI13", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI14", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI15", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI16", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI17", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI18", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI19", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI20", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI21", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI22", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI23", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI24", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI25", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI26", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI27", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI28", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI29", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI30", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI31", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI32", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI33", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI34", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI35", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI36", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI37", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI38", 0, 0, 1, -1e9, +1e9 },
    {  "meas_fiI39", 0, 0, 1, -1e9, +1e9 },   
    {  "meas_fiI40", 0, 0, 1, -1e9, +1e9 },             
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

int write_in_progress = 0;
int new_file = 1;
FILE *fp;


const char *rp_app_desc(void)
{
    return (const char *)"Red Pitaya power analyzer application.\n";
}

int rp_app_init(void)
{
    fprintf(stderr, "Loading power analyzer version %s-%s.\n", VERSION_STR, REVISION_STR);
    
    rp_default_calib_params(&rp_main_calib_params);
    if(rp_read_calib_params(&rp_main_calib_params) < 0) {
        fprintf(stderr, "rp_read_calib_params() failed, using default"
                " parameters\n");
    }
    
    if(rp_pwr_worker_init(&rp_main_params[0], PARAMS_NUM, 
                          &rp_main_calib_params) < 0) {
        return -1;
    }

    rp_set_params(&rp_main_params[0], PARAMS_NUM);


    return 0;
}

int rp_app_exit(void)
{
    fprintf(stderr, "Unloading power analyzer version %s-%s.\n", VERSION_STR, REVISION_STR);

    rp_pwr_worker_exit();

    return 0;
}

int time_range_to_time_unit(int range)
{
    int unit = 2;
    
    switch (range) {
    case 0:
    case 1:
    case 6:
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

float probe_select_to_factor(int probe, int fact_change)
{	
	float k0_init = 1;
	static float k0_change;
	float k1_init = 50.32;
	static float k1_change;
	float k2_init = 19.99;
	static float k2_change;
	float k3_init = 10.06;
	static float k3_change;
	float k4_init = 5.015;
	static float k4_change;
	
	static int changed0;
	static int changed1;
	static int changed2;
	static int changed3;
	static int changed4;
	
	float k;
	
	if(fact_change) {
		switch (probe) {
			case 0: 
				k0_change = rp_main_params[SCALE_CH2].value;
				changed0 = 1;
				break;
			case 1:
				k1_change = rp_main_params[SCALE_CH2].value;
				changed1 = 1;
				break;
			case 2:
				k2_change = rp_main_params[SCALE_CH2].value;
				changed2 = 1;
				break;
			case 3:
				k3_change = rp_main_params[SCALE_CH2].value;
				changed3 = 1;
				break;
			case 4:
				k4_change = rp_main_params[SCALE_CH2].value;
				changed4 = 1;
				break;
			default:
				k = 1;
		}
	}
	
	switch (probe) {
	    case 0: 
	        if(changed0 == 1) {
				k = k0_change;
				break;
			} else {
		        k = k0_init;
		        break;
		    }
	    case 1:
	        if(changed1 == 1) {
				k = k1_change;
				break;
			} else {
		        k = k1_init;
		        break;
		    }
	    case 2:
	        if(changed2 == 1) {
				k = k2_change;
				break;
			} else {
		        k = k2_init;
		        break;
			}
	    case 3:
	        if(changed3 == 1) {
				k = k3_change;
				break;
			} else {
		        k = k3_init;
		        break;
		    }
	    case 4:
	        if(changed4 == 1) {
				k = k4_change;
				break;
			} else {
		        k = k4_init;
		        break;
			}
	    default:
		    k = 1;
	}
	
	return k;
}

float voltage_probe_factor_select(int fact_select, int fact_change)
{	
	float k0_init = 20;
	static float k0_change;
	float k1_init = 200;
	static float k1_change;
	float k2_init = 1;
	static float k2_change;
	
	static int changed0;
	static int changed1;
	static int changed2;
	
	float k;
	
	if(fact_change) {
		switch (fact_select) {
			case 0: 
				k0_change = rp_main_params[SCALE_CH1].value;
				changed0 = 1;
				break;
			case 1:
				k1_change = rp_main_params[SCALE_CH1].value;
				changed1 = 1;
				break;
			case 2:
				k2_change = rp_main_params[SCALE_CH1].value;
				changed2 = 1;
				break;
			default:
				k = 1;
		}
	}
	
	switch (fact_select) {
	    case 0: 
	        if(changed0 == 1) {
				k = k0_change;
				break;
			} else {
		        k = k0_init;
		        break;
		    }
	    case 1:
	        if(changed1 == 1) {
				k = k1_change;
				break;
			} else {
		        k = k1_init;
		        break;
		    }
	    case 2:
	        if(changed2 == 1) {
				k = k2_change;
				break;
			} else {
		        k = k2_init;
		        break;
			}
	    default:
		    k = 1;
	}
	
	return k;
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
    

    int time_unit = time_range_to_time_unit(p[TIME_RANGE_PARAM].value);
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

    //int time_unit_gui = time_unit;

    int dec;
    double rdec;

    /* Calculate the suitable FPGA decimation setting that optimally covers the GUI time frame */
    if (p[TRIG_MODE_PARAM].value == 0) {
        /* Autotriggering mode => acquisition starts at time t = 0 */
        rdec = (xmax - 0) * c_pwr_fpga_smpl_freq / PWR_FPGA_SIG_LEN;
    } else {
        double rxmax = (xmax < 0) ? 0 : xmax;
        rdec = (rxmax - xmin) * c_pwr_fpga_smpl_freq / PWR_FPGA_SIG_LEN;
    }

    /* Find optimal decimation setting */
    for (i = 0; i < 6; i++) {
        dec = pwr_fpga_cnv_time_range_to_dec(i);
        if (dec >= rdec) {
            break;
        }
    }
    if (i > 5)
        i = 5;

    /* Apply decimation parameter (time range), but not when forcing GUI client
     * or during reset zoom.
     */
    /* Not applying decimation, selectable in gui 
    if ((forcex_state == 0) && (reset_zoom == 0)) {
        p[TIME_RANGE_PARAM].value = i;
    }
    */
    TRACE("TR: Dcimation: %6.2f -> %dx\n", rdec, dec);

    /* New time_unit & factor */
    time_unit = time_range_to_time_unit(p[TIME_RANGE_PARAM].value);
    t_unit_factor = pow(10, 3*(2 - time_unit));

    /* Update time unit Min and Max, but not if GUI hasn't responded to "forceX" command. */
    /*if (forcex_state == 0) {
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
	*/
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
        t_delay= PWR_FPGA_SIG_LEN ;
    } else {
        t_delay= PWR_FPGA_SIG_LEN + (xmin * c_pwr_fpga_smpl_freq / dec);
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
        p[TRIG_DLY_PARAM].value = ((t_delay - PWR_FPGA_SIG_LEN) * dec / c_pwr_fpga_smpl_freq);
    } else {
        p[TRIG_DLY_PARAM].value = forced_delay;
    }

    /* Server issues a forceX command when time units change wrt. GUI (client) units */
    /*if ((time_unit != time_unit_gui)) {
        p[FORCEX_FLAG_PARAM].value = 1.0;
        forcex_state = 1;
	*/
        /* Other settings frozen until GUI recovers */
     /*   forced_xmin = p[MIN_GUI_PARAM].value;
        forced_xmax = p[MAX_GUI_PARAM].value;
        forced_units = p[TIME_UNIT_PARAM].value;
        forced_delay = p[TRIG_DLY_PARAM].value;
    }
	*/
    /* When client issues a zoom reset, a particular ForceX command with
     * the initial 0 - 130 us time range.
     */
    if (reset_zoom == 1) {
        p[FORCEX_FLAG_PARAM].value  = 1.0;
        forcex_state = 1;

        forced_xmin = 0.0;
        forced_xmax = PWR_FPGA_SIG_LEN * dec / c_pwr_fpga_smpl_freq;
        forced_units = p[TIME_UNIT_PARAM].value;
        forced_delay = 0;

        p[MIN_GUI_PARAM].value = forced_xmin;
        p[MAX_GUI_PARAM].value = forced_xmax;
        p[GUI_XMIN].value = p[MIN_GUI_PARAM].value;
        p[GUI_XMAX].value = p[MAX_GUI_PARAM].value;
        p[TIME_UNIT_PARAM].value = forced_units;
        p[TRIG_DLY_PARAM].value = forced_delay;
        //p[TIME_RANGE_PARAM].value = 0;
    }

    TRACE("TR: Trigger delay: %.6f\n", p[TRIG_DLY_PARAM].value);

    return ret;
}


void transform_to_iface_units(rp_app_params_t *p)
{
    float scale1 = voltage_probe_factor_select(p[DIFF_PRB_ATT].value, 0);
    float scale2 = probe_select_to_factor(p[CURR_PROBE_SELECT].value, 0);
    float maxv = pwr_fpga_calc_adc_max_v(rp_main_calib_params.fe_ch1_fs_g_lo)
                 * 2 / 3; //vse sonde dajo okol 10V rms outputa
    
    /* Re-calculate output parameters */
    p[GUI_RST_Y_RANGE_U].value = round(maxv * scale1);
    p[GUI_RST_Y_RANGE_I].value = round(maxv * scale2);
    p[GUI_RST_Y_RANGE_P].value = p[GUI_RST_Y_RANGE_U].value * 
                                 p[GUI_RST_Y_RANGE_I].value;
    
    p[MIN_Y_U_PARAM].value = p[MIN_Y_NORM_U].value * scale1;
    p[MAX_Y_U_PARAM].value = p[MAX_Y_NORM_U].value * scale1;
    
    p[MIN_Y_I_PARAM].value = p[MIN_Y_NORM_I].value * scale2;
    p[MAX_Y_I_PARAM].value = p[MAX_Y_NORM_I].value * scale2;
    
    p[MIN_Y_P_PARAM].value = p[MIN_Y_NORM_P].value * scale1 * scale2;
    p[MAX_Y_P_PARAM].value = p[MAX_Y_NORM_P].value * scale1 * scale2;

    p[GEN_DC_OFFS_1].value = p[GEN_DC_NORM_1].value * scale1;
    p[GEN_DC_OFFS_2].value = p[GEN_DC_NORM_2].value * scale2;

    p[SCALE_CH1].value = scale1;
    p[SCALE_CH2].value = scale2;
}

void transform_from_iface_units(rp_app_params_t *p, 
								int u_scale_radio, int u_scale_input,
								int i_scale_radio, int i_scale_input)
{
    float scale1 = 1;
    float scale2 = 1;
    if (u_scale_radio) {
	    scale1 = voltage_probe_factor_select(p[DIFF_PRB_ATT].value, 0);
	    u_scale_input = 0;
	} else if (u_scale_input){
        scale1 = voltage_probe_factor_select(p[DIFF_PRB_ATT].value, 1);
	} else {
		scale1 = voltage_probe_factor_select(p[DIFF_PRB_ATT].value, 0);
	}
    if (i_scale_radio) {
	    scale2 = probe_select_to_factor(p[CURR_PROBE_SELECT].value, 0);
	    i_scale_input = 0;
	} else if (i_scale_input){
        scale2 = probe_select_to_factor(p[CURR_PROBE_SELECT].value, 1);
	} else {
		scale2 = probe_select_to_factor(p[CURR_PROBE_SELECT].value, 0);
	}
    //error, see if it works without float maxv = pwr_fpga_calc_adc_max_v(rp_main_calib_params.fe_ch1_fs_g_lo);

    /* Re-calculate input parameters */
    p[GEN_DC_NORM_1].value = p[GEN_DC_OFFS_1].value / scale1;
    p[GEN_DC_NORM_2].value = p[GEN_DC_OFFS_2].value / scale2;
}

int rp_set_params(rp_app_params_t *p, int len)
{
    int i;
    int fpga_update = 1;
    int params_change = 0;
    int u_scale_change_radio = 0;
    int u_scale_change_input = 0;
    int i_scale_change_radio = 0;
    int i_scale_change_input = 0;
 
    
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
            params_change = 1;
            if(p_idx == DIFF_PRB_ATT) {
                u_scale_change_radio = 1;
			}
			if(p_idx == SCALE_CH1) {
                u_scale_change_input = 1;
			}
            if(p_idx == CURR_PROBE_SELECT) {
                i_scale_change_radio = 1;
			}
			if(p_idx == SCALE_CH2) {
                i_scale_change_input = 1;
			}
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
    
    transform_from_iface_units(&rp_main_params[0],
							   u_scale_change_radio, u_scale_change_input,
                               i_scale_change_radio, i_scale_change_input);
    pthread_mutex_unlock(&rp_main_params_mutex);
    

    /* Set parameters in HW/FPGA only if they have changed */
    if(params_change || (params_init == 0)) {

        pthread_mutex_lock(&rp_main_params_mutex);
        /* Xmin & Xmax public copy to be served to clients */
        rp_main_params[GUI_XMIN].value = p[MIN_GUI_PARAM].value;
        rp_main_params[GUI_XMAX].value = p[MAX_GUI_PARAM].value;
        transform_acq_params(rp_main_params);
        set_transducer_and_led(rp_main_params[CURR_PROBE_SELECT].value);
        pthread_mutex_unlock(&rp_main_params_mutex);

        /* First do health check and then send it to the worker! */
        int mode = rp_main_params[TRIG_MODE_PARAM].value;
        int time_range = rp_main_params[TIME_RANGE_PARAM].value;
        int time_unit = 2;
        /* Get info from FPGA module about clocks/decimation, ...*/
        int dec_factor = pwr_fpga_cnv_time_range_to_dec(time_range);
        float smpl_period = c_pwr_fpga_smpl_period * dec_factor;
        /* t_delay - trigger delay in seconds */
        float t_delay = rp_main_params[TRIG_DLY_PARAM].value;
        float t_unit_factor = 1; /* to convert to seconds */

        /* Our time window with current settings:
         *   - time_delay is added later, when we check if it is correct 
         *     setting 
         */
        float t_min = 0;
        float t_max = ((PWR_FPGA_SIG_LEN-1) * smpl_period);
        float t_max_minus = ((PWR_FPGA_SIG_LEN-6) * smpl_period);

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

            rp_pwr_clean_signals();
            rp_pwr_worker_change_state(rp_pwr_auto_set_state);
            /* AUTO_FLAG_PARAM is cleared when Auto-set algorithm finishes */
            
            /* Wait for auto-set algorithm to finish or timeout */
            int timeout = 10000000; // [us]
            const int step = 50000; // [us]
            rp_pwr_worker_state_t state;
            while (timeout > 0) {
         
                rp_pwr_worker_get_state(&state);
                if (state != rp_pwr_auto_set_state) {
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
         *  t_delay <= PWR_FPGA_MAX_TRIG_DELAY
         */
        if(t_delay < -t_max_minus) {
            t_delay = -t_max_minus;
        } else if(t_delay > (PWR_FPGA_TRIG_DLY_MASK * smpl_period)) {
            t_delay = PWR_FPGA_TRIG_DLY_MASK * smpl_period;
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

        if((((t_stop_idx-t_start_idx)/(float)(SIGNAL_LENGTH-1))) >= 1) {
            t_step_idx = ceil((t_stop_idx-t_start_idx)/(float)(SIGNAL_LENGTH-1));
            int max_step = PWR_FPGA_SIG_LEN/SIGNAL_LENGTH;
            if(t_step_idx > max_step)
                t_step_idx = max_step;

            t_stop = t_start + SIGNAL_LENGTH * t_step_idx * smpl_period;
        }

        TRACE("PC: t_stop (rounded) = %.9f\n", t_stop);

        /* write back and convert to set units */
        rp_main_params[MIN_GUI_PARAM].value = t_start;
        rp_main_params[MAX_GUI_PARAM].value = t_stop;

        rp_pwr_worker_update_params((rp_app_params_t *)&rp_main_params[0], 
                                    fpga_update);

        /* check if we need to change state */
        switch(mode) {
        case 0:
            /* auto */
            rp_pwr_worker_change_state(rp_pwr_auto_state);
            break;
        case 1:
            /* normal */
            rp_pwr_worker_change_state(rp_pwr_normal_state);
            break;
        case 2:
            /* single - clear last ok buffer */
            rp_pwr_worker_change_state(rp_pwr_idle_state);
            rp_pwr_clean_signals();
            break;
        default:
            return -1;
        }

        if(rp_main_params[SINGLE_BUT_PARAM].value == 1) {
            rp_main_params[SINGLE_BUT_PARAM].value = 0;
            rp_pwr_clean_signals();
            rp_pwr_worker_change_state(rp_pwr_single_state);
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
    *sig_len = SIGNAL_LENGTH;

    ret_val = rp_pwr_get_signals(s, &sig_idx);

    /* Not finished signal */
    if((ret_val != -1) && sig_idx != SIGNAL_LENGTH-1) {
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

int rp_update_ch_meas_data(rp_pwr_ch_meas_res_t u_meas, rp_pwr_ch_meas_res_t i_meas)
{
    pthread_mutex_lock(&rp_main_params_mutex);
    rp_main_params[MEAS_MIN_U].value = u_meas.min;
    rp_main_params[MEAS_MAX_U].value = u_meas.max;
    rp_main_params[MEAS_AMP_U].value = u_meas.amp;
    rp_main_params[MEAS_AVG_U].value = u_meas.avg;

    rp_main_params[MEAS_MIN_I].value = i_meas.min;
    rp_main_params[MEAS_MAX_I].value = i_meas.max;
    rp_main_params[MEAS_AMP_I].value = i_meas.amp;
    rp_main_params[MEAS_AVG_I].value = i_meas.avg;
    
    pthread_mutex_unlock(&rp_main_params_mutex);
    
    return 0;
}

int rp_update_meas_data(rp_pwr_meas_res_t pwr_meas)
{
    pthread_mutex_lock(&rp_main_params_mutex);
    
    rp_main_params[MEAS_U_EF].value = pwr_meas.Uef;
    rp_main_params[MEAS_I_EF].value = pwr_meas.Ief;
    rp_main_params[MEAS_P].value = pwr_meas.p;
    rp_main_params[MEAS_Q].value = pwr_meas.q;
    rp_main_params[MEAS_S].value = pwr_meas.s;
    rp_main_params[MEAS_P_1].value = pwr_meas.p1;
    rp_main_params[MEAS_Q_1].value = pwr_meas.q1;
    rp_main_params[MEAS_P_H].value = pwr_meas.ph;
    rp_main_params[MEAS_Q_H1].value = pwr_meas.qh1;
    rp_main_params[MEAS_Q_H2].value = pwr_meas.qh2;
    rp_main_params[MEAS_Q_2].value = pwr_meas.q2;
    rp_main_params[MEAS_FREQ].value = pwr_meas.freq;
    rp_main_params[MEAS_COS_FI_1].value = pwr_meas.cos_fi_fund;
    rp_main_params[MEAS_PF].value = pwr_meas.pf;
    rp_main_params[MEAS_U_1_FFT].value = pwr_meas.u1_fft;
    rp_main_params[MEAS_I_1_FFT].value = pwr_meas.i1_fft;
    rp_main_params[MEAS_SUM_U_H].value = pwr_meas.sum_Uh;
    rp_main_params[MEAS_SUM_I_H].value = pwr_meas.sum_Ih;
    rp_main_params[MEAS_THD_U].value = pwr_meas.thd_u;
    rp_main_params[MEAS_THD_I].value = pwr_meas.thd_i;

	if(rp_main_params[WRITE_PARAMS].value == 1) {
		if(write_to_file() < 0) {
			fprintf(stderr, "Failed to write to file.\n");
		}
	} else if(rp_main_params[WRITE_PARAMS].value==0 && write_in_progress==1) {
		write_in_progress = 0;
		new_file = 1;
	}
    
    pthread_mutex_unlock(&rp_main_params_mutex);
    
    return 0;
}

int rp_update_harmonics_data(rp_pwr_harm_t *harm)
{
	pthread_mutex_lock(&rp_main_params_mutex);
	
	int i, j;
	
	for(i=PARAMS_HARM_U_PARAMS, j=0;
	    i<PARAMS_HARM_I_PARAMS; i++, j++ ) {
			
	    rp_main_params[i].value = harm[j].U;
    }
    
    for(i=PARAMS_HARM_I_PARAMS, j=0;
	    i<PARAMS_FI_U_PARAMS; i++, j++ ) {
			
	    rp_main_params[i].value = harm[j].I;
    }
    
    for(i=PARAMS_FI_U_PARAMS, j=1;
	    i<PARAMS_FI_I_PARAMS; i++, j++ ) {
			
	    rp_main_params[i].value = harm[j].fiU;
    }
    
    for(i=PARAMS_FI_I_PARAMS, j=1;
	    i<PARAMS_NUM; i++, j++ ) {
			
	    rp_main_params[i].value = harm[j].fiI;
    }
	
	pthread_mutex_unlock(&rp_main_params_mutex);
    return 0;
}

int write_to_file(void)
{
	float param_value;
    int i, j=0;
    char *c_date;
    char c_time[11];
	
	if(new_file == 1) {
		if(create_new_file() < 0) {
			fprintf(stderr, "Failed to create new file.\n");
			return -1;
		} else {
			new_file = 0;
			write_in_progress = 1;
		}
    }
    
    c_date = get_current_time();
    while (j < 8) {
		c_time[j] = c_date[j+11];
		j++;
	}
	c_time[8] = '\0';
    
    fprintf(fp, "%s\t", c_time);
    
    for(i=19; i<=44; i++) {
	   param_value = rp_main_params[i].value;
	   fprintf(fp, "%f\t", param_value);
	}
	for(i=PARAMS_HARM_U_PARAMS; i<PARAMS_NUM; i++) {
	   param_value = rp_main_params[i].value;
	   fprintf(fp, "%f\t", param_value);
	}
	fprintf(fp, "\n");
	
	return 0;
	
}

char *get_current_time(void) 
{
	time_t current_time;
	char *time_string;
	
	/* Obtain current time. */
    current_time = time(NULL);

    if (current_time == ((time_t)-1)) {
        fprintf(stderr, "Failure to obtain the current time.\n");
    }

    /* Convert to local time format. */
    time_string = ctime(&current_time);

    if (time_string == NULL) {
        fprintf(stderr, "Failure to convert the current time.\n");
    }
    
	return time_string;
}

int create_new_file(void)
{
	
    char *c_time_string;
    char file_name[40];
    char *param_name;
    int i;
    
    c_time_string = get_current_time();
    c_time_string[strlen(c_time_string) - 1]  = '\0';
    
    if(fp) {
       fclose(fp);
    }
    
    sprintf(file_name, "/measurements/%s", c_time_string);
    
    fp = fopen(file_name, "w");
    if(fp == NULL) {
	    fprintf(stderr, "fopen failed.\n");
	    return -1;
    }
    
    fprintf(fp, "time\t\t");
    
    for(i=19; i<=44; i++) {
	   param_name = rp_main_params[i].name;
	   fprintf(fp, "%8s\t", param_name);
	}
	for(i=PARAMS_HARM_U_PARAMS; i<PARAMS_NUM; i++) {
	   param_name = rp_main_params[i].name;
	   fprintf(fp, "%8s\t", param_name);
	}
	fprintf(fp, "\n");
	
	return 0;        
}

void pisi_error(int p)
{
	char *c_date;
    char c_time[11];
    int j=0;
    
	c_date = get_current_time();
    while (j < 8) {
		c_time[j] = c_date[j+11];
		j++;
	}
	c_time[8] = '\0';
    
    fprintf(stderr, "%s\t%d\n", c_time, p);
}
