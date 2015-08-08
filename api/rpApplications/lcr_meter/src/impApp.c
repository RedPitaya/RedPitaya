/**
* $Id: $
*
* @brief Red Pitaya application Impendace Analyzer module interface
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

#include "impApp.h"
#include "common.h"

/* Resource managment functions */
int impApp_ImpInit(){
	return imp_Init();
}

int impApp_ImpRelease(){
	return imp_Release();
}

int impApp_ImpReset(){
	return imp_Reset();
}

int impApp_ImpRun(int measurment){
	return imp_Run(measurment);
}

int impApp_ImpSetAmpl(float amplitude){
	return imp_SetAmplitude(amplitude);
}

int impApp_ImpGetAmpl(float *amplitude){
	return imp_GetAmplitude(amplitude);
}

int impApp_ImpSetDcBias(float dc_bias){
	return imp_SetDcBias(dc_bias);
}

int impApp_ImpGetDcBias(float *dc_bias){
	return imp_GetDcBias(dc_bias);
}