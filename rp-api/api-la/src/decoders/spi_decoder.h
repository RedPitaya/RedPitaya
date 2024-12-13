#pragma once

#include "decoder.h"
#include "spi_settings.h"
#include <vector>


namespace spi{

class SPIDecoder : public Decoder
{
	class Impl;

public:
    SPIDecoder(int decoderType, const std::string& _name = "spi");
    ~SPIDecoder();

    auto setParameters(const SPIParameters& _new_params) -> void;
    auto getParametersInJSON() -> std::string override;
    auto setParametersInJSON(const std::string &parameter) -> void override;
    auto getMemoryUsage() -> uint64_t override;
    auto decode(const uint8_t* _input, uint32_t _size) -> void override;
    auto getSignal() -> std::vector<OutputPacket> override;
    auto reset() -> void override;

private:
	Impl *m_impl_miso;
	Impl *m_impl_mosi;
};

}