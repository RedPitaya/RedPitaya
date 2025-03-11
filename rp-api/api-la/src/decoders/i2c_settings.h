#pragma once

#include "decoder.h"

#include <cstring>
#include <deque>
#include <string>
#include <vector>

namespace i2c {

enum AddressFormat { Shifted = 0, Unshifted = 1 };

enum I2CAnnotations { START = 0, REPEAT_START = 1, STOP = 2, ACK = 3, NACK = 4, READ_ADDRESS = 5, WRITE_ADDRESS = 6, DATA_READ = 7, DATA_WRITE = 8, ENUM_END };

class I2CParameters : public DecoderParameters {
   public:
    uint32_t m_scl;  // 1...8
    uint32_t m_sda;  // 1...8
    uint32_t m_acq_speed;
    AddressFormat m_address_format;
    uint32_t m_invert_bit;

    I2CParameters();

    auto toJson() -> std::string override;
    auto fromJson(const std::string& json) -> bool override;

    static std::string getI2CAnnotationsString(I2CAnnotations value);

    auto setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool override;
    auto getDecoderSettingsUInt(std::string& key, uint32_t* value) -> bool override;
};

}  // namespace i2c