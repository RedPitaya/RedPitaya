#pragma once

#include <vector>
#include <deque>
#include <string>
#include <cstdint>
#include <cstdlib>

#include "decoder.h"


namespace spi {

	enum CsPolartiy
	{
		ActiveLow,
		ActiveHigh
	};

	enum BitOrder
	{
		MsbFirst,
		LsbFirst
	};

	// // For control byte
	// #define SPI_DATA_ERROR 0x80

	enum SPIAnnotations
	{
		DATA = 0,
		NOTHING = 10
	};

	class SPIParameters : public DecoderParameters
	{
		public:
			uint32_t m_clk; 	// 1...8 	ch Number
			uint32_t m_data; 	// 1...8
			uint32_t m_cs;  	// 0...8, 	0 if is not set
			uint32_t m_cpol = 0;
			uint32_t m_cpha = 0;
			uint32_t m_word_size = 8;
			uint32_t m_acq_speed;
			CsPolartiy m_cs_polarity = ActiveLow;
			BitOrder m_bit_order = MsbFirst;
			uint32_t m_invert_bit;

		SPIParameters();

		auto toJson() -> std::string;
    	auto fromJson(const std::string &json) -> bool;

		static constexpr std::string_view getSPIAnnotationsString(SPIAnnotations value);
	};
}

