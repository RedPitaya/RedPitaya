/**
 * $Id: $
 *
 * @file rp_gen.h
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

#ifndef __RP_GEN_H
#define __RP_GEN_H

#include <stdint.h>
#include <stdbool.h>
#include "rp_enums.h"

/** @name Generate
*/
///@{


/**
* Sets generate to default values.
*/
int rp_GenReset();

/**
* Enables output
* @param channel Channel A or B which we want to enable
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenOutEnable(rp_channel_t channel);

/**
* Runs/Stop two channels synchronously
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenOutEnableSync(bool enable);

/**
* Disables output
* @param channel Channel A or B which we want to disable
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenOutDisable(rp_channel_t channel);

/**
* Gets value true if channel is enabled otherwise return false.
* @param channel Channel A or B.
* @param value Pointer where value will be returned
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenOutIsEnabled(rp_channel_t channel, bool *value);

/**
* Sets channel signal peak to peak amplitude.
* @param channel Channel A or B for witch we want to set amplitude
* @param amplitude Amplitude of the generated signal. From 0 to max value. Max amplitude is 1
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenAmp(rp_channel_t channel, float amplitude);

/**
* Gets channel signal peak to peak amplitude.
* @param channel Channel A or B for witch we want to get amplitude.
* @param amplitude Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetAmp(rp_channel_t channel, float *amplitude);

/**
* Sets DC offset of the signal. signal = signal + DC_offset.
* @param channel Channel A or B for witch we want to set DC offset.
* @param offset DC offset of the generated signal. Max offset is 2.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenOffset(rp_channel_t channel, float offset);

/**
* Gets DC offset of the signal.
* @param channel Channel A or B for witch we want to get amplitude.
* @param offset Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetOffset(rp_channel_t channel, float *offset);

/**
* Sets channel signal frequency.
* @param channel Channel A or B for witch we want to set frequency.
* @param frequency Frequency of the generated signal in Hz.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenFreq(rp_channel_t channel, float frequency);

/**
* Sets channel signal frequency in fpga without reset generator and rebuild signal.
* @param channel Channel A or B for witch we want to set frequency.
* @param frequency Frequency of the generated signal in Hz.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenFreqDirect(rp_channel_t channel, float frequency);

/**
* Gets channel signal frequency.
* @param channel Channel A or B for witch we want to get frequency.
* @param frequency Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetFreq(rp_channel_t channel, float *frequency);


/**
* Sets channel sweep signal start frequency.
* @param channel Channel A or B for witch we want to set frequency.
* @param frequency Frequency of the generated signal in Hz.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSweepStartFreq(rp_channel_t channel, float frequency);

/**
* Gets channel sweep signal start frequency.
* @param channel Channel A or B for witch we want to get frequency.
* @param frequency Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetSweepStartFreq(rp_channel_t channel, float *frequency);

/**
* Sets channel sweep signal end frequency.
* @param channel Channel A or B for witch we want to set frequency.
* @param frequency Frequency of the generated signal in Hz.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSweepEndFreq(rp_channel_t channel, float frequency);

/**
* Gets channel sweep signal end frequency.
* @param channel Channel A or B for witch we want to get frequency.
* @param frequency Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetSweepEndFreq(rp_channel_t channel, float *frequency);

/**
* Sets channel signal phase. This shifts the signal in time.
* @param channel Channel A or B for witch we want to set phase.
* @param phase Phase in degrees of the generated signal. From 0 deg to 180 deg.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenPhase(rp_channel_t channel, float phase);

/**
* Gets channel signal phase.
* @param channel Channel A or B for witch we want to get phase.
* @param phase Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetPhase(rp_channel_t channel, float *phase);

/**
* Sets channel signal waveform. This determines how the signal looks.
* @param channel Channel A or B for witch we want to set waveform type.
* @param form Wave form of the generated signal [SINE, SQUARE, TRIANGLE, SAWTOOTH, PWM, DC, ARBITRARY, SWEEP].
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenWaveform(rp_channel_t channel, rp_waveform_t type);

/**
* Gets channel signal waveform.
* @param channel Channel A or B for witch we want to get waveform.
* @param type Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetWaveform(rp_channel_t channel, rp_waveform_t *type);

/**
* Sets the generation mode for the sweep signal.
* @param channel Channel A or B for witch we want to set waveform type.
* @param mode Mode of the generated signal [RP_GEN_SWEEP_MODE_LINEAR, RP_GEN_SWEEP_MODE_LOG].
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSweepMode(rp_channel_t channel, rp_gen_sweep_mode_t mode);

/**
* Gets the generation mode for the sweep signal.
* @param channel Channel A or B for witch we want to get waveform.
* @param mode Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetSweepMode(rp_channel_t channel, rp_gen_sweep_mode_t *mode);


/**
* Sets the direction of frequency change for sweep.
* @param channel Channel A or B for witch we want to set waveform type.
* @param mode Wave form of the generated signal [RP_GEN_SWEEP_DIR_NORMAL, RP_GEN_SWEEP_DIR_UP_DOWN].
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSweepDir(rp_channel_t channel, rp_gen_sweep_dir_t mode);

/**
* Gets the direction of frequency change for sweep.
* @param channel Channel A or B for witch we want to get waveform.
* @param mode Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetSweepDir(rp_channel_t channel, rp_gen_sweep_dir_t *mode);

/**
* Sets user defined waveform.
* @param channel Channel A or B for witch we want to set waveform.
* @param waveform Use defined wave form, where min is -1V an max is 1V.
* @param length Length of waveform.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenArbWaveform(rp_channel_t channel, float *waveform, uint32_t length);

/**
* Gets user defined waveform.
* @param channel Channel A or B for witch we want to get waveform.
* @param waveform Pointer where waveform will be returned.
* @param length Pointer where waveform length will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetArbWaveform(rp_channel_t channel, float *waveform, uint32_t *length);

/**
* Sets duty cycle of PWM signal.
* @param channel Channel A or B for witch we want to set duty cycle.
* @param ratio Ratio betwen the time when signal in HIGH vs the time when signal is LOW.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenDutyCycle(rp_channel_t channel, float ratio);

/**
* Gets duty cycle of PWM signal.
* @param channel Channel A or B for witch we want to get duty cycle.
* @param ratio Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetDutyCycle(rp_channel_t channel, float *ratio);

/**
* Sets rise time of square signal.
* @param channel Channel A or B for witch we want to set rise time.
* @param time Rise time in microseconds.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/

int rp_GenRiseTime(rp_channel_t channel, float time);

/**
* Gets rise time of square signal.
* @param channel Channel A or B for witch we want to set rise time.
* @param time Rise time in microseconds.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/

int rp_GenGetRiseTime(rp_channel_t channel, float *time);

/**
 * Sets fall time of square signal.
 * @param channel Channel A or B for witch we want to set fall time.
 * @param time Fall time in microseconds.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */

int rp_GenFallTime(rp_channel_t channel, float time);

/**
 * Gets fall time of square signal.
 * @param channel Channel A or B for witch we want to set fall time.
 * @param time Fall time in microseconds.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */

int rp_GenGetFallTime(rp_channel_t channel, float *time);

/**
* Sets generation mode.
* @param channel Channel A or B for witch we want to set generation mode.
* @param mode Type of signal generation (CONTINUOUS, BURST, STREAM).
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenMode(rp_channel_t channel, rp_gen_mode_t mode);

/**
* Gets generation mode.
* @param channel Channel A or B for witch we want to get generation mode.
* @param mode Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetMode(rp_channel_t channel, rp_gen_mode_t *mode);

/**
* Sets number of generated waveforms in a burst.
* @param channel Channel A or B for witch we want to set number of generated waveforms in a burst.
* @param num Number of generated waveforms. If -1 a continuous signal will be generated.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenBurstCount(rp_channel_t channel, int num);

/**
* Gets number of generated waveforms in a burst.
* @param channel Channel A or B for witch we want to get number of generated waveforms in a burst.
* @param num Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetBurstCount(rp_channel_t channel, int *num);

/**
* Sets the value to be set at the end of the generated signal in burst mode.
* @param channel Channel A or B for witch we want to set number of generated waveforms in a burst.
* @param amplitude Amplitude level at the end of the signal (Volt).
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenBurstLastValue(rp_channel_t channel, float amplitude);

/**
* Gets the value to be set at the end of the generated signal in burst mode.
* @param channel Channel A or B for witch we want to get number of generated waveforms in a burst.
* @param amplitude Amplitude where value will be returned (Volt).
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetBurstLastValue(rp_channel_t channel, float *amplitude);

/**
* The level of which is set by the generator after the outputs are turned on before the signal is generated.
* @param channel Channel A or B for witch we want to set number of generated waveforms in a burst.
* @param amplitude Amplitude level at the end of the signal (Volt).
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetInitGenValue(rp_channel_t channel, float amplitude);

/**
* Gets the value of the initial signal level.
* @param channel Channel A or B for witch we want to get number of generated waveforms in a burst.
* @param amplitude Amplitude where value will be returned (Volt).
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetInitGenValue(rp_channel_t channel, float *amplitude);

/**
* Sets number of burst repetitions. This determines how many bursts will be generated.
* @param channel Channel A or B for witch we want to set number of burst repetitions.
* @param repetitions Number of generated bursts. If 0x10000, infinite bursts will be generated.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenBurstRepetitions(rp_channel_t channel, int repetitions);

/**
* Gets number of burst repetitions.
* @param channel Channel A or B for witch we want to get number of burst repetitions.
* @param repetitions Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetBurstRepetitions(rp_channel_t channel, int *repetitions);

/**
* Sets the time/period of one burst in micro seconds. Period must be equal or greater then the time of one burst.
* If it is greater than the difference will be the delay between two consequential bursts.
* @param channel Channel A or B for witch we want to set burst period.
* @param period Time in micro seconds.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenBurstPeriod(rp_channel_t channel, uint32_t period);

/**
* Gets the period of one burst in micro seconds.
* @param channel Channel A or B for witch we want to get burst period.
* @param period Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetBurstPeriod(rp_channel_t channel, uint32_t *period);

/**
* Sets trigger source.
* @param channel Channel A or B for witch we want to set trigger source.
* @param src Trigger source (INTERNAL, EXTERNAL_PE, EXTERNAL_NE, GATED_BURST).
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenTriggerSource(rp_channel_t channel, rp_trig_src_t src);

/**
* Gets trigger source.
* @param channel Channel A or B for witch we want to get burst period.
* @param src Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetTriggerSource(rp_channel_t channel, rp_trig_src_t *src);

/**
* The generator is reset on both channels.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSynchronise();

/**
* The generator is reset on channels.
* @param channel Channel A or B
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenResetTrigger(rp_channel_t channel);

/**
* Emit trigger for selected channel
* @param channel Channel A or B
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/

int rp_GenTriggerOnly(rp_channel_t channel);

/**
* Sets the DAC protection mode from overheating. Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B for witch we want to set protection.
* @param enable Flag enabling protection mode.total
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_SetEnableTempProtection(rp_channel_t channel, bool enable);

/**
* Get status of DAC protection mode from overheating. Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B for witch we want to set protection.
* @param enable Flag return current status.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GetEnableTempProtection(rp_channel_t channel, bool *enable);

/**
* Resets the flag indicating that the DAC is overheated. Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B.
* @param status  New status for latch trigger.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_SetLatchTempAlarm(rp_channel_t channel, bool status);

/**
* Returns the status that there was an overheat. Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B.
* @param status  State of latch trigger.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GetLatchTempAlarm(rp_channel_t channel, bool *status);

/**
* Returns the current DAC overheat status in real time. Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B.
* @param status  Get current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GetRuntimeTempAlarm(rp_channel_t channel, bool *status);

/**
* Sets the gain modes for output.
* Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B.
* @param mode Set current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetGainOut(rp_channel_t channel,rp_gen_gain_t mode);

/**
* Get the gain modes for output.
* Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B.
* @param status Set current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetGainOut(rp_channel_t channel,rp_gen_gain_t *status);

/**
 * Sets ext. trigger debouncer for generation in Us (Value must be positive).
 * @param value Value in microseconds.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GenSetExtTriggerDebouncerUs(double value);

/**
 * Gets ext. trigger debouncer for generation in Us
 * @param value Return value in microseconds.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GenGetExtTriggerDebouncerUs(double *value);

///@}

#endif //__RP_GEN_H