#pragma once

#include <memory>
#include "rp.h"
#include "acq.h"

enum ClalibValue{
    ADC_CH_OFF,
    ADC_CH_GAIN,
    DAC_CH_OFF,
    DAC_CH_GAIN
#if defined Z10 || defined Z20_125 || Z20_125_4CH
    ,
    F_AA_CH,
    F_BB_CH,
    F_PP_CH,
    F_KK_CH
#endif
};

class CCalibMan {
public:
    using Ptr = std::shared_ptr<CCalibMan>;
    static Ptr Create(COscilloscope::Ptr _acq);

    CCalibMan(COscilloscope::Ptr _acq);
    CCalibMan(const CCalibMan &) = delete;
    CCalibMan(CCalibMan &&) = delete;
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
    auto getCalibValue(rp_channel_t _ch, ClalibValue _type) -> int;
    auto updateAcqFilter(rp_channel_t _ch) -> void;
    auto setCalibValue(rp_channel_t _ch, ClalibValue _type,int _value) -> int;
    auto setModeLV_HV(rp_pinState_t _mode) -> void;
    auto getModeLV_HV() ->rp_pinState_t;
    auto setDefualtFilter(rp_channel_t _ch) -> int;

#if defined Z20_250_12 || defined Z10 || defined Z20 || defined Z20_125
    auto enableGen(rp_channel_t _ch,bool _enable) -> int;
    auto setFreq(rp_channel_t _ch,int _freq) -> int;
    auto setAmp(rp_channel_t _ch,float _ampl) -> int;
    auto setOffset(rp_channel_t _ch,float _offset) -> int;
    auto setGenType(rp_channel_t _ch,int _type) -> int;
    auto updateGen() -> void;
#endif

#ifdef Z20_250_12
    auto setModeAC_DC(rp_acq_ac_dc_mode_t _mode) -> void;
    auto getModeAC_DC() -> rp_acq_ac_dc_mode_t;
    auto setGenGain(rp_gen_gain_t _mode) -> void;
    auto getGenGain() -> rp_gen_gain_t;
#endif
            

private:
                int m_calibMode;
 COscilloscope::Ptr m_acq;
      rp_pinState_t m_currentGain;  
  rp_calib_params_t m_calib_parameters;

#ifdef Z20_250_12
rp_acq_ac_dc_mode_t m_currentAC_DC;
      rp_gen_gain_t m_currentGenGain;
#endif   
};