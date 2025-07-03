#pragma once

#include <cstdint>
#include <cstdlib>
#include <deque>
#include <string>
#include <vector>

#include "common/enums.h"
#include "decoder.h"

namespace uart {

enum UARTAnnotations { PARITY_ERR = 0, START_BIT_ERR = 1, STOP_BIT_ERR = 2, DATA = 3, START_BIT = 4, STOP_BIT = 5, PARITY_BIT = 6, ENUM_END };

ENUM(InvertBit, No = 0, "No", Yes = 1, "Yes")
ENUM(UartBitOrder, LsbFirst = 0, "LsbFirst", MsbFirst = 1, "MsbFirst")
ENUM(Parity, None = 0, "None", Even = 1, "Even", Odd = 2, "Odd", Always_0 = 3, "Always_0", Always_1 = 4, "Always_1")
ENUM(NumDataBits, Bits5 = 5, "Bits5", Bits6 = 6, "Bits6", Bits7 = 7, "Bits7", Bits8 = 8, "Bits8", Bits9 = 9, "Bits9")
ENUM(NumStopBits, Stop_Bit_No = 0, "Stop_Bit_No", Stop_Bit_05 = 1, "Stop_Bit_05", Stop_Bit_10 = 2, "Stop_Bit_10", Stop_Bit_15 = 3, "Stop_Bit_15", Stop_Bit_20 = 4, "Stop_Bit_20")
ENUM(Lines, None = 0, "None", DIN0 = 1, "DIN0", DIN1 = 2, "DIN1", DIN2 = 3, "DIN2", DIN3 = 4, "DIN3", DIN4 = 5, "DIN4", DIN5 = 6, "DIN5", DIN6 = 7, "DIN6", DIN7 = 8, "DIN7")

class UARTParameters : public DecoderParameters {
   public:
    Lines m_rx = Lines::None;  // 1..8
    Lines m_tx = Lines::None;  // 1..8
    uint32_t m_baudrate;
    InvertBit m_invert = InvertBit::No;
    UartBitOrder m_bitOrder = UartBitOrder::LsbFirst;
    NumDataBits m_num_data_bits = NumDataBits::Bits8;  // 5..9
    Parity m_parity = Parity::None;
    NumStopBits m_num_stop_bits = NumStopBits::Stop_Bit_10;
    uint32_t m_samplerate;

    UARTParameters();

    auto toJson() -> std::string override;
    auto fromJson(const std::string& json) -> bool override;

    static std::string getUARTAnnotationsString(UARTAnnotations value);

    auto setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool override;
    auto getDecoderSettingsUInt(std::string& key, uint32_t* value) -> bool override;
};
}  // namespace uart