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
#include <stdint.h>
#include <stdbool.h>
#include <string>

#include "lcrApp.h"
#include "rp.h"

#define LCR_AMPLITUDE			0.25

#define APP_PATH				"/opt/redpitaya/www/apps/lcr_meter/"

/* Calibration params */
#define CALIB_STEPS				4
#define START_CORR_FREQ			100
#define END_CORR_FREQ			100000

#define MAX_ADC_CHANNELS 4

/* Main lcr params structure */
typedef struct params_e{
	float   frequency;
	int     r_shunt;
	calib_t calibration;
	bool    tolerance;
	int     range;
	int     range_format;
	int     range_units;
	bool    series;
} lcr_params_t;


static inline const char *stringFromCalib(enum calibration calib)
{
    static std::string strings[] = { "CALIB_NONE", 
    						   "CALIB_OPEN", 
    						   "CALIB_SHORT"};

    return strings[(int)calib].c_str();
}

/* Resource managment functions */
int   lcr_Init();
int   lcr_Release();
int   lcr_Reset();
int   lcr_SetDefaultValues();

/* Main lcr functions */
int   lcr_Run();
int   lcr_Stop();
int   lcr_GenRun();
int   lcr_GenStop();
void *lcr_MainThread(void*);

/* Measurment functions */
int   lcr_SafeThreadGen(rp_channel_t channel, float frequency);
int   lcr_ThreadAcqData(float **data,uint32_t *acq_size);
int   lcr_getImpedance(const float **data,const uint32_t acq_size);
void  lcr_CheckShunt(const float **data,const uint32_t acq_size);

// int lcr_Correction();
int lcr_CalculateData(float _Complex Z_measured, float phase_measured);
int lcr_CopyParams(lcr_main_data_t *params);
bool lcr_isSine();

int lcr_data_analysis(const float **data, 
                      uint32_t size,
                      float dc_bias,
                      double r_shunt,
                      float w_out,
                      int decimation);

/* Getters and Setters */
int lcr_SetFrequency(float frequency);
int lcr_GetFrequency(float *frequency);
int lcr_setRShunt(int r_shunt);
int lcr_getRShunt(int *r_shunt);
int lcr_setRShuntIsAuto(bool isAuto);
int lcr_SetCalibMode(calib_t mode);
int lcr_GetCalibMode(calib_t *mode);
int lcr_SetMeasSeries(bool serial);
int lcr_GetMeasSeries(bool *serial);
int lcr_SetMeasTolerance(int tolerance);
int lcr_GetMeasTolerance(int *tolerance);
int lcr_SetMeasRangeMode(int range);
int lcr_GetMeasRangeMode(int *range);
int lcr_SetRangeFormat(int format);
int lcr_GetRangeFormat(int *format);
int lcr_SetRangeUnits(int units);
int lcr_GetRangeUnits(int *units);

int lcr_CheckModuleConnection();

#endif //__LCRMETER_H


