#pragma once

#include <memory>
#include "acq.h"
#include "common.h"
#include "rp.h"
#include "rp_hw_calib.h"

enum ClalibValue { ADC_CH_OFF, ADC_CH_GAIN, DAC_CH_OFF, DAC_CH_GAIN, F_AA_CH, F_BB_CH, F_PP_CH, F_KK_CH };

class CCalibMan {
   public:
    using Ptr = std::shared_ptr<CCalibMan>;
    static Ptr Create(COscilloscope::Ptr _acq);

    CCalibMan(COscilloscope::Ptr _acq);
    CCalibMan(const CCalibMan&) = delete;
    CCalibMan(CCalibMan&&) = delete;
    ~CCalibMan();

    auto init() -> void;
    auto initSq(int _decimation) -> void;
    auto getCalibMode() -> int;
    auto changeDecimation(int _decimation) -> void;
    auto changeChannel(rp_channel_t _ch) -> void;
    auto readCalib() -> int;
    auto readCalibEpprom() -> int;
    auto updateCalib() -> void;
    auto writeCalib() -> void;
    auto getCalibValue(rp_channel_t _ch, ClalibValue _type) -> double;
    auto updateAcqFilter(rp_channel_t _ch) -> void;
    auto setCalibValue(rp_channel_t _ch, ClalibValue _type, double _value) -> int;
    auto setModeLV_HV(rp_pinState_t _mode) -> void;
    auto getModeLV_HV() -> rp_pinState_t;
    auto setDefualtFilter(rp_channel_t _ch) -> int;
    auto setDisableFilter(rp_channel_t _ch) -> int;

    auto enableGen(rp_channel_t _ch, bool _enable) -> int;
    auto setFreq(rp_channel_t _ch, int _freq) -> int;
    auto setAmp(rp_channel_t _ch, float _ampl) -> int;
    auto setOffset(rp_channel_t _ch, float _offset) -> int;
    auto setGenType(rp_channel_t _ch, int _type) -> int;
    auto updateGen() -> void;

    auto setModeAC_DC(rp_acq_ac_dc_mode_t _mode) -> void;
    auto getModeAC_DC() -> rp_acq_ac_dc_mode_t;
    auto setGenGain(rp_gen_gain_t _mode) -> void;
    auto getGenGain() -> rp_gen_gain_t;

   private:
    int m_calibMode;
    COscilloscope::Ptr m_acq;
    rp_pinState_t m_currentGain;
    rp_calib_params_t m_calib_parameters;

    rp_acq_ac_dc_mode_t m_currentAC_DC;
    rp_gen_gain_t m_currentGenGain;
};