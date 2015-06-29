/**
 * $Id: worker.h 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Spectrum Analyzer worker.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __SPECTROMETERAPP_H
#define __SPECTROMETERAPP_H

#include "../../rpbase/src/rp.h"
#include "rpApp.h"

/* Parameters description structure - must be the same for all RP controllers */
typedef struct rp_app_params_s {
    char  *name;
    float  value;
    int    fpga_update;
    int    read_only;
    float  min_val;
    float  max_val;
} rp_app_params_t;


typedef enum rp_spectr_worker_state_e {
    rp_spectr_idle_state = 0, /* do nothing */
    rp_spectr_quit_state, /* shutdown worker */
    rp_spectr_abort_state, /* abort current measurement */
    rp_spectr_auto_state, /* auto mode acquisition */
    rp_spectr_nonexisting_state /* must be last */
} rp_spectr_worker_state_t;

/* Worker results (not signal but calculated peaks and jpeg index */
typedef struct rp_spectr_worker_res_s {
    int   jpg_idx;
    float peak_pw_cha;
    float peak_pw_freq_cha;
    float peak_pw_chb;
    float peak_pw_freq_chb;
} rp_spectr_worker_res_t;

/* Parameters indexes - these defines should be in the same order as
 * rp_app_params_t structure defined in main.c */
#define PARAMS_NUM             12
#define MIN_GUI_PARAM          0
#define MAX_GUI_PARAM          1
#define FREQ_RANGE_PARAM       2
#define FREQ_UNIT_PARAM        3
#define PEAK_PW_FREQ_CHA_PARAM 4
#define PEAK_PW_CHA_PARAM      5
#define PEAK_UNIT_CHA_PARAM    6
#define PEAK_PW_FREQ_CHB_PARAM 7
#define PEAK_PW_CHB_PARAM      8
#define PEAK_UNIT_CHB_PARAM    9
#define JPG_FILE_IDX_PARAM     10
#define EN_AVG_AT_DEC   		11

/* Output signals */
#define SPECTR_OUT_SIG_LEN (2*1024)
#define SPECTR_OUT_SIG_NUM   3

extern rp_app_params_t rp_main_params[PARAMS_NUM+1];

int rp_app_init(void);
int rp_app_exit(void);
int rp_set_params(rp_app_params_t *p, int len);
int rp_get_params(rp_app_params_t **p);
int rp_get_signals(float ***s, int *sig_num, int *sig_len);
int rp_spectr_get_signals_channel(float **signals, size_t size);
int rp_spectr_get_params(rp_spectr_worker_res_t *result);

/* Internal helper functions */
int  rp_create_signals(float ***a_signals);
void rp_cleanup_signals(float ***a_signals);


int rp_spectr_worker_init(const wf_func_table_t* wf_f);
int rp_spectr_worker_clean(void);
int rp_spectr_worker_exit(void);
int rp_spectr_worker_change_state(rp_spectr_worker_state_t new_state);
int rp_spectr_worker_update_params(rp_app_params_t *params, int fpga_update);
int rp_spectr_worker_update_params_by_idx(int value, size_t idx, int fpga_update);

/* removes 'dirty' flags */
int rp_spectr_clean_signals(void);
/* Cleans up temporary directory (JPGs) */
int rp_spectr_clean_tmpdir(const char *dir);

/* Returns:
 *  0 - new signals (dirty signal) are copied to the output 
 *  1 - no new signals available (dirty signal was not set - we need to wait)
 */
int rp_spectr_get_signals(float ***signals, rp_spectr_worker_res_t *result);

/* Fills the output signal structure from temp one after calculation is done 
 * and marks it dirty 
 */

int rp_spectr_set_signals(float **source, rp_spectr_worker_res_t result);


int spec_run(const wf_func_table_t* wf_f);

int spec_stop();

int spec_running(); // true/false

int spec_reset();

int spec_getViewData(float **signals, size_t size);

int spec_getJpgIdx(int* jpg);

int spec_getPeakPower(int channel, float* power);

int spec_getPeakFreq(int channel, float* freq);

int spec_setFreqRange(float _freq_min, float freq);

int spec_setUnit(int unit);

int spec_getUnit();

int spec_getFpgaFreq(float* freq);

int spec_getFreqMax(float* freq);

int spec_getFreqMin(float* freq);


#endif /* __SPECTROMETERAPP_H*/
