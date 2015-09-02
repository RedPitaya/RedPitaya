/**
 * $Id$
 *
 * @brief Red Pitaya LTI main module.
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
#define PARAMS_NUM             16
#define MIN_GUI_PARAM          0
#define MAX_GUI_PARAM          1
#define FREQ_RANGE_PARAM       2
#define FREQ_UNIT_PARAM        3
#define EN_AVG_AT_DEC   	4
#define LTI_B0     		5
#define LTI_B1     		6
#define LTI_B2     		7
#define LTI_B3     		8
#define LTI_B4     		9
#define LTI_B5     		10
#define LTI_A1     		11
#define LTI_A2     		12
#define LTI_A3     		13
#define LTI_A4     		14
#define LTI_A5     		15



/* Output signals */
#define LTI_OUT_SIG_LEN (2*1024)
#define LTI_OUT_SIG_NUM   3

int rp_app_init(void);
int rp_app_exit(void);
int rp_set_params(rp_app_params_t *p, int len);
int rp_get_params(rp_app_params_t **p);
int rp_get_signals(float ***s, int *sig_num, int *sig_len);

/* Internal helper functions */
int  rp_create_signals(float ***a_signals);
void rp_cleanup_signals(float ***a_signals);

void dir_gen_set(int ch, int param, int value);


#endif /*  __MAIN_H */
