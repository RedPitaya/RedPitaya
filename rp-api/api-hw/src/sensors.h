/**
 * $Id: $
 *
 * @brief Red Pitaya Led System Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef SENSORS_H
#define SENSORS_H

float sens_GetCPUTemp(uint32_t *raw);
int sens_GetPowerI4(uint32_t *raw,float* value);
int sens_GetPowerVCCPINT(uint32_t *raw,float* value);
int sens_GetPowerVCCPAUX(uint32_t *raw,float* value);
int sens_GetPowerVCCBRAM(uint32_t *raw,float* value);
int sens_GetPowerVCCINT(uint32_t *raw,float* value);
int sens_GetPowerVCCAUX(uint32_t *raw,float* value);
int sens_GetPowerVCCDDR(uint32_t *raw,float* value);

#endif