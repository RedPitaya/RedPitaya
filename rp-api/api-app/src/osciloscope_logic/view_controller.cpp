#include <limits.h>
#include <math.h>
#include "view_controller.h"
#include "common.h"

CViewController::CViewController():
    m_viewGridXCount(DIVISIONS_COUNT_X),
    m_viewGridYCount(DIVISIONS_COUNT_Y),
    m_viewSizeInPoints(VIEW_SIZE_DEFAULT),
    m_updateViewFromADCRequest(false),
    m_updateViewRequest(false),
    m_autoScale(false),
    m_timeScale(1),
    m_timeOffet(0),
    m_oscIsRunning(false),
    m_triggerState(false),
    m_ViewMode(NORMAL),
    m_oscPerSec(0),
    m_oscPerSecCounter(0)
{
    initView();
    prepareOscillogramBuffer(DEFAULT_OSCILOGRAMM_BUFFERS);
    setViewSize(VIEW_SIZE_DEFAULT);
    m_currentBuffer = 0;
    m_lastTimeCapture = std::chrono::system_clock::now();
}

CViewController::~CViewController(){
    releaseView();
}

CViewController::Oscillogram::Oscillogram(){
    auto *m_acqData = rp_createBuffer(MAX_ADC_CHANNELS,ADC_BUFFER_SIZE,true,false,true);
    if (m_acqData == NULL){
        FATAL("Can't allocate enough memory")
    }
    m_data = m_acqData;
}

CViewController::Oscillogram::~Oscillogram(){
    std::lock_guard lock(m_viewMutex); // Wait for release data
    rp_deleteBuffer(m_data); ;
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

auto CViewController::prepareOscillogramBuffer(size_t _maxBuffers) -> void{
    if (_maxBuffers == 0) FATAL("Buffer cannot be zero length")
    std::lock_guard lock(m_viewControllerMutex);
    for(size_t i = 0; i < m_origialData.size(); i++){
        delete m_origialData[i];
    }
    m_origialData.resize(_maxBuffers);
    for(size_t i = 0; i < m_origialData.size(); i++){
        m_origialData[i] = new Oscillogram();
        m_origialData[i]->m_index = i + 1;
    }
    resetCurrentBuffer();
}

auto CViewController::resetCurrentBuffer() -> void{
    m_currentBuffer = 0;
}

auto CViewController::nextBuffer() -> void{
    m_currentBuffer = (m_currentBuffer + 1) % m_origialData.size();
    m_stoppedBuffer = m_currentBuffer;
}

auto CViewController::getOscillogramBufferCount() -> size_t{
    return m_origialData.size();
}

auto CViewController::getCurrentOscillogram() -> Oscillogram*{
    return m_origialData[m_currentBuffer];
}

auto CViewController::getOscillogramForView() -> Oscillogram*{
    auto viewBuff = m_currentBuffer;
    if (viewBuff == 0){
        viewBuff = m_origialData.size() - 1;
    }else
        viewBuff--;
    return m_origialData[viewBuff];
}

auto CViewController::setViewSize(vsize_t _size) -> void{
    std::lock_guard lock(m_viewMutex);
    m_viewSizeInPoints = _size;
    for(int i = 0; i < MAX_VIEW_CHANNELS;i++){
        m_view[i].resize(m_viewSizeInPoints);
        m_viewInfo[i] = OscillogramInfo();
    }
}

auto CViewController::getSamplesPerDivision() const -> float{
    return (float)getViewSize() / (float)getGridXCount();
}

auto CViewController::initView() -> bool{
    std::lock_guard lock(m_viewControllerMutex);
    return RP_OK;
}

auto CViewController::releaseView() -> void{
    std::lock_guard lock(m_viewControllerMutex);
    for(size_t i = 0; i < m_origialData.size(); i++){
        delete m_origialData[i];
        m_origialData[i] = NULL;
    }
    m_currentBuffer = 0;
}

auto CViewController::getView(rpApp_osc_source _channel) -> std::vector<float>*{
    return &m_view[_channel];
}

auto CViewController::lockScreenView() -> void{
    m_viewMutex.lock();
}

auto CViewController::unlockScreenView() -> void{
    m_viewMutex.unlock();
}

auto CViewController::lockControllerView() -> void{
    m_viewControllerMutex.lock();
}

auto CViewController::unlockControllerView() -> void{
    m_viewControllerMutex.unlock();
}

auto CViewController::getOriginalData(rpApp_osc_source _channel) -> std::vector<float>*{
    return &m_viewRaw[_channel];
}

auto CViewController::getViewInfo(rpApp_osc_source _channel) -> OscillogramInfo*{
    return &m_viewInfo[_channel];
}


// auto CViewController::getAcqBuffers() -> buffers_t*{
//     return m_acqData;
// }

// Return value in milliseconds 1.0 = 1ms
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

// Calculate in milliseconds
auto CViewController::calculateTimeOut(float _timeScale) -> double{
    double timeout = MAX(0.1f , (2.f * _timeScale * (float)getGridXCount()));
    return timeout;
}

auto CViewController::viewIndexToTime(int _index) -> float{
    return indexToTime(_index - m_viewSizeInPoints / 2.0) + m_timeOffet;
}

auto CViewController::setTimeScale(float _scale) -> int{
    std::lock_guard lock(m_viewControllerMutex);
    m_timeScale = _scale;
    return RP_OK;
}

auto CViewController::getTimeScale() -> float{
    return m_timeScale;
}

auto CViewController::setTimeOffset(float _offset) -> int{
    std::lock_guard<std::mutex> lock(m_viewControllerMutex);
    m_timeOffet = _offset;
    return RP_OK;
}

auto CViewController::getTimeOffset() -> float{
    return m_timeOffet;
}

auto CViewController::calculateDecimation(float _scale,uint32_t *_decimation,bool _continuesMode) -> int{
    static double rate = getADCRate();
    float maxDeltaSample = rate * _scale / 1000.0f / getSamplesPerDivision();
    float ratio = (float) ADC_BUFFER_SIZE / (float) getViewSize();

    if (maxDeltaSample / 65536.0f > ratio) {
        return RP_EOOR;
    }
    TRACE_SHORT("maxDeltaSample %f ratio %f _scale %f rate %f",maxDeltaSample,ratio,_scale,rate)
    // contition: viewBuffer cannot be larger than adcBuffer
  //  ratio *= _continuesMode ? 1.0 : 0.9;
    ratio *= 0.5;
    *_decimation = std::numeric_limits<uint32_t>::max();
    if (maxDeltaSample <= ratio) {
        *_decimation = RP_DEC_1;
    }
    else if (maxDeltaSample / 4.0f <= ratio) {
        *_decimation = RP_DEC_4;
    }
    else if (maxDeltaSample / 8.0f <= ratio) {
        *_decimation = RP_DEC_8;
    }
    else {
        for(int i = 16; i <= RP_DEC_65536;i++){
            if (maxDeltaSample / (float)i <= ratio){
                *_decimation = i;
                break;
            }
        }
    }
    if (*_decimation == std::numeric_limits<uint32_t>::max())
        *_decimation = RP_DEC_65536;
    return RP_OK;
}

auto CViewController::getCurrentDecimation(bool _continuesMode) -> uint32_t{
    uint32_t dec;
    if (calculateDecimation(getTimeScale(),&dec,_continuesMode) != RP_OK){
        dec = RP_DEC_1;
    }
    return dec;
}


// auto CViewController::clearView() -> void{
//     std::lock_guard lock(m_viewControllerMutex);
//     auto view = getView(RPAPP_OSC_SOUR_MATH);
//     auto viewSize = getViewSize();
//     for (vsize_t i = 0; i < viewSize; ++i) {
//         (*view)[i] = 0;
//     }
// }

auto CViewController::runOsc() -> void{
    m_oscIsRunning = true;
}

auto CViewController::stopOsc() -> void{
    m_oscIsRunning = false;
}

auto CViewController::isOscRun() const -> bool{
    return m_oscIsRunning;
}

auto CViewController::bufferSelectNext() -> void{
    if (!isOscRun()){
        auto size = m_origialData.size();
        if (m_stoppedBuffer != m_currentBuffer){
            m_currentBuffer = (m_currentBuffer + 1) % size;
        }
    }
}

auto CViewController::bufferSelectPrev() -> void{
    if (!isOscRun()){
        auto size = m_origialData.size();
        auto stoppedBuffer = (m_stoppedBuffer + 1) % size;
        if (stoppedBuffer != m_currentBuffer){
            m_currentBuffer = (size + m_currentBuffer - 1) % size;
        }
    }
}

auto CViewController::bufferCurrent(int32_t *current) -> void{
    auto size = m_origialData.size();
    auto x = m_currentBuffer > m_stoppedBuffer ? m_currentBuffer - size : m_currentBuffer;
    *current = (x - m_stoppedBuffer);
}

auto CViewController::setTriggerState(bool _state) -> void{
    m_triggerState = _state;
}

auto CViewController::isTriggered() const -> bool{
    return m_triggerState;
}

// auto CViewController::setDataWithTrigger(bool _state) -> void{
//     m_dataHasTrigger = _state;
// }

// auto CViewController::isDataWithTrigger() -> bool{
//     return m_dataHasTrigger;
// }

auto CViewController::getViewMode() -> EViewMode{
    return m_ViewMode;
}

auto CViewController::setViewMode(EViewMode _mode) -> void{
    std::lock_guard lock(m_viewMutex);
    m_ViewMode = _mode;
}

auto CViewController::getSampledAfterTriggerInView() -> uint32_t{
    auto decFactor = timeToIndexD(m_timeScale) / (double)getSamplesPerDivision();
    int posInPoints = ((m_timeOffet / m_timeScale) * getSamplesPerDivision());
    auto x = m_viewSizeInPoints/2.0 + posInPoints;
    auto extraPoints = calcExtraPoints();
    return MIN(x * decFactor + extraPoints,ADC_BUFFER_SIZE);
}

auto CViewController::calcExtraPoints() -> uint32_t{
    auto decFactor = timeToIndexD(m_timeScale) / (double)getSamplesPerDivision();
    return (floor((float)ADC_BUFFER_SIZE / (float)getViewSize())) * decFactor + 4.0;
}

// auto CViewController::setCapturedDecimation(uint32_t _dec) -> void{
//     m_capturedDecimation = _dec;
// }

// auto CViewController::getCapturedDecimation() -> uint32_t{
//     return m_capturedDecimation;
// }

auto CViewController::addOscCounter() -> void{
    auto now = std::chrono::system_clock::now();
    auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto lastTime = std::chrono::time_point_cast<std::chrono::milliseconds>(m_lastTimeCapture);
    if ((curTime - lastTime).count() > 1000){
        m_lastTimeCapture = curTime;
        m_oscPerSec = m_oscPerSecCounter;
        m_oscPerSecCounter = 0;

    }else{
        m_oscPerSecCounter++;
    }

}

auto CViewController::getOscPerSec() -> uint32_t{
    return m_oscPerSec;
}