#pragma once

#include "decoder.h"

#include <cstring>
#include <string>
#include <vector>
#include <deque>

namespace i2c {
	enum AddressFormat
	{
		Shifted		= 0,
		Unshifted	= 1
	};

	enum I2CAnnotations
	{
		START 			= 1,
		REPEAT_START	= 2,
		STOP			= 3,
		ACK				= 4,
		NACK			= 5,
		READ_ADDRESS	= 6,
		WRITE_ADDRESS	= 7,
		DATA_READ		= 8,
		DATA_WRITE		= 9,
		NOTHING			= 10
	};

	class I2CParameters : public DecoderParameters
	{
		public:

			uint32_t 		m_scl; // 1...8
			uint32_t 		m_sda; // 1...8
			uint32_t 		m_acq_speed;
			AddressFormat 	m_address_format;
			uint32_t  		m_invert_bit;


		I2CParameters();

		auto toJson() -> std::string;
    	auto fromJson(const std::string &json) -> bool;

		static constexpr std::string_view getI2CAnnotationsString(I2CAnnotations value);
	};

}