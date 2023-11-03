#ifndef __VIEW_CONTROLLER_H
#define __VIEW_CONTROLLER_H

#include <stdint.h>
#include <mutex>
#include <atomic>
#include <vector>

#include "rpApp.h"
#include "constants.h"

class CViewController{
    
public:

    enum EViewMode{
        NORMAL = 0,
        ROLL   = 1
    };

    CViewController();
    ~CViewController();

    CViewController(CViewController &) = delete;
    CViewController(CViewController &&) = delete;

    auto getGridXCount() const -> uint16_t;
    auto getGridYCount() const -> uint16_t;

    auto getViewSize() const -> vsize_t;
    auto setViewSize(vsize_t _size) -> void;

    auto getSamplesPerDivision() const -> float;
    
    auto lockView() -> void;
    auto unlockView() -> void;

    auto getView(rpApp_osc_source _channel) -> std::vector<float>*;
    auto getOriginalData(rpApp_osc_source _channel) -> std::vector<float>*;
    auto getAcqBuffers() -> buffers_t*;

    auto getClock() -> double;

    auto isSine(rpApp_osc_source _channel) -> bool;

    auto requestUpdateViewFromADC() -> void;
    auto updateViewFromADCDone() -> void;
    auto isNeedUpdateViewFromADC() -> bool;

    auto requestUpdateView() -> void;
    auto updateViewDone() -> void;
    auto isNeedUpdateView() -> bool;

    auto setAutoScale(bool _state) -> void;
    auto getAutoScale() -> bool;

    auto convertSamplesToTime(int32_t _samples) -> double;
    auto calculateTimeOut(float _timeScale) -> double;

    auto viewIndexToTime(int _index) -> float;
    
    auto setTimeScale(float _scale) -> int;
    auto getTimeScale() -> float;

    auto setTimeOffset(float _offset) -> int;
    auto getTimeOffset() -> float;

    auto calculateDecimation(float _scale,rp_acq_decimation_t *_decimation) -> int;
    auto getCurrentDecimation() -> rp_acq_decimation_t;

    auto clearView() -> void;

    auto runOsc() -> void;
    auto stopOsc() -> void;
    auto isOscRun() const -> bool;

    auto setTriggerState(bool _state) -> void;
    auto isTriggered() const -> bool;

    auto getViewMode() -> EViewMode;
    auto setViewMode(EViewMode _mode) -> void;

    auto getSampledAfterTriggerInView() -> uint32_t;

    auto setCapturedDecimation(rp_acq_decimation_t _dec) -> void;
    auto getCapturedDecimation() -> rp_acq_decimation_t;
    
private: 

    auto initView() -> bool;
    auto releaseView() -> void;

    uint16_t m_viewGridXCount;
    uint16_t m_viewGridYCount;
    vsize_t  m_viewSizeInPoints;
    std::vector<float> m_view[MAX_VIEW_CHANNELS];
    std::vector<float> m_origialData[MAX_VIEW_CHANNELS];

    std::mutex m_viewMutex;

    buffers_t *m_acqData;

    std::atomic_bool m_updateViewFromADCRequest;
    std::atomic_bool m_updateViewRequest;
    std::atomic_bool m_autoScale;

    float m_timeScale;
    float m_timeOffet;

    std::atomic_bool m_oscIsRunning;
    std::atomic_bool m_triggerState;

    EViewMode m_ViewMode;

    rp_acq_decimation_t m_capturedDecimation;
};

#endif // __VIEW_CONTROLLER_H