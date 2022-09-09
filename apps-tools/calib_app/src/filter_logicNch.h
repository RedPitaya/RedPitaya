#pragma once

#include "rp.h"
#include "acq.h"
#include "calib_man.h"
#include "filter_logic.h"

class CFilter_logicNch {
public:
    using Ptr = std::shared_ptr<CFilter_logicNch>;
    static Ptr Create(CCalibMan::Ptr _calib_man);

    CFilter_logicNch(CCalibMan::Ptr _calib_man);
    CFilter_logicNch(const CFilter_logic &) = delete;
    CFilter_logicNch(CFilter_logic &&) = delete;
        
    auto init() -> void;
    auto print() -> void;
    auto setCalibParameters() -> int;
    auto setCalculatedValue(COscilloscope::DataPassAutoFilterSync item) -> void;
    auto getCalibCount() -> int;
    auto getCalibDone() -> int;
    auto removeHalfCalib() -> void;
    auto nextSetupCalibParameters() -> int;
    auto calcProgress() -> int;
    auto setGoodCalibParameterCh(rp_channel_t ch) -> void;    
    auto calibPPCh(rp_channel_t ch, COscilloscope::DataPassAutoFilterSync item,float _nominal) -> int;    
    auto setCalibRef(float _value) -> void;
    auto setCalibMode(int _mode) -> void;

private:

    CFilter_logic::Ptr m_fl[ADC_CHANNELS];        
    CCalibMan::Ptr     m_calib_man;
};