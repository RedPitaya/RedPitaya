#include "calib_man.h"
#include "common.h"

#include <ctime>
#include <fstream>

namespace rp_calib {

CCalibMan::Ptr CCalibMan::Create(COscilloscope::Ptr _acq) {
    return std::make_shared<CCalibMan>(_acq);
}

CCalibMan::CCalibMan(COscilloscope::Ptr _acq) : m_calibMode(0), m_acq(_acq) {
    m_currentGain = RP_LOW;
    m_currentAC_DC = RP_DC;
    m_currentGenGain = RP_GAIN_1X;
    readCalib();
}

CCalibMan::~CCalibMan() {}

void CCalibMan::init() {
    m_acq->startNormal();
    if (getDACChannels() >= 2) {
        m_acq->resetGen();
    }

    m_currentGain = RP_LOW;
    m_acq->setLV();
    if (rp_HPGetFastADCIsAC_DCOrDefault()) {
        m_currentAC_DC = RP_DC;
        m_acq->setDC();
    }
    if (rp_HPGetIsGainDACx5OrDefault()) {
        m_currentGenGain = RP_GAIN_1X;
        m_acq->setGenGainx1();
    }
    readCalib();
    m_calibMode = 0;
}

void CCalibMan::initSq(int _decimation) {
    m_acq->startSquare(_decimation);
    m_acq->setHyst(0.05);
    this->setModeLV_HV(RP_LOW);
    this->changeChannel(RP_CH_1);
    if (getDACChannels() >= 2) {
        setGenType(RP_CH_1, (int)RP_WAVEFORM_SQUARE);
        setGenType(RP_CH_2, (int)RP_WAVEFORM_SQUARE);
        enableGen(RP_CH_1, false);
        enableGen(RP_CH_2, false);
    }

    readCalib();
    m_calibMode = 1;
}

int CCalibMan::getCalibMode() {
    return m_calibMode;
}

void CCalibMan::changeDecimation(int _decimation) {
    m_acq->startSquare(_decimation);
}

void CCalibMan::changeChannel(rp_channel_t _ch) {
    m_acq->setAcquireChannel(_ch);
}

int CCalibMan::readCalib() {
    m_calib_parameters = rp_GetCalibrationSettings();
    return 0;
}

int CCalibMan::readCalibEpprom() {
    rp_CalibInit();
    m_calib_parameters = rp_GetCalibrationSettings();
    return 0;
}

void CCalibMan::updateCalib() {
    rp_CalibrationSetParams(m_calib_parameters);
}

void CCalibMan::writeCalib() {
    readCalib();
    rp_CalibrationWriteParams(m_calib_parameters, false);
}

void CCalibMan::setModeLV_HV(rp_pinState_t _mode) {
    m_currentGain = _mode;
    if (m_currentGain == RP_LOW) {
        m_acq->setLV();
    }
    if (m_currentGain == RP_HIGH) {
        m_acq->setHV();
    }
}

rp_pinState_t CCalibMan::getModeLV_HV() {
    return m_currentGain;
}

void CCalibMan::setModeAC_DC(rp_acq_ac_dc_mode_t _mode) {
    if (!rp_HPGetFastADCIsAC_DCOrDefault()) {
        FATAL("AC/DC mode not present on board");
        exit(-1);
    }

    m_currentAC_DC = _mode;
    if (m_currentAC_DC == RP_DC) {
        m_acq->setDC();
    }
    if (m_currentAC_DC == RP_AC) {
        m_acq->setAC();
    }
}

rp_acq_ac_dc_mode_t CCalibMan::getModeAC_DC() {
    return m_currentAC_DC;
}

void CCalibMan::setGenGain(rp_gen_gain_t _mode) {
    if (!rp_HPGetIsGainDACx5OrDefault()) {
        FATAL("Gen gain mode not present on board");
        exit(-1);
    }

    m_currentGenGain = _mode;
    if (m_currentGenGain == RP_GAIN_1X) {
        m_acq->setGenGainx1();
    }
    if (m_currentGenGain == RP_GAIN_5X) {
        m_acq->setGenGainx5();
    }
}

rp_gen_gain_t CCalibMan::getGenGain() {
    return m_currentGenGain;
}

double CCalibMan::getCalibValue(rp_channel_t ch, ClalibValue _type) {
    auto g = getModeLV_HV();
    auto ac_dc = getModeAC_DC();
    auto gen_g = getGenGain();

    switch (_type) {
        case ADC_CH_OFF: {
            if (getADCChannels() <= ch) {
                FATAL("Wrong channel");
                exit(-1);
            }
            if (g == RP_LOW)
                return (ac_dc == RP_DC) ? m_calib_parameters.fast_adc_1_1[ch].offset : m_calib_parameters.fast_adc_1_1_ac[ch].offset;
            if (g == RP_HIGH)
                return (ac_dc == RP_DC) ? m_calib_parameters.fast_adc_1_20[ch].offset : m_calib_parameters.fast_adc_1_20_ac[ch].offset;
            break;
        }

        case DAC_CH_OFF: {
            if (getDACChannels() <= ch) {
                FATAL("Wrong channel");
                exit(-1);
            }
            return (gen_g == RP_GAIN_1X) ? m_calib_parameters.fast_dac_x1_HiZ[ch].offset : m_calib_parameters.fast_dac_x5_HiZ[ch].offset;
        }

        case ADC_CH_GAIN: {
            if (getADCChannels() <= ch) {
                FATAL("Wrong channel");
                exit(-1);
            }
            if (g == RP_LOW)
                return (ac_dc == RP_DC) ? m_calib_parameters.fast_adc_1_1[ch].gainCalc : m_calib_parameters.fast_adc_1_1_ac[ch].gainCalc;
            if (g == RP_HIGH)
                return (ac_dc == RP_DC) ? m_calib_parameters.fast_adc_1_20[ch].gainCalc : m_calib_parameters.fast_adc_1_20_ac[ch].gainCalc;
            break;
        }

        case DAC_CH_GAIN: {
            if (getDACChannels() <= ch) {
                FATAL("Wrong channel");
                exit(-1);
            }
            return (gen_g == RP_GAIN_1X) ? m_calib_parameters.fast_dac_x1_HiZ[ch].gainCalc : m_calib_parameters.fast_dac_x5_HiZ[ch].gainCalc;
        }

        case F_AA_CH: {
            if (getADCChannels() <= ch) {
                FATAL("Wrong channel");
                exit(-1);
            }
            if (!rp_HPGetFastADCIsFilterPresentOrDefault()) {
                FATAL("Filter not supported");
                exit(-1);
            }
            if (g == RP_LOW)
                return m_calib_parameters.fast_adc_filter_1_1[ch].aa;
            if (g == RP_HIGH)
                return m_calib_parameters.fast_adc_filter_1_20[ch].aa;
            break;
        }

        case F_BB_CH: {
            if (getADCChannels() <= ch) {
                FATAL("Wrong channel");
                exit(-1);
            }
            if (!rp_HPGetFastADCIsFilterPresentOrDefault()) {
                FATAL("Filter not supported");
                exit(-1);
            }
            if (g == RP_LOW)
                return m_calib_parameters.fast_adc_filter_1_1[ch].bb;
            if (g == RP_HIGH)
                return m_calib_parameters.fast_adc_filter_1_20[ch].bb;
            break;
        }

        case F_PP_CH: {
            if (getADCChannels() <= ch) {
                FATAL("Wrong channel");
                exit(-1);
            }
            if (!rp_HPGetFastADCIsFilterPresentOrDefault()) {
                FATAL("Filter not supported");
                exit(-1);
            }
            if (g == RP_LOW)
                return m_calib_parameters.fast_adc_filter_1_1[ch].pp;
            if (g == RP_HIGH)
                return m_calib_parameters.fast_adc_filter_1_20[ch].pp;
            break;
        }

        case F_KK_CH: {
            if (getADCChannels() <= ch) {
                FATAL("Wrong channel");
                exit(-1);
            }
            if (!rp_HPGetFastADCIsFilterPresentOrDefault()) {
                FATAL("Filter not supported");
                exit(-1);
            }
            if (g == RP_LOW)
                return m_calib_parameters.fast_adc_filter_1_1[ch].kk;
            if (g == RP_HIGH)
                return m_calib_parameters.fast_adc_filter_1_20[ch].kk;
            break;
        }

        default:
            ERROR_LOG("Unknow mode");
    }
    return 0;
}

int setCalibInt(int32_t* _x, int _value) {
    *_x = _value;
    return 0;
}

int setCalibUInt(uint32_t* _x, int _value) {
    *_x = _value;
    return 0;
}

int CCalibMan::setCalibValue(rp_channel_t ch, ClalibValue _type, double _value) {
    auto g = getModeLV_HV();
    auto ac_dc = getModeAC_DC();
    auto gen_g = getGenGain();
    switch (_type) {
        case ADC_CH_OFF: {
            if (getADCChannels() <= ch) {
                FATAL("Wrong channel");
                exit(-1);
            }
            if (g == RP_LOW)
                (ac_dc == RP_DC) ? m_calib_parameters.fast_adc_1_1[ch].offset = _value : m_calib_parameters.fast_adc_1_1_ac[ch].offset = _value;
            if (g == RP_HIGH)
                (ac_dc == RP_DC) ? m_calib_parameters.fast_adc_1_20[ch].offset = _value : m_calib_parameters.fast_adc_1_20_ac[ch].offset = _value;
            break;
        }

        case ADC_CH_GAIN: {
            if (getADCChannels() <= ch) {
                FATAL("Wrong channel");
                exit(-1);
            }
            if (g == RP_LOW)
                (ac_dc == RP_DC) ? m_calib_parameters.fast_adc_1_1[ch].gainCalc = _value : m_calib_parameters.fast_adc_1_1_ac[ch].gainCalc = _value;
            if (g == RP_HIGH)
                (ac_dc == RP_DC) ? m_calib_parameters.fast_adc_1_20[ch].gainCalc = _value : m_calib_parameters.fast_adc_1_20_ac[ch].gainCalc = _value;
            break;
        }

        case DAC_CH_OFF: {
            if (getDACChannels() <= ch) {
                FATAL("Wrong channel");
                exit(-1);
            }
            return (gen_g == RP_GAIN_1X) ? m_calib_parameters.fast_dac_x1_HiZ[ch].offset = _value : m_calib_parameters.fast_dac_x5_HiZ[ch].offset = _value;
        }

        case DAC_CH_GAIN: {
            if (getDACChannels() <= ch) {
                FATAL("Wrong channel");
                exit(-1);
            }
            return (gen_g == RP_GAIN_1X) ? m_calib_parameters.fast_dac_x1_HiZ[ch].gainCalc = _value : m_calib_parameters.fast_dac_x5_HiZ[ch].gainCalc = _value;
        }

        case F_AA_CH: {
            if (getADCChannels() <= ch) {
                FATAL("Wrong channel");
                exit(-1);
            }
            if (!rp_HPGetFastADCIsFilterPresentOrDefault()) {
                FATAL("Filter not supported");
                exit(-1);
            }
            if (g == RP_LOW)
                m_calib_parameters.fast_adc_filter_1_1[ch].aa = _value;
            if (g == RP_HIGH)
                m_calib_parameters.fast_adc_filter_1_20[ch].aa = _value;
            break;
        }

        case F_BB_CH: {
            if (getADCChannels() <= ch) {
                FATAL("Wrong channel");
                exit(-1);
            }
            if (!rp_HPGetFastADCIsFilterPresentOrDefault()) {
                FATAL("Filter not supported");
                exit(-1);
            }
            if (g == RP_LOW)
                m_calib_parameters.fast_adc_filter_1_1[ch].bb = _value;
            if (g == RP_HIGH)
                m_calib_parameters.fast_adc_filter_1_20[ch].bb = _value;
            break;
        }

        case F_PP_CH: {
            if (getADCChannels() <= ch) {
                FATAL("Wrong channel");
                exit(-1);
            }
            if (!rp_HPGetFastADCIsFilterPresentOrDefault()) {
                FATAL("Filter not supported");
                exit(-1);
            }
            if (g == RP_LOW)
                m_calib_parameters.fast_adc_filter_1_1[ch].pp = _value;
            if (g == RP_HIGH)
                m_calib_parameters.fast_adc_filter_1_20[ch].pp = _value;
            break;
        }

        case F_KK_CH: {
            if (getADCChannels() <= ch) {
                FATAL("Wrong channel");
                exit(-1);
            }
            if (!rp_HPGetFastADCIsFilterPresentOrDefault()) {
                FATAL("Filter not supported");
                exit(-1);
            }
            if (g == RP_LOW)
                m_calib_parameters.fast_adc_filter_1_1[ch].kk = _value;
            if (g == RP_HIGH)
                m_calib_parameters.fast_adc_filter_1_20[ch].kk = _value;
            break;
        }
        default:
            return -1;
    }
    return 0;
}

int CCalibMan::enableGen(rp_channel_t _ch, bool _enable) {
    if (getDACChannels() == 0) {
        ERROR_LOG("Wrong channel");
        return -1;
    }
    m_acq->enableGen(_ch, _enable);
    return 0;
}

int CCalibMan::setFreq(rp_channel_t _ch, int _freq) {
    if (getDACChannels() == 0) {
        ERROR_LOG("Wrong channel");
        return -1;
    }
    return m_acq->setFreq(_ch, _freq);
}

int CCalibMan::setAmp(rp_channel_t _ch, float _ampl) {
    if (getDACChannels() == 0) {
        ERROR_LOG("Wrong channel");
        return -1;
    }
    return m_acq->setAmp(_ch, _ampl);
}

int CCalibMan::setOffset(rp_channel_t _ch, float _offset) {
    if (getDACChannels() == 0) {
        ERROR_LOG("Wrong channel");
        return -1;
    }
    return m_acq->setOffset(_ch, _offset);
}

int CCalibMan::setGenType(rp_channel_t _ch, int _type) {
    if (getDACChannels() == 0) {
        ERROR_LOG("Wrong channel");
        return -1;
    }
    return m_acq->setGenType(_ch, _type);
}

void CCalibMan::updateGen() {
    m_acq->updateGenCalib();
}

void CCalibMan::updateAcqFilter(rp_channel_t _ch) {
    m_acq->updateAcqFilter(_ch);
}

int CCalibMan::setDefualtFilter(rp_channel_t _ch) {

    if (!rp_HPGetFastADCIsFilterPresentOrDefault()) {
        ERROR_LOG("Filter not present in board");
        return -1;
    }

    if (getADCChannels() <= _ch) {
        ERROR_LOG("Wrong channel");
        return -1;
    }

    auto g = getModeLV_HV();
    auto x = rp_GetDefaultCalibrationSettings();
    if (g == RP_LOW) {
        setCalibValue(_ch, F_AA_CH, x.fast_adc_filter_1_1[_ch].aa);
        setCalibValue(_ch, F_BB_CH, x.fast_adc_filter_1_1[_ch].bb);
        setCalibValue(_ch, F_PP_CH, x.fast_adc_filter_1_1[_ch].pp);
        setCalibValue(_ch, F_KK_CH, x.fast_adc_filter_1_1[_ch].kk);
    }
    if (g == RP_HIGH) {
        setCalibValue(_ch, F_AA_CH, x.fast_adc_filter_1_20[_ch].aa);
        setCalibValue(_ch, F_BB_CH, x.fast_adc_filter_1_20[_ch].bb);
        setCalibValue(_ch, F_PP_CH, x.fast_adc_filter_1_20[_ch].pp);
        setCalibValue(_ch, F_KK_CH, x.fast_adc_filter_1_20[_ch].kk);
    }

    return 0;
}

int CCalibMan::setDisableFilter(rp_channel_t _ch) {

    if (!rp_HPGetFastADCIsFilterPresentOrDefault()) {
        ERROR_LOG("Filter not present in board");
        return -1;
    }

    if (getADCChannels() <= _ch) {
        ERROR_LOG("Wrong channel");
        return -1;
    }

    auto g = getModeLV_HV();
    if (g == RP_LOW) {
        setCalibValue(_ch, F_AA_CH, 0);
        setCalibValue(_ch, F_BB_CH, 0);
        setCalibValue(_ch, F_PP_CH, 0);
        setCalibValue(_ch, F_KK_CH, 0x00FFFFFF);
    }
    if (g == RP_HIGH) {
        setCalibValue(_ch, F_AA_CH, 0);
        setCalibValue(_ch, F_BB_CH, 0);
        setCalibValue(_ch, F_PP_CH, 0);
        setCalibValue(_ch, F_KK_CH, 0x00FFFFFF);
    }

    return 0;
}

}  // namespace rp_calib