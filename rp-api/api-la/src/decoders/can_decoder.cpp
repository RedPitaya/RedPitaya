#include "can_decoder.h"

#include <initializer_list>
#include <string>
#include <math.h>
#include <cstdio>
#include <limits>
#include <list>
#include <algorithm>
#include <cassert>

#include "rp.h"
#include "common/profiler.h"

using namespace can;


class CANDecoder::Impl{
	enum States
	{
		S_START_OF_FRAME,
		S_ID_1,
		S_ID_2,
		S_RR_12_BIT,			// Remote request
		S_RR_32_BIT,			// Remote request Bit 32 for Ext frame
		S_IDE_BIT,				// Identifier extension bit
		S_R0,					// Reserved bit. Must be dominant (0), but accepted as either dominant or recessive.
		S_R1,
		S_R1_EXT,
		S_DLC,
		S_BRS,
		S_ESI,
		S_DATA,
		S_CRC,
		S_CRC_15,
		S_CRC_17,
		S_CRC_21,
		S_CRC_DEL,
		S_STUFF_BITS,
		S_ACK,
		S_ACK_DEL,
		S_END
	};


	public:

	Impl();


	CANParameters m_options;

	bool m_oldCanRX; // Old state of RX line
	int32_t  m_samplenum; // Len of sample
	States   m_state;
	uint32_t m_samplerate;
	int32_t  m_samplePoint;
	uint16_t m_frame_limit;
	float    m_bitwidth;


	OutputPacket m_ssBit12; // RTR or SRR bit
	bool     m_isFlexibleData; // Bit 14: FDF (Flexible data format)

	uint32_t m_id;
	uint32_t m_crc;
	uint32_t m_silenceLength;
	uint32_t m_prevBitStart;
	uint32_t m_bitAccumulate;
	uint32_t m_startDataBits;
	uint32_t m_startBitSamplePoint;
	int64_t  m_startBlockSamplePoint;
	uint32_t m_bitBegin;
	uint32_t m_bitEnd;
	uint32_t m_bitSamplePointPos;
	uint32_t m_bitsArray;
	uint32_t m_bitsArrayCount;
	uint32_t m_bitsArrayCountSaved;
	uint32_t m_bitsArrayWithStuff;
	uint32_t m_bitsArrayCountWithStuff;
	uint32_t m_curByte;
	uint32_t m_fsbCounter;
	uint32_t m_stuffBitCounter;
	uint8_t	 m_dlc;
	uint32_t m_fullFDCrc;
	uint32_t m_fullFDCrcBits;
	uint32_t m_fullFDCrcStart;

	bool	 m_bitDetect;
	bool	 m_samplePointValue;
	bool	 m_bitValue;

	FrameFormat m_frameType;


	std::vector<OutputPacket> m_result;


	auto resetDecoder() -> void;
	auto resetState() -> void;
	auto decode(const uint8_t* _input, uint32_t _size) -> void;
	auto setBitRate(uint32_t _bitrate) -> void;
	auto setNominalBitrate() -> void;
	auto setFastBitrate() -> void;
	auto getMemoryUsage() -> uint64_t;
	auto handleBit(const bool _can_rx) -> bool;
	auto decodeStandardFrame(const bool bit) -> bool;
	auto decodeExtendedFrame(const bool bit) -> bool;
	auto decodeEndFrame(const bool bit) -> bool;
	auto dlc2Len(uint8_t _dlc) -> uint8_t;
	auto getBit(bool bit) -> bool;
	inline auto isStuffBit(bool bit) -> std::tuple<bool,bool>;

};

CANDecoder::Impl::Impl(){

}

CANDecoder::CANDecoder(int decoderType, const std::string& _name){
	m_decoderType = decoderType;
	m_name = _name;
	m_impl = new Impl();
	m_impl->resetDecoder();
	setParameters(can::CANParameters());
}

CANDecoder::~CANDecoder(){
	delete m_impl;
}

auto CANDecoder::getMemoryUsage() -> uint64_t{
	return m_impl->getMemoryUsage();
}

auto CANDecoder::Impl::getMemoryUsage() -> uint64_t{
	uint64_t size = sizeof(Impl);
	size += m_result.size() * sizeof(OutputPacket);
	return size;
}

auto CANDecoder::setParameters(const CANParameters& _new_params) -> void{
	m_impl->m_options = _new_params;
	m_impl->m_samplerate = _new_params.m_acq_speed;
	m_impl->m_bitwidth = (float)m_impl->m_samplerate / (float)(_new_params.m_nominal_bitrate);
	m_impl->m_samplePoint = (m_impl->m_bitwidth  / 100.0) * _new_params.m_sample_point;
	TRACE_SHORT("Bitwidth %f, SamplePoint %d Samplerate %d\n",
		m_impl->m_bitwidth ,
		m_impl->m_samplePoint,
		m_impl->m_samplerate)
}

auto CANDecoder::getSignal() -> std::vector<OutputPacket>{
	return m_impl->m_result;
}


auto CANDecoder::Impl::resetDecoder() -> void {
	m_oldCanRX = true;
	m_samplenum = 0;
	m_result.clear();
	m_silenceLength = 0;
	m_prevBitStart = 0;
	m_bitAccumulate = 0;
	m_startDataBits = 0;
	m_startBitSamplePoint = 0;
	m_bitDetect = false;
	m_samplePointValue = false;
	m_bitValue = false;

	m_bitBegin = 0;
	m_bitEnd = 0;
	m_bitSamplePointPos = 0;

	resetState();
}

auto CANDecoder::Impl::resetState() -> void{
	m_isFlexibleData = false;
	m_frameType = None;
	m_crc = 0;
	m_id = 0;
	m_bitsArray = 0;
	m_bitsArrayCount = 0;
	m_bitsArrayCountSaved = 0;
	m_bitsArrayWithStuff = 0;
	m_bitsArrayCountWithStuff = 0;
	m_state = S_START_OF_FRAME;
	m_startBlockSamplePoint = -1;
	m_dlc = 0;
	m_curByte = 0;
	m_stuffBitCounter = 0;
	m_fullFDCrc = 0;
	m_fullFDCrcStart = 0;
	m_fullFDCrcBits = 0;
	setNominalBitrate();
}

auto CANDecoder::reset() -> void{
	m_impl->resetDecoder();
}

auto CANDecoder::getParametersInJSON() -> std::string{
	return m_impl->m_options.toJson();
}

auto CANDecoder::setParametersInJSON(const std::string &parameter) -> void{
	CANParameters param;
	if (param.fromJson(parameter)){
		setParameters(param);
	}else{
		ERROR_LOG("Error set parameters %s", parameter.c_str())
	}
}

auto CANDecoder::setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool{
	auto opt = m_impl->m_options;
	if (opt.setDecoderSettingsUInt(key,value)){
		setParameters(opt);
		return true;
	}
	return false;
}

auto CANDecoder::setDecoderSettingsFloat(std::string& key, float value) -> bool{
	auto opt = m_impl->m_options;
	if (opt.setDecoderSettingsFloat(key,value)){
		setParameters(opt);
		return true;
	}
	return false;
}

auto CANDecoder::getDecoderSettingsUInt(std::string& key, uint32_t *value) -> bool{
	return m_impl->m_options.getDecoderSettingsUInt(key,value);
}

auto CANDecoder::getDecoderSettingsFloat(std::string& key, float *value) -> bool{
	return m_impl->m_options.getDecoderSettingsFloat(key,value);
}

auto CANDecoder::decode(const uint8_t* _input, uint32_t _size) -> void{
	m_impl->decode(_input, _size);
}

auto CANDecoder::Impl::getBit(bool bit) -> bool{
	if (bit != m_oldCanRX && m_bitDetect == false){
		m_bitDetect = true;
		m_startBitSamplePoint = m_samplenum;
		m_bitAccumulate += bit;
	}
	else{
		if (m_bitDetect){
			m_bitAccumulate += bit;
			// Skip bit state change to SAMPLE POINT position
			if (m_oldCanRX == bit || ((uint32_t)m_samplenum <= (uint32_t)m_samplePoint + m_startBitSamplePoint)){
				if ((uint32_t)m_samplenum <= (uint32_t)m_samplePoint + m_startBitSamplePoint){
					m_samplePointValue = bit;
					m_bitSamplePointPos = m_samplenum;
				}
				if ((m_samplenum < m_bitwidth + m_startBitSamplePoint)){
					return false;
				}
			}else{
				m_bitAccumulate -= bit;
			}
			m_bitValue = round((float)m_bitAccumulate / (float)(m_samplenum - m_startBitSamplePoint));
			m_bitBegin = m_startBitSamplePoint;
			m_bitEnd = m_samplenum;
			m_startBitSamplePoint = m_samplenum;
			m_bitAccumulate = 0;
			return true;
		}
	}
	return false;
}

auto CANDecoder::Impl::decode(const uint8_t* _input, uint32_t _size) -> void{
	if (!_input) FATAL("Input value is null")
	if (_size == 0) FATAL("Input value size == 0")
	if (_size & 0x1) FATAL("Input value is odd")
	if (m_options.m_can_rx == 0 || m_options.m_can_rx > 8) {
		ERROR_LOG("RX not specified. Valid values from 1 to 8")
		return;
	}

	TRACE_SHORT("Input data size %d",_size)

	resetDecoder();

	// profiler::clearHistory("1");

	uint8_t line_rx = m_options.m_can_rx - 1;
	for (size_t i = 0; i < _size; i += 2)
	{
		const uint8_t count = _input[i]; // 0 = 1 count, 1 = 2, etc.
		const uint8_t data  = _input[i + 1];
		for (uint16_t j = 0; j < count + 1; ++j)
		{

			// Get bit state of line in LA
			bool can_rx = data & (1 << line_rx);
			if (m_options.m_invert_bit){
				can_rx = !can_rx;
			}

			if (getBit(can_rx)){
				handleBit(m_samplePointValue);
			}

			m_oldCanRX = can_rx;
			assert(m_samplenum < std::numeric_limits<decltype(m_samplenum)>::max() && "m_Samplenum overflow");
			m_samplenum++;
		}
	}
	TRACE_SHORT("Frames found: %d\n",m_result.size());
}

auto CANDecoder::Impl::setBitRate(uint32_t _bitrate) -> void{
    m_bitwidth = float(m_samplerate) / float(_bitrate);
    m_samplePoint = (m_bitwidth / 100.0) * m_options.m_sample_point;
}

auto CANDecoder::Impl::setNominalBitrate() -> void{
	setBitRate(m_options.m_nominal_bitrate);
}

auto CANDecoder::Impl::setFastBitrate() -> void{
	setBitRate(m_options.m_fast_bitrate);
}


auto CANDecoder::Impl::handleBit(const bool bit) -> bool{

	// Check stuff bit
	auto [isStuff,isStuffError] = isStuffBit(bit);

	m_bitsArrayWithStuff = m_bitsArrayWithStuff << 1;
	m_bitsArrayWithStuff |= bit;
	m_bitsArrayCountWithStuff++;

	if (isStuff && m_state != S_END){
		if (isStuffError){
			m_result.push_back(OutputPacket{"rx", STUFF_BIT_ERROR, 0, 1 , (double)m_bitBegin,(double)(m_bitEnd - m_bitBegin)});
			resetState();
			m_bitDetect = false;
			return false;
		}
		else{
			m_result.push_back(OutputPacket{"rx", STUFF_BIT, 0, 1 , (double)m_bitBegin,(double)(m_bitEnd - m_bitBegin)});
		}
		m_stuffBitCounter++;
		return false;
	}

	m_bitsArray = m_bitsArray << 1;
	m_bitsArray |= bit;
	m_bitsArrayCount++;

	// Bit 0: Start of frame (SOF) bit
	if (m_state == S_START_OF_FRAME && bit == 0){
		m_result.push_back(OutputPacket{"rx", START_OF_FRAME, 0, 1 , (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
		m_state = S_ID_1;
		return true;
	}

	// Bits 1-11: Identifier (ID[10..0])
	// The bits ID[10..4] must NOT be all recessive.
	if (m_state == S_ID_1){
		if (m_startBlockSamplePoint == -1){
			m_startBlockSamplePoint = m_bitBegin;
			m_bitsArrayCountSaved = m_bitsArrayCount - 1;
			m_stuffBitCounter = 0;
		}

		if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 11){
			auto id_1 = m_bitsArray & 0x7FF;
			m_id = id_1;
			m_result.push_back(OutputPacket{"rx", ID, id_1,
												(float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter,
												(double)m_startBlockSamplePoint,
												(double)(m_bitEnd - (uint32_t)m_startBlockSamplePoint)});
			if ((id_1 & 0x7F0) == 0x7F0){
				m_result.push_back(OutputPacket{"rx", WARNING_1, id_1,
												(float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter,
												(double)m_startBlockSamplePoint,
												(double)(m_bitEnd - (uint32_t)m_startBlockSamplePoint)});
			}
			m_state = S_RR_12_BIT;
			m_startBlockSamplePoint = -1;
		}
		return true;
	}

	if (m_state == S_RR_12_BIT){
		m_ssBit12 = OutputPacket{"rx", RTR, bit, 1 , (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)};
		m_state = S_IDE_BIT;
		return true;
	}

	// Bit 13: Identifier extension (IDE) bit
    // Standard frame: dominant, extended frame: recessive
	if (m_state == S_IDE_BIT){
		m_frameType = bit ? Extended: Standart;
		m_result.push_back(OutputPacket{"rx", IDE, bit, 1 , (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
		m_state = bit ? S_ID_2 : S_R0;
		return true;
	}

	if (m_frameType == Standart){
		return decodeStandardFrame(bit);
	}

	if (m_frameType == Extended){
		return decodeExtendedFrame(bit);
	}

	return false;
}

auto CANDecoder::Impl::decodeStandardFrame(const bool bit) -> bool{
	// Bit 14: Reserve bit r0
	// FDF (Flexible data format)
    // Has to be sent dominant when FD frame, has to be sent recessive when classic CAN frame.

	if (m_state == S_R0){
		m_isFlexibleData = bit;
		if (bit){
			m_result.push_back(OutputPacket{"rx", RESERV_BIT_FLEX, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
			auto b12 = m_ssBit12;
			b12.control = SRR;
			m_result.push_back(b12);
			m_state = S_R1;
		}else{
			m_result.push_back(m_ssBit12);
			m_state = S_DLC;
		}
		m_result.push_back(OutputPacket{"rx", RESERV_BIT, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
		return true;
	}

	if (m_state == S_R1){
		m_result.push_back(OutputPacket{"rx", RESERV_BIT, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
		m_state = S_BRS;
		return true;
	}

	if (m_state == S_BRS){
		m_result.push_back(OutputPacket{"rx", BRS, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
		if (bit)
			setFastBitrate();
		m_state = S_ESI;
		return true;
	}

	if (m_state == S_ESI){
		m_result.push_back(OutputPacket{"rx", ESI, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
		m_state = S_DLC;
		return true;
	}

	if (m_state == S_DLC){
		if (m_startBlockSamplePoint == -1){
			m_startBlockSamplePoint = m_bitBegin;
			m_bitsArrayCountSaved = m_bitsArrayCount - 1;
			m_stuffBitCounter = 0;
		}

		if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 4){
			m_dlc = m_bitsArray & 0xF;
			// The values 0-8 indicate 0-8 bytes like classic CAN. The values 9-15 are translated to a value
			// between 12-64 which is the actual length of the data field:
			// 9→12   10→16   11→20   12→24   13→32   14→48   15→64
			if (m_isFlexibleData){
				m_dlc = dlc2Len(m_dlc);
			}

			m_result.push_back(OutputPacket{"rx", DLC, m_dlc,
												(float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter,
												(double)m_startBlockSamplePoint,
												(double)(m_bitEnd - (uint32_t)m_startBlockSamplePoint)});
			m_state = m_dlc != 0 ? S_DATA : S_CRC;
			m_curByte = 0;
			m_startBlockSamplePoint = -1;
		}
		return true;
	}

	if (m_state == S_DATA){
		if (m_startBlockSamplePoint == -1){
			m_startBlockSamplePoint = m_bitBegin;
			m_bitsArrayCountSaved = m_bitsArrayCount - 1;
			m_stuffBitCounter = 0;
		}

		if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 8){
			auto data = m_bitsArray & 0xFF;

			m_result.push_back(OutputPacket{"rx", PAYLOAD_DATA, data,
												(float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter,
												(double)m_startBlockSamplePoint,
												(double)(m_bitEnd - (uint32_t)m_startBlockSamplePoint)});
			m_curByte++;
			if (m_curByte == m_dlc){
				m_state = S_CRC;
				m_fullFDCrc = 0;
				m_fullFDCrcBits = 0;
			}
			m_startBlockSamplePoint = -1;
		}
		return true;
	}

	return decodeEndFrame(bit);
}


auto CANDecoder::Impl::decodeExtendedFrame(const bool bit) -> bool{


	// Bits 14-31: Extended identifier (EID[17..0])
	if (m_state == S_ID_2){
		if (m_startBlockSamplePoint == -1){
			m_startBlockSamplePoint = m_bitBegin;
			m_bitsArrayCountSaved = m_bitsArrayCount - 1;
			m_stuffBitCounter = 0;
		}

		if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 18){
			auto id_2 = m_bitsArray & 0x3FFFF;
			m_result.push_back(OutputPacket{"rx", EXT_ID, id_2,
												(float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter,
												(double)m_startBlockSamplePoint,
												(double)(m_bitEnd - (uint32_t)m_startBlockSamplePoint)});
			auto f_id = m_id << 18 | id_2;
			m_result.push_back(OutputPacket{"rx", FULL_ID, f_id,
												(float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter,
												(double)m_startBlockSamplePoint,
												(double)(m_bitEnd - (uint32_t)m_startBlockSamplePoint)});
			m_state = S_RR_32_BIT;
			m_startBlockSamplePoint = -1;
		}
		return true;
	}

	if (m_state == S_RR_32_BIT){
		m_result.push_back(OutputPacket{"rx", RTR, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
		m_state = S_R0;
		return true;
	}

	// Bit 33: RB1 (reserved bit)
	if (m_state == S_R0){
		m_isFlexibleData = bit;
		if (bit){
			m_result.push_back(OutputPacket{"rx", RESERV_BIT_FLEX, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
			m_state = S_R1;
		}else{
			m_state = S_R1_EXT;
		}
		auto b12 = m_ssBit12;
		b12.control = SRR;
		m_result.push_back(b12);
		m_result.push_back(OutputPacket{"rx", RESERV_BIT, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
		return true;
	}

	if (m_state == S_R1_EXT){
		m_result.push_back(OutputPacket{"rx", RESERV_BIT, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
		m_state = S_DLC;
		return true;
	}

	if (m_state == S_R1){
		m_result.push_back(OutputPacket{"rx", RESERV_BIT, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
		m_state = S_BRS;
		return true;
	}

	if (m_state == S_BRS){
		m_result.push_back(OutputPacket{"rx", BRS, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
		if (bit)
			setFastBitrate();
		m_state = S_ESI;
		return true;
	}

	if (m_state == S_ESI){
		m_result.push_back(OutputPacket{"rx", ESI, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
		m_state = S_DLC;
		return true;
	}

	if (m_state == S_DLC){
		if (m_startBlockSamplePoint == -1){
			m_startBlockSamplePoint = m_bitBegin;
			m_bitsArrayCountSaved = m_bitsArrayCount - 1;
			m_stuffBitCounter = 0;
		}

		if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 4){
			m_dlc = m_bitsArray & 0xF;
			// The values 0-8 indicate 0-8 bytes like classic CAN. The values 9-15 are translated to a value
			// between 12-64 which is the actual length of the data field:
			// 9→12   10→16   11→20   12→24   13→32   14→48   15→64
			if (m_isFlexibleData){
				m_dlc = dlc2Len(m_dlc);
			}

			m_result.push_back(OutputPacket{"rx", DLC, m_dlc,
												(float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter,
												(double)m_startBlockSamplePoint,
												(double)(m_bitEnd - (uint32_t)m_startBlockSamplePoint)});
			m_state = m_dlc != 0 ? S_DATA : S_CRC;
			m_curByte = 0;
			m_startBlockSamplePoint = -1;
		}
		return true;
	}

	if (m_state == S_DATA){
		if (m_startBlockSamplePoint == -1){
			m_startBlockSamplePoint = m_bitBegin;
			m_bitsArrayCountSaved = m_bitsArrayCount - 1;
			m_stuffBitCounter = 0;
		}

		if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 8){
			auto data = m_bitsArray & 0xFF;

			m_result.push_back(OutputPacket{"rx", PAYLOAD_DATA, data,
												(float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter,
												(double)m_startBlockSamplePoint,
												(double)(m_bitEnd - (uint32_t)m_startBlockSamplePoint)});
			m_curByte++;
			if (m_curByte == m_dlc){
				m_state = S_CRC;
				m_fullFDCrc = 0;
				m_fullFDCrcBits = 0;
			}
			m_startBlockSamplePoint = -1;
		}
		return true;
	}

	return decodeEndFrame(bit);
}

auto CANDecoder::Impl::decodeEndFrame(const bool bit) -> bool{
 	// Remember start of CRC sequence (see below).

	if (m_state == S_CRC){
		if (m_isFlexibleData) {
			m_result.push_back(OutputPacket{"rx", FSB, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
			m_state = S_STUFF_BITS;
			m_fullFDCrc = m_fullFDCrc << 1 | bit;
			m_fullFDCrcStart = m_bitBegin;
			m_fullFDCrcBits++;
			return true;
		}else{
			m_state = S_CRC_15;
		}
	}


	if (m_state == S_CRC_15){
		if (m_startBlockSamplePoint == -1){
			m_startBlockSamplePoint = m_bitBegin;
			m_bitsArrayCountSaved = m_bitsArrayCount - 1;
			m_stuffBitCounter = 0;
		}

		if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 15){
			m_crc = m_bitsArray & 0x7FFF;
			m_result.push_back(OutputPacket{"rx", CRC_15_VAL, m_crc,
												(float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter,
												(double)m_startBlockSamplePoint,
												(double)(m_bitEnd - (uint32_t)m_startBlockSamplePoint)});
			m_state = S_CRC_DEL;
			m_startBlockSamplePoint = -1;
		}
		return true;
	}

	if (m_state == S_STUFF_BITS){
		if (m_startBlockSamplePoint == -1){
			m_startBlockSamplePoint = m_bitBegin;
			m_bitsArrayCountSaved = m_bitsArrayCount - 1;
			m_stuffBitCounter = 0;
		}
		m_fullFDCrc = m_fullFDCrc << 1 | bit;
		m_fullFDCrcBits++;
		if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 4){
			auto sb = m_bitsArray & 0xF;
			m_result.push_back(OutputPacket{"rx", SBC, sb,
												(float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter,
												(double)m_startBlockSamplePoint,
												(double)(m_bitEnd - (uint32_t)m_startBlockSamplePoint)});
			m_state = m_dlc < 16 ? S_CRC_17 : S_CRC_21;
			m_startBlockSamplePoint = -1;
		}
		return true;
	}

	if (m_state == S_CRC_17 || m_state == S_CRC_21){
		if (m_startBlockSamplePoint == -1){
			m_startBlockSamplePoint = m_bitBegin;
			m_bitsArrayCountSaved = m_bitsArrayCount - 1;
			m_crc = 0;
			m_fsbCounter = 0;
			m_stuffBitCounter = 0;
		}
		m_fullFDCrc = m_fullFDCrc << 1 | bit;
		m_fullFDCrcBits++;
		if ((m_bitsArrayCount - m_bitsArrayCountSaved) == 1 + (m_fsbCounter * 5)){
			m_result.push_back(OutputPacket{"rx", FSB, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
			m_fsbCounter++;
		}else{
			m_crc = m_crc << 1;
			m_crc |= bit;
		}
		uint32_t l = m_state == S_CRC_17 ? 22 : 27;
		if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= l){
			m_result.push_back(OutputPacket{"rx", (m_state == S_CRC_17 ? CRC_17_VAL : CRC_21_VAL), m_crc,
												(float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter,
												(double)m_startBlockSamplePoint,
												(double)(m_bitEnd - (uint32_t)m_startBlockSamplePoint)});
			m_result.push_back(OutputPacket{"rx", CRC_FSB_SBC, m_fullFDCrc,
												(float)(m_fullFDCrcBits),
												(double)m_fullFDCrcStart,
												(double)(m_bitEnd - (uint32_t)m_startBlockSamplePoint)});
			m_state = S_CRC_DEL;
			m_startBlockSamplePoint = -1;
		}
		return true;
	}

	// CRC delimiter bit (recessive)
	if (m_state == S_CRC_DEL){
		m_result.push_back(OutputPacket{"rx", CRC_DELIMITER, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
		if (!bit){
			m_result.push_back(OutputPacket{"rx", WARNING_2, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
		}
		if (m_isFlexibleData) {
	 		setNominalBitrate();
	 	}
		m_state = S_ACK;
		return true;
	}

	// ACK slot bit (dominant: ACK, recessive: NACK)
	if (m_state == S_ACK){
		m_result.push_back(OutputPacket{"rx", ACK_SLOT, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
		m_state = S_ACK_DEL;
		return true;
	}

	// ACK delimiter bit (recessive)
	if (m_state == S_ACK_DEL){
		m_result.push_back(OutputPacket{"rx", ACK_DELIMITER, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
		if (!bit){
			m_result.push_back(OutputPacket{"rx", WARNING_3, bit, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
		}
		m_state = S_END;
		return true;
	}

	if (m_state == S_END){
		if (m_startBlockSamplePoint == -1){
			m_startBlockSamplePoint = m_bitBegin;
			m_bitsArrayCountSaved = m_bitsArrayCount - 1;
		}
		if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 7){
			auto end = m_bitsArray & 0x7F;
			m_result.push_back(OutputPacket{"rx", END_OF_FRAME, end,
												(float)(m_bitsArrayCount - m_bitsArrayCountSaved),
												(double)m_startBlockSamplePoint,
												(double)(m_bitEnd - (uint32_t)m_startBlockSamplePoint)});
			if ((end & 0x7F) != 0x7F){
				m_result.push_back(OutputPacket{"rx", ERROR_3, end, 1, (double)m_bitBegin, (double)(m_bitEnd - m_bitBegin)});
			}
			resetState();
			m_bitDetect = false;
			m_startBlockSamplePoint = -1;
		}
		return true;
	}
	return false;
}

auto CANDecoder::Impl::isStuffBit(bool bit) -> std::tuple<bool,bool>{
	if (m_bitsArrayCountWithStuff >= 5){
		if ((m_bitsArrayWithStuff & 0x1F) == 0){
			if (bit)
				return std::make_tuple(true,false);
			else
				return std::make_tuple(true,true);
		}

		if ((m_bitsArrayWithStuff & 0x1F) == 0x1F){
			if (!bit)
				return std::make_tuple(true,false);
			else
				return std::make_tuple(true,true);
		}
	}
	return std::make_tuple(false,false);
}

auto CANDecoder::Impl::dlc2Len(uint8_t _dlc) -> uint8_t{
	uint8_t code_page[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
	return code_page[_dlc];
}