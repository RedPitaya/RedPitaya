/**
* $Id: $
*
* @brief Red Pitaya application lcr module interface
*
* @Author Luka Golinar
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#include <stdio.h>

#include "lcrApp.h"
#include "common.h"

/* Resource managment functions */
int lcrApp_LcrInit(){
	return lcr_Init();
}

int lcrApp_LcrRelease(){
	return lcr_Release();
}

int lcrApp_LcrReset(){
	return lcr_Reset();
}

int lcrApp_LcrRun(int measurment){
	return lcr_Run(measurment);
}

int lcrApp_LcrSetAmpl(float amplitude){
	return lcr_SetAmplitude(amplitude);
}

int lcrApp_LcrGetAmpl(float *amplitude){
	return lcr_GetAmplitude(amplitude);
}

int lcrApp_LcrSetDcBias(float dc_bias){
	return lcr_SetDcBias(dc_bias);
}

int lcrApp_LcrGetDcBias(float *dc_bias){
	return lcr_GetDcBias(dc_bias);
}