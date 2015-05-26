#pragma once


#include <DataManager.h>
#include <CustomParameters.h>
#include "rpApp.h"

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

#define IF_VALUE_CHANGED_DEBUG(X, ACTION) \
if (X.Value() != X.NewValue()) { \
    float val = (float)X.NewValue(); \
    fprintf(stderr, "------------  Got new value for %s:\t\t%f  at %s:%d\n", #X, val, __FILE__, __LINE__);\
    int res = ACTION;\
    if (res == RP_OK) { \
        fprintf(stderr, "------------  Action %s\t\tSUCCES  at %s:%d\n", #ACTION, __FILE__, __LINE__); \
        X.Update(); \
        fprintf(stderr, "------------  Value updated to: %f   at %s:%d\n", (float)X.Value(), __FILE__, __LINE__); \
    } else { \
        fprintf(stderr, "------------  Action %s\t\treturnes %d - %s\t\t   at %s:%d\n", #ACTION, res, rpApp_GetError(res), __FILE__, __LINE__); \
    }\
}

#define IF_VALUE_CHANGED_BOOL_DEBUG(X, ACTION1, ACTION2) \
if (X.Value() != X.NewValue()) { \
    fprintf(stderr, "------------  Got new value for %s:\t\t%s  at %s:%d\n", #X, X.NewValue() ? "ON" : "OFF", __FILE__, __LINE__);\
    if (X.NewValue()) { \
        int res = ACTION1;    X.Update();fprintf(stderr, "------------  Action %s returned %d   at %s:%d\n", #ACTION1, res, __FILE__, __LINE__); \
    } else { \
        int res = ACTION2;    X.Update();fprintf(stderr, "------------  Action %s returned %d   at %s:%d\n", #ACTION2, res, __FILE__, __LINE__);  }}

#define IF_VALUE_CHANGED(X, A) IF_VALUE_CHANGED_DEBUG(X, A)
#define IF_VALUE_CHANGED_BOOL(X, A1, A2)  IF_VALUE_CHANGED_BOOL_DEBUG(X, A1, A2)


float getMeasureValue(int measure);
void synthesis_sin(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff);
void synthesis_triangle(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff);
void synthesis_square(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff);
void generate(rp_channel_t channel);


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

#ifdef __cplusplus
}
#endif
