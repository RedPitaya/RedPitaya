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
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <math.h>
#include <float.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <inttypes.h>
#include <mutex>
#include <thread>

#include "osciloscopeApp.h"
#include "common.h"
#include "neon_asm.h"

#include "osciloscope_logic/data_decimator.h"
#include "osciloscope_logic/view_controller.h"
#include "osciloscope_logic/measure_controller.h"
#include "osciloscope_logic/adc_controller.h"

#define FLOAT_EPS 0.00001f

std::atomic_bool g_threadRun = false;

volatile double ch_ampOffset[MAX_ADC_CHANNELS], math_ampOffset;
volatile double ch_ampScale[MAX_ADC_CHANNELS],  math_ampScale = 1;
volatile float ch_probeAtt[MAX_ADC_CHANNELS];
volatile bool ch_inverted[MAX_ADC_CHANNELS], math_inverted = false;

volatile rpApp_osc_math_oper_t operation;
volatile rp_channel_t mathSource1, mathSource2;

std::thread *g_thread = NULL;

std::mutex g_mutex;

CDataDecimator g_decimator;
CViewController g_viewController;
CMeasureController g_measureController;
CADCController g_adcController;

void mainThreadFun();

void checkAutoscale(bool fromThread);

static inline float sign(float a) {
    return (a < 0.f) ? -1.f : 1.f;
}

// static inline double linear(float x0, float y0, float x1, float y1, double x) {
//     double k = (y1 - y0) / (x1 - x0);
//     double b = y0 - (k * x0);
//     return (k * x) + b;
// }

inline void update_view() {
    if(g_viewController.isOscRun()) {
        g_viewController.requestUpdateViewFromADC();
    }
    g_viewController.requestUpdateView();
}

static inline int scaleChannel(rpApp_osc_source channel, float vpp, float vMean) {
    float scale1 = (float) (vpp * AUTO_SCALE_AMP_SCA_FACTOR / (float)g_viewController.getGridYCount());
    float scale2 = (float) ((fabs(vpp) + (fabs(vMean) * 2.f)) * AUTO_SCALE_AMP_SCA_FACTOR / (float)g_viewController.getGridYCount());
    float scale = MAX(MAX(scale1, scale2), 0.002);
    ECHECK_APP(osc_setAmplitudeScale(channel, roundUpTo125(scale)));
    ECHECK_APP(osc_setAmplitudeOffset(channel, -vMean));
    return RP_OK;
}


int osc_Init() {
    g_decimator.setViewSize(g_viewController.getViewSize());
    g_decimator.setScaleFunction(scaleAmplitudeChannel);
    g_measureController.setUnScaleFunction(unscaleAmplitudeChannel);
    g_measureController.setscaleFunction(scaleAmplitudeChannel);
    g_measureController.setAttenuateAmplitudeChannelFunction(attenuateAmplitudeChannel);
    g_adcController.setAttenuateAmplitudeChannelFunction(attenuateAmplitudeChannel);
    g_adcController.setUnAttenuateAmplitudeChannelFunction(unattenuateAmplitudeChannel);    
    g_threadRun = true;
    g_thread = new std::thread(mainThreadFun);
    return RP_OK;
}

int osc_Release() {
    g_threadRun = false;
    if (g_thread){
        if (g_thread->joinable()){
            g_thread->join();
        }
    }
    return RP_OK;
}

int osc_SetDefaultValues() {

    for(int i = 0; i< MAX_ADC_CHANNELS; i++){
        ch_inverted[i] = false;
    }

    ECHECK_APP(osc_setAmplitudeOffset(RPAPP_OSC_SOUR_CH1, 0));
    ECHECK_APP(osc_setAmplitudeOffset(RPAPP_OSC_SOUR_CH2, 0));
    ECHECK_APP(osc_setAmplitudeOffset(RPAPP_OSC_SOUR_MATH, 0));
    ECHECK_APP(osc_setAmplitudeScale(RPAPP_OSC_SOUR_CH1, 1));
    ECHECK_APP(osc_setAmplitudeScale(RPAPP_OSC_SOUR_CH2, 1));
    ECHECK_APP(osc_setAmplitudeScale(RPAPP_OSC_SOUR_MATH, 1));
    ECHECK_APP(osc_setProbeAtt(RP_CH_1, 1));
    ECHECK_APP(osc_setProbeAtt(RP_CH_2, 1));
    ECHECK_APP(osc_setInputGain(RP_CH_1, RPAPP_OSC_IN_GAIN_LV))
    ECHECK_APP(osc_setInputGain(RP_CH_2, RPAPP_OSC_IN_GAIN_LV))
    ECHECK_APP(osc_setTimeOffset(0));
    ECHECK_APP(osc_setTriggerSlope(RPAPP_OSC_TRIG_SLOPE_PE));

    if (rp_HPGetIsExternalTriggerLevelPresentOrDefault())
        ECHECK_APP(rp_AcqSetTriggerLevel(RP_T_CH_EXT, 0));

    ECHECK_APP(rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE));
    ECHECK_APP(osc_setTriggerLevel(0));
    ECHECK_APP(osc_setTriggerSweep(RPAPP_OSC_TRIG_AUTO));
    ECHECK_APP(osc_setTriggerSource(RPAPP_OSC_TRIG_SRC_CH1));
    ECHECK_APP(osc_setTimeScale(1));
    ECHECK_APP(osc_setMathOperation(RPAPP_OSC_MATH_NONE));
    ECHECK_APP(osc_setMathSources(RP_CH_1, RP_CH_2));

    static auto channels = getADCChannels();

    if (channels >= 3){
        ECHECK_APP(osc_setAmplitudeOffset(RPAPP_OSC_SOUR_CH3, 0));
        ECHECK_APP(osc_setAmplitudeScale(RPAPP_OSC_SOUR_CH3, 1));
        ECHECK_APP(osc_setProbeAtt(RP_CH_3, 1));
        ECHECK_APP(osc_setInputGain(RP_CH_3, RPAPP_OSC_IN_GAIN_LV))
    }

    if (channels >= 4){
        ECHECK_APP(osc_setAmplitudeOffset(RPAPP_OSC_SOUR_CH4, 0));
        ECHECK_APP(osc_setAmplitudeScale(RPAPP_OSC_SOUR_CH4, 1));
        ECHECK_APP(osc_setProbeAtt(RP_CH_4, 1));
        ECHECK_APP(osc_setInputGain(RP_CH_4, RPAPP_OSC_IN_GAIN_LV))
    }

    return RP_OK;
}

int osc_run() {
    g_viewController.runOsc();
    g_viewController.requestUpdateViewFromADC();
    WARNING("osc_run")
    return RP_OK;
}

int osc_stop() {
    g_viewController.stopOsc();
    WARNING("osc_stop")
    return RP_OK;
}

int osc_reset() {
    g_viewController.stopOsc();
    ECHECK_APP(osc_SetDefaultValues());
    WARNING("osc_reset")
    return RP_OK;
}

int osc_single() {
    auto trigSweep = g_adcController.getTriggerSweep();
    if (trigSweep != RPAPP_OSC_TRIG_SINGLE) {
        ECHECK_APP(osc_setTriggerSweep(RPAPP_OSC_TRIG_SINGLE));
    }
    g_viewController.runOsc();
    g_viewController.requestUpdateViewFromADC();
    return RP_OK;
}



int osc_autoScale() {
    auto trigSweep = g_adcController.getTriggerSweep();
    if (trigSweep != RPAPP_OSC_TRIG_AUTO) {
        osc_setTriggerSweep(RPAPP_OSC_TRIG_AUTO);
    }
    // Need for init static variables
    checkAutoscale(false);
    return RP_OK;
}

int osc_getAutoScale(bool *_state){
    *_state = g_viewController.getAutoScale();
    return RP_OK;
}

int osc_isRunning(bool *running) {
    *running = g_viewController.isOscRun();
    return RP_OK;
}

int osc_isTriggered() {
    return g_viewController.isTriggered();
}

int osc_setTimeScale(float _scale) {
    std::lock_guard<std::mutex> lock(g_mutex);
    rp_acq_decimation_t newDecimation;
    auto ret = g_viewController.calculateDecimation(_scale,&newDecimation);
    if (ret != RP_OK){
        WARNING("Can't calculate new decimation for scale %f",_scale)
        return ret;
    }
    g_viewController.setTimeScale(_scale);
    TRACE("NEW TS %f",_scale);
    if (_scale < CONTIOUS_MODE_SCALE_THRESHOLD/* || sweep != RPAPP_OSC_TRIG_AUTO*/){
        ECHECK_APP(g_adcController.setContinuousMode(false))
    } else {
        ECHECK_APP(g_adcController.setContinuousMode(true))
    }
    g_adcController.requestResetWaitTrigger();
    update_view();
    return RP_OK;
}

int osc_getTimeScale(float *division) {
    *division = g_viewController.getTimeScale();
    return RP_OK;
}

int osc_setTimeOffset(float _offset) {
    std::lock_guard<std::mutex> lock(g_mutex);
    auto ret = g_viewController.setTimeOffset(_offset);
    g_adcController.requestResetWaitTrigger();
    update_view();
    return ret;
}

int osc_getTimeOffset(float *offset) {
    *offset = g_viewController.getTimeOffset();
    return RP_OK;
}

int osc_setProbeAtt(rp_channel_t channel, float att) {
    CHECK_CHANNEL()
    CHANNEL_ACTION_4CH(channel,
                   ch_probeAtt[0] = att,
                   ch_probeAtt[1] = att,
                   ch_probeAtt[2] = att,
                   ch_probeAtt[3] = att)

    EXECUTE_ATOMICALLY(g_mutex, update_view());
    return RP_OK;
}

int osc_getProbeAtt(rp_channel_t channel, float *att) {
    CHECK_CHANNEL()
    CHANNEL_ACTION_4CH(channel,
                   *att = ch_probeAtt[0],
                   *att = ch_probeAtt[1],
                   *att = ch_probeAtt[2],
                   *att = ch_probeAtt[3])
    return RP_OK;
}

int osc_setInputGain(rp_channel_t channel, rpApp_osc_in_gain_t gain) {
    CHECK_CHANNEL()
    std::lock_guard<std::mutex> lock(g_mutex);
    switch (gain) {
        case RPAPP_OSC_IN_GAIN_LV:
            ECHECK_APP(rp_AcqSetGain(channel, RP_LOW));
            break;
        case RPAPP_OSC_IN_GAIN_HV:
            ECHECK_APP(rp_AcqSetGain(channel, RP_HIGH));
            break;
        default:
            WARNING("Unknown value %d",gain)
            return RP_EOOR;
    }
    update_view();
    return RP_OK;
}

int osc_getInputGain(rp_channel_t channel, rpApp_osc_in_gain_t *gain) {
    CHECK_CHANNEL()
    rp_pinState_t state;
    ECHECK_APP(rp_AcqGetGain(channel, &state));
    switch (state) {
        case RP_LOW:
            *gain = RPAPP_OSC_IN_GAIN_LV;
            break;
        case RP_HIGH:
            *gain = RPAPP_OSC_IN_GAIN_HV;
            break;
        default:
            WARNING("Unknown value %d",*gain)
            return RP_EOOR;
    }
    return RP_OK;
}

int osc_setAmplitudeScale(rpApp_osc_source source, double scale) {
    double offset, currScale;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        ECHECK_APP(osc_getAmplitudeOffset(source, &offset));
        ECHECK_APP(osc_getAmplitudeScale(source, &currScale));
        offset = offset / currScale;

        SOURCE_ACTION_4CH(source,
                    ch_ampScale[0] = scale,
                    ch_ampScale[1] = scale,
                    ch_ampScale[2] = scale,
                    ch_ampScale[3] = scale,
                    math_ampScale = scale)

        offset *= currScale;
    }
    if (!isnan(offset)) {
        ECHECK_APP(osc_setAmplitudeOffset(source, offset));
    }
    EXECUTE_ATOMICALLY(g_mutex, update_view());
    return RP_OK;
}

int osc_getAmplitudeScale(rpApp_osc_source source, double *scale) {
    SOURCE_ACTION_4CH(source,
                  *scale = ch_ampScale[0],
                  *scale = ch_ampScale[1],
                  *scale = ch_ampScale[2],
                  *scale = ch_ampScale[3],
                  *scale = math_ampScale)
    return RP_OK;
}

int osc_setAmplitudeOffset(rpApp_osc_source source, double offset) {
    std::lock_guard<std::mutex> lock(g_mutex);
    SOURCE_ACTION_4CH(source,
                  ch_ampOffset[0] = offset,
                  ch_ampOffset[1] = offset,
                  ch_ampOffset[2] = offset,
                  ch_ampOffset[3] = offset,
                  math_ampOffset = offset)

    update_view();
    return RP_OK;
}

int osc_getAmplitudeOffset(rpApp_osc_source source, double *offset) {
    SOURCE_ACTION_4CH(source,
                  *offset = ch_ampOffset[0],
                  *offset = ch_ampOffset[1],
                  *offset = ch_ampOffset[2],
                  *offset = ch_ampOffset[3],
                  *offset = math_ampOffset)
    return RP_OK;
}

int osc_setTriggerSource(rpApp_osc_trig_source_t _triggerSource) {
    std::lock_guard<std::mutex> lock(g_mutex);
    auto trigSource = g_adcController.getTriggerSources();
    if (trigSource != _triggerSource) {
        g_viewController.requestUpdateViewFromADC();
    }
    return g_adcController.setTriggerSources(_triggerSource);
}

int osc_getTriggerSource(rpApp_osc_trig_source_t *triggerSource) {
    *triggerSource = g_adcController.getTriggerSources();
    return RP_OK;
}

int osc_setTriggerSlope(rpApp_osc_trig_slope_t _slope) {
    std::lock_guard<std::mutex> lock(g_mutex);
     auto trigSlope = g_adcController.getTriggerSlope();
    if (trigSlope != _slope) {
        g_viewController.requestUpdateViewFromADC();
    }
    return g_adcController.setTriggerSlope(_slope);
}

int osc_getTriggerSlope(rpApp_osc_trig_slope_t *slope) {
    *slope = g_adcController.getTriggerSlope();
    return RP_OK;
}

int osc_setTriggerLevel(float _level) {
    std::lock_guard<std::mutex> lock(g_mutex);   
    ECHECK_APP(g_adcController.setTriggetLevel(_level));
    update_view();
    return RP_OK;
}

int osc_getTriggerLevel(float *_level) {
    return g_adcController.getTriggerLevel(_level);
}

int osc_setTriggerSweep(rpApp_osc_trig_sweep_t sweep) {
    g_adcController.setTriggerSweep(sweep);
     if (g_viewController.getTimeScale() < CONTIOUS_MODE_SCALE_THRESHOLD){
        ECHECK_APP(g_adcController.setContinuousMode(false))
    } else {
        ECHECK_APP(g_adcController.setContinuousMode(true))
    }
    g_viewController.requestUpdateViewFromADC();
    g_adcController.requestResetWaitTrigger();
    return RP_OK;
}

int osc_getTriggerSweep(rpApp_osc_trig_sweep_t *sweep) {
    *sweep = g_adcController.getTriggerSweep();
    return RP_OK;
}

int osc_SetSmoothMode(rp_channel_t _channel, rpApp_osc_interpolationMode _mode){
    std::lock_guard<std::mutex> lock(g_mutex);   
    g_decimator.setInterpolationMode(_channel,_mode);
    return RP_OK;
}

int osc_GetSmoothMode(rp_channel_t _channel, rpApp_osc_interpolationMode *_mode){
    *_mode = g_decimator.getInterpolationMode(_channel);
    return RP_OK;
}

int osc_setInverted(rpApp_osc_source source, bool inverted) {
    std::lock_guard<std::mutex> lock(g_mutex);
    SOURCE_ACTION_4CH(source,
                  ch_inverted[0] = inverted,
                  ch_inverted[1] = inverted,
                  ch_inverted[2] = inverted,
                  ch_inverted[3] = inverted,
                  math_inverted = inverted)
    return RP_OK;
}

int osc_isInverted(rpApp_osc_source source, bool *inverted) {
    SOURCE_ACTION_4CH(source,
                  *inverted = ch_inverted[0],
                  *inverted = ch_inverted[1],
                  *inverted = ch_inverted[2],
                  *inverted = ch_inverted[3],
                  *inverted = math_inverted)
    return RP_OK;
}

int osc_getViewPart(float *ratio) {
    auto timeScale = g_viewController.getTimeScale();
    auto spd = g_viewController.getSamplesPerDivision();
    auto viewSize = g_viewController.getViewSize();
    *ratio = ((float)viewSize * (float)timeToIndexI(timeScale) / spd) 
                 / (float)ADC_BUFFER_SIZE;
    return RP_OK;
}

int osc_measureVpp(rpApp_osc_source _source, float *_Vpp) {
    g_viewController.lockView();
    auto *data = g_viewController.getView(_source);
    auto ret = g_measureController.measureVpp(_source,data,_Vpp);
    g_viewController.unlockView();
    return ret;
}

int osc_measureMax(rpApp_osc_source _source, float *_Max) {
    g_viewController.lockView();
    auto *data = g_viewController.getView(_source);
    auto ret = g_measureController.measureMax(_source,data,_Max);
    g_viewController.unlockView();
    return ret;
}

int osc_measureMin(rpApp_osc_source _source, float *_Min) {
    g_viewController.lockView();
    auto *data = g_viewController.getView(_source);
    auto ret = g_measureController.measureMin(_source,data,_Min);
    g_viewController.unlockView();
    return ret;
}

int osc_measureMeanVoltage(rpApp_osc_source _source, float *_meanVoltage) {
    g_viewController.lockView();
    auto *data = g_viewController.getView(_source);
    auto ret = g_measureController.measureMeanVoltage(_source,data,_meanVoltage);
    g_viewController.unlockView();
    return ret;
}

int osc_measureMaxVoltage(rpApp_osc_source _source, float *_Vmax) {
    g_viewController.lockView();
    auto inverted = (_source != RPAPP_OSC_SOUR_MATH && ch_inverted[(int)_source]) || (_source == RPAPP_OSC_SOUR_MATH && math_inverted);
    auto *data = g_viewController.getView(_source);
    auto ret = g_measureController.measureMaxVoltage(_source,inverted,data,_Vmax);
    g_viewController.unlockView();
    return ret;
}

int osc_measureMinVoltage(rpApp_osc_source _source, float *_Vmin) {
    g_viewController.lockView();
    auto inverted = (_source != RPAPP_OSC_SOUR_MATH && ch_inverted[(int)_source]) || (_source == RPAPP_OSC_SOUR_MATH && math_inverted);
    auto *data = g_viewController.getView(_source);
    auto ret = g_measureController.measureMinVoltage(_source,inverted,data,_Vmin);
    g_viewController.unlockView();
    return ret;
}

int osc_measureFrequency(rpApp_osc_source _source, float *_frequency) {
    float period;
    ECHECK_APP(osc_measurePeriod(_source, &period));
    period = (period == 0.f) ?  0.000001f : period;
    *_frequency = (float) (1 / (period / 1000.0));
    return RP_OK;
}

int osc_measurePeriodMath(float *_period) {
    g_viewController.lockView();
    auto data = g_viewController.getView(RPAPP_OSC_SOUR_MATH);
    auto sampPerDiv = g_viewController.getSamplesPerDivision();
    float tSacale = 0;
    osc_getTimeScale(&tSacale);
    auto ret = g_measureController.measurePeriodMath(tSacale,sampPerDiv,data,_period);
    g_viewController.unlockView();
    return ret;
    
}

int osc_measurePeriodCh(rpApp_osc_source _source, float *_period) {
    rp_channel_t ch = RP_CH_1;
    if (_source == RPAPP_OSC_SOUR_CH2) ch = RP_CH_2;
    if (_source == RPAPP_OSC_SOUR_CH3) ch = RP_CH_3;
    if (_source == RPAPP_OSC_SOUR_CH4) ch = RP_CH_4;
    
    g_viewController.lockView();
    auto *data = g_viewController.getAcqBuffers();
    if (data->channels < ch) {
        g_viewController.unlockView();
        return RP_EOOR;
    }
    auto data_f = data->ch_f[ch];
    auto dataSize = data->size;
    auto ret = g_measureController.measurePeriodCh(data_f,dataSize,_period);
    g_viewController.unlockView();
    return ret;
}

int osc_measurePeriod(rpApp_osc_source _source, float *_period) {
	if(_source == RPAPP_OSC_SOUR_MATH)
		return osc_measurePeriodMath(_period);
	else
		return osc_measurePeriodCh(_source, _period);
}

int osc_measureDutyCycle(rpApp_osc_source _source, float *_dutyCycle) {
    g_viewController.lockView();
    auto *data = g_viewController.getView(_source);
    auto ret = g_measureController.measureDutyCycle(_source,data,_dutyCycle);
    g_viewController.unlockView();
    return ret;
}

int osc_measureRootMeanSquare(rpApp_osc_source _source, float *_rms) {
    g_viewController.lockView();
    auto *data = g_viewController.getView(_source);
    auto ret = g_measureController.measureRootMeanSquare(_source,data,_rms);
    g_viewController.unlockView();
    return ret;
}

int osc_getCursorVoltage(rpApp_osc_source _source, uint32_t _cursor, float *_value) {
    g_viewController.lockView();
    auto view = g_viewController.getView(_source);
    auto value = (*view)[_cursor];
    g_viewController.unlockView();
    return unscaleAmplitudeChannel(_source, value, _value);
}

int osc_getCursorTime(uint32_t _cursor, float *_value) {
    if (static_cast<vsize_t>(_cursor) >= g_viewController.getViewSize()) {
        return RP_EOOR;
    }
    *_value = g_viewController.viewIndexToTime(_cursor);
    return RP_OK;
}

int osc_getCursorDeltaTime(uint32_t _cursor1, uint32_t _cursor2, float *_value) {
    auto viewSize = g_viewController.getViewSize();
    if (static_cast<vsize_t>(_cursor1) >= viewSize || static_cast<vsize_t>(_cursor2) >= viewSize) {
        return RP_EOOR;
    }
    *_value = indexToTime(abs((int64_t)_cursor1 - (int64_t)_cursor2));
    return RP_OK;
}

int oscGetCursorDeltaAmplitude(rpApp_osc_source _source, uint32_t _cursor1, uint32_t _cursor2, float *_value) {
    auto viewSize = g_viewController.getViewSize();
    if (static_cast<vsize_t>(_cursor1) >= viewSize || static_cast<vsize_t>(_cursor2) >= viewSize) {
        return RP_EOOR;
    }
    float cursor1Amplitude, cursor2Amplitude;
    ECHECK_APP(osc_getCursorVoltage(_source, _cursor1, &cursor1Amplitude));
    ECHECK_APP(osc_getCursorVoltage(_source, _cursor2, &cursor2Amplitude));
    *_value = (float) fabs(cursor2Amplitude - cursor1Amplitude);
    return RP_OK;
}

int osc_getCursorDeltaFrequency(uint32_t _cursor1, uint32_t _cursor2, float *_value) {
    auto viewSize = g_viewController.getViewSize();
    if (static_cast<vsize_t>(_cursor1) >= viewSize || static_cast<vsize_t>(_cursor2) >= viewSize) {
        return RP_EOOR;
    }
    float deltaTime;
    ECHECK_APP(osc_getCursorDeltaTime(_cursor1, _cursor2, &deltaTime));
    *_value = 1 / deltaTime;
    return RP_OK;
}

int osc_setMathOperation(rpApp_osc_math_oper_t _op) {
    std::lock_guard<std::mutex> lock(g_mutex);
    operation = _op;
    update_view();
    return RP_OK;
}

int osc_getMathOperation(rpApp_osc_math_oper_t *_op) {
    *_op = operation;
    return RP_OK;
}

int osc_setMathSources(rp_channel_t _source1, rp_channel_t _source2) {
    std::lock_guard<std::mutex> lock(g_mutex);
    mathSource1 = _source1;
    mathSource2 = _source2;
    update_view();
    return RP_OK;
}

int osc_getMathSources(rp_channel_t *_source1, rp_channel_t *_source2) {
    *_source1 = mathSource1;
    *_source2 = mathSource2;
    return RP_OK;
}

int osc_getData(rpApp_osc_source _source, float *_data, uint32_t _size) {
    g_viewController.lockView();
    auto view = g_viewController.getView(_source);
    if (view->size() < _size){
        g_viewController.unlockView();
        WARNING("There's less data %d than required %d",view->size() ,_size)
        return RP_EOOR;
    }
    memcpy_neon(_data,view->data(),_size * sizeof(float));
    g_viewController.unlockView();
    return RP_OK;
}

int osc_getExportedData(rpApp_osc_source _source, rpApp_osc_exportMode _mode, bool _normalize, float *_data, uint32_t *_size){

    auto norma = [](float *data, uint32_t size){
        if (size == 0) return;

        float max = data[0];
        float min = data[0];
        for(auto i = 1u; i < size; ++i){
            if (max < data[i]){
                max = data[i];
            }
            if (min > data[i]){
                min = data[i];
            }
        }
        float amp = (max - min) / 2.0;
        for(auto i = 0u; i < size; ++i){
            data[i] -= (amp + min);
            data[i] /= amp;
        }
    };

    if (_mode == RPAPP_VIEW_EXPORT){
        uint32_t ret_size = *_size;

        auto ret = osc_getData(_source,_data,ret_size);
        if (ret != RP_OK){
            *_size = 0;
            return ret;
        }
   
        for(auto i = 0u; i < *_size; ++i){
            unscaleAmplitudeChannel(_source, _data[i],&_data[i]);
        }

        if (_normalize){
            norma(_data,ret_size);
        }
        return RP_OK;
    }

    if (_mode == RPAPP_RAW_EXPORT){

        if (_source != RPAPP_OSC_SOUR_MATH){
            g_viewController.lockView();
            auto view = g_viewController.getOriginalData(_source);
            if (view->size() > *_size){
                g_viewController.unlockView();
                WARNING("There's more data %d than required %d",view->size() ,*_size)
                *_size = 0;
                return RP_EOOR;
            }
    
            memcpy_neon(_data,view->data(),view->size() * sizeof(float));
        
            g_viewController.unlockView();

            *_size = view->size();
            
            if (_normalize){
                norma(_data,view->size());
            }
            return RP_OK;
        }
    }

    return RP_EOOR;
}


int osc_getRawData(rp_channel_t _source, uint16_t *_data, uint32_t _size) {
    g_viewController.lockView();
    auto rawData = g_viewController.getAcqBuffers();
    if (rawData->channels < _source) {
        g_viewController.unlockView();
        WARNING("Channel selection error")
        return RP_EOOR;
    } 

    if (rawData->size < _size){
        g_viewController.unlockView();
        WARNING("There's less data %d than required %d",rawData->size,_size)
        return RP_EOOR;
    }
    memcpy_neon(_data,rawData->ch_i[_source],_size * sizeof(uint16_t));
    g_viewController.unlockView();
    return RP_OK;
}

int osc_setViewSize(uint32_t _size) {
    g_viewController.lockView();
    g_viewController.setViewSize(_size);
    update_view();
    g_viewController.unlockView();
    return RP_OK;
}

int osc_getViewSize(uint32_t *_size) {
    *_size = g_viewController.getViewSize();
    return RP_OK;
}

int osc_getViewLimits(uint32_t* _start, uint32_t* _end) {
    if(g_viewController.getAutoScale()) {
        *_start = 0;
        *_end = 0;
    } else {
        *_start = 0;
        *_end = g_viewController.getViewSize();
    }
    return RP_OK;
}

int osc_refreshViewData(){
    update_view();
    return RP_OK;
}



/*
* Utils
*/

int threadSafe_acqStart() {
    return g_adcController.startAcq();
}

int threadSafe_acqStop() {
    return g_adcController.stopAcq();
}

double scaleAmplitude(double _volts, double _ampScale, double _probeAtt, double _ampOffset, double _invertFactor) {
    return (_volts * _invertFactor * _probeAtt + _ampOffset) / _ampScale;
}

double unscaleAmplitude(double _value, double _ampScale, double _probeAtt, double _ampOffset, double _invertFactor) {
    return ((_value * _ampScale) - _ampOffset) / _probeAtt / _invertFactor;
}

double unOffsetAmplitude(double _value, double _ampScale, double _ampOffset) {
    return _value - (_ampOffset / _ampScale);
}

int scaleAmplitudeChannel(rpApp_osc_source _source, float _volts, float *_res) {
    double ampOffset, ampScale;
    float probeAtt = 1;
    bool inverted;
    ECHECK_APP(osc_getAmplitudeOffset(_source, &ampOffset));
    ECHECK_APP(osc_getAmplitudeScale(_source, &ampScale));
    if (_source != RPAPP_OSC_SOUR_MATH)
        ECHECK_APP(osc_getProbeAtt((rp_channel_t)_source, &probeAtt));
    ECHECK_APP(osc_isInverted(_source, &inverted));
    *_res = scaleAmplitude(_volts, ampScale, probeAtt, ampOffset, inverted ? -1 : 1);
    return RP_OK;
}

int unscaleAmplitudeChannel(rpApp_osc_source _source, float _value, float *_res) {
    double ampOffset, ampScale;
    float probeAtt=1;
    bool inverted;
    ECHECK_APP(osc_getAmplitudeOffset(_source, &ampOffset));
    ECHECK_APP(osc_getAmplitudeScale(_source, &ampScale));
    ECHECK_APP(osc_isInverted(_source, &inverted));
    if (_source != RPAPP_OSC_SOUR_MATH)
        ECHECK_APP(osc_getProbeAtt((rp_channel_t)_source, &probeAtt));
    *_res = unscaleAmplitude(_value, ampScale, probeAtt, ampOffset, inverted ? -1 : 1);
    return RP_OK;
}

int unOffsetAmplitudeChannel(rpApp_osc_source _source, float _value, float *_res) {
    double ampOffset, ampScale;
    ECHECK_APP(osc_getAmplitudeOffset(_source, &ampOffset));
    ECHECK_APP(osc_getAmplitudeScale(_source, &ampScale));
    *_res = unOffsetAmplitude(_value, ampScale, ampOffset);
    return RP_OK;
}

int attenuateAmplitudeChannel(rpApp_osc_source _source, float _value, float *_res) {
    float probeAtt = 1.f;
    if (_source != RPAPP_OSC_SOUR_MATH)
        ECHECK_APP(osc_getProbeAtt((rp_channel_t)_source, &probeAtt));

    *_res = scaleAmplitude(_value, 1.f, probeAtt, 0.f, 1.f);
    return RP_OK;
}

int unattenuateAmplitudeChannel(rpApp_osc_source _source, float _value, float *_res) {
    float probeAtt = 1.f;
    if (_source != RPAPP_OSC_SOUR_MATH)
        ECHECK_APP(osc_getProbeAtt((rp_channel_t)_source, &probeAtt));

    *_res = unscaleAmplitude(_value, 1.f, probeAtt, 0.f, 1.f);
    return RP_OK;
}

void calculateIntegral(rp_channel_t _channel, float _scale, float _offset, float _invertFactor) {
    auto timeScale = g_viewController.getTimeScale();
    auto samplesPerDivision = g_viewController.getSamplesPerDivision();

    auto dt = timeScale / samplesPerDivision;
    auto v = 0.0f;

    bool invert = ch_inverted[(int)_channel];
    float ch_sign = invert ? -1.f : 1.f;

    g_viewController.lockView();
    auto view = g_viewController.getView((rpApp_osc_source) _channel);
    auto viewMath = g_viewController.getView(RPAPP_OSC_SOUR_MATH);
    auto viewSize = g_viewController.getViewSize();
    ECHECK_APP_NO_RET(unscaleAmplitudeChannel((rpApp_osc_source) _channel, (*view)[0], &v));
	ECHECK_APP_NO_RET(attenuateAmplitudeChannel((rpApp_osc_source) _channel, v, &v));

    (*viewMath)[0] = ch_sign * v * dt;

    for (int i = 1; i < viewSize; ++i) {
        ECHECK_APP_NO_RET(unscaleAmplitudeChannel((rpApp_osc_source) _channel, (*view)[i], &v));
		ECHECK_APP_NO_RET(attenuateAmplitudeChannel((rpApp_osc_source) _channel, v, &v));
        (*viewMath)[i] = (*viewMath)[i-1] + (ch_sign * v * dt);
        (*viewMath)[i-1] = scaleAmplitude((*viewMath)[i-1], _scale, 1, _offset, _invertFactor);
    }
    (*viewMath)[viewSize - 1] = scaleAmplitude((*viewMath)[viewSize - 1], _scale, 1, _offset, _invertFactor);
    g_viewController.unlockView();
}

void calculateDevivative(rp_channel_t _channel, float _scale, float _offset, float _invertFactor) {
    auto timeScale = g_viewController.getTimeScale();
    auto samplesPerDivision = g_viewController.getSamplesPerDivision();

    float dt2 = 2.0f * timeScale / 1000.0f / samplesPerDivision;
    float v1, v2;

    bool invert = ch_inverted[(int)_channel];
    float ch_sign = invert ? -1.f : 1.f;

    g_viewController.lockView();
    auto view = g_viewController.getView((rpApp_osc_source) _channel);
    auto viewMath = g_viewController.getView(RPAPP_OSC_SOUR_MATH);
    auto viewSize = g_viewController.getViewSize();

    ECHECK_APP_NO_RET(unscaleAmplitudeChannel((rpApp_osc_source) _channel, (*view)[0], &v2));
	ECHECK_APP_NO_RET(attenuateAmplitudeChannel((rpApp_osc_source) _channel, v2, &v2));
    for (int i = 0; i < viewSize - 1; ++i) {
        v1 = v2;
        ECHECK_APP_NO_RET(unscaleAmplitudeChannel((rpApp_osc_source) _channel, (*view)[i + 1], &v2));
		ECHECK_APP_NO_RET(attenuateAmplitudeChannel((rpApp_osc_source) _channel, v2, &v2));
        (*viewMath)[i] = scaleAmplitude(ch_sign * (v2 - v1) / dt2, _scale, 1, _offset, _invertFactor);
    }
    (*viewMath)[viewSize - 1] = (*viewMath)[viewSize - 2];
    g_viewController.unlockView();
}

float calculateMath(float _v1, float _v2, rpApp_osc_math_oper_t _op) {
    float ret = 0;
    switch (_op) {
        case RPAPP_OSC_MATH_ADD:
            ret = _v1 + _v2;
            break;
        case RPAPP_OSC_MATH_SUB:
            ret = _v1 - _v2;
            break;
        case RPAPP_OSC_MATH_MUL:
            ret = _v1 * _v2;
            break;
        case RPAPP_OSC_MATH_DIV:
            if (_v2 != 0)
                ret = _v1 / _v2;
            else
                ret = _v1 > 0 ? FLT_MAX * 0.9f : -FLT_MAX * 0.9f;
            break;
        case RPAPP_OSC_MATH_ABS:
            ret = (float) fabs(_v1);
            break;
        default:
            return 0;
    }
    return ret;
}

double roundUpTo125(double data) {
    double power = ceil(log(data) / log(10)) - 1;       // calculate normalization factor
    double dataNorm = data / pow(10, power);            // normalize data, so that 1 < data < 10
    if (dataNorm < 2)                                   // round normalized data
        dataNorm = 2;
    else if (dataNorm < 5)
        dataNorm = 5;
    else
        dataNorm = 10;
    return (dataNorm * pow(10, power));         // unnormalize data
}

double roundUpTo25(double data) {
    double power = ceil(log(data) / log(10)) - 1;       // calculate normalization factor
    double dataNorm = data / pow(10, power);            // normalize data, so that 1 < data < 10
    if (dataNorm < 2)                                   // round normalized data
        dataNorm = 2;
    else if (dataNorm < 5)
        dataNorm = 5;
    else
        dataNorm = 20;
    return (dataNorm * pow(10, power));         // unnormalize data
}

int waitToFillPreTriggerBuffer(float _timescale) {
    auto contMode = g_adcController.getContinuousMode();
    auto trigSweep = g_adcController.getTriggerSweep();
    // Don't wait in continuos mode
    if (contMode && trigSweep != RPAPP_OSC_TRIG_SINGLE) {
        return RP_OK;
    }

    // Max timeout 1 second
    auto timeOut  = MIN(g_viewController.calculateTimeOut(_timescale),1000.0);

    timeOut += g_viewController.getClock();

    uint32_t preTriggerCount;
    int triggerDelay;
    auto viewSize = g_viewController.getViewSize();
    auto samplesPerDivision = g_viewController.getSamplesPerDivision();
    auto deltaSample = timeToIndexD(_timescale) / samplesPerDivision;
    auto viewInSamples = viewSize * deltaSample + 4;
    auto needWaitSamples = 0u;
    auto exitByTimout = false;
    auto exitByPreTrigger = false;

    do {
        ECHECK_APP(rp_AcqGetTriggerDelay(&triggerDelay));
        ECHECK_APP(rp_AcqGetPreTriggerCounter(&preTriggerCount));
        needWaitSamples = viewInSamples - triggerDelay;
        exitByTimout = timeOut > g_viewController.getClock();
        exitByPreTrigger = preTriggerCount < needWaitSamples;
    } while (exitByPreTrigger && exitByTimout);
    return RP_OK;
}


int waitTrigger(float _timescale,bool _disableTimeout,bool *_isresetted,bool *_exitByTimeout) {
    auto trig_state = RP_TRIG_STATE_WAITING;
    auto timeout_state = false;
    // Max timeout 1 second
    auto timeOut  = MIN(g_viewController.calculateTimeOut(_timescale),1000.0);
    timeOut += g_viewController.getClock();
    
    g_adcController.resetWaitTriggerRequest();
    *_isresetted = false;
    *_exitByTimeout = false;

    g_viewController.setTriggerState(false);
    do {
        if (g_adcController.isNeedResetWaitTrigger()){
            *_isresetted = true;
            break;
        }
        timeout_state = _disableTimeout ? true : timeOut > g_viewController.getClock();
        ECHECK_APP(rp_AcqGetTriggerState(&trig_state));
    } while ((trig_state != RP_TRIG_STATE_TRIGGERED) && timeout_state);
    g_viewController.setTriggerState(trig_state == RP_TRIG_STATE_TRIGGERED);
    *_exitByTimeout = (!timeout_state) && (trig_state != RP_TRIG_STATE_TRIGGERED);
    return RP_OK;
}

int waitToFillAfterTriggerBuffer(float _timescale) {
    auto contMode = g_adcController.getContinuousMode();
    auto trigSweep = g_adcController.getTriggerSweep();
    // Don't wait in continuos mode
    if (contMode && trigSweep != RPAPP_OSC_TRIG_SINGLE) {
        return RP_OK;
    }

    // Max timeout 1 second
    auto timeOut  = MIN(g_viewController.calculateTimeOut(_timescale),1000.0);
    timeOut += g_viewController.getClock();
    bool bufferIsFill = false;
    do {
        ECHECK_APP(rp_AcqGetBufferFillState(&bufferIsFill));
    } while (!bufferIsFill && timeOut > g_viewController.getClock());
    // fprintf(stderr,"Buff is fill %d\n",bufferIsFill);
    return RP_OK;
}

/*
* Thread functions
*/

int osc_scaleMath() {
    if (!g_viewController.getAutoScale()) {
        float vpp, vMean;
        ECHECK_APP_NO_RET(osc_measureVpp(RPAPP_OSC_SOUR_MATH, &vpp));
        ECHECK_APP_NO_RET(osc_measureMeanVoltage(RPAPP_OSC_SOUR_MATH, &vMean));
        // Calculate scale
        float scale = vpp * AUTO_SCALE_AMP_SCA_FACTOR / DIVISIONS_COUNT_Y;
		if (scale <= FLOAT_EPS) {
			ECHECK_APP_NO_RET(osc_setAmplitudeScale(RPAPP_OSC_SOUR_MATH, roundUpTo25(1)));
		} else {
			ECHECK_APP_NO_RET(osc_setAmplitudeScale(RPAPP_OSC_SOUR_MATH, roundUpTo25(scale)));
			ECHECK_APP_NO_RET(osc_setAmplitudeOffset(RPAPP_OSC_SOUR_MATH, -vMean));
		}
	}
    return RP_OK;
}

void mathThreadFunction() {
    if (operation != RPAPP_OSC_MATH_NONE) {
        bool invert;
        ECHECK_APP_NO_RET(osc_isInverted(RPAPP_OSC_SOUR_MATH, &invert))
        float invertFactor = invert ? -1 : 1;
        if (operation == RPAPP_OSC_MATH_DER) {
            calculateDevivative(mathSource1, math_ampScale, math_ampOffset, invertFactor);
        } else if (operation == RPAPP_OSC_MATH_INT) {
            calculateIntegral(mathSource1, math_ampScale, math_ampOffset, invertFactor);
        } else {
            float v1, v2;
            bool invert1 = ch_inverted[mathSource1];
            bool invert2 = ch_inverted[mathSource2];
            float sign1 = invert1 ? -1.f : 1.f;
            float sign2 = invert2 ? -1.f : 1.f;

            g_viewController.lockView();
            auto viewSize = g_viewController.getViewSize();
            auto viewS1 = g_viewController.getView((rpApp_osc_source) mathSource1);
            auto viewS2 = g_viewController.getView((rpApp_osc_source) mathSource2);
            auto viewMath = g_viewController.getView(RPAPP_OSC_SOUR_MATH);
            for (vsize_t i = 0; i < viewSize; ++i) {
                ECHECK_APP_NO_RET(unscaleAmplitudeChannel((rpApp_osc_source) mathSource1, (*viewS1)[i], &v1));
                ECHECK_APP_NO_RET(unscaleAmplitudeChannel((rpApp_osc_source) mathSource2, (*viewS2)[i], &v2));

                ECHECK_APP_NO_RET(attenuateAmplitudeChannel((rpApp_osc_source) mathSource1, v1, &v1));
                ECHECK_APP_NO_RET(attenuateAmplitudeChannel((rpApp_osc_source) mathSource2, v2, &v2));
                auto v = scaleAmplitude(calculateMath(sign1 * v1, sign2 * v2, operation), math_ampScale, 1, math_ampOffset, invertFactor);
                (*viewMath)[i] = v;
            }
             g_viewController.unlockView();
        }
    }
}

void checkAutoscale(bool fromThread) {
    auto autoScale = g_viewController.getAutoScale();
    if((autoScale == false) && (fromThread == true)){
        return;
    }

    static const float scales[AUTO_SCALE_NUM_OF_SCALE] = {0.00005f, 0.0001f, 0.0002f, 0.0005f, 0.001f, 0.002f, 0.005f, 0.01f, 0.02f, 0.05f, 0.1f, 0.2f, 0.5f, 1.f, 2.f, 5.f, 10.f, 20.f, 50.f, 100.f};
    static int timeScaleIdx = 0;
    constexpr int allChannels = RPAPP_OSC_SOUR_MATH + 1;
    static float periods[allChannels][AUTO_SCALE_NUM_OF_SCALE];
    static float vpps[allChannels][AUTO_SCALE_NUM_OF_SCALE];
    static float vMeans[allChannels][AUTO_SCALE_NUM_OF_SCALE];
    static float savedTimeScale;
    static int measCount;

    float period, vpp, vMean;
    int ret;

    int periodsIdx[MAX_ADC_CHANNELS];
    int repCounts[MAX_ADC_CHANNELS];

    int vMeansIdx[MAX_ADC_CHANNELS];
    int vMeansRepCounts[MAX_ADC_CHANNELS];

    float period_to_set = 1.f;

    if(!fromThread) {
        if(autoScale) {
            return;
        }
        g_viewController.setAutoScale(true);
        osc_getTimeScale(&savedTimeScale);
        timeScaleIdx = 0;
        period_to_set = scales[timeScaleIdx];
        measCount = 0;
        
    } else {       
        if(++measCount < 2) {
            return;
        }

        measCount = 0;

        auto channels = getADCChannels();
        for (int source = RPAPP_OSC_SOUR_CH1; source <= RPAPP_OSC_SOUR_MATH; ++source) {
            if (source < channels || source == RPAPP_OSC_SOUR_MATH){
                ECHECK_APP_NO_RET(osc_measureVpp((rpApp_osc_source)source, &vpp));
                ECHECK_APP_NO_RET(osc_measureMeanVoltage((rpApp_osc_source)source, &vMean));            
                if (fabs(vpp) > SIGNAL_EXISTENCE) {
                    ret = osc_measurePeriod((rpApp_osc_source)source, &period);
                    periods[source][timeScaleIdx] = (ret == RP_OK) ? period : 0.f;
                } else {
                    periods[source][timeScaleIdx] = 0.f;
                }

                vpps[source][timeScaleIdx] = vpp;
                vMeans[source][timeScaleIdx] = vMean;
                WARNING("Source %d ts %d per %f vpp %f",source,timeScaleIdx,periods[source][timeScaleIdx],vpp);
            }
            
        }


        if(++timeScaleIdx >= AUTO_SCALE_NUM_OF_SCALE) {
            WARNING("stop scale")
            g_viewController.setAutoScale(false);

            for (auto source = 0; source < channels; ++source) {
                repCounts[source] = 0;
                vMeansRepCounts[source] = 0;

                for(int i = (AUTO_SCALE_NUM_OF_SCALE - 1); i >= 1; --i) {
                    int count = 0;

                    for(int j = (i - 1); j >= 0; --j) {
                        if(fabs((vMeans[source][i] - vMeans[source][j]) / vMeans[source][i]) < AUTO_SCALE_VMEAN_ERROR)
                            count++;
                    }

                    if(count > vMeansRepCounts[source]) {
                        vMeansRepCounts[source] = count;
                        vMeansIdx[source] = i;
                    }

                    count = 0;
                    if(fabs(periods[source][i])  < 0.00001)
                        continue;

                    for(int j = (i - 1); j >= 0; --j) {
                        if(fabs(periods[source][j])  < 0.00001)
                            continue;

                        if(fabs((periods[source][i] - periods[source][j]) / periods[source][i]) < AUTO_SCALE_PERIOD_ERROR)
                            count++;
                    }

                    if(count > repCounts[source]) {
                        repCounts[source] = count;
                        periodsIdx[source] = i;
                    }
                }
            }

            if((repCounts[0] >= PERIOD_REP_COUNT_MIN) && (repCounts[1] >= PERIOD_REP_COUNT_MIN)) {
                period_to_set = MIN(periods[0][periodsIdx[0]], periods[1][periodsIdx[1]]);
                period_to_set = period_to_set * AUTO_SCALE_PERIOD_COUNT / DIVISIONS_COUNT_X;
            } else if(repCounts[0] > 0 && channels >= 1) {
                period_to_set = periods[0][periodsIdx[0]] * AUTO_SCALE_PERIOD_COUNT / DIVISIONS_COUNT_X;
            } else if(repCounts[1] > 0 && channels >= 2) {
                period_to_set = periods[1][periodsIdx[1]] * AUTO_SCALE_PERIOD_COUNT / DIVISIONS_COUNT_X;
            } else if(repCounts[2] > 0 && channels >= 3) {
                period_to_set = periods[2][periodsIdx[2]] * AUTO_SCALE_PERIOD_COUNT / DIVISIONS_COUNT_X;
            } else if(repCounts[3] > 0  && channels >=4) {
                period_to_set = periods[3][periodsIdx[3]] * AUTO_SCALE_PERIOD_COUNT / DIVISIONS_COUNT_X;
            } else {
                period_to_set = savedTimeScale;
            }

            period_to_set = MAX(0.0001f, period_to_set);
            period_to_set = MIN(500.f, period_to_set);



            for (auto source = 0; source <= RPAPP_OSC_SOUR_MATH; ++source) {
                if (source < channels || source == RPAPP_OSC_SOUR_MATH){

                    vpp = vpps[source][0];
                    for(int i = 1; i < AUTO_SCALE_NUM_OF_SCALE; ++i) {
                        vpp = MAX(vpp, vpps[source][i]);
                    }

                    if(vMeansRepCounts[source] >= VMEAN_REP_COUNT_MIN) {
                        vMean = vMeans[source][vMeansIdx[source]];
                    } else {
                        vMean = vMeans[source][0];

                        for(int i = 1; i < AUTO_SCALE_NUM_OF_SCALE; ++i) {
                            if(fabs(vMean) > fabs(vMeans[source][i])) {
                                vMean = vMeans[source][i];
                            }
                        }
                    }

                    ECHECK_APP_NO_RET(scaleChannel((rpApp_osc_source)source, vpp, vMean));
                }
            }
        } else {
            period_to_set = scales[timeScaleIdx];
        }
    }
    
    ECHECK_APP_NO_RET(osc_setTimeScale(period_to_set));
    ECHECK_APP_NO_RET(osc_setTimeOffset(AUTO_SCALE_TIME_OFFSET));
}

void mainThreadFun() {
    auto pPosition = 0u;
    auto trigLevel = 0.0f;
    auto adc_channels = getADCChannels();
    
    while (g_threadRun) {

        auto tScaleAcq = 0.0f;
        auto tOffsetAcq = 0.0f;
        auto initFromAcq = false;

        if (g_viewController.isNeedUpdateViewFromADC() && g_viewController.isOscRun()){

            g_viewController.lockView();
            initFromAcq = true;
            auto decimationInACQ = g_viewController.getCurrentDecimation();
            tScaleAcq = g_viewController.getTimeScale();
            tOffsetAcq = g_viewController.getTimeOffset();
            // Need set before calculate trigger deleay
            ECHECK_APP_NO_RET(rp_AcqSetDecimation(decimationInACQ));
            auto delay = g_viewController.getSampledAfterTriggerInView();
            ECHECK_APP_NO_RET(rp_AcqSetTriggerDelayDirect(delay));           
            g_viewController.unlockView();

            ECHECK_APP_NO_RET(rp_AcqSetTriggerSrc(RP_TRIG_SRC_DISABLED));
            ECHECK_APP_NO_RET(threadSafe_acqStart());
           
            auto trigSweep = g_adcController.getTriggerSweep();
            auto trigSource = g_adcController.getTriggerSources();
            auto contMode = g_adcController.getContinuousMode();
            auto viewMode = g_viewController.getViewMode();

            ECHECK_APP_NO_RET(waitToFillPreTriggerBuffer(tScaleAcq));
            ECHECK_APP_NO_RET(osc_setTriggerSource(trigSource));
            auto isReset = false;
            auto disableTimeout = false;
            auto exitByTimeout = false;
            
            if (trigSweep == RPAPP_OSC_TRIG_NORMAL || trigSweep == RPAPP_OSC_TRIG_SINGLE){
                disableTimeout = true;
            }
            
            waitTrigger(tScaleAcq,disableTimeout,&isReset,&exitByTimeout);
            
            if (isReset){
                continue;
            }

            if (exitByTimeout){
                ECHECK_APP_NO_RET(rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW));
            }

            waitToFillAfterTriggerBuffer(tScaleAcq);

            if (viewMode == CViewController::ROLL && contMode){
                ECHECK_APP_NO_RET(rp_AcqGetWritePointer(&pPosition));
            }else{
                ECHECK_APP_NO_RET(rp_AcqGetWritePointerAtTrig(&pPosition));
            }
            g_viewController.lockView();
            g_viewController.setCapturedDecimation(decimationInACQ);
            auto buff = g_viewController.getAcqBuffers();
            ECHECK_APP_NO_RET(rp_AcqGetData(pPosition,buff));
            g_viewController.unlockView();
            
            if (!contMode){
                ECHECK_APP_NO_RET(threadSafe_acqStop());
            }

            g_viewController.updateViewFromADCDone();
            g_viewController.requestUpdateView();

            if (trigSweep == RPAPP_OSC_TRIG_SINGLE){
                osc_stop();
            }
        }

        if (g_viewController.isNeedUpdateView()){
            g_mutex.lock();
            g_viewController.lockView();
            
            auto contMode = g_adcController.getContinuousMode();
            auto viewMode =  g_viewController.getViewMode();
            auto spd = g_viewController.getSamplesPerDivision();
            auto tScale = initFromAcq ? tScaleAcq : g_viewController.getTimeScale();
            auto tOffset = initFromAcq ? tOffsetAcq : g_viewController.getTimeOffset();
            auto trigSource = g_adcController.getTriggerSources();
            auto _deltaSample = timeToIndexD(tScale) / (double)spd;
            ECHECK_APP_NO_RET(g_adcController.getTriggerLevel(&trigLevel));
            g_decimator.setDecimationFactor(_deltaSample);
            g_decimator.setTriggerLevel(trigLevel);
            int posInPoints = ((tOffset / tScale) * spd);
            auto buff = g_viewController.getAcqBuffers();
            g_decimator.resetOffest();
            auto tsChannel = convertCh(trigSource);
            if (tsChannel != -1){
                g_decimator.precalculateOffset(buff->ch_f[tsChannel],ADC_BUFFER_SIZE); 
            }
            for (auto channel = 0u; channel < adc_channels; ++channel) {
                auto view = g_viewController.getView((rpApp_osc_source)channel);
                auto orignalData = g_viewController.getOriginalData((rpApp_osc_source)channel);
                auto viewSize = g_viewController.getViewSize();
                if (viewMode == CViewController::ROLL && contMode){
                    posInPoints = -viewSize / 2.0;
                }
                g_decimator.decimate((rp_channel_t) channel,buff->ch_f[channel],ADC_BUFFER_SIZE,posInPoints,view ,orignalData);
            }
            g_viewController.unlockView();
            mathThreadFunction();
            g_mutex.unlock();
            g_viewController.updateViewDone();
            checkAutoscale(true);
        }
    }
}
