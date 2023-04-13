#include <fstream>
#include <ctime>
#include "calib.h"

// float calibFullScaleToVoltage(uint32_t fullScaleGain) {
//     if (fullScaleGain == 0) return 1;
//     return (float) ((float)fullScaleGain  * 100.0 / ((uint64_t)1<<32));
// }

// uint32_t calibFullScaleFromVoltage(float voltageScale) {
//     return (uint32_t) (voltageScale / 100.0 * ((uint64_t)1<<32));
// }


CCalib::Ptr CCalib::Create(COscilloscope::Ptr _acq)
{
    return std::make_shared<CCalib>(_acq);
}

CCalib::CCalib(COscilloscope::Ptr _acq):
m_acq(_acq),
m_current_step(-1),
m_channels(0)
{
    m_channels = getADCChannels();
    for(int i = 0; i < m_channels; i++){
        m_pass_data.ch[i] = 0;
    }
    m_calib_parameters_old = rp_GetCalibrationSettings();
}

CCalib::~CCalib()
{

}

int CCalib::resetCalibToZero(){
    return rp_CalibrationReset(false);
}

int CCalib::resetCalibToFactory(){
    return rp_CalibrationFactoryReset();
}

void CCalib::restoreCalib(){
    rp_CalibrationWriteParams(m_calib_parameters_old,false);
    rp_CalibInit();
}

int CCalib::calib(uint16_t _step,float _refdc){
    switch (getModel())
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
            return calib_board_z10(_step,_refdc);

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:{
            fprintf(stderr,"[Error] Board can't be calibrate\n");
            exit(-1);
        }


        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            return calib_board_z20_4ch(_step,_refdc);

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_120:
            return calib_board_z20_250_12(_step,_refdc);
        default:
            fprintf(stderr,"[Error] Can't get board model\n");
            exit(-1);
    }
    return 0;
}

COscilloscope::DataPass CCalib::getData(int skip_read){
    auto old_d = m_acq->getData();
    while(skip_read > 0){
        auto new_d = m_acq->getData();
        skip_read -= (new_d.index - old_d.index);
        old_d = new_d;
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    return old_d;
}

CCalib::DataPass CCalib::getCalibData(){
    return m_pass_data;
}


int CCalib::calib_board_z10(uint16_t _step,float _refdc){
    if (m_current_step == _step) return 0;
    m_current_step = _step;
    switch(_step){
        case 0: {
            m_acq->startNormal();
            m_calib_parameters_old = rp_GetCalibrationSettings();
            resetCalibToZero();
            m_calib_parameters = rp_GetCalibrationSettings();
            return 0;
        }

        case 1: {
            m_acq->setHV();
            auto x = getData(10);
            m_calib_parameters.fast_adc_1_20[0].offset = x.ch_avg_raw[0];
            m_calib_parameters.fast_adc_1_20[1].offset = x.ch_avg_raw[1];
            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();
            m_calib_parameters = rp_GetCalibrationSettings();
            m_pass_data.ch[0] = m_calib_parameters.fast_adc_1_20[0].offset;
            m_pass_data.ch[1] = m_calib_parameters.fast_adc_1_20[1].offset;
            return 0;
        }

        case 2: {
            m_acq->setHV();
            auto x = getData(10);
            m_calib_parameters.fast_adc_1_20[0].gainCalc = _refdc / x.ch_avg[0];
            m_calib_parameters.fast_adc_1_20[1].gainCalc = _refdc / x.ch_avg[1];
            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();
            m_calib_parameters = rp_GetCalibrationSettings();
            m_pass_data.ch[0] = m_calib_parameters.fast_adc_1_20[0].calibValue;
            m_pass_data.ch[1] = m_calib_parameters.fast_adc_1_20[1].calibValue;
            return 0;
        }

        case 3: {
            m_acq->setLV();
            auto x = getData(10);
            m_calib_parameters.fast_adc_1_1[0].offset = x.ch_avg_raw[0];
            m_calib_parameters.fast_adc_1_1[1].offset = x.ch_avg_raw[1];
            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();
            m_calib_parameters = rp_GetCalibrationSettings();
            m_pass_data.ch[0] = m_calib_parameters.fast_adc_1_1[0].offset;
            m_pass_data.ch[1] = m_calib_parameters.fast_adc_1_1[1].offset;
            return 0;
        }

        case 4: {
            m_acq->setLV();
            auto x = getData(10);
            m_calib_parameters.fast_adc_1_1[0].gainCalc = _refdc / x.ch_avg[0];
            m_calib_parameters.fast_adc_1_1[1].gainCalc = _refdc / x.ch_avg[1];
            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();
            m_calib_parameters = rp_GetCalibrationSettings();
            m_pass_data.ch[0] = m_calib_parameters.fast_adc_1_1[0].calibValue;
            m_pass_data.ch[1] = m_calib_parameters.fast_adc_1_1[1].calibValue;
            return 0;
        }

        case 5: {
            m_acq->setGEN_DISABLE();
            m_acq->setLV();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            return 0;
        }

        case 6: {
            m_acq->setGEN_DISABLE();
            m_acq->setLV();
            auto x = getData(30);

            auto ch1_bits = rp_HPGetFastADCBitsOrDefault(RP_CH_1);
            auto ch2_bits = rp_HPGetFastADCBitsOrDefault(RP_CH_2);

            m_calib_parameters.fast_dac_x1[0].offset = x.ch_avg[0] * -(1 << ch1_bits-1);
            m_calib_parameters.fast_dac_x1[1].offset = x.ch_avg[1] * -(1 << ch2_bits-1);
            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();
            m_calib_parameters = rp_GetCalibrationSettings();
            m_pass_data.ch[0] = m_calib_parameters.fast_dac_x1[0].offset;
            m_pass_data.ch[1] = m_calib_parameters.fast_dac_x1[1].offset;
            return 0;
        }

        case 7: {
            m_acq->setGEN0_5();
            m_acq->setLV();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            return 0;
        }

        case 8: {
            m_acq->setGEN0_5();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            m_acq->setLV();
            auto x = getData(30);
            // uint32_t ch1_calib = calibFullScaleFromVoltage(1.f * x.ch_avg[0] / 0.5);
            // uint32_t ch2_calib = calibFullScaleFromVoltage(1.f * x.ch_avg[1] / 0.5);
            m_calib_parameters.fast_dac_x1[0].gainCalc = x.ch_avg[0] / 0.5;
            m_calib_parameters.fast_dac_x1[1].gainCalc = x.ch_avg[1] / 0.5;
            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();
            m_calib_parameters = rp_GetCalibrationSettings();
            m_pass_data.ch[0] = m_calib_parameters.fast_dac_x1[0].calibValue;
            m_pass_data.ch[1] = m_calib_parameters.fast_dac_x1[1].calibValue;
            m_acq->setGEN0_5();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            return 0;
        }
    }
    return 0;
}


int CCalib::calib_board_z20_4ch(uint16_t _step,float _refdc){
    if (m_current_step == _step) return 0;
    m_current_step = _step;
    switch(_step){
        case 0: {
            m_acq->startNormal();
            m_calib_parameters_old = rp_GetCalibrationSettings();
            resetCalibToZero();
            m_calib_parameters = rp_GetCalibrationSettings();
            return 0;
        }

        case 1: {
            m_acq->setHV();
            auto x = getData(10);
            for(int ch = 0; ch < m_channels; ch++){
                m_calib_parameters.fast_adc_1_20[ch].offset = x.ch_avg_raw[ch];
            }
            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();

            m_calib_parameters = rp_GetCalibrationSettings();
            for(int ch = 0; ch < m_channels; ch++){
                m_pass_data.ch[ch] = m_calib_parameters.fast_adc_1_20[ch].offset;
            }
            return 0;
        }

        case 2: {
            m_acq->setHV();
            auto x = getData(10);
            for(int ch = 0; ch < m_channels; ch++){
                m_calib_parameters.fast_adc_1_20[ch].gainCalc = _refdc / x.ch_avg[ch];
            }

            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();

            m_calib_parameters = rp_GetCalibrationSettings();
            for(int ch = 0; ch < m_channels; ch++){
                m_pass_data.ch[ch] = m_calib_parameters.fast_adc_1_20[ch].calibValue;
            }
            return 0;
        }

        case 3: {
            m_acq->setLV();
            auto x = getData(10);
             for(int ch = 0; ch < m_channels; ch++){
                m_calib_parameters.fast_adc_1_1[ch].offset = x.ch_avg_raw[ch];
            }
            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();

            m_calib_parameters = rp_GetCalibrationSettings();
            for(int ch = 0; ch < m_channels; ch++){
                m_pass_data.ch[ch] = m_calib_parameters.fast_adc_1_1[ch].offset;
            }
            return 0;
        }

        case 4: {
            m_acq->setLV();
            auto x = getData(10);
            for(int ch = 0; ch < m_channels; ch++){
                m_calib_parameters.fast_adc_1_1[ch].gainCalc = _refdc / x.ch_avg[ch];
            }

            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();

            m_calib_parameters = rp_GetCalibrationSettings();
            for(int ch = 0; ch < m_channels; ch++){
                m_pass_data.ch[ch] = m_calib_parameters.fast_adc_1_1[ch].calibValue;
            }
            return 0;
        }
    }
    return 0;
}

int CCalib::calib_board_z20_250_12(uint16_t _step,float _refdc){
    if (m_current_step == _step) return 0;
    m_current_step = _step;
    switch(_step){
        case 0: {
            m_calib_parameters_old = rp_GetCalibrationSettings();
            resetCalibToZero();
            m_calib_parameters = rp_GetCalibrationSettings();
            return 0;
        }

        case 1: {
            m_acq->setLV();
            m_acq->setDC();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            return 0;
        }

        case 2: {
            m_acq->setLV();
            m_acq->setDC();
            auto x = getData(10);
            for(int ch = 0; ch < m_channels; ch++){
                m_calib_parameters.fast_adc_1_1[ch].offset = x.ch_avg_raw[ch];
            }
            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();

            m_calib_parameters = rp_GetCalibrationSettings();
            for(int ch = 0; ch < m_channels; ch++){
                m_pass_data.ch[ch] = m_calib_parameters.fast_adc_1_1[ch].offset;
            }
            return 0;
        }

        case 3: {
            m_acq->setDC();
            m_acq->setLV();
            auto x = getData(10);
            for(int ch = 0; ch < m_channels; ch++){
                m_calib_parameters.fast_adc_1_1[ch].gainCalc = _refdc / x.ch_avg[ch];
            }

            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();

            m_calib_parameters = rp_GetCalibrationSettings();
            for(int ch = 0; ch < m_channels; ch++){
                m_pass_data.ch[ch] = m_calib_parameters.fast_adc_1_1[ch].calibValue;
            }
            return 0;
        }

        case 4: {
            m_acq->setHV();
            m_acq->setDC();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            return 0;
        }


        case 5: {
            m_acq->setHV();
            m_acq->setDC();
            auto x = getData(10);
            for(int ch = 0; ch < m_channels; ch++){
                m_calib_parameters.fast_adc_1_20[ch].offset = x.ch_avg_raw[ch];
            }
            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();

            m_calib_parameters = rp_GetCalibrationSettings();
            for(int ch = 0; ch < m_channels; ch++){
                m_pass_data.ch[ch] = m_calib_parameters.fast_adc_1_20[ch].offset;
            }
            return 0;
        }

         case 6: {
            m_acq->setDC();
            m_acq->setHV();
            auto x = getData(10);
            for(int ch = 0; ch < m_channels; ch++){
                m_calib_parameters.fast_adc_1_20[ch].gainCalc = _refdc / x.ch_avg[ch];
            }

            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();

            m_calib_parameters = rp_GetCalibrationSettings();
            for(int ch = 0; ch < m_channels; ch++){
                m_pass_data.ch[ch] = m_calib_parameters.fast_adc_1_20[ch].calibValue;
            }
            return 0;
        }

        case 7: {
            m_acq->setGEN_DISABLE();
            m_acq->setLV();
            m_acq->setDC();
            m_acq->setGenGainx1();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            return 0;
        }

        case 8: {
            m_acq->setGEN_DISABLE();
            m_acq->setLV();
            m_acq->setDC();
            m_acq->setGenGainx1();
            auto x = getData(30);
            auto ch1_bits = rp_HPGetFastADCBitsOrDefault(RP_CH_1);
            auto ch2_bits = rp_HPGetFastADCBitsOrDefault(RP_CH_2);

            m_calib_parameters.fast_dac_x1[0].offset = x.ch_avg[0] * -(1 << ch1_bits-1);
            m_calib_parameters.fast_dac_x1[1].offset = x.ch_avg[1] * -(1 << ch2_bits-1);
            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();
            m_calib_parameters = rp_GetCalibrationSettings();
            m_pass_data.ch[0] = m_calib_parameters.fast_dac_x1[0].offset;
            m_pass_data.ch[1] = m_calib_parameters.fast_dac_x1[1].offset;
            return 0;
        }

        case 9: {
            m_acq->setGEN0_5();
            m_acq->setLV();
            m_acq->setDC();
            m_acq->setGenGainx1();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            return 0;
        }

        case 10: {
            m_acq->setGEN0_5();
            m_acq->setDC();
            m_acq->setGenGainx1();
            m_acq->setLV();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            auto x = getData(30);
            m_calib_parameters.fast_dac_x1[0].gainCalc = x.ch_avg[0] / 0.5;
            m_calib_parameters.fast_dac_x1[1].gainCalc = x.ch_avg[1] / 0.5;
            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();
            m_calib_parameters = rp_GetCalibrationSettings();
            m_pass_data.ch[0] = m_calib_parameters.fast_dac_x1[0].calibValue;
            m_pass_data.ch[1] = m_calib_parameters.fast_dac_x1[1].calibValue;
            m_acq->setGEN0_5();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            return 0;
        }

        case 11: {
            m_acq->setGEN_DISABLE();
            m_acq->setLV();
            m_acq->setDC();
            m_acq->setGenGainx5();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            return 0;
        }

        case 12: {
            m_acq->setGEN_DISABLE();
            m_acq->setLV();
            m_acq->setDC();
            m_acq->setGenGainx5();
            auto x = getData(30);
            auto ch1_bits = rp_HPGetFastADCBitsOrDefault(RP_CH_1);
            auto ch2_bits = rp_HPGetFastADCBitsOrDefault(RP_CH_2);

            m_calib_parameters.fast_dac_x5[0].offset = x.ch_avg[0] * -(1 << ch1_bits-1);
            m_calib_parameters.fast_dac_x5[1].offset = x.ch_avg[1] * -(1 << ch2_bits-1);
            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();
            m_calib_parameters = rp_GetCalibrationSettings();
            m_pass_data.ch[0] = m_calib_parameters.fast_dac_x5[0].offset;
            m_pass_data.ch[1] = m_calib_parameters.fast_dac_x5[1].offset;
            return 0;
        }

        case 13: {
            m_acq->setGEN0_5();
            m_acq->setHV();
            m_acq->setDC();
            m_acq->setGenGainx5();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            return 0;
        }

        case 14: {
            m_acq->setGEN0_5();
            m_acq->setDC();
            m_acq->setGenGainx5();
            m_acq->setHV();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            auto x = getData(30);
            m_calib_parameters.fast_dac_x5[0].gainCalc = x.ch_avg[0] / 2.5;
            m_calib_parameters.fast_dac_x5[1].gainCalc = x.ch_avg[1] / 2.5;
            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();
            m_calib_parameters = rp_GetCalibrationSettings();
            m_pass_data.ch[0] = m_calib_parameters.fast_dac_x5[0].calibValue;
            m_pass_data.ch[1] = m_calib_parameters.fast_dac_x5[1].calibValue;
            m_acq->setGEN0_5();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            return 0;
        }

        case 15: {
            m_acq->setGEN_DISABLE();
            m_acq->setLV();
            m_acq->setAC();
            m_acq->setGenGainx1();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            return 0;
        }

        case 16: {
            m_acq->setGEN_DISABLE();
            m_acq->setLV();
            m_acq->setAC();
            m_acq->setGenGainx1();
            auto x = getData(10);
            for(int ch = 0; ch < m_channels; ch++){
                m_calib_parameters.fast_adc_1_1_ac[ch].offset = x.ch_avg_raw[ch];
            }
            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();

            m_calib_parameters = rp_GetCalibrationSettings();
            for(int ch = 0; ch < m_channels; ch++){
                m_pass_data.ch[ch] = m_calib_parameters.fast_adc_1_1_ac[ch].offset;
            }
            return 0;
        }

        case 17: {
            m_acq->setGEN0_5_SINE();
            m_acq->setLV();
            m_acq->setAC();
            m_acq->setGenGainx1();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            return 0;
        }

        case 18: {
            m_acq->setGEN0_5_SINE();
            m_acq->setLV();
            m_acq->setAC();
            m_acq->setGenGainx1();
            auto x = getData(30);
            for(int ch = 0; ch < m_channels; ch++){
                m_calib_parameters.fast_adc_1_1_ac[ch].gainCalc = _refdc / x.ch_avg[ch];
            }

            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();

            m_calib_parameters = rp_GetCalibrationSettings();
            for(int ch = 0; ch < m_channels; ch++){
                m_pass_data.ch[ch] = m_calib_parameters.fast_adc_1_1_ac[ch].calibValue;
            }
            return 0;
        }

        case 19: {
            m_acq->setGEN_DISABLE();
            m_acq->setHV();
            m_acq->setAC();
            m_acq->setGenGainx1();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            return 0;
        }

        case 20: {
            m_acq->setGEN_DISABLE();
            m_acq->setHV();
            m_acq->setAC();
            m_acq->setGenGainx1();
            auto x = getData(10);
            for(int ch = 0; ch < m_channels; ch++){
                m_calib_parameters.fast_adc_1_20_ac[ch].offset = x.ch_avg_raw[ch];
            }
            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();

            m_calib_parameters = rp_GetCalibrationSettings();
            for(int ch = 0; ch < m_channels; ch++){
                m_pass_data.ch[ch] = m_calib_parameters.fast_adc_1_20_ac[ch].offset;
            }
            return 0;
        }

        case 21: {
            m_acq->setGEN0_5_SINE();
            m_acq->setHV();
            m_acq->setAC();
            m_acq->setGenGainx5();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            return 0;
        }

        case 22: {
            m_acq->setGEN0_5_SINE();
            m_acq->setHV();
            m_acq->setAC();
            m_acq->setGenGainx5();
            auto x = getData(30);
            for(int ch = 0; ch < m_channels; ch++){
                m_calib_parameters.fast_adc_1_20_ac[ch].gainCalc = _refdc / x.ch_avg[ch];
            }

            rp_CalibrationWriteParams(m_calib_parameters,false);
            rp_CalibInit();

            m_calib_parameters = rp_GetCalibrationSettings();
            for(int ch = 0; ch < m_channels; ch++){
                m_pass_data.ch[ch] = m_calib_parameters.fast_adc_1_20_ac[ch].calibValue;
            }
            return 0;
        }

    }
    return 0;
}
