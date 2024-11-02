#pragma once

#include "decoder.h"
#include "i2c_settings.h"

#include <vector>

namespace i2c{

class I2CDecoder : public Decoder
{
	class Impl;

public:
    I2CDecoder(const std::string& _name = "i2c");
    ~I2CDecoder();

    void setParameters(const I2CParameters& _new_params);
    void decode(const uint8_t* _input, uint32_t _size) override;

    std::vector<OutputPacket> getSignal();

private:

	Impl *m_impl;

};

}