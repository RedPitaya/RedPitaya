#pragma once

#include "decoder.h"
#include "uart_settings.h"
#include <vector>


namespace uart{

class UARTDecoder : public Decoder
{
	class Impl;

public:
    UARTDecoder(int decoderType, const std::string& _name = "uart");
    ~UARTDecoder();

    auto setParameters(const UARTParameters& _new_params) -> void;
    auto getParametersInJSON() -> std::string override;
    auto setParametersInJSON(const std::string &parameter) -> void override;
    auto getMemoryUsage() -> uint64_t override;
    auto decode(const uint8_t* _input, uint32_t _size) -> void override;
    auto getSignal() -> std::vector<OutputPacket> override;
    auto reset() -> void override;

    auto setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool override;
	auto getDecoderSettingsUInt(std::string& key, uint32_t *value) -> bool override;

private:
	Impl *m_impl_rx;
	Impl *m_impl_tx;
};

}
