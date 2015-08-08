/**
* $Id: $
*
* @brief Red Pitaya application library impedance anaylzer module library interface
*
* @Author Luka Golinar
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#ifndef __IMPEDANCEANALYZER_H
#define __IMPEDANCEANALYZER_H

#include <complex.h>
#include "../../../rpbase/src/rp.h"

#define AMPLITUDE_MAX			1.0
#define ADC_BUFF_SIZE			16 * 1024
#define M_PI					3.14159265358979323846
#define TRANS_EFFECT_STEPS		10
#define SAMPLE_RATE				125e6
#define RP_SNOMATCH				-10



typedef enum{
	IMP_SCALE_LINEAR,
	IMP_SCALE_LOGARITHMIC,
} imp_scale_e;

typedef enum{
	IMP_CALIB_NONE,
	IMP_CALIB_OPEN,
	IMP_CALIB_SHORT,
	IMP_CALIB_LOAD,
	IMP_CALIB_Z_REF,
} imp_calib_e;

typedef enum{
	IMP_MEASURMENT_SWEEP,
	IMP_FREQUENCY_SWEEP,
} imp_sweep_e;

typedef enum{
	IMP_R_SHUNT_30,
	IMP_R_SHUNT_75,
	IMP_R_SHUNT_300,
	IMP_R_SHUNT_750,
	IMP_R_SHUNT_3K,
	IMP_R_SHUNT_7_5K,
	IMP_R_SHUNT_30K,
	IMP_R_SHUNT_80K,
	IMP_R_SHUNT_430K,
	IMP_R_SHUNT_3M,
} imp_r_shunt_e;

/* Main lcr params structure */
typedef struct imp_params_e{
	float amplitude;
	float dc_bias;
	float avg;
	imp_r_shunt_e r_shunt;
	imp_calib_e mode; //lcr_calib_e
	float ref_real;
	float ref_img;
	uint32_t steps;
	float start_freq;
	float end_freq;
	imp_scale_e scale;
	imp_sweep_e sweep; //lcr_sweep_e
	bool user_wait;
} imp_params_t;

/* Resource managment functions */
int imp_Init();
int imp_Release();
int imp_Reset();
int imp_SetDefaultValues();

/* Main lcr functions */
int imp_Run(int measurment);
void *imp_MainThread(int measurment);

/* Measurment functions */
int imp_SafeThreadGen(rp_channel_t channel, float ampl, float freq);

int imp_SafeThreadAcqData(rp_channel_t channel, 
	float *data, rp_acq_decimation_t decimation);

int imp_FreqSweep(float **calib_data);
int imp_MeasSweep(float **calib_data);

float imp_data_analysis(float **data, uint32_t size, float dc_bias, 
		float r_shunt, float complex *Z, float w_out, int decimation);

/* Helper functions */
int imp_GetRshuntFactor(float *r_shunt_factor);
int calculateShunt(float z_ampl);

/* Getters and Setters */
int imp_SetAmplitude(float ampl);
int imp_GetAmplitude(float *ampl);
int imp_SetDcBias(float dc_bias);
int imp_GetDcBias(float *dc_bias);
int imp_SetAveraging(float averaging);
int imp_GetAveraging(float *averaging);
int imp_SetRshunt(imp_r_shunt_e r_shunt);
int imp_GetRShunt(imp_r_shunt_e *r_shunt);
int imp_SetCalibMode(imp_calib_e mode);
int imp_GetCalibMode(imp_calib_e *mode);

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
int imp_SetScaleType(imp_scale_e scale);
int imp_GetScaleType(imp_scale_e *scale);
int imp_SetSweepMode(imp_sweep_e sweep);
int imp_GetSweepMode(imp_sweep_e *sweep);
int imp_SetUserWait(bool wait);
int imp_GetUserWait(bool *wait);

int imp_SetUserView(uint32_t view);
int imp_GetUserView(uint32_t *view);

#endif //__IMPEDANCEANALYZER_H


