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

#include "../../rp_sdk/include/rpApp.h"

/* Describe app. parameters with some info/limitations */
pthread_mutex_t rp_main_params_mutex = PTHREAD_MUTEX_INITIALIZER;

const char *rp_app_desc(void) {
    return (const char *)"Red Pitaya osciloscope application.\n";
}

int rp_app_init(void) {
    fprintf(stderr, "Loading scope version %s-%s.\n", VERSION_STR, REVISION_STR);
    rpApp_Init();
    rpApp_OscRun();
    return 0;
}

int rp_app_exit(void) {
    fprintf(stderr, "Unloading scope version %s-%s.\n", VERSION_STR, REVISION_STR);
    rpApp_Release();
    return 0;
}

int rp_set_params(rp_app_params_t *p, int len) {
    return 0;
}

/* Returned vector must be free'd externally! */
int rp_get_params(rp_app_params_t **p) {
    rp_app_params_t *p_copy = NULL;
    int i;

    pthread_mutex_lock(&rp_main_params_mutex);
    for(i = 0; i < PARAMS_NUM; i++) {
        p_copy[i].name[0]='\0';
    }
    pthread_mutex_unlock(&rp_main_params_mutex);
    p_copy[PARAMS_NUM].name = NULL;

    *p = p_copy;
    return PARAMS_NUM;
}

int rp_get_signals(float ***s, int *sig_num, int *sig_len) {
    int ret_val;
    int sig_idx;

    if(*s == NULL)
        return -1;

    *sig_num = SIGNALS_NUM;
    *sig_len = SIGNAL_LENGTH;

    ret_val = rp_osc_get_signals(s, &sig_idx);

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

int rp_create_signals(float ***a_signals) {
    return 0;
}

void rp_cleanup_signals(float ***a_signals) { }

int rp_copy_params(rp_app_params_t *src, rp_app_params_t **dst) {
    rp_app_params_t *p_new = *dst;
    *dst = p_new;
    return 0;
}

int rp_clean_params(rp_app_params_t *params) {
    return 0;
}

int rp_update_main_params(rp_app_params_t *params) {
    return 0;
}

int rp_update_meas_data(rp_osc_meas_res_t ch1_meas, rp_osc_meas_res_t ch2_meas) {
    return 0;
}










pthread_mutex_t       rp_osc_sig_mutex = PTHREAD_MUTEX_INITIALIZER;
float               **rp_osc_signals;
int                   rp_osc_signals_dirty = 0;
int                   rp_osc_sig_last_idx = 0;

/*----------------------------------------------------------------------------------*/
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