#include "calib.h"
#include <unistd.h>
#include <ctime>
#include <fstream>
#include "common.h"

namespace rp_calib {

#define FOR(X)                           \
    for (int i = 0; i < m_channels; i++) \
        X;

CCalib::Ptr CCalib::Create(COscilloscope::Ptr _acq) {
    return std::make_shared<CCalib>(_acq);
}

CCalib::CCalib(COscilloscope::Ptr _acq) : m_acq(_acq), m_current_step(""), m_channels(0) {
    m_channels = getADCChannels();
    for (int i = 0; i < m_channels; i++) {
        m_pass_data.ch[i] = 0;
        m_pass_data.ch_gain[i] = 0;
    }
    m_calib_parameters = rp_GetCalibrationSettings();
    m_calib_parameters_old = rp_GetCalibrationSettings();
}

CCalib::~CCalib() {}

int CCalib::resetCalibToZero() {
    return rp_CalibrationReset(false, true, false, RP_HW_PACK_ID_V6);
}

int CCalib::resetCalibToFactory() {
    return rp_CalibrationFactoryReset(true);
}

void CCalib::restoreCalib() {
    rp_CalibrationWriteParams(m_calib_parameters_old, false);
    rp_CalibInit();
}

int CCalib::calib(std::string& _step, float _refdc) {
    switch (getModel()) {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
        case STEM_125_14_v2_0:
        case STEM_125_14_Pro_v2_0:
        case STEM_125_14_Z7020_Pro_v2_0:
        case STEM_125_14_Z7020_Ind_v2_0:
        case STEM_125_14_Z7020_Pro_v1_0:
        case STEM_125_14_Z7020_LL_v1_1:
        case STEM_125_14_Z7020_LL_v1_2:
        case STEM_65_16_Z7020_LL_v1_1:
        case STEM_65_16_Z7020_TI_v1_3:
        case STEM_125_14_Z7020_TI_v1_3:
            return calib_board_z10(_step, _refdc);

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1: {
            FATAL("Board can't be calibrate");
            exit(-1);
        }

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
        case STEM_125_14_Z7020_4IN_BO_v1_3:
            return calib_board_z20_4ch(_step, _refdc);

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
        case STEM_250_12_120:
            return calib_board_z20_250_12(_step, _refdc);
        default:
            FATAL("Can't get board model");
            exit(-1);
    }
    return 0;
}

COscilloscope::DataPass CCalib::getData(int skip_read) {
    auto old_d = m_acq->getData();
    while (skip_read > 0) {
        auto new_d = m_acq->getData();
        skip_read -= (new_d.index - old_d.index);
        old_d = new_d;
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    return old_d;
}

CCalib::DataPass CCalib::getCalibData() {
    return m_pass_data;
}

int CCalib::calib_board_z10(std::string& _step, float _refdc) {

    if (m_current_step == _step)
        return 0;
    m_current_step = _step;

    auto bits = rp_HPGetFastADCBitsOrDefault();
    auto adc_fs = rp_HPGetHWADCFullScaleOrDefault();
    auto dac_bits = rp_HPGetFastDACBitsOrDefault();
    auto dac_fs = rp_HPGetHWDACFullScaleOrDefault();
    auto bit_diff = bits - dac_bits;

    if (_step == "RESET_DEFAULT") {  // Reset to Default
        m_acq->setAvgFilter(false);
        m_acq->resetAvgFilter();
        m_acq->setDeciamtion(1024);
        m_acq->setFilterBypass(true);
        m_acq->startNormal();
        m_calib_parameters_old = rp_GetCalibrationSettings();
        resetCalibToZero();
        m_calib_parameters = rp_GetCalibrationSettings();
        return 0;
    }

    if (_step == "INIT_ADC_HV") {
        m_acq->setHV();
        return 0;
    }

    if (_step == "INIT_ADC_LV") {
        m_acq->setLV();
        return 0;
    }

    if (_step == "CALIB_ADC_HV_OFFSET") {
        auto x = getData(50);
        FOR(m_calib_parameters.fast_adc_1_20[i].offset = x.ch_avg_raw[i])
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_calib_parameters = rp_GetCalibrationSettings();
        m_acq->setHV();  // init calib values in FPGA
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_adc_1_20[i].offset)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_adc_1_20[i].gainCalc)
        return 0;
    }

    if (_step == "CALIB_ADC_HV_GAIN") {
        auto x = getData(50);
        FOR(m_calib_parameters.fast_adc_1_20[i].gainCalc = _refdc / (double)x.ch_avg[i])
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_calib_parameters = rp_GetCalibrationSettings();
        m_acq->setHV();  // init calib values in FPGA
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_adc_1_20[i].calibValue)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_adc_1_20[i].gainCalc)
        return 0;
    }

    if (_step == "CALIB_ADC_LV_OFFSET") {
        auto x = getData(50);
        FOR(m_calib_parameters.fast_adc_1_1[i].offset = x.ch_avg_raw[i])
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_calib_parameters = rp_GetCalibrationSettings();
        m_acq->setLV();  // init calib values in FPGA
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_adc_1_1[i].offset)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_adc_1_1[i].gainCalc)
        return 0;
    }

    if (_step == "CALIB_ADC_LV_GAIN") {
        auto x = getData(50);
        FOR(m_calib_parameters.fast_adc_1_1[i].gainCalc = _refdc / (double)x.ch_avg[i])
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_calib_parameters = rp_GetCalibrationSettings();
        m_acq->setLV();  // init calib values in FPGA
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_adc_1_1[i].calibValue)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_adc_1_1[i].gainCalc)
        return 0;
    }

    if (_step == "GEN_DISABLE") {
        m_acq->setGEN_DISABLE();
        m_acq->setLV();
        return 0;
    }

    if (_step == "GEN_0") {
        m_acq->setGEN0();
        m_acq->setLV();
        return 0;
    }

    if (_step == "GEN_LV_0_5") {
        m_acq->setGEN0_5();
        m_acq->setLV();
        return 0;
    }

    if (_step == "GEN_LV_0_5_NEG") {
        m_acq->setGEN0_5_NEG();
        m_acq->setLV();
        return 0;
    }

    if (_step == "GEN_LV_0_5_SINE") {
        m_acq->setGEN0_5_SINE();
        m_acq->setLV();
        return 0;
    }

    if (_step == "GEN_LV_CALIB_GAIN_F_STAGE") {

        m_acq->setLV();

        m_acq->setGEN0_5_NEG();
        auto x_neg = getData(50);

        m_acq->setGEN0_5();
        auto x_pos = getData(50);

        // 1Vp2p
        FOR(m_calib_parameters.fast_dac_x1[i].gainCalc = 1.0 / ((double)x_pos.ch_avg[i] - (double)x_neg.ch_avg[i]))
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_acq->setGEN0_5();
        m_calib_parameters = rp_GetCalibrationSettings();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_dac_x1[i].calibValue)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_dac_x1[i].gainCalc)
        return 0;
    }

    if (_step == "GEN_LV_CALIB_OFFSET_F_STAGE") {

        m_acq->setLV();
        m_acq->setGEN0();
        auto x = getData(100);

        for (int ch = 0; ch < 2; ch++) {
            // Convert BITS and Full scale
            auto raw = (float)(bit_diff >= 0 ? x.ch_avg_raw[ch] >> abs(bit_diff) : x.ch_avg_raw[ch] << abs(bit_diff)) / (dac_fs / adc_fs);
            m_calib_parameters.fast_dac_x1[ch].offset = -1 * raw;
        }

        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_acq->setGEN0();
        m_calib_parameters = rp_GetCalibrationSettings();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_dac_x1[i].offset)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_dac_x1[i].gainCalc)
        return 0;
    }

    if (_step == "GEN_LV_CALIB_GAIN_S_STAGE") {

        m_acq->setGEN0_5_NEG();
        auto x_neg = getData(50);

        m_acq->setGEN0_5();
        auto x_pos = getData(50);

        // 1Vp2p
        FOR(m_calib_parameters.fast_dac_x1[i].gainCalc = (double)m_calib_parameters.fast_dac_x1[i].gainCalc * 1.0 / ((double)x_pos.ch_avg[i] - (double)x_neg.ch_avg[i]))

        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_acq->setGEN0_5();
        m_calib_parameters = rp_GetCalibrationSettings();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_dac_x1[i].calibValue)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_dac_x1[i].gainCalc)
        return 0;
    }

    if (_step == "GEN_LV_CALIB_OFFSET_S_STAGE") {

        m_acq->setLV();
        m_acq->setGEN0();
        auto x = getData(50);

        for (int ch = 0; ch < 2; ch++) {
            // Convert BITS and Full scale
            auto raw = (float)(bit_diff >= 0 ? x.ch_avg_raw[ch] >> abs(bit_diff) : x.ch_avg_raw[ch] << abs(bit_diff)) / (dac_fs / adc_fs);
            m_calib_parameters.fast_dac_x1[ch].offset = -1 * raw + m_calib_parameters.fast_dac_x1[ch].offset;
        }

        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_acq->setGEN0();
        m_calib_parameters = rp_GetCalibrationSettings();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_dac_x1[i].offset)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_dac_x1[i].gainCalc)
        return 0;
    }

    return 0;
}

int CCalib::calib_board_z20_4ch(std::string& _step, float _refdc) {
    if (m_current_step == _step)
        return 0;
    m_current_step = _step;

    if (_step == "RESET_DEFAULT") {  // Reset to Default
        m_acq->setAvgFilter(false);
        m_acq->resetAvgFilter();
        m_acq->setDeciamtion(1024);
        m_acq->setFilterBypass(true);
        m_acq->startNormal();
        m_calib_parameters_old = rp_GetCalibrationSettings();
        resetCalibToZero();
        m_calib_parameters = rp_GetCalibrationSettings();
        return 0;
    }

    if (_step == "INIT_ADC_HV") {
        m_acq->setHV();
        return 0;
    }

    if (_step == "INIT_ADC_LV") {
        m_acq->setLV();
        return 0;
    }

    if (_step == "CALIB_ADC_HV_OFFSET") {
        auto x = getData(50);
        FOR(m_calib_parameters.fast_adc_1_20[i].offset = x.ch_avg_raw[i])
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_calib_parameters = rp_GetCalibrationSettings();
        m_acq->setHV();  // init calib values in FPGA
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_adc_1_20[i].offset)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_adc_1_20[i].gainCalc)
        return 0;
    }

    if (_step == "CALIB_ADC_HV_GAIN") {
        auto x = getData(50);
        FOR(m_calib_parameters.fast_adc_1_20[i].gainCalc = _refdc / (double)x.ch_avg[i])
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_calib_parameters = rp_GetCalibrationSettings();
        m_acq->setHV();  // init calib values in FPGA
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_adc_1_20[i].calibValue)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_adc_1_20[i].gainCalc)
        return 0;
    }

    if (_step == "CALIB_ADC_LV_OFFSET") {
        auto x = getData(50);
        FOR(m_calib_parameters.fast_adc_1_1[i].offset = x.ch_avg_raw[i])
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_calib_parameters = rp_GetCalibrationSettings();
        m_acq->setLV();  // init calib values in FPGA
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_adc_1_1[i].offset)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_adc_1_1[i].gainCalc)
        return 0;
    }

    if (_step == "CALIB_ADC_LV_GAIN") {
        auto x = getData(50);
        FOR(m_calib_parameters.fast_adc_1_1[i].gainCalc = _refdc / (double)x.ch_avg[i])
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_calib_parameters = rp_GetCalibrationSettings();
        m_acq->setLV();  // init calib values in FPGA
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_adc_1_1[i].calibValue)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_adc_1_1[i].gainCalc)
        return 0;
    }

    return 0;
}

int CCalib::calib_board_z20_250_12(std::string& _step, float _refdc) {
    if (m_current_step == _step)
        return 0;
    m_current_step = _step;

    auto bits = rp_HPGetFastADCBitsOrDefault();
    auto adc_fs = rp_HPGetHWADCFullScaleOrDefault();
    auto dac_bits = rp_HPGetFastDACBitsOrDefault();
    auto dac_fs = rp_HPGetHWDACFullScaleOrDefault();
    auto bit_diff = bits - dac_bits;

    if (_step == "RESET_DEFAULT") {  // Reset to Default
        m_acq->setAvgFilter(false);
        m_acq->resetAvgFilter();
        m_acq->setDeciamtion(1024);
        m_acq->startNormal();
        m_calib_parameters_old = rp_GetCalibrationSettings();
        resetCalibToZero();
        m_calib_parameters = rp_GetCalibrationSettings();
        return 0;
    }

    m_acq->setDeciamtion(1024);

    if (_step == "INIT_ADC_HV_DC") {
        m_acq->setHV();
        m_acq->setDC();
        return 0;
    }

    if (_step == "INIT_ADC_LV_DC") {
        m_acq->setLV();
        m_acq->setDC();
        return 0;
    }

    if (_step == "INIT_ADC_HV_AC") {
        m_acq->setHV();
        m_acq->setAC();
        return 0;
    }

    if (_step == "INIT_ADC_LV_AC") {
        m_acq->setLV();
        m_acq->setAC();
        return 0;
    }

    if (_step == "CALIB_ADC_LV_DC_OFFSET") {
        auto x = getData(50);
        FOR(m_calib_parameters.fast_adc_1_1[i].offset = x.ch_avg_raw[i])
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_calib_parameters = rp_GetCalibrationSettings();
        m_acq->setLV();  // init calib values in FPGA
        m_acq->setDC();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_adc_1_1[i].offset)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_adc_1_1[i].gainCalc)
        return 0;
    }

    if (_step == "CALIB_ADC_LV_DC_GAIN") {
        auto x = getData(50);
        FOR(m_calib_parameters.fast_adc_1_1[i].gainCalc = _refdc / (double)x.ch_avg[i])
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_calib_parameters = rp_GetCalibrationSettings();
        m_acq->setLV();  // init calib values in FPGA
        m_acq->setDC();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_adc_1_1[i].calibValue)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_adc_1_1[i].gainCalc)
        return 0;
    }

    if (_step == "CALIB_ADC_HV_DC_OFFSET") {
        auto x = getData(50);
        FOR(m_calib_parameters.fast_adc_1_20[i].offset = x.ch_avg_raw[i])
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_calib_parameters = rp_GetCalibrationSettings();
        m_acq->setHV();  // init calib values in FPGA
        m_acq->setDC();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_adc_1_20[i].offset)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_adc_1_20[i].gainCalc)
        return 0;
    }

    if (_step == "CALIB_ADC_HV_DC_GAIN") {
        auto x = getData(50);
        FOR(m_calib_parameters.fast_adc_1_20[i].gainCalc = _refdc / (double)x.ch_avg[i])
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_calib_parameters = rp_GetCalibrationSettings();
        m_acq->setHV();  // init calib values in FPGA
        m_acq->setDC();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_adc_1_20[i].calibValue)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_adc_1_20[i].gainCalc)
        return 0;
    }

    if (_step == "GEN_0_X1") {
        m_acq->setGEN0();
        m_acq->setLV();
        m_acq->setDC();
        m_acq->setGenGainx1();
        return 0;
    }

    if (_step == "GEN_LV_0_5_X1") {
        m_acq->setLV();
        m_acq->setDC();
        m_acq->setGenGainx1();
        m_acq->setGEN0_5();
        return 0;
    }

    if (_step == "GEN_LV_0_5_SINE_X1") {
        m_acq->setGenGainx1();
        m_acq->setGEN0_5_SINE();
        return 0;
    }

    if (_step == "GEN_LV_0_9_SINE_X1") {
        m_acq->setGenGainx1();
        m_acq->setGEN0_9_SINE();
        return 0;
    }

    if (_step == "GEN_HV_0_9_X5") {
        m_acq->setHV();
        m_acq->setDC();
        m_acq->setGenGainx5();
        m_acq->setGEN0_9();
        return 0;
    }

    if (_step == "GEN_HV_0_15_X5") {
        m_acq->setLV();
        m_acq->setDC();
        m_acq->setGenGainx5();
        m_acq->setGEN0_15();
        return 0;
    }

    if (_step == "GEN_DISABLE_X1") {
        m_acq->setLV();
        m_acq->setDC();
        m_acq->setGenGainx1();
        m_acq->setGEN0();
        return 0;
    }

    if (_step == "GEN_DISABLE_X5") {
        m_acq->setLV();
        m_acq->setDC();
        m_acq->setGenGainx5();
        m_acq->setGEN0();
        return 0;
    }

    if (_step == "GEN_LV_CALIB_GAIN_F_STAGE_X1") {
        m_acq->setLV();
        m_acq->setGenGainx1();
        m_acq->setGEN0_5_NEG();
        auto x_neg = getData(50);

        m_acq->setGEN0_5();
        auto x_pos = getData(50);

        // 1Vp2p
        FOR(m_calib_parameters.fast_dac_x1[i].gainCalc = 1.0 / ((double)x_pos.ch_avg[i] - (double)x_neg.ch_avg[i]))
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_acq->setGEN0_5();
        m_calib_parameters = rp_GetCalibrationSettings();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_dac_x1[i].calibValue)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_dac_x1[i].gainCalc)
        return 0;
    }

    if (_step == "GEN_LV_CALIB_OFFSET_F_STAGE_X1") {

        m_acq->setLV();
        m_acq->setGenGainx1();
        m_acq->setGEN0();
        auto x = getData(50);

        for (int ch = 0; ch < 2; ch++) {
            // Convert BITS and Full scale
            auto raw = (float)(bit_diff >= 0 ? x.ch_avg_raw[ch] >> abs(bit_diff) : x.ch_avg_raw[ch] << abs(bit_diff)) / (dac_fs / adc_fs);
            m_calib_parameters.fast_dac_x1[ch].offset = -1 * raw;
        }

        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_acq->setGEN0();
        m_calib_parameters = rp_GetCalibrationSettings();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_dac_x1[i].offset)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_dac_x1[i].gainCalc)
        return 0;
    }

    if (_step == "GEN_LV_CALIB_GAIN_S_STAGE_X1") {

        m_acq->setLV();
        m_acq->setGenGainx1();

        m_acq->setGEN0_5_NEG();
        auto x_neg = getData(50);

        m_acq->setGEN0_5();
        auto x_pos = getData(50);

        // 1Vp2p
        FOR(m_calib_parameters.fast_dac_x1[i].gainCalc = (double)m_calib_parameters.fast_dac_x1[i].gainCalc * 1.0 / ((double)x_pos.ch_avg[i] - (double)x_neg.ch_avg[i]))

        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_acq->setGEN0_5();
        m_calib_parameters = rp_GetCalibrationSettings();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_dac_x1[i].calibValue)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_dac_x1[i].gainCalc)
        return 0;
    }

    if (_step == "GEN_LV_CALIB_OFFSET_S_STAGE_X1") {

        m_acq->setLV();
        m_acq->setGenGainx1();
        m_acq->setGEN0();

        auto x = getData(50);

        for (int ch = 0; ch < 2; ch++) {
            // Convert BITS and Full scale
            auto raw = (float)(bit_diff >= 0 ? x.ch_avg_raw[ch] >> abs(bit_diff) : x.ch_avg_raw[ch] << abs(bit_diff)) / (dac_fs / adc_fs);
            m_calib_parameters.fast_dac_x1[ch].offset = -1 * raw + m_calib_parameters.fast_dac_x1[ch].offset;
        }

        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_acq->setGEN0();
        m_calib_parameters = rp_GetCalibrationSettings();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_dac_x1[i].offset)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_dac_x1[i].gainCalc)
        return 0;
    }

    if (_step == "GEN_CALIB_GAIN_F_STAGE_X5") {

        m_acq->setLV();
        m_acq->setGenGainx5();
        m_acq->setGEN0_15_NEG();
        auto x_neg = getData(50);

        m_acq->setGEN0_15();
        auto x_pos = getData(50);

        // 1Vp2p
        FOR(m_calib_parameters.fast_dac_x5[i].gainCalc = 1.5 / ((double)x_pos.ch_avg[i] - (double)x_neg.ch_avg[i]))
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_acq->setGEN0_15();
        m_calib_parameters = rp_GetCalibrationSettings();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_dac_x5[i].calibValue)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_dac_x5[i].gainCalc)
        return 0;
    }

    if (_step == "GEN_CALIB_OFFSET_F_STAGE_X5") {

        m_acq->setLV();
        m_acq->setGenGainx5();
        m_acq->setGEN0();
        auto x = getData(50);

        for (int ch = 0; ch < 2; ch++) {
            // Convert BITS and Full scale
            auto raw = (float)(bit_diff >= 0 ? x.ch_avg_raw[ch] >> abs(bit_diff) : x.ch_avg_raw[ch] << abs(bit_diff)) / (dac_fs / adc_fs);
            m_calib_parameters.fast_dac_x5[ch].offset = -1.0 * (double)raw / 5.0;
        }

        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_acq->setGEN0();
        m_calib_parameters = rp_GetCalibrationSettings();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_dac_x5[i].offset)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_dac_x5[i].gainCalc)
        return 0;
    }

    if (_step == "GEN_CALIB_GAIN_S_STAGE_X5") {

        m_acq->setLV();
        m_acq->setGenGainx5();

        m_acq->setGEN0_15_NEG();
        auto x_neg = getData(50);

        m_acq->setGEN0_15();
        auto x_pos = getData(50);

        // 1Vp2p
        FOR(m_calib_parameters.fast_dac_x5[i].gainCalc = (double)m_calib_parameters.fast_dac_x5[i].gainCalc * 1.5 / ((double)x_pos.ch_avg[i] - (double)x_neg.ch_avg[i]))

        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_acq->setGEN0_15();
        m_calib_parameters = rp_GetCalibrationSettings();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_dac_x5[i].calibValue)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_dac_x5[i].gainCalc)
        return 0;
    }

    if (_step == "GEN_CALIB_OFFSET_S_STAGE_X5") {

        m_acq->setLV();
        m_acq->setGenGainx5();
        m_acq->setGEN0();

        auto x = getData(50);

        for (int ch = 0; ch < 2; ch++) {
            // Convert BITS and Full scale
            auto raw = (float)(bit_diff >= 0 ? x.ch_avg_raw[ch] >> abs(bit_diff) : x.ch_avg_raw[ch] << abs(bit_diff)) / (dac_fs / adc_fs);
            m_calib_parameters.fast_dac_x5[ch].offset = -1 * raw / 5.0 + m_calib_parameters.fast_dac_x5[ch].offset;
        }

        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_acq->setGEN0();
        m_calib_parameters = rp_GetCalibrationSettings();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_dac_x5[i].offset)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_dac_x5[i].gainCalc)
        return 0;
    }

    if (_step == "CALIB_ADC_LV_AC_OFFSET") {
        auto x = getData(50);
        FOR(m_calib_parameters.fast_adc_1_1_ac[i].offset = x.ch_mean_raw[i])
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_calib_parameters = rp_GetCalibrationSettings();
        m_acq->setLV();  // init calib values in FPGA
        m_acq->setAC();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_adc_1_1_ac[i].offset)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_adc_1_1_ac[i].gainCalc)
        return 0;
    }

    if (_step == "CALIB_ADC_LV_AC_GAIN") {
        auto x = getData(50);
        FOR(m_calib_parameters.fast_adc_1_1_ac[i].gainCalc = 0.5 / (double)x.ch_max[i])
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_calib_parameters = rp_GetCalibrationSettings();
        m_acq->setLV();  // init calib values in FPGA
        m_acq->setAC();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_adc_1_1_ac[i].calibValue)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_adc_1_1_ac[i].gainCalc)
        return 0;
    }

    if (_step == "CALIB_ADC_HV_AC_OFFSET") {
        auto x = getData(50);
        FOR(m_calib_parameters.fast_adc_1_20_ac[i].offset = x.ch_mean_raw[i])
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_calib_parameters = rp_GetCalibrationSettings();
        m_acq->setHV();  // init calib values in FPGA
        m_acq->setAC();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_adc_1_20_ac[i].offset)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_adc_1_20_ac[i].gainCalc)
        return 0;
    }

    if (_step == "CALIB_ADC_HV_AC_GAIN") {
        auto x = getData(50);
        FOR(m_calib_parameters.fast_adc_1_20_ac[i].gainCalc = 0.9 / (double)x.ch_max[i])
        rp_CalibrationWriteParams(m_calib_parameters, false);
        rp_CalibInit();
        m_calib_parameters = rp_GetCalibrationSettings();
        m_acq->setHV();  // init calib values in FPGA
        m_acq->setAC();
        FOR(m_pass_data.ch[i] = m_calib_parameters.fast_adc_1_20_ac[i].calibValue)
        FOR(m_pass_data.ch_gain[i] = m_calib_parameters.fast_adc_1_20_ac[i].gainCalc)
        return 0;
    }

    return 0;
}

}  // namespace rp_calib