/**
* $Id: $
*
* @brief Red Pitaya application Impedance Analyzer module interface
*
* @Author Luka Golinar <luka.golinar@redpitaya.com>
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#include <stdio.h>
#include <sys/syslog.h>

#include "lcrApp.h"
#include "common.h"

/* Resource managment functions */
int lcrApp_lcrInit(){
	return lcr_Init();
}

int lcrApp_LcrRelease(){
	return lcr_Release();
}

int lcrApp_LcrReset(){
	return lcr_Reset();
}

int lcrApp_LcrRun(){
	return lcr_Run();
}

int lcrApp_LcrCopyParams(lcr_main_data_t *data){
	return lcr_CopyParams(data);
}

int lcrApp_LcrStartCorrection(){
	return lcr_Correction();
}

int lcrApp_LcrSetFrequency(float frequency){
	return lcr_SetFrequency(frequency);
}

int lcrApp_LcrGetFrequency(float *frequency){
	return lcr_GetFrequency(frequency);
}

int lcrApp_LcrSetCalibMode(calib_t calib_mode){
	return lcr_SetCalibMode(calib_mode);
}

int lcrApp_LcrGetCalibMode(calib_t *calib_mode){
	return lcr_GetCalibMode(calib_mode);
}

int lcrApp_LcrSetMeasSeries(bool series){
	return lcr_SetMeasSeries(series);
}

int lcrApp_LcrGetMeasSeries(bool *series){
	return lcr_GetMeasSeries(series);
}

int lcrApp_LcrSetMeasTolerance(bool tolerance){
	return lcr_SetMeasTolerance(tolerance);
}

int lcrApp_LcrGetMeasTolerance(bool *tolerance){
	return lcr_GetMeasTolerance(tolerance);
}

int lcrApp_LcrSetMeasRangeMode(bool range_mode){
	return lcr_SetMeasRangeMode(range_mode);
}

int lcrApp_LcrGetMeasRangeMode(bool *range_mode){
	return lcr_GetMeasRangeMode(range_mode);
}
