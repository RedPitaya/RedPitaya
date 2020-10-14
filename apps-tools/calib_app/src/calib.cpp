#include <fstream>
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


// int getCalibGainLV(rp_channel_t channel, float referentialVoltage, rp_calib_params_t* out_params) {
//     rp_calib_params_t params;
//     calib_ReadParams(&params,false);
// 	failsafa_params = params;

//     /* Reset current calibration parameters*/
//     CHANNEL_ACTION(channel,
//             params.fe_ch1_fs_g_lo = cmn_CalibFullScaleFromVoltage(20),
//             params.fe_ch2_fs_g_lo = cmn_CalibFullScaleFromVoltage(20))
//     /* Acquire uses this calibration parameters - reset them */
//     calib = params;

//     /* Calculate real max adc voltage */
//     float value = calib_GetDataMedianFloat(channel, RP_LOW);
//     uint32_t calibValue = cmn_CalibFullScaleFromVoltage(20.f * referentialVoltage / value);

//     CHANNEL_ACTION(channel,
//             params.fe_ch1_fs_g_lo = calibValue,
//             params.fe_ch2_fs_g_lo = calibValue )

//     /* Set new local parameter */
//     if  (out_params) {
// 		//	*out_params = params;
// 		CHANNEL_ACTION(channel,
// 				out_params->fe_ch1_fs_g_lo = params.fe_ch1_fs_g_lo,
// 				out_params->fe_ch2_fs_g_lo = params.fe_ch2_fs_g_lo)
// 	}
//     else
// 		calib_WriteParams(params,false);
//     return calib_Init();
// }



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

#define Z10
#ifdef Z10
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
    return 0;
}
#endif