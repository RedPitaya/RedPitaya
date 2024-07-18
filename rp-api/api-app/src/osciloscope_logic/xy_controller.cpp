#include <limits.h>
#include <math.h>
#include "math/rp_math.h"
#include "xy_controller.h"
#include "common.h"

CXYController::CXYController():
    m_viewGridXCount(DIVISIONS_COUNT_X),
    m_viewGridYCount(DIVISIONS_COUNT_Y),
    m_isEnable(false)
{
    m_src_axis[0] = m_src_axis[1] = RPAPP_OSC_SOUR_CH1;

    setViewSize(VIEW_SIZE_DEFAULT);
}

CXYController::~CXYController(){
}

auto CXYController::getGridXCount() const -> uint16_t{
    return m_viewGridXCount;
}

auto CXYController::getGridYCount() const -> uint16_t{
    return m_viewGridYCount;
}

auto CXYController::getViewSize() const -> vsize_t{
    return m_viewSizeInPoints;
}

auto CXYController::setViewSize(vsize_t _size) -> void{
    m_viewSizeInPoints = _size;
    m_x_axis.resize(m_viewSizeInPoints);
    m_y_axis.resize(m_viewSizeInPoints);
}

auto CXYController::lockView() -> void{
    m_viewMutex.lock();
}

auto CXYController::unlockView() -> void{
    m_viewMutex.unlock();
}

auto CXYController::getXAxis() -> std::vector<float>*{
    return &m_x_axis;
}

auto CXYController::getYAxis() -> std::vector<float>*{
    return &m_y_axis;
}

auto CXYController::clearView() -> void{
    std::lock_guard lock(m_viewMutex);
    auto viewx = getXAxis();
    auto viewy = getYAxis();
    auto viewSize = getViewSize();
    for (vsize_t i = 0; i < viewSize; ++i) {
        (*viewx)[i] = 0;
        (*viewy)[i] = 0;
    }
}

auto CXYController::isEnable() -> bool{
    return m_isEnable;
}

auto CXYController::setEnable(bool _enable) -> void{
    if (m_isEnable != _enable){
        m_isEnable = _enable;
        if (m_isEnable){
            clearView();
        }
    }
}

auto CXYController::setSrcXAxis(rpApp_osc_source _channel) -> void{
    m_src_axis[0] = _channel;
}

auto CXYController::getSrcXAxis() -> rpApp_osc_source{
    return m_src_axis[0];
}

auto CXYController::setSrcYAxis(rpApp_osc_source _channel) -> void{
    m_src_axis[1] = _channel;
}

auto CXYController::getSrcYAxis() -> rpApp_osc_source{
    return m_src_axis[1];
}
