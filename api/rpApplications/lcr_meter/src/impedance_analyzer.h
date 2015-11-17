/**
* $Id: $
*
* @brief Red Pitaya application library Impedance Analyzer module interface
*
* @Author Luka Golinar <luka.golinar@gmail.com>
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#ifndef __IMPEDANCEANALYZER_H
#define __IMPEDANCEANALYZER_H

#ifndef __cplusplus 
#include <complex.h> 
#endif

#include "redpitaya/rp.h"

#define REPEAT do
#define UNTIL(exp) while(!(exp))

/* Random global defines */
#define AMPLITUDE_MAX			1.0
#define ADC_BUFF_SIZE			16384
#define M_PI					3.14159265358979323846
#define TRANS_EFFECT_STEPS		10
#define SAMPLE_RATE				125e6
#define PARAMS_NUM				13


/* Calibration params */
#define CALIB_SIZE				100
#define START_CALIB_FREQ		100 //TODO
#define END_CALIB_FREQ			1e6


/* R_shunt constans */
static const uint32_t R_SHUNT_30	 = 30;
static const uint32_t R_SHUNT_75     = 75;
static const uint32_t R_SHUNT_300    = 300;
static const uint32_t R_SHUNT_750    = 750;
static const uint32_t R_SHUNT_3K     = 3000;
static const uint32_t R_SHUNT_7_5K   = 7500;
static const uint32_t R_SHUNT_30K    = 30000;
static const uint32_t R_SHUNT_80K    = 80000;
static const uint32_t R_SHUNT_430K   = 430000;
static const uint32_t R_SHUNT_3M     = 3000000;

typedef enum{
	IMP_SCALE_LINEAR,
	IMP_SCALE_LOGARITHMIC,
} imp_scale_t;

typedef enum{
	IMP_CALIB_NONE,
	IMP_CALIB_OPEN,
	IMP_CALIB_SHORT,
	IMP_CALIB_LOAD,
} imp_calib_t;

typedef enum{
	IMP_MEASURMENT_SWEEP,
	IMP_FREQUENCY_SWEEP,
} imp_sweep_t;

/* Main lcr params structure */
typedef struct imp_params_e{
	float amplitude;
	float dc_bias;
	float avg;
	uint32_t r_shunt;
	imp_calib_t mode;
	float ref_real;
	float ref_img;
	uint32_t steps;
	float start_freq;
	float end_freq;
	imp_scale_t scale;
	imp_sweep_t sweep; //imp_sweep_e
	bool user_wait;
	bool no_calib;
} imp_params_t;

/* Resource managment functions */
int imp_Init();
int imp_Release();
int imp_Reset();
int imp_SetDefaultValues();

/* Main lcr functions */
int imp_Run();
void *imp_MainThread();

/* Measurment functions */
int imp_SafeThreadGen(rp_channel_t channel, float ampl, float freq);

int imp_SafeThreadAcqData(rp_channel_t channel, 
	float *data, rp_acq_decimation_t decimation);

int imp_Sweep(float *ampl_z_out);

float imp_data_analysis(float **data, uint32_t size, float dc_bias, 
		uint32_t r_shunt, float _Complex *Z, float w_out, int decimation);

/* Helper functions */
uint32_t imp_shuntAlgorithm(float z_ampl);
int imp_Interpolate(float *calib_data, imp_calib_t calib_mode);

int findInterpFreq(float input_freq, 
				   float *out_index, 
				   bool start_interval);

int findIntrpInterv(float *in_z_ampl,
					float *out_sub_arry,
					int start_interval, 
					int end_interval);

int interpolationFunc(float *calib_data, float frequency);

/* Getters and Setters */
int imp_SetAmplitude(float ampl);
int imp_GetAmplitude(float *ampl);
int imp_SetDcBias(float dc_bias);
int imp_GetDcBias(float *dc_bias);
int imp_SetAveraging(float averaging);
int imp_GetAveraging(float *averaging);
int imp_SetRshunt(uint32_t r_shunt);
int imp_GetRShunt(uint32_t *r_shunt);
int imp_SetCalibMode(imp_calib_t mode);
int imp_GetCalibMode(imp_calib_t *mode);
int imp_SetNoCalibration(bool no_calib);

int imp_SetRefReal(float ref_real);
int imp_GetRefReal(float *ref_real);
int imp_SetRefImg(float ref_img);
int imp_GetRefImg(float *ref_img);

int imp_SetSteps(uint32_t steps);
int imp_GetSteps(uint32_t *steps);
int imp_SetStartFreq(float start_freq);
int imp_GetStartFreq(float *start_freq);
int imp_SetEndFreq(float end_freq);
int imp_GetEndFreq(float *end_freq);
int imp_SetScaleType(imp_scale_t scale);
int imp_GetScaleType(imp_scale_t *scale);
int imp_SetSweepMode(imp_sweep_t sweep);
int imp_GetSweepMode(imp_sweep_t *sweep);
int imp_SetUserWait(bool wait);
int imp_GetUserWait(bool *wait);

int imp_SetUserView(uint32_t view);
int imp_GetUserView(uint32_t *view);

#endif //__IMPEDANCEANALYZER_H


