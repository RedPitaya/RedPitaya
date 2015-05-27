/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server utils module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdbool.h>
#include "../../api-mockup/rpbase/src/rp.h"
#include "acquire.h"
#include "../../api-mockup/rpApplications/src/rpApp.h"

#define SET_OK(cont) \
    	SCPI_ResultString(cont, "OK"); \
    	return SCPI_RES_OK;

int getRpDpin(const char* pinStr, rp_dpin_t *rpPin);
int getRpDirection(const char *dirStr, rp_pinDirection_t *direction);
int getRpApin(const char *pinStr, rp_apin_t *rpPin);


int getRpDecimation(int decimationInt, rp_acq_decimation_t *decimation);
int getRpDecimationInt(rp_acq_decimation_t decimation, int *decimationInt);
int getRpSamplingRateString(rp_acq_sampling_rate_t samplingRate, char *samplingRateString);
int getRpSamplingRate(const char *samplingRateString, rp_acq_sampling_rate_t *samplingRate);
int getRpGain(const char *gainStr, rp_pinState_t *state);
int getRpTriggerSource(const char *sourceStr, rp_acq_trig_src_t *source);
int getRpTriggerSourceString(rp_acq_trig_src_t source, char *triggSourceString);

int getRpWaveform(const char *waveformString, rp_waveform_t *waveform);
int getRpWaveformString(rp_waveform_t waveform, char *waveformString);
int getRpGenTriggerSource(const char *triggerSourceString, rp_trig_src_t *triggerSource);
int getRpGenTriggerSourceString(rp_trig_src_t triggerSource, char *string);

int getRpChannel(const char *string, rp_channel_t *op);
int getRpChannelString(rp_channel_t op, char *string);

int getRpAppInputGain(const char *string, rpApp_osc_in_gain_t *gain);
int getRpAppInputGainString(rpApp_osc_in_gain_t gain, char *string);
int getRpAppTrigSource(const char *string, rpApp_osc_trig_source_t *source);
int getRpAppTrigSourceString(rpApp_osc_trig_source_t source, char *string);
int getRpAppTrigSlope(const char *string, rpApp_osc_trig_slope_t *slope);
int getRpAppTrigSlopeString(rpApp_osc_trig_slope_t slope, char *string);
int getRpAppTrigSweep(const char *string, rpApp_osc_trig_sweep_t *sweep);
int getRpAppTrigSweepString(rpApp_osc_trig_sweep_t sweep, char *string);
int getRpAppMathOperation(const char *string, rpApp_osc_math_oper_t *op);
int getRpAppMathOperationString(rpApp_osc_math_oper_t op, char *string);

int getRpInfinityInteger(const char *string, int32_t *value);
int getRpInfinityIntegerString(int32_t value, char *string);
int getRpUnit(const char *unitString, rp_scpi_acq_unit_t *unit);

#endif /* UTILS_H_ */
