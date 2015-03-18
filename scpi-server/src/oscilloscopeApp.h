/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server oscilloscope application SCPI commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */


#ifndef OSCILLOSCOPE_APP_H_
#define OSCILLOSCOPE_APP_H_

#include "../3rdparty/libs/scpi-parser/libscpi/inc/scpi/types.h"
#include "../../api-mockup/rpbase/src/rp.h"

scpi_result_t RP_APP_OscRun(scpi_t *context);
scpi_result_t RP_APP_OscStop(scpi_t *context);
scpi_result_t RP_APP_OscReset(scpi_t *context);
scpi_result_t RP_APP_OscAutoscale(scpi_t *context);
scpi_result_t RP_APP_OscSingle(scpi_t *context);
scpi_result_t RP_APP_OscChannel1SetAmplitudeOffset(scpi_t *context);
scpi_result_t RP_APP_OscChannel2SetAmplitudeOffset(scpi_t *context);
scpi_result_t RP_APP_OscChannel1GetAmplitudeOffset(scpi_t *context);
scpi_result_t RP_APP_OscChannel2GetAmplitudeOffset(scpi_t *context);
scpi_result_t RP_APP_OscChannel1SetAmplitudeScale(scpi_t *context);
scpi_result_t RP_APP_OscChannel2SetAmplitudeScale(scpi_t *context);
scpi_result_t RP_APP_OscChannel1GetAmplitudeScale(scpi_t *context);
scpi_result_t RP_APP_OscChannel2GetAmplitudeScale(scpi_t *context);
scpi_result_t RP_APP_OscChannel1SetProbeAtt(scpi_t *context);
scpi_result_t RP_APP_OscChannel2SetProbeAtt(scpi_t *context);
scpi_result_t RP_APP_OscChannel1GetProbeAtt(scpi_t *context);
scpi_result_t RP_APP_OscChannel2GetProbeAtt(scpi_t *context);
scpi_result_t RP_APP_OscChannel1SetInputGain(scpi_t *context);
scpi_result_t RP_APP_OscChannel2SetInputGain(scpi_t *context);
scpi_result_t RP_APP_OscChannel1GetInputGain(scpi_t *context);
scpi_result_t RP_APP_OscChannel2GetInputGain(scpi_t *context);
scpi_result_t RP_APP_OscSetTimeOffset(scpi_t *context);
scpi_result_t RP_APP_OscGetTimeOffset(scpi_t *context);
scpi_result_t RP_APP_OscSetTimeScale(scpi_t *context);
scpi_result_t RP_APP_OscGetTimeScale(scpi_t *context);
scpi_result_t RP_APP_OscSetTriggerSweep(scpi_t *context);
scpi_result_t RP_APP_OscGetTriggerSweep(scpi_t *context);
scpi_result_t RP_APP_OscSetTriggerSource(scpi_t *context);
scpi_result_t RP_APP_OscGetTriggerSource(scpi_t *context);
scpi_result_t RP_APP_OscSetTriggerSlope(scpi_t *context);
scpi_result_t RP_APP_OscGetTriggerSlope(scpi_t *context);
scpi_result_t RP_APP_OscSetTriggerLevel(scpi_t *context);
scpi_result_t RP_APP_OscGetTriggerLevel(scpi_t *context);
scpi_result_t RP_APP_OscGetViewSize(scpi_t *context);
scpi_result_t RP_APP_OscChannel1GetViewData(scpi_t *context);
scpi_result_t RP_APP_OscChannel2GetViewData(scpi_t *context);
scpi_result_t RP_APP_OscChannel1MeasureAmplitude(scpi_t *context);
scpi_result_t RP_APP_OscChannel2MeasureAmplitude(scpi_t *context);
scpi_result_t RP_APP_OscChannel1MeasureMeanVoltage(scpi_t *context);
scpi_result_t RP_APP_OscChannel2MeasureMeanVoltage(scpi_t *context);
scpi_result_t RP_APP_OscChannel1MeasureAmplitudeMax(scpi_t *context);
scpi_result_t RP_APP_OscChannel2MeasureAmplitudeMax(scpi_t *context);
scpi_result_t RP_APP_OscChannel1MeasureAmplitudeMin(scpi_t *context);
scpi_result_t RP_APP_OscChannel2MeasureAmplitudeMin(scpi_t *context);
scpi_result_t RP_APP_OscChannel1MeasureFrequency(scpi_t *context);
scpi_result_t RP_APP_OscChannel2MeasureFrequency(scpi_t *context);
scpi_result_t RP_APP_OscChannel1MeasurePeriod(scpi_t *context);
scpi_result_t RP_APP_OscChannel2MeasurePeriod(scpi_t *context);
scpi_result_t RP_APP_OscChannel1MeasureDutyCycle(scpi_t *context);
scpi_result_t RP_APP_OscChannel2MeasureDutyCycle(scpi_t *context);
scpi_result_t RP_APP_OscChannel1GetCursorVoltage(scpi_t *context);
scpi_result_t RP_APP_OscChannel2GetCursorVoltage(scpi_t *context);
scpi_result_t RP_APP_OscChannel1GetCursorDeltaAmplitude(scpi_t *context);
scpi_result_t RP_APP_OscChannel2GetCursorDeltaAmplitude(scpi_t *context);
scpi_result_t RP_APP_OscGetCursorDeltaTime(scpi_t *context);
scpi_result_t RP_APP_OscGetCursorTime(scpi_t *context);
scpi_result_t RP_APP_OscGetCursorDeltaFrequency(scpi_t *context);

scpi_result_t RP_APP_OscSetAmplitudeOffset(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_APP_OscGetAmplitudeOffset(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_APP_OscSetAmplitudeScale(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_APP_OscGetAmplitudeScale(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_APP_OscSetProbeAtt(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_APP_OscGetProbeAtt(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_APP_OscSetInputGain(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_APP_OscGetInputGain(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_APP_OscGetViewData(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_APP_OscMeasureAmplitude(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_APP_OscMeasureMeanVoltage(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_APP_OscMeasureAmplitudeMax(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_APP_OscMeasureAmplitudeMin(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_APP_OscMeasureFrequency(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_APP_OscMeasurePeriod(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_APP_OscMeasureDutyCycle(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_APP_OscGetCursorVoltage(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_APP_OscGetCursorDeltaAmplitude(rp_channel_t channel, scpi_t *context);


#endif /* OSCILLOSCOPE_APP_H_ */
