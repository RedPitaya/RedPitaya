/** @file main.h
 *
 * $Id: main.h 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Main module (C Header).
 * @author Jure Menart <juremenart@gmail.com>
 * @copyright Red Pitaya  http://www.redpitaya.com
 */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef DEBUG
#  define TRACE(args...) fprintf(stderr, args)
#else
#  define TRACE(args...) {}
#endif

/* Parameters description structure - must be the same for all RP controllers */
typedef struct rp_app_params_s {
    char  *name;
    float  value;
    int    fpga_update;
    int    read_only;
    float  min_val;
    float  max_val;
} rp_app_params_t;

/* Signal measurement results structure - filled in worker and updated when
 * also measurement signal is stored from worker 
 */

 /* TODO: Delete this strcuture */
typedef struct rp_osc_meas_res_s {
    float min;
    float max;
    float amp;
    float avg;
    float freq;
    float period;
} rp_osc_meas_res_t;


/* Parameters indexes - these defines should be in the same order as 
 * rp_app_params_t structure defined in main.c */
#define PARAMS_NUM        64
#define MIN_GUI_PARAM     0
#define MAX_GUI_PARAM     1
#define TRIG_MODE_PARAM   2
#define TRIG_SRC_PARAM    3
#define TRIG_EDGE_PARAM   4
#define TRIG_DLY_PARAM    5
#define TRIG_LEVEL_PARAM  6
#define SINGLE_BUT_PARAM  7
#define TIME_RANGE_PARAM  8
#define TIME_UNIT_PARAM   9
#define EN_AVG_AT_DEC     10
#define AUTO_FLAG_PARAM   11
#define MIN_Y_PARAM       12
#define MAX_Y_PARAM       13
#define FORCEX_FLAG_PARAM  14
#define MEAS_MIN_CH1      15
#define MEAS_MAX_CH1      16
#define MEAS_AMP_CH1      17
#define MEAS_AVG_CH1      18
#define MEAS_FREQ_CH1     19
#define MEAS_PER_CH1      20
#define MEAS_MIN_CH2      21
#define MEAS_MAX_CH2      22
#define MEAS_AMP_CH2      23
#define MEAS_AVG_CH2      24
#define MEAS_FREQ_CH2     25
#define MEAS_PER_CH2      26
#define PRB_ATT_CH1       27
#define GAIN_CH1          28
#define PRB_ATT_CH2       29
#define GAIN_CH2          30
#define GUI_RST_Y_RANGE   31
#define PREPARE_WAVE      32
#define GEN_DC_OFFS_1     33
#define GEN_DC_OFFS_2     34
/*lcr parameters */
#define START_MEASURE     35
#define GEN_AMP           36
#define GEN_AVG           37
#define GEN_DC_BIAS       38
#define GEN_R_SHUNT       39
#define LCR_STEPS         40
#define START_FREQ        41
#define END_FREQ          42
#define PLOT_Y_SCALE_DATA 43
#define LCR_SCALE_TYPE    44
#define GEN_FS_LOADRE     45
#define GEN_FS_LOADIM     46
#define LCR_CALIBRATION   47
#define LCR_PROGRESS      48
#define LCR_SAVE_DATA     49
/* AWG parameters */
#define GEN_TRIG_MODE_CH1 50
#define GEN_SIG_TYPE_CH1  51
#define GEN_ENABLE_CH1    52
#define GEN_SINGLE_CH1    53
#define GEN_SIG_AMP_CH1   54
#define GEN_SIG_FREQ_CH1  55
#define GEN_SIG_DCOFF_CH1 56
#define GEN_TRIG_MODE_CH2 57
#define GEN_SIG_TYPE_CH2  58
#define GEN_ENABLE_CH2    59
#define GEN_SINGLE_CH2    60
#define GEN_SIG_AMP_CH2   61
#define GEN_SIG_FREQ_CH2  62
#define GEN_SIG_DCOFF_CH2 63

/* Defines from which parameters on are AWG parameters (used in set_param() to
 * trigger update only on needed part - either Oscilloscope or AWG */
#define PARAMS_AWG_PARAMS 50

/* Output signals */
#define SIGNALS_NUM   3

/* Measurment length - always 1024. Stripped in JS */
//#define SIGNAL_LENGTH 1000

/* module entry points */
int rp_app_init(void);
int rp_app_exit(void);
int rp_set_params(rp_app_params_t *p, int len);
int rp_get_params(rp_app_params_t **p);
int rp_get_signals(float ***s, int *sig_num, int *sig_len);

/* Internal helper functions */
int  rp_create_signals(float ***a_signals);
void rp_cleanup_signals(float ***a_signals);

/* copies parameters from src to dst - if dst does not exists, it creates it */
int rp_copy_params(rp_app_params_t *src, rp_app_params_t **dst);

/* cleans up memory of parameters structure */
int rp_clean_params(rp_app_params_t *params);

/* Updates all parameters (structure must be aligned with main parameter
 * structure - this includes also ready-only parameters. After the 
* parameters are updated it also changed the worker state machine.
 */
int rp_update_main_params(rp_app_params_t *params);

/* Waveform generator frequency limiter. */
float rp_gen_limit_freq(float freq, float gen_type);

/* LCR get parameters */
float rp_get_params_lcr(int pos);

/* LCR set parameters */
void rp_set_params_lcr(int pos, float val);

#endif /*  __MAIN_H */
