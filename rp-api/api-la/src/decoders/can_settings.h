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
		CRC_SEQ			= 11, // 'CRC sequence'
		CRC_DELIMITER   = 12, // 'CRC delimiter'
		ACK_SLOT		= 13, // 'ACK slot'
		ACK_DELIMITER	= 14, // 'ACK delimiter'
		STUFF_BIT	 	= 15, // 'Stuff bit'
		WARNING			= 16, // 'Warning unknow'
		BIT				= 17, // 'Bit'
		ERROR_1 		= 18, // 'Start of frame (SOF) must be a dominant bit'
		ERROR_2			= 19, // 'Data length code (DLC) > 8 is not allowed'
		ERROR_3			= 20, // 'End of frame (EOF) must be 7 recessive bits'
		WARNING_1       = 21, // 'Identifier bits 10..4 must not be all recessive'
		WARNING_2		= 22, // 'CRC delimiter must be a recessive bit'
		WARNING_3		= 23, // 'ACK delimiter must be a recessive bit'
		BRS				= 24, // 'Bit rate switch'
		ESI 			= 25, // 'Error state indicator'
		CRC_LEN         = 26, // 'Crc type
		RESERV_BIT_FLEX	= 27, // 'Flexible data'
		NOTHING			= 28,
		SYNC			= 29,
		ENUM_END
	};

	class CANParameters : public DecoderParameters
	{
		public:
			uint32_t m_can_rx; 				// 1...8
			uint32_t m_nominal_bitrate;  	// default value  1000000 (bits/s)
			uint32_t m_fast_bitrate; 		// default value  2000000 (bits/s)
			float    m_sample_point; 		// default value  70.0 (%)
			uint32_t m_frame_limit;
			uint32_t m_acq_speed;
			uint32_t m_invert_bit;

		CANParameters();

		auto toJson() -> std::string;
    	auto fromJson(const std::string &json) -> bool;

		static std::string getCANAnnotationsString(CANAnnotations value);
	};
}



