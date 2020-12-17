#pragma once

#include "rp.h"
#include "acq.h"
#include "calib_man.h"
#include "filter_logic.h"

class CFilter_logic2ch {
    public:
         using Ptr = std::shared_ptr<CFilter_logic2ch>;
         static Ptr Create(CCalibMan::Ptr _calib_man);
                    CFilter_logic2ch(CCalibMan::Ptr _calib_man);
                    CFilter_logic2ch(const CFilter_logic &) = delete;
                    CFilter_logic2ch(CFilter_logic &&) = delete;
                    
                auto init() -> void;
                auto print() -> void;
                auto setCalibParameters() -> int;
                auto setCalculatedValue(COscilloscope::DataPassAutoFilter2Ch item) -> void;
                auto getCalibCount() -> int;
                auto getCalibDone() -> int;
                auto removeHalfCalib() -> void;
                auto nextSetupCalibParameters() -> int;
                auto calcProgress() -> int;
                auto setGoodCalibParameterCh1() -> void;
                auto setGoodCalibParameterCh2() -> void;
                auto calibPPCh1(COscilloscope::DataPassAutoFilter2Ch item,float _nominal) -> int;
                auto calibPPCh2(COscilloscope::DataPassAutoFilter2Ch item,float _nominal) -> int;
                auto setCalibRef(float _value) -> void;
                auto setCalibMode(int _mode) -> void;

    private:
        CFilter_logic::Ptr m_fl1;
        CFilter_logic::Ptr m_fl2;
        CCalibMan::Ptr     m_calib_man;
};