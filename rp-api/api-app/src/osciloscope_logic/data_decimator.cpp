#include "data_decimator.h"
#include <limits.h>
#include <math.h>
#include "common.h"
#include "math/rp_interpolation.h"

CDataDecimator::CDataDecimator() : m_decimationFactor(1), m_viewSize(0), m_scaleFunc(NULL), m_settingsMutex(), m_decimatedData(), m_triggerLevel(0), m_dataOffset(0) {
    for (auto i = 0u; i < MAX_ADC_CHANNELS; ++i) {
        m_mode[i] = DISABLED;
    }
    m_originalData.resize(ADC_BUFFER_SIZE);
}

CDataDecimator::~CDataDecimator() {
    std::lock_guard lock(m_settingsMutex);
    m_scaleFunc = NULL;
}

auto CDataDecimator::setViewSize(vsize_t _size) -> void {
    std::lock_guard lock(m_settingsMutex);
    m_viewSize = _size;
    m_decimatedData.resize(m_viewSize);
}

auto CDataDecimator::getViewSize() const -> vsize_t {
    return m_viewSize;
}

auto CDataDecimator::getView() -> std::vector<float>* {
    return &m_decimatedData;
}

auto CDataDecimator::setDecimationFactor(float _factor) -> void {
    std::lock_guard lock(m_settingsMutex);
    m_decimationFactor = _factor;
    if (m_decimationFactor <= 0) {
        m_decimationFactor = 0.0000001f;
    }
}

auto CDataDecimator::getDecimationFactor() const -> float {
    return m_decimationFactor;
}

auto CDataDecimator::setInterpolationMode(rp_channel_t _channel, rpApp_osc_interpolationMode _mode) -> void {
    std::lock_guard lock(m_settingsMutex);
    m_mode[_channel] = _mode;
}

auto CDataDecimator::getInterpolationMode(rp_channel_t _channel) const -> rpApp_osc_interpolationMode {
    return m_mode[_channel];
}

auto CDataDecimator::setScaleFunction(func_t _func) -> void {
    std::lock_guard lock(m_settingsMutex);
    m_scaleFunc = _func;
}

auto CDataDecimator::getScaleFunction() const -> func_t {
    return m_scaleFunc;
}

auto CDataDecimator::setTriggerLevel(float _level) -> void {
    std::lock_guard lock(m_settingsMutex);
    m_triggerLevel = _level;
}

auto CDataDecimator::getTriggerLevel() const -> float {
    return m_triggerLevel;
}

auto CDataDecimator::precalculateOffset(const float* _data, vsize_t _dataSize) -> int {
    std::lock_guard lock(m_settingsMutex);
    if (_dataSize == 0)
        return -1;
    if (m_decimationFactor < 1) {
        double offset = 0;
        double xLen = 1.0 / m_decimationFactor;
        double d1 = _data[_dataSize - 1];  // Pre trigger
        double d2 = _data[0];              // After trigger
        double v = xLen;
        double w = d2 - d1;
        double v2 = xLen;

        if (w != 0 && v2 != 0) {
            double t2 = (v * m_triggerLevel - v * d1) / (w * v2);
            double t = (v2 * t2) / v;
            if (t >= 0 || t <= 1 || t2 >= 0 || t2 <= 1) {
                offset = v2 * t2;
            }
        }
        m_dataOffset = offset;
    } else {
        m_dataOffset = 0;
    }
    return 0;
}

auto CDataDecimator::resetOffest() -> void {
    std::lock_guard lock(m_settingsMutex);
    m_dataOffset = 0;
}

auto CDataDecimator::decimate(rp_channel_t _channel, const float* _data, vsize_t _dataSize, int _triggerPointPos) -> bool {
    DataInfo view;
    DataInfo viewRaw;
    return decimate(_channel, _data, _dataSize, _triggerPointPos, &m_decimatedData, &m_originalData, &view, &viewRaw, ValidRange());
}

auto CDataDecimator::decimate(rp_channel_t _channel, const float* _data, vsize_t _dataSize, int _triggerPointPos, std::vector<float>* _view, std::vector<float>* _originalData,
                              DataInfo* _viewInfo, DataInfo* _viewRawInfo, ValidRange range) -> bool {
    std::lock_guard lock(m_settingsMutex);

    if (m_scaleFunc == NULL)
        return false;

    // auto screenToBufferRepeated = [=](int i, float dec, float *t) -> int {
    //     float z = (float)i * dec - 1;
    //     int x = floor(z);
    //     *t = z - x;
    //     if (x >= 0){
    //         return x % _dataSize;
    //     }else{
    //         while(x < 0){
    //             x += _dataSize;
    //         }
    //     }
    //     return  x % _dataSize;
    // };

    auto screenToBuffer = [=](int i, float dec, float* t) -> int {
        float z = (float)i * dec - 1;
        int x = floor(z);
        *t = z - x;
        if (x > _dataSize / 2.0)
            return INT32_MAX;
        if (x < -_dataSize / 2.0)
            return INT32_MAX;
        if (range.m_validBeforeTrigger != -1 && -range.m_validBeforeTrigger > x)
            return INT32_MAX;
        if (range.m_validAfterTrigger != -1 && range.m_validAfterTrigger < x)
            return INT32_MAX;
        if (x >= 0) {
            return x;
        } else {
            x += _dataSize;
        }
        return x;
    };
    auto viewSize = _view->size();
    int centerView = viewSize / 2;
    int trigPostInView = centerView - _triggerPointPos;

    if (((float)viewSize * m_decimationFactor) > (_dataSize)) {
        //   TRACE("Buffer size is smaller than needed for display buffer size %d factor %f",_dataSize,m_decimationFactor)
    }

    int startView, stopView;

    _viewInfo->m_max = std::numeric_limits<float>::lowest();
    _viewInfo->m_min = std::numeric_limits<float>::max();
    _viewInfo->m_mean = 0;
    _viewInfo->m_maxUnscale = std::numeric_limits<float>::lowest();
    _viewInfo->m_minUnscale = std::numeric_limits<float>::max();
    _viewInfo->m_meanUnscale = 0;

    _viewRawInfo->m_max = 0;
    _viewRawInfo->m_min = 0;
    _viewRawInfo->m_mean = 0;
    _viewRawInfo->m_maxUnscale = std::numeric_limits<float>::lowest();
    _viewRawInfo->m_minUnscale = std::numeric_limits<float>::max();
    _viewRawInfo->m_meanUnscale = 0;

    float scaleFuncCof1 = 1, scaleFuncCof2 = 1;
    ECHECK_APP_NO_RET(m_scaleFunc((rpApp_osc_source)_channel, scaleFuncCof1, scaleFuncCof2))
    if (m_decimationFactor < 1) {
        trigPostInView -= m_dataOffset;
        startView = 0 - trigPostInView;
        stopView = viewSize - trigPostInView;

        float t = 0;
        float t_prev = 0;
        float scaledValue = 0, y = 0;
        uint16_t iView = 0;
        uint32_t count = 0;
        for (int idx = startView; idx < stopView; idx++, iView++) {
            int dataIndex1 = screenToBuffer(idx, m_decimationFactor, &t);
            y = 0;
            scaledValue = 0;
            if (dataIndex1 != INT32_MAX) {
                int dataIndex2 = (dataIndex1 + 1) % _dataSize;
                switch (m_mode[_channel]) {

                    case DISABLED: {
                        (*_view)[iView] = std::numeric_limits<float>::signaling_NaN();
                        if (t < t_prev) {
                            y = _data[dataIndex1];
                            t_prev = t;
                        } else {
                            t_prev = t;
                            continue;
                        }
                        break;
                    }

                    case LINEAR: {
                        y = linear<float>(0, _data[dataIndex1], _data[dataIndex2], 0, t);
                        break;
                    }

                    case BSPLINE: {
                        int dataIndex0 = (dataIndex1 - 1) < 0 ? _dataSize - 1 : (dataIndex1 - 1) % _dataSize;
                        int dataIndex3 = (dataIndex1 + 2) % _dataSize;
                        y = bSpline<float>(_data[dataIndex0], _data[dataIndex1], _data[dataIndex2], _data[dataIndex3], t);
                        break;
                    }

                    case CATMULLROM: {
                        int dataIndex0 = (dataIndex1 - 1) < 0 ? _dataSize - 1 : (dataIndex1 - 1) % _dataSize;
                        int dataIndex3 = (dataIndex1 + 2) % _dataSize;
                        y = catmullRom<float>(_data[dataIndex0], _data[dataIndex1], _data[dataIndex2], _data[dataIndex3], t);
                        break;
                    }
                    case LANCZOS: {
                        int dataIndex_1 = (dataIndex1 - 2) < 0 ? _dataSize - 2 : (dataIndex1 - 2) % _dataSize;
                        int dataIndex0 = (dataIndex1 - 1) < 0 ? _dataSize - 1 : (dataIndex1 - 1) % _dataSize;
                        int dataIndex3 = (dataIndex1 + 2) % _dataSize;
                        y = lanczos<float>(_data[dataIndex_1], _data[dataIndex0], _data[dataIndex1], _data[dataIndex2], _data[dataIndex3], t);
                        break;
                    }
                    default:
                        FATAL("No function for interpolation mode")
                        break;
                }

                if (_viewInfo->m_maxUnscale < y)
                    _viewInfo->m_maxUnscale = y;
                if (_viewInfo->m_minUnscale > y)
                    _viewInfo->m_minUnscale = y;
                _viewInfo->m_meanUnscale += y;
                scaledValue = scaleAmplitude<float>(y, scaleFuncCof1, scaleFuncCof2);
                if (_viewInfo->m_max < scaledValue)
                    _viewInfo->m_max = scaledValue;
                if (_viewInfo->m_min > scaledValue)
                    _viewInfo->m_min = scaledValue;
                _viewInfo->m_mean += scaledValue;
                count++;
            } else {
                scaledValue = std::numeric_limits<float>::signaling_NaN();
            }
            // ECHECK_APP_NO_RET(m_scaleFunc((rpApp_osc_source)_channel,y,&scaledValue))
            // x -> y
            // scaledValue = idx / 16384;
            (*_view)[iView] = scaledValue;
        }
        _viewInfo->m_mean /= count ? count : 1;
        _viewInfo->m_meanUnscale /= count ? count : 1;
    } else {
        startView = 0 - trigPostInView;
        stopView = viewSize - trigPostInView;

        float t = 0;
        float scaledValue = 0;
        uint16_t iView = 0;
        uint32_t count = 0;
        for (int idx = startView; idx < stopView; idx++, iView++) {
            int dataIndex = screenToBuffer(idx, m_decimationFactor, &t);
            // ECHECK_APP_NO_RET(m_scaleFunc((rpApp_osc_source)_channel,_data[dataIndex],&scaledValue))
            scaledValue = 0;
            if (dataIndex != INT32_MAX) {
                auto y = _data[dataIndex];
                scaledValue = scaleAmplitude<float>(y, scaleFuncCof1, scaleFuncCof2);
                // x -> y
                // scaledValue = idx / 16384;
                if (_viewInfo->m_maxUnscale < y)
                    _viewInfo->m_maxUnscale = y;
                if (_viewInfo->m_minUnscale > y)
                    _viewInfo->m_minUnscale = y;
                _viewInfo->m_meanUnscale += y;
                if (_viewInfo->m_max < scaledValue)
                    _viewInfo->m_max = scaledValue;
                if (_viewInfo->m_min > scaledValue)
                    _viewInfo->m_min = scaledValue;
                _viewInfo->m_mean += scaledValue;
                count++;
            } else {
                scaledValue = std::numeric_limits<float>::signaling_NaN();
            }
            (*_view)[iView] = scaledValue;
        }
        _viewInfo->m_mean /= count ? count : 1;
        _viewInfo->m_meanUnscale /= count ? count : 1;
    }

    if (_originalData) {
        _originalData->resize(0);
        float t = 0;
        int dataIndexStart = INT32_MAX;
        int dataIndexEnd = INT32_MAX;
        int x = startView;
        while (dataIndexStart == INT32_MAX && x <= stopView) {
            dataIndexStart = screenToBuffer(x, m_decimationFactor, &t);
            x++;
        }
        x = stopView;
        while (dataIndexEnd == INT32_MAX && x >= startView) {
            dataIndexEnd = screenToBuffer(x, m_decimationFactor, &t);
            x--;
        }
        uint32_t count = 0;
        for (int idx = dataIndexStart; idx != dataIndexEnd; idx = (idx + 1) % ADC_BUFFER_SIZE) {
            auto y = _data[idx];
            _originalData->push_back(y);
            if (_viewRawInfo->m_maxUnscale < y)
                _viewRawInfo->m_maxUnscale = y;
            if (_viewRawInfo->m_minUnscale > y)
                _viewRawInfo->m_minUnscale = y;
            _viewRawInfo->m_meanUnscale += y;
            count++;
        }
        _originalData->push_back(_data[dataIndexEnd]);
        _viewRawInfo->m_meanUnscale /= count ? count : 1;
    }

    return true;
}
