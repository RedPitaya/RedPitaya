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
		NO,
		RX_DATA,
		RX_START,
		RX_PARITY_OK,
		RX_PARITY_ERR,
		RX_STOP,
		RX_WARNING,
		RX_DATA_BITS
	};

	class UARTParameters : public DecoderParameters
	{
		public:

			uint32_t 		m_rx; 		// 1..8
			uint32_t 		m_baudrate;
			uint32_t 		m_invert_rx;
			UartBitOrder 	m_bitOrder;
			NumDataBits 	m_num_data_bits; // 5..9
			Parity 			m_parity;
			NumStopBits 	m_num_stop_bits;
			uint32_t 		m_samplerate;

		UARTParameters();

		auto toJson() -> std::string;
    	auto fromJson(const std::string &json) -> bool;

		static constexpr std::string_view getUARTAnnotationsString(UARTAnnotations value);
	};
}