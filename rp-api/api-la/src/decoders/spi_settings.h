#pragma once

#include <cstdint>
#include <cstdlib>
#include <deque>
#include <string>
#include <vector>

#include "common/enums.h"
#include "decoder.h"

namespace spi {

ENUM(InvertBit, No = 0, "No", Yes = 1, "Yes")
ENUM(CsPolartiy, ActiveLow = 0, "ActiveLow", ActiveHigh = 1, "ActiveHigh")
ENUM(BitOrder, MsbFirst = 0, "MsbFirst", LsbFirst = 1, "LsbFirst")
ENUM(Lines, None = 0, "None", DIN0 = 1, "DIN0", DIN1 = 2, "DIN1", DIN2 = 3, "DIN2", DIN3 = 4, "DIN3", DIN4 = 5, "DIN4", DIN5 = 6, "DIN5", DIN6 = 7, "DIN6", DIN7 = 8, "DIN7")

enum SPIAnnotations { DATA = 0, ENUM_END };

class SPIParameters : public DecoderParameters {
   public:
    Lines m_clk = Lines::None;   // 1...8 	ch Number
    Lines m_miso = Lines::None;  // 1...8
    Lines m_mosi = Lines::None;  // 1...8
    Lines m_cs = Lines::None;    // 0...8, 	0 if is not set
    uint32_t m_cpol = 0;
    uint32_t m_cpha = 0;
    uint32_t m_word_size = 8;
    uint32_t m_acq_speed;
    CsPolartiy m_cs_polarity = CsPolartiy::ActiveLow;
    BitOrder m_bit_order = BitOrder::MsbFirst;
    InvertBit m_invert_bit = InvertBit::No;

    SPIParameters();

    auto toJson() -> std::string override;
    auto fromJson(const std::string& json) -> bool override;

    static std::string getSPIAnnotationsString(SPIAnnotations value);

    auto setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool override;
    auto getDecoderSettingsUInt(std::string& key, uint32_t* value) -> bool override;
};
}  // namespace spi
