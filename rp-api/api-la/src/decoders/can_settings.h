#pragma once

#include "decoder.h"
#include <string_view>

namespace can {

	// this enum for Control byte in output structure

	enum FrameFormat
	{
		None,
		Standart,
		Extended
	};

	enum CANAnnotations
	{
		PAYLOAD_DATA 	= 0,  // 'Payload data'
		START_OF_FRAME 	= 1,  // 'Start of frame'
		END_OF_FRAME 	= 2,  // 'End of frame'
		ID 				= 3,  // 'Identifier'
		EXT_ID 			= 4,  // 'Extended identifier'
		FULL_ID 		= 5,  // 'Full identifier'
		IDE 			= 6,  // 'Identifier extension bit'
		RESERV_BIT		= 7,  // 'Reserved bit 0 and 1'
		RTR 			= 8,  // 'Remote transmission request'
		SRR				= 9,  // 'Substitute remote request'
		DLC             = 10, // 'Data length count'
		CRC_DELIMITER   = 11, // 'CRC delimiter'
		ACK_SLOT		= 12, // 'ACK slot'
		ACK_DELIMITER	= 13, // 'ACK delimiter'
		STUFF_BIT	 	= 14, // 'Stuff bit'
		ERROR_3			= 15, // 'End of frame (EOF) must be 7 recessive bits'
		WARNING_1       = 16, // 'Identifier bits 10..4 must not be all recessive'
		WARNING_2		= 17, // 'CRC delimiter must be a recessive bit'
		WARNING_3		= 18, // 'ACK delimiter must be a recessive bit'
		BRS				= 19, // 'Bit rate switch'
		ESI 			= 20, // 'Error state indicator'
		RESERV_BIT_FLEX	= 21, // 'Flexible data'
		STUFF_BIT_ERROR	= 22, // 'Stuff bit error'
		CRC_VAL			= 23, // CRC Value
		FSB				= 24, // fixed stuff-bit (FSB)
		SBC				= 25, // Stuff bits before CRC in FD mode
		ENUM_END
	};

	class CANParameters : public DecoderParameters
	{
		public:
			uint32_t m_can_rx; 				// 1...8
			uint32_t m_nominal_bitrate;  	// default value  1000000 (bits/s)
			uint32_t m_fast_bitrate; 		// default value  2000000 (bits/s)
			float    m_sample_point; 		// default value  70.0 (%)
			uint32_t m_acq_speed;
			uint32_t m_invert_bit;

		CANParameters();

		auto toJson() -> std::string override;
    	auto fromJson(const std::string &json) -> bool override;

		static std::string getCANAnnotationsString(CANAnnotations value);

		auto setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool override;
		auto setDecoderSettingsFloat(std::string& key, float value) -> bool override;

		auto getDecoderSettingsUInt(std::string& key, uint32_t *value) -> bool override;
		auto getDecoderSettingsFloat(std::string& key, float *value) -> bool override;

	};
}



