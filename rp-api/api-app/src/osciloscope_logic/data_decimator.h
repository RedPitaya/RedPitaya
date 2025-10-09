#ifndef __DATA_DECIMATOR_H
#define __DATA_DECIMATOR_H

#include <stdint.h>
#include <functional>
#include <mutex>
#include <vector>
#include "common.h"
#include "constants.h"
#include "rpApp.h"

class CDataDecimator {

   public:
    struct DataInfo {
        float m_minUnscale = 0;
        float m_maxUnscale = 0;
        float m_meanUnscale = 0;
        float m_min = 0;
        float m_max = 0;
        float m_mean = 0;
    };

    struct ValidRange {
        int32_t m_validBeforeTrigger = -1;
        int32_t m_validAfterTrigger = -1;
    };

    typedef std::function<int(rpApp_osc_source source, float& coff1, float& coff2)> func_t;

    CDataDecimator();
    ~CDataDecimator();

    CDataDecimator(CDataDecimator&) = delete;
    CDataDecimator(CDataDecimator&&) = delete;

    auto setViewSize(vsize_t _size) -> void;
    auto getViewSize() const -> vsize_t;
    auto getView() -> std::vector<float>*;

    auto setDecimationFactor(float _factor) -> void;
    auto getDecimationFactor() const -> float;

    auto setInterpolationMode(rp_channel_t _channel, rpApp_osc_interpolationMode _mode) -> void;
    auto getInterpolationMode(rp_channel_t _channel) const -> rpApp_osc_interpolationMode;

    auto setScaleFunction(func_t _func) -> void;
    auto getScaleFunction() const -> func_t;

    auto setTriggerLevel(float _level) -> void;
    auto getTriggerLevel() const -> float;

    auto precalculateOffset(const float* _data, vsize_t _dataSize) -> int;
    auto resetOffest() -> void;

    auto decimate(rp_channel_t _channel, const float* _data, vsize_t _dataSize, int _triggerPointPos) -> bool;
    auto decimate(rp_channel_t _channel, const float* _data, vsize_t _dataSize, int _triggerPointPos, std::vector<float>* _view, std::vector<float>* _originalData,
                  DataInfo* _viewInfo, DataInfo* _viewRawInfo, ValidRange range, std::vector<float>* _unscaledView) -> bool;

   private:
    float m_decimationFactor;
    vsize_t m_viewSize;
    rpApp_osc_interpolationMode m_mode[MAX_ADC_CHANNELS];
    func_t m_scaleFunc;
    std::mutex m_settingsMutex;
    std::vector<float> m_decimatedData;
    std::vector<float> m_originalData;

    float m_triggerLevel;
    double m_dataOffset;
};

#endif  // __DATA_DECIMATOR_H