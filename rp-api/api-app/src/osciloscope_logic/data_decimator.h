#ifndef __DATA_DECIMATOR_H
#define __DATA_DECIMATOR_H

#include <stdint.h>
#include <mutex>
#include <functional>
#include "rpApp.h"
#include "constants.h"

class CDataDecimator{
    
public:

    typedef std::function<int(rpApp_osc_source source, float volts, float *res)> func_t;

    CDataDecimator();
    ~CDataDecimator();

    CDataDecimator(CDataDecimator &) = delete;
    CDataDecimator(CDataDecimator &&) = delete;

    auto setViewSize(vsize_t _size) -> void;
    auto getViewSize() const -> vsize_t;
    auto getView() const -> float*;

    auto setDecimationFactor(float _factor) -> void;
    auto getDecimationFactor() const -> float;

    auto setInterpolationMode(rp_channel_t _channel, rpApp_osc_interpolationMode _mode) -> void;
    auto getInterpolationMode(rp_channel_t _channel) const -> rpApp_osc_interpolationMode;

    auto setScaleFunction(func_t _func) -> void;
    auto getScaleFunction() const -> func_t;

    auto setTriggerLevel(float _level) -> void;
    auto getTriggerLevel() const -> float;

    auto precalculateOffset(const float *_data,vsize_t _dataSize) -> int;
    auto resetOffest() -> void;

    auto decimate(rp_channel_t _channel, const float *_data,vsize_t _dataSize, int _triggerPointPos) -> bool;
    auto decimate(rp_channel_t _channel, const float *_data,vsize_t _dataSize, int _triggerPointPos,float *_view,vsize_t _viewSize) -> bool;

private:
    float m_decimationFactor;
    vsize_t  m_viewSize;
    rpApp_osc_interpolationMode m_mode[MAX_ADC_CHANNELS];
    func_t m_scaleFunc;
    std::mutex m_settingsMutex;
    float *m_decimatedData;
    float m_triggerLevel;
    double m_dataOffset;
};

#endif // __DATA_DECIMATOR_H