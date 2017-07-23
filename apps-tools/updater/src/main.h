#pragma once


#include <DataManager.h>
#include <CustomParameters.h>

#define CH_SIGNAL_SIZE_DEFAULT		1024
#define CALIB_FE_LV_REF_V           1.0f
#define CALIB_FE_HV_REF_V           5.0f

#define IF_VALUE_CHANGED(X, ACTION) \
if (X.Value() != X.NewValue()) { \
    int res = ACTION;\
    if (res == RP_OK) { \
        X.Update(); \
    } \
}

#define IF_VALUE_CHANGED_BOOL(X, ACTION1, ACTION2) \
if (X.Value() != X.NewValue()) { \
    if (X.NewValue()) { \
        ACTION1;    X.Update(); \
    } else { \
        ACTION2;    X.Update(); }}

#define IS_NEW(X) X.Value() != X.NewValue()


#ifdef __cplusplus
extern "C" {
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
typedef struct rp_osc_meas_res_s {
    float min;
    float max;
    float amp;
    float avg;
    float freq;
    float period;
} rp_osc_meas_res_t;

const char *rp_app_desc(void);
int rp_app_init(void);
int rp_app_exit(void);
int rp_set_params(rp_app_params_t *p, int len);
int rp_get_params(rp_app_params_t **p);
int rp_get_signals(float ***s, int *sig_num, int *sig_len);
int IsRunning(void);

#ifdef __cplusplus
}
#endif
