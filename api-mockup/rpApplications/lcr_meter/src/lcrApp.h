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
#define M_PI					3.14159265358979323846
#define TRANS_EFFECT_STEPS		10

typedef enum{
	LCR_SCALE_LINEAR,
	LCR_SCALE_LOGARITHMIC,
} lcr_scale_e;

typedef enum{
	LCR_CALIB_NONE,
	LCR_CALIB_OPEN,
	LCR_CALIB_SHORT,
	LCR_CALIB_Z_REF,
} lcr_calib_e;

typedef enum{
	LCR_MEASURMENT_SWEEP,
	LCR_FREQUENCY_SWEEP,
} lcr_sweep_e;

/* Main lcr params structure */
typedef struct lcr_params_t{
	float amplitude;
	float dc_bias;
	float r_shunt;
	float avg;
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

/* Functions definitions */
int lcr_Init();
int lcr_Release();
int lcr_Reset();
int lcr_SetDefaultValues();

/* Main lcr function */
int lcr_Run();
int lcr_MainThread();
void *lcr_FreqSweep();
void *lcr_MeasSweep();

/* Measurment functions */
int lcr_SafeThreadGen(rp_channel_t channel, float ampl, float start_freq, 
					  float end_freq, float *data);

int lcr_SafeThreadAcqData(rp_channel_t channel, float *data);


/* Getters and Setters */
int lcr_SetAmplitude(float ampl);
int lcr_GetAmplitude(float *ampl);
int lcr_SetDcBias(float dc_bias);
int lcr_GetDcBias(float *dc_bias);
int lcr_SetRshunt(float r_shunt);
int lcr_GetRshunt(float *r_shunt);
int lcr_SetAveraging(float averaging);
int lcr_GetAveraging(float *averaging);
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

#endif //__LCRMETER_H


