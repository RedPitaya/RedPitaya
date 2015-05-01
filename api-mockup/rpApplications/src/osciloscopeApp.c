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
uint32_t viewSize = VIEW_SIZE_DEFAULT;
float *view;
float ch1_ampOffset, ch2_ampOffset;
float ch1_ampScale, ch2_ampScale;
float ch1_probeAtt, ch2_probeAtt;
float timeScale=1, timeOffset=0;
rpApp_osc_trig_sweep_t trigSweep;
rpApp_osc_trig_source_t trigSource = RPAPP_OSC_TRIG_SRC_CH1;
rpApp_osc_trig_slope_t trigSlope = RPAPP_OSC_TRIG_SLOPE_PE;
float samplesPerDivision = (float) VIEW_SIZE_DEFAULT / (float) DIVISIONS_COUNT_X;

pthread_t mainThread = (pthread_t) -1;
pthread_mutex_t mutex;


int osc_Init() {
    pthread_mutex_init(&mutex, NULL);
    view = malloc(2 * viewSize * sizeof(float));
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
    ECHECK_APP(osc_setAmplitudeOffset(RP_CH_1, 0));
    ECHECK_APP(osc_setAmplitudeOffset(RP_CH_2, 0));
    ECHECK_APP(osc_setAmplitudeScale(RP_CH_1, 1));
    ECHECK_APP(osc_setAmplitudeScale(RP_CH_2, 1));
    ECHECK_APP(osc_setProbeAtt(RP_CH_1, 10));
    ECHECK_APP(osc_setProbeAtt(RP_CH_2, 10));
    ECHECK_APP(osc_setInputGain(RP_CH_1, RPAPP_OSC_IN_GAIN_LV))
    ECHECK_APP(osc_setInputGain(RP_CH_2, RPAPP_OSC_IN_GAIN_LV))
    ECHECK_APP(osc_setTimeOffset(0));
    ECHECK_APP(osc_setTriggerSlope(RPAPP_OSC_TRIG_SLOPE_PE));
    ECHECK_APP(rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE));
    ECHECK_APP(osc_setTriggerLevel(0));
    ECHECK_APP(osc_setTriggerSweep(RPAPP_OSC_TRIG_AUTO));
    ECHECK_APP(osc_setTimeScale(1));
    return RP_OK;
}

int osc_run() {
    ECHECK_APP(threadSafe_acqStart());
    ECHECK_APP(osc_setTriggerSource(trigSource));

    bool single = false;
    START_THREAD(mainThread, mainThreadFun, &single);
    return RP_OK;
}

int osc_stop() {
    ECHECK_APP(threadSafe_acqStop());
    return RP_OK;
}

int osc_reset() {
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
    // Sleep thet FGPA acquires some data
    usleep(1000);
    ECHECK_APP(osc_setTriggerSource(trigSource));
    return RP_OK;
}

int osc_autoScale() {
    float period, vMax, vMin, vMean;
    bool isAutoScaled = false;

    for (rp_channel_t channel = RP_CH_1; channel <= RP_CH_2; ++channel) {
        ECHECK_APP(osc_measureMinVoltage(channel, &vMin));
        ECHECK_APP(osc_measureMaxVoltage(channel, &vMax));
        ECHECK_APP(osc_measureMeanVoltage(channel, &vMean));

        // If there is signal on input
        if (fabs(vMin) > SIGNAL_EXISTENCE || fabs(vMax) > SIGNAL_EXISTENCE) {
            ECHECK_APP(osc_setAmplitudeOffset(channel, vMean));
            // Calculate scale
            float scale = (float) (1/((vMax - vMin) * AUTO_SCALE_AMP_SCA_FACTOR * DIVISIONS_COUNT_Y / 2) * (channel == RP_CH_1 ? ch1_probeAtt : ch2_probeAtt));
            ECHECK_APP(osc_setAmplitudeScale(channel, scale));

            if (!isAutoScaled) {
                // set time scale only based on one channel
                ECHECK_APP(osc_measurePeriod(channel, &period));
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

int osc_setTimeScale(float scale) {
    rp_acq_decimation_t decimation;
    float oscilloscopeConstant = viewSize / DIVISIONS_COUNT_X;

    if (scale < TIME_SCALE_FOR_DIV_1 * oscilloscopeConstant) {
        decimation = RP_DEC_1;
    }
    else if (scale < TIME_SCALE_FOR_DIV_8 * oscilloscopeConstant) {
        decimation = RP_DEC_8;
    }
    else if (scale < TIME_SCALE_FOR_DIV_64 * oscilloscopeConstant) {
        decimation = RP_DEC_64;
    }
    else if (scale < TIME_SCALE_FOR_DIV_1024 * oscilloscopeConstant) {
        decimation = RP_DEC_1024;
    }
    else if (scale < TIME_SCALE_FOR_DIV_8192 * oscilloscopeConstant) {
        decimation = RP_DEC_8192;
    }
    else {
        decimation = RP_DEC_65536;
    }

    pthread_mutex_lock(&mutex);
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

int osc_setAmplitudeScale(rp_channel_t channel, float scale) {
    CHANNEL_ACTION(channel,
                   ch1_ampScale = scale,
                   ch2_ampScale = scale)
    return RP_OK;
}

int osc_getAmplitudeScale(rp_channel_t channel, float *scale) {
    CHANNEL_ACTION(channel,
                   *scale = ch1_ampScale,
                   *scale = ch2_ampScale)
    return RP_OK;
}

int osc_setAmplitudeOffset(rp_channel_t channel, float offset) {
    CHANNEL_ACTION(channel,
                   ch1_ampOffset = offset,
                   ch2_ampOffset = offset)
    return RP_OK;
}

int osc_getAmplitudeOffset(rp_channel_t channel, float *offset) {
    CHANNEL_ACTION(channel,
                   *offset = ch1_ampOffset,
                   *offset = ch2_ampOffset)
    return RP_OK;}

int osc_setTriggerSource(rpApp_osc_trig_source_t triggerSource) {
    pthread_mutex_lock(&mutex);
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
    ECHECK_APP(rp_AcqSetTriggerLevel(level));
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

int osc_getTriggerLevel(float *level) {
    return rp_AcqGetTriggerLevel(level);
}

int osc_setTriggerSweep(rpApp_osc_trig_sweep_t sweep) {
    switch (sweep) {
        case RPAPP_OSC_TRIG_SINGLE:
            break;
        case RPAPP_OSC_TRIG_AUTO:
            pthread_mutex_lock(&mutex);
            auto_freRun_mode = false;
            pthread_mutex_unlock(&mutex);
        case RPAPP_OSC_TRIG_NORMAL:
            // If previous sweep was SINLEGLE
            if (trigSweep == RPAPP_OSC_TRIG_SINGLE) {
                ECHECK_APP(threadSafe_acqStart());
                ECHECK_APP(osc_setTriggerSource(trigSource))
            }
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

int osc_measureVpp(rp_channel_t channel, float *Vpp) {
    float resMax, resMin, max = FLT_MIN, min = FLT_MAX;
    for (int i = 0; i < viewSize; ++i) {
        if (view[channel*viewSize + i] > max) {
            max = view[channel*viewSize + i];
        }
        if (view[channel*viewSize + i] < min) {
            min = view[channel*viewSize + i];
        }
    }
    resMax = unscaleAmplitudeChannel(channel, max);
    resMin = unscaleAmplitudeChannel(channel, min);
    *Vpp = resMax - resMin;
    return RP_OK;
}

int osc_measureMeanVoltage(rp_channel_t channel, float *meanVoltage) {
    float sum = 0;
    for (int i = 0; i < viewSize; ++i) {
        sum += view[channel*viewSize + i];
    }
    *meanVoltage = unscaleAmplitudeChannel(channel, sum / viewSize);
    return RP_OK;
}

int osc_measureMaxVoltage(rp_channel_t channel, float *Vmax) {
    float max = FLT_MIN;
    for (int i = 0; i < viewSize; ++i) {
        if (view[channel*viewSize + i] > max) {
            max = view[channel*viewSize + i];
        }
    }
    *Vmax = unscaleAmplitudeChannel(channel, max);
    return RP_OK;
}

int osc_measureMinVoltage(rp_channel_t channel, float *Vmin) {
    float min = FLT_MAX;
    for (int i = 0; i < viewSize; ++i) {
        if (view[channel*viewSize + i] < min) {
            min = view[channel*viewSize + i];
        }
    }
    *Vmin = unscaleAmplitudeChannel(channel, min);
    return RP_OK;
}

int osc_measureFrequency(rp_channel_t channel, float *frequency) {
    float period;
    ECHECK_APP(osc_measurePeriod(channel, &period));
    *frequency = (float) (1 / (period / 1000.0));
    return RP_OK;
}

int osc_measurePeriod(rp_channel_t channel, float *period) {
    uint32_t dataSize = ADC_BUFFER_SIZE;
    float data[dataSize];
    pthread_mutex_lock(&mutex);
    ECHECK_APP(rp_AcqGetLatestDataV(channel, &dataSize, data));
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

int osc_measureDutyCycle(rp_channel_t channel, float *dutyCycle) {
    int highTime = 0;
    float meanValue;
    ECHECK_APP(osc_measureMeanVoltage(channel, &meanValue));
    for (int i = 0; i < viewSize; ++i) {
        if (view[channel*viewSize + i] > meanValue) {
            ++highTime;
        }
    }
    *dutyCycle = (float)highTime / (float)viewSize;
    return RP_OK;
}

int osc_measureRootMeanSquare(rp_channel_t channel, float *rms) {
    float rmsValue = 0;
    for (int i = 0; i < viewSize; ++i) {
        rmsValue += view[channel*viewSize + i] * view[channel*viewSize + i];
    }
    *rms = (float) sqrt(rmsValue / viewSize);
    return RP_OK;
}

int osc_getCursorVoltage(rp_channel_t channel, uint32_t cursor, float *value) {
    *value = unscaleAmplitudeChannel(channel, view[channel*viewSize + cursor]);
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

int oscGetCursorDeltaAmplitude(rp_channel_t channel, uint32_t cursor1, uint32_t cursor2, float *value) {
    if (cursor1 < 0 || cursor1 >= viewSize || cursor2 < 0 || cursor2 >= viewSize) {
        return RP_EOOR;
    }
    float cursor1Amplitude, cursor2Amplitude;
    ECHECK_APP(osc_getCursorVoltage(channel, cursor1, &cursor1Amplitude));
    ECHECK_APP(osc_getCursorVoltage(channel, cursor2, &cursor2Amplitude));
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

int osc_getViewData(rp_channel_t channel, float *data, uint32_t size) {
    for (int i = 0; i < size; ++i) {
        data[i] = view[channel*viewSize + i];
    }
    return RP_OK;
}

int osc_setViewSize(uint32_t size) {
    viewSize = size;
    samplesPerDivision = (float) viewSize / (float) DIVISIONS_COUNT_X;
    view = realloc(view, 2 * viewSize * sizeof(float));
    if (view == NULL) {
        free(view);
        view = NULL;
        return RP_EAA;
    }
    return RP_OK;
}

int osc_getViewSize(uint32_t *size) {
    *size = viewSize;
    return 0;
}

int osc_getInvViewData(rp_channel_t channel, float *data, uint32_t size){
    for(int i = 0; i < size; i++){
        data[i] = -1 * (view[channel*viewSize + i]);
    }
    return RP_OK;
}

/*
* Utils
*/

int threadSafe_acqStart() {
    pthread_mutex_lock(&mutex);
    ECHECK_APP(rp_AcqStart())
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

int threadSafe_acqStop() {
    pthread_mutex_lock(&mutex);
    ECHECK_APP(rp_AcqStop())
    pthread_mutex_unlock(&mutex);
    return RP_OK;
}

float scaleAmplitude(float volts, float ampScale, float probeAtt, float ampOffset) {
    return (volts + ampOffset) * probeAtt / ampScale ;
}

float unscaleAmplitude(float value, float ampScale, float probeAtt, float ampOffset) {
    return (value * ampScale / probeAtt) - ampOffset;
}

float scaleAmplitudeChannel(rp_channel_t channel, float volts) {
    float ampOffset, ampScale, probeAtt;
    osc_getAmplitudeOffset(channel, &ampOffset);
    osc_getAmplitudeScale(channel, &ampScale);
    osc_getProbeAtt(channel, &probeAtt);
    return scaleAmplitude(volts, ampScale, probeAtt, ampOffset);
}

float unscaleAmplitudeChannel(rp_channel_t channel, float value) {
    float ampOffset, ampScale, probeAtt;
    osc_getAmplitudeOffset(channel, &ampOffset);
    osc_getAmplitudeScale(channel, &ampScale);
    osc_getProbeAtt(channel, &probeAtt);
    return unscaleAmplitude(value, ampScale, probeAtt, ampOffset);
}

float viewIndexToTime(int index) {
    return indexToTime(index - viewSize / 2) + timeOffset;
}


/*
* Thread function
*/
void *mainThreadFun(void *arg) {
    rp_acq_trig_state_t state, preState = RP_TRIG_STATE_TRIGGERED;
    double timer = clock();
    uint32_t buffSize = ADC_BUFFER_SIZE, _triggerPosition, getBufSize, startIndex, _writePointer = 0, _preWritePointer = 0;
    int _triggerDelay;
    float deltaSample, _timeOffset, _timeScale;
    float data[2][buffSize];

    do {
        rp_AcqGetTriggerState(&state);

        // If in auto mode end trigger timed out
        if (trigSweep == RPAPP_OSC_TRIG_AUTO && !auto_freRun_mode && (clock() - timer) / CLOCKS_PER_SEC > AUTO_TRIG_TIMEOUT) {
            rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);
            pthread_mutex_lock(&mutex);
            auto_freRun_mode = true;
            pthread_mutex_unlock(&mutex);
        }

        if (preState == RP_TRIG_STATE_WAITING && state == RP_TRIG_STATE_TRIGGERED) {
            // Read parameters
            rp_AcqGetWritePointerAtTrig(&_triggerPosition);
            rp_AcqGetTriggerDelay(&_triggerDelay);
            rp_AcqGetWritePointer(&_writePointer);
            osc_getTimeScale(&_timeScale);
            osc_getTimeOffset(&_timeOffset);

            // Calculate transformation (form data to view) parameters
            deltaSample = timeToIndex(_timeScale) / samplesPerDivision;
            _triggerDelay = _triggerDelay % ADC_BUFFER_SIZE;
            startIndex = (_triggerPosition + _triggerDelay - (int) (viewSize / 2 * deltaSample)) % ADC_BUFFER_SIZE;
            getBufSize = (uint32_t) ((viewSize * deltaSample) + 1);

            // Get data
            rp_AcqGetDataV(RP_CH_1, startIndex, &getBufSize, data[0]);
            rp_AcqGetDataV(RP_CH_2, startIndex, &getBufSize, data[1]);

            // Re-set trigger
            if (trigSweep != RPAPP_OSC_TRIG_SINGLE) {
                threadSafe_acqStart();
            }

            // Reset autoSweep timer
            if (trigSweep == RPAPP_OSC_TRIG_AUTO) {
                timer = clock();
                pthread_mutex_lock(&mutex);
                auto_freRun_mode = false;
                pthread_mutex_unlock(&mutex);
            }

            for (rp_channel_t channel = RP_CH_1; channel <= RP_CH_2; ++channel) {
                // If previous trigger is a problem <=> if 3 data points from trigger are collinear
                bool preTrigProblem = false;
                int preWPIndex = (_preWritePointer - startIndex) % ADC_BUFFER_SIZE;
                if (fabs(data[channel][preWPIndex+2] + data[channel][preWPIndex]- 2*data[channel][preWPIndex+1]) > PRE_WPOINTER_EPSILON) {
                    preTrigProblem = true;
                }

                // Write data to view buffer
                for (int i = 0; i < viewSize && i*deltaSample < buffSize; i++) {
                    if (preTrigProblem && (_preWritePointer - startIndex) % ADC_BUFFER_SIZE > i * deltaSample) {
                        view[channel*viewSize + i] = scaleAmplitudeChannel(channel, 0);
                    }
                    else {
                        view[channel*viewSize + i] = scaleAmplitudeChannel(channel, data[channel][(int) (i * deltaSample)]);
                    }
                }
            }
            // Re-set trigger
            if (trigSweep != RPAPP_OSC_TRIG_SINGLE) {
                usleep(MAIN_THREAT_SLEEP);
                osc_setTriggerSource(trigSource);
            }
        }
        preState = state;
        _preWritePointer = _writePointer;
    } while (!*(bool *) arg);
    return RP_OK;
}