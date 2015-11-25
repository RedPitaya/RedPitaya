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
#include "lcr_meter.h"

/** @name General
*/
///@{

int lcrApp_lcrInit();
int lcrApp_LcrRelease();

int lcrApp_LcrRun(float *amplitudez);

//Getters, setters
int lcrApp_LcrSetFrequency(float frequency);
int lcrApp_LcrGetFrequency(float *frequency);

#ifdef __cplusplus
}
#endif

#endif //__LCR_APP_H
