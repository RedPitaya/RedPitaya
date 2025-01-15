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
		ActiveLow	= 0,
		ActiveHigh	= 1
	};

	enum BitOrder
	{
		MsbFirst	= 0,
		LsbFirst	= 1
	};

	enum SPIAnnotations
	{
		DATA 	= 0,
		ENUM_END
	};

	class SPIParameters : public DecoderParameters
	{
		public:
			uint32_t m_clk; 	// 1...8 	ch Number
			uint32_t m_miso; 	// 1...8
			uint32_t m_mosi; 	// 1...8
			uint32_t m_cs;  	// 0...8, 	0 if is not set
			uint32_t m_cpol = 0;
			uint32_t m_cpha = 0;
			uint32_t m_word_size = 8;
			uint32_t m_acq_speed;
			CsPolartiy m_cs_polarity = ActiveLow;
			BitOrder m_bit_order = MsbFirst;
			uint32_t m_invert_bit;

		SPIParameters();

		auto toJson() -> std::string override;
    	auto fromJson(const std::string &json) -> bool override;

		static std::string getSPIAnnotationsString(SPIAnnotations value);

		auto setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool override;
		auto getDecoderSettingsUInt(std::string& key, uint32_t *value) -> bool override;
	};
}

