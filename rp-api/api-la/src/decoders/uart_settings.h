#pragma once

#include "decoder.h"

#include <vector>
#include <deque>
#include <string>
#include <cstdint>
#include <cstdlib>


namespace uart {

	enum UartBitOrder
	{
		LSB_FIRST,
		MSB_FIRST
	};

	enum NumDataBits
	{
		DATA_BITS_5 = 5,
		DATA_BITS_6,
		DATA_BITS_7,
		DATA_BITS_8,
		DATA_BITS_9
	};

	enum Parity
	{
		NONE,
		ODD,
		EVEN
	};

	enum NumStopBits
	{
		STOP_BITS_NO = 0,
		STOP_BITS_05,
		STOP_BITS_10,
		STOP_BITS_15,
		STOP_BITS_20
	};

	enum UARTAnnotations
	{
		DATA			= 1,
		PARITY_ERR		= 2,
		START_BIT_ERR	= 4,
		STOP_BIT_ERR	= 8,
		NOTHING			= 16,
		START_BIT		= 32,
		STOP_BIT		= 64,
		PARITY_BIT		= 128,
		ENUM_END		= 256
	};

	class UARTParameters : public DecoderParameters
	{
		public:

			uint32_t 		m_rx; 		// 1..8
			uint32_t 		m_tx; 		// 1..8
			uint32_t 		m_baudrate;
			uint32_t 		m_invert;
			UartBitOrder 	m_bitOrder;
			NumDataBits 	m_num_data_bits; // 5..9
			Parity 			m_parity;
			NumStopBits 	m_num_stop_bits;
			uint32_t 		m_samplerate;

		UARTParameters();

		auto toJson() -> std::string;
    	auto fromJson(const std::string &json) -> bool;

		static std::string getUARTAnnotationsString(UARTAnnotations value);
	};
}