#pragma once

#include <memory>
#include <string>
#include "acq.h"
#include "rp.h"
#include "rp_hw_calib.h"

namespace rp_calib {

class CCalib {
   public:
    struct DataPass {
        int32_t ch[RP_CALIB_MAX_ADC_CHANNELS];
        double ch_gain[RP_CALIB_MAX_ADC_CHANNELS];
    };

    using Ptr = std::shared_ptr<CCalib>;
    static Ptr Create(COscilloscope::Ptr _acq);

    CCalib(COscilloscope::Ptr _acq);
    CCalib(const CCalib&) = delete;
    CCalib(CCalib&&) = delete;
    ~CCalib();

    auto resetCalibToZero() -> int;
    auto resetCalibToFactory() -> int;
    auto calib(std::string& _step, float _refdc) -> int;
    auto getCalibData() -> DataPass;
    auto restoreCalib() -> void;

   private:
    auto calib_board_z10(std::string& _step, float _refdc) -> int;
    auto calib_board_z20_4ch(std::string& _step, float _refdc) -> int;
    auto calib_board_z20_250_12(std::string& _step, float _refdc) -> int;

    auto getData(int skip_read) -> COscilloscope::DataPass;

    COscilloscope::Ptr m_acq;
    std::string m_current_step;
    rp_calib_params_t m_calib_parameters;
    rp_calib_params_t m_calib_parameters_old;
    DataPass m_pass_data;
    uint8_t m_channels;
};

}  // namespace rp_calib