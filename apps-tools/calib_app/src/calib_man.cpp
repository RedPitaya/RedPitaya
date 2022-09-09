#include <fstream>
#include <ctime>
#include "calib_man.h"


float calibFullScaleToVoltageMan(uint32_t fullScaleGain) {
    if (fullScaleGain == 0) return 1;
    return (float) ((float)fullScaleGain  * 100.0 / ((uint64_t)1<<32));
}

uint32_t calibFullScaleFromVoltageMan(float voltageScale) {
    return (uint32_t) (voltageScale / 100.0 * ((uint64_t)1<<32));
}


CCalibMan::Ptr CCalibMan::Create(COscilloscope::Ptr _acq)
{
    return std::make_shared<CCalibMan>(_acq);
}

CCalibMan::CCalibMan(COscilloscope::Ptr _acq):
m_acq(_acq),
m_calibMode(0)
{
    m_currentGain = RP_LOW;
#ifdef Z20_250_12
    m_currentAC_DC = RP_DC;
    m_currentGenGain = RP_GAIN_1X;
#endif
    readCalib();
}

CCalibMan::~CCalibMan()
{
}

void CCalibMan::init(){
    m_acq->startNormal();
#if defined Z20_250_12 || defined Z10 || defined Z20 || defined Z20_125
    m_acq->resetGen();
#endif    
    m_currentGain = RP_LOW;
    m_acq->setLV();
#ifdef Z20_250_12
    m_currentAC_DC = RP_DC;
    m_acq->setDC();
    m_currentGenGain = RP_GAIN_1X;
    m_acq->setGenGainx1();
#endif
    readCalib();
    m_calibMode = 0;
}

void CCalibMan::initSq(int _decimation){
    m_acq->startSquare(_decimation);
	m_acq->setHyst(0.05);
    this->setModeLV_HV(RP_LOW);	
	this->changeChannel(RP_CH_1);
#if defined Z20_250_12 || defined Z10 || defined Z20 || defined Z20_125
    setGenType(RP_CH_1,(int)RP_WAVEFORM_SQUARE);
    setGenType(RP_CH_2,(int)RP_WAVEFORM_SQUARE);
    enableGen(RP_CH_1,false);
    enableGen(RP_CH_2,false);
#endif    
    readCalib();
    m_calibMode = 1;
}

int CCalibMan::getCalibMode(){
    return m_calibMode;
}

void CCalibMan::changeDecimation(int _decimation){
    m_acq->startSquare(_decimation);
}

void CCalibMan::changeChannel(rp_channel_t _ch){
    m_acq->setAcquireChannel(_ch);
}

int CCalibMan::readCalib(){
    m_calib_parameters = rp_GetCalibrationSettings();
    return 0;
}

int CCalibMan::readCalibEpprom(){
    rp_CalibInit();
    m_calib_parameters = rp_GetCalibrationSettings();
    return 0;
}

void CCalibMan::updateCalib(){
    rp_CalibrationSetParams(m_calib_parameters);
}

void CCalibMan::writeCalib(){
    readCalib();
    rp_CalibrationWriteParams(m_calib_parameters);
}

void CCalibMan::setModeLV_HV(rp_pinState_t _mode){
    m_currentGain = _mode;
    if (m_currentGain == RP_LOW){
        m_acq->setLV();
    }
    if (m_currentGain == RP_HIGH){
        m_acq->setHV();
    }
}

rp_pinState_t CCalibMan::getModeLV_HV(){
    return m_currentGain;
}

#ifdef Z20_250_12
void CCalibMan::setModeAC_DC(rp_acq_ac_dc_mode_t _mode){
    m_currentAC_DC = _mode;
    if (m_currentAC_DC == RP_DC){
        m_acq->setDC();
    }
    if (m_currentAC_DC == RP_AC){
        m_acq->setAC();
    }
}

rp_acq_ac_dc_mode_t CCalibMan::getModeAC_DC(){
    return m_currentAC_DC;
}

void CCalibMan::setGenGain(rp_gen_gain_t _mode){
    m_currentGenGain = _mode;
    if (m_currentGenGain == RP_GAIN_1X){
        m_acq->setGenGainx1();
    }
    if (m_currentGenGain == RP_GAIN_5X){
        m_acq->setGenGainx5();
    }
}

rp_gen_gain_t CCalibMan::getGenGain(){
    return m_currentGenGain; 
}           
#endif

int CCalibMan::getCalibValue(rp_channel_t ch,ClalibValue _type){
    auto g = getModeLV_HV();
#ifdef Z20_250_12
    auto ac_dc = getModeAC_DC();
    auto gen_g = getGenGain();
    switch (_type)
    {
        case ADC_CH_OFF:  {
            if (ch == RP_CH_1){
                if (g == RP_LOW)  return  (ac_dc == RP_DC) ? m_calib_parameters.osc_ch1_off_1_dc  : m_calib_parameters.osc_ch1_off_1_ac;
                if (g == RP_HIGH) return  (ac_dc == RP_DC) ? m_calib_parameters.osc_ch1_off_20_dc : m_calib_parameters.osc_ch1_off_20_ac;
            }
            if (ch == RP_CH_2){
                if (g == RP_LOW)  return  (ac_dc == RP_DC) ? m_calib_parameters.osc_ch2_off_1_dc  : m_calib_parameters.osc_ch2_off_1_ac;
                if (g == RP_HIGH) return  (ac_dc == RP_DC) ? m_calib_parameters.osc_ch2_off_20_dc : m_calib_parameters.osc_ch2_off_20_ac;
            }
            break;
        }
        
        case ADC_CH_GAIN: {
            if (ch == RP_CH_1){
                if (g == RP_LOW)  return  (ac_dc == RP_DC) ? m_calib_parameters.osc_ch1_g_1_dc  : m_calib_parameters.osc_ch1_g_1_ac;
                if (g == RP_HIGH) return  (ac_dc == RP_DC) ? m_calib_parameters.osc_ch1_g_20_dc : m_calib_parameters.osc_ch1_g_20_ac;
            }
            if (ch == RP_CH_2){
                if (g == RP_LOW)  return  (ac_dc == RP_DC) ? m_calib_parameters.osc_ch2_g_1_dc  : m_calib_parameters.osc_ch2_g_1_ac;
                if (g == RP_HIGH) return  (ac_dc == RP_DC) ? m_calib_parameters.osc_ch2_g_20_dc : m_calib_parameters.osc_ch2_g_20_ac;
            }
            break;
        }

        case DAC_CH_OFF:  {
            if (ch == RP_CH_1)
                return  (gen_g == RP_GAIN_1X) ? m_calib_parameters.gen_ch1_off_1  : m_calib_parameters.gen_ch1_off_5;
            if (ch == RP_CH_2)
                return  (gen_g == RP_GAIN_1X) ? m_calib_parameters.gen_ch2_off_1  : m_calib_parameters.gen_ch2_off_5;
        }        
        case DAC_CH_GAIN: {
            if (ch == RP_CH_1)
                return  (gen_g == RP_GAIN_1X) ? m_calib_parameters.gen_ch1_g_1  : m_calib_parameters.gen_ch1_g_5;
            if (ch == RP_CH_2)
                return  (gen_g == RP_GAIN_1X) ? m_calib_parameters.gen_ch2_g_1  : m_calib_parameters.gen_ch2_g_5;
        }        
    }
#endif

#if defined Z10 || defined Z20_125
    switch (_type)
    {
        case ADC_CH_OFF:  {
            if (ch == RP_CH_1){
                if (g == RP_LOW)  return m_calib_parameters.fe_ch1_lo_offs;
                if (g == RP_HIGH) return m_calib_parameters.fe_ch1_hi_offs;
            }
            if (ch == RP_CH_2){
                if (g == RP_LOW)  return m_calib_parameters.fe_ch2_lo_offs;
                if (g == RP_HIGH) return m_calib_parameters.fe_ch2_hi_offs;
            }
            break;
        }

        case ADC_CH_GAIN: {
            if (ch == RP_CH_1){
                if (g == RP_LOW)  return m_calib_parameters.fe_ch1_fs_g_lo;
                if (g == RP_HIGH) return m_calib_parameters.fe_ch1_fs_g_hi;
            }
            if (ch == RP_CH_2){
                if (g == RP_LOW)  return m_calib_parameters.fe_ch2_fs_g_lo;
                if (g == RP_HIGH) return m_calib_parameters.fe_ch2_fs_g_hi;
            }
            break;
        }
       
        case DAC_CH_OFF:  {
            if (ch == RP_CH_1)
                return m_calib_parameters.be_ch1_dc_offs;
            if (ch == RP_CH_2)
                return m_calib_parameters.be_ch2_dc_offs;
        }
        
        case DAC_CH_GAIN: {
            if (ch == RP_CH_1)
                return m_calib_parameters.be_ch1_fs;
            if (ch == RP_CH_2)
                return m_calib_parameters.be_ch2_fs;
        }
        
        case F_AA_CH: {
            if (ch == RP_CH_1) {
                if (g == RP_LOW)  return m_calib_parameters.low_filter_aa_ch1;
                if (g == RP_HIGH) return m_calib_parameters.hi_filter_aa_ch1;
            }
            if (ch == RP_CH_2) {
                if (g == RP_LOW)  return m_calib_parameters.low_filter_aa_ch2;
                if (g == RP_HIGH) return m_calib_parameters.hi_filter_aa_ch2;
            }
            break;
        }
        
        case F_BB_CH: {
            if (ch == RP_CH_1) {
                if (g == RP_LOW)  return m_calib_parameters.low_filter_bb_ch1;
                if (g == RP_HIGH) return m_calib_parameters.hi_filter_bb_ch1;
            }
            if (ch == RP_CH_2) {
                if (g == RP_LOW)  return m_calib_parameters.low_filter_bb_ch2;
                if (g == RP_HIGH) return m_calib_parameters.hi_filter_bb_ch2;
            }
            break;
        }
        
        case F_PP_CH: {
            if (ch == RP_CH_1) {
                if (g == RP_LOW)  return m_calib_parameters.low_filter_pp_ch1;
                if (g == RP_HIGH) return m_calib_parameters.hi_filter_pp_ch1;
            }
            if (ch == RP_CH_2) {
                if (g == RP_LOW)  return m_calib_parameters.low_filter_pp_ch2;
                if (g == RP_HIGH) return m_calib_parameters.hi_filter_pp_ch2;
            }
            break;
        }
        
         case F_KK_CH: {
            if (ch == RP_CH_1) {
                if (g == RP_LOW)  return m_calib_parameters.low_filter_kk_ch1;
                if (g == RP_HIGH) return m_calib_parameters.hi_filter_kk_ch1;
            }
            if (ch == RP_CH_1) {
                if (g == RP_LOW)  return m_calib_parameters.low_filter_kk_ch2;
                if (g == RP_HIGH) return m_calib_parameters.hi_filter_kk_ch2;
            }
            break;
        }        
    }
#endif
#if defined Z20_125_4CH
switch (_type)
    {
        case ADC_CH_OFF:  {
            if (ch == RP_CH_1){
                if (g == RP_LOW)  return m_calib_parameters.chA_low_offs;
                if (g == RP_HIGH) return m_calib_parameters.chA_hi_offs;
            }
            if (ch == RP_CH_2){
                if (g == RP_LOW)  return m_calib_parameters.chB_low_offs;
                if (g == RP_HIGH) return m_calib_parameters.chB_hi_offs;
            }
            if (ch == RP_CH_3){
                if (g == RP_LOW)  return m_calib_parameters.chC_low_offs;
                if (g == RP_HIGH) return m_calib_parameters.chC_hi_offs;
            }
            if (ch == RP_CH_4){
                if (g == RP_LOW)  return m_calib_parameters.chD_low_offs;
                if (g == RP_HIGH) return m_calib_parameters.chD_hi_offs;
            }

            break;
        }

        case ADC_CH_GAIN: {
            if (ch == RP_CH_1){
                if (g == RP_LOW)  return m_calib_parameters.chA_g_low;
                if (g == RP_HIGH) return m_calib_parameters.chA_g_hi;
            }
            if (ch == RP_CH_2){
                if (g == RP_LOW)  return m_calib_parameters.chB_g_low;
                if (g == RP_HIGH) return m_calib_parameters.chB_g_hi;
            }
            if (ch == RP_CH_3){
                if (g == RP_LOW)  return m_calib_parameters.chC_g_low;
                if (g == RP_HIGH) return m_calib_parameters.chC_g_hi;
            }
            if (ch == RP_CH_4){
                if (g == RP_LOW)  return m_calib_parameters.chD_g_low;
                if (g == RP_HIGH) return m_calib_parameters.chD_g_hi;
            }
            break;
        }
               
        case F_AA_CH: {
            if (ch == RP_CH_1) {
                if (g == RP_LOW)  return m_calib_parameters.chA_low_aa;
                if (g == RP_HIGH) return m_calib_parameters.chA_hi_aa;
            }
            if (ch == RP_CH_2) {
                if (g == RP_LOW)  return m_calib_parameters.chB_low_aa;
                if (g == RP_HIGH) return m_calib_parameters.chB_hi_aa;
            }
            if (ch == RP_CH_3) {
                if (g == RP_LOW)  return m_calib_parameters.chC_low_aa;
                if (g == RP_HIGH) return m_calib_parameters.chC_hi_aa;
            }
            if (ch == RP_CH_4) {
                if (g == RP_LOW)  return m_calib_parameters.chD_low_aa;
                if (g == RP_HIGH) return m_calib_parameters.chD_hi_aa;
            }
            break;
        }
        
        case F_BB_CH: {
            if (ch == RP_CH_1) {
                if (g == RP_LOW)  return m_calib_parameters.chA_low_bb;
                if (g == RP_HIGH) return m_calib_parameters.chA_hi_bb;
            }
            if (ch == RP_CH_2) {
                if (g == RP_LOW)  return m_calib_parameters.chB_low_bb;
                if (g == RP_HIGH) return m_calib_parameters.chB_hi_bb;
            }
            if (ch == RP_CH_3) {
                if (g == RP_LOW)  return m_calib_parameters.chC_low_bb;
                if (g == RP_HIGH) return m_calib_parameters.chC_hi_bb;
            }
            if (ch == RP_CH_4) {
                if (g == RP_LOW)  return m_calib_parameters.chD_low_bb;
                if (g == RP_HIGH) return m_calib_parameters.chD_hi_bb;
            }
            break;
        }
        
        case F_PP_CH: {
            if (ch == RP_CH_1) {
                if (g == RP_LOW)  return m_calib_parameters.chA_low_pp;
                if (g == RP_HIGH) return m_calib_parameters.chA_hi_pp;
            }
            if (ch == RP_CH_2) {
                if (g == RP_LOW)  return m_calib_parameters.chB_low_pp;
                if (g == RP_HIGH) return m_calib_parameters.chB_hi_pp;
            }
            if (ch == RP_CH_3) {
                if (g == RP_LOW)  return m_calib_parameters.chC_low_pp;
                if (g == RP_HIGH) return m_calib_parameters.chC_hi_pp;
            }
            if (ch == RP_CH_4) {
                if (g == RP_LOW)  return m_calib_parameters.chD_low_pp;
                if (g == RP_HIGH) return m_calib_parameters.chD_hi_pp;
            }
            break;
        }
        
         case F_KK_CH: {
            if (ch == RP_CH_1) {
                if (g == RP_LOW)  return m_calib_parameters.chA_low_kk;
                if (g == RP_HIGH) return m_calib_parameters.chA_hi_kk;
            }
            if (ch == RP_CH_2) {
                if (g == RP_LOW)  return m_calib_parameters.chB_low_kk;
                if (g == RP_HIGH) return m_calib_parameters.chB_hi_kk;
            }
            if (ch == RP_CH_3) {
                if (g == RP_LOW)  return m_calib_parameters.chC_low_kk;
                if (g == RP_HIGH) return m_calib_parameters.chC_hi_kk;
            }
            if (ch == RP_CH_4) {
                if (g == RP_LOW)  return m_calib_parameters.chD_low_kk;
                if (g == RP_HIGH) return m_calib_parameters.chD_hi_kk;
            }
            break;
        }        
    }
#endif
    return 0;
}

int setCalibInt(int32_t *_x, int _value){
    *_x = _value;
    return 0;
}

int setCalibUInt(uint32_t *_x, int _value){
    *_x = _value;   
    return 0;
}

int CCalibMan::setCalibValue(rp_channel_t ch,ClalibValue _type, int _value){
auto g = getModeLV_HV();
#ifdef Z20_250_12
    auto ac_dc = getModeAC_DC();
    auto gen_g = getGenGain();
    switch (_type)
    {
        case ADC_CH_OFF:  {
            if (ch == RP_CH_1){
                if (g == RP_LOW)  return  (ac_dc == RP_DC) ? setCalibInt(&m_calib_parameters.osc_ch1_off_1_dc,_value)  : setCalibInt(&m_calib_parameters.osc_ch1_off_1_ac,_value);
                if (g == RP_HIGH) return  (ac_dc == RP_DC) ? setCalibInt(&m_calib_parameters.osc_ch1_off_20_dc,_value) : setCalibInt(&m_calib_parameters.osc_ch1_off_20_ac,_value);
            }
            if (ch == RP_CH_2){
                if (g == RP_LOW)  return  (ac_dc == RP_DC) ? setCalibInt(&m_calib_parameters.osc_ch2_off_1_dc,_value)  : setCalibInt(&m_calib_parameters.osc_ch2_off_1_ac ,_value);
                if (g == RP_HIGH) return  (ac_dc == RP_DC) ? setCalibInt(&m_calib_parameters.osc_ch2_off_20_dc,_value) : setCalibInt(&m_calib_parameters.osc_ch2_off_20_ac,_value);
            }
            break;
        }
        
        case ADC_CH_GAIN: {
            if (ch == RP_CH_1){
                if (g == RP_LOW)  return  (ac_dc == RP_DC) ? setCalibUInt(&m_calib_parameters.osc_ch1_g_1_dc,_value)  : setCalibUInt(&m_calib_parameters.osc_ch1_g_1_ac,_value);
                if (g == RP_HIGH) return  (ac_dc == RP_DC) ? setCalibUInt(&m_calib_parameters.osc_ch1_g_20_dc,_value) : setCalibUInt(&m_calib_parameters.osc_ch1_g_20_ac,_value);
            }
            if (ch == RP_CH_2){
                if (g == RP_LOW)  return  (ac_dc == RP_DC) ? setCalibUInt(&m_calib_parameters.osc_ch2_g_1_dc,_value)  : setCalibUInt(&m_calib_parameters.osc_ch2_g_1_ac,_value);
                if (g == RP_HIGH) return  (ac_dc == RP_DC) ? setCalibUInt(&m_calib_parameters.osc_ch2_g_20_dc,_value) : setCalibUInt(&m_calib_parameters.osc_ch2_g_20_ac,_value);
            }
            break;
        }
        
        case DAC_CH_OFF:   {
            if (ch == RP_CH_1)
                return  (gen_g == RP_GAIN_1X) ? setCalibInt(&m_calib_parameters.gen_ch1_off_1,_value)  : setCalibInt(&m_calib_parameters.gen_ch1_off_5,_value);
            if (ch == RP_CH_1)
                return  (gen_g == RP_GAIN_1X) ? setCalibInt(&m_calib_parameters.gen_ch2_off_1,_value)  : setCalibInt(&m_calib_parameters.gen_ch2_off_5,_value);
        }
        
        case DAC_CH_GAIN:   {
            if (ch == RP_CH_1)
                return  (gen_g == RP_GAIN_1X) ? setCalibUInt(&m_calib_parameters.gen_ch1_g_1,_value)  : setCalibUInt(&m_calib_parameters.gen_ch1_g_5,_value);
            if (ch == RP_CH_2)
                return  (gen_g == RP_GAIN_1X) ? setCalibUInt(&m_calib_parameters.gen_ch2_g_1,_value)  : setCalibUInt(&m_calib_parameters.gen_ch2_g_5,_value);
        }
    }
#endif

#if defined Z10 || defined Z20_125

    switch (_type)
    {
        case ADC_CH_OFF:  {
            if (ch == RP_CH_1) {
                if (g == RP_LOW)  return setCalibInt(&m_calib_parameters.fe_ch1_lo_offs,_value);
                if (g == RP_HIGH) return setCalibInt(&m_calib_parameters.fe_ch1_hi_offs,_value);
            }
            if (ch == RP_CH_2) {
                if (g == RP_LOW)  return setCalibInt(&m_calib_parameters.fe_ch2_lo_offs,_value);
                if (g == RP_HIGH) return setCalibInt(&m_calib_parameters.fe_ch2_hi_offs,_value);
            }
            break;
        }
        
        case ADC_CH_GAIN: {
            if (ch == RP_CH_1) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.fe_ch1_fs_g_lo,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.fe_ch1_fs_g_hi,_value);
            }
            if (ch == RP_CH_2) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.fe_ch2_fs_g_lo,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.fe_ch2_fs_g_hi,_value);
            }
            break;
        }

        case F_AA_CH: {
            m_calib_parameters.magic = 0xDDCCBBAA;
            if (ch == RP_CH_1){
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.low_filter_aa_ch1,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.hi_filter_aa_ch1,_value);
            }
            if (ch == RP_CH_2){
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.low_filter_aa_ch2,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.hi_filter_aa_ch2,_value);            
            }
            break;
        }
        
        case F_BB_CH: {
            m_calib_parameters.magic = 0xDDCCBBAA;
            if (ch == RP_CH_1){
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.low_filter_bb_ch1,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.hi_filter_bb_ch1,_value);
            }
            if (ch == RP_CH_2){
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.low_filter_bb_ch2,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.hi_filter_bb_ch2,_value);            
            }
            break;
        }


        case F_PP_CH: {
            m_calib_parameters.magic = 0xDDCCBBAA;
            if (ch == RP_CH_1){
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.low_filter_pp_ch1,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.hi_filter_pp_ch1,_value);
            }
            if (ch == RP_CH_2){
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.low_filter_pp_ch2,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.hi_filter_pp_ch2,_value);            
            }
            break;
        }

        case F_KK_CH: {
            m_calib_parameters.magic = 0xDDCCBBAA;
            if (ch == RP_CH_1){
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.low_filter_kk_ch1,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.hi_filter_kk_ch1,_value);
            }
            if (ch == RP_CH_2){
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.low_filter_kk_ch2,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.hi_filter_kk_ch2,_value);            
            }
            break;
        }

       
        case DAC_CH_OFF:  {
            if (ch == RP_CH_1) return setCalibInt(&m_calib_parameters.be_ch1_dc_offs,_value);
            if (ch == RP_CH_2) return setCalibInt(&m_calib_parameters.be_ch2_dc_offs,_value);
        }
        
        case DAC_CH_GAIN:  {
            if (ch == RP_CH_1) return setCalibUInt(&m_calib_parameters.be_ch1_fs,_value);
            if (ch == RP_CH_2) return setCalibUInt(&m_calib_parameters.be_ch2_fs,_value);
        }
    }
#endif

#if defined Z20_125_4CH
    switch (_type)
    {
        case ADC_CH_OFF:  {
            if (ch == RP_CH_1) {
                if (g == RP_LOW)  return setCalibInt(&m_calib_parameters.chA_low_offs,_value);
                if (g == RP_HIGH) return setCalibInt(&m_calib_parameters.chA_hi_offs,_value);
            }
            if (ch == RP_CH_2) {
                if (g == RP_LOW)  return setCalibInt(&m_calib_parameters.chB_low_offs,_value);
                if (g == RP_HIGH) return setCalibInt(&m_calib_parameters.chB_hi_offs,_value);
            }
            if (ch == RP_CH_3) {
                if (g == RP_LOW)  return setCalibInt(&m_calib_parameters.chC_low_offs,_value);
                if (g == RP_HIGH) return setCalibInt(&m_calib_parameters.chC_hi_offs,_value);
            }
            if (ch == RP_CH_4) {
                if (g == RP_LOW)  return setCalibInt(&m_calib_parameters.chD_low_offs,_value);
                if (g == RP_HIGH) return setCalibInt(&m_calib_parameters.chD_hi_offs,_value);
            }
            break;
        }
        
        case ADC_CH_GAIN: {
            if (ch == RP_CH_1) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chA_g_low,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chA_g_hi,_value);
            }
            if (ch == RP_CH_2) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chB_g_low,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chB_g_hi,_value);
            }
            if (ch == RP_CH_3) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chC_g_low,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chC_g_hi,_value);
            }
            if (ch == RP_CH_4) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chD_g_low,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chD_g_hi,_value);
            }
            break;
        }


        case F_AA_CH: {
            if (ch == RP_CH_1) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chA_low_aa,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chA_hi_aa,_value);
            }
            if (ch == RP_CH_2) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chB_low_aa,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chB_hi_aa,_value);
            }
            if (ch == RP_CH_3) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chC_low_aa,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chC_hi_aa,_value);
            }
            if (ch == RP_CH_4) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chD_low_aa,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chD_hi_aa,_value);
            }
            break;
        }

        case F_BB_CH: {
            if (ch == RP_CH_1) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chA_low_bb,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chA_hi_bb,_value);
            }
            if (ch == RP_CH_2) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chB_low_bb,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chB_hi_bb,_value);
            }
            if (ch == RP_CH_3) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chC_low_bb,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chC_hi_bb,_value);
            }
            if (ch == RP_CH_4) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chD_low_bb,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chD_hi_bb,_value);
            }
            break;
        }

        case F_PP_CH: {
            if (ch == RP_CH_1) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chA_low_pp,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chA_hi_pp,_value);
            }
            if (ch == RP_CH_2) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chB_low_pp,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chB_hi_pp,_value);
            }
            if (ch == RP_CH_3) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chC_low_pp,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chC_hi_pp,_value);
            }
            if (ch == RP_CH_4) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chD_low_pp,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chD_hi_pp,_value);
            }
            break;
        }

        case F_KK_CH: {
            if (ch == RP_CH_1) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chA_low_kk,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chA_hi_kk,_value);
            }
            if (ch == RP_CH_2) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chB_low_kk,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chB_hi_kk,_value);
            }
            if (ch == RP_CH_3) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chC_low_kk,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chC_hi_kk,_value);
            }
            if (ch == RP_CH_4) {
                if (g == RP_LOW)  return setCalibUInt(&m_calib_parameters.chD_low_kk,_value);
                if (g == RP_HIGH) return setCalibUInt(&m_calib_parameters.chD_hi_kk,_value);
            }
            break;
        }        
    }
#endif
    return -1;
}

#if defined Z20_250_12 || defined Z10 || defined Z20 || defined Z20_125

int CCalibMan::enableGen(rp_channel_t _ch,bool _enable){
    m_acq->enableGen(_ch,_enable);
    return 0;
}


int CCalibMan::setFreq(rp_channel_t _ch,int _freq){
    return m_acq->setFreq(_ch,_freq);    
}

int CCalibMan::setAmp(rp_channel_t _ch,float _ampl){
    return m_acq->setAmp(_ch,_ampl);    
}

int CCalibMan::setOffset(rp_channel_t _ch,float _offset){
    return m_acq->setOffset(_ch,_offset);    
}

int CCalibMan::setGenType(rp_channel_t _ch,int _type){
    return m_acq->setGenType(_ch,_type);
}

void CCalibMan::updateGen(){
    m_acq->updateGenCalib();
}

#endif

void CCalibMan::updateAcqFilter(rp_channel_t _ch){
    m_acq->updateAcqFilter(_ch);
}

int CCalibMan::setDefualtFilter(rp_channel_t _ch){
#if defined Z10 || defined Z20_125
    auto x = rp_GetDefaultCalibrationSettings();
    auto g = getModeLV_HV();
    if (_ch == RP_CH_1){
        if (g == RP_LOW)  {
            setCalibValue(_ch,F_AA_CH,x.low_filter_aa_ch1);
            setCalibValue(_ch,F_BB_CH,x.low_filter_bb_ch1);
            setCalibValue(_ch,F_PP_CH,x.low_filter_pp_ch1);
            setCalibValue(_ch,F_KK_CH,x.low_filter_kk_ch1);
        }
        if (g == RP_HIGH) {
            setCalibValue(_ch,F_AA_CH,x.hi_filter_aa_ch1);
            setCalibValue(_ch,F_BB_CH,x.hi_filter_bb_ch1);
            setCalibValue(_ch,F_PP_CH,x.hi_filter_pp_ch1);
            setCalibValue(_ch,F_KK_CH,x.hi_filter_kk_ch1);
        }
    }

    if (_ch == RP_CH_2){
        if (g == RP_LOW)  {
            setCalibValue(_ch,F_AA_CH,x.low_filter_aa_ch2);
            setCalibValue(_ch,F_BB_CH,x.low_filter_bb_ch2);
            setCalibValue(_ch,F_PP_CH,x.low_filter_pp_ch2);
            setCalibValue(_ch,F_KK_CH,x.low_filter_kk_ch2);
        }
        if (g == RP_HIGH) {
            setCalibValue(_ch,F_AA_CH,x.hi_filter_aa_ch2);
            setCalibValue(_ch,F_BB_CH,x.hi_filter_bb_ch2);
            setCalibValue(_ch,F_PP_CH,x.hi_filter_pp_ch2);
            setCalibValue(_ch,F_KK_CH,x.hi_filter_kk_ch2);
        }
    }
    return 0;
#else
#if defined Z20_125_4CH

    auto x = rp_GetDefaultCalibrationSettings();
    auto g = getModeLV_HV();
    if (_ch == RP_CH_1){
        if (g == RP_LOW)  {
            setCalibValue(_ch,F_AA_CH,x.chA_low_aa);
            setCalibValue(_ch,F_BB_CH,x.chA_low_bb);
            setCalibValue(_ch,F_PP_CH,x.chA_low_pp);
            setCalibValue(_ch,F_KK_CH,x.chA_low_kk);
        }
        if (g == RP_HIGH) {
            setCalibValue(_ch,F_AA_CH,x.chA_hi_aa);
            setCalibValue(_ch,F_BB_CH,x.chA_hi_bb);
            setCalibValue(_ch,F_PP_CH,x.chA_hi_pp);
            setCalibValue(_ch,F_KK_CH,x.chA_hi_kk);
        }
    }

    if (_ch == RP_CH_2){
        if (g == RP_LOW)  {
            setCalibValue(_ch,F_AA_CH,x.chB_low_aa);
            setCalibValue(_ch,F_BB_CH,x.chB_low_bb);
            setCalibValue(_ch,F_PP_CH,x.chB_low_pp);
            setCalibValue(_ch,F_KK_CH,x.chB_low_kk);
        }
        if (g == RP_HIGH) {
            setCalibValue(_ch,F_AA_CH,x.chB_hi_aa);
            setCalibValue(_ch,F_BB_CH,x.chB_hi_bb);
            setCalibValue(_ch,F_PP_CH,x.chB_hi_pp);
            setCalibValue(_ch,F_KK_CH,x.chB_hi_kk);
        }
    }

    if (_ch == RP_CH_3){
        if (g == RP_LOW)  {
            setCalibValue(_ch,F_AA_CH,x.chC_low_aa);
            setCalibValue(_ch,F_BB_CH,x.chC_low_bb);
            setCalibValue(_ch,F_PP_CH,x.chC_low_pp);
            setCalibValue(_ch,F_KK_CH,x.chC_low_kk);
        }
        if (g == RP_HIGH) {
            setCalibValue(_ch,F_AA_CH,x.chC_hi_aa);
            setCalibValue(_ch,F_BB_CH,x.chC_hi_bb);
            setCalibValue(_ch,F_PP_CH,x.chC_hi_pp);
            setCalibValue(_ch,F_KK_CH,x.chC_hi_kk);
        }
    }

    if (_ch == RP_CH_4){
        if (g == RP_LOW)  {
            setCalibValue(_ch,F_AA_CH,x.chD_low_aa);
            setCalibValue(_ch,F_BB_CH,x.chD_low_bb);
            setCalibValue(_ch,F_PP_CH,x.chD_low_pp);
            setCalibValue(_ch,F_KK_CH,x.chD_low_kk);
        }
        if (g == RP_HIGH) {
            setCalibValue(_ch,F_AA_CH,x.chD_hi_aa);
            setCalibValue(_ch,F_BB_CH,x.chD_hi_bb);
            setCalibValue(_ch,F_PP_CH,x.chD_hi_pp);
            setCalibValue(_ch,F_KK_CH,x.chD_hi_kk);
        }
    }
   
    return 0;
#else
    return -1;
#endif
#endif
}