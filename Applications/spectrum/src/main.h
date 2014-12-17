/**
 * $Id$
 *
 * @brief Red Pitaya Spectrum Analyzer main module.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __MAIN_H
#define __MAIN_H

/* Parameters description structure - must be the same for all RP controllers */
typedef struct rp_app_params_s {
    char  *name;
    float  value;
    int    fpga_update;
    int    read_only;
    float  min_val;
    float  max_val;
} rp_app_params_t;

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

int rp_app_init(void);
int rp_app_exit(void);
int rp_set_params(rp_app_params_t *p, int len);
int rp_get_params(rp_app_params_t **p);
int rp_get_signals(float ***s, int *sig_num, int *sig_len);

/* Internal helper functions */
int  rp_create_signals(float ***a_signals);
void rp_cleanup_signals(float ***a_signals);

#endif /*  __MAIN_H */
