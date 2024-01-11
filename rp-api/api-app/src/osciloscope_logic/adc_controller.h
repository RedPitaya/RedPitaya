#ifndef __ADC_CONTROLLER_H
#define __ADC_CONTROLLER_H

#include <stdint.h>
#include <mutex>
#include <functional>
#include <atomic>

#include "rpApp.h"
#include "constants.h"

class CADCController{

public:
    typedef std::function<int(rpApp_osc_source source, float volts, float *res)> func_t;

    CADCController();
    ~CADCController();

    CADCController(CADCController &) = delete;
    CADCController(CADCController &&) = delete;

    auto startAcq() -> int;
    auto stopAcq() -> int;

    auto isAdcRun() -> bool;

    auto setContinuousMode(bool _enable) -> int;
    auto getContinuousMode() -> bool;

    auto setTriggerSweep(rpApp_osc_trig_sweep_t _mode) -> void;
    auto getTriggerSweep() -> rpApp_osc_trig_sweep_t;

    auto setTriggerSources(rpApp_osc_trig_source_t _source) -> int;
    auto getTriggerSources() -> rpApp_osc_trig_source_t;
    auto setTriggerToADC() -> int;

    auto setTriggerSourceInFPGA() -> int;
    auto isInternalTrigger() -> bool;
    auto isExternalHasLevel() -> bool;

    auto setTriggerSlope(rpApp_osc_trig_slope_t _slope) -> int;
    auto getTriggerSlope() -> rpApp_osc_trig_slope_t;

    auto setTriggetLevel(float _level) -> int;
    auto getTriggerLevel(float *_level) -> int;

    auto setAttenuateAmplitudeChannelFunction(func_t _func) -> void;
    auto getAttenuateAmplitudeChannelFunction() const -> func_t;

    auto setUnAttenuateAmplitudeChannelFunction(func_t _func) -> void;
    auto getUnAttenuateAmplitudeChannelFunction() const -> func_t;

    auto requestResetWaitTrigger() -> void;
    auto isNeedResetWaitTrigger() -> bool;
    auto resetWaitTriggerRequest() -> void;

private:

    auto setTriggerSourcesUnsafe(rpApp_osc_trig_source_t _source) -> int;

    std::atomic_bool m_isAdcRun;
    std::atomic_bool m_continuousMode;
    std::mutex m_acqMutex;

    rpApp_osc_trig_sweep_t  m_trigSweep;
    rpApp_osc_trig_source_t m_trigSource;
    rpApp_osc_trig_slope_t  m_trigSlope;

    func_t m_attAmplFunc;
    func_t m_UnAttAmplFunc;
    std::atomic_bool m_resetWaitTrigggerRequest;
};

#endif // __ADC_CONTROLLER_H