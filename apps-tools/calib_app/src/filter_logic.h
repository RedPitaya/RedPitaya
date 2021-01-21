#pragma once

#include "rp.h"
#include "acq.h"
#include "calib_man.h"
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <algorithm>  
#include <vector>

class CFilter_logic {
    public:
    struct GridItem{
        int64_t index;
        rp_channel_t ch;
        uint32_t aa;
        uint32_t bb;
        double   value;
        double   value_raw;
        double   deviationFromAVG;
        double   lastValue;
        double   lastDeviation;
        bool     calculate; 
    };
 
        using Ptr = std::shared_ptr<CFilter_logic>;
         static Ptr Create(CCalibMan::Ptr _calib_man);
                    CFilter_logic(CCalibMan::Ptr _calib_man);
                    CFilter_logic(const CFilter_logic &) = delete;
                    CFilter_logic(CFilter_logic &&) = delete;

                    auto init(rp_channel_t _ch) -> void;
                    auto print() -> void;
                    auto sort() -> void;
                    auto setCalibParameters() -> int;
                    auto setCalculatedValue(COscilloscope::DataPassAutoFilter item) -> int;
                    auto nextSetupCalibParameters() -> int;
                    auto removeHalfCalib() -> void;
                    auto getCalibCount() -> int;
                    auto getCalibDone() -> int;
                    auto calcProgress() -> int;
                    auto setGoodCalibParameter() -> void;
                    auto calibPP(COscilloscope::DataPassAutoFilter item,float _nominal) -> int;
                    auto setCalibRef(float _value) -> void;
                    auto setCalibMode(int _mode) -> void;
    private:
        std::vector<GridItem> m_grid; 
        GridItem              m_lastGood;
        CCalibMan::Ptr        m_calib_man;
        rp_channel_t          m_channel;
        int64_t               m_index;
        double                m_percent;  
        int                   m_calibAmpl; // step calib amlitude 
        float                 m_oldcalibAmpl;
        int64_t               m_oldPP;
        int                   m_calibMode; // 0 - External, 1 - Internal
        float                 m_calibRef; 
};