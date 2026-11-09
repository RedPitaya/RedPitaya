#include "board.h"

#ifdef RP_PLATFORM
#include "api250-12/rp-i2c-max7311.h"
#include "rp_hw-profiles.h"
#include "rp_hw_calib.h"
#endif

#include "logger_lib/file_logger.h"

using namespace board_lib;
using namespace uio_lib;

CDummyBoard::CDummyBoard(BoardInfo info) : m_info(info) {}

auto CDummyBoard::info() const -> BoardInfo {
    return m_info;
}

auto CDummyBoard::loadCalibration() -> bool {
    calibrationLoads++;
    return calibrationReadable;
}

auto CDummyBoard::adcCalibrationPlace() -> ADCCalibrationPlace {
    return place;
}

auto CDummyBoard::adcCalibration(uint8_t ch, bool att1_20, bool dc) -> ADCCalibration {
    return ch < 4 ? adc[ch][att1_20][dc] : ADCCalibration();
}

auto CDummyBoard::dacCalibration(uint8_t ch, bool gain5x) -> DACCalibration {
    return ch < 2 ? dac[ch][gain5x] : DACCalibration();
}

auto CDummyBoard::setADCFrontEnd(uint8_t ch, bool att1_20, bool dc) -> void {
    if (ch < 4)
        frontEnd[ch] = {true, att1_20, dc};
}

auto CDummyBoard::setDACGain(uint8_t ch, bool gain5x) -> void {
    if (ch < 2)
        dacGain[ch] = gain5x ? 1 : 0;
}

#ifdef RP_PLATFORM

namespace {

class CRPBoard : public IBoard {
   public:
    CRPBoard() {
        if (rp_HPGetFastADCChannelsCount(&m_info.adcChannels) != RP_HP_OK)
            ERROR_LOG("Can't get fast ADC channels count")
        if (rp_HPGetFastDACChannelsCount(&m_info.dacChannels) != RP_HP_OK)
            ERROR_LOG("Can't get fast DAC channels count")
        if (rp_HPGetBaseFastADCSpeedHz(&m_info.adcRate) != RP_HP_OK)
            ERROR_LOG("Can't get fast ADC rate")
        if (rp_HPGetBaseFastDACSpeedHz(&m_info.dacRate) != RP_HP_OK)
            ERROR_LOG("Can't get fast DAC rate")
        if (rp_HPGetFastADCBits(&m_info.adcBits) != RP_HP_OK)
            ERROR_LOG("Can't get fast ADC bits")
        m_info.adcFilter = rp_HPGetFastADCIsFilterPresentOrDefault();
        m_info.dacGain = rp_HPGetIsGainDACx5OrDefault();
        if (rp_HPGetIsXStreamingAvailable(&m_info.daisyChain) != RP_HP_OK)
            ERROR_LOG("Can't get the x-streaming mode")
        m_info.adcRanges = rp_HPGetFastADCIsLV_HVOrDefault();
        m_info.adcACDC = rp_HPGetFastADCIsAC_DCOrDefault();
        m_attenuator = rp_HPGetIsAttenuatorControllerPresentOrDefault();
        rp_HPeModels_t model = STEM_125_14_v1_0;
        if (rp_HPGetModel(&model) == RP_HP_OK)
            m_info.model = static_cast<uint8_t>(model);
        else
            ERROR_LOG("Can't get the board model")
        char* mac = nullptr;
        if (rp_HPGetModelETH_MAC_Address(&mac) == RP_HP_OK && mac)
            m_info.mac = mac;
        else
            ERROR_LOG("Can't get the MAC address")
        // the relay controller of the 250-12 (attenuators, AC/DC, DAC gain) to its defaults
        if (m_attenuator && m_info.adcACDC && m_info.dacGain)
            rp_max7311::rp_initController();
    }

    auto info() const -> BoardInfo override { return m_info; }

    auto loadCalibration() -> bool override {
        if (rp_CalibInit() != RP_HW_CALIB_OK) {
            ERROR_LOG("Error init calibration")
            return false;
        }
        uint8_t version = 0;
        if (rp_GetCalibrationVersion(&version) == RP_HW_CALIB_OK) {
            m_place = version >= RP_HW_PACK_ID_V6 ? ADCCalibrationPlace::BEFORE_FILTER : ADCCalibrationPlace::AFTER_FILTER;
        } else {
            ERROR_LOG("Error get calibration version")
            m_place = ADCCalibrationPlace::AFTER_FILTER;
        }
        return true;
    }

    auto adcCalibrationPlace() -> ADCCalibrationPlace override { return m_place; }

    auto adcCalibration(uint8_t ch, bool att1_20, bool dc) -> ADCCalibration override {
        ADCCalibration c;
        auto mode = dc ? RP_DC_CALIB : RP_AC_CALIB;
        auto channel = (rp_channel_calib_t)ch;
        auto res = att1_20 ? rp_CalibGetFastADCCalibValue_1_20(channel, mode, &c.gain, &c.offset) : rp_CalibGetFastADCCalibValue(channel, mode, &c.gain, &c.offset);
        if (res != RP_HW_CALIB_OK)
            ERROR_LOG("Error get calibration channel: %d", ch)
        if (m_info.adcFilter) {
            channel_filter_t f;
            res = att1_20 ? rp_CalibGetFastADCFilter_1_20(channel, &f) : rp_CalibGetFastADCFilter(channel, &f);
            if (res == RP_HW_CALIB_OK) {
                c.aa = f.aa;
                c.bb = f.bb;
                c.kk = f.kk;
                c.pp = f.pp;
            } else {
                ERROR_LOG("Error get filter value: %d", ch)
            }
        }
        return c;
    }

    auto dacCalibration(uint8_t ch, bool gain5x) -> DACCalibration override {
        DACCalibration c;
        if (rp_CalibGetFastDACCalibValue((rp_channel_calib_t)ch, gain5x ? RP_GAIN_CALIB_5X : RP_GAIN_CALIB_1X, &c.gain, &c.offset) != RP_HW_CALIB_OK)
            ERROR_LOG("Error get calibration channel: %d", ch)
        return c;
    }

    // the relays exist for IN1 and IN2 only
    auto setADCFrontEnd(uint8_t ch, bool att1_20, bool dc) -> void override {
        if (ch > 1)
            return;
        auto in = ch == 0 ? RP_MAX7311_IN1 : RP_MAX7311_IN2;
        if (m_attenuator)
            rp_max7311::rp_setAttenuator(in, att1_20 ? RP_ATTENUATOR_1_20 : RP_ATTENUATOR_1_1);
        if (m_info.adcACDC)
            rp_max7311::rp_setAC_DC(in, dc ? RP_DC_MODE : RP_AC_MODE);
    }

    auto setDACGain(uint8_t ch, bool gain5x) -> void override {
        if (ch > 1 || !m_info.dacGain)
            return;
        rp_max7311::rp_setGainOut(ch == 0 ? RP_MAX7311_OUT1 : RP_MAX7311_OUT2, gain5x ? RP_GAIN_10V : RP_GAIN_2V);
    }

   private:
    BoardInfo m_info;
    bool m_attenuator = false;  // the relay controller of the 250-12
    ADCCalibrationPlace m_place = ADCCalibrationPlace::AFTER_FILTER;
};

}  // namespace

auto board_lib::createBoard() -> std::shared_ptr<IBoard> {
    return std::make_shared<CRPBoard>();
}

#else

auto board_lib::createBoard() -> std::shared_ptr<IBoard> {
    // the emulation of the former servers
    BoardInfo info;
    info.adcRate = 10000000;
    info.dacRate = 10000000;
    info.adcBits = 16;
    return std::make_shared<CDummyBoard>(info);
}

#endif
