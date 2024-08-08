#ifndef __XY_CONTROLLER_H
#define __XY_CONTROLLER_H

#include <stdint.h>
#include <mutex>
#include <atomic>
#include <vector>

#include "rpApp.h"
#include "constants.h"

class CXYController{

public:

    CXYController();
    ~CXYController();

    CXYController(CXYController &) = delete;
    CXYController(CXYController &&) = delete;

    auto getGridXCount() const -> uint16_t;
    auto getGridYCount() const -> uint16_t;

    auto getViewSize() const -> vsize_t;
    auto setViewSize(vsize_t _size) -> void;

    auto lockView() -> void;
    auto unlockView() -> void;

    auto getXAxis() -> std::vector<float>*;
    auto getYAxis() -> std::vector<float>*;

    auto clearView() -> void;

    auto isEnable() -> bool;
    auto setEnable(bool _enable) -> void;

    auto setSrcXAxis(rpApp_osc_source _channel) -> void;
    auto getSrcXAxis() -> rpApp_osc_source;

    auto setSrcYAxis(rpApp_osc_source _channel) -> void;
    auto getSrcYAxis() -> rpApp_osc_source;

private:

    uint16_t m_viewGridXCount;
    uint16_t m_viewGridYCount;
    vsize_t  m_viewSizeInPoints;

    std::vector<float> m_x_axis;
    std::vector<float> m_y_axis;

    std::mutex m_viewMutex;

    bool m_isEnable;
    rpApp_osc_source m_src_axis[2];
};

#endif // __XY_CONTROLLER_H