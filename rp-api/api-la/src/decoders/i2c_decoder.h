#pragma once

#include "decoder.h"
#include "i2c_settings.h"

#include <vector>

namespace i2c {

class I2CDecoder : public Decoder {
    class Impl;

   public:
    I2CDecoder(int decoderType, const std::string& _name = "i2c");
    ~I2CDecoder();

    auto setParameters(const I2CParameters& _new_params) -> void;
    auto getParametersInJSON() -> std::string override;
    auto setParametersInJSON(const std::string& parameter) -> void override;
    auto getMemoryUsage() -> uint64_t override;
    auto decode(const uint8_t* _input, uint32_t _size) -> void override;
    auto getSignal() -> std::vector<OutputPacket> override;
    auto reset() -> void override;

    auto setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool override;
    auto getDecoderSettingsUInt(std::string& key, uint32_t* value) -> bool override;

   private:
    Impl* m_impl;
};

}  // namespace i2c