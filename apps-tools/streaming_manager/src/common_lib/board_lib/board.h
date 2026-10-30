#ifndef BOARD_LIB_BOARD_H
#define BOARD_LIB_BOARD_H

#include <array>
#include <memory>

#include "uio_lib/hardware.h"

namespace board_lib {

// The board the streaming server runs on: the profile (librp-hw-profiles), the calibration in
// the EEPROM (librp-hw-calib) and the front end relays (librp-hw). A host build gets a dummy.
auto createBoard() -> std::shared_ptr<uio_lib::IBoard>;

// Host builds: the values set by its user (tests, emulation); records the front end it is given
class CDummyBoard : public uio_lib::IBoard {
   public:
    explicit CDummyBoard(uio_lib::BoardInfo info = {});

    auto info() const -> uio_lib::BoardInfo override;
    auto loadCalibration() -> bool override;
    auto adcCalibrationPlace() -> uio_lib::ADCCalibrationPlace override;
    auto adcCalibration(uint8_t ch, bool att1_20, bool dc) -> uio_lib::ADCCalibration override;
    auto dacCalibration(uint8_t ch, bool gain5x) -> uio_lib::DACCalibration override;
    auto setADCFrontEnd(uint8_t ch, bool att1_20, bool dc) -> void override;
    auto setDACGain(uint8_t ch, bool gain5x) -> void override;

    struct FrontEnd {
        bool set = false;
        bool att1_20 = false;
        bool dc = false;
    };

    uio_lib::ADCCalibrationPlace place = uio_lib::ADCCalibrationPlace::BEFORE_FILTER;
    bool calibrationReadable = true;
    // [channel][att1_20][dc] and [channel][gain5x]
    std::array<std::array<std::array<uio_lib::ADCCalibration, 2>, 2>, 4> adc{};
    std::array<std::array<uio_lib::DACCalibration, 2>, 2> dac{};
    std::array<FrontEnd, 4> frontEnd{};
    std::array<int, 2> dacGain{-1, -1};  // -1 not set, 0 x1, 1 x5
    int calibrationLoads = 0;

   private:
    uio_lib::BoardInfo m_info;
};

}  // namespace board_lib

#endif
