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

int acq_SetGain(rp_channel_t channel, rp_pinState_t state);
int acq_GetGain(rp_channel_t channel, rp_pinState_t* state);
int acq_GetGainV(rp_channel_t channel, float* voltage);
int acq_SetDecimation(rp_acq_decimation_t decimation);
int acq_GetDecimation(rp_acq_decimation_t* decimation);
int acq_GetDecimationFactor(uint32_t* decimation);
int acq_SetSamplingRate(rp_acq_sampling_rate_t sampling_rate);
int acq_GetSamplingRate(rp_acq_sampling_rate_t* sampling_rate);
int acq_GetSamplingRateHz(float* sampling_rate);
int acq_SetAveraging(bool enable);
int acq_GetAveraging(bool* enable);
int acq_SetTriggerSrc(rp_acq_trig_src_t source);
int acq_GetTriggerSrc(rp_acq_trig_src_t* source);
int acq_SetTriggerDelay(uint32_t decimated_data_num, bool updateMaxValue);
int acq_GetTriggerDelay(uint32_t* decimated_data_num);
int acq_SetTriggerDelayNs(uint64_t time_ns, bool updateMaxValue);
int acq_GetTriggerDelayNs(uint64_t* time_ns);
int acq_SetChannelThreshold(rp_channel_t channel, float voltage);
int acq_GetChannelThreshold(rp_channel_t channel, float* voltage);
int acq_SetChannelThresholdHyst(rp_channel_t channel, float voltage);
int acq_GetChannelThresholdHyst(rp_channel_t channel, float* voltage);
int acq_GetWritePointer(uint32_t* pos);
int acq_GetWritePointerAtTrig(uint32_t* pos);
int acq_Start();
int acq_Stop();

int acq_GetDataRaw(rp_channel_t channel,  uint32_t pos, uint32_t size, uint16_t* buffer);
int acq_GetOldestDataRaw(rp_channel_t channel, uint32_t size, uint16_t* buffer);
int acq_GetLatestDataRaw(rp_channel_t channel, uint32_t size, uint16_t* buffer);
int acq_GetDataV(rp_channel_t channel,  uint32_t pos, uint32_t size, float* buffer);
int acq_GetOldestDataV(rp_channel_t channel, uint32_t size, float* buffer);
int acq_GetLatestDataV(rp_channel_t channel, uint32_t size, float* buffer);


int acq_SetDefault();


#endif /* SRC_ACQ_HANDLER_H_ */
