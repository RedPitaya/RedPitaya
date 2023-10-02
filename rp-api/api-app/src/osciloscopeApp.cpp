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

#include "osciloscopeApp.h"
#include "common.h"
#include "neon_asm.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define FLOAT_EPS 0.00001f

#define CONTIOUS_MODE_SCALE_THRESHOLD 1 // ms
#define WAIT_TO_FILL_BUF_TIMEOUT      500.f //(2*CLOCKS_PER_SEC)


volatile bool acqRunning = false;
volatile bool oscRunning = false;
volatile bool clear = false;
volatile bool continuousMode = false;
volatile int32_t viewSize = VIEW_SIZE_DEFAULT;
float *view;
volatile double ch_ampOffset[MAX_ADC_CHANNELS], math_ampOffset;
volatile double ch_ampScale[MAX_ADC_CHANNELS],  math_ampScale = 1;
volatile float ch_probeAtt[MAX_ADC_CHANNELS];
volatile bool ch_inverted[MAX_ADC_CHANNELS], math_inverted = false;
volatile float timeScale=1, timeOffset=0;
volatile rpApp_osc_trig_sweep_t trigSweep;
volatile rpApp_osc_trig_source_t trigSource = RPAPP_OSC_TRIG_SRC_CH1;
volatile rpApp_osc_trig_slope_t trigSlope = RPAPP_OSC_TRIG_SLOPE_PE;
volatile rpApp_osc_math_oper_t operation;
volatile rp_channel_t mathSource1, mathSource2;
volatile bool updateView = false;
volatile bool autoScale = false;

volatile uint16_t *raw_data[MAX_ADC_CHANNELS] = { nullptr,nullptr,nullptr,nullptr };

volatile float samplesPerDivision = (float) VIEW_SIZE_DEFAULT / (float) DIVISIONS_COUNT_X;

volatile double threadTimer;

volatile double g_triggerTS = 0;

volatile int32_t viewStartPos = 0;
volatile int32_t viewEndPos = VIEW_SIZE_DEFAULT;

pthread_t mainThread = (pthread_t) -1;
std::recursive_mutex mutex;

void *mainThreadFun(void* argA);

static inline double _clock() {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return ((double)tp.tv_sec * 1000.f) + ((double)tp.tv_nsec / 1000000.f);
}

static inline float sign(float a) {
    return (a < 0.f) ? -1.f : 1.f;
}

static inline double linear(float x0, float y0, float x1, float y1, double x) {
    double k = (y1 - y0) / (x1 - x0);
    double b = y0 - (k * x0);
    return (k * x) + b;
}

static inline void update_view() {
    if((trigSweep == RPAPP_OSC_TRIG_AUTO) && oscRunning) {
        clearView();
        updateView = false;
    } else {
        updateView = true;
    }
}

static inline int scaleChannel(rp_channel_t channel, float vpp, float vMean) {

    CHECK_CHANNEL("scaleChannel");

    rpApp_osc_source src;
    switch (channel)
    {
    case RP_CH_1:
        src = RPAPP_OSC_SOUR_CH1;
        break;
    case RP_CH_2:
        src = RPAPP_OSC_SOUR_CH2;
        break;
    case RP_CH_3:
        src = RPAPP_OSC_SOUR_CH3;
        break;
    case RP_CH_4:
        src = RPAPP_OSC_SOUR_CH4;
        break;
    default:
        fprintf(stderr,"[Fatal Error] scaleChannel: unknown value %d\n",channel);
        return RP_EOOR;
    }
    float scale1 = (float) (vpp * AUTO_SCALE_AMP_SCA_FACTOR / DIVISIONS_COUNT_Y);
    float scale2 = (float) ((fabs(vpp) + (fabs(vMean) * 2.f)) * AUTO_SCALE_AMP_SCA_FACTOR / DIVISIONS_COUNT_Y);
    float scale = MAX(MAX(scale1, scale2), 0.002);
    ECHECK_APP(osc_setAmplitudeScale(src, roundUpTo125(scale)));
    ECHECK_APP(osc_setAmplitudeOffset(src, -vMean));
    return RP_OK;
}

inline double cnvSmplsToTime(int32_t samples)
{
    auto rate = getADCRate();
    /* Calculate time (including decimation) */
    uint32_t decimation;
    rp_AcqGetDecimationFactor(&decimation);
    return (double)samples * ((double)decimation / rate) * 1000 ;
}

inline double calculateTimeOut(float timeScale){

    double timeout = MAX(0.1f , (2.f * timeScale * (float)DIVISIONS_COUNT_X));
    return timeout;
}


void checkAutoscale(bool fromThread);

int osc_Init() {
    view = (float*)calloc((RPAPP_OSC_SOUR_MATH + 1) * viewSize, sizeof(float));
    viewStartPos = 0;
    viewEndPos = viewSize;
    return RP_OK;
}

int osc_Release() {
    STOP_THREAD(mainThread);
    if (view != NULL) {
        free(view);
        view = NULL;
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

    auto channels = getADCChannels();

    if (channels >=4){
        ECHECK_APP(osc_setAmplitudeOffset(RPAPP_OSC_SOUR_CH3, 0));
        ECHECK_APP(osc_setAmplitudeOffset(RPAPP_OSC_SOUR_CH4, 0));
        ECHECK_APP(osc_setAmplitudeScale(RPAPP_OSC_SOUR_CH3, 1));
        ECHECK_APP(osc_setAmplitudeScale(RPAPP_OSC_SOUR_CH4, 1));
        ECHECK_APP(osc_setProbeAtt(RP_CH_3, 1));
        ECHECK_APP(osc_setProbeAtt(RP_CH_4, 1));
        ECHECK_APP(osc_setInputGain(RP_CH_3, RPAPP_OSC_IN_GAIN_LV))
        ECHECK_APP(osc_setInputGain(RP_CH_4, RPAPP_OSC_IN_GAIN_LV))
    }
    return RP_OK;
}

int osc_run() {
    clearView();
    EXECUTE_ATOMICALLY(mutex, oscRunning = true);
    ECHECK_APP(threadSafe_acqStart());

    if (trigSweep == RPAPP_OSC_TRIG_SINGLE) {
        ECHECK_APP(waitToFillPreTriggerBuffer(false));
        ECHECK_APP(osc_setTriggerSource(trigSource));
    } else {
        ECHECK_APP(osc_setTriggerSource(trigSource));
    }

    START_THREAD(mainThread, mainThreadFun);
    return RP_OK;
}

int osc_stop() {
    EXECUTE_ATOMICALLY(mutex, oscRunning = false);
    ECHECK_APP(threadSafe_acqStop());
    return RP_OK;
}

int osc_reset() {
    clearView();
    STOP_THREAD(mainThread);
    EXECUTE_ATOMICALLY(mutex, oscRunning = false);
    ECHECK_APP(threadSafe_acqStop());
    ECHECK_APP(osc_SetDefaultValues());
    return RP_OK;
}

int osc_single() {
    if (trigSweep != RPAPP_OSC_TRIG_SINGLE) {
        ECHECK_APP(osc_setTriggerSweep(RPAPP_OSC_TRIG_SINGLE));
    }
    ECHECK_APP(threadSafe_acqStart());
    ECHECK_APP(waitToFillPreTriggerBuffer(false));
    ECHECK_APP(osc_setTriggerSource(trigSource));
    return RP_OK;
}

int osc_autoScale() {
    if (trigSweep != RPAPP_OSC_TRIG_AUTO) {
        osc_setTriggerSweep(RPAPP_OSC_TRIG_AUTO);
    }

    checkAutoscale(false);

    return RP_OK;
}

int osc_isRunning(bool *running) {
    *running = oscRunning;

    if (oscRunning && (trigSweep == RPAPP_OSC_TRIG_SINGLE)) {
        *running = acqRunning;
    }

    return RP_OK;
}

int osc_isTriggered() {
    return _clock() < g_triggerTS;
}

int osc_setTimeScale(float scale) {
    double rate = getADCRate();
    float maxDeltaSample = rate * scale / 1000.0f / samplesPerDivision;
    float ratio = (float) ADC_BUFFER_SIZE / (float) viewSize;

    if (maxDeltaSample / 65536.0f > ratio) {
        return RP_EOOR;
    }

    rp_acq_decimation_t decimation;

    // contition: viewBuffer cannot be larger than adcBuffer
    if (maxDeltaSample <= ratio) {
        decimation = RP_DEC_1;
    }
    else if (maxDeltaSample / 8.0f <= ratio) {
        decimation = RP_DEC_8;
    }
    else if (maxDeltaSample / 64.0f <= ratio) {
        decimation = RP_DEC_64;
    }
    else if (maxDeltaSample / 1024.0f <= ratio) {
        decimation = RP_DEC_1024;
    }
    else if (maxDeltaSample / 8192.0f <= ratio) {
        decimation = RP_DEC_8192;
    }
    else {
        decimation = RP_DEC_65536;
    }

    std::lock_guard<std::recursive_mutex> lock(mutex);
    rpApp_osc_trig_sweep_t sweep;
    osc_getTriggerSweep(&sweep);
    if (scale < CONTIOUS_MODE_SCALE_THRESHOLD/* || sweep != RPAPP_OSC_TRIG_AUTO*/){
        ECHECK_APP(rp_AcqSetArmKeep(false))
        continuousMode = false;
    } else {
        ECHECK_APP(rp_AcqSetArmKeep(true))
        continuousMode = true;

        if (trigSweep == RPAPP_OSC_TRIG_NORMAL) {
            ECHECK_APP(threadSafe_acqStart())
        }
    }

    timeScale = scale;
    ECHECK_APP(rp_AcqSetDecimation(decimation))
    update_view();
    return RP_OK;
}

int osc_getTimeScale(float *division) {
    *division = timeScale;
    return RP_OK;
}

int osc_setTimeOffset(float offset) {
    float deltaSample = timeToIndex(timeScale) / samplesPerDivision;
    if (offset < ((int)viewSize/2-ADC_BUFFER_SIZE/2) * deltaSample || offset > indexToTime((int64_t) MAX_UINT)) {
        return RP_EOOR;
    }

    std::lock_guard<std::recursive_mutex> lock(mutex);
    timeOffset = offset;
    ECHECK_APP(rp_AcqSetTriggerDelayNs((int64_t)(offset * MILLI_TO_NANO)));
    update_view();
    return RP_OK;
}

int osc_getTimeOffset(float *offset) {
    *offset = timeOffset;
    return RP_OK;
}

int osc_setProbeAtt(rp_channel_t channel, float att) {

    CHECK_CHANNEL("osc_setProbeAtt")

    CHANNEL_ACTION_4CH(channel,
                   ch_probeAtt[0] = att,
                   ch_probeAtt[1] = att,
                   ch_probeAtt[2] = att,
                   ch_probeAtt[3] = att)

    EXECUTE_ATOMICALLY(mutex, update_view());
    return RP_OK;
}

int osc_getProbeAtt(rp_channel_t channel, float *att) {

    CHECK_CHANNEL("osc_getProbeAtt")

    CHANNEL_ACTION_4CH(channel,
                   *att = ch_probeAtt[0],
                   *att = ch_probeAtt[1],
                   *att = ch_probeAtt[2],
                   *att = ch_probeAtt[3])
    return RP_OK;
}

int osc_setInputGain(rp_channel_t channel, rpApp_osc_in_gain_t gain) {

    CHECK_CHANNEL("osc_setInputGain")

    std::lock_guard<std::recursive_mutex> lock(mutex);
    switch (gain) {
        case RPAPP_OSC_IN_GAIN_LV:
            ECHECK_APP(rp_AcqSetGain(channel, RP_LOW));
            break;
        case RPAPP_OSC_IN_GAIN_HV:
            ECHECK_APP(rp_AcqSetGain(channel, RP_HIGH));
            break;
        default:
            fprintf(stderr,"[Fatal Error] osc_setInputGain: unknown value %d\n",gain);
            return RP_EOOR;
    }
    update_view();
    return RP_OK;
}

int osc_getInputGain(rp_channel_t channel, rpApp_osc_in_gain_t *gain) {

    CHECK_CHANNEL("osc_getInputGain")

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
            fprintf(stderr,"[Fatal Error] osc_getInputGain: unknown value %d\n",*gain);
            return RP_EOOR;
    }
    return RP_OK;
}

int osc_setAmplitudeScale(rpApp_osc_source source, double scale) {

    double offset, currScale;

    {
        std::lock_guard<std::recursive_mutex> lock(mutex);
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
    EXECUTE_ATOMICALLY(mutex, update_view());
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

    std::lock_guard<std::recursive_mutex> lock(mutex);

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

int osc_setTriggerSource(rpApp_osc_trig_source_t triggerSource) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (trigSource != triggerSource) {
        clearView();
    }
    rp_acq_trig_src_t src;
    switch (triggerSource) {
        case RPAPP_OSC_TRIG_SRC_CH1:
            if (trigSlope == RPAPP_OSC_TRIG_SLOPE_NE) {
                src = RP_TRIG_SRC_CHA_NE;
            }
            else {
                src = RP_TRIG_SRC_CHA_PE;
            }
            break;
        case RPAPP_OSC_TRIG_SRC_CH2:
            if (trigSlope == RPAPP_OSC_TRIG_SLOPE_NE) {
                src = RP_TRIG_SRC_CHB_NE;
            }
            else {
                src = RP_TRIG_SRC_CHB_PE;
            }
            break;
        case RPAPP_OSC_TRIG_SRC_CH3:
            if (trigSlope == RPAPP_OSC_TRIG_SLOPE_NE) {
                src = RP_TRIG_SRC_CHC_NE;
            }
            else {
                src = RP_TRIG_SRC_CHC_PE;
            }
            break;
        case RPAPP_OSC_TRIG_SRC_CH4:
            if (trigSlope == RPAPP_OSC_TRIG_SLOPE_NE) {
                src = RP_TRIG_SRC_CHD_NE;
            }
            else {
                src = RP_TRIG_SRC_CHD_PE;
            }
            break;
        case RPAPP_OSC_TRIG_SRC_EXTERNAL:
            if (trigSlope == RPAPP_OSC_TRIG_SLOPE_NE) {
                src = RP_TRIG_SRC_EXT_NE;
            }
            else {
                src = RP_TRIG_SRC_EXT_PE;
            }
            break;
        default:
            fprintf(stderr,"[Fatal Error] osc_setTriggerSource: unknown value %d\n",triggerSource);
            return RP_EOOR;
    }

    trigSource = triggerSource;
    ECHECK_APP(rp_AcqSetTriggerSrc(src));
    usleep(300);
    return RP_OK;
}

int osc_getTriggerSource(rpApp_osc_trig_source_t *triggerSource) {
    *triggerSource = trigSource;
    return RP_OK;
}

int osc_setTriggerSlope(rpApp_osc_trig_slope_t slope) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    clearView();
    rp_acq_trig_src_t src;
    switch (trigSource) {
        case RPAPP_OSC_TRIG_SRC_CH1:
            if (slope == RPAPP_OSC_TRIG_SLOPE_NE) {
                src = RP_TRIG_SRC_CHA_NE;
            }
            else {
                src = RP_TRIG_SRC_CHA_PE;
            }
            break;
        case RPAPP_OSC_TRIG_SRC_CH2:
            if (slope == RPAPP_OSC_TRIG_SLOPE_NE) {
                src = RP_TRIG_SRC_CHB_NE;
            }
            else {
                src = RP_TRIG_SRC_CHB_PE;
            }
            break;
        case RPAPP_OSC_TRIG_SRC_CH3:
            if (slope == RPAPP_OSC_TRIG_SLOPE_NE) {
                src = RP_TRIG_SRC_CHC_NE;
            }
            else {
                src = RP_TRIG_SRC_CHC_PE;
            }
            break;
        case RPAPP_OSC_TRIG_SRC_CH4:
            if (slope == RPAPP_OSC_TRIG_SLOPE_NE) {
                src = RP_TRIG_SRC_CHD_NE;
            }
            else {
                src = RP_TRIG_SRC_CHD_PE;
            }
            break;
        case RPAPP_OSC_TRIG_SRC_EXTERNAL:
            if (slope == RPAPP_OSC_TRIG_SLOPE_NE) {
                src = RP_TRIG_SRC_EXT_NE;
            }
            else {
                src = RP_TRIG_SRC_EXT_PE;
            }
            break;
        default:
            fprintf(stderr,"[Fatal Error] osc_setTriggerSlope: unknown value %d\n",slope);
            return RP_EOOR;
    }

    trigSlope = slope;
    ECHECK_APP(rp_AcqSetTriggerSrc(src));
    return RP_OK;
}

int osc_getTriggerSlope(rpApp_osc_trig_slope_t *slope) {
    *slope = trigSlope;
    return RP_OK;
}

int osc_setTriggerLevel(float level) {

    std::lock_guard<std::recursive_mutex> lock(mutex);

    if (rp_HPGetIsExternalTriggerLevelPresentOrDefault()){
        if (trigSource == RPAPP_OSC_TRIG_SRC_EXTERNAL){
            ECHECK_APP(rp_AcqSetTriggerLevel(RP_T_CH_EXT, level));
        }
    }

    if(trigSource != RPAPP_OSC_TRIG_SRC_EXTERNAL) {
        rpApp_osc_source source = RPAPP_OSC_SOUR_CH1;
        if (trigSource == RPAPP_OSC_TRIG_SRC_CH2) source = RPAPP_OSC_SOUR_CH2;
        if (trigSource == RPAPP_OSC_TRIG_SRC_CH3) source = RPAPP_OSC_SOUR_CH3;
        if (trigSource == RPAPP_OSC_TRIG_SRC_CH4) source = RPAPP_OSC_SOUR_CH4;
        ECHECK_APP(unattenuateAmplitudeChannel(source, level, &level));
    }

    if (trigSource == RPAPP_OSC_TRIG_SRC_CH1){
        ECHECK_APP(rp_AcqSetTriggerLevel(RP_T_CH_1, level));
    }
    if (trigSource == RPAPP_OSC_TRIG_SRC_CH2){
        ECHECK_APP(rp_AcqSetTriggerLevel(RP_T_CH_2, level));
    }
    if (trigSource == RPAPP_OSC_TRIG_SRC_CH3){
        ECHECK_APP(rp_AcqSetTriggerLevel(RP_T_CH_3, level));
    }
    if (trigSource == RPAPP_OSC_TRIG_SRC_CH4){
        ECHECK_APP(rp_AcqSetTriggerLevel(RP_T_CH_4, level));
    }

    update_view();
    return RP_OK;
}

int osc_getTriggerLevel(float *level) {

    if (rp_HPGetIsExternalTriggerLevelPresentOrDefault()){
        if (trigSource == RPAPP_OSC_TRIG_SRC_EXTERNAL){
            ECHECK_APP(rp_AcqGetTriggerLevel(RP_T_CH_EXT,level));
        }
    }

    if (trigSource == RPAPP_OSC_TRIG_SRC_CH1){
        ECHECK_APP(rp_AcqGetTriggerLevel(RP_T_CH_1,level));
    }

    if (trigSource == RPAPP_OSC_TRIG_SRC_CH2){
        ECHECK_APP(rp_AcqGetTriggerLevel(RP_T_CH_2,level));
    }
    if (trigSource == RPAPP_OSC_TRIG_SRC_CH3){
        ECHECK_APP(rp_AcqGetTriggerLevel(RP_T_CH_3,level));
    }

    if (trigSource == RPAPP_OSC_TRIG_SRC_CH4){
        ECHECK_APP(rp_AcqGetTriggerLevel(RP_T_CH_4,level));
    }

    if(trigSource != RPAPP_OSC_TRIG_SRC_EXTERNAL) {
        rpApp_osc_source source = RPAPP_OSC_SOUR_CH1;
        if (trigSource == RPAPP_OSC_TRIG_SRC_CH2) source = RPAPP_OSC_SOUR_CH2;
        if (trigSource == RPAPP_OSC_TRIG_SRC_CH3) source = RPAPP_OSC_SOUR_CH3;
        if (trigSource == RPAPP_OSC_TRIG_SRC_CH4) source = RPAPP_OSC_SOUR_CH4;
        ECHECK_APP(attenuateAmplitudeChannel(source, *level, level));
    }
    return RP_OK;
}

int osc_setTriggerSweep(rpApp_osc_trig_sweep_t sweep) {
    EXECUTE_ATOMICALLY(mutex, clearView());
    switch (sweep) {
        case RPAPP_OSC_TRIG_SINGLE:
            break;
        case RPAPP_OSC_TRIG_AUTO:
        case RPAPP_OSC_TRIG_NORMAL:
            if (!acqRunning) {
                ECHECK_APP(threadSafe_acqStart());
            }
            break;
        default:
            fprintf(stderr,"[Fatal Error] osc_setTriggerSweep: unknown value %d\n",sweep);
            return RP_EOOR;
    }

    trigSweep = sweep;

    float scale;
    osc_getTimeScale(&scale);
    osc_setTimeScale(scale);
    return RP_OK;
}

int osc_getTriggerSweep(rpApp_osc_trig_sweep_t *sweep) {
    *sweep = trigSweep;
    return RP_OK;
}

int osc_setInverted(rpApp_osc_source source, bool inverted) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
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
    *ratio = ((float)viewSize * (float)timeToIndex(timeScale) / samplesPerDivision) / (float)ADC_BUFFER_SIZE;
    return RP_OK;
}

int osc_measureVpp(rpApp_osc_source source, float *Vpp) {
    float resMax, resMin, max = -FLT_MAX, min = FLT_MAX;

    mutex.lock();
    for (int i = viewStartPos; i < viewEndPos; ++i) {
        if (view[source*viewSize + i] > max) {
            max = view[source*viewSize + i];
        }
        if (view[source*viewSize + i] < min) {
            min = view[source*viewSize + i];
        }
    }
    mutex.unlock();

    ECHECK_APP(unscaleAmplitudeChannel(source, max, &resMax));
    ECHECK_APP(unscaleAmplitudeChannel(source, min, &resMin));
    *Vpp = resMax - resMin;
    ECHECK_APP(attenuateAmplitudeChannel(source, *Vpp, Vpp));
    *Vpp = fabs(*Vpp);
    return RP_OK;
}

int osc_measureMax(rpApp_osc_source source, float *Max) {

    std::lock_guard<std::recursive_mutex> lock(mutex);

    float resMax, max = -FLT_MAX;

    for (int i = viewStartPos; i < viewEndPos; ++i) {
        if (view[source*viewSize + i] > max) {
            max = view[source*viewSize + i];
        }
    }

    ECHECK_APP(unscaleAmplitudeChannel(source, max, &resMax));
    *Max = resMax;

    return RP_OK;
}

int osc_measureMin(rpApp_osc_source source, float *Min) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    float resMin, min = FLT_MAX;

    for (int i = viewStartPos; i < viewEndPos; ++i) {
        if (view[source*viewSize + i] < min) {
            min = view[source*viewSize + i];
        }
    }

    ECHECK_APP(unscaleAmplitudeChannel(source, min, &resMin));
    *Min = resMin;

    return RP_OK;
}

int osc_measureMeanVoltage(rpApp_osc_source source, float *meanVoltage) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    double sum = 0;

    for (int i = viewStartPos; i < viewEndPos; ++i) {
        sum += view[source*viewSize + i];
    }

    ECHECK_APP(unscaleAmplitudeChannel(source, sum / (viewEndPos - viewStartPos), meanVoltage));
    ECHECK_APP(attenuateAmplitudeChannel(source, *meanVoltage, meanVoltage));
    return RP_OK;
}

int osc_measureMaxVoltage(rpApp_osc_source source, float *Vmax) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    float max = view[source*viewSize];

    bool inverted = (source != RPAPP_OSC_SOUR_MATH && ch_inverted[(int)source]) || (source == RPAPP_OSC_SOUR_MATH && math_inverted);
    for (int i = viewStartPos; i < viewEndPos; ++i) {
        if (inverted ? view[source*viewSize + i] < max : view[source*viewSize + i] > max) {
            max = view[source*viewSize + i];
        }
    }
    *Vmax = max;

    ECHECK_APP(unscaleAmplitudeChannel(source, max, Vmax));
    ECHECK_APP(attenuateAmplitudeChannel(source, *Vmax, Vmax));
    return RP_OK;
}

int osc_measureMinVoltage(rpApp_osc_source source, float *Vmin) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    float min = view[source*viewSize];

    bool inverted = (source != RPAPP_OSC_SOUR_MATH && ch_inverted[(int)source]) || (source == RPAPP_OSC_SOUR_MATH && math_inverted);
    for (int i = viewStartPos; i < viewEndPos; ++i) {
        if (inverted ? view[source*viewSize + i] > min : view[source*viewSize + i] < min) {
            min = view[source*viewSize + i];
        }
    }
    *Vmin = min;

    ECHECK_APP(unscaleAmplitudeChannel(source, min, Vmin));
    ECHECK_APP(attenuateAmplitudeChannel(source, *Vmin, Vmin));
    return RP_OK;
}

int osc_measureFrequency(rpApp_osc_source source, float *frequency) {
    float period;
    ECHECK_APP(osc_measurePeriod(source, &period));
    period = (period == 0.f) ?  0.000001f : period;
    *frequency = (float) (1 / (period / 1000.0));
    return RP_OK;
}

int osc_measurePeriodMath(rpApp_osc_source source, float *period) {

    int size = viewEndPos - viewStartPos;
    float data[VIEW_SIZE_DEFAULT];
    float* ch_view = view + source*viewSize + viewStartPos;

    mutex.lock();
    float mean = 0;
    for (int i = 0; i < size; ++i) {
        data[i] = ch_view[i];
        mean += data[i];
    }
    mutex.unlock();

    mean = mean / size;
    for (int i = 0; i < size; ++i){
        data[i] -= mean;
    }

    // calculate signal correlation
    float xcorr[VIEW_SIZE_DEFAULT];
    for (int i = 0; i < size; ++i) {
        xcorr[i] = 0;
        for (int j = 0; j < size-i; ++j) {
            xcorr[i] += data[j] * data[j+i];
        }
        xcorr[i] /= size-i;
    }

    // The main problem is the presence lot of noise in the signal
    // We can filter correlation function and differentiate it to find local maximum, but it could fail on high frequencies I suppose
    // So lets try to find local maximum logically, idea is:
    // signal: ZxxbbbbbaaAaaBbbbbxxYxbbbbBaaAaaa
    // 'a' - values below acceptable threshold
    // 'b', 'x', 'y' - values above acceptable threshold
    // 'Y' - local maximum value
    // 'Z' - reference value
    // 'x' - almost y
    // need find left 'A', then we can find left 'B'
    // then can need find right 'A', then can find right 'B'
    // then we can find 'Y' between left and right 'B'
    // then we can find left and right 'x'
    // guess extreme point locates in the middle of left and right 'x'
    // we can not use 'Y' only because it could be (x + noise)

    int left_idx = 0;
    int right_idx = 0;
    int left_edge_idx = 0;
    int right_edge_idx = size-2;

    // search for left point where correlation function is less than it's expected
    for (int i = 1; i < size-1; ++i) {
        if((xcorr[i] / xcorr[0]) < PERIOD_EXISTS_MIN_THRESHOLD) {
            left_edge_idx = i;
            break;
        }
    }

    if(left_edge_idx == 0) {
        return RP_APP_ECP;
    }

    // search for left point where correlation function is greater than it's expected
    for (int i = left_edge_idx; i < size-1; ++i) {
        if((xcorr[i] / xcorr[0]) >= PERIOD_EXISTS_MAX_THRESHOLD) {
            left_idx = i;
            break;
        }
    }

    if(left_idx == 0) {
        return RP_APP_ECP;
    }

    // search for right point where correlation function is less than it's expected
    for (int i = left_idx; i < size-1; ++i) {
        if((xcorr[i] / xcorr[0]) < PERIOD_EXISTS_MIN_THRESHOLD) {
            right_edge_idx = i;
            break;
        }
    }

    // search for right point where correlation function is greater than it's expected
    for (int i = right_edge_idx; i >= left_idx; --i) {
        if((xcorr[i] / xcorr[0]) >= PERIOD_EXISTS_MAX_THRESHOLD) {
            right_idx = i;
            break;
        }
    }

    // search for local maximum
    float loc_max = xcorr[left_idx];
    int max_idx = left_idx;
    for (int i = left_idx; i <= right_idx; ++i) {
        if(loc_max < xcorr[i]) {
            loc_max = xcorr[i];
            max_idx = i;
        }
    }

    // search for left point which is almost equal to maximum
    int left_amax_idx = max_idx;
    int right_amax_idx = max_idx;
    for (int i = left_idx; i <= right_idx; ++i) {
        if(xcorr[i] >= loc_max * PERIOD_EXISTS_PEAK_THRESHOLD) {
            left_amax_idx = i;
            break;
        }
    }

    // search for right point which is almost equal to maximum
    for (int i = right_edge_idx; i >= left_idx; --i) {
        if(xcorr[i] >= loc_max * PERIOD_EXISTS_PEAK_THRESHOLD) {
            right_amax_idx = i;
            break;
        }
    }

    // guess extreme point locates between 'left_amax_idx' and 'right_amax_idx'
    float timeScale, viewScale;
    ECHECK_APP(osc_getTimeScale(&timeScale));
    viewScale = timeToIndex(timeScale) / samplesPerDivision;

    float idx = ((left_amax_idx + right_amax_idx) / 2.f) * viewScale;
    *period = indexToTime(idx);

    return RP_OK;
}

int32_t osc_adc_sign(uint32_t cnts, uint8_t bits){
    int32_t m;
    cnts &= ((1 << bits) - 1);
    /* check sign */
    if(cnts & (1 << (bits - 1))) {
        /* negative number */
        m = -1 *((cnts ^ ((1 << bits) - 1)) + 1);
    } else {
        /* positive number */
        m = cnts;
    }

    return m;
}


int osc_measurePeriodCh(rpApp_osc_source source, float *period) {
    double sample_per;
    if (getADCSamplePeriod(&sample_per) != RP_OK){
        return RP_EOOR;
    }
    const float c_osc_fpga_smpl_freq = getADCRate();
    const float c_meas_freq_thr = 100;
    uint32_t start, end;
    osc_getViewLimits(&start, &end);

    int size = ADC_BUFFER_SIZE;
    const int c_meas_time_thr = ADC_BUFFER_SIZE / sample_per;
    const float c_min_period = 19.6e-9; // 51 MHz

    float thr1, thr2, cen;
    int state = 0;
    int trig_t[2] = { 0, 0 };
    int trig_cnt = 0;
    int ix;

    uint16_t data[ADC_BUFFER_SIZE];
    int32_t data_i[ADC_BUFFER_SIZE];
    rp_channel_t ch = RP_CH_1;
    if (source == RPAPP_OSC_SOUR_CH2) ch = RP_CH_2;
    if (source == RPAPP_OSC_SOUR_CH3) ch = RP_CH_3;
    if (source == RPAPP_OSC_SOUR_CH4) ch = RP_CH_4;
    uint8_t bits;
    rp_HPGetFastADCBits(&bits);

    osc_getRawData(ch, data, size);
    for(int i = 0; i< size; i++){
        // Shift by 16 bit for detect lower signal
        data_i[i] = osc_adc_sign(data[i],bits) << 16;
    }


    float meas_max, meas_min;

    meas_max = data_i[0] ;
    meas_min = data_i[0] ;
    for(int i = 0; i < size; i++)
    {
        meas_max = (meas_max > data_i[i] )? meas_max : data_i[i] ;
        meas_min = (meas_min < data_i[i] )? meas_min : data_i[i] ;
    }

	uint32_t dec_factor = 1;
    ECHECK_APP(rp_AcqGetDecimationFactor(&dec_factor));

    float acq_dur=(float)(size)/((float) c_osc_fpga_smpl_freq) * (float) dec_factor;
    cen = (meas_max + meas_min) / 2;
    thr1 = cen + 0.2 * (meas_min - cen);
    thr2 = cen + 0.2 * (meas_max - cen);
    float res_period = 0;
    for(ix = 0; ix < (size); ix++) {
        auto sa = data_i[ix];

        /* Lower transitions */
        if((state == 0) && (sa < thr1)) {
            state = 1;
        }
        /* Upper transitions - count them & store edge times. */
        if((state == 1) && (sa >= thr2) ) {
            state = 0;
            if (trig_cnt++ == 0) {
                trig_t[0] = ix;
            } else {
                trig_t[1] = ix;
            }
        }
        if ((trig_t[1] - trig_t[0]) > c_meas_time_thr) {
            break;
        }
    }
    /* Period calculation - taking into account at least meas_time_thr samples */
    if(trig_cnt >= 2) {
       res_period = (float)(trig_t[1] - trig_t[0]) /
            ((float)c_osc_fpga_smpl_freq * (trig_cnt - 1)) * dec_factor;
    }

    if( ((thr2 - thr1) < c_meas_freq_thr) ||
         (res_period * 3 >= acq_dur)    ||
         (res_period < c_min_period) )
    {
        res_period = 0;
    }
    *period = res_period * 1000.f;
    return RP_OK;
}

int osc_measurePeriod(rpApp_osc_source source, float *period) {
	if(source == RPAPP_OSC_SOUR_MATH)
		return osc_measurePeriodMath(source, period);
	else
		return osc_measurePeriodCh(source, period);
}

int osc_measureDutyCycle(rpApp_osc_source source, float *dutyCycle) {
    int highTime = 0;
    float meanValue;
    ECHECK_APP(osc_measureMeanVoltage(source, &meanValue));
    ECHECK_APP(scaleAmplitudeChannel(source, meanValue, &meanValue))

    std::lock_guard<std::recursive_mutex> lock(mutex);
    for (int i = viewStartPos; i < viewEndPos; ++i) {
        if (view[source*viewSize + i] > meanValue) {
            ++highTime;
        }
    }

    *dutyCycle = (float)highTime / (float)(viewEndPos - viewStartPos);
    return RP_OK;
}

int osc_measureRootMeanSquare(rpApp_osc_source source, float *rms) {
    double rmsValue = 0;

    std::lock_guard<std::recursive_mutex> lock(mutex);
    for (int i = viewStartPos; i < viewEndPos; ++i) {
        float tmp;
        unscaleAmplitudeChannel(source, view[source*viewSize + i], &tmp);
        rmsValue += tmp*tmp;
    }

    *rms = (double) sqrt(rmsValue / (double)(viewEndPos - viewStartPos));
    ECHECK_APP(attenuateAmplitudeChannel(source, *rms, rms));
    return RP_OK;
}

int osc_getCursorVoltage(rpApp_osc_source source, uint32_t cursor, float *value) {
    return unscaleAmplitudeChannel(source, view[source*viewSize + cursor], value);
}

int osc_getCursorTime(uint32_t cursor, float *value) {
    if (cursor < 0 || cursor >= viewSize) {
        return RP_EOOR;
    }
    *value = viewIndexToTime(cursor);
    return RP_OK;
}

int osc_getCursorDeltaTime(uint32_t cursor1, uint32_t cursor2, float *value) {
    if (cursor1 < 0 || cursor1 >= viewSize || cursor2 < 0 || cursor2 >= viewSize) {
        return RP_EOOR;
    }
    *value = indexToTime(abs((long long)cursor1 - (long long)cursor2));
    return RP_OK;
}

int oscGetCursorDeltaAmplitude(rpApp_osc_source source, uint32_t cursor1, uint32_t cursor2, float *value) {
    if (cursor1 < 0 || cursor1 >= viewSize || cursor2 < 0 || cursor2 >= viewSize) {
        return RP_EOOR;
    }
    float cursor1Amplitude, cursor2Amplitude;
    ECHECK_APP(osc_getCursorVoltage(source, cursor1, &cursor1Amplitude));
    ECHECK_APP(osc_getCursorVoltage(source, cursor2, &cursor2Amplitude));
    *value = (float) fabs(cursor2Amplitude - cursor1Amplitude);
    return RP_OK;
}

int osc_getCursorDeltaFrequency(uint32_t cursor1, uint32_t cursor2, float *value) {
    if (cursor1 < 0 || cursor1 >= viewSize || cursor2 < 0 || cursor2 >= viewSize) {
        return RP_EOOR;
    }
    float deltaTime;
    ECHECK_APP(osc_getCursorDeltaTime(cursor1, cursor2, &deltaTime));
    *value = 1 / deltaTime;
    return RP_OK;
}

int osc_setMathOperation(rpApp_osc_math_oper_t op) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    operation = op;
    clearMath();
    update_view();
    return RP_OK;
}

int osc_getMathOperation(rpApp_osc_math_oper_t *op) {
    *op = operation;
    return RP_OK;
}

int osc_setMathSources(rp_channel_t source1, rp_channel_t source2) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    mathSource1 = source1;
    mathSource2 = source2;
    clearMath();
    update_view();
    return RP_OK;
}

int osc_getMathSources(rp_channel_t *source1, rp_channel_t *source2) {
    *source1 = mathSource1;
    *source2 = mathSource2;
    return RP_OK;
}

int osc_getData(rpApp_osc_source source, float *data, uint32_t size) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    memcpy_neon(data,&view[source * viewSize],size * sizeof(float));
    return RP_OK;
}

int osc_getRawData(rp_channel_t source, uint16_t *data, uint32_t size) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (raw_data[source] != nullptr){
        memcpy_neon(data,raw_data[source],size * sizeof(uint16_t));
    }
    return RP_OK;
}

int osc_setViewSize(uint32_t size) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    viewSize = size;
    samplesPerDivision = (float) viewSize / (float) DIVISIONS_COUNT_X;

    view = (float*)realloc(view, (RPAPP_OSC_SOUR_MATH + 1) * viewSize * sizeof(float));
    if (view == NULL) {
        return RP_EAA;
    }
    update_view();
    return RP_OK;
}

int osc_getViewSize(uint32_t *size) {
    *size = viewSize;
    return RP_OK;
}

int osc_getViewLimits(uint32_t* start, uint32_t* end) {
    if(autoScale) {
        *start = 0;
        *end = 0;
    } else {
        *start = viewStartPos;
        *end = viewEndPos;
    }
    return RP_OK;
}

/*
* Utils
*/

int threadSafe_acqStart() {
    if(!oscRunning)
        return RP_OK;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    ECHECK_APP(rp_AcqStart())
    ECHECK_APP(rp_AcqSetArmKeep(trigSweep != RPAPP_OSC_TRIG_SINGLE && continuousMode));
    acqRunning = true;
    return RP_OK;
}

int threadSafe_acqStop() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    ECHECK_APP(rp_AcqStop())
    ECHECK_APP(rp_AcqSetArmKeep(false))
    acqRunning = false;
    return RP_OK;
}

double scaleAmplitude(double volts, double ampScale, double probeAtt, double ampOffset, double invertFactor) {
    return (volts * invertFactor * probeAtt + ampOffset) / ampScale;
}

double unscaleAmplitude(double value, double ampScale, double probeAtt, double ampOffset, double invertFactor) {
    return ((value * ampScale) - ampOffset) / probeAtt / invertFactor;
}

double unOffsetAmplitude(double value, double ampScale, double ampOffset) {
    return value - (ampOffset / ampScale);
}

int scaleAmplitudeChannel(rpApp_osc_source source, float volts, float *res) {
    double ampOffset, ampScale;
    float probeAtt = 1;
    bool inverted;
    ECHECK_APP(osc_getAmplitudeOffset(source, &ampOffset));
    ECHECK_APP(osc_getAmplitudeScale(source, &ampScale));
    if (source != RPAPP_OSC_SOUR_MATH)
        ECHECK_APP(osc_getProbeAtt((rp_channel_t)source, &probeAtt));
    ECHECK_APP(osc_isInverted(source, &inverted));
    *res = scaleAmplitude(volts, ampScale, probeAtt, ampOffset, inverted ? -1 : 1);
    return RP_OK;
}

int unscaleAmplitudeChannel(rpApp_osc_source source, float value, float *res) {
    double ampOffset, ampScale;
    float probeAtt=1;
    bool inverted;
    ECHECK_APP(osc_getAmplitudeOffset(source, &ampOffset));
    ECHECK_APP(osc_getAmplitudeScale(source, &ampScale));
    ECHECK_APP(osc_isInverted(source, &inverted));
    if (source != RPAPP_OSC_SOUR_MATH)
        ECHECK_APP(osc_getProbeAtt((rp_channel_t)source, &probeAtt));
    *res = unscaleAmplitude(value, ampScale, probeAtt, ampOffset, inverted ? -1 : 1);
    return RP_OK;
}

int unOffsetAmplitudeChannel(rpApp_osc_source source, float value, float *res) {
    double ampOffset, ampScale;
    ECHECK_APP(osc_getAmplitudeOffset(source, &ampOffset));
    ECHECK_APP(osc_getAmplitudeScale(source, &ampScale));
    *res = unOffsetAmplitude(value, ampScale, ampOffset);
    return RP_OK;
}

int attenuateAmplitudeChannel(rpApp_osc_source source, float value, float *res) {
    float probeAtt = 1.f;
    if (source != RPAPP_OSC_SOUR_MATH)
        ECHECK_APP(osc_getProbeAtt((rp_channel_t)source, &probeAtt));

    *res = scaleAmplitude(value, 1.f, probeAtt, 0.f, 1.f);
    return RP_OK;
}

int unattenuateAmplitudeChannel(rpApp_osc_source source, float value, float *res) {
    float probeAtt = 1.f;
    if (source != RPAPP_OSC_SOUR_MATH)
        ECHECK_APP(osc_getProbeAtt((rp_channel_t)source, &probeAtt));

    *res = unscaleAmplitude(value, 1.f, probeAtt, 0.f, 1.f);
    return RP_OK;
}

float viewIndexToTime(int index) {
    return indexToTime(index - viewSize / 2) + timeOffset;
}

void calculateIntegral(rp_channel_t channel, float scale, float offset, float invertFactor) {
    float dt = timeScale / samplesPerDivision;
    float v;

    bool invert = ch_inverted[(int)channel];
    float ch_sign = invert ? -1.f : 1.f;

    ECHECK_APP_NO_RET(unscaleAmplitudeChannel((rpApp_osc_source) channel, view[channel*viewSize + viewStartPos], &v));
	ECHECK_APP_NO_RET(attenuateAmplitudeChannel((rpApp_osc_source) channel, v, &v));
    view[RPAPP_OSC_SOUR_MATH*viewSize] = ch_sign * v * dt;
    for (int i = viewStartPos + 1; i < viewEndPos; ++i) {
        ECHECK_APP_NO_RET(unscaleAmplitudeChannel((rpApp_osc_source) channel, view[channel*viewSize + i], &v));
		ECHECK_APP_NO_RET(attenuateAmplitudeChannel((rpApp_osc_source) channel, v, &v));
        view[RPAPP_OSC_SOUR_MATH*viewSize + i] = view[RPAPP_OSC_SOUR_MATH*viewSize + i-1] + (ch_sign * v * dt);
        view[RPAPP_OSC_SOUR_MATH*viewSize + i-1] = scaleAmplitude(view[RPAPP_OSC_SOUR_MATH*viewSize + i-1], scale, 1, offset, invertFactor);
    }
    view[RPAPP_OSC_SOUR_MATH*viewSize + viewEndPos - 1] = scaleAmplitude(view[RPAPP_OSC_SOUR_MATH*viewSize + viewEndPos - 1], scale, 1, offset, invertFactor);
}

void calculateDevivative(rp_channel_t channel, float scale, float offset, float invertFactor) {
    float dt2 = 2*timeScale / 1000 / samplesPerDivision;
    float v1, v2;

    bool invert = ch_inverted[(int)channel];
    float ch_sign = invert ? -1.f : 1.f;

    ECHECK_APP_NO_RET(unscaleAmplitudeChannel((rpApp_osc_source) channel, view[channel*viewSize + viewStartPos], &v2));
	ECHECK_APP_NO_RET(attenuateAmplitudeChannel((rpApp_osc_source) channel, v2, &v2));
    for (int i = viewStartPos; i < viewEndPos - 1; ++i) {
        v1 = v2;
        ECHECK_APP_NO_RET(unscaleAmplitudeChannel((rpApp_osc_source) channel, view[channel*viewSize + i + 1], &v2));
		ECHECK_APP_NO_RET(attenuateAmplitudeChannel((rpApp_osc_source) channel, v2, &v2));
        view[RPAPP_OSC_SOUR_MATH*viewSize + i] = scaleAmplitude(ch_sign * (v2 - v1) / dt2, scale, 1, offset, invertFactor);
    }
    view[RPAPP_OSC_SOUR_MATH*viewSize + viewEndPos - 1] = view[RPAPP_OSC_SOUR_MATH*viewSize + viewEndPos - 2];
}

float calculateMath(float v1, float v2, rpApp_osc_math_oper_t op) {
    float ret = 0;
    switch (op) {
        case RPAPP_OSC_MATH_ADD:
            ret = v1 + v2;
            break;
        case RPAPP_OSC_MATH_SUB:
            ret = v1 - v2;
            break;
        case RPAPP_OSC_MATH_MUL:
            ret = v1 * v2;
            break;
        case RPAPP_OSC_MATH_DIV:
            if (v2 != 0)
                ret = v1 / v2;
            else
                ret = v1 > 0 ? FLT_MAX * 0.9f : -FLT_MAX * 0.9f;
            break;
        case RPAPP_OSC_MATH_ABS:
            ret = (float) fabs(v1);
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

void clearView() {
    clear = true;
    viewStartPos = 0;
    viewEndPos = viewSize;
}

void clearMath() {
    for (int i = 0; i < viewSize; ++i) {
        view[RPAPP_OSC_SOUR_MATH*viewSize + i] = 0;
    }
}

int waitToFillPreTriggerBuffer(bool testcancel) {
    if (continuousMode && trigSweep != RPAPP_OSC_TRIG_SINGLE) {
        return RP_OK;
    }

    double localTimer = testcancel ? threadTimer : _clock() + WAIT_TO_FILL_BUF_TIMEOUT;
    float deltaSample, timeScale;
    uint32_t preTriggerCount;
    int triggerDelay;

    do {
        ECHECK_APP(rp_AcqGetTriggerDelay(&triggerDelay));
        ECHECK_APP(rp_AcqGetPreTriggerCounter(&preTriggerCount));
        ECHECK_APP(osc_getTimeScale(&timeScale));
        deltaSample = timeToIndex(timeScale) / samplesPerDivision;

        if(testcancel)
            pthread_testcancel();
    } while (
        preTriggerCount < viewSize/2*deltaSample - triggerDelay &&
        localTimer > _clock());
    return RP_OK;
}

int waitToFillAfterTriggerBuffer(bool testcancel,uint32_t _triggerPosition) {
    float _timeScale = 0;
    ECHECK_APP(osc_getTimeScale(&_timeScale));
    double t = calculateTimeOut(_timeScale);
    double localTimer = _clock() + t;
    //double localTimer = testcancel ? threadTimer : _clock() + WAIT_TO_FILL_BUF_TIMEOUT;
    float deltaSample, timeScale;
    uint32_t _writePointer;
    int triggerDelay;

    ECHECK_APP(rp_AcqGetTriggerDelay(&triggerDelay));
    ECHECK_APP(osc_getTimeScale(&timeScale));
    deltaSample = timeToIndex(timeScale) / samplesPerDivision;

    do {
        ECHECK_APP(rp_AcqGetWritePointer(&_writePointer));

        if(testcancel)
            pthread_testcancel();
        //assert(((viewSize/2.f) * deltaSample) + triggerDelay + 1 <= ADC_BUFFER_SIZE);
    } while (
        (((_writePointer - _triggerPosition) % ADC_BUFFER_SIZE) <= ((viewSize/2.f) * deltaSample) + triggerDelay + 1) &&
        localTimer > _clock());
    return RP_OK;
}

/*
* Thread functions
*/

int osc_scaleMath() {
    if (!autoScale) {
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

            for (int i = viewStartPos; i < viewEndPos; ++i) {
                ECHECK_APP_NO_RET(unscaleAmplitudeChannel((rpApp_osc_source) mathSource1, view[mathSource1*viewSize + i], &v1));
                ECHECK_APP_NO_RET(unscaleAmplitudeChannel((rpApp_osc_source) mathSource2, view[mathSource2*viewSize + i], &v2));

                ECHECK_APP_NO_RET(attenuateAmplitudeChannel((rpApp_osc_source) mathSource1, v1, &v1));
                ECHECK_APP_NO_RET(attenuateAmplitudeChannel((rpApp_osc_source) mathSource2, v2, &v2));
                auto v = scaleAmplitude(calculateMath(sign1 * v1, sign2 * v2, operation), math_ampScale, 1, math_ampOffset, invertFactor);
                view[RPAPP_OSC_SOUR_MATH*viewSize + i] = v;
            }
        }
    }
}

void checkAutoscale(bool fromThread) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if((autoScale == false) && (fromThread == true))
    {
        return;
    }

    static const float scales[AUTO_SCALE_NUM_OF_SCALE] = {0.00005f, 0.0001f, 0.0002f, 0.0005f, 0.001f, 0.002f, 0.005f, 0.01f, 0.02f, 0.05f, 0.1f, 0.2f, 0.5f, 1.f, 2.f, 5.f, 10.f, 20.f, 50.f, 100.f};
    static int timeScaleIdx = 0;
    static float periods[MAX_ADC_CHANNELS][AUTO_SCALE_NUM_OF_SCALE];
    static float vpps[MAX_ADC_CHANNELS][AUTO_SCALE_NUM_OF_SCALE];
    static float vMeans[MAX_ADC_CHANNELS][AUTO_SCALE_NUM_OF_SCALE];
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

        autoScale = true;
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
        for (auto source = 0; source < channels; ++source) {
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
            // fprintf(stderr,"Source %d ts %d per %f\n",source,timeScaleIdx,periods[source][timeScaleIdx]);

        }


        if(++timeScaleIdx >= AUTO_SCALE_NUM_OF_SCALE) {
            autoScale = false;

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
            } else if(repCounts[0] > 0) {
                period_to_set = periods[0][periodsIdx[0]] * AUTO_SCALE_PERIOD_COUNT / DIVISIONS_COUNT_X;
            } else if(repCounts[1] > 0) {
                period_to_set = periods[1][periodsIdx[1]] * AUTO_SCALE_PERIOD_COUNT / DIVISIONS_COUNT_X;
            } else if(repCounts[2] > 0) {
                period_to_set = periods[2][periodsIdx[2]] * AUTO_SCALE_PERIOD_COUNT / DIVISIONS_COUNT_X;
            } else if(repCounts[3] > 0) {
                period_to_set = periods[3][periodsIdx[3]] * AUTO_SCALE_PERIOD_COUNT / DIVISIONS_COUNT_X;
            } else {
                period_to_set = savedTimeScale;
            }

            period_to_set = MAX(0.0001f, period_to_set);
            period_to_set = MIN(500.f, period_to_set);



            for (auto source = 0; source < channels; ++source) {
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

                ECHECK_APP_NO_RET(scaleChannel((rp_channel_t)source, vpp, vMean));
            }
        } else {
            period_to_set = scales[timeScaleIdx];
        }
    }
    ECHECK_APP_NO_RET(osc_setTimeScale(period_to_set));
    ECHECK_APP_NO_RET(osc_setTimeOffset(AUTO_SCALE_TIME_OFFSET));
}

static inline void threadUpdateView(volatile uint16_t *data[MAX_ADC_CHANNELS],
                                    uint32_t _getBufSize,
                                    double _deltaSample,
                                    float _timeScale,
                                    float _lastTimeScale,
                                    float _lastTimeOffset,
                                    rp_pinState_t* raw_state_ch) {

    {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        updateView = false;

        if(_getBufSize == 0) {
            clearView();
            return;
        }
        double curDeltaSample = _deltaSample * (_timeScale / _lastTimeScale);
        int requiredBuffSize = viewSize * curDeltaSample;
        int bufferEars = ((int)ADC_BUFFER_SIZE - requiredBuffSize) / 2;
        int viewEars = -MIN(bufferEars / curDeltaSample, 0);
        bufferEars = MAX(0, bufferEars);

        float tOff = _lastTimeOffset - timeOffset;
        float tMax = ((float)ADC_BUFFER_SIZE / (samplesPerDivision * curDeltaSample)) * _timeScale;

        if (tOff > tMax) tOff = 0.f;//tMax;
        else if (tOff < -tMax) tOff = 0.f; //-tMax;

        int viewOffset = (tOff * (float)samplesPerDivision) / _timeScale;
        int buffOffset = viewOffset * curDeltaSample;

        if(viewEars) {
            buffOffset = 0;
        } else {
            if((bufferEars + (ADC_BUFFER_SIZE / 2)) < abs(buffOffset) && ((abs(buffOffset) - bufferEars) * sign(buffOffset) / curDeltaSample) < 1024*3) {
                viewOffset = (abs(buffOffset) - bufferEars) * sign(buffOffset) / curDeltaSample;
                buffOffset = bufferEars * sign(buffOffset);
            } else {
                viewOffset = 0;
            }
        }

        int maxViewIdx = MIN(viewSize, (viewSize - 2 * viewEars));
        int buffFullOffset = bufferEars - buffOffset - (ADC_BUFFER_SIZE - _getBufSize)/2;

        float gainV[MAX_ADC_CHANNELS];
        rp_pinState_t gain[MAX_ADC_CHANNELS];
        rp_acq_ac_dc_mode_t power_mode[MAX_ADC_CHANNELS];
        uint8_t bits[MAX_ADC_CHANNELS];
        uint_gain_calib_t calib[MAX_ADC_CHANNELS];

        auto adc_channels = getADCChannels();
        for(auto ch = 0 ;ch < adc_channels; ++ch){

            if (rp_AcqGetGainV((rp_channel_t)ch, &gainV[ch]) != RP_OK){
                fprintf(stderr,"[Error:threadUpdateView] rp_AcqGetGainV Unknown error: %d\n",gain[ch]);
                return;
            }

            if (rp_AcqGetGain((rp_channel_t)ch, &gain[ch]) != RP_OK){
                fprintf(stderr,"[Error:threadUpdateView] rp_AcqGetGain Unknown error: %d\n",gain[ch]);
                return;
            }

            if (rp_HPGetFastADCIsAC_DCOrDefault() && rp_AcqGetAC_DC((rp_channel_t)ch, &power_mode[ch]) != RP_OK){
                fprintf(stderr,"[Error:threadUpdateView] rp_AcqGetAC_DC Unknown error: %d\n",gain[ch]);
                return;
            }else{
                power_mode[ch] = RP_DC;
            }

            int ret = rp_HPGetFastADCBits(&bits[ch]);
            switch (gain[ch])
            {
                case RP_LOW:
                    ret |= rp_CalibGetFastADCCalibValueI(convertCh(convertChFromIndex(ch)),convertPower(power_mode[ch]),&calib[ch]);
                    break;

                case RP_HIGH:
                    ret |= rp_CalibGetFastADCCalibValue_1_20I(convertCh(convertChFromIndex(ch)),convertPower(power_mode[ch]),&calib[ch]);
                    break;

                default:
                    fprintf(stderr,"[Error:acq_GetDataVEx] Unknown mode: %d\n",gain[ch]);
                    return;
                    break;
            }

            if (ret != RP_HW_CALIB_OK){
                fprintf(stderr,"[Error:acq_GetDataVEx] Error get calibaration: %d\n",ret);
                return;
            }
        }

        for (auto channel = 0; channel < adc_channels; ++channel) {
            int viewFullOffset = (channel * viewSize) + viewEars + viewOffset;
            for(int i = 0; i < viewEars + viewOffset && (int)channel * viewSize + i < (RPAPP_OSC_SOUR_MATH + 1) * viewSize; ++i) {
                view[(int)channel * viewSize + i] = 0.f;
            }

            for(int i = 0; i < viewEars - viewOffset && (int)channel * viewSize + viewSize - i - 1 < (RPAPP_OSC_SOUR_MATH + 1) * viewSize; ++i) {
                view[(int)channel * viewSize + viewSize - i - 1] = 0.f;
            }

            if(curDeltaSample < 1.0f) {
                int i;
                for (i = 0; i < maxViewIdx && viewFullOffset + i < (RPAPP_OSC_SOUR_MATH + 1) * viewSize ; ++i) {
                    int x0 = (((double)i * curDeltaSample) + buffFullOffset);
                    int x00 = ((size_t)x0) % ADC_BUFFER_SIZE;
                    int x1 = ((size_t)(x0 + 1)) % ADC_BUFFER_SIZE;
                    float y = linear(x0,
                                    convertToVoltSigned(data[channel][x00],bits[channel], gainV[channel],calib[channel].gain,calib[channel].base,calib[channel].offset),
                                    x0 + 1,
                                    convertToVoltSigned(data[channel][x1],bits[channel], gainV[channel],calib[channel].gain,calib[channel].base,calib[channel].offset),
                                    ((double)i * curDeltaSample) + buffFullOffset);

                    ECHECK_APP_NO_RET(scaleAmplitudeChannel((rpApp_osc_source) channel, y, view + viewFullOffset + i));
                }
                maxViewIdx = i;
            } else {
                int i;
                for (i = 0; i < maxViewIdx && viewFullOffset + i < (RPAPP_OSC_SOUR_MATH + 1) * viewSize ; ++i) {
                    float s = ((double)i)*curDeltaSample + buffFullOffset;
                    size_t idx = ((size_t)(int)s) % ADC_BUFFER_SIZE; // avoid UB
                    auto value = convertToVoltSigned(data[channel][idx] ,bits[channel] ,gainV[channel],calib[channel].gain,calib[channel].base,calib[channel].offset);
                    ECHECK_APP_NO_RET(scaleAmplitudeChannel((rpApp_osc_source) channel, value , view + viewFullOffset + i));
                }
                maxViewIdx = i;
            }
        }
        viewStartPos = viewEars + viewOffset;
        viewEndPos = viewStartPos + maxViewIdx;
    }
    mathThreadFunction();
}

int RestartAcq(float _timeScale) {
	ECHECK_APP(rp_AcqSetTriggerSrc(RP_TRIG_SRC_DISABLED));
	ECHECK_APP(threadSafe_acqStart());
	if (trigSweep == RPAPP_OSC_TRIG_AUTO) {
		threadTimer = calculateTimeOut(_timeScale);
	} else {
		float sampling_rate;
		ECHECK_APP_NO_RET(rp_AcqGetSamplingRateHz(&sampling_rate));

		// Buffer fill time
		threadTimer = ((double)ADC_BUFFER_SIZE) / sampling_rate * 1000.;
	}

	threadTimer += _clock();

	waitToFillPreTriggerBuffer(true);

	ECHECK_APP_NO_RET(osc_setTriggerSource(trigSource));
	EXECUTE_ATOMICALLY(mutex, clear = false)
	EXECUTE_ATOMICALLY(mutex, updateView = false)

	return RP_OK;
}

double timeToIndex1(double time) {
    float samplingRate;
    ECHECK_APP(rp_AcqGetSamplingRateHz(&samplingRate));
    return (double)samplingRate * time / 1000.0;
}

void *mainThreadFun(void *arg) {
    rp_acq_trig_src_t _triggerSource;
    rp_acq_trig_state_t _state;
    rp_acq_decimation_t decimation;
    uint32_t _triggerPosition, _getBufSize = 0, _startIndex, _preTriggerCount, _writePointer;
    int _triggerDelay, _preZero, _postZero;
    double _deltaSample;
    float _timeScale = 0;
    float  trigLevel = 0;
    float _lastTimeScale, _lastTimeOffset = 0;

    mutex.lock();
    float *data[MAX_ADC_CHANNELS];
    for(int i = 0; i < MAX_ADC_CHANNELS;i++){
        data[i] = new float[ADC_BUFFER_SIZE];
        raw_data[i] = new uint16_t[ADC_BUFFER_SIZE];
        if (data[i] == nullptr || raw_data[i] == nullptr){
            fprintf(stderr,"[Fatal error] Non memory\n");
            exit(-1);
        }
    }
    mutex.unlock();

    bool thisLoopAcqStart, manuallyTriggered = false;
    bool fill_state;

    // Used to recalculate equal values during the stop of the ADC
    rp_pinState_t raw_data_state_ch[MAX_ADC_CHANNELS];
    auto adc_channels = getADCChannels();
    for(auto ch = 0 ;ch < adc_channels; ++ch){
        rp_AcqGetGain((rp_channel_t)ch, &raw_data_state_ch[ch]);
    }

    ECHECK_APP_NO_RET(osc_getTimeScale(&_timeScale));
    threadTimer = _clock() + calculateTimeOut(_timeScale);
	_lastTimeScale = _timeScale;
	_deltaSample = (double)timeToIndex1(_timeScale) / (double)samplesPerDivision;
    while (true) {
        do{
            if(updateView) {
                ECHECK_APP_NO_RET(osc_getTimeScale(&_timeScale));
                threadUpdateView(raw_data, _getBufSize, _deltaSample, _timeScale, _lastTimeScale, _lastTimeOffset,raw_data_state_ch);
            }

            pthread_testcancel();
        }while(!oscRunning || !acqRunning);

        thisLoopAcqStart = false;

        ECHECK_APP_NO_RET(osc_getTimeScale(&_timeScale));

        if (clear && acqRunning) {
			RestartAcq(_timeScale);
			manuallyTriggered = false;
            _getBufSize = 0;
        }

        // If in auto mode end trigger timed out
        if (acqRunning && !manuallyTriggered && trigSweep == RPAPP_OSC_TRIG_AUTO && (threadTimer + 100 < _clock())) { // +100 need for fix timeout. Then signal is very low for viewsize
            ECHECK_APP_NO_RET(rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW));
            manuallyTriggered = true;
        } else if (acqRunning && trigSweep != RPAPP_OSC_TRIG_AUTO && (threadTimer < _clock())) {
            manuallyTriggered = false;
        }

        ECHECK_APP_NO_RET(rp_AcqGetTriggerSrc(&_triggerSource));
        ECHECK_APP_NO_RET(rp_AcqGetTriggerState(&_state));
        ECHECK_APP_NO_RET(rp_AcqGetBufferFillState(&fill_state));
        if(updateView && !((_state == RP_TRIG_STATE_TRIGGERED) || fill_state)) {
            threadUpdateView(raw_data, _getBufSize, _deltaSample, _timeScale, _lastTimeScale, _lastTimeOffset,raw_data_state_ch);
        } else if ((_state == RP_TRIG_STATE_TRIGGERED) || fill_state) {

            EXECUTE_ATOMICALLY(mutex, updateView = false);
            // Read parameters
            ECHECK_APP_NO_RET(rp_AcqGetWritePointerAtTrig(&_triggerPosition));
            ECHECK_APP_NO_RET(rp_AcqGetDecimation(&decimation));
            ECHECK_APP_NO_RET(rp_AcqGetTriggerDelay(&_triggerDelay));
            ECHECK_APP_NO_RET(rp_AcqGetPreTriggerCounter(&_preTriggerCount));
            ECHECK_APP_NO_RET(rp_AcqGetBufferFillState(&fill_state));
            ECHECK_APP_NO_RET(osc_getTriggerLevel(&trigLevel));
            // if (decimation == RP_DEC_1){
            //     _triggerPosition = (ADC_BUFFER_SIZE + _triggerPosition + 2) % ADC_BUFFER_SIZE;
            // }
            // if (decimation == RP_DEC_2){
            //     _triggerPosition = (ADC_BUFFER_SIZE + _triggerPosition + 1) % ADC_BUFFER_SIZE;
            // }

            if ((_state == RP_TRIG_STATE_TRIGGERED) && (!fill_state)){
                waitToFillAfterTriggerBuffer(true,_triggerPosition);
            }

            // Calculate transformation (form data to view) parameters
            _deltaSample = (double)timeToIndex1(_timeScale) / (double)samplesPerDivision;
            _lastTimeScale = _timeScale;
            _lastTimeOffset = timeOffset;
            _triggerDelay = _triggerDelay % ADC_BUFFER_SIZE;

            _preZero = 0; //continuousMode ? 0 : (int) MAX(0, viewSize/2 - (_triggerDelay+_preTriggerCount)/_deltaSample);
            _postZero = 0; //(int) MAX(0, viewSize/2 - (_writePointer-(_triggerPosition+_triggerDelay))/_deltaSample);
            _startIndex = (_triggerPosition + _triggerDelay - (uint32_t) ((double)(viewSize/2 -_preZero)*_deltaSample)- 1) % ADC_BUFFER_SIZE;
            _getBufSize = (uint32_t) ((double)(viewSize-(_preZero + _postZero))*_deltaSample) + 2;
            if(manuallyTriggered && continuousMode) {
                ECHECK_APP_NO_RET(rp_AcqGetWritePointer(&_writePointer));
                _startIndex = (_writePointer - _getBufSize) % ADC_BUFFER_SIZE;
            }
            // Get data
            mutex.lock();
            buffers_t buff_out;
            for(auto z = 0; z < MAX_ADC_CHANNELS;z++){
                buff_out.ch_f[z] = data[z];
                buff_out.ch_i[z] = (int16_t*)raw_data[z];
            }

            buff_out.size =  ADC_BUFFER_SIZE;
            ECHECK_APP_NO_RET(rp_AcqGetDataV2(_startIndex,&buff_out));
            buff_out.size =  ADC_BUFFER_SIZE;
            ECHECK_APP_NO_RET(rp_AcqGetDataRawV2(_startIndex,&buff_out)); // Need for calculate measures

            for(auto ch = 0 ;ch < adc_channels; ++ch){
               rp_AcqGetGain((rp_channel_t)ch, &raw_data_state_ch[ch]);
            }
            mutex.unlock();

            if (trigSweep == RPAPP_OSC_TRIG_SINGLE) {
                ECHECK_APP_NO_RET(threadSafe_acqStop());
            }

            if (fill_state && acqRunning) {
                if (trigSweep != RPAPP_OSC_TRIG_SINGLE) {
                  //  if (!continuousMode) {
                        ECHECK_APP_NO_RET(threadSafe_acqStart());
                  //  }
                    thisLoopAcqStart = true;
                }
            }

            g_triggerTS = _clock() + MAX((5.f * _timeScale * (float)DIVISIONS_COUNT_X), 20);

            // Reset autoSweep timer
            if (trigSweep == RPAPP_OSC_TRIG_AUTO) {
                double t = calculateTimeOut(_timeScale);
                threadTimer = _clock() + t;
                if (manuallyTriggered && !thisLoopAcqStart) {
                    ECHECK_APP_NO_RET(threadSafe_acqStart());
                    thisLoopAcqStart = true;
                }
            } else {
                threadTimer = _clock() + WAIT_TO_FILL_BUF_TIMEOUT;
            }

            mutex.lock();
            // Write data to view buffer
            for (auto channel = (int)0; channel < (int)adc_channels; ++channel) {
                // first preZero data are wrong - from previout trigger. Last preZero data hasn't been overwritten


                if(_deltaSample < 1.0f) {

                    double offset = 0;
                    // // This logic for calculate trigger position
                    // int trigPos = _getBufSize / 2;
                    // double xLen = 1/_deltaSample;
                    // double d1 = data[channel][trigPos-1];
                    // double d2 = data[channel][trigPos];
                    // double v = xLen;
                    // double w = d2 - d1;

                    // double v2 = xLen;
                    // double w2 = 0;


                    // double lenBlue = sqrt(v * v + w * w);
                    // double lenRed = xLen;

                    // double x = v / lenBlue;
                    // double y = w / lenBlue;
                    // double x2 = 1;
                    // double y2 = 0;

                    // if (w != 0 && v2 != 0){
                    //     double t2 = (v * trigLevel - v * d1) / (w * v2);
                    //     double t = (v2 * t2) / v;
                    //     if (t >= 0 || t <= 1 || t2 >= 0 || t2 <= 1){
                    //         offset = v2 * t2;
                    //     }
                    // }

                    //    fprintf(stderr,"s1 %f s2 %f _deltaSample %lf points smaples %f offset %f\n",data[channel][trigPos-1],data[channel][trigPos],_deltaSample,1/_deltaSample,offset);


                    for (int i = offset; i < (viewSize-_postZero + 1 + offset) && (int) ((double)i * _deltaSample) < _getBufSize ; ++i) {
                        int x0 = (int)((double)i * _deltaSample);
                        int x00 = ((size_t)x0) % ADC_BUFFER_SIZE;
                        int x1 = ((x0 + 1) < (_getBufSize - 1)) ? (x0 + 1) : (_getBufSize - 1);
                        float y = linear(x0, data[channel][x00], x0 + 1, data[channel][x1], ((double)i * _deltaSample));
//                        if (i > (int)offset)
                            ECHECK_APP_NO_RET(scaleAmplitudeChannel((rpApp_osc_source) channel, y, view + ((channel * viewSize) + i - (int)offset + _preZero)));
                    }
                } else {
                    for (int i = 0; i < (viewSize-_postZero + 1) && (int) ((double)i * _deltaSample) < _getBufSize && (int) ((double)i * _deltaSample) <= ADC_BUFFER_SIZE ; ++i) {
                        ECHECK_APP_NO_RET(scaleAmplitudeChannel((rpApp_osc_source) channel, data[channel][(int) ((double)i * _deltaSample)], view + ((channel * viewSize) + i + _preZero)));
                    }
                }
            }

            viewStartPos = 0;
            viewEndPos = viewSize;
            mutex.unlock();

            mathThreadFunction();
            manuallyTriggered = false;
            checkAutoscale(true);
        }

        if (thisLoopAcqStart) {
            waitToFillPreTriggerBuffer(true);
            ECHECK_APP_NO_RET(osc_setTriggerSource(trigSource));
        }
    }
    mutex.lock();
    for(int i = 0; i < MAX_ADC_CHANNELS;i++){
        delete[] data[i];
        delete[] raw_data[i];
        data[i] = nullptr;
        raw_data[i] = nullptr;
    }
    mutex.unlock();
}
