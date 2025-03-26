#pragma once

#include <memory>
#include "acq.h"
#include "common.h"
#include "rp.h"
#include "rp_hw_calib.h"

class CCalib {
   public:
    struct DataPass {
        int32_t ch[MAX_ADC_CHANNELS];
    };

    using Ptr = std::shared_ptr<CCalib>;
    static Ptr Create(COscilloscope::Ptr _acq);

    CCalib(COscilloscope::Ptr _acq);
    CCalib(const CCalib&) = delete;
    CCalib(CCalib&&) = delete;
    ~CCalib();

    auto resetCalibToZero() -> int;
    auto resetCalibToFactory() -> int;
    auto calib(uint16_t _step, float _refdc) -> int;
    auto getCalibData() -> DataPass;
    auto restoreCalib() -> void;

   private:
    auto calib_board_z10(uint16_t _step, float _refdc) -> int;
    auto calib_board_z20_4ch(uint16_t _step, float _refdc) -> int;
    auto calib_board_z20_250_12(uint16_t _step, float _refdc) -> int;

    auto getData(int skip_read) -> COscilloscope::DataPass;

    COscilloscope::Ptr m_acq;
    int m_current_step;
    rp_calib_params_t m_calib_parameters;
    rp_calib_params_t m_calib_parameters_old;
    DataPass m_pass_data;
    uint8_t m_channels;
};