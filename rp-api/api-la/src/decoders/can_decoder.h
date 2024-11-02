#pragma once

#include "decoder.h"
#include "can_settings.h"
#include <vector>

namespace can{

class CANDecoder : public Decoder
{
	class Impl;

public:
    CANDecoder(const std::string& _name = "can");
    ~CANDecoder();

    void setParameters(const CANParameters& _new_params);
    void decode(const uint8_t* _input, uint32_t _size) override;
    std::vector<OutputPacket> getSignal();

private:
	Impl *m_impl;
};

}