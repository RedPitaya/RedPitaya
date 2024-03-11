/**
* $Id: $
*
* @brief Red Pitaya application impedance module application
*
* @Author Luka Golinar
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#ifndef __LCR_APP_H
#define __LCR_APP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum calibration{
	CALIB_NONE,
	CALIB_OPEN,
	CALIB_SHORT,
} calib_t;

/* Main lcr measurment data */
typedef struct data_e {
	double lcr_amplitude;
	double lcr_phase;
	double lcr_D;
	double lcr_Q;
	double lcr_ESR;
	double lcr_L;
	double lcr_C;
	double lcr_R;
// values for console tool
	double lcr_L_s;
	double lcr_C_s;
	double lcr_R_s;
	double lcr_L_p;
	double lcr_C_p;
	double lcr_R_p;
	double lcr_D_s;
	double lcr_Q_s;
	double lcr_D_p;
	double lcr_Q_p;
	double lcr_X_s;
    double lcr_G_p;
    double lcr_B_p;
	double lcr_Y_abs;
	double lcr_Phase_Y;

}lcr_main_data_t;


/** @name General
*/
///@{

int lcrApp_lcrInit();
int lcrApp_LcrRelease();

int lcrApp_LcrRun();
int lcrApp_GenRun();
int lcrApp_GenStop();
// int lcrApp_LcrStartCorrection();
int lcrApp_LcrCopyParams(lcr_main_data_t *data);
bool lcrApp_LcrIsSine();

//Getters, setters
int lcrApp_LcrSetFrequency(float frequency);
int lcrApp_LcrGetFrequency(float *frequency);
int lcrApp_LcrSetShunt(int shunt);
int lcrApp_LcrGetShunt(int *shunt);
int lcrApp_LcrSetShuntIsAuto(bool isShuntAuto);
int lcrApp_LcrSetCalibMode(calib_t calib_mode);
int lcrApp_LcrGetCalibMode(calib_t *calib_mode);
int lcrApp_LcrSetMeasSeries(bool series);
int lcrApp_LcrGetMeasSeries(bool *series);
int lcrApp_LcrSetMeasTolerance(int tolerance);
int lcrApp_LcrGetMeasTolerance(int *tolerance);
int lcrApp_LcrSetMeasRangeMode(int range_mode);
int lcrApp_LcrGetMeasRangeMode(int *range_mode);
int lcrApp_LcrSetMeasRangeFormat(int format);
int lcrApp_LcrGetMeasRangeFormat(int *format);
int lcrApp_LcrSetMeasRangeUnits(int units);
int lcrApp_LcrGetMeasRangeUnits(int *units);
int lcrApp_LcrCheckExtensionModuleConnection();

#ifdef __cplusplus
}
#endif

#endif //__LCR_APP_H
