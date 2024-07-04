#include "adc_controller.h"
#include "common.h"

CADCController::CADCController():
    m_isAdcRun(false),
    m_continuousMode(false),
    m_trigSweep(RPAPP_OSC_TRIG_AUTO),
    m_trigSource(RPAPP_OSC_TRIG_SRC_CH1),
    m_trigSlope(RPAPP_OSC_TRIG_SLOPE_PE),
    m_attAmplFunc(NULL),
    m_UnAttAmplFunc(NULL),
    m_resetWaitTrigggerRequest(false)
{

}

CADCController::~CADCController(){

}

auto CADCController::startAcq() -> int{
    std::lock_guard lock(m_acqMutex);
    ECHECK(rp_AcqUnlockTrigger())
    if (m_isAdcRun)
        return RP_OK;
    ECHECK(rp_AcqSetArmKeep(m_trigSweep == RPAPP_OSC_TRIG_AUTO && m_continuousMode));
    ECHECK(rp_AcqStart())
    m_isAdcRun = true;
    return RP_OK;
}

auto CADCController::stopAcq() -> int{
    std::lock_guard<std::mutex> lock(m_acqMutex);
    ECHECK(rp_AcqStop())
    ECHECK(rp_AcqSetArmKeep(false))
    m_isAdcRun = false;
    return RP_OK;
}

auto CADCController::isAdcRun() -> bool{
    return m_isAdcRun;
}

auto CADCController::setContinuousMode(bool _enable) -> int{
    std::lock_guard lock(m_acqMutex);
    ECHECK(rp_AcqSetArmKeep(_enable))
    m_continuousMode = _enable;
    return RP_OK;
}

auto CADCController::getContinuousMode() -> bool{
    return m_continuousMode;
}

auto CADCController::setTriggerSweep(rpApp_osc_trig_sweep_t _mode) -> void{
    std::lock_guard lock(m_acqMutex);
    m_trigSweep = _mode;
}

auto CADCController::getTriggerSweep() -> rpApp_osc_trig_sweep_t{
    return m_trigSweep;
}

auto CADCController::setTriggerSourceInFPGA() -> int{
    return setTriggerSources(m_trigSource);
}

auto CADCController::setTriggerSources(rpApp_osc_trig_source_t _source) -> int{
    std::lock_guard lock(m_acqMutex);
    return setTriggerSourcesUnsafe(_source);
}

auto CADCController::requestResetWaitTrigger() -> void{
    m_resetWaitTrigggerRequest = true;
}

auto CADCController::isNeedResetWaitTrigger() -> bool{
    return m_resetWaitTrigggerRequest;
}

auto CADCController::resetWaitTriggerRequest() -> void{
    m_resetWaitTrigggerRequest = false;
}


auto CADCController::setTriggerSourcesUnsafe(rpApp_osc_trig_source_t _source) -> int{
    m_trigSource = _source;
    return RP_OK;
}

auto CADCController::setTriggerToADC() -> int{
    rp_acq_trig_src_t src;
    switch (m_trigSource) {
        case RPAPP_OSC_TRIG_SRC_CH1:
            if (m_trigSlope == RPAPP_OSC_TRIG_SLOPE_NE) {
                src = RP_TRIG_SRC_CHA_NE;
            }
            else {
                src = RP_TRIG_SRC_CHA_PE;
            }
            break;
        case RPAPP_OSC_TRIG_SRC_CH2:
            if (m_trigSlope == RPAPP_OSC_TRIG_SLOPE_NE) {
                src = RP_TRIG_SRC_CHB_NE;
            }
            else {
                src = RP_TRIG_SRC_CHB_PE;
            }
            break;
        case RPAPP_OSC_TRIG_SRC_CH3:
            if (m_trigSlope == RPAPP_OSC_TRIG_SLOPE_NE) {
                src = RP_TRIG_SRC_CHC_NE;
            }
            else {
                src = RP_TRIG_SRC_CHC_PE;
            }
            break;
        case RPAPP_OSC_TRIG_SRC_CH4:
            if (m_trigSlope == RPAPP_OSC_TRIG_SLOPE_NE) {
                src = RP_TRIG_SRC_CHD_NE;
            }
            else {
                src = RP_TRIG_SRC_CHD_PE;
            }
            break;
        case RPAPP_OSC_TRIG_SRC_EXTERNAL:
            if (m_trigSlope == RPAPP_OSC_TRIG_SLOPE_NE) {
                src = RP_TRIG_SRC_EXT_NE;
            }
            else {
                src = RP_TRIG_SRC_EXT_PE;
            }
            break;
        default:
            WARNING("Unknown value %d",m_trigSource)
            return RP_EOOR;
    }
    ECHECK(rp_AcqSetTriggerSrc(src));
    return RP_OK;
}


auto CADCController::getTriggerSources() -> rpApp_osc_trig_source_t{
    return m_trigSource;
}

auto CADCController::isInternalTrigger() -> bool{
    return m_trigSource != RPAPP_OSC_TRIG_SRC_EXTERNAL;
}

auto CADCController::isExternalHasLevel() -> bool{
    bool state;
    if (rp_HPGetIsExternalTriggerLevelPresent(&state) == RP_OK){
        return state;
    }
    return false;
}

auto CADCController::setTriggerSlope(rpApp_osc_trig_slope_t _slope) -> int{
    std::lock_guard<std::mutex> lock(m_acqMutex);
    auto tmp = m_trigSlope;
    m_trigSlope = _slope;
    auto ret = setTriggerSourcesUnsafe(m_trigSource);
    if (ret != RP_OK){
        m_trigSlope = tmp;
        return ret;
    }
    return RP_OK;
}

auto CADCController::getTriggerSlope() -> rpApp_osc_trig_slope_t{
    return m_trigSlope;
}

auto CADCController::setAttenuateAmplitudeChannelFunction(func_t _func) -> void{
    std::lock_guard lock(m_acqMutex);
    m_attAmplFunc = _func;
}

auto CADCController::getAttenuateAmplitudeChannelFunction() const -> func_t{
    return m_attAmplFunc;
}

auto CADCController::setUnAttenuateAmplitudeChannelFunction(func_t _func) -> void{
    std::lock_guard lock(m_acqMutex);
    m_UnAttAmplFunc = _func;
}

auto CADCController::getUnAttenuateAmplitudeChannelFunction() const -> func_t{
    return m_UnAttAmplFunc;
}

auto CADCController::setTriggerLevel(float _level) -> int{
    std::lock_guard lock(m_acqMutex);

    if(m_trigSource != RPAPP_OSC_TRIG_SRC_EXTERNAL) {
        rpApp_osc_source source = RPAPP_OSC_SOUR_CH1;
        if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH2) source = RPAPP_OSC_SOUR_CH2;
        if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH3) source = RPAPP_OSC_SOUR_CH3;
        if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH4) source = RPAPP_OSC_SOUR_CH4;
        ECHECK_APP(m_UnAttAmplFunc(source, _level, &_level));
    }
    if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH1){
        ECHECK(rp_AcqSetTriggerLevel(RP_T_CH_1, _level));
    }
    if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH2){
        ECHECK(rp_AcqSetTriggerLevel(RP_T_CH_2, _level));
    }
    if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH3){
        ECHECK(rp_AcqSetTriggerLevel(RP_T_CH_3, _level));
    }
    if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH4){
        ECHECK(rp_AcqSetTriggerLevel(RP_T_CH_4, _level));
    }
    return RP_OK;
}

auto CADCController::getTriggerLevel(float *_level) -> int{
    std::lock_guard lock(m_acqMutex);

    if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH1){
        ECHECK(rp_AcqGetTriggerLevel(RP_T_CH_1,_level));
    }

    if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH2){
        ECHECK(rp_AcqGetTriggerLevel(RP_T_CH_2,_level));
    }
    if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH3){
        ECHECK(rp_AcqGetTriggerLevel(RP_T_CH_3,_level));
    }

    if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH4){
        ECHECK(rp_AcqGetTriggerLevel(RP_T_CH_4,_level));
    }

    if(m_trigSource != RPAPP_OSC_TRIG_SRC_EXTERNAL) {
        rpApp_osc_source source = RPAPP_OSC_SOUR_CH1;
        if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH2) source = RPAPP_OSC_SOUR_CH2;
        if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH3) source = RPAPP_OSC_SOUR_CH3;
        if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH4) source = RPAPP_OSC_SOUR_CH4;
        ECHECK_APP(m_attAmplFunc(source, *_level, _level));
    }
    return RP_OK;
}

auto CADCController::getTriggerLevelRaw(float *_level) -> int{
    std::lock_guard lock(m_acqMutex);

    if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH1){
        ECHECK(rp_AcqGetTriggerLevel(RP_T_CH_1,_level));
    }

    if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH2){
        ECHECK(rp_AcqGetTriggerLevel(RP_T_CH_2,_level));
    }
    if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH3){
        ECHECK(rp_AcqGetTriggerLevel(RP_T_CH_3,_level));
    }

    if (m_trigSource == RPAPP_OSC_TRIG_SRC_CH4){
        ECHECK(rp_AcqGetTriggerLevel(RP_T_CH_4,_level));
    }

    return RP_OK;
}

auto CADCController::setExternalTriggerLevel(float _level) -> int{
    std::lock_guard lock(m_acqMutex);
    static auto IsExternalTriggerLevelPresent = rp_HPGetIsExternalTriggerLevelPresentOrDefault();
    if (IsExternalTriggerLevelPresent){
        ECHECK(rp_SetExternalTriggerLevel(_level));
    }
    return RP_OK;
}

auto CADCController::getExternalTriggerLevel(float *_level) -> int{
    std::lock_guard lock(m_acqMutex);
    static auto IsExternalTriggerLevelPresent = rp_HPGetIsExternalTriggerLevelPresentOrDefault();
    if (IsExternalTriggerLevelPresent){
        ECHECK(rp_GetExternalTriggerLevel(_level));
    }else{
        *_level = 0;
    }
    return RP_OK;
}