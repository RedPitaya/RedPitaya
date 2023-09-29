#include <limits.h>
#include <math.h>
#include "view_controller.h"
#include "common.h"

CViewController::CViewController():
    m_viewGridXCount(DIVISIONS_COUNT_X),
    m_viewGridYCount(DIVISIONS_COUNT_Y),
    m_viewSizeInPoints(VIEW_SIZE_DEFAULT),
    m_acqData(NULL),
    m_updateViewFromADCRequest(false),
    m_updateViewRequest(false),
    m_autoScale(false),
    m_timeScale(1),
    m_timeOffet(0),
    m_oscIsRunning(false),
    m_triggerState(false),
    m_ViewMode(NORMAL),
    m_capturedDecimation(RP_DEC_1)
{
    initView();
}

CViewController::~CViewController(){
    releaseView();
}

auto CViewController::getGridXCount() const -> uint16_t{
    return m_viewGridXCount;
}

auto CViewController::getGridYCount() const -> uint16_t{
    return m_viewGridYCount;
}

auto CViewController::getViewSize() const -> vsize_t{
    return m_viewSizeInPoints;
}

auto CViewController::setViewSize(vsize_t _size) -> void{
    m_viewSizeInPoints = _size;
}

auto CViewController::getSamplesPerDivision() const -> float{
    return (float)getViewSize() / (float)getGridXCount();
}

auto CViewController::initView() -> bool{
    std::lock_guard<std::mutex> lock(m_viewMutex);
    for(int i = 0; i < MAX_VIEW_CHANNELS;i++){
        m_view[i] = new float[VIEW_SIZE_MAX];
        if (m_view[i] == NULL)
            FATAL("Can't allocate enough memory")
    }

    m_acqData = rp_createBuffer(MAX_ADC_CHANNELS,ADC_BUFFER_SIZE,true,false,true);
    if (m_acqData == NULL)
        FATAL("Can't allocate enough memory")
    return RP_OK;
}

auto CViewController::releaseView() -> void{
    std::lock_guard<std::mutex> lock(m_viewMutex);
    for(int i = 0; i < MAX_VIEW_CHANNELS;i++){
        delete[] m_view[i];
    }
    rp_deleteBuffer(m_acqData);
}

auto CViewController::lockView() -> void{
    m_viewMutex.lock();
}

auto CViewController::unlockView() -> void{
    m_viewMutex.unlock();
}

auto CViewController::getView(rpApp_osc_source _channel) -> float*{
    return m_view[_channel];
}

auto CViewController::getAcqBuffers() -> buffers_t*{
    return m_acqData;
}

auto CViewController::getClock() -> double {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return ((double)tp.tv_sec * 1000.f) + ((double)tp.tv_nsec / 1000000.f);
}

auto CViewController::requestUpdateViewFromADC() -> void{
    m_updateViewFromADCRequest = true;
}

auto CViewController::isNeedUpdateViewFromADC() -> bool{
    return m_updateViewFromADCRequest;
}

auto CViewController::updateViewFromADCDone() -> void{
    m_updateViewFromADCRequest = false;
}

auto CViewController::requestUpdateView() -> void{
    m_updateViewRequest = true;
}

auto CViewController::updateViewDone() -> void{
    m_updateViewRequest = false;
}

auto CViewController::isNeedUpdateView() -> bool{
    return m_updateViewRequest;
}

auto CViewController::setAutoScale(bool _state) -> void{
    m_autoScale = _state;
}

auto CViewController::getAutoScale() -> bool{
    return m_autoScale;
}

auto CViewController::convertSamplesToTime(int32_t samples) -> double{
    static auto rate = getADCRate();
    /* Calculate time (including decimation) */
    uint32_t decimation;
    rp_AcqGetDecimationFactor(&decimation);
    return (double)samples * ((double)decimation / rate) * 1000 ;
}

auto CViewController::calculateTimeOut(float _timeScale) -> double{
    double timeout = MAX(0.1f , (2.f * _timeScale * (float)getGridXCount()));
    return timeout;
}

auto CViewController::viewIndexToTime(int _index) -> float{
    return indexToTime(_index - m_viewSizeInPoints / 2.0) + m_timeOffet;
}

auto CViewController::setTimeScale(float _scale) -> int{
    std::lock_guard<std::mutex> lock(m_viewMutex);
    m_timeScale = _scale;
    return RP_OK;
}

auto CViewController::getTimeScale() -> float{
    return m_timeScale;
}

auto CViewController::setTimeOffset(float _offset) -> int{
    std::lock_guard<std::mutex> lock(m_viewMutex);
    m_timeOffet = _offset;
    return RP_OK;
}

auto CViewController::getTimeOffset() -> float{
    return m_timeOffet;
}

auto CViewController::calculateDecimation(float _scale,rp_acq_decimation_t *_decimation) -> int{
    static double rate = getADCRate();
    float maxDeltaSample = rate * _scale / 1000.0f / getSamplesPerDivision();
    float ratio = (float) ADC_BUFFER_SIZE / (float) getViewSize();

    if (maxDeltaSample / 65536.0f > ratio) {
        return RP_EOOR;
    }

    // contition: viewBuffer cannot be larger than adcBuffer
    if (maxDeltaSample <= ratio) {
        *_decimation = RP_DEC_1;
    }
    else if (maxDeltaSample / 8.0f <= ratio) {
        *_decimation = RP_DEC_8;
    }
    else if (maxDeltaSample / 64.0f <= ratio) {
        *_decimation = RP_DEC_64;
    }
    else if (maxDeltaSample / 1024.0f <= ratio) {
        *_decimation = RP_DEC_1024;
    }
    else if (maxDeltaSample / 8192.0f <= ratio) {
        *_decimation = RP_DEC_8192;
    }
    else {
        *_decimation = RP_DEC_65536;
    }
    return RP_OK;
}

auto CViewController::getCurrentDecimation() -> rp_acq_decimation_t{
    rp_acq_decimation_t dec;
    if (calculateDecimation(getTimeScale(),&dec) != RP_OK){
        dec = RP_DEC_1;
    }
    return dec;
}


auto CViewController::clearView() -> void{
    std::lock_guard<std::mutex> lock(m_viewMutex);
    auto view = getView(RPAPP_OSC_SOUR_MATH);
    auto viewSize = getViewSize();
    for (vsize_t i = 0; i < viewSize; ++i) {
        view[i] = 0;
    }
}

auto CViewController::runOsc() -> void{
    m_oscIsRunning = true;
}

auto CViewController::stopOsc() -> void{
    m_oscIsRunning = false;
}

auto CViewController::isOscRun() const -> bool{
    return m_oscIsRunning;
}

auto CViewController::setTriggerState(bool _state) -> void{
    m_triggerState = _state;
}

auto CViewController::isTriggered() const -> bool{
    return m_triggerState;
}

auto CViewController::getViewMode() -> EViewMode{
    return m_ViewMode;
}

auto CViewController::setViewMode(EViewMode _mode) -> void{
    std::lock_guard<std::mutex> lock(m_viewMutex);
    m_ViewMode = _mode;
}

auto CViewController::getSampledAfterTriggerInView() -> uint32_t{
    auto decFactor = timeToIndexD(m_timeScale) / (double)getSamplesPerDivision();
    int posInPoints = ((m_timeOffet / m_timeScale) * getSamplesPerDivision());
    auto x = m_viewSizeInPoints/2.0 + posInPoints;
    return MIN(x * decFactor + 2,ADC_BUFFER_SIZE);
}

auto CViewController::setCapturedDecimation(rp_acq_decimation_t _dec) -> void{
    m_capturedDecimation = _dec;
}

auto CViewController::getCapturedDecimation() -> rp_acq_decimation_t{
    return m_capturedDecimation;
}

