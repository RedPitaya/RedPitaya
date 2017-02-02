/**
 * $Id: main.h 881 2013-12-16 05:37:34Z rp_jmenart $
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
typedef struct rp_pwr_ch_meas_res_s {
    float min;
    float max;
    float amp;
    float avg;
    
} rp_pwr_ch_meas_res_t;

typedef struct rp_pwr_meas_res_s {    
    float p;
    float q;
    float s;
    float p1;
    float q1;
    float ph;
    float qh1;
    float qh2;
    float q2;
    float freq;
    float cos_fi_fund;
    float pf;
    float Uef;
    float Ief;
    float u1_fft;
    float i1_fft;
    float sum_Uh;
    float sum_Ih;
    float thd_u;
    float thd_i;
} rp_pwr_meas_res_t;

typedef struct rp_pwr_harm_s {
	float U;
	float I;
	float fiU;
	float fiI;
} rp_pwr_harm_t;	

/* Parameters indexes - these defines should be in the same order as 
 * rp_app_params_t structure defined in main.c */
#define PARAMS_NUM        228
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
#define MIN_Y_U_PARAM     12
#define MAX_Y_U_PARAM     13
#define MIN_Y_I_PARAM     14
#define MAX_Y_I_PARAM     15
#define MIN_Y_P_PARAM     16
#define MAX_Y_P_PARAM     17
#define FORCEX_FLAG_PARAM 18
#define MEAS_MIN_U		  19
#define MEAS_MAX_U        20
#define MEAS_AMP_U        21
#define MEAS_AVG_U        22
#define MEAS_MIN_I        23
#define MEAS_MAX_I        24
#define MEAS_AMP_I        25
#define MEAS_AVG_I        26
#define MEAS_U_EF         27
#define MEAS_I_EF         28
#define MEAS_P            29
#define MEAS_Q            30
#define MEAS_S            31
#define MEAS_P_1          32
#define MEAS_Q_1          33
#define MEAS_P_H          34
#define MEAS_Q_H1         35
#define MEAS_Q_H2         36      
#define MEAS_Q_2          37            
#define MEAS_FREQ         38      
#define MEAS_COS_FI_1     39
#define MEAS_PF           40
#define MEAS_U_1_FFT      41
#define MEAS_I_1_FFT      42 
#define MEAS_SUM_U_H      43 
#define MEAS_SUM_I_H      44
#define MEAS_THD_U        45
#define MEAS_THD_I        46
#define DIFF_PRB_ATT      47
#define GAIN_CH1          48
#define CURR_PROBE_SELECT 49
#define GAIN_CH2          50
#define GUI_RST_Y_RANGE_U 51
#define GUI_RST_Y_RANGE_I 52
#define GUI_RST_Y_RANGE_P 53
#define GEN_DC_OFFS_1     54
#define GEN_DC_OFFS_2     55
#define GUI_XMIN          56
#define GUI_XMAX          57
#define MIN_Y_NORM_U      58
#define MAX_Y_NORM_U      59
#define MIN_Y_NORM_I      60
#define MAX_Y_NORM_I      61
#define MIN_Y_NORM_P      62
#define MAX_Y_NORM_P      63
#define GEN_DC_NORM_1     64
#define GEN_DC_NORM_2     65
#define SCALE_CH1         66
#define SCALE_CH2         67
#define HARM_NUM          68
#define WRITE_PARAMS      69
/*harmonics*/
#define U_1               70
#define U_2               71
#define U_3               72
#define U_4               73
#define U_5               74
#define U_6               75
#define U_7               76
#define U_8               77
#define U_9               78
#define U_10              79
#define U_11              80
#define U_12              81
#define U_13              82
#define U_14              83
#define U_15              84
#define U_16              85
#define U_17              86
#define U_18              87
#define U_19              88
#define U_20              89
#define U_21              90
#define U_22              91
#define U_23              92
#define U_24              93
#define U_25              94
#define U_26              95
#define U_27              96
#define U_28              97
#define U_29              98
#define U_30              99
#define U_31              100
#define U_32              101
#define U_33              102
#define U_34              103
#define U_35              104
#define U_36              105
#define U_37              106
#define U_38              107
#define U_39              108
#define U_40              109
#define I_1               110
#define I_2               111
#define I_3               112
#define I_4               113
#define I_5               114
#define I_6               115
#define I_7               116
#define I_8               117
#define I_9               118
#define I_10              119
#define I_11              120
#define I_12              121
#define I_13              122
#define I_14              123
#define I_15              124
#define I_16              125
#define I_17              126
#define I_18              127
#define I_19              128
#define I_20              129
#define I_21              130
#define I_22              131
#define I_23              132
#define I_24              133
#define I_25              134
#define I_26              135
#define I_27              136
#define I_28              137
#define I_29              138
#define I_30              139
#define I_31              140
#define I_32              141
#define I_33              142
#define I_34              143
#define I_35              144
#define I_36              145
#define I_37              146
#define I_38              147
#define I_39              148
#define I_40              149
#define FI_U_2            150
#define FI_U_3            151
#define FI_U_4            152
#define FI_U_5            153
#define FI_U_6            154
#define FI_U_7            155
#define FI_U_8            156
#define FI_U_9            157
#define FI_U_10           158
#define FI_U_11           159
#define FI_U_12           160
#define FI_U_13           161
#define FI_U_14           162
#define FI_U_15           163
#define FI_U_16           164
#define FI_U_17           165
#define FI_U_18           166
#define FI_U_19           167
#define FI_U_20           168
#define FI_U_21           169
#define FI_U_22           170
#define FI_U_23           171
#define FI_U_24           172
#define FI_U_25           173
#define FI_U_26           174
#define FI_U_27           175
#define FI_U_28           176
#define FI_U_29           177
#define FI_U_30           178
#define FI_U_31           179
#define FI_U_32           180
#define FI_U_33           181
#define FI_U_34           182
#define FI_U_35           183
#define FI_U_36           184
#define FI_U_37           185
#define FI_U_38           186
#define FI_U_39           187
#define FI_U_40           188
#define FI_I_2            189
#define FI_I_3            190
#define FI_I_4            191
#define FI_I_5            192
#define FI_I_6            193
#define FI_I_7            194
#define FI_I_8            195
#define FI_I_9            196
#define FI_I_10           197
#define FI_I_11           198
#define FI_I_12           199
#define FI_I_13           200
#define FI_I_14           201
#define FI_I_15           202
#define FI_I_16           203
#define FI_I_17           204
#define FI_I_18           205
#define FI_I_19           206
#define FI_I_20           207
#define FI_I_21           208
#define FI_I_22           209
#define FI_I_23           210
#define FI_I_24           211
#define FI_I_25           212
#define FI_I_26           213
#define FI_I_27           214
#define FI_I_28           215
#define FI_I_29           216
#define FI_I_30           217
#define FI_I_31           218
#define FI_I_32           219
#define FI_I_33           220
#define FI_I_34           221
#define FI_I_35           222
#define FI_I_36           223
#define FI_I_37           224
#define FI_I_38           225
#define FI_I_39           226
#define FI_I_40           227

#define PARAMS_HARM_U_PARAMS 70 //from which parameter on are U harmonics
#define PARAMS_HARM_I_PARAMS 110 //from which parameter on are I harmonics
#define PARAMS_FI_U_PARAMS   150 //from which parameter on are U angles
#define PARAMS_FI_I_PARAMS   189 //from which parameter on are I angles


/* Output signals */
#define SIGNAL_LENGTH (1024) /* Must be 2^n! */
#define SIGNALS_NUM   3


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
void transform_to_iface_units(rp_app_params_t *p);
void transform_from_iface_units(rp_app_params_t *p, int u_scale_radio, 
								int u_scale_input, int i_scale_radio, 
								int i_scale_input);

/* sets the measurement data to output parameters structure - these parameters
 * are read-only for the client and there is no need to update them internally
 * in the application 
 */
int rp_update_ch_meas_data(rp_pwr_ch_meas_res_t u_meas, rp_pwr_ch_meas_res_t i_meas);
int rp_update_meas_data(rp_pwr_meas_res_t pwr_meas);
int rp_update_harmonics_data(rp_pwr_harm_t *harm);

int time_range_to_time_unit(int range);
float probe_select_to_factor(int probe, int fact_change);
float voltage_probe_factor_select(int fact_select, int fact_change);

int write_to_file(void);
char *get_current_time(void);
int create_new_file(void);

void pisi_error(int p);

#endif /*  __MAIN_H */
