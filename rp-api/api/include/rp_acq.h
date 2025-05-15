/**
 * $Id: $
 *
 * @file rp_acq.h
 * @brief Red Pitaya library API interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __RP_ACQ_H
#define __RP_ACQ_H

#include <stdbool.h>
#include <stdint.h>
#include "rp_enums.h"

/** @name Acquire
 */
///@{

/**
 * Enables the mode when triggers in the oscilloscope operate independently in the FPGA.
 * @param enable True for enabling and false disabling
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetSplitTrigger(bool enable);

/**
 * Returns the split mode state of the trigger.
 * @param state Returns status
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetSplitTrigger(bool* state);

/**
 * This mode makes it possible to call the rp_Acq*Ch function even if trigger sharing is not supported. Then these functions work as usual.
 * @param enable True for enabling and false disabling
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetSplitTriggerPass(bool enable);

/**
 * Returns the state of function forwarding.
 * @param state Returns status
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetSplitTriggerPass(bool* state);

/**
 * Enables continous acquirement even after trigger has happened.
 * @param enable True for enabling and false disabling
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetArmKeep(bool enable);

/**
 * Gets status of continous acquirement even after trigger has happened.
 * @param state Returns status
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetArmKeep(bool* state);

/**
 * Enables continous acquirement even after trigger has happened.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param enable True for enabling and false disabling
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetArmKeepCh(rp_channel_t channel, bool enable);

/**
 * Gets status of continous acquirement even after trigger has happened.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param state Returns status
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetArmKeepCh(rp_channel_t channel, bool* state);

/**
 * Indicates whether the ADC buffer was full of data. The length of the buffer is determined by the delay. By default, the delay is half the buffer.
 * @param state Returns status
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetBufferFillState(bool* state);

/**
 * Indicates whether the ADC buffer was full of data. The length of the buffer is determined by the delay. By default, the delay is half the buffer.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param state Returns status
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetBufferFillStateCh(rp_channel_t channel, bool* state);

/**
 * Sets the decimation used at acquiring signal. There is only a set of pre-defined decimation
 * values which can be specified. See the #rp_acq_decimation_t enum values.
 * @param decimation Specify one of pre-defined decimation values
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetDecimation(rp_acq_decimation_t decimation);

/**
 * Sets the decimation used at acquiring signal. There is only a set of pre-defined decimation
 * values which can be specified. See the #rp_acq_decimation_t enum values.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param decimation Specify one of pre-defined decimation values
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetDecimationCh(rp_channel_t channel, rp_acq_decimation_t decimation);

/**
 * Gets the decimation used at acquiring signal. There is only a set of pre-defined decimation
 * values which can be specified. See the #rp_acq_decimation_t enum values.
 * @param decimation Returns one of pre-defined decimation values which is currently set.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDecimation(rp_acq_decimation_t* decimation);

/**
 * Gets the decimation used at acquiring signal. There is only a set of pre-defined decimation
 * values which can be specified. See the #rp_acq_decimation_t enum values.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param decimation Returns one of pre-defined decimation values which is currently set.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDecimationCh(rp_channel_t channel, rp_acq_decimation_t* decimation);

/**
 * Convert factor to decimation used at acquiring signal. There is only a get of pre-defined decimation
 * values which can be specified. See the #rp_acq_decimation_t enum values.
 * @param factor Decimation factor.
 * @param decimation Returns one of pre-defined decimation values which is currently set.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqConvertFactorToDecimation(uint32_t factor, rp_acq_decimation_t* decimation);

/**
 * Sets the decimation used at acquiring signal.
 * You can specify values in the range (1,2,4,8,16-65536)
 * @param decimation Decimation values
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetDecimationFactor(uint32_t decimation);

/**
 * Gets the decimation factor used at acquiring signal in a numerical form. Although this method returns an integer
 * value representing the current factor of the decimation, there is only a set of pre-defined decimation
 * factor values which can be returned. See the #rp_acq_decimation_t enum values.
 * @param decimation Returns decimation factor value which is currently set.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDecimationFactor(uint32_t* decimation);

/**
 * Sets the decimation used at acquiring signal.
 * You can specify values in the range (1,2,4,8,16-65536)
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param decimation Decimation values
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetDecimationFactorCh(rp_channel_t channel, uint32_t decimation);

/**
 * Gets the decimation factor used at acquiring signal in a numerical form. Although this method returns an integer
 * value representing the current factor of the decimation, there is only a set of pre-defined decimation
 * factor values which can be returned. See the #rp_acq_decimation_t enum values.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param decimation Returns decimation factor value which is currently set.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDecimationFactorCh(rp_channel_t channel, uint32_t* decimation);

/**
 * Gets the sampling rate for acquiring signal in a numerical form in Hz. Although this method returns a float
 * value representing the current value of the sampling rate, there is only a set of pre-defined sampling rate
 * values which can be returned. See the #rp_acq_sampling_rate_t enum values.
 * @param sampling_rate returns currently set sampling rate in Hz
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetSamplingRateHz(float* sampling_rate);

/**
 * Gets the sampling rate for acquiring signal in a numerical form in Hz. Although this method returns a float
 * value representing the current value of the sampling rate, there is only a set of pre-defined sampling rate
 * values which can be returned. See the #rp_acq_sampling_rate_t enum values.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param sampling_rate returns currently set sampling rate in Hz
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetSamplingRateHzCh(rp_channel_t channel, float* sampling_rate);

/**
 * Enables or disables averaging of data between samples.
 * Data between samples can be averaged by setting the averaging flag in the Data decimation register.
 * @param enabled When true, the averaging is enabled, otherwise it is disabled.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetAveraging(bool enable);

/**
 * Returns information if averaging of data between samples is enabled or disabled.
 * Data between samples can be averaged by setting the averaging flag in the Data decimation register.
 * @param enabled Set to true when the averaging is enabled, otherwise is it set to false.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetAveraging(bool* enable);

/**
 * Adds a voltage offset when requesting data. Only affects float and double data types. Raw data remains unchanged.
 * @param channel Channel A, B, C or D
 * @param value Offset value in volts
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetOffset(rp_channel_t channel, float value);

/**
 * Returns the offset value.
 * @param channel Channel A, B, C or D
 * @param value Offset value in volts
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetOffset(rp_channel_t channel, float* value);

/**
 * Enables or disables averaging of data between samples.
 * Data between samples can be averaged by setting the averaging flag in the Data decimation register.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param enabled When true, the averaging is enabled, otherwise it is disabled.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetAveragingCh(rp_channel_t channel, bool enable);

/**
 * Returns information if averaging of data between samples is enabled or disabled.
 * Data between samples can be averaged by setting the averaging flag in the Data decimation register.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param enabled Set to true when the averaging is enabled, otherwise is it set to false.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetAveragingCh(rp_channel_t channel, bool* enable);

/**
 * Sets the trigger source used at acquiring signal. When acquiring is started,
 * the FPGA waits for the trigger condition on the specified source and when the condition is met, it
 * starts writing the signal to the buffer.
 * @param source Trigger source.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerSrc(rp_acq_trig_src_t source);

/**
 * Sets the trigger source used at acquiring signal. When acquiring is started,
 * the FPGA waits for the trigger condition on the specified source and when the condition is met, it
 * starts writing the signal to the buffer.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param source Trigger source.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerSrcCh(rp_channel_t channel, rp_acq_trig_src_t source);

/**
 * Gets the trigger source used at acquiring signal. When acquiring is started,
 * the FPGA waits for the trigger condition on the specified source and when the condition is met, it
 * starts writing the signal to the buffer.
 * @param source Currently set trigger source.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerSrc(rp_acq_trig_src_t* source);

/**
 * Gets the trigger source used at acquiring signal. When acquiring is started,
 * the FPGA waits for the trigger condition on the specified source and when the condition is met, it
 * starts writing the signal to the buffer.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param source Currently set trigger source.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerSrcCh(rp_channel_t channel, rp_acq_trig_src_t* source);

/**
 * Returns the trigger state. Either it is waiting for a trigger to happen, or it has already been triggered.
 * By default it is in the triggered state, which is treated the same as disabled.
 * @param state Trigger state
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerState(rp_acq_trig_state_t* state);

/**
 * Returns the trigger state. Either it is waiting for a trigger to happen, or it has already been triggered.
 * By default it is in the triggered state, which is treated the same as disabled.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param state Trigger state
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerStateCh(rp_channel_t channel, rp_acq_trig_state_t* state);

/**
 * Sets the number of decimated data after trigger written into memory.
 * @param decimated_data_num Number of decimated data. It must not be higher than the ADC buffer size.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerDelay(int32_t decimated_data_num);

/**
 * Sets the number of decimated data after trigger written into memory.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param decimated_data_num Number of decimated data. It must not be higher than the ADC buffer size.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerDelayCh(rp_channel_t channel, int32_t decimated_data_num);

/**
 * Returns current number of decimated data after trigger written into memory.
 * @param decimated_data_num Number of decimated data.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerDelay(int32_t* decimated_data_num);

/**
 * Returns current number of decimated data after trigger written into memory.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param decimated_data_num Number of decimated data.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerDelayCh(rp_channel_t channel, int32_t* decimated_data_num);

/**
 * Sets the number of decimated data after trigger written into memory.
 * @param decimated_data_num Number of decimated data. It must not be higher than the ADC buffer size.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerDelayDirect(uint32_t decimated_data_num);

/**
 * Sets the number of decimated data after trigger written into memory.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param decimated_data_num Number of decimated data. It must not be higher than the ADC buffer size.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerDelayDirectCh(rp_channel_t channel, uint32_t decimated_data_num);

/**
 * Returns current number of decimated data after trigger written into memory.
 * @param decimated_data_num Number of decimated data.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerDelayDirect(uint32_t* decimated_data_num);

/**
 * Returns current number of decimated data after trigger written into memory.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param decimated_data_num Number of decimated data.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerDelayDirectCh(rp_channel_t channel, uint32_t* decimated_data_num);

/**
 * Sets the amount of decimated data in nanoseconds after trigger written into memory.
 * @param time_ns Time in nanoseconds. Number of ADC samples within the specified
 * time must not be higher than the ADC buffer size.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerDelayNs(int64_t time_ns);

/**
 * Sets the amount of decimated data in nanoseconds after trigger written into memory.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param time_ns Time in nanoseconds. Number of ADC samples within the specified
 * time must not be higher than the ADC buffer size.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerDelayNsCh(rp_channel_t channel, int64_t time_ns);

/**
 * Returns the current amount of decimated data in nanoseconds after trigger written into memory.
 * @param time_ns Time in nanoseconds.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerDelayNs(int64_t* time_ns);

/**
 * Returns the current amount of decimated data in nanoseconds after trigger written into memory.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param time_ns Time in nanoseconds.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerDelayNsCh(rp_channel_t channel, int64_t* time_ns);

/**
 * Sets the amount of decimated data in nanoseconds after trigger written into memory.
 * @param time_ns Time in nanoseconds. Number of ADC samples within the specified
 * time must not be higher than the ADC buffer size.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerDelayNsDirect(uint64_t time_ns);

/**
 * Sets the amount of decimated data in nanoseconds after trigger written into memory.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param time_ns Time in nanoseconds. Number of ADC samples within the specified
 * time must not be higher than the ADC buffer size.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerDelayNsDirectCh(rp_channel_t channel, uint64_t time_ns);

/**
 * Returns the current amount of decimated data in nanoseconds after trigger written into memory.
 * @param time_ns Time in nanoseconds.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerDelayNsDirect(uint64_t* time_ns);

/**
 * Returns the current amount of decimated data in nanoseconds after trigger written into memory.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param time_ns Time in nanoseconds.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerDelayNsDirectCh(rp_channel_t channel, uint64_t* time_ns);

/**
 * Returns the number of valid data ponts before trigger.
 * @param time_ns number of data points.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetPreTriggerCounter(uint32_t* value);

/**
 * Returns the number of valid data ponts before trigger.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param time_ns number of data points.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetPreTriggerCounterCh(rp_channel_t channel, uint32_t* value);

/**
 * Sets the trigger threshold value in volts. Makes the trigger when ADC value crosses this value.
 * @param voltage Threshold value for the channel
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerLevel(rp_channel_trigger_t channel, float voltage);

/**
 * Gets currently set trigger threshold value in volts
 * @param voltage Current threshold value for the channel
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerLevel(rp_channel_trigger_t channel, float* voltage);

/**
 * Sets the trigger threshold hysteresis value in volts.
 * Value must be outside to enable the trigger again.
 * @param voltage Threshold hysteresis value for the channel
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerHyst(float voltage);

/**
 * Gets currently set trigger threshold hysteresis value in volts
 * @param voltage Current threshold hysteresis value for the channel
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerHyst(float* voltage);

/**
 * Sets the acquire gain state. The gain should be set to the same value as it is set on the Red Pitaya
 * hardware by the LV/HV gain jumpers. LV = 1V; HV = 20V.
 * @param channel Channel A or B
 * @param state High or Low state
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetGain(rp_channel_t channel, rp_pinState_t state);

/**
 * Returns the currently set acquire gain state in the library. It may not be set to the same value as
 * it is set on the Red Pitaya hardware by the LV/HV gain jumpers. LV = 1V; HV = 20V.
 * @param channel Channel A or B
 * @param state Currently set High or Low state in the library.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetGain(rp_channel_t channel, rp_pinState_t* state);

/**
 * Returns the currently set acquire gain in the library. It may not be set to the same value as
 * it is set on the Red Pitaya hardware by the LV/HV gain jumpers. Returns value in Volts.
 * @param channel Channel A or B
 * @param voltage Currently set gain in the library. 1.0 or 20.0 Volts
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetGainV(rp_channel_t channel, float* voltage);

/**
 * Returns current position of ADC write pointer.
 *
 * The write pointer position is the index, within the ADC buffer, of the last
 * array cell that has been written to.
 * @note The ADC buffer is a ring buffer. When it is full, the index of the oldest ADC sample is
 *     `(write_pointer_position + 1) % ADC_BUFFER_SIZE`
 * @param pos Write pointer position
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetWritePointer(uint32_t* pos);

/**
 * Returns current position of ADC write pointer.
 *
 * The write pointer position is the index, within the ADC buffer, of the last
 * array cell that has been written to.
 * @note The ADC buffer is a ring buffer. When it is full, the index of the oldest ADC sample is
 *     `(write_pointer_position + 1) % ADC_BUFFER_SIZE`
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param pos Write pointer position
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetWritePointerCh(rp_channel_t channel, uint32_t* pos);

/**
 * Returns position of ADC write pointer at time when trigger arrived.
 * @param pos Write pointer position
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetWritePointerAtTrig(uint32_t* pos);

/**
 * Returns position of ADC write pointer at time when trigger arrived.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param pos Write pointer position
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetWritePointerAtTrigCh(rp_channel_t channel, uint32_t* pos);

/**
 * Starts the acquire. Signals coming from the input channels are acquired and written into memory.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqStart();

/**
 * Starts the acquire. Signals coming from the input channels are acquired and written into memory.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqStartCh(rp_channel_t channel);

/**
* Stops the acquire.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_AcqStop();

/**
* Stops the acquire.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_AcqStopCh(rp_channel_t channel);

/**
 * Resets the acquire writing state machine and set by default all parameters.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqReset();

/**
 * Resets the acquire writing state machine and set by default all parameters.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqResetCh(rp_channel_t channel);

/**
 * Resets the acquire writing state machine.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqResetFpga();

/**
 * Unlocks trigger capture after a trigger has been detected.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqUnlockTrigger();

/**
 * Unlocks trigger capture after a trigger has been detected.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqUnlockTriggerCh(rp_channel_t channel);

/**
 * Returns the trigger's current blocking state..
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetUnlockTrigger(bool* state);

/**
 * Returns the trigger's current blocking state..
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetUnlockTriggerCh(rp_channel_t channel, bool* state);

/**
 * Normalizes the ADC buffer position. Returns the modulo operation of ADC buffer size...
 * @param pos position to be normalized
 * @return Normalized position (pos % ADC_BUFFER_SIZE)
 */
uint32_t rp_AcqGetNormalizedDataPos(uint32_t pos);

/**
 * Returns the ADC buffer in raw units from start to end position.
 *
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param start_pos Starting position of the ADC buffer to retrieve.
 * @param end_pos Ending position of the ADC buffer to retrieve.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @param buffer_size Length of input buffer. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataPosRaw(rp_channel_t channel, uint32_t start_pos, uint32_t end_pos, int16_t* buffer, uint32_t* buffer_size);

/**
 * Returns the ADC buffer in raw units from start to end position.
 *
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param start_pos Starting position of the ADC buffer to retrieve.
 * @param end_pos Ending position of the ADC buffer to retrieve.
 * @param np_buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @param buffer_size Length of input buffer. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataPosRawNP(rp_channel_t channel, uint32_t start_pos, uint32_t end_pos, int16_t* np_buffer, int size);

/**
 * Returns the ADC buffer in Volt units from start to end position.
 *
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param start_pos Starting position of the ADC buffer to retrieve.
 * @param end_pos Ending position of the ADC buffer to retrieve.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @param buffer_size Length of input buffer. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataPosV(rp_channel_t channel, uint32_t start_pos, uint32_t end_pos, float* buffer, uint32_t* buffer_size);

/**
 * Returns the ADC buffer in Volt units from start to end position.
 *
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param start_pos Starting position of the ADC buffer to retrieve.
 * @param end_pos Ending position of the ADC buffer to retrieve.
 * @param np_buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @param buffer_size Length of input buffer. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataPosVNP(rp_channel_t channel, uint32_t start_pos, uint32_t end_pos, float* np_buffer, int size);

/**
 * Returns the ADC buffer in raw units from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param pos Starting position of the ADC buffer to retrieve.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataRaw(rp_channel_t channel, uint32_t pos, uint32_t* size, int16_t* buffer);

/**
 * Returns the ADC buffer in raw units from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param pos Starting position of the ADC buffer to retrieve.
 * @param np_buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataRawNP(rp_channel_t channel, uint32_t pos, int16_t* np_buffer, int size);

/**
 * Returns the ADC buffer in calibrated raw units from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param pos Starting position of the ADC buffer to retrieve.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataRawWithCalib(rp_channel_t channel, uint32_t pos, uint32_t* size, int16_t* buffer);

/**
 * Returns the ADC buffer in calibrated raw units from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param pos Starting position of the ADC buffer to retrieve.
 * @param np_buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataRawWithCalibNP(rp_channel_t channel, uint32_t pos, int16_t* np_buffer, int size);

/**
 * Returns the ADC buffer in raw units from the oldest sample to the newest one.
 * Output buffer must be at least 'size' long.
 * CAUTION: Use this method only when write pointer has stopped (Trigger happened and writing stopped).
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetOldestDataRaw(rp_channel_t channel, uint32_t* size, int16_t* buffer);

/**
 * Returns the ADC buffer in raw units from the oldest sample to the newest one.
 * Output buffer must be at least 'size' long.
 * CAUTION: Use this method only when write pointer has stopped (Trigger happened and writing stopped).
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetOldestDataRawNP(rp_channel_t channel, int16_t* np_buffer, int size);

/**
 * Returns the latest ADC buffer samples in raw units.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetLatestDataRaw(rp_channel_t channel, uint32_t* size, int16_t* buffer);

/**
 * Returns the latest ADC buffer samples in raw units.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param np_buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetLatestDataRawNP(rp_channel_t channel, int16_t* np_buffer, int size);

/**
 * Returns the ADC buffer in Volt units from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param pos Starting position of the ADC buffer to retrieve
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataV(rp_channel_t channel, uint32_t pos, uint32_t* size, float* buffer);

/**
 * Returns the ADC buffer in Volt units from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param pos Starting position of the ADC buffer to retrieve
 * @param np_buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataVNP(rp_channel_t channel, uint32_t pos, float* np_buffer, int size);

/**
 * Returns the ADC buffers from specified position.
 * @param pos Starting position of the ADC buffer to retrieve
 * @param out The buffer will be filled according to the settings.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetData(uint32_t pos, buffers_t* out);

/**
 * Returns the ADC buffers from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * @param pos Starting position of the ADC buffer to retrieve
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param offset Correcting data offset in the output buffer
 * @param out The buffer will be filled according to the settings.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataWithCorrection(uint32_t pos, uint32_t* size, int32_t offset, buffers_t* out);

/**
 * Returns the ADC buffer in Volt units from the oldest sample to the newest one.
 * Output buffer must be at least 'size' long.
 * CAUTION: Use this method only when write pointer has stopped (Trigger happened and writing stopped).
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetOldestDataV(rp_channel_t channel, uint32_t* size, float* buffer);

/**
 * Returns the ADC buffer in Volt units from the oldest sample to the newest one.
 * Output buffer must be at least 'size' long.
 * CAUTION: Use this method only when write pointer has stopped (Trigger happened and writing stopped).
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetOldestDataVNP(rp_channel_t channel, float* np_buffer, int size);

/**
 * Returns the latest ADC buffer samples in Volt units.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetLatestDataV(rp_channel_t channel, uint32_t* size, float* buffer);

/**
 * Returns the latest ADC buffer samples in Volt units.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param np_buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetLatestDataVNP(rp_channel_t channel, float* np_buffer, int size);

/**
 * Returns the ADC buffer size in samples.
 *
 * @param size Size of the ADC buffer in samples.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetBufSize(uint32_t* size);

/**
* The function enables or disables the filter in the FPGA.
* @param enabled When true, the bypass is enabled, otherwise it is disabled.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_AcqSetBypassFilter(rp_channel_t channel, bool enable);

/**
* Gets the current filter bypass from the FPGA
* @param enable Returns the filter bypass
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_AcqGetBypassFilter(rp_channel_t channel, bool* enable);

/**
* Sets the current calibration values from temporary memory to the FPGA filter
* @param channel Channel A or B.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_AcqUpdateAcqFilter(rp_channel_t channel);

/**
* Sets the current calibration values from temporary memory to the FPGA filter
* @param channel Channel A or B.
* @param coef_aa Return AA coefficient.
* @param coef_bb Return BB coefficient.
* @param coef_kk Return KK coefficient.
* @param coef_pp Return PP coefficient.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_AcqGetFilterCalibValue(rp_channel_t channel, uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp);

/**
* Sets the current calibration values from temporary memory to the FPGA
* @param channel Channel A or B.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_AcqSetCalibInFPGA(rp_channel_t channel);

/**
 * Sets ext. trigger debouncer for acquisition in Us (Value must be positive).
 * @param value Value in microseconds.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetExtTriggerDebouncerUs(double value);

/**
 * Gets ext. trigger debouncer for acquisition in Us
 * @param value Return value in microseconds.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetExtTriggerDebouncerUs(double* value);

/**
* Sets the AC / DC modes for input.
* Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B.
* @param mode Set current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_AcqSetAC_DC(rp_channel_t channel, rp_acq_ac_dc_mode_t mode);

/**
* Get the AC / DC modes for input.
* Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B.
* @param status Set current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_AcqGetAC_DC(rp_channel_t channel, rp_acq_ac_dc_mode_t* status);

/**
* Initializes buffers to the specified length.
* @param maxChannels Number of channels.
* @param length Buffer length.
* @param initInt16 Initializes an integer type buffer.
* @param initDouble Initializes a double type buffer.
* @param initFloat Initializes a floating point buffer.
* @return Returns null if memory allocation fails.
*/
buffers_t* rp_createBuffer(uint8_t maxChannels, uint32_t length, bool initInt16, bool initDouble, bool initFloat);

/**
* Removes initialized buffers. The structure itself is not deleted. Only internal content.
* @param _in_buffer Buffer pointer.
*/
void rp_deleteBuffer(buffers_t* _in_buffer);

///@}

#endif  //__RP_ACQ_H
