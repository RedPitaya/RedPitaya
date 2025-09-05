#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Parameters description structure - must be the same for all RP controllers */
typedef struct rp_app_params_s {
    char* name;
    float value;
    int fpga_update;
    int read_only;
    float min_val;
    float max_val;
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

//Rp app functions
const char* rp_app_desc(void);
int rp_app_init(void);
int rp_app_exit(void);
int rp_set_params(rp_app_params_t*, int);
int rp_get_params(rp_app_params_t**);
int rp_get_signals(float***, int*, int*);

#ifdef __cplusplus
}
#endif
