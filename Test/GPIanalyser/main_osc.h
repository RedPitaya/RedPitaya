/** @file main_osc.h
 *
 * $Id: main_osc.h 881 2013-12-16 05:37:34Z rp_jmenart $
 * 
 * @brief Red Pitaya Oscilloscope main application.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *
 * This is C header file. It includes various defines, structure definitions and
 * function declarations for main oscilloscope module. Please visit:
 * http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language and principle used herein.
 * 
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __MAIN_OSC_H
#define __MAIN_OSC_H

/** @defgroup main_osc_h main_osc_h
 * @{
 */

/** @brief Oscilloscope parameters structure
 *
 * Oscilloscope parameter structure, which is a holder for parameters, defines
 * the behavior (if FPGA update is needed, if it is read only) and minimal and
 * maximal values.
 */
typedef struct rp_osc_params_s {
    /** @brief Value holder */
    float value;
    /** @brief FPGA update needed for parameters. */
    int   fpga_update;
    /** @brief Read only parameters (writes are ignored) */
    int   read_only;
    /** @brief Minimal value allowed for the parameter */
    float min_val;
    /** @brief Maximum value allowed for the parameter */
    float max_val;
} rp_osc_params_t;

/* Parameters indexes */
/** Number of parameters defined in main module */
#define PARAMS_NUM      10
/** Minimal time in output time vector */
#define MIN_GUI_PARAM    0
/** Maximal time in output time vector */
#define MAX_GUI_PARAM    1
/** Trigger mode */
#define TRIG_MODE_PARAM  2
/** Trigger source */
#define TRIG_SRC_PARAM   3
/** Trigger edge */
#define TRIG_EDGE_PARAM  4
/** Trigger delay */
#define TRIG_DLY_PARAM   5
/** Trigger level */
#define TRIG_LEVEL_PARAM 6
/** Single acquisition requested */
#define SINGLE_BUT_PARAM 7
/** Time range */
#define TIME_RANGE_PARAM 8
/** Time unit (read-only) */
#define TIME_UNIT_PARAM  9

/** Output signal length  */
#define SIGNAL_LENGTH (16*1024)
/** Number of output signals */
#define SIGNALS_NUM   3

/** @} */

int rp_app_init(void);
int rp_app_exit(void);
int rp_set_params(float *p, int len);
int rp_get_params(float **p);
int rp_get_signals(float ***s, int *sig_num, int *sig_len);

/* Internal helper functions */
int  rp_create_signals(float ***a_signals);
void rp_cleanup_signals(float ***a_signals);

#endif /*  __MAIN_OSC_H */
