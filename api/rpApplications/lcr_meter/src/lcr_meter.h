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

#ifndef __LCRMETER_H
#define __LCRMETER_H

#include "redpitaya/rp.h"

#define REPEAT do
#define UNTIL(exp) while(!(exp))

/* Random global defines */
#define ADC_BUFF_SIZE			16384
#define M_PI					3.14159265358979323846
#define SAMPLE_RATE				125e6
#define LCR_AMPLITUDE			0.25

#define APP_PATH	"/opt/redpitaya/www/apps/lcr_meter/"

/* Calibration params */
#define CALIB_STEPS				4
#define START_CORR_FREQ			100
#define END_CORR_FREQ			100000

typedef enum calibration{
	CALIB_NONE,
	CALIB_OPEN,
	CALIB_SHORT,
} calib_t;

/* Main lcr params structure */
typedef struct params_e{
	float frequency;
	int r_shunt;
	calib_t calibration;
	bool tolerance;
	bool range;
	bool series;
} lcr_params_t;

/* Main lcr measurment data */
typedef struct data_e {
	float lcr_amplitude;
	float lcr_phase;
	float lcr_D;
	float lcr_Q;
	float lcr_ESR;
	float lcr_L;
	float lcr_C;
	float lcr_R;
}lcr_main_data_t;


static inline char *stringFromCalib(enum calibration calib)
{
    static char *strings[] = { "CALIB_NONE", 
    						   "CALIB_OPEN", 
    						   "CALIB_SHORT"};

    return strings[calib];
}

/* Resource managment functions */
int lcr_Init();
int lcr_Release();
int lcr_Reset();
int lcr_SetDefaultValues();

/* Main lcr functions */
int lcr_Run();
void *lcr_MainThread();

/* Measurment functions */
int lcr_SafeThreadGen(rp_channel_t channel, float frequency);

int lcr_SafeThreadAcqData(float **data, 
						  rp_acq_decimation_t decimation,
						  int acq_size,
						  int dec);

int lcr_getImpedance(float frequency, 
	                 float _Complex *z_out,
	                 float *phase_out);

int lcr_Correction();
int lcr_CalculateData(float _Complex Z_measured, float phase_measured);
int lcr_CopyParams(lcr_main_data_t *params);

int lcr_data_analysis(float **data, 
	                  uint32_t size, 
	                  float dc_bias, 
		              uint32_t r_shunt,  
		              float w_out, 
		              int decimation,
		              float _Complex *z_out,
		              float *phase_out);

/* Getters and Setters */
int lcr_SetFrequency(float frequency);
int lcr_GetFrequency(float *frequency);
int lcr_setRShunt(int r_shunt);
int lcr_getRShunt(int *r_shunt);
int lcr_SetCalibMode(calib_t mode);
int lcr_GetCalibMode(calib_t *mode);
int lcr_SetMeasSeries(bool serial);
int lcr_GetMeasSeries(bool *serial);
int lcr_SetMeasTolerance(bool tolerance);
int lcr_GetMeasTolerance(bool *tolerance);
int lcr_SetMeasRangeMode(bool range);
int lcr_GetMeasRangeMode(bool *range);

#endif //__LCRMETER_H


