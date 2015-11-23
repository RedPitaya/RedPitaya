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

#include <complex.h> 

#include "redpitaya/rp.h"
#include "lcrApp.h"

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
	SCALE_LINEAR,
	SCALE_LOGARITHMIC,
} scale_t;

typedef enum calibration{
	CALIB_NONE,
	CALIB_OPEN,
	CALIB_SHORT,
	CALIB_LOAD,
} calib_t;

typedef enum{
	MEASURMENT_SWEEP,
	FREQUENCY_SWEEP,
} sweep_t;

/* Main lcr params structure */
typedef struct params_e{
	float amplitude;
	float dc_bias;
	int avg;
	uint32_t r_shunt;
	calib_t calibration;
	float ref_real;
	float ref_img;
	uint32_t steps;
	float start_freq;
	float end_freq;
	scale_t scale;
	sweep_t sweep; //imp_sweep_e
	bool user_wait;
	bool no_calib;
} params_t;


static inline char *stringFromCalib(enum calibration calib)
{
    static char *strings[] = { "CALIB_NONE", 
    						   "CALIB_OPEN", 
    						   "CALIB_SHORT", 
    						   "CALIB_LOAD"};

    return strings[calib];
}

/* Resource managment functions */
int imp_Init();
int imp_Release();
int imp_Reset();
int imp_SetDefaultValues();

/* Main lcr functions */
int lcr_Run(float *amplitude);
void *imp_MainThread();

/* Measurment functions */
int _SafeThreadGen(rp_channel_t channel, float ampl, float freq);

int _SafeThreadAcqData(rp_channel_t channel, 
	float *data, rp_acq_decimation_t decimation);

int main_sweep(float *ampl_z_out);

int _data_analysis(float **data, uint32_t size, float dc_bias, 
		uint32_t r_shunt, float _Complex *Z, float w_out, int decimation);

/* Helper functions */
uint32_t imp_shuntAlgorithm(float z_ampl);

/* Getters and Setters */
int imp_SetAmplitude(float ampl);
int imp_GetAmplitude(float *ampl);
int imp_SetDcBias(float dc_bias);
int imp_GetDcBias(float *dc_bias);
int imp_SetAveraging(float averaging);
int imp_GetAveraging(float *averaging);
int _setRShunt(uint32_t r_shunt);
int _getRShunt(uint32_t *r_shunt);
int imp_SetCalibMode(calib_t mode);
int imp_GetCalibMode(calib_t *mode);
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
int imp_SetScaleType(scale_t scale);
int imp_GetScaleType(scale_t *scale);
int imp_SetSweepMode(sweep_t sweep);
int imp_GetSweepMode(sweep_t *sweep);
int imp_SetUserWait(bool wait);
int imp_GetUserWait(bool *wait);

int imp_SetUserView(uint32_t view);
int imp_GetUserView(uint32_t *view);

#endif //__IMPEDANCEANALYZER_H


