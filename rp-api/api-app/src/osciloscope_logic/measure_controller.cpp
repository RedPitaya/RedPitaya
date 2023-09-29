#include <float.h>
#include <math.h>
#include "measure_controller.h"
#include "common.h"

CMeasureController::CMeasureController():
    m_unscaleFunc(NULL),
    m_scaleFunc(NULL),
    m_attAmplFunc(NULL)
{
    if (getADCSamplePeriod(&m_sample_per) != RP_OK){
        FATAL("Can't get a period of samples")
    }
    m_osc_fpga_smpl_freq = getADCRate();

    if (rp_HPGetFastADCBits(&m_adc_bits)  != RP_OK){
        FATAL("Can't get a adc bits")
    }
}

CMeasureController::~CMeasureController(){
    setUnScaleFunction(NULL);
}

auto CMeasureController::setUnScaleFunction(func_t _func) -> void{
    std::lock_guard<std::mutex> lock(m_settingsMutex);
    m_unscaleFunc = _func;
}

auto CMeasureController::getUnScaleFunction() const -> func_t{
    return m_unscaleFunc;
}

auto CMeasureController::setAttenuateAmplitudeChannelFunction(func_t _func) -> void{
    std::lock_guard<std::mutex> lock(m_settingsMutex);
    m_attAmplFunc = _func;
}

auto CMeasureController::getAttenuateAmplitudeChannelFunction() const -> func_t{
    return m_attAmplFunc;
}

auto CMeasureController::setscaleFunction(func_t _func) -> void{
    std::lock_guard<std::mutex> lock(m_settingsMutex);
    m_scaleFunc = _func;
}

auto CMeasureController::getscaleFunction() const -> func_t{
    return m_scaleFunc;
}


auto CMeasureController::check(const void *_data, vsize_t _sizeView) -> int{
    if (m_unscaleFunc == NULL) {
        WARNING("Undefined unscale function")
        return RP_EOOR;
    }
    
    if (m_attAmplFunc == NULL) {
        WARNING("Undefined attenuate function")
        return RP_EOOR;
    }

    if (_data == NULL) {
        WARNING("No data")
        return RP_EOOR;
    }

    if (_sizeView == 0) {
        WARNING("Data size is zero")
        return RP_EOOR;
    }
    return RP_OK;
}

auto CMeasureController::measureVpp(const rpApp_osc_source _channel, const float *_data, vsize_t _viewSize, float *_Vpp) -> int{
    std::lock_guard<std::mutex> lock(m_settingsMutex);
    
    auto ret = check(_data,_viewSize);
    if (ret != RP_OK)
        return ret;

    float resMax, resMin, max = -FLT_MAX, min = FLT_MAX;

    for (vsize_t i = 0; i < _viewSize; ++i) {
        auto z = _data[i];
        max = MAX(z,max);
        min = MIN(z,min);
    }

    ECHECK_APP(m_unscaleFunc(_channel, max, &resMax));
    ECHECK_APP(m_unscaleFunc(_channel, min, &resMin));
    *_Vpp = resMax - resMin;
    ECHECK_APP(m_attAmplFunc(_channel, *_Vpp, _Vpp));
    *_Vpp = fabs(*_Vpp);
    return RP_OK;
}

auto CMeasureController::measureMax(const rpApp_osc_source _channel, const float *_data, vsize_t _viewSize, float *_Max) -> int{
    std::lock_guard<std::mutex> lock(m_settingsMutex);

    auto ret = check(_data,_viewSize);
    if (ret != RP_OK)
        return ret;


    float resMax, max = -FLT_MAX;

    for (vsize_t i = 0; i < _viewSize; ++i) {
        max = MAX(_data[i],max);
    }

    ECHECK_APP(m_unscaleFunc(_channel, max, &resMax));
    *_Max = resMax;
    return RP_OK;
}

auto CMeasureController::measureMin(const rpApp_osc_source _channel, const float *_data, vsize_t _viewSize, float *_Min) -> int{
    std::lock_guard<std::mutex> lock(m_settingsMutex);

    auto ret = check(_data,_viewSize);
    if (ret != RP_OK)
        return ret;

    float resMax, min = FLT_MAX;

    for (vsize_t i = 0; i < _viewSize; ++i) {
        min = MIN(_data[i],min);
    }

    ECHECK_APP(m_unscaleFunc(_channel, min, &resMax));
    *_Min = resMax;
    return RP_OK;
}

auto CMeasureController::measureMeanVoltage(const rpApp_osc_source _channel, const float *_data, vsize_t _viewSize, float *_meanVoltage) -> int{
    std::lock_guard<std::mutex> lock(m_settingsMutex);

    auto ret = check(_data,_viewSize);
    if (ret != RP_OK)
        return ret;

    double sum = 0;

    for (vsize_t i = 0; i < _viewSize; ++i) {
        sum += _data[i];
    }

    ECHECK_APP(m_unscaleFunc(_channel, sum / static_cast<double>(_viewSize), _meanVoltage));
    ECHECK_APP(m_attAmplFunc(_channel, *_meanVoltage, _meanVoltage));
    return RP_OK;
}

auto CMeasureController::measureMaxVoltage(const rpApp_osc_source _channel, bool _inverted, const float *_data, vsize_t _viewSize, float *_Vmax) -> int{
    std::lock_guard<std::mutex> lock(m_settingsMutex);

    auto ret = check(_data,_viewSize);
    if (ret != RP_OK)
        return ret;

    float max = _data[0];

    for (vsize_t i = 0; i < _viewSize; ++i) {
        auto z = _data[i];
        if (_inverted ? z < max : z > max) {
            max = z;
        }
    }
    *_Vmax = max;
    ECHECK_APP(m_unscaleFunc(_channel, max, _Vmax));
    ECHECK_APP(m_attAmplFunc(_channel, *_Vmax, _Vmax));
    return RP_OK;
}

auto CMeasureController::measureMinVoltage(const rpApp_osc_source _channel, bool _inverted, const float *_data, vsize_t _viewSize, float *_Vmin) -> int{
    std::lock_guard<std::mutex> lock(m_settingsMutex);
    
    auto ret = check(_data,_viewSize);
    if (ret != RP_OK)
        return ret;

    float min = _data[0];

    for (vsize_t i = 0; i < _viewSize; ++i) {
        auto z = _data[i];
        if (_inverted ? z > min : z < min) {
            min = z;
        }
    }
    *_Vmin = min;
    ECHECK_APP(m_unscaleFunc(_channel, min, _Vmin));
    ECHECK_APP(m_attAmplFunc(_channel, *_Vmin, _Vmin));
    return RP_OK;
}

auto CMeasureController::measureDutyCycle(const rpApp_osc_source _channel, const float *_data, vsize_t _viewSize, float *_dutyCycle) -> int{
    int highTime = 0;
    float meanValue;
    ECHECK_APP(measureMeanVoltage(_channel, _data, _viewSize, &meanValue));
    ECHECK_APP(m_scaleFunc(_channel, meanValue, &meanValue))

    std::lock_guard<std::mutex> lock(m_settingsMutex);
    auto ret = check(_data,_viewSize);
    if (ret != RP_OK)
        return ret;
    
    for (vsize_t i = 0; i < _viewSize; ++i) {
        if (_data[i] > meanValue) {
            ++highTime;
        }
    }

    *_dutyCycle = (float)highTime / (float)(_viewSize);
    return RP_OK;
    
}

auto CMeasureController::measureRootMeanSquare(const rpApp_osc_source _channel, const float *_data, vsize_t _viewSize, float *_rms) -> int{
    std::lock_guard<std::mutex> lock(m_settingsMutex);

    auto ret = check(_data,_viewSize);
    if (ret != RP_OK)
        return ret;

    double rmsValue = 0;
    for (vsize_t i = 0; i < _viewSize; ++i) {
        auto z = _data[i];
        float tmp;
        ECHECK_APP(m_unscaleFunc(_channel, z, &tmp));
        rmsValue += tmp * tmp;
    }
    *_rms = (double) sqrt(rmsValue / (double)(_viewSize));
    ECHECK_APP(m_attAmplFunc(_channel, *_rms, _rms));
    return RP_OK;
}



auto CMeasureController::measurePeriodCh(const float *_dataRaw, vsize_t _dataSize, float *period) -> int{
    auto ret = check(_dataRaw,_dataSize);
    if (ret != RP_OK)
        return ret;

    static const float c_meas_freq_thr = 0.0005;
    
    int size = _dataSize;
    const int c_meas_time_thr = _dataSize / m_sample_per;
    const double c_min_period = 2.0 / m_osc_fpga_smpl_freq; // fpga rate / 2
    float thr1, thr2, cen;
    int state = 0;
    int trig_t[2] = { 0, 0 };
    int trig_cnt = 0;
    int ix;

    float meas_max, meas_min;
    float z = _dataRaw[0];
    meas_max = z;
    meas_min = z;
    for(vsize_t i = 0; i < size; i++)
    {
        z = _dataRaw[i];
        meas_max = MAX(z,meas_max);
        meas_min = MIN(z,meas_max);
    }

	uint32_t dec_factor = 1;
    ECHECK_APP(rp_AcqGetDecimationFactor(&dec_factor));

    float acq_dur = (float)(size)/(m_osc_fpga_smpl_freq) * (float) dec_factor;
    cen = (meas_max + meas_min) / 2;
    thr1 = cen + 0.2 * (meas_min - cen);
    thr2 = cen + 0.2 * (meas_max - cen);
    float res_period = 0;
    for(ix = 0; ix < size; ix++) {
        auto sa = _dataRaw[ix];

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
            (m_osc_fpga_smpl_freq * (trig_cnt - 1)) * dec_factor;
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

auto CMeasureController::measurePeriodMath(float _timeScale, float _sampPerDev, const float *_data, vsize_t _dataSize, float *period) -> int{
    float m_viewTmp[VIEW_SIZE_MAX];
    float xcorr[VIEW_SIZE_MAX];
    
    auto ret = check(_data,_dataSize);
    if (ret != RP_OK)
        return ret;
    
    auto size = _dataSize;

    if (size > VIEW_SIZE_MAX){
        FATAL("The view buffer is larger than the allocated memory")
        return RP_EOOR;
    }

    float mean = 0;
    for (vsize_t i = 0; i < size; ++i) {
        m_viewTmp[i] = _data[i];
        mean += _data[i];
    }

    mean = mean / size;
    for (int i = 0; i < size; ++i){
        m_viewTmp[i] -= mean;
    }

    // calculate signal correlation

    for (vsize_t i = 0; i < size; ++i) {
        xcorr[i] = 0;
        for (int j = 0; j < size-i; ++j) {
            xcorr[i] += m_viewTmp[j] * m_viewTmp[j+i];
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
    auto viewScale = timeToIndexD(_timeScale) / _sampPerDev;

    float idx = ((left_amax_idx + right_amax_idx) / 2.f) * viewScale;
    *period = indexToTime(idx);

    return RP_OK;
}

