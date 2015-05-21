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
#include "common.h"
#include "../../rpbase/src/common.h"

bool auto_freRun_mode = 0;
bool acqRunning = false;
bool clear = false;
uint32_t viewSize = VIEW_SIZE_DEFAULT;
float *view;
float ch1_ampOffset, ch2_ampOffset, math_ampOffset;
float ch1_ampScale,  ch2_ampScale,  math_ampScale;
float ch1_probeAtt, ch2_probeAtt;
float timeScale=1, timeOffset=0;
rpApp_osc_trig_sweep_t trigSweep;
rpApp_osc_trig_source_t trigSource = RPAPP_OSC_TRIG_SRC_CH1;
rpApp_osc_trig_slope_t trigSlope = RPAPP_OSC_TRIG_SLOPE_PE;
rpApp_osc_math_oper_t operation;
rp_channel_t mathSource1, mathSource2;

float samplesPerDivision = (float) VIEW_SIZE_DEFAULT / (float) DIVISIONS_COUNT_X;

pthread_t mainThread = (pthread_t) -1;
pthread_mutex_t mutex;


int osc_Init() {
    pthread_mutex_init(&mutex, NULL);
    view = calloc(3 * viewSize, sizeof(float));
    if (view == NULL) {
        free(view);
        view = NULL;
        return RP_EAA;
    }
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
    ECHECK_APP(osc_setTimeScale(1));
    ECHECK_APP(osc_setMathOperation(RPAPP_OSC_MATH_NONE));
    ECHECK_APP(osc_setMathSources(RP_CH_1, RP_CH_2));

    return RP_OK;
}

int osc_run() {
    clearView();
    ECHECK_APP(threadSafe_acqStart());
    ECHECK_APP(osc_setTriggerSource(trigSource));
    START_THREAD(mainThread, mainThreadFun);
    return RP_OK;
}

int osc_stop() {
    ECHECK_APP(threadSafe_acqStop());
    return RP_OK;
}

int osc_reset() {
    clearView();
    STOP_THREAD(mainThread);
    ECHECK_APP(threadSafe_acqStop());

    ECHECK_APP(osc_SetDefaultValues());
    return RP_OK;
}

int osc_single() {
    if (trigSweep != RPAPP_OSC_TRIG_SINGLE) {
        ECHECK_APP(osc_setTriggerSweep(RPAPP_OSC_TRIG_SINGLE));
    }
    ECHECK_APP(threadSafe_acqStart());
    waitToFillPreTriggerBuffer();
    ECHECK_APP(osc_setTriggerSource(trigSource));
    return RP_OK;
}

int osc_autoScale() {
    float period, vpp, vMean;
    bool isAutoScaled = false;

    for (rpApp_osc_source source = RPAPP_OSC_SOUR_CH1; source <= RPAPP_OSC_SOUR_CH2; ++source) {
        ECHECK_APP(osc_measureVpp(source, &vpp));
        ECHECK_APP(osc_measureMeanVoltage(source, &vMean));

        // If there is signal on input
        if (fabs(vpp) > SIGNAL_EXISTENCE) {
            ECHECK_APP(osc_setAmplitudeOffset(source, vMean));
            // Calculate scale
            float scale = (float) (vpp * AUTO_SCALE_AMP_SCA_FACTOR / DIVISIONS_COUNT_Y * (source == RPAPP_OSC_SOUR_CH1 ? ch1_probeAtt : ch2_probeAtt));
            ECHECK_APP(osc_setAmplitudeScale(source, roundUpTo125(scale)));

            if (!isAutoScaled) {
                // set time scale only based on one channel
                ECHECK_APP(osc_measurePeriod(source, &period));
                ECHECK_APP(osc_setTimeOffset(AUTO_SCALE_TIME_OFFSET));
                ECHECK_APP(osc_setTimeScale(period * AUTO_SCALE_PERIOD_COUNT / DIVISIONS_COUNT_X));
                isAutoScaled = true;
            }
        }
    }
    if (isAutoScaled) {
        return RP_OK;
    }
    else {
        return RP_APP_ENS;
    }
}

int osc_isRunning(bool *running) {
    *running = acqRunning;
    return RP_OK;
}

int osc_setTimeScale(float scale) {
    rp_acq_decimation_t decimation;
    float constant = (float)viewSize * 125000000.0f * timeScale / 1000.0f / samplesPerDivision / (float)ADC_BUFFER_SIZE;
    // contition:
    // sampleRate / samplesPerDivision * viewSize < TIME_SCALE_RATIO_THRESHOLD

    if (constant / 1.0f < TIME_SCALE_RATIO_THRESHOLD) {
        decimation = RP_DEC_1;
    }
    else if (constant / 8.0f < TIME_SCALE_RATIO_THRESHOLD) {
        decimation = RP_DEC_8;
    }
    else if (constant / 64.0f < TIME_SCALE_RATIO_THRESHOLD) {
        decimation = RP_DEC_64;
    }
    else if (constant / 1024.0f < TIME_SCALE_RATIO_THRESHOLD) {
        decimation = RP_DEC_1024;
    }
    else if (constant / 8192.0f < TIME_SCALE_RATIO_THRESHOLD) {
        decimation = RP_DEC_8192;
    }
    else {
        decimation = RP_DEC_65536;
    }

    pthread_mutex_lock(&mutex);
    clearView();
    timeScale = scale;
    ECHECK_APP(rp_AcqSetDecimation(decimation))
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
    clearView();
    timeOffset = offset;
    ECHECK_APP(rp_AcqSetTriggerDelayNs((int64_t)(offset * MILLI_TO_NANO)));
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
    EXECUTE_ATOMICALLY(mutex, clearView());
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
    clearView();
    switch (gain) {
        case RPAPP_OSC_IN_GAIN_LV:
            rp_AcqSetGain(channel, RP_LOW);
            break;
        case RPAPP_OSC_IN_GAIN_HV:
            rp_AcqSetGain(channel, RP_HIGH);
            break;
        default:
            return RP_EOOR;
    }
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

int osc_setAmplitudeScale(rpApp_osc_source source, float scale) {
    SOURCE_ACTION(source,
                  ch1_ampScale = scale,
                  ch2_ampScale = scale,
                  math_ampScale = scale)
    EXECUTE_ATOMICALLY(mutex, clearView());
    return RP_OK;
}

int osc_getAmplitudeScale(rpApp_osc_source source, float *scale) {
    SOURCE_ACTION(source,
                  *scale = ch1_ampScale,
                  *scale = ch2_ampScale,
                  *scale = math_ampScale)
    return RP_OK;
}

int osc_setAmplitudeOffset(rpApp_osc_source source, float offset) {
    SOURCE_ACTION(source,
                  ch1_ampOffset = offset,
                  ch2_ampOffset = offset,
                  math_ampOffset = offset)
    EXECUTE_ATOMICALLY(mutex, clearView());
    return RP_OK;
}

int osc_getAmplitudeOffset(rpApp_osc_source source, float *offset) {
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
    ECHECK_APP(rp_AcqSetTriggerSrc(src));
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
    ECHECK_APP(rp_AcqSetTriggerSrc(src));
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

int osc_getTriggerSlope(rpApp_osc_trig_slope_t *slope) {
    *slope = trigSlope;
    return RP_OK;
}

int osc_setTriggerLevel(float level) {
    pthread_mutex_lock(&mutex);
    clearView();
    ECHECK_APP(rp_AcqSetTriggerLevel(level));
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

int osc_getTriggerLevel(float *level) {
    return rp_AcqGetTriggerLevel(level);
}

int osc_setTriggerSweep(rpApp_osc_trig_sweep_t sweep) {
    EXECUTE_ATOMICALLY(mutex, clearView());
    switch (sweep) {
        case RPAPP_OSC_TRIG_NORMAL:
        case RPAPP_OSC_TRIG_SINGLE:
            break;
        case RPAPP_OSC_TRIG_AUTO:
            EXECUTE_ATOMICALLY(mutex, auto_freRun_mode = false)
            break;
        default:
            return RP_EOOR;
    }
    trigSweep = sweep;
    return RP_OK;
}

int osc_getTriggerSweep(rpApp_osc_trig_sweep_t *sweep) {
    *sweep = trigSweep;
    return RP_OK;
}

int osc_getViewPos(float *positionRatio) {
    int32_t tmp;
    ECHECK_APP(rp_AcqGetTriggerDelay(&tmp));
    *positionRatio = 0.5f + (float)(tmp % ADC_BUFFER_SIZE) / ADC_BUFFER_SIZE;
    return RP_OK;
}

int osc_getViewPart(float *ratio) {
    *ratio = ((float)viewSize * (float)timeToIndex(timeScale) / samplesPerDivision) / (float)ADC_BUFFER_SIZE;
    return RP_OK;
}

int osc_measureVpp(rpApp_osc_source source, float *Vpp) {
    float resMax, resMin, max = FLT_MIN, min = FLT_MAX;
    for (int i = 0; i < viewSize; ++i) {
        if (view[source*viewSize + i] > max) {
            max = view[source*viewSize + i];
        }
        if (view[source*viewSize + i] < min) {
            min = view[source*viewSize + i];
        }
    }
    resMax = unscaleAmplitudeChannel(source, max);
    resMin = unscaleAmplitudeChannel(source, min);
    *Vpp = resMax - resMin;
    return RP_OK;
}

int osc_measureMeanVoltage(rpApp_osc_source source, float *meanVoltage) {
    float sum = 0;
    for (int i = 0; i < viewSize; ++i) {
        sum += view[source*viewSize + i];
    }
    *meanVoltage = unscaleAmplitudeChannel(source, sum / viewSize);
    return RP_OK;
}

int osc_measureMaxVoltage(rpApp_osc_source source, float *Vmax) {
    float max = FLT_MIN;
    for (int i = 0; i < viewSize; ++i) {
        if (view[source*viewSize + i] > max) {
            max = view[source*viewSize + i];
        }
    }
    *Vmax = unscaleAmplitudeChannel(source, max);
    return RP_OK;
}

int osc_measureMinVoltage(rpApp_osc_source source, float *Vmin) {
    float min = FLT_MAX;
    for (int i = 0; i < viewSize; ++i) {
        if (view[source*viewSize + i] < min) {
            min = view[source*viewSize + i];
        }
    }
    *Vmin = unscaleAmplitudeChannel(source, min);
    return RP_OK;
}

int osc_measureFrequency(rpApp_osc_source source, float *frequency) {
    float period;
    ECHECK_APP(osc_measurePeriod(source, &period));
    *frequency = (float) (1 / (period / 1000.0));
    return RP_OK;
}

int osc_measurePeriod(rpApp_osc_source source, float *period) {
    uint32_t dataSize = source == RPAPP_OSC_SOUR_MATH ? viewSize : ADC_BUFFER_SIZE;
    float data[dataSize];
    pthread_mutex_lock(&mutex);
    if (source == RPAPP_OSC_SOUR_MATH) {
        for (int i = 0; i < dataSize; ++i) {
            data[i] = view[RPAPP_OSC_SOUR_MATH*viewSize + i];
        }
    } else {
        ECHECK_APP(rp_AcqGetLatestDataV((rp_channel_t)source, &dataSize, data));
    }
    pthread_mutex_unlock(&mutex);

    float mean = 0;
    for (int i = 0; i < dataSize; ++i) {
        mean += data[i];
    }
    mean = mean / dataSize;

    int preTrig_P = -1, preTrig_N = -1;
    int periods[PERIOD_STORAGE_PERIOD_COUNT];
    int nextPeriod = 0;

    for (int i = 1; i < dataSize; ++i) {
        if (data[i - 1] < mean && data[i] >= mean) {
            if (preTrig_P != -1 && nextPeriod < PERIOD_STORAGE_PERIOD_COUNT) {
                periods[nextPeriod++] = i - preTrig_P;
            }
            preTrig_P = i;
        }
        if (data[i - 1] > mean && data[i] <= mean) {
            if (preTrig_N != -1 && nextPeriod < PERIOD_STORAGE_PERIOD_COUNT) {
                periods[nextPeriod++] = i - preTrig_N;
            }
            preTrig_N = i;
        }
    }
    qsort(periods, (size_t) nextPeriod, sizeof(int), intCmp);
    *period = indexToTime(periods[nextPeriod/2]);
    return RP_OK;
}

int osc_measureDutyCycle(rpApp_osc_source source, float *dutyCycle) {
    int highTime = 0;
    float meanValue;
    ECHECK_APP(osc_measureMeanVoltage(source, &meanValue));
    for (int i = 0; i < viewSize; ++i) {
        if (view[source*viewSize + i] > meanValue) {
            ++highTime;
        }
    }
    *dutyCycle = (float)highTime / (float)viewSize;
    return RP_OK;
}

int osc_measureRootMeanSquare(rpApp_osc_source source, float *rms) {
    float rmsValue = 0;
    for (int i = 0; i < viewSize; ++i) {
        rmsValue += view[source*viewSize + i] * view[source*viewSize + i];
    }
    *rms = (float) sqrt(rmsValue / viewSize);
    return RP_OK;
}

int osc_getCursorVoltage(rpApp_osc_source source, uint32_t cursor, float *value) {
    *value = unscaleAmplitudeChannel(source, view[source*viewSize + cursor]);
    return RP_OK;
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
    operation = op;
    EXECUTE_ATOMICALLY(mutex, clearMath())
    return RP_OK;
}

int osc_getMathOperation(rpApp_osc_math_oper_t *op) {
    *op = operation;
    return RP_OK;
}

int osc_setMathSources(rp_channel_t source1, rp_channel_t source2) {
    mathSource1 = source1;
    mathSource2 = source2;
    EXECUTE_ATOMICALLY(mutex, clearMath())
    return RP_OK;
}

int osc_getMathSources(rp_channel_t *source1, rp_channel_t *source2) {
    *source1 = mathSource1;
    *source2 = mathSource2;
    return RP_OK;
}

int osc_getData(rpApp_osc_source source, float *data, uint32_t size) {
    for (int i = 0; i < size; ++i) {
        data[i] = view[source*viewSize + i];
    }
    return RP_OK;
}

int osc_setViewSize(uint32_t size) {
    viewSize = size;
    samplesPerDivision = (float) viewSize / (float) DIVISIONS_COUNT_X;
    view = realloc(view, 3 * viewSize * sizeof(float));
    if (view == NULL) {
        free(view);
        view = NULL;
        return RP_EAA;
    }
    EXECUTE_ATOMICALLY(mutex, clearView());
    return RP_OK;
}

int osc_getViewSize(uint32_t *size) {
    *size = viewSize;
    return 0;
}

int osc_getInvData(rpApp_osc_source source, float *data, uint32_t size){
    for(int i = 0; i < size; i++){
        data[i] = -1 * (view[source*viewSize + i]);
    }
    return RP_OK;
}

/*
* Utils
*/

int threadSafe_acqStart() {
    pthread_mutex_lock(&mutex);
    ECHECK_APP(rp_AcqStart())
    acqRunning = true;
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

int threadSafe_acqStop() {
    pthread_mutex_lock(&mutex);
    ECHECK_APP(rp_AcqStop())
    acqRunning = false;
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

float scaleAmplitude(float volts, float ampScale, float probeAtt, float ampOffset) {
    return (volts + ampOffset) * probeAtt / ampScale;
}

float unscaleAmplitude(float value, float ampScale, float probeAtt, float ampOffset) {
    return (value * ampScale / probeAtt) - ampOffset;
}

float scaleAmplitudeChannel(rp_channel_t channel, float volts) {
    float ampOffset, ampScale, probeAtt;
    osc_getAmplitudeOffset((rpApp_osc_source) channel, &ampOffset);
    osc_getAmplitudeScale((rpApp_osc_source) channel, &ampScale);
    osc_getProbeAtt(channel, &probeAtt);
    return scaleAmplitude(volts, ampScale, probeAtt, ampOffset);
}

float unscaleAmplitudeChannel(rpApp_osc_source source, float value) {
    float ampOffset, ampScale, probeAtt=1;
    osc_getAmplitudeOffset(source, &ampOffset);
    osc_getAmplitudeScale(source, &ampScale);
    if (source != RPAPP_OSC_SOUR_MATH)
        osc_getProbeAtt((rp_channel_t)source, &probeAtt);
    return unscaleAmplitude(value, ampScale, probeAtt, ampOffset);
}

float viewIndexToTime(int index) {
    return indexToTime(index - viewSize / 2) + timeOffset;
}

void calculateIntegral(rp_channel_t channel, float scale, float offset) {
    float dt = timeScale / samplesPerDivision;
    view[RPAPP_OSC_SOUR_MATH*viewSize] = view[channel*viewSize]* dt * scale + offset;
    for (int i = 1; i < viewSize; ++i) {
        view[RPAPP_OSC_SOUR_MATH*viewSize + i] =
                (view[RPAPP_OSC_SOUR_MATH*viewSize + i-1] + (view[channel*viewSize + i]* (dt))) * scale + offset;
    }
}

void calculateDevivative(rp_channel_t channel, float scale, float offset) {
    double dt2 = 2*timeScale / samplesPerDivision;
    view[RPAPP_OSC_SOUR_MATH*viewSize] =
            (float) ((view[channel*viewSize+1] - view[channel*viewSize]) / dt2 / 2 * scale + offset);
    for (int i = 1; i < viewSize - 1; ++i) {
        view[RPAPP_OSC_SOUR_MATH*viewSize + i] =
                (float) ((view[channel*viewSize + i+1] - view[channel*viewSize + i-1]) / dt2 * scale + offset);
    }
}

float calculateMath(float v1, float v2, rpApp_osc_math_oper_t op, float scale, float offset) {
    switch (op) {
        case RPAPP_OSC_MATH_ADD:
            return (v1 + v2) * scale + offset;
        case RPAPP_OSC_MATH_SUB:
            return (v1 - v2) * scale + offset;
        case RPAPP_OSC_MATH_MUL:
            return (v1 * v2) * scale + offset;
        case RPAPP_OSC_MATH_DIV:
            return (v1 / v2) * scale + offset;
        case RPAPP_OSC_MATH_ABS:
            return (float) (fabs(v1) * scale + offset);
        default:
            return 0;
    }
}

float roundUpTo125(float data) {
    double power = ceil(log(data) / log(10)) - 1;       // calculate normalization factor
    double dataNorm = data / pow(10, power);            // normalize data, so that 1 < data < 10
    if (dataNorm < 2)                                   // round normalized data
        dataNorm = 2;
    else if (dataNorm < 5)
        dataNorm = 5;
    else
        dataNorm = 10;
    return (float) (dataNorm * pow(10, power));         // unnormalize data
}

void clearView() {
    int size = 3*viewSize;
    for (int i = 0; i < size; ++i) {
        view[i] = 0;
    }
    clear = true;
}

void clearMath() {
    for (int i = 0; i < viewSize; ++i) {
        view[RPAPP_OSC_SOUR_MATH*viewSize + i] = 0;
    }
}

void waitToFillPreTriggerBuffer() {
    float deltaSample, timeScale;
    uint32_t preTriggerCount;
    int triggerDelay;
    clock_t timer = clock();
    do {
        ECHECK_APP(rp_AcqGetTriggerDelay(&triggerDelay));
        ECHECK_APP(rp_AcqGetPreTriggerCounter(&preTriggerCount));
        ECHECK_APP(osc_getTimeScale(&timeScale));
        deltaSample = timeToIndex(timeScale) / samplesPerDivision;
    } while (preTriggerCount < viewSize/2*deltaSample - triggerDelay && clock() - timer < WAIT_TO_FILL_BUF_TIMEOUT);
}

/*
* Thread functions
*/

void mathThreadFunction() {
    if (operation != RPAPP_OSC_MATH_NONE) {
        if (operation == RPAPP_OSC_MATH_DER) {
            calculateDevivative(mathSource1, math_ampScale, math_ampOffset);
        } else if (operation == RPAPP_OSC_MATH_INT) {
            calculateIntegral(mathSource1, math_ampScale, math_ampOffset);
        } else {
            for (int i = 0; i < viewSize; ++i) {
                view[RPAPP_OSC_SOUR_MATH*viewSize + i] =
                        calculateMath(view[mathSource1*viewSize + i], view[mathSource2*viewSize + i],
                                      operation, math_ampScale, math_ampOffset);
            }
        }
    }
}

void *mainThreadFun() {
    rp_acq_trig_src_t _triggerSource;
    rp_acq_trig_state_t _state;
    double _timer = clock();
    uint32_t _triggerPosition, _getBufSize, _startIndex, _writePointer = 0, _preTriggerCount = 0;
    int _triggerDelay, _preZero, _postZero;
    float _deltaSample, _timeScale;
    float data[2][ADC_BUFFER_SIZE];
    bool thisLoopAcqStart = false;

    while (true) {
        thisLoopAcqStart = false;

        if (clear) {
            ECHECK_APP(rp_AcqSetTriggerSrc(RP_TRIG_SRC_DISABLED));
            threadSafe_acqStart();
            waitToFillPreTriggerBuffer();
            thisLoopAcqStart = false;
            ECHECK_APP(osc_setTriggerSource(trigSource));
            EXECUTE_ATOMICALLY(mutex, auto_freRun_mode = false)
            EXECUTE_ATOMICALLY(mutex, clear = false)
        }


        ECHECK_APP(rp_AcqGetTriggerState(&_state));
        ECHECK_APP(rp_AcqGetTriggerSrc(&_triggerSource));

        // If in auto mode end trigger timed out
        if (trigSweep == RPAPP_OSC_TRIG_AUTO && !auto_freRun_mode && (clock() - _timer) / CLOCKS_PER_SEC > AUTO_TRIG_TIMEOUT) {
            rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);
            EXECUTE_ATOMICALLY(mutex, auto_freRun_mode = true)
        }

        ECHECK_APP(osc_getTimeScale(&_timeScale));

        if ((_state == RP_TRIG_STATE_TRIGGERED && _timeScale >= MIN_TIME_TO_DRAW_BEFORE_TIG ) || _triggerSource == RP_TRIG_SRC_DISABLED) {
            // Read parameters
            ECHECK_APP(rp_AcqGetWritePointer(&_writePointer));
            ECHECK_APP(rp_AcqGetWritePointerAtTrig(&_triggerPosition));
            ECHECK_APP(rp_AcqGetTriggerDelay(&_triggerDelay));

            ECHECK_APP(rp_AcqGetPreTriggerCounter(&_preTriggerCount));

            // Calculate transformation (form data to view) parameters
            _deltaSample = timeToIndex(_timeScale) / samplesPerDivision;
            _triggerDelay = _triggerDelay % ADC_BUFFER_SIZE;

            _preZero = (int) MAX(0, viewSize/2 - (_triggerDelay+_preTriggerCount)/_deltaSample);
            _postZero = (int) MAX(0, viewSize/2 - (_writePointer-(_triggerPosition+_triggerDelay))/_deltaSample);
            _startIndex = (_triggerPosition + _triggerDelay - (uint32_t) ((viewSize/2 -_preZero)*_deltaSample)) % ADC_BUFFER_SIZE;
            _getBufSize = (viewSize-(_preZero + _postZero))*_deltaSample;

            // Get data
            ECHECK_APP(rp_AcqGetDataV(RP_CH_1, _startIndex, &_getBufSize, data[0]));
            ECHECK_APP(rp_AcqGetDataV(RP_CH_2, _startIndex, &_getBufSize, data[1]));

            if (trigSweep != RPAPP_OSC_TRIG_SINGLE && _triggerSource == RP_TRIG_SRC_DISABLED) {
                threadSafe_acqStart();
                thisLoopAcqStart = true;
            }

            // Reset autoSweep timer
            if (trigSweep == RPAPP_OSC_TRIG_AUTO) {
                _timer = clock();
                EXECUTE_ATOMICALLY(mutex, auto_freRun_mode = false)
            }

            // Write data to view buffer
            for (rp_channel_t channel = RP_CH_1; channel <= RP_CH_2; ++channel) {
                // first preZero data are wrong - from previout trigger. Last preZero data hasent been overwritten
                for (int i = 0; i < _preZero; ++i) {
                    view[channel * viewSize + i] = 0;
                }
                for (int i = 0; i < viewSize-_postZero && (int) (i * _deltaSample) < _getBufSize; ++i) {
                    view[channel * viewSize + i+_preZero] = scaleAmplitudeChannel(channel, data[channel][(int) (i * _deltaSample)]);
                }
            }

            mathThreadFunction();
        }


        if (thisLoopAcqStart) {
            waitToFillPreTriggerBuffer();
            ECHECK_APP(osc_setTriggerSource(trigSource));
        }
    }
    return RP_OK;
}
