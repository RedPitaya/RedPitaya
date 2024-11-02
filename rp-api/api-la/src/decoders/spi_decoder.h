#pragma once

#include "decoder.h"
#include "spi_settings.h"
#include <vector>


namespace spi{

class SPIDecoder : public Decoder
{
	class Impl;

public:
    SPIDecoder(const std::string& _name = "spi");
    ~SPIDecoder();

    void setParameters(const SPIParameters& _new_params);
    void decode(const uint8_t* _input, uint32_t _size) override;
    std::vector<OutputPacket> getSignal();

private:
	Impl *m_impl;
};

}