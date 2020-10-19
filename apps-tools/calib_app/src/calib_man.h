#pragma once

#include <memory>
#include "rp.h"
#include "acq.h"

enum ClalibValue{
    ADC_CH1_OFF,
    ADC_CH2_OFF,
    ADC_CH1_GAIN,
    ADC_CH2_GAIN,
    DAC_CH1_OFF,
    DAC_CH2_OFF,
    DAC_CH1_GAIN,
    DAC_CH2_GAIN    
};

class CCalibMan {
    public:
                    using Ptr = std::shared_ptr<CCalibMan>;
                    static Ptr Create(COscilloscope::Ptr _acq);

                    CCalibMan(COscilloscope::Ptr _acq);
                    CCalibMan(const CCalibMan &) = delete;
                    CCalibMan(CCalibMan &&) = delete;
                    ~CCalibMan();
               void init();
                int readCalib();
               void updateCalib();
                int getCalibValue(ClalibValue _type);
                int setCalibValue(ClalibValue _type,int _value);
               void setModeLV_HV(rp_pinState_t _mode);
      rp_pinState_t getModeLV_HV();
                int enableGen(rp_channel_t _ch,bool _enable);
                int setFreq(rp_channel_t _ch,int _freq);
                int setAmp(rp_channel_t _ch,float _ampl);
                int setOffset(rp_channel_t _ch,float _offset);
                int setGenType(rp_channel_t _ch,int _type);
#ifdef Z20_250_12
               void setModeAC_DC(rp_acq_ac_dc_mode_t _mode);
rp_acq_ac_dc_mode_t getModeAC_DC();
               void setGenGain(rp_gen_gain_t _mode);
      rp_gen_gain_t getGenGain();           
#endif
             

private:
 COscilloscope::Ptr m_acq;
      rp_pinState_t m_currentGain;  
  rp_calib_params_t m_calib_parameters;
#ifdef Z20_250_12
rp_acq_ac_dc_mode_t m_currentAC_DC;
      rp_gen_gain_t m_currentGenGain;
#endif   
};