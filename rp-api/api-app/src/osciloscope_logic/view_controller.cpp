#include <limits.h>
#include <math.h>
#include "view_controller.h"
#include "common.h"

CViewController::CViewController():
    m_viewGridXCount(DIVISIONS_COUNT_X),
    m_viewGridYCount(DIVISIONS_COUNT_Y),
    m_viewSizeInPoints(VIEW_SIZE_DEFAULT),
    m_acqData(NULL),
    m_dataHasTrigger(false),
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
    setViewSize(VIEW_SIZE_DEFAULT);
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
    std::lock_guard<std::mutex> lock(m_viewMutex);
    m_viewSizeInPoints = _size;
    for(int i = 0; i < MAX_VIEW_CHANNELS;i++){
        m_view[i].resize(m_viewSizeInPoints);
        m_origialData[i].resize(ADC_BUFFER_SIZE);
    }
}

auto CViewController::getSamplesPerDivision() const -> float{
    return (float)getViewSize() / (float)getGridXCount();
}

auto CViewController::initView() -> bool{
    std::lock_guard<std::mutex> lock(m_viewMutex);

    m_acqData = rp_createBuffer(MAX_ADC_CHANNELS,ADC_BUFFER_SIZE,true,false,true);
    if (m_acqData == NULL)
        FATAL("Can't allocate enough memory")
    return RP_OK;
}

auto CViewController::releaseView() -> void{
    std::lock_guard<std::mutex> lock(m_viewMutex);
    rp_deleteBuffer(m_acqData);
}

auto CViewController::lockView() -> void{
    m_viewMutex.lock();
}

auto CViewController::unlockView() -> void{
    m_viewMutex.unlock();
}

auto CViewController::getView(rpApp_osc_source _channel) -> std::vector<float>*{
    return &m_view[_channel];
}

auto CViewController::getOriginalData(rpApp_osc_source _channel) -> std::vector<float>*{
    return &m_origialData[_channel];
}

auto CViewController::getAcqBuffers() -> buffers_t*{
    return m_acqData;
}

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
    ratio *= 0.9;
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
    return RP_OK;
}

auto CViewController::getCurrentDecimation(bool _continuesMode) -> uint32_t{
    uint32_t dec;
    if (calculateDecimation(getTimeScale(),&dec,_continuesMode) != RP_OK){
        dec = RP_DEC_1;
    }
    return dec;
}


auto CViewController::clearView() -> void{
    std::lock_guard<std::mutex> lock(m_viewMutex);
    auto view = getView(RPAPP_OSC_SOUR_MATH);
    auto viewSize = getViewSize();
    for (vsize_t i = 0; i < viewSize; ++i) {
        (*view)[i] = 0;
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

auto CViewController::setDataWithTrigger(bool _state) -> void{
    m_dataHasTrigger = _state;
}

auto CViewController::isDataWithTrigger() -> bool{
    return m_dataHasTrigger;
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
    auto extraPoints = calcExtraPoints();
    return MIN(x * decFactor + extraPoints,ADC_BUFFER_SIZE);
}

auto CViewController::calcExtraPoints() -> uint32_t{
    auto decFactor = timeToIndexD(m_timeScale) / (double)getSamplesPerDivision();
    return floor((float)ADC_BUFFER_SIZE / (float)getViewSize()) * decFactor + 2;
}

auto CViewController::setCapturedDecimation(uint32_t _dec) -> void{
    m_capturedDecimation = _dec;
}

auto CViewController::getCapturedDecimation() -> uint32_t{
    return m_capturedDecimation;
}


auto CViewController::isSine(rpApp_osc_source _channel) -> bool{

    auto trapezoidalApprox = [](double *data, float T, int size) -> float{
        double result = 0;
        for(int i = 0; i < size - 1; i++){
            result += data[i] + data[i+1];
        }
        result = ((T / 2.0) * result);
        return result;
    };

    auto isSineTester = [=](float *data, uint32_t size) -> bool{
            static double rate = getADCRate();
            double T = (m_capturedDecimation / rate);
            double ch_rms[VIEW_SIZE_MAX];
            double ch_avr[VIEW_SIZE_MAX];
            for(uint32_t i = 0; i < size; i++) {
                    ch_rms[i] = data[i] * data[i];
                    ch_avr[i] = fabs(data[i]);
            }
            double K0 = sqrtf(T * size * trapezoidalApprox(ch_rms, T, size)) / trapezoidalApprox(ch_avr, T, size);
            return ((K0 > 1.10) && (K0 < 1.12));
    };

    std::lock_guard<std::mutex> lock(m_viewMutex);
    auto view = getView(_channel);
    auto viewSize = getViewSize();

    return isSineTester(view->data(),viewSize);
}
