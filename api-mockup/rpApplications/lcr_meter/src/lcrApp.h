/**
* $Id: $
*
* @brief Red Pitaya application library lcr meter module interface
*
* @Author Luka Golinar
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#ifndef __LCRMETER_H
#define __LCRMETER_H

#include <complex.h>
#include "../../../rpbase/src/rp.h"

#define AMPLITUDE_MAX			1.0
#define ADC_BUFF_SIZE			16 * 1024
#define M_PI					3.14159265358979323846
#define TRANS_EFFECT_STEPS		10
#define SAMPLE_RATE				125e6

/* Decimation */
#define LCR_DEC_1 				1
#define LCR_DEC_8				8
#define LCR_DEC_64				64
#define LCR_DEC_1024			1024
#define LCR_DEC_8192			8192
#define LCR_DEC_65536			65536

typedef enum{
	LCR_SCALE_LINEAR,
	LCR_SCALE_LOGARITHMIC,
} lcr_scale_e;

typedef enum{
	LCR_CALIB_NONE,
	LCR_CALIB_OPEN,
	LCR_CALIB_SHORT,
	LCR_CALIB_LOAD,
	LCR_CALIB_Z_REF,
} lcr_calib_e;

typedef enum{
	LCR_MEASURMENT_SWEEP,
	LCR_FREQUENCY_SWEEP,
} lcr_sweep_e;

typedef enum{
	LCR_R_SHUNT_30,
	LCR_R_SHUNT_75,
	LCR_R_SHUNT_300,
	LCR_R_SHUNT_750,
	LCR_R_SHUNT_3K,
	LCR_R_SHUNT_7_5K,
	LCR_R_SHUNT_30K,
	LCR_R_SHUNT_80K,
	LCR_R_SHUNT_430K,
	LCR_R_SHUNT_3M,
} lcr_r_shunt_e;

/* Main lcr params structure */
typedef struct lcr_params_t{
	float amplitude;
	float dc_bias;
	float avg;
	lcr_r_shunt_e r_shunt;
	lcr_calib_e mode; //lcr_calib_e
	float ref_real;
	float ref_img;
	uint32_t steps;
	float start_freq;
	float end_freq;
	lcr_scale_e scale;
	lcr_sweep_e sweep; //lcr_sweep_e
	bool user_wait;
} lcr_params_e;

/* Resource managment functions */
int lcr_Init();
int lcr_Release();
int lcr_Reset();
int lcr_SetDefaultValues();

/* Main lcr functions */
int lcr_Run();
void *lcr_MainThread();

/* Measurment functions */
int lcr_SafeThreadGen(rp_channel_t channel, float ampl, float freq);
int lcr_SafeThreadAcqData(rp_channel_t channel, int16_t *data);
int lcr_FreqSweep(int16_t **calib_data);
int lcr_FreqSweep(int16_t **calib_data);

/* Helper functions */
int lcr_GetRshuntFactor(float *r_shunt_factor);

/* Getters and Setters */
int lcr_SetAmplitude(float ampl);
int lcr_GetAmplitude(float *ampl);
int lcr_SetDcBias(float dc_bias);
int lcr_GetDcBias(float *dc_bias);
int lcr_SetAveraging(float averaging);
int lcr_GetAveraging(float *averaging);
int lcr_SetRshunt(lcr_r_shunt_e r_shunt);
int lcr_GetRShunt(lcr_r_shunt_e *r_shunt);
int lcr_SetCalibMode(lcr_calib_e mode);
int lcr_GetCalibMode(lcr_calib_e *mode);

int lcr_SetRefReal(float ref_real);
int lcr_GetRefReal(float *ref_real);
int lcr_SetRefImg(float ref_img);
int lcr_GetRefImg(float *ref_img);

int lcr_SetSteps(uint32_t steps);
int lcr_GetSteps(uint32_t *steps);
int lcr_SetStartFreq(float start_freq);
int lcr_GetStartFreq(float *start_freq);
int lcr_SetEndFreq(float end_freq);
int lcr_GetEndFreq(float *end_freq);
int lcr_SetScaleType(lcr_scale_e scale);
int lcr_GetScaleType(lcr_scale_e *scale);
int lcr_SetSweepMode(lcr_sweep_e sweep);
int lcr_GetSweepMode(lcr_sweep_e *sweep);
int lcr_SetUserWait(bool wait);
int lcr_GetUserWait(bool *wait);

int lcr_SetUserView(uint32_t view);
int lcr_GetUserView(uint32_t *view);

#endif //__LCRMETER_H


