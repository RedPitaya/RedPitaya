/**
 * $Id: $
 *
 * @brief Red Pitaya library Acquire signal handler interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef SRC_ACQ_HANDLER_H_
#define SRC_ACQ_HANDLER_H_

#include <stdint.h>
#include <stdbool.h>
#include "rp.h"

int acq_SetSamplingRate(rp_acq_sampling_rate_t sampling_rate);
int acq_GetSamplingRate(rp_acq_sampling_rate_t* sampling_rate);
int acq_GetSamplingRateHz(float* sampling_rate);
int acq_SetAveraging(bool enable);
int acq_GetAveraging(bool* enable);

#endif /* SRC_ACQ_HANDLER_H_ */
