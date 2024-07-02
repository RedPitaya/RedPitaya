#ifndef __VIEW_CONTROLLER_H
#define __VIEW_CONTROLLER_H

#include <stdint.h>
#include <mutex>
#include <atomic>
#include <vector>
#include <chrono>

#include "rpApp.h"
#include "constants.h"

class CViewController{

public:

    struct OscillogramInfo{
        float m_min = 0;
        float m_max = 0;
        float m_mean = 0;
        float m_minUnscale = 0;
        float m_maxUnscale = 0;
        float m_meanUnscale = 0;
        float m_minRaw = 0;
        float m_maxRaw = 0;
        float m_meanRaw = 0;
        uint32_t m_decimatoion = 1;
        bool m_dataHasTrigger;
    };

    struct Oscillogram
    {
        buffers_t* m_data;
        std::mutex m_viewMutex;
        uint32_t m_decimation;
        bool m_dataHasTrigger;
        int m_index;
        uint32_t m_pointerPosition;
        uint32_t m_validBeforeTrigger;
        uint32_t m_validAfterTrigger;
        Oscillogram();
        ~Oscillogram();
        Oscillogram(Oscillogram &&) = delete;
    };


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

    auto prepareOscillogramBuffer(size_t _maxBuffers) -> void;
    auto resetCurrentBuffer() -> void;
    auto nextBuffer() -> void;
    auto getOscillogramBufferCount() -> size_t;

    auto lockScreenView() -> void;
    auto unlockScreenView() -> void;

    auto lockControllerView() -> void;
    auto unlockControllerView() -> void;

    // auto lockCurrentOscilogramm() -> void;
    // auto unlockCurrentOscilogramm() -> void;

    auto getCurrentOscillogram() -> Oscillogram*;
    auto getOscillogramForView() -> Oscillogram*;

    auto getView(rpApp_osc_source _channel) -> std::vector<float>*;
    auto getOriginalData(rpApp_osc_source _channel) -> std::vector<float>*;
    auto getViewInfo(rpApp_osc_source _channel) -> OscillogramInfo*;
    // auto getAcqBuffers() -> buffers_t*;

    auto getClock() -> double;

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

    auto calculateDecimation(float _scale,uint32_t *_decimation,bool _continuesMode) -> int;
    auto getCurrentDecimation(bool _continuesMode) -> uint32_t;

    // auto clearView() -> void;

    auto runOsc() -> void;
    auto stopOsc() -> void;
    auto isOscRun() const -> bool;

    auto setTriggerState(bool _state) -> void;
    auto isTriggered() const -> bool;
    // auto setDataWithTrigger(bool _state) -> void;
    // auto isDataWithTrigger() -> bool;

    auto getViewMode() -> EViewMode;
    auto setViewMode(EViewMode _mode) -> void;

    auto getSampledAfterTriggerInView() -> uint32_t;
    auto calcExtraPoints() -> uint32_t;

    auto addOscCounter() -> void;
    auto getOscPerSec() -> uint32_t;
    auto bufferSelectNext() -> void;
    auto bufferSelectPrev() -> void;
    auto bufferCurrent(int32_t *current) -> void;

    // auto setCapturedDecimation(uint32_t _dec) -> void;
    // auto getCapturedDecimation() -> uint32_t;


private:

    auto initView() -> bool;
    auto releaseView() -> void;

    uint16_t m_viewGridXCount;
    uint16_t m_viewGridYCount;
    vsize_t  m_viewSizeInPoints;

    std::vector<float> m_view[MAX_VIEW_CHANNELS];
    std::vector<float> m_viewRaw[MAX_VIEW_CHANNELS];
    OscillogramInfo m_viewInfo[MAX_VIEW_CHANNELS];

    std::vector<Oscillogram*> m_origialData;

    std::mutex m_viewControllerMutex;
    std::mutex m_viewMutex;
    std::mutex m_viewBuffersMutex;

    // buffers_t *m_acqData;
    // bool       m_dataHasTrigger;

    std::atomic_bool m_updateViewFromADCRequest;
    std::atomic_bool m_updateViewRequest;
    std::atomic_bool m_autoScale;

    float m_timeScale;
    float m_timeOffet;

    std::atomic_bool m_oscIsRunning;
    std::atomic_bool m_triggerState;

    EViewMode m_ViewMode;

    uint32_t m_currentBuffer;
    uint32_t m_stoppedBuffer;

    std::chrono::time_point<std::chrono::system_clock> m_lastTimeCapture;
    uint32_t m_oscPerSec;
    uint32_t m_oscPerSecCounter;

};

#endif // __VIEW_CONTROLLER_H