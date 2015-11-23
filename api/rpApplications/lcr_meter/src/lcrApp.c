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

#include "lcrApp.h"
#include "impedance_analyzer.h"
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

int lcrApp_LcrRun(float *amplitudez){
	return lcr_Run(amplitudez);
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

int impApp_ImpSetSteps(uint32_t steps){
	return imp_SetSteps(steps);
}

int impApp_ImpGetSteps(uint32_t *steps){
	return imp_GetSteps(steps);
}

int impApp_ImpSetStartFreq(float s_freq){
	return imp_SetStartFreq(s_freq);
}

int impApp_ImpGetStartFreq(float *s_freq){
	return imp_GetStartFreq(s_freq);
}

int impApp_ImpSetEndFreq(float e_freq){
	return imp_SetEndFreq(e_freq);
}

int impApp_ImpGetEndFreq(float *e_freq){
	return imp_GetEndFreq(e_freq);
}

/* TODO: Implement in first layer - lcr_meter */
int impApp_SetYplot(int y_plot) { return 0; }

int impApp_GetYplot(int y_plot) { return 0; }

int impApp_ImpSetLoadRe(float load_re){
	return imp_SetRefReal(load_re);
}

int impApp_ImpGetLoadRe(float *load_re){
	return imp_GetRefReal(load_re);
}

int impApp_ImpSetLoadImg(float load_img){
	return imp_SetRefImg(load_img);
}

int impApp_ImpGetLoadImg(float *load_img){
	return imp_GetRefImg(load_img);
}

//int impApp_ImpSetCalib(int calib){
//	return imp_SetCalibMode(calib);
//X}

//int impApp_ImpGetCalib(int *calib){
//	return imp_GetCalibMode(calib);
//}



