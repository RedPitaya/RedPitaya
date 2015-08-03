/**
* $Id: $
*
* @brief Red Pitaya application lcr module application
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

#include "../../../rpbase/src/rp.h"
#include "lcr_meter.h"

/** @name General
*/
///@{

/** TODO: Function desciption is still to be added. */

/* lcr meter measurment specific functions */
int lcrApp_LcrInit();
int lcrApp_LcrRun(int measurment);
int lcrApp_LcrStop();
int lcrApp_LcrRelease();
int lcrApp_LcrReset();

/* Getters and setters */
int lcrApp_LcrSetAmpl(float amplitude);
int lcrApp_LcrSetAvg(int averaging);
int lcrApp_LcrSetDcBias(float dc_bias);
int lcrApp_LcrSetSteps(int steps);
int lcrApp_LcrSetStartFreq(float s_freq);
int lcrApp_LcrSetEndFreq(float e_freq);
int lcrApp_LcrSetYplot(int y_plot);
int lcrApp_LcrSetScale(int scale);
int lcrApp_LcrSetLoadRe(float load_re);
int lcrApp_LcrSetLoadImg(float load_img);
int lcrApp_LcrSetCalib(int calib);

int lcrApp_LcrGetAmpl(float *amplitude);
int lcrApp_LcrGetAvg(int *averaging);
int lcrApp_LcrGetDcBias(float *dc_bias);
int lcrApp_LcrGetSteps(int *steps);
int lcrApp_LcrGetStartFreq(float *s_freq);
int lcrApp_LcrGetEndFreq(float *e_freq);
int lcrApp_LcrGetYplot(int *y_plot);
int lcrApp_LcrGetScale(int *scale);
int lcrApp_LcrGetLoadRe(float *load_re);
int lcrApp_LcrGetLoadImg(float *load_img);
int lcrApp_LcrGetCalib(int *calib);

int lcrApp_LcrSaveData(bool save);

#ifdef __cplusplus
}
#endif

#endif //__LCR_APP_H