#pragma once

#include "common/enums.h"
#include "decoder.h"

#include <cstring>
#include <deque>
#include <string>
#include <vector>

namespace i2c {

ENUM(InvertBit, No = 0, "No", Yes = 1, "Yes")
ENUM(AddressFormat, Shifted = 0, "Shifted", Unshifted = 1, "Unshifted")
ENUM(Lines, None = 0, "None", DIN0 = 1, "DIN0", DIN1 = 2, "DIN1", DIN2 = 3, "DIN2", DIN3 = 4, "DIN3", DIN4 = 5, "DIN4", DIN5 = 6, "DIN5", DIN6 = 7, "DIN6", DIN7 = 8, "DIN7")

enum I2CAnnotations { START = 0, REPEAT_START = 1, STOP = 2, ACK = 3, NACK = 4, READ_ADDRESS = 5, WRITE_ADDRESS = 6, DATA_READ = 7, DATA_WRITE = 8, ENUM_END };

class I2CParameters : public DecoderParameters {
   public:
    Lines m_scl = Lines::None;  // 1...8
    Lines m_sda = Lines::None;  // 1...8
    uint32_t m_acq_speed;
    AddressFormat m_address_format = AddressFormat::Shifted;
    InvertBit m_invert_bit = InvertBit::No;

    I2CParameters();

    auto toJson() -> std::string override;
    auto fromJson(const std::string& json) -> bool override;

    static std::string getI2CAnnotationsString(I2CAnnotations value);

    auto setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool override;
    auto getDecoderSettingsUInt(std::string& key, uint32_t* value) -> bool override;
};

}  // namespace i2c