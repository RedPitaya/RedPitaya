/**
* $Id: $
*
* @brief Red Pitaya application library osciloscope module interface
*
* @Author Red Pitaya
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#ifndef __OSCILOSCOPE_H
#define __OSCILOSCOPE_H

#include "../../rpbase/src/rp.h"
#include "rpApp.h"

#define VIEW_SIZE_DEFAULT           1024
#define DIVISIONS_COUNT_X           10
#define DIVISIONS_COUNT_Y           8

#define AUTO_TRIG_TIMEOUT           1       // s
#define SIGNAL_EXISTENCE            0.01    // V
#define PERIOD_STORAGE_PERIOD_COUNT 15
#define AUTO_SCALE_PERIOD_COUNT     2
#define AUTO_SCALE_AMP_SCA_FACTOR   1.5
#define AUTO_SCALE_TIME_OFFSET      0
#define PRE_WPOINTER_EPSILON        0.001
#define MAIN_THREAT_SLEEP           500     // us
#define MAX_UINT                    4294967296

#define TIME_SCALE_FOR_DIV_1        1e-5
#define TIME_SCALE_FOR_DIV_8        1e-4
#define TIME_SCALE_FOR_DIV_64       6e-3
#define TIME_SCALE_FOR_DIV_1024     5e-2
#define TIME_SCALE_FOR_DIV_8192     0.5


int osc_Init();
int osc_Release();
int osc_SetDefaultValues();

int osc_run();
int osc_stop();
int osc_reset();
int osc_single();
int osc_autoScale();
int osc_setTimeScale(float scale);
int osc_getTimeScale(float *division);
int osc_setTimeOffset(float offset);
int osc_getTimeOffset(float *offset);
int osc_setProbeAtt(rp_channel_t channel, float att);
int osc_getProbeAtt(rp_channel_t channel, float *att);
int osc_setInputGain(rp_channel_t channel, rpApp_osc_in_gain_t gain);
int osc_getInputGain(rp_channel_t channel, rpApp_osc_in_gain_t *gain);
int osc_setAmplitudeScale(rp_channel_t channel, float scale);
int osc_getAmplitudeScale(rp_channel_t channel, float *scale);
int osc_setAmplitudeOffset(rp_channel_t channel, float offset);
int osc_getAmplitudeOffset(rp_channel_t channel, float *offset);
int osc_setTriggerSource(rpApp_osc_trig_source_t triggerSource);
int osc_getTriggerSource(rpApp_osc_trig_source_t *triggerSource);
int osc_setTriggerSlope(rpApp_osc_trig_slope_t slope);
int osc_getTriggerSlope(rpApp_osc_trig_slope_t *slope);
int osc_setTriggerLevel(float level);
int osc_getTriggerLevel(float *level);
int osc_setTriggerSweep(rpApp_osc_trig_sweep_t mode);
int osc_getTriggerSweep(rpApp_osc_trig_sweep_t *mode);
int osc_measureVpp(rp_channel_t channel, float *Vpp);
int osc_measureMeanVoltage(rp_channel_t channel, float *meanVoltage);
int osc_measureMaxVoltage(rp_channel_t channel, float *Vmax);
int osc_measureMinVoltage(rp_channel_t channel, float *Vmin);
int osc_measureFrequency(rp_channel_t channel, float *frequency);
int osc_measurePeriod(rp_channel_t channel, float *period);
int osc_measureDutyCycle(rp_channel_t channel, float *dutyCycle);
int osc_measureRootMeanSquare(rp_channel_t channel, float *rms);
int osc_getCursorVoltage(rp_channel_t channel, uint32_t cursor, float *value);
int osc_getCursorTime(uint32_t cursor, float *value);
int osc_getCursorDeltaTime(uint32_t cursor1, uint32_t cursor2, float *value);
int oscGetCursorDeltaAmplitude(rp_channel_t channel, uint32_t cursor1, uint32_t cursor2, float *value);
int osc_getCursorDeltaFrequency(uint32_t cursor1, uint32_t cursor2, float *value);
int osc_getViewData(rp_channel_t channel_t, float *data, uint32_t size);
int osc_getInvViewData(rp_channel_t channel, float *data, uint32_t size);
int osc_setViewSize(uint32_t size);
int osc_getViewSize(uint32_t *size);

int threadSafe_acqStart();
int threadSafe_acqStop() ;
float scaleAmplitude(float volts, float ampScale, float probeAtt, float ampOffset);
float scaleAmplitudeChannel(rp_channel_t channel, float volts);
float unscaleAmplitude(float value, float ampScale, float probeAtt, float ampOffset);
float unscaleAmplitudeChannel(rp_channel_t channel, float value);
float viewIndexToTime(int index);

void *mainThreadFun(void *arg);

#endif //__OSCILOSCOPE_H
