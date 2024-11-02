#pragma once

#include "decoder.h"
#include "uart_settings.h"
#include <vector>


namespace uart{

class UARTDecoder : public Decoder
{
	class Impl;

public:
    UARTDecoder(const std::string& _name = "uart");
    ~UARTDecoder();

    void setParameters(const UARTParameters& _new_params);
    void decode(const uint8_t* _input, uint32_t _size) override;
    std::vector<OutputPacket> getSignal();

private:
	Impl *m_impl;
};

}
