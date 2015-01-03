/**
 * $Id: $
 *
 * @brief Red Pitaya library oscilloscope module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */


#ifndef SRC_OSCILLOSCOPE_H_
#define SRC_OSCILLOSCOPE_H_

#include <stdint.h>
#include <stdbool.h>

int osc_Init();
int osc_Release();


int osc_SetDecimation(uint32_t decimation);
int osc_GetDecimation(uint32_t* decimation);
int osc_SetAveraging(bool enable);
int osc_GetAveraging(bool* enable);
int osc_SetTriggerSource(uint32_t source);
int osc_GetTriggerSource(uint32_t* source);
int osc_WriteDataIntoMemory(bool enable);
int osc_SetThresholdChA(uint32_t threshold);
int osc_GetThresholdChA(uint32_t* threshold);
int osc_SetThresholdChB(uint32_t threshold);
int osc_GetThresholdChB(uint32_t* threshold);
int osc_SetHysteresisChA(uint32_t hysteresis);
int osc_GetHysteresisChA(uint32_t* hysteresis);
int osc_SetHysteresisChB(uint32_t hysteresis);
int osc_GetHysteresisChB(uint32_t* hysteresis);

#endif /* SRC_OSCILLOSCOPE_H_ */
