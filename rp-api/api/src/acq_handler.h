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

#include <stdbool.h>
#include <stdint.h>
#include <span>
#include <vector>
#include "rp.h"

//#define ADC_SAMPLE_PERIOD_DEF ((double)1e9/(double)ADC_SAMPLE_RATE)
/* @brief Sampling period (non-decimated) - 8 [ns]. */
//static const uint64_t ADC_SAMPLE_PERIOD = ADC_SAMPLE_PERIOD_DEF;

int acq_SetSplitTriggerMode(bool enable);
int acq_GetSplitTriggerMode(bool* state);
int acq_SetArmKeep(rp_channel_t channel, bool enable);
int acq_GetArmKeep(rp_channel_t channel, bool* state);
int acq_GetBufferFillState(rp_channel_t channel, bool* state);
int acq_SetGain(rp_channel_t channel, rp_pinState_t state);
int acq_GetGain(rp_channel_t channel, rp_pinState_t* state);
int acq_GetGainV(rp_channel_t channel, float* voltage);
int acq_SetDecimation(rp_channel_t channel, rp_acq_decimation_t decimation);
int acq_GetDecimation(rp_channel_t channel, rp_acq_decimation_t* decimation);
int acq_SetDecimationFactor(rp_channel_t channel, uint32_t decimation);
int acq_GetDecimationFactor(rp_channel_t channel, uint32_t* decimation);
int acq_ConvertFactorToDecimation(uint32_t factor, rp_acq_decimation_t* decimation);
int acq_GetSamplingRateHz(rp_channel_t channel, float* sampling_rate);
int acq_SetAveraging(rp_channel_t channel, bool enable);
int acq_GetAveraging(rp_channel_t channel, bool* enable);
int acq_SetTriggerSrc(rp_channel_t channel, rp_acq_trig_src_t source);
int acq_GetTriggerSrc(rp_channel_t channel, rp_acq_trig_src_t* source);
int acq_GetTriggerState(rp_channel_t channel, rp_acq_trig_state_t* state);
int acq_SetTriggerDelay(rp_channel_t channel, int32_t decimated_data_num);
int acq_GetTriggerDelay(rp_channel_t channel, int32_t* decimated_data_num);
int acq_SetTriggerDelayNs(rp_channel_t channel, int64_t time_ns);
int acq_GetTriggerDelayNs(rp_channel_t channel, int64_t* time_ns);
int acq_SetTriggerDelayDirect(rp_channel_t channel, uint32_t decimated_data_num);
int acq_GetTriggerDelayDirect(rp_channel_t channel, uint32_t* decimated_data_num);
int acq_SetTriggerDelayNsDirect(rp_channel_t channel, uint64_t time_ns);
int acq_GetTriggerDelayNsDirect(rp_channel_t channel, uint64_t* time_ns);

int acq_SetTriggerLevel(rp_channel_trigger_t channel, float voltage);
int acq_GetTriggerLevel(rp_channel_trigger_t channel, float* voltage);
int acq_GetPreTriggerCounter(rp_channel_t channel, uint32_t* value);
int acq_SetChannelThreshold(rp_channel_t channel, float voltage);
int acq_GetChannelThreshold(rp_channel_t channel, float* voltage);
int acq_SetTriggerHyst(float voltage);
int acq_GetTriggerHyst(float* voltage);
int acq_SetChannelThresholdHyst(rp_channel_t channel, float voltage);
int acq_GetChannelThresholdHyst(rp_channel_t channel, float* voltage);
int acq_GetWritePointer(rp_channel_t channel, uint32_t* pos);
int acq_GetWritePointerAtTrig(rp_channel_t channel, uint32_t* pos);

int acq_Start(rp_channel_t channel);
int acq_Stop(rp_channel_t channel);
int acq_Reset(rp_channel_t channel);
int acq_ResetFpga();
int acq_SetUnlockTrigger(rp_channel_t channel);
int acq_GetUnlockTrigger(rp_channel_t channel, bool* state);
int acq_GetADCSamplePeriod(double* value);

int acq_GetDataPosRaw(rp_channel_t channel, uint32_t start_pos, uint32_t end_pos, int16_t* buffer, uint32_t* buffer_size);
int acq_GetDataPosV(rp_channel_t channel, uint32_t start_pos, uint32_t end_pos, float* buffer, uint32_t* buffer_size);
int acq_GetDataRaw(rp_channel_t channel, uint32_t pos, uint32_t* size, int16_t* buffer, bool use_calib);
int acq_GetOldestDataRaw(rp_channel_t channel, uint32_t* size, int16_t* buffer);
int acq_GetLatestDataRaw(rp_channel_t channel, uint32_t* size, int16_t* buffer);
int acq_GetDataV(rp_channel_t channel, uint32_t pos, uint32_t* size, float* buffer);
int acq_GetOldestDataV(rp_channel_t channel, uint32_t* size, float* buffer);
int acq_GetLatestDataV(rp_channel_t channel, uint32_t* size, float* buffer);

int acq_GetBufferSize(uint32_t* size);

int acq_SetDefaultAll();
int acq_SetDefault(rp_channel_t channel);

uint32_t acq_GetNormalizedDataPos(uint32_t pos);

int acq_GetData(uint32_t pos, buffers_t* out);
int acq_GetDataWithCorrection(uint32_t pos, uint32_t* size, int32_t offset, buffers_t* out);

int acq_SetAC_DC(rp_channel_t channel, rp_acq_ac_dc_mode_t mode);
int acq_GetAC_DC(rp_channel_t channel, rp_acq_ac_dc_mode_t* status);

int acq_SetEqFilterBypass(rp_channel_t channel, bool enable);
int acq_GetEqFilterBypass(rp_channel_t channel, bool* enable);

int acq_UpdateAcqFilter(rp_channel_t channel);
int acq_GetFilterCalibValue(rp_channel_t channel, uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp);

int acq_SetCalibInFPGA(rp_channel_t channel);
int acq_GetCalibInFPGA(rp_channel_t channel, bool* state);

int acq_SetExtTriggerDebouncerUs(double value);
int acq_GetExtTriggerDebouncerUs(double* value);

int acq_SetOffset(rp_channel_t channel, float voltage);
int acq_GetOffset(rp_channel_t channel, float* voltage);

int acq_axi_Enable(rp_channel_t channel, bool enable);
int acq_axi_GetMemoryRegion(uint32_t* _start, uint32_t* _size);
int acq_axi_GetBufferFillState(rp_channel_t channel, bool* state);
int acq_axi_SetOffset(rp_channel_t channel, float voltage);
int acq_axi_GetOffset(rp_channel_t channel, float* voltage);
int acq_axi_SetDecimationFactor(rp_channel_t channel, uint32_t decimation);
int acq_axi_GetDecimationFactor(rp_channel_t channel, uint32_t* decimation);
int acq_axi_SetTriggerDelay(rp_channel_t channel, int32_t decimated_data_num);
int acq_axi_GetTriggerDelay(rp_channel_t channel, int32_t* decimated_data_num);
int acq_axi_SetTriggerDelayNs(rp_channel_t channel, int64_t time_ns);
int acq_axi_GetTriggerDelayNs(rp_channel_t channel, int64_t* time_ns);
int acq_axi_GetWritePointer(rp_channel_t channel, uint32_t* pos);
int acq_axi_GetWritePointerAtTrig(rp_channel_t channel, uint32_t* pos);
int acq_axi_SetBufferSamples(rp_channel_t channel, uint32_t address, uint32_t _samples);
int acq_axi_SetBufferBytes(rp_channel_t channel, uint32_t address, uint32_t _size);
int acq_axi_GetDataRaw(rp_channel_t channel, uint32_t pos, uint32_t* size, int16_t* buffer);
int acq_axi_GetDataV(rp_channel_t channel, uint32_t pos, uint32_t* size, float* buffer);
int acq_axi_GetDataRawDirect(rp_channel_t channel, uint32_t pos, uint32_t size, std::vector<std::span<int16_t>>* data);

#endif /* SRC_ACQ_HANDLER_H_ */
