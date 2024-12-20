#pragma once

#include "decoder.h"
#include "can_settings.h"
#include <vector>

namespace can{

class CANDecoder : public Decoder
{
	class Impl;

public:
    CANDecoder(int decoderType, const std::string& _name = "can");
    ~CANDecoder();

    auto setParameters(const CANParameters& _new_params) -> void;
    auto getParametersInJSON() -> std::string override;
    auto setParametersInJSON(const std::string &parameter) -> void override;
    auto getMemoryUsage() -> uint64_t override;
    auto decode(const uint8_t* _input, uint32_t _size) -> void override;
    auto getSignal() -> std::vector<OutputPacket> override;
    auto reset() -> void override;

    auto setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool override;
	auto setDecoderSettingsFloat(std::string& key, float value) -> bool override;
	auto getDecoderSettingsUInt(std::string& key, uint32_t *value) -> bool override;
    auto getDecoderSettingsFloat(std::string& key, float *value) -> bool override;

private:
	Impl *m_impl;
};

}