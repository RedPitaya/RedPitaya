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

#include "redpitaya/rp.h"

/** @name General
*/
///@{

/** TODO: Function desciption is still to be added. */

/* Impedance analyzer measurment specific functions */
int impApp_ImpInit();
int lcrApp_LcrRun(float *amplitudez);
int impApp_ImpStop();
int impApp_ImpRelease();
int impApp_ImpReset();

/* Getters and setters */
int impApp_ImpSetAmpl(float amplitude);
int impApp_ImpSetAvg(int averaging);
int impApp_ImpSetDcBias(float dc_bias);
int impApp_ImpSetSteps(uint32_t steps);
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
int impApp_ImpGetSteps(uint32_t *steps);
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

#endif //__LCR_APP_H
