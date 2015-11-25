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
#define LCR_AMPLITUDE			0.5


/* Calibration params */
#define CALIB_STEPS				100
#define START_CALIB_FREQ		100 //TODO
#define END_CALIB_FREQ			1e6

typedef enum calibration{
	CALIB_NONE,
	CALIB_OPEN,
	CALIB_SHORT,
	CALIB_LOAD,
} calib_t;

/* Main lcr params structure */
typedef struct params_e{
	float frequency;
	uint32_t r_shunt;
	calib_t calibration;
	bool tolerance;
} lcr_params_t;


static inline char *stringFromCalib(enum calibration calib)
{
    static char *strings[] = { "CALIB_NONE", 
    						   "CALIB_OPEN", 
    						   "CALIB_SHORT", 
    						   "CALIB_LOAD"};

    return strings[calib];
}

/* Resource managment functions */
int lcr_Init();
int lcr_Release();
int lcr_Reset();
int lcr_SetDefaultValues();

/* Main lcr functions */
int lcr_Run(float *amplitude);
void *lcr_MainThread();

/* Measurment functions */
int lcr_SafeThreadGen(rp_channel_t channel, float frequency);

int lcr_SafeThreadAcqData(rp_channel_t channel, 
	float *data, rp_acq_decimation_t decimation);

int lcr_getImpedance(float frequency, float *Z_out);
int lcr_Correction();

int lcr_data_analysis(float **data, uint32_t size, float dc_bias, 
		uint32_t r_shunt, float _Complex *Z, float w_out, int decimation);



/* Getters and Setters */
int lcr_SetFrequency(float frequency);
int lcr_GetFrequency(float *frequency);
int lcr_setRShunt(uint32_t r_shunt);
int lcr_getRShunt(uint32_t *r_shunt);
int lcr_SetCalibMode(calib_t mode);
int lcr_GetCalibMode(calib_t *mode);

#endif //__IMPEDANCEANALYZER_H


