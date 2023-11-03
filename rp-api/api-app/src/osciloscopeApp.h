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

#include "rpApp.h"

#define SPECTR_ADC_SAMPLE_RATE ADC_SAMPLE_RATE
#define SPECTR_ADC_BITS ADC_BITS


#define SIGNAL_EXISTENCE              0.01    // V
#define AUTO_SCALE_PERIOD_COUNT       2
#define AUTO_SCALE_AMP_SCA_FACTOR     1.05
#define AUTO_SCALE_TIME_OFFSET        0
#define AUTO_SCALE_PERIOD_ERROR       0.08
#define AUTO_SCALE_VMEAN_ERROR        0.08
#define AUTO_SCALE_NUM_OF_SCALE       20
#define MIN_TIME_TO_DRAW_BEFORE_TIG   100

#define PERIOD_REP_COUNT_MIN          3
#define VMEAN_REP_COUNT_MIN           3

int osc_Init();
int osc_Release();
int osc_SetDefaultValues();

int osc_run();
int osc_stop();
int osc_reset();
int osc_single();
int osc_autoScale();
int osc_getAutoScale(bool *_state);
int osc_isRunning(bool *running);
int osc_isTriggered();
int osc_setTimeScale(float scale);
int osc_getTimeScale(float *division);
int osc_setTimeOffset(float offset);
int osc_getTimeOffset(float *offset);
int osc_setProbeAtt(rp_channel_t channel, float att);
int osc_getProbeAtt(rp_channel_t channel, float *att);
int osc_setInputGain(rp_channel_t channel, rpApp_osc_in_gain_t gain);
int osc_getInputGain(rp_channel_t channel, rpApp_osc_in_gain_t *gain);
int osc_setAmplitudeScale(rpApp_osc_source source, double scale);
int osc_getAmplitudeScale(rpApp_osc_source source, double *scale);
int osc_setAmplitudeOffset(rpApp_osc_source source, double offset);
int osc_getAmplitudeOffset(rpApp_osc_source source, double *offset);
int osc_setTriggerSource(rpApp_osc_trig_source_t triggerSource);
int osc_getTriggerSource(rpApp_osc_trig_source_t *triggerSource);
int osc_setTriggerSlope(rpApp_osc_trig_slope_t slope);
int osc_getTriggerSlope(rpApp_osc_trig_slope_t *slope);
int osc_setTriggerLevel(float level);
int osc_getTriggerLevel(float *level);
int osc_setTriggerSweep(rpApp_osc_trig_sweep_t mode);
int osc_getTriggerSweep(rpApp_osc_trig_sweep_t *mode);
int osc_setInverted(rpApp_osc_source source, bool inverted);
int osc_isInverted(rpApp_osc_source source, bool *inverted);
int osc_getViewPart(float *ratio);
int osc_measureVpp(rpApp_osc_source source, float *Vpp);
int osc_measureMeanVoltage(rpApp_osc_source source, float *meanVoltage);
int osc_measureMaxVoltage(rpApp_osc_source source, float *Vmax);
int osc_measureMinVoltage(rpApp_osc_source source, float *Vmin);
int osc_measureFrequency(rpApp_osc_source source, float *frequency);
int osc_measurePeriod(rpApp_osc_source source, float *period);
int osc_measurePeriodCh(rpApp_osc_source source, float *period);
int osc_measurePeriodMath(float *period);
int osc_measureDutyCycle(rpApp_osc_source source, float *dutyCycle);
int osc_measureRootMeanSquare(rpApp_osc_source source, float *rms);
int osc_getCursorVoltage(rpApp_osc_source source, uint32_t cursor, float *value);
int osc_getCursorTime(uint32_t cursor, float *value);
int osc_getCursorDeltaTime(uint32_t cursor1, uint32_t cursor2, float *value);
int oscGetCursorDeltaAmplitude(rpApp_osc_source source, uint32_t cursor1, uint32_t cursor2, float *value);
int osc_getCursorDeltaFrequency(uint32_t cursor1, uint32_t cursor2, float *value);
int osc_getData(rpApp_osc_source source_t, float *data, uint32_t size);
int osc_getRawData(rp_channel_t source, uint16_t *data, uint32_t size);
int osc_getExportedData(rpApp_osc_source source, rpApp_osc_exportMode mode, bool normalize, float *data, uint32_t *size);

int osc_setMathOperation(rpApp_osc_math_oper_t op);
int osc_getMathOperation(rpApp_osc_math_oper_t *op);
int osc_setMathSources(rp_channel_t source1, rp_channel_t source2);
int osc_getMathSources(rp_channel_t *source1, rp_channel_t *source2);
int osc_getMathOperation(rpApp_osc_math_oper_t *op);
int osc_setViewSize(uint32_t size);
int osc_getViewSize(uint32_t *size);
int osc_getViewLimits(uint32_t* start, uint32_t* end);
int osc_scaleMath();
int osc_refreshViewData();


int osc_SetSmoothMode(rp_channel_t _channel, rpApp_osc_interpolationMode _mode);
int osc_GetSmoothMode(rp_channel_t _channel, rpApp_osc_interpolationMode *_mode);

int threadSafe_acqStart();
int threadSafe_acqStop();
double scaleAmplitude(double volts, double ampScale, double probeAtt, double ampOffset, double invertFactor);
int scaleAmplitudeChannel(rpApp_osc_source source, float volts, float *res);
double unscaleAmplitude(double value, double ampScale, double probeAtt, double ampOffset, double invertFactor);
int unscaleAmplitudeChannel(rpApp_osc_source source, float value, float *res);
int attenuateAmplitudeChannel(rpApp_osc_source source, float value, float *res);
int unattenuateAmplitudeChannel(rpApp_osc_source source, float value, float *res);
double roundUpTo125(double data);
double roundUpTo25(double data);

void calculateIntegral(rp_channel_t channel, float scale, float offset, float invertFactor);
void calculateDevivative(rp_channel_t channel, float scale, float offset, float invertFactor);
float calculateMath(float v1, float v2, rpApp_osc_math_oper_t op);
double unOffsetAmplitude(double value, double ampScale, double ampOffset);
int unscaleAmplitudeChannel(rpApp_osc_source source, float value, float *res);
int unOffsetAmplitudeChannel(rpApp_osc_source source, float value, float *res);

// void clearView();
// void clearMath();

int osc_measureMax(rpApp_osc_source source, float *Max);
int osc_measureMin(rpApp_osc_source source, float *Min);

#endif //__OSCILOSCOPE_H
