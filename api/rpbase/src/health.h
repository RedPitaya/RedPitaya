/**
 * $Id: $
 *
 * @brief Red Pitaya library Health module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __HEALTH_H
#define __HEALTH_H

int health_Init();
int health_Release();

int health_GetTemperature(float* value);
int health_GetVccPint(float* value);
int health_GetVccPaux(float* value);
int health_GetVccBram(float* value);
int health_GetVccInt(float* value);
int health_GetVccAux(float* value);
int health_GetVccDdr(float* value);

#endif //__HEALTH_H
