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


#include <math.h>
#include <float.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "osciloscopeApp.h"
#include "../../rpbase/src/common.h"
#include "./common.h"
#include "redpitaya/rp.h"

typedef float thread_data_t[2][ADC_BUFFER_SIZE];
typedef uint16_t raw_data_t[2][ADC_BUFFER_SIZE];

volatile bool acqRunning = false;
volatile bool oscRunning = false;
volatile bool clear = false;
volatile bool continuousMode = false;
volatile uint32_t viewSize = VIEW_SIZE_DEFAULT;
float *view;
volatile double ch1_ampOffset, ch2_ampOffset, math_ampOffset;
volatile double ch1_ampScale,  ch2_ampScale,  math_ampScale;
volatile float ch1_probeAtt, ch2_probeAtt;
volatile bool ch1_inverted = false; bool ch2_inverted = false, math_inverted = false;
volatile float timeScale=1, timeOffset=0;
volatile rpApp_osc_trig_sweep_t trigSweep;
volatile rpApp_osc_trig_source_t trigSource = RPAPP_OSC_TRIG_SRC_CH1;
volatile rpApp_osc_trig_slope_t trigSlope = RPAPP_OSC_TRIG_SLOPE_PE;
volatile rpApp_osc_math_oper_t operation;
volatile rp_channel_t mathSource1, mathSource2;
volatile bool updateView = false;
volatile bool autoScale = false;

raw_data_t raw_data;

volatile float samplesPerDivision = (float) VIEW_SIZE_DEFAULT / (float) DIVISIONS_COUNT_X;

volatile double threadTimer;
volatile bool mathChanged = false;

volatile double g_triggerTS = 0;

volatile uint32_t viewStartPos = 0;
volatile uint32_t viewEndPos = VIEW_SIZE_DEFAULT;

pthread_t mainThread = (pthread_t) -1;
pthread_mutex_t mutex;

static inline double _clock() {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return ((double)tp.tv_sec * 1000.f) + ((double)tp.tv_nsec / 1000000.f);
}

static inline float sign(float a) {
    return (a < 0.f) ? -1.f : 1.f;
}

static inline float linear(float x0, float y0, float x1, float y1, float x) {
    float k = (y1 - y0) / (x1 - x0);
    float b = y0 - (k * x0);
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
    float scale1 = (float) (vpp * AUTO_SCALE_AMP_SCA_FACTOR / DIVISIONS_COUNT_Y);
    float scale2 = (float) ((fabs(vpp) + (fabs(vMean) * 2.f)) * AUTO_SCALE_AMP_SCA_FACTOR / DIVISIONS_COUNT_Y);
    float scale = MAX(MAX(scale1, scale2), 0.002);
    ECHECK_APP(osc_setAmplitudeScale(channel, roundUpTo125(scale)));
    ECHECK_APP(osc_setAmplitudeOffset(channel, -vMean));
    return RP_OK;
}

static inline float convertRawData(uint16_t value, float gainV, uint32_t calibScale, int32_t dc_offs)
{
	const int ADC_BITS = 14;
	return rp_CmnCnvCntToV(ADC_BITS, value, gainV, calibScale, dc_offs, 0.0);
}

void checkAutoscale(bool fromThread);

int osc_Init() {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutex, &attr);

    view = calloc(3 * viewSize, sizeof(float));
    if (view == NULL) {
        free(view);
        view = NULL;
        return RP_EAA;
    }
    viewStartPos = 0;
    viewEndPos = viewSize;
    return RP_OK;
}

int osc_Release() {
    STOP_THREAD(mainThread);
    pthread_mutex_destroy(&mutex);
    if (view != NULL) {
        free(view);
        view = NULL;
    }
    return RP_OK;
}

int osc_SetDefaultValues() {
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
    ECHECK_APP(rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE));
    ECHECK_APP(osc_setTriggerLevel(0));
    ECHECK_APP(osc_setTriggerSweep(RPAPP_OSC_TRIG_AUTO));
    ECHECK_APP(osc_setTriggerSource(RPAPP_OSC_TRIG_SRC_CH1));
    ECHECK_APP(osc_setTimeScale(1));
    ECHECK_APP(osc_setMathOperation(RPAPP_OSC_MATH_NONE));
    ECHECK_APP(osc_setMathSources(RP_CH_1, RP_CH_2));

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
    float maxDeltaSample = 125000000.0f * scale / 1000.0f / samplesPerDivision;
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

    pthread_mutex_lock(&mutex);
    rpApp_osc_trig_sweep_t sweep;
    osc_getTriggerSweep(&sweep);
    if (scale < CONTIOUS_MODE_SCALE_THRESHOLD || sweep != RPAPP_OSC_TRIG_AUTO){
        ECHECK_APP_MUTEX(mutex, rp_AcqSetArmKeep(false))
        continuousMode = false;
    } else {
        ECHECK_APP_MUTEX(mutex, rp_AcqSetArmKeep(true))
        continuousMode = true;
    }

    timeScale = scale;
    ECHECK_APP_MUTEX(mutex, rp_AcqSetDecimation(decimation))
    update_view();
    pthread_mutex_unlock(&mutex);
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

    pthread_mutex_lock(&mutex);
    timeOffset = offset;
    ECHECK_APP_MUTEX(mutex, rp_AcqSetTriggerDelayNs((int64_t)(offset * MILLI_TO_NANO)));
    update_view();
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

int osc_getTimeOffset(float *offset) {
    *offset = timeOffset;
    return RP_OK;
}

int osc_setProbeAtt(rp_channel_t channel, float att) {
    CHANNEL_ACTION(channel,
                   ch1_probeAtt = att,
                   ch2_probeAtt = att)

    EXECUTE_ATOMICALLY(mutex, update_view());
    return RP_OK;
}

int osc_getProbeAtt(rp_channel_t channel, float *att) {
    CHANNEL_ACTION(channel,
                   *att = ch1_probeAtt,
                   *att = ch2_probeAtt)
    return 0;
}

int osc_setInputGain(rp_channel_t channel, rpApp_osc_in_gain_t gain) {
    pthread_mutex_lock(&mutex);
    switch (gain) {
        case RPAPP_OSC_IN_GAIN_LV:
            ECHECK_APP_MUTEX(mutex, rp_AcqSetGain(channel, RP_LOW));
            break;
        case RPAPP_OSC_IN_GAIN_HV:
            ECHECK_APP_MUTEX(mutex, rp_AcqSetGain(channel, RP_HIGH));
            break;
        default:
            return RP_EOOR;
    }
    update_view();
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

int osc_getInputGain(rp_channel_t channel, rpApp_osc_in_gain_t *gain) {
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
            return RP_EOOR;
    }
    return RP_OK;
}

int osc_setAmplitudeScale(rpApp_osc_source source, double scale) {
    double offset, currScale;
    pthread_mutex_lock(&mutex);
    ECHECK_APP_MUTEX(mutex, osc_getAmplitudeOffset(source, &offset));
    ECHECK_APP_MUTEX(mutex, osc_getAmplitudeScale(source, &currScale));
    offset = offset / currScale;
    SOURCE_ACTION(source,
                  ch1_ampScale = scale,
                  ch2_ampScale = scale,
                  math_ampScale = scale)
    offset *= currScale;
    pthread_mutex_unlock(&mutex);
    if (!isnan(offset)) {
        ECHECK_APP(osc_setAmplitudeOffset(source, offset));
    }
    EXECUTE_ATOMICALLY(mutex, update_view());
    return RP_OK;
}

int osc_getAmplitudeScale(rpApp_osc_source source, double *scale) {
    SOURCE_ACTION(source,
                  *scale = ch1_ampScale,
                  *scale = ch2_ampScale,
                  *scale = math_ampScale)
    return RP_OK;
}

int osc_setAmplitudeOffset(rpApp_osc_source source, double offset) {
    pthread_mutex_lock(&mutex);
    SOURCE_ACTION(source,
                  ch1_ampOffset = offset,
                  ch2_ampOffset = offset,
                  math_ampOffset = offset)

    update_view();
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

int osc_getAmplitudeOffset(rpApp_osc_source source, double *offset) {
    SOURCE_ACTION(source,
                  *offset = ch1_ampOffset,
                  *offset = ch2_ampOffset,
                  *offset = math_ampOffset)
    return RP_OK;
}

int osc_setTriggerSource(rpApp_osc_trig_source_t triggerSource) {
    pthread_mutex_lock(&mutex);
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
        case RPAPP_OSC_TRIG_SRC_EXTERNAL:
            if (trigSlope == RPAPP_OSC_TRIG_SLOPE_NE) {
                src = RP_TRIG_SRC_EXT_NE;
            }
            else {
                src = RP_TRIG_SRC_EXT_PE;
            }
            break;
        default:
            return RP_EOOR;
    }

    trigSource = triggerSource;
    ECHECK_APP_MUTEX(mutex, rp_AcqSetTriggerSrc(src));
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

int osc_getTriggerSource(rpApp_osc_trig_source_t *triggerSource) {
    *triggerSource = trigSource;
    return RP_OK;
}

int osc_setTriggerSlope(rpApp_osc_trig_slope_t slope) {
    pthread_mutex_lock(&mutex);
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
        case RPAPP_OSC_TRIG_SRC_EXTERNAL:
            if (slope == RPAPP_OSC_TRIG_SLOPE_NE) {
                src = RP_TRIG_SRC_EXT_NE;
            }
            else {
                src = RP_TRIG_SRC_EXT_PE;
            }
            break;
        default:
            return RP_EOOR;
    }

    trigSlope = slope;
    ECHECK_APP_MUTEX(mutex, rp_AcqSetTriggerSrc(src));
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

int osc_getTriggerSlope(rpApp_osc_trig_slope_t *slope) {
    *slope = trigSlope;
    return RP_OK;
}

int osc_setTriggerLevel(float level) {
    pthread_mutex_lock(&mutex);

    if((trigSource == RPAPP_OSC_TRIG_SRC_CH1) || (trigSource == RPAPP_OSC_TRIG_SRC_CH2)) {
        rpApp_osc_source source = (trigSource == RPAPP_OSC_TRIG_SRC_CH1) ? RPAPP_OSC_SOUR_CH1 : RPAPP_OSC_SOUR_CH2;
        ECHECK_APP_MUTEX(mutex, unattenuateAmplitudeChannel(source, level, &level));
    }

    ECHECK_APP_MUTEX(mutex, rp_AcqSetTriggerLevel(level));
    update_view();
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

int osc_getTriggerLevel(float *level) {
    ECHECK_APP(rp_AcqGetTriggerLevel(level));

    if((trigSource == RPAPP_OSC_TRIG_SRC_CH1) || (trigSource == RPAPP_OSC_TRIG_SRC_CH2)) {
        rpApp_osc_source source = (trigSource == RPAPP_OSC_TRIG_SRC_CH1) ? RPAPP_OSC_SOUR_CH1 : RPAPP_OSC_SOUR_CH2;
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
    pthread_mutex_lock(&mutex);
    SOURCE_ACTION(source,
                  ch1_inverted = inverted,
                  ch2_inverted = inverted,
                  math_inverted = inverted)
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

int osc_isInverted(rpApp_osc_source source, bool *inverted) {
    SOURCE_ACTION(source,
                  *inverted = ch1_inverted,
                  *inverted = ch2_inverted,
                  *inverted = math_inverted)
    return RP_OK;
}

int osc_getViewPart(float *ratio) {
    *ratio = ((float)viewSize * (float)timeToIndex(timeScale) / samplesPerDivision) / (float)ADC_BUFFER_SIZE;
    return RP_OK;
}

int osc_measureVpp(rpApp_osc_source source, float *Vpp) {
    float resMax, resMin, max = -FLT_MAX, min = FLT_MAX;

    pthread_mutex_lock(&mutex);
    for (int i = viewStartPos; i < viewEndPos; ++i) {
        if (view[source*viewSize + i] > max) {
            max = view[source*viewSize + i];
        }
        if (view[source*viewSize + i] < min) {
            min = view[source*viewSize + i];
        }
    }
    pthread_mutex_unlock(&mutex);

    ECHECK_APP(unscaleAmplitudeChannel(source, max, &resMax));
    ECHECK_APP(unscaleAmplitudeChannel(source, min, &resMin));
    *Vpp = resMax - resMin;
    ECHECK_APP(attenuateAmplitudeChannel(source, *Vpp, Vpp));
    *Vpp = fabs(*Vpp);
    return RP_OK;
}

int osc_measureMeanVoltage(rpApp_osc_source source, float *meanVoltage) {
    double sum = 0;

    pthread_mutex_lock(&mutex);
    for (int i = viewStartPos; i < viewEndPos; ++i) {
        sum += view[source*viewSize + i];
    }
    pthread_mutex_unlock(&mutex);

    ECHECK_APP(unscaleAmplitudeChannel(source, sum / (viewEndPos - viewStartPos), meanVoltage));
    ECHECK_APP(attenuateAmplitudeChannel(source, *meanVoltage, meanVoltage));
    return RP_OK;
}

int osc_measureMaxVoltage(rpApp_osc_source source, float *Vmax) {
    float max = view[source*viewSize];

    pthread_mutex_lock(&mutex);
    bool inverted = (source == 0 && ch1_inverted) || (source == 1 && ch2_inverted) || (source == 2 && math_inverted);
    for (int i = viewStartPos; i < viewEndPos; ++i) {
        if (inverted ? view[source*viewSize + i] < max : view[source*viewSize + i] > max) {
            max = view[source*viewSize + i];
        }
    }
    *Vmax = max;
    pthread_mutex_unlock(&mutex);

    ECHECK_APP(unscaleAmplitudeChannel(source, max, Vmax));
    ECHECK_APP(attenuateAmplitudeChannel(source, *Vmax, Vmax));
    return RP_OK;
}

int osc_measureMinVoltage(rpApp_osc_source source, float *Vmin) {
    float min = view[source*viewSize];

    pthread_mutex_lock(&mutex);
    bool inverted = (source == 0 && ch1_inverted) || (source == 1 && ch2_inverted) || (source == 2 && math_inverted);
    for (int i = viewStartPos; i < viewEndPos; ++i) {
        if (inverted ? view[source*viewSize + i] > min : view[source*viewSize + i] < min) {
            min = view[source*viewSize + i];
        }
    }
    *Vmin = min;
    pthread_mutex_unlock(&mutex);

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

int osc_measurePeriodMath(rpApp_osc_source source, float *period)
{
    int size = viewEndPos - viewStartPos;
    float data[size];
    float* ch_view = view + source*viewSize + viewStartPos;

    pthread_mutex_lock(&mutex);
    float mean = 0;
    for (int i = 0; i < size; ++i) {
        data[i] = ch_view[i];
        mean += data[i];
    }
    pthread_mutex_unlock(&mutex);

    mean = mean / size;
    for (int i = 0; i < size; ++i){
        data[i] -= mean;
    }

    // calculate signal correlation
    float xcorr[size];
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


int osc_adc_sign(int in_data)
{
	const int c_osc_fpga_adc_bits = 14;
    int s_data = in_data;
    if(s_data & (1<<(c_osc_fpga_adc_bits-1)))
        s_data = -1 * ((s_data ^ ((1<<c_osc_fpga_adc_bits)-1)) + 1);
    return s_data;
}


int osc_measurePeriodCh(rpApp_osc_source source, float *period) {
    const float c_osc_fpga_smpl_freq = 125000000;
    const float c_meas_freq_thr = 100;
    uint32_t start, end;
    osc_getViewLimits(&start, &end);

    int size = ADC_BUFFER_SIZE;
    const int c_meas_time_thr = ADC_BUFFER_SIZE / 8;
    const float c_min_period = 19.6e-9; // 51 MHz

    float thr1, thr2, cen;
    int state = 0;
    int trig_t[2] = { 0, 0 };
    int trig_cnt = 0;
    int ix;

    uint16_t data[ADC_BUFFER_SIZE];
    osc_getRawData((source == RPAPP_OSC_SOUR_CH1) ? RP_CH_1 : RP_CH_2, data, ADC_BUFFER_SIZE);

	float meas_max, meas_min;
	//if (dataSize.Value() > 0)
	{
		meas_max = data[0];
		meas_min = data[0];
		int i = 0;
		for(; i<size; i++)
		{
			meas_max = (meas_max > data[i])? meas_max : data[i];
			meas_min = (meas_min < data[i])? meas_min : data[i];
		}
	}

	uint32_t dec_factor = 1;
    ECHECK_APP(rp_AcqGetDecimationFactor(&dec_factor));

    float acq_dur=(float)(size)/((float) c_osc_fpga_smpl_freq) * (float) dec_factor;
    cen = (meas_max + meas_min) / 2;
    thr1 = cen + 0.2 * (meas_min - cen);
    thr2 = cen + 0.2 * (meas_max - cen);
    float res_period = 0;
    for(ix = 0; ix < (size); ix++) {
        uint16_t sa = (uint16_t)osc_adc_sign((int)data[ix]);


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

    pthread_mutex_lock(&mutex);
    for (int i = viewStartPos; i < viewEndPos; ++i) {
        if (view[source*viewSize + i] > meanValue) {
            ++highTime;
        }
    }
    pthread_mutex_unlock(&mutex);

    *dutyCycle = (float)highTime / (float)(viewEndPos - viewStartPos);
    return RP_OK;
}

int osc_measureRootMeanSquare(rpApp_osc_source source, float *rms) {
    double rmsValue = 0;

    pthread_mutex_lock(&mutex);
    for (int i = viewStartPos; i < viewEndPos; ++i) {
        float tmp;
        unscaleAmplitudeChannel(source, view[source*viewSize + i], &tmp);
        rmsValue += tmp*tmp;
    }
    pthread_mutex_unlock(&mutex);

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
    *value = indexToTime(abs(cursor1 - cursor2));
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
    pthread_mutex_lock(&mutex);
    operation = op;
    mathChanged = true;
    clearMath();
    update_view();
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

int osc_getMathOperation(rpApp_osc_math_oper_t *op) {
    *op = operation;
    return RP_OK;
}

int osc_setMathSources(rp_channel_t source1, rp_channel_t source2) {
    pthread_mutex_lock(&mutex);
    mathSource1 = source1;
    mathSource2 = source2;
    mathChanged = true;
    clearMath();
    update_view();
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

int osc_getMathSources(rp_channel_t *source1, rp_channel_t *source2) {
    *source1 = mathSource1;
    *source2 = mathSource2;
    return RP_OK;
}

int osc_getData(rpApp_osc_source source, float *data, uint32_t size) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < size; ++i) {
        data[i] = view[source*viewSize + i];
    }
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

int osc_getRawData(rp_channel_t source, uint16_t *data, uint32_t size) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < size; ++i) {
        data[i] = raw_data[source][i];
    }
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

int osc_setViewSize(uint32_t size) {
    viewSize = size;
    samplesPerDivision = (float) viewSize / (float) DIVISIONS_COUNT_X;

    pthread_mutex_lock(&mutex);
    view = realloc(view, 3 * viewSize * sizeof(float));
    if (view == NULL) {
        pthread_mutex_unlock(&mutex);
        return RP_EAA;
    }
    pthread_mutex_unlock(&mutex);
    EXECUTE_ATOMICALLY(mutex, update_view());
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

    pthread_mutex_lock(&mutex);
    ECHECK_APP_MUTEX(mutex, rp_AcqStart())
    ECHECK_APP_MUTEX(mutex, rp_AcqSetArmKeep(trigSweep != RPAPP_OSC_TRIG_SINGLE && continuousMode));
    acqRunning = true;
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

int threadSafe_acqStop() {
    pthread_mutex_lock(&mutex);
    ECHECK_APP_MUTEX(mutex, rp_AcqStop())
    ECHECK_APP_MUTEX(mutex, rp_AcqSetArmKeep(false))
    acqRunning = false;
    pthread_mutex_unlock(&mutex);
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

    bool invert = (channel == RP_CH_1) ? ch1_inverted : ch2_inverted;
    float ch_sign = invert ? -1.f : 1.f;

    ECHECK_APP_THREAD(unscaleAmplitudeChannel((rpApp_osc_source) channel, view[channel*viewSize + viewStartPos], &v));
	ECHECK_APP_THREAD(attenuateAmplitudeChannel((rpApp_osc_source) channel, v, &v));
    view[RPAPP_OSC_SOUR_MATH*viewSize] = ch_sign * v * dt;
    for (int i = viewStartPos + 1; i < viewEndPos; ++i) {
        ECHECK_APP_THREAD(unscaleAmplitudeChannel((rpApp_osc_source) channel, view[channel*viewSize + i], &v));
		ECHECK_APP_THREAD(attenuateAmplitudeChannel((rpApp_osc_source) channel, v, &v));
        view[RPAPP_OSC_SOUR_MATH*viewSize + i] = view[RPAPP_OSC_SOUR_MATH*viewSize + i-1] + (ch_sign * v * dt);
        view[RPAPP_OSC_SOUR_MATH*viewSize + i-1] = scaleAmplitude(view[RPAPP_OSC_SOUR_MATH*viewSize + i-1], scale, 1, offset, invertFactor);
    }
    view[RPAPP_OSC_SOUR_MATH*viewSize + viewEndPos - 1] = scaleAmplitude(view[RPAPP_OSC_SOUR_MATH*viewSize + viewEndPos - 1], scale, 1, offset, invertFactor);
}

void calculateDevivative(rp_channel_t channel, float scale, float offset, float invertFactor) {
    float dt2 = 2*timeScale / 1000 / samplesPerDivision;
    float v1, v2;

    bool invert = (channel == RP_CH_1) ? ch1_inverted : ch2_inverted;
    float ch_sign = invert ? -1.f : 1.f;

    ECHECK_APP_THREAD(unscaleAmplitudeChannel((rpApp_osc_source) channel, view[channel*viewSize + viewStartPos], &v2));
	ECHECK_APP_THREAD(attenuateAmplitudeChannel((rpApp_osc_source) channel, v2, &v2));
    for (int i = viewStartPos; i < viewEndPos - 1; ++i) {
        v1 = v2;
        ECHECK_APP_THREAD(unscaleAmplitudeChannel((rpApp_osc_source) channel, view[channel*viewSize + i + 1], &v2));
		ECHECK_APP_THREAD(attenuateAmplitudeChannel((rpApp_osc_source) channel, v2, &v2));
        view[RPAPP_OSC_SOUR_MATH*viewSize + i] = scaleAmplitude(ch_sign * (v2 - v1) / dt2, scale, 1, offset, invertFactor);
    }
    view[RPAPP_OSC_SOUR_MATH*viewSize + viewEndPos - 1] = view[RPAPP_OSC_SOUR_MATH*viewSize + viewEndPos - 2];
}

float calculateMath(float v1, float v2, rpApp_osc_math_oper_t op) {
    switch (op) {
        case RPAPP_OSC_MATH_ADD:
            return v1 + v2;
        case RPAPP_OSC_MATH_SUB:
            return v1 - v2;
        case RPAPP_OSC_MATH_MUL:
            return v1 * v2;
        case RPAPP_OSC_MATH_DIV:
            if (v2 != 0)
                return v1 / v2;
            else
                return v1 > 0 ? FLT_MAX * 0.9f : -FLT_MAX * 0.9f;
        case RPAPP_OSC_MATH_ABS:
            return (float) fabs(v1);
        default:
            return 0;
    }
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
    int size = 3*viewSize;
    for (int i = 0; i < size; ++i) {
        view[i] = 0;
    }
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
    } while (preTriggerCount < viewSize/2*deltaSample - triggerDelay && localTimer > _clock());
    return RP_OK;
}

int waitToFillAfterTriggerBuffer(bool testcancel) {
    double localTimer = testcancel ? threadTimer : _clock() + WAIT_TO_FILL_BUF_TIMEOUT;
    float deltaSample, timeScale;
    uint32_t _writePointer, _triggerPosition;
    int triggerDelay;

    ECHECK_APP_THREAD(rp_AcqGetWritePointerAtTrig(&_triggerPosition));
    ECHECK_APP(rp_AcqGetTriggerDelay(&triggerDelay));
    ECHECK_APP(osc_getTimeScale(&timeScale));
    deltaSample = timeToIndex(timeScale) / samplesPerDivision;

    do {
        ECHECK_APP_THREAD(rp_AcqGetWritePointer(&_writePointer));

        if(testcancel)
            pthread_testcancel();

    } while ((((_writePointer - _triggerPosition) % ADC_BUFFER_SIZE) <= ((viewSize/2.f) * deltaSample) + triggerDelay) && localTimer > _clock());
    return RP_OK;
}

/*
* Thread functions
*/

static inline void scaleMath() {
    if (mathChanged && !autoScale) {
        mathChanged = false;
        float vpp, vMean;
        ECHECK_APP_THREAD(osc_measureVpp(RPAPP_OSC_SOUR_MATH, &vpp));
        ECHECK_APP_THREAD(osc_measureMeanVoltage(RPAPP_OSC_SOUR_MATH, &vMean));
        // Calculate scale
        float scale = vpp * AUTO_SCALE_AMP_SCA_FACTOR / DIVISIONS_COUNT_Y;
		if (scale <= FLOAT_EPS) {
			ECHECK_APP_THREAD(osc_setAmplitudeScale(RPAPP_OSC_SOUR_MATH, roundUpTo25(1)));
		} else {
			ECHECK_APP_THREAD(osc_setAmplitudeScale(RPAPP_OSC_SOUR_MATH, roundUpTo25(scale)));
			ECHECK_APP_THREAD(osc_setAmplitudeOffset(RPAPP_OSC_SOUR_MATH, -vMean));
		}
	}
}

void mathThreadFunction() {
    if (operation != RPAPP_OSC_MATH_NONE) {
        bool invert;
        ECHECK_APP_THREAD(osc_isInverted(RPAPP_OSC_SOUR_MATH, &invert))
        float invertFactor = invert ? -1 : 1;
        if (operation == RPAPP_OSC_MATH_DER) {
            calculateDevivative(mathSource1, math_ampScale, math_ampOffset, invertFactor);
        } else if (operation == RPAPP_OSC_MATH_INT) {
            calculateIntegral(mathSource1, math_ampScale, math_ampOffset, invertFactor);
        } else {
            float v1, v2;
            bool invert1 = (mathSource1 == RP_CH_1) ? ch1_inverted : ch2_inverted;
            bool invert2 = (mathSource2 == RP_CH_1) ? ch1_inverted : ch2_inverted;
            float sign1 = invert1 ? -1.f : 1.f;
            float sign2 = invert2 ? -1.f : 1.f;

            for (int i = viewStartPos; i < viewEndPos; ++i) {
                ECHECK_APP_THREAD(unscaleAmplitudeChannel((rpApp_osc_source) mathSource1, view[mathSource1*viewSize + i], &v1));
                ECHECK_APP_THREAD(unscaleAmplitudeChannel((rpApp_osc_source) mathSource2, view[mathSource2*viewSize + i], &v2));

                ECHECK_APP_THREAD(attenuateAmplitudeChannel((rpApp_osc_source) mathSource1, v1, &v1));
                ECHECK_APP_THREAD(attenuateAmplitudeChannel((rpApp_osc_source) mathSource2, v2, &v2));

                view[RPAPP_OSC_SOUR_MATH*viewSize + i] = scaleAmplitude(calculateMath(sign1 * v1, sign2 * v2, operation), math_ampScale, 1, math_ampOffset, invertFactor);
            }
        }
        scaleMath();
    }
}

void checkAutoscale(bool fromThread) {
    pthread_mutex_lock(&mutex);
    if((autoScale == false) && (fromThread == true))
    {
        pthread_mutex_unlock(&mutex);
        return;
    }

    static const float scales[AUTO_SCALE_NUM_OF_SCALE] = {0.00005f, 0.0001f, 0.0002f, 0.0005f, 0.001f, 0.002f, 0.005f, 0.01f, 0.02f, 0.05f, 0.1f, 0.2f, 0.5f, 1.f, 2.f, 5.f, 10.f, 20.f, 50.f, 100.f};
    static int timeScaleIdx = 0;
    static float periods[2][AUTO_SCALE_NUM_OF_SCALE];
    static float vpps[2][AUTO_SCALE_NUM_OF_SCALE];
    static float vMeans[2][AUTO_SCALE_NUM_OF_SCALE];
    static float savedTimeScale;
    static int measCount;

    float period, vpp, vMean;
    int ret;

    int periodsIdx[2];
    int repCounts[2];

    int vMeansIdx[2];
    int vMeansRepCounts[2];

    float period_to_set = 1.f;

    if(!fromThread) {
        if(autoScale) {
            pthread_mutex_unlock(&mutex);
            return;
        }

        autoScale = true;
        osc_getTimeScale(&savedTimeScale);
        timeScaleIdx = 0;
        period_to_set = scales[timeScaleIdx];
        measCount = 0;
    } else {
        if(++measCount < 2) {
            pthread_mutex_unlock(&mutex);
            return;
        }

        measCount = 0;

        for (rpApp_osc_source source = RPAPP_OSC_SOUR_CH1; source <= RPAPP_OSC_SOUR_CH2; ++source) {
            ECHECK_APP_THREAD(osc_measureVpp(source, &vpp));
            ECHECK_APP_THREAD(osc_measureMeanVoltage(source, &vMean));

            if (fabs(vpp) > SIGNAL_EXISTENCE) {
                ret = osc_measurePeriod(source, &period);
                periods[source][timeScaleIdx] = (ret == RP_OK) ? period : 0.f;
            } else {
                periods[source][timeScaleIdx] = 0.f;
            }

            vpps[source][timeScaleIdx] = vpp;
            vMeans[source][timeScaleIdx] = vMean;
        }

        if(++timeScaleIdx >= AUTO_SCALE_NUM_OF_SCALE) {
            autoScale = false;

            for (rpApp_osc_source source = RPAPP_OSC_SOUR_CH1; source <= RPAPP_OSC_SOUR_CH2; ++source) {
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
            } else {
                period_to_set = savedTimeScale;
            }

            period_to_set = MAX(0.0001f, period_to_set);
            period_to_set = MIN(500.f, period_to_set);

            for (rpApp_osc_source source = RPAPP_OSC_SOUR_CH1; source <= RPAPP_OSC_SOUR_CH2; ++source) {
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

                ECHECK_APP_THREAD(scaleChannel(source, vpp, vMean));
            }
        } else {
            period_to_set = scales[timeScaleIdx];
        }
    }
    ECHECK_APP_THREAD(osc_setTimeScale(period_to_set));
    ECHECK_APP_THREAD(osc_setTimeOffset(AUTO_SCALE_TIME_OFFSET));
    pthread_mutex_unlock(&mutex);
}

static inline void threadUpdateView(uint16_t data[2][ADC_BUFFER_SIZE], uint32_t _getBufSize, float _deltaSample, float _timeScale, float _lastTimeScale, float _lastTimeOffset) {
    pthread_mutex_lock(&mutex);
    updateView = false;

    if(_getBufSize == 0) {
        clearView();
        pthread_mutex_unlock(&mutex);
        return;
    }

    float curDeltaSample = _deltaSample * (_timeScale / _lastTimeScale);
    int requiredBuffSize = viewSize * curDeltaSample;
    int bufferEars = ((int)ADC_BUFFER_SIZE - requiredBuffSize) / 2;
    int viewEars = -MIN(bufferEars / curDeltaSample, 0);
    bufferEars = MAX(0, bufferEars);
    int viewOffset = ((_lastTimeOffset - timeOffset) * (float)samplesPerDivision) / _timeScale;
    int buffOffset = viewOffset * curDeltaSample;

    if(viewEars) {
        buffOffset = 0;
    } else {
        if((bufferEars + (ADC_BUFFER_SIZE / 2)) < abs(buffOffset)) {
            viewOffset = (abs(buffOffset) - bufferEars) * sign(buffOffset) / curDeltaSample;
            buffOffset = bufferEars * sign(buffOffset);
        } else {
            viewOffset = 0;
        }
    }
    int maxViewIdx = MIN(viewSize, (viewSize - 2*viewEars));
    int buffFullOffset = bufferEars - buffOffset - (ADC_BUFFER_SIZE - _getBufSize)/2;

    float gainV1, gainV2;
    rp_pinState_t gain1, gain2;

    rp_AcqGetGainV(RP_CH_1, &gainV1);
    rp_AcqGetGain(RP_CH_1, &gain1);
    rp_AcqGetGainV(RP_CH_2, &gainV2);
    rp_AcqGetGain(RP_CH_2, &gain2);

    rp_calib_params_t calib = rp_GetCalibrationSettings();
    int32_t dc_offs1 = gain1 == RP_HIGH ? calib.fe_ch1_hi_offs : calib.fe_ch1_lo_offs;
    uint32_t calibScale1 = gain1 == RP_HIGH ? calib.fe_ch1_fs_g_hi : calib.fe_ch1_fs_g_lo;
    int32_t dc_offs2 = gain2 == RP_HIGH ? calib.fe_ch2_hi_offs : calib.fe_ch2_lo_offs;
	uint32_t calibScale2 = gain2 == RP_HIGH ? calib.fe_ch2_fs_g_hi : calib.fe_ch2_fs_g_lo;

    // Write data to view buffer
    for (rp_channel_t channel = RP_CH_1; channel <= RP_CH_2; ++channel) {
        int viewFullOffset = (channel * viewSize) + viewEars + viewOffset;
        for(int i = 0; i < viewEars + viewOffset; ++i) {
            view[(int)channel * viewSize + i] = 0.f;
        }

        for(int i = 0; i < viewEars - viewOffset; ++i) {
            view[(int)channel * viewSize + viewSize - i - 1] = 0.f;
        }

        if(curDeltaSample < 1.0f) {
            int i;
            for (i = 0; i < maxViewIdx /*&& (int) (((float)i * curDeltaSample) + buffFullOffset) < ADC_BUFFER_SIZE*/; ++i) {
                int x0 = ((int)((float)i * curDeltaSample) + buffFullOffset) % ADC_BUFFER_SIZE;
                int x1 = (x0 + 1) % ADC_BUFFER_SIZE;
                float y = linear(x0, convertRawData(data[channel][x0], gainV1, calibScale1, dc_offs1), x0 + 1,
					convertRawData(data[channel][x1], gainV2, calibScale2, dc_offs2), ((float)i * curDeltaSample) + buffFullOffset);
                ECHECK_APP_THREAD(scaleAmplitudeChannel((rpApp_osc_source) channel, y, view + viewFullOffset + i));
            }
            maxViewIdx = i;
        } else {
            int i;
            for (i = 0; i < maxViewIdx /*&& (int) (((float)i * curDeltaSample) + buffFullOffset) < ADC_BUFFER_SIZE*/; ++i) {
				const size_t idx = ((size_t)((float)i * curDeltaSample) + buffFullOffset) % ADC_BUFFER_SIZE;
				ECHECK_APP_THREAD(scaleAmplitudeChannel((rpApp_osc_source) channel, convertRawData(data[channel][idx], gainV1, calibScale1, dc_offs1), view + viewFullOffset + i));
/*
                ECHECK_APP_THREAD(scaleAmplitudeChannel((rpApp_osc_source) channel,
					convertRawData(data[channel][((size_t) ((size_t)((float)i * curDeltaSample) + buffFullOffset)) % ADC_BUFFER_SIZE], gainV1, calibScale1, dc_offs1), view + viewFullOffset + i));
*/
            }
            maxViewIdx = i;
        }
    }
    viewStartPos = viewEars + viewOffset;
    viewEndPos = viewStartPos + maxViewIdx;

    mathThreadFunction();
    pthread_mutex_unlock(&mutex);
}

int RestartAcq(float _timeScale) {
	ECHECK_APP_THREAD(rp_AcqSetTriggerSrc(RP_TRIG_SRC_DISABLED));
	ECHECK_APP_THREAD(threadSafe_acqStart());

	threadTimer = (trigSweep == RPAPP_OSC_TRIG_AUTO) ? MAX(0.1f, (2.f * _timeScale * (float)DIVISIONS_COUNT_X)) : WAIT_TO_FILL_BUF_TIMEOUT;
	threadTimer += _clock();

	waitToFillPreTriggerBuffer(true);

	ECHECK_APP_THREAD(osc_setTriggerSource(trigSource));
	EXECUTE_ATOMICALLY(mutex, clear = false)
	EXECUTE_ATOMICALLY(mutex, updateView = false)

	return RP_OK;
}

void *mainThreadFun() {
    rp_acq_trig_src_t _triggerSource;
    rp_acq_trig_state_t _state;
    uint32_t _triggerPosition, _getBufSize = 0, _startIndex, _preTriggerCount, _writePointer;
    int _triggerDelay, _preZero, _postZero;
    float _deltaSample, _timeScale, _lastTimeScale, _lastTimeOffset = 0;
    thread_data_t data;
    bool thisLoopAcqStart, manuallyTriggered = false;

    ECHECK_APP_THREAD(osc_getTimeScale(&_timeScale));
    threadTimer = _clock() + MAX(0.1f, (2.f * _timeScale * (float)DIVISIONS_COUNT_X));
	_lastTimeScale = _timeScale;
	_deltaSample = (float)timeToIndex(_timeScale) / (float)samplesPerDivision;

    while (true) {
        do{
            if(updateView) {
                ECHECK_APP_THREAD(osc_getTimeScale(&_timeScale));
                threadUpdateView(raw_data, _getBufSize, _deltaSample, _timeScale, _lastTimeScale, _lastTimeOffset);
            }

            pthread_testcancel();
        }while(!oscRunning || !acqRunning);

        thisLoopAcqStart = false;

        ECHECK_APP_THREAD(osc_getTimeScale(&_timeScale));

        if (clear && acqRunning) {
			RestartAcq(_timeScale);
			manuallyTriggered = false;
            _getBufSize = 0;
        }

        // If in auto mode end trigger timed out
        if (acqRunning && !manuallyTriggered && trigSweep == RPAPP_OSC_TRIG_AUTO && (threadTimer < _clock())) {
            ECHECK_APP_THREAD(rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW));
            manuallyTriggered = true;
        } else if (acqRunning && trigSweep != RPAPP_OSC_TRIG_AUTO && (threadTimer < _clock())) {
            RestartAcq(_timeScale);
            manuallyTriggered = false;
        }

        ECHECK_APP_THREAD(rp_AcqGetTriggerSrc(&_triggerSource));
        ECHECK_APP_THREAD(rp_AcqGetTriggerState(&_state));

        if(updateView && !((_state == RP_TRIG_STATE_TRIGGERED) || (_triggerSource == RP_TRIG_SRC_DISABLED))) {
            threadUpdateView(raw_data, _getBufSize, _deltaSample, _timeScale, _lastTimeScale, _lastTimeOffset);

        } else if ((_state == RP_TRIG_STATE_TRIGGERED) || (_triggerSource == RP_TRIG_SRC_DISABLED)) {
            EXECUTE_ATOMICALLY(mutex, updateView = false);
            // Read parameters
            ECHECK_APP_THREAD(rp_AcqGetWritePointerAtTrig(&_triggerPosition));
            ECHECK_APP_THREAD(rp_AcqGetTriggerDelay(&_triggerDelay));
            ECHECK_APP_THREAD(rp_AcqGetPreTriggerCounter(&_preTriggerCount));

            if((_state == RP_TRIG_STATE_TRIGGERED) && (_triggerSource != RP_TRIG_SRC_DISABLED)) {
                waitToFillAfterTriggerBuffer(true);
            }

            // Calculate transformation (form data to view) parameters
            _deltaSample = (float)timeToIndex(_timeScale) / (float)samplesPerDivision;
            _lastTimeScale = _timeScale;
            _lastTimeOffset = timeOffset;
            _triggerDelay = _triggerDelay % ADC_BUFFER_SIZE;

            _preZero = 0; //continuousMode ? 0 : (int) MAX(0, viewSize/2 - (_triggerDelay+_preTriggerCount)/_deltaSample);
            _postZero = 0; //(int) MAX(0, viewSize/2 - (_writePointer-(_triggerPosition+_triggerDelay))/_deltaSample);
            _startIndex = (_triggerPosition + _triggerDelay - (uint32_t) ((viewSize/2 -_preZero)*_deltaSample)) % ADC_BUFFER_SIZE;
            _getBufSize = (uint32_t) ((viewSize-(_preZero + _postZero))*_deltaSample);

            if(manuallyTriggered && continuousMode) {
                ECHECK_APP_THREAD(rp_AcqGetWritePointer(&_writePointer));
                _startIndex = (_writePointer - _getBufSize) % ADC_BUFFER_SIZE;
            }
            // Get data
            uint32_t _size = ADC_BUFFER_SIZE;
            ECHECK_APP_THREAD(rp_AcqGetDataV2(_startIndex, &_getBufSize, data[0], data[1]));
            ECHECK_APP_THREAD(rp_AcqGetDataRawV2(_startIndex, &_size, raw_data[0], raw_data[1]));

            if (trigSweep == RPAPP_OSC_TRIG_SINGLE) {
                ECHECK_APP_THREAD(threadSafe_acqStop());
            }

            if (_triggerSource == RP_TRIG_SRC_DISABLED && acqRunning) {
                if (trigSweep != RPAPP_OSC_TRIG_SINGLE) {
                    if (!continuousMode) {
                        ECHECK_APP_THREAD(threadSafe_acqStart());
                    }
                    thisLoopAcqStart = true;
                }
            }

            g_triggerTS = _clock() + 5.f * _timeScale * (float)DIVISIONS_COUNT_X;

            // Reset autoSweep timer
            if (trigSweep == RPAPP_OSC_TRIG_AUTO) {
                threadTimer = _clock() + MAX(0.1f, (2.f * _timeScale * (float)DIVISIONS_COUNT_X));

                if (manuallyTriggered && !thisLoopAcqStart) {
                    ECHECK_APP_THREAD(threadSafe_acqStart());
                    thisLoopAcqStart = true;
                }
            } else {
                threadTimer = _clock() + WAIT_TO_FILL_BUF_TIMEOUT;
            }

            pthread_mutex_lock(&mutex);
            // Write data to view buffer
            for (rp_channel_t channel = RP_CH_1; channel <= RP_CH_2; ++channel) {
                // first preZero data are wrong - from previout trigger. Last preZero data hasn't been overwritten
//                for (int i = 0; i < _preZero; ++i) {
//                    view[channel * viewSize + i] = 0;
//                }
                if(_deltaSample < 1.0f) {
                    for (int i = 0; i < viewSize-_postZero && (int) ((float)i * _deltaSample) < _getBufSize; ++i) {
                        int x0 = (int)((float)i * _deltaSample);
                        int x1 = ((x0 + 1) < (_getBufSize - 1)) ? (x0 + 1) : (_getBufSize - 1);
                        float y = linear(x0, data[channel][x0], x0 + 1, data[channel][x1], ((float)i * _deltaSample));
                        ECHECK_APP_THREAD(scaleAmplitudeChannel((rpApp_osc_source) channel, y, view + ((channel * viewSize) + i + _preZero)));
                    }
                } else {
                    for (int i = 0; i < viewSize-_postZero && (int) ((float)i * _deltaSample) < _getBufSize; ++i) {
                        ECHECK_APP_THREAD(scaleAmplitudeChannel((rpApp_osc_source) channel, data[channel][(int) ((float)i * _deltaSample)], view + ((channel * viewSize) + i + _preZero)));
                    }
                }
            }

            viewStartPos = 0;
            viewEndPos = viewSize;

            mathThreadFunction();
            pthread_mutex_unlock(&mutex);

            manuallyTriggered = false;
            checkAutoscale(true);
        }

        if (thisLoopAcqStart) {
            waitToFillPreTriggerBuffer(true);
            ECHECK_APP_THREAD(osc_setTriggerSource(trigSource));
        }
    }
}
