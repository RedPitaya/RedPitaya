#include <fstream>
#include <ctime>
#include "calib.h"


void PrintLogInFile2(const char *message){
	std::time_t result = std::time(nullptr);
	std::fstream fs;
  	fs.open ("/tmp/debug.log", std::fstream::in | std::fstream::out | std::fstream::app);
	fs << std::asctime(std::localtime(&result)) << " : " << message << "\n";
	fs.close();
}

float calibFullScaleToVoltage(uint32_t fullScaleGain) {
    if (fullScaleGain == 0) return 1;
    return (float) ((float)fullScaleGain  * 100.0 / ((uint64_t)1<<32));
}

uint32_t calibFullScaleFromVoltage(float voltageScale) {
    return (uint32_t) (voltageScale / 100.0 * ((uint64_t)1<<32));
}


CCalib::Ptr CCalib::Create(COscilloscope::Ptr _acq)
{
    return std::make_shared<CCalib>(_acq);
}

CCalib::CCalib(COscilloscope::Ptr _acq):
m_acq(_acq),
m_current_step(-1)
{
   m_pass_data.ch1 = 0;
   m_pass_data.ch2 = 0;
   m_calib_parameters_old = rp_GetCalibrationSettings();
}

CCalib::~CCalib()
{

}

int CCalib::resetCalibToZero(){
    return rp_CalibrationReset();
}

int CCalib::resetCalibToFactory(){
    return rp_CalibrationFactoryReset();
}

void CCalib::restoreCalib(){
    rp_CalibrationWriteParams(m_calib_parameters_old);
    rp_CalibInit();
}

int CCalib::calib(uint16_t _step,float _refdc){
    return calib_board(_step,_refdc);
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

#if defined Z10 || defined Z20_125
int CCalib::calib_board(uint16_t _step,float _refdc){
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
            m_calib_parameters.fe_ch1_hi_offs = x.ch1_avg_raw;
            m_calib_parameters.fe_ch2_hi_offs = x.ch2_avg_raw;
            m_pass_data.ch1 = m_calib_parameters.fe_ch1_hi_offs;
            m_pass_data.ch2 = m_calib_parameters.fe_ch2_hi_offs;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
            return 0;
        }

        case 2: {
            m_acq->setHV();
            auto x = getData(10);
            uint32_t ch1_calib = calibFullScaleFromVoltage(1.f * _refdc / x.ch1_avg);
            uint32_t ch2_calib = calibFullScaleFromVoltage(1.f * _refdc / x.ch2_avg);
            m_calib_parameters.fe_ch1_fs_g_hi = ch1_calib;
            m_calib_parameters.fe_ch2_fs_g_hi = ch2_calib;
            m_pass_data.ch1 = m_calib_parameters.fe_ch1_fs_g_hi;
            m_pass_data.ch2 = m_calib_parameters.fe_ch2_fs_g_hi;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
            return 0;
        }

        case 3: {
            m_acq->setLV();
            auto x = getData(10);
            m_calib_parameters.fe_ch1_lo_offs = x.ch1_avg_raw;
            m_calib_parameters.fe_ch2_lo_offs = x.ch2_avg_raw;
            m_pass_data.ch1 = m_calib_parameters.fe_ch1_lo_offs;
            m_pass_data.ch2 = m_calib_parameters.fe_ch2_lo_offs;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
            return 0;
        }

        case 4: {
            m_acq->setLV();
            auto x = getData(10);
            uint32_t ch1_calib = calibFullScaleFromVoltage(20.f * _refdc / x.ch1_avg);
            uint32_t ch2_calib = calibFullScaleFromVoltage(20.f * _refdc / x.ch2_avg);
            m_calib_parameters.fe_ch1_fs_g_lo = ch1_calib;
            m_calib_parameters.fe_ch2_fs_g_lo = ch2_calib;
            m_pass_data.ch1 = m_calib_parameters.fe_ch1_fs_g_lo;
            m_pass_data.ch2 = m_calib_parameters.fe_ch2_fs_g_lo;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
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
            m_calib_parameters.be_ch1_dc_offs = x.ch1_avg * -(1 << ADC_BITS-1);
            m_calib_parameters.be_ch2_dc_offs = x.ch2_avg * -(1 << ADC_BITS-1);
            m_pass_data.ch1 = m_calib_parameters.be_ch1_dc_offs;
            m_pass_data.ch2 = m_calib_parameters.be_ch2_dc_offs;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
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
            uint32_t ch1_calib = calibFullScaleFromVoltage(1.f * x.ch1_avg / 0.5);
            uint32_t ch2_calib = calibFullScaleFromVoltage(1.f * x.ch2_avg / 0.5);
            m_calib_parameters.be_ch1_fs = ch1_calib;
            m_calib_parameters.be_ch2_fs = ch2_calib;
            m_pass_data.ch1 = m_calib_parameters.be_ch1_fs;
            m_pass_data.ch2 = m_calib_parameters.be_ch2_fs;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
            m_acq->setGEN0_5();
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            return 0;
        }
    }
    return 0;
}
#endif

#ifdef Z20_250_12
int CCalib::calib_board(uint16_t _step,float _refdc){
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
            m_calib_parameters.osc_ch1_off_1_dc = x.ch1_avg_raw;
            m_calib_parameters.osc_ch2_off_1_dc = x.ch2_avg_raw;
            m_pass_data.ch1 = m_calib_parameters.osc_ch1_off_1_dc;
            m_pass_data.ch2 = m_calib_parameters.osc_ch2_off_1_dc;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
            return 0;
        }

        case 3: {
            m_acq->setDC();
            m_acq->setLV();
            auto x = getData(10);
            uint32_t ch1_calib = calibFullScaleFromVoltage(20.f * _refdc / x.ch1_avg);
            uint32_t ch2_calib = calibFullScaleFromVoltage(20.f * _refdc / x.ch2_avg);
            m_calib_parameters.osc_ch1_g_1_dc = ch1_calib;
            m_calib_parameters.osc_ch2_g_1_dc = ch2_calib;
            m_pass_data.ch1 = m_calib_parameters.osc_ch1_g_1_dc;
            m_pass_data.ch2 = m_calib_parameters.osc_ch2_g_1_dc;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
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
            m_calib_parameters.osc_ch1_off_20_dc = x.ch1_avg_raw;
            m_calib_parameters.osc_ch2_off_20_dc = x.ch2_avg_raw;
            m_pass_data.ch1 = m_calib_parameters.osc_ch1_off_20_dc;
            m_pass_data.ch2 = m_calib_parameters.osc_ch2_off_20_dc;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
            return 0;
        }

         case 6: {
            m_acq->setDC();
            m_acq->setHV();
            auto x = getData(10);
            uint32_t ch1_calib = calibFullScaleFromVoltage(1.f * _refdc / x.ch1_avg);
            uint32_t ch2_calib = calibFullScaleFromVoltage(1.f * _refdc / x.ch2_avg);
            m_calib_parameters.osc_ch1_g_20_dc = ch1_calib;
            m_calib_parameters.osc_ch2_g_20_dc = ch2_calib;
            m_pass_data.ch1 = m_calib_parameters.osc_ch1_g_20_dc;
            m_pass_data.ch2 = m_calib_parameters.osc_ch2_g_20_dc;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
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
            m_calib_parameters.gen_ch1_off_1 = x.ch1_avg * -(1 << ADC_BITS-1);
            m_calib_parameters.gen_ch2_off_1 = x.ch2_avg * -(1 << ADC_BITS-1);
            m_pass_data.ch1 = m_calib_parameters.gen_ch1_off_1;
            m_pass_data.ch2 = m_calib_parameters.gen_ch2_off_1;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
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
            uint32_t ch1_calib = calibFullScaleFromVoltage(2.f * x.ch1_avg / 0.5);
            uint32_t ch2_calib = calibFullScaleFromVoltage(2.f * x.ch2_avg / 0.5);
            m_calib_parameters.gen_ch1_g_1 = ch1_calib;
            m_calib_parameters.gen_ch2_g_1 = ch2_calib;
            m_pass_data.ch1 = m_calib_parameters.gen_ch1_g_1;
            m_pass_data.ch2 = m_calib_parameters.gen_ch2_g_1;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
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
            m_calib_parameters.gen_ch1_off_5 = (x.ch1_avg * -(1 << ADC_BITS-1)) / 5.0;
            m_calib_parameters.gen_ch2_off_5 = (x.ch2_avg * -(1 << ADC_BITS-1)) / 5.0;
            m_pass_data.ch1 = m_calib_parameters.gen_ch1_off_5;
            m_pass_data.ch2 = m_calib_parameters.gen_ch2_off_5;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
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
            uint32_t ch1_calib = calibFullScaleFromVoltage(2.f * x.ch1_avg / 2.5);
            uint32_t ch2_calib = calibFullScaleFromVoltage(2.f * x.ch2_avg / 2.5);
            m_calib_parameters.gen_ch1_g_5 = ch1_calib;
            m_calib_parameters.gen_ch2_g_5 = ch2_calib;
            m_pass_data.ch1 = m_calib_parameters.gen_ch1_g_5;
            m_pass_data.ch2 = m_calib_parameters.gen_ch2_g_5;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
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
            m_calib_parameters.osc_ch1_off_1_ac = x.ch1_avg_raw;
            m_calib_parameters.osc_ch2_off_1_ac = x.ch2_avg_raw;
            m_pass_data.ch1 = m_calib_parameters.osc_ch1_off_1_ac;
            m_pass_data.ch2 = m_calib_parameters.osc_ch2_off_1_ac;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
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
            uint32_t ch1_calib = calibFullScaleFromVoltage(20.f * 0.5 / x.ch1_max);
            uint32_t ch2_calib = calibFullScaleFromVoltage(20.f * 0.5 / x.ch2_max);
            m_calib_parameters.osc_ch1_g_1_ac = ch1_calib;
            m_calib_parameters.osc_ch2_g_1_ac = ch2_calib;
            m_pass_data.ch1 = m_calib_parameters.osc_ch1_g_1_ac;
            m_pass_data.ch2 = m_calib_parameters.osc_ch2_g_1_ac;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
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
            m_calib_parameters.osc_ch1_off_20_ac = x.ch1_avg_raw;
            m_calib_parameters.osc_ch2_off_20_ac = x.ch2_avg_raw;
            m_pass_data.ch1 = m_calib_parameters.osc_ch1_off_20_ac;
            m_pass_data.ch2 = m_calib_parameters.osc_ch2_off_20_ac;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
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
            uint32_t ch1_calib = calibFullScaleFromVoltage(1.f * 2.5 / x.ch1_max);
            uint32_t ch2_calib = calibFullScaleFromVoltage(1.f * 2.5 / x.ch2_max);
            m_calib_parameters.osc_ch1_g_20_ac = ch1_calib;
            m_calib_parameters.osc_ch2_g_20_ac = ch2_calib;
            m_pass_data.ch1 = m_calib_parameters.osc_ch1_g_20_ac;
            m_pass_data.ch2 = m_calib_parameters.osc_ch2_g_20_ac;
            rp_CalibrationWriteParams(m_calib_parameters);
            rp_CalibInit();
            return 0;
        }
       
    }
    return 0;
}
#endif