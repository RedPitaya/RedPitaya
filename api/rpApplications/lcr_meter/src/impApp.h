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

#ifndef __IMPEDANCE_APP_H
#define __IMPEDANCE_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../rpbase/src/rp.h"
#include "impedance_analyzer.h"

/** @name General
*/
///@{

/** TODO: Function desciption is still to be added. */

/* Impedance analyzer measurment specific functions */
int impApp_ImpInit();
int impApp_ImpRun(int measurment);
int impApp_ImpStop();
int impApp_ImpRelease();
int impApp_ImpReset();

/* Getters and setters */
int impApp_ImpSetAmpl(float amplitude);
int impApp_ImpSetAvg(int averaging);
int impApp_ImpSetDcBias(float dc_bias);
int impApp_ImpSetSteps(int steps);
int impApp_ImpSetStartFreq(float s_freq);
int impApp_ImpSetEndFreq(float e_freq);
int impApp_ImpSetYplot(int y_plot);
int impApp_ImpSetScale(int scale);
int impApp_ImpSetLoadRe(float load_re);
int impApp_ImpSetLoadImg(float load_img);
int impApp_ImpSetCalib(int calib);

int impApp_ImpGetAmpl(float *amplitude);
int impApp_ImpGetAvg(int *averaging);
int impApp_ImpGetDcBias(float *dc_bias);
int impApp_ImpGetSteps(int *steps);
int impApp_ImpGetStartFreq(float *s_freq);
int impApp_ImpGetEndFreq(float *e_freq);
int impApp_ImpGetYplot(int *y_plot);
int impApp_ImpGetScale(int *scale);
int impApp_ImpGetLoadRe(float *load_re);
int impApp_ImpGetLoadImg(float *load_img);
int impApp_ImpGetCalib(int *calib);

/* Impedance anaylzer save data functionality */
int lcrApp_ImpSaveData(bool save);

#ifdef __cplusplus
}
#endif

#endif //__IMPEDANCE_APP_H