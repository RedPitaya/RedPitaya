#include "can_decoder.h"

#include <initializer_list>
#include <string>
#include <cstdio>
#include <limits>
#include <algorithm>
#include <cassert>

#include "rp.h"

using namespace can;


class CANDecoder::Impl{
	enum States
	{
		IDLE,
		CORRECT,
		GET_INFO
	};

	public:

	Impl();

	std::string m_name;

	CANParameters m_options;

	bool m_oldCanRX; // Old state of RX line
	uint32_t m_dom_edge_snum;
	uint32_t m_dom_edge_bcount;
	int32_t  m_samplenum; // Len of sample
	int32_t  m_sampleSkip;
	States   m_state;
	bool	 m_syncMode;
	uint32_t m_samplerate;
	int32_t  m_samplePoint;
	uint16_t m_frame_limit;
	float    m_bitwidth;


	uint32_t m_ssBlock; // start smaple of ID;
	uint32_t m_ssBit12; // RTR or SRR bit
	uint32_t m_id;
	int32_t  m_curBit; // Current bit of CAN frame (bit 0 == SOF)
	uint32_t m_dataLenghtCode; // dlc
	uint32_t m_dlcStart;
	uint32_t m_lastDatabit;
	uint32_t m_rtr;
	uint32_t m_crc;
	uint32_t m_crcLen;
	bool     m_isFlexibleData; // Bit 14: FDF (Flexible data format)


	std::vector<OutputPacket> m_tempResult;
	std::vector<OutputPacket> m_result;
	std::vector<bool> m_syncBits;

	//improves the decoder's reliability when the input signal's bitrate does not exactly match the nominal rate.

	std::vector<bool> m_bits; // Only actual CAN frame bits (no stuff bits)
	std::vector<bool> m_rawBits; // All bits, including stuff bits
	std::vector<uint32_t> m_dataBits;


	void resetDecoder();
	void resetState();

	void decode(const uint8_t* _input, uint32_t _size);

	inline bool isLowState(bool _can_rx) const;
	inline bool isFallEdgeState(bool _can_rx) const;
	inline bool isStuffBit();

	void 	 domEdgeSeen();
	uint32_t getSamplePoint(uint32_t _curbit);
	void     setBitRate(uint32_t _bitrate);
	void 	 setNominalBitrate();
	void 	 setFastBitrate();

	bool 	 handleBit(const bool _can_rx);
	bool	 decodeStandardFrame(const bool _can_rx,uint32_t bitnum);
	bool	 decodeExtendedFrame(const bool _can_rx,uint32_t bitnum);
	bool     decodeEndFrame(const bool _can_rx,uint32_t bitnum);

	uint8_t  dlc2Len(uint8_t _dlc);
	uint32_t convert(const std::vector<bool> &_bits);
	std::vector<OutputPacket> packResult(std::vector<OutputPacket> &_data);


	FrameFormat m_frameType;
};

CANDecoder::Impl::Impl(){

}

CANDecoder::CANDecoder(const std::string& _name){
	m_impl = new Impl();
	m_impl->m_name = _name;
	m_impl->resetDecoder();
}

CANDecoder::~CANDecoder(){
	delete m_impl;
}

void CANDecoder::setParameters(const CANParameters& _new_params)
{
	m_impl->m_options = _new_params;
	m_impl->m_samplerate = _new_params.m_acq_speed;
	m_impl->m_bitwidth = (float)m_impl->m_samplerate / (float)(_new_params.m_nominal_bitrate);
	m_impl->m_samplePoint = (m_impl->m_bitwidth  / 100.0) * _new_params.m_sample_point;
	TRACE_SHORT("Bitwidth %f, SamplePoint %d Samplerate %d\n",
		m_impl->m_bitwidth ,
		m_impl->m_samplePoint,
		m_impl->m_samplerate)
}

std::vector<OutputPacket> CANDecoder::getSignal(){
	return m_impl->m_result;
}


void CANDecoder::Impl::resetDecoder()
{
	m_oldCanRX = 1;
	m_dom_edge_snum = 0;
	m_dom_edge_bcount = 0;
	m_samplenum = 0;
	m_sampleSkip = 0;
	m_state = IDLE;
	m_syncMode = true;
	m_tempResult.clear();
	m_syncBits.clear();
	resetState();
}

void CANDecoder::Impl::resetState(){
	m_isFlexibleData = false;
	m_frameType = None;
	m_dataLenghtCode = 0;
	m_lastDatabit = 999; // Positive value that bitnum+x will never match
	m_ssBlock = 0;
	m_dlcStart = 0;
	m_crcLen = 0;
	m_crc = 0;
	m_dataBits.clear();
	m_rawBits.clear();
	m_bits.clear();
	m_curBit = 0;
	m_id = 0;
	m_rtr = 0;
}

void CANDecoder::decode(const uint8_t* _input, uint32_t _size){
	m_impl->decode(_input, _size);
}

void CANDecoder::Impl::decode(const uint8_t* _input, uint32_t _size)
{
	if (!_input) FATAL("Input value is null")
	if (_size == 0) FATAL("Input value size == 0")
	if (_size & 0x1) FATAL("Input value is odd")
	if (m_options.m_can_rx > 8) FATAL("RX line more than 8")
	if (m_options.m_can_rx == 0) FATAL("RX line should not be equal to 0")

	TRACE_SHORT("Input data size %d",_size)

	resetDecoder();

	bool disableSynOnStart = true;
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

			if (m_state == IDLE)
			{
				if (isLowState(can_rx)){
					domEdgeSeen();
					m_state = CORRECT;
					disableSynOnStart = false;
				}else{
					m_tempResult.push_back(OutputPacket{NOTHING, can_rx, 1});
					if (m_bitwidth * 8 < m_samplenum  && disableSynOnStart) {
						m_syncMode = false;
						disableSynOnStart = false;
					}
				}
			}

			if (m_state == CORRECT){
				int32_t pos = getSamplePoint(m_curBit);
				m_sampleSkip = pos - m_samplenum;
				m_state = GET_INFO;
			}

			if (m_state == GET_INFO){
				if (isFallEdgeState(can_rx)){
					domEdgeSeen();
					m_state = CORRECT;
				}

				if(m_sampleSkip <= 0){
					if (m_syncMode){
						m_syncBits.push_back(can_rx);
						m_curBit++;
						m_tempResult.push_back(OutputPacket{SYNC, can_rx, 1});
						if (m_syncBits.size() >= 7){
							std::vector<bool> last_7_bits(m_syncBits.end() - 7, m_syncBits.end());
							std::vector<bool> data { 1, 1, 1, 1, 1, 1, 1 };
							if (last_7_bits == data){
								m_state = IDLE;
								m_syncMode = false;

							}else{
								m_state = CORRECT;
							}
						}else{
							m_state = CORRECT;
						}
					}else{
						if (handleBit(can_rx)) {
							if (m_tempResult.back().control == ERROR_3){
								m_syncMode = true;
								m_syncBits.clear();
								m_state = CORRECT;
							}else{
								m_state = IDLE;
							}
						}else{
							m_state = CORRECT;
						}
					}
				}else{
					m_tempResult.push_back(OutputPacket{NOTHING, can_rx, 1});
				}

				if (m_sampleSkip > 0){
					m_sampleSkip--;
				}
			}

			m_oldCanRX = can_rx;

			assert(m_samplenum < std::numeric_limits<decltype(m_samplenum)>::max() && "m_Samplenum overflow");
			m_samplenum++;
		}
	}
	m_tempResult = packResult(m_tempResult);
	uint32_t sum = 0;
	for(auto i = 0u; i < m_tempResult.size(); i ++) {
		if (sum <= m_options.m_frame_limit) {
				m_result.push_back(m_tempResult[i]);
			}
		if (m_tempResult[i].control == END_OF_FRAME) {
			sum++;
		}
	}
	fprintf(stderr,"Pack result: %d frames found: %d\n",m_tempResult.size(),sum);
}

std::vector<OutputPacket> CANDecoder::Impl::packResult(std::vector<OutputPacket> &_data){
	std::vector<OutputPacket> new_result;
	for (size_t i = 0; i < _data.size(); i++){
		if (new_result.size () > 0 ){
			if ((new_result.back().control == _data[i].control) &&
					(new_result.back().data == _data[i].data)){
						uint32_t sum_size = _data[i].length + new_result.back().length;
						if (sum_size > 0xFFFF){
							new_result.back().length = 0xFFFF;
							new_result.push_back(_data[i]);
							new_result.back().length = sum_size - 0xFFFF;
						}else{
							new_result.back().length = sum_size;
						}
					}else{
						new_result.push_back(_data[i]);
					}
		}else{
			new_result.push_back(_data[i]);
		}
	}
	return new_result;
}


void CANDecoder::Impl::domEdgeSeen(){
	m_dom_edge_snum = m_samplenum;
	m_dom_edge_bcount = m_curBit;
}

uint32_t CANDecoder::Impl::getSamplePoint(uint32_t _curbit){
  	uint32_t samplenum = m_dom_edge_snum;
    samplenum += m_bitwidth * (_curbit - m_dom_edge_bcount);
    samplenum += m_samplePoint;
    return samplenum;
}

void CANDecoder::Impl::setBitRate(uint32_t _bitrate){
    m_bitwidth = float(m_samplerate) / float(_bitrate);
    m_samplePoint = (m_bitwidth / 100.0) * m_options.m_sample_point;
}

void CANDecoder::Impl::setNominalBitrate(){
	setBitRate(m_options.m_nominal_bitrate);
}

void CANDecoder::Impl::setFastBitrate(){
	setBitRate(m_options.m_fast_bitrate);
}

bool CANDecoder::Impl::isLowState(bool _can_rx) const
{
	return (_can_rx == 0);
}

bool CANDecoder::Impl::isFallEdgeState(bool _can_rx) const
{
	return (m_oldCanRX == 1 && _can_rx == 0);
}


bool CANDecoder::Impl::handleBit(const bool _can_rx){
	m_bits.push_back(_can_rx);
	m_rawBits.push_back(_can_rx);
	uint32_t bitnum = m_bits.size() - 1;

	if (m_isFlexibleData && _can_rx){
		if ((bitnum == 16 && m_frameType == Standart) || (bitnum == 35 && m_frameType == Extended)){
			domEdgeSeen();
			setFastBitrate();
		}
	}

	// If this is a stuff bit, remove it from self.bits and ignore it.

	if (isStuffBit()){
		m_tempResult.push_back(OutputPacket{STUFF_BIT, _can_rx, 1}); // Put stuff bit
		m_curBit ++; // Increase curbit (bitnum is not affected).
		return false;
	}else{
		m_tempResult.push_back(OutputPacket{BIT, _can_rx, 1}); // Put bit
	}


 	// Bit 0: Start of frame (SOF) bit
    if (bitnum == 0) {
        m_tempResult.push_back(OutputPacket{START_OF_FRAME, _can_rx, 1});
        if (_can_rx != 0){
			m_tempResult.push_back(OutputPacket{ERROR_1, _can_rx, 1});
		}
	}

	// Remember start of ID (see below).
	if (bitnum == 1) {
        m_ssBlock = m_samplenum;
	}

	// Bits 1-11: Identifier (ID[10..0])
	// The bits ID[10..4] must NOT be all recessive.
    if (bitnum == 11) {
		std::vector<bool> id_bits(m_bits.begin() + 1, m_bits.end());
		m_id = convert(id_bits);
		m_tempResult.push_back(OutputPacket{ID, m_id, 1});
		if ((m_id & 0x7f0) == 0x7f0){
			m_tempResult.push_back(OutputPacket{WARNING_1, m_id, 1});
		}
	}

	// RTR or SRR bit, depending on frame type (gets handled later).
    if (bitnum == 12){
        m_ssBit12 = m_samplenum;
	}

	// Bit 13: Identifier extension (IDE) bit
    // Standard frame: dominant, extended frame: recessive
    if (bitnum == 13)
	{
		m_frameType = Standart;
		if (_can_rx == 1) m_frameType = Extended;
		m_tempResult.push_back(OutputPacket{IDE, _can_rx, 1});
	}

	if (bitnum >= 14){
		bool ret = false;
        if (m_frameType == Standart){
            ret = decodeStandardFrame(_can_rx, bitnum);
		}else{
            ret = decodeExtendedFrame(_can_rx, bitnum);
		}
        // The handlers return True if a frame ended (EOF).
        if (ret) return true;
	}
	m_curBit ++;
	return false;
}

bool CANDecoder::Impl::decodeStandardFrame(const bool _can_rx, uint32_t bitnum){
	// Bit 14: FDF (Flexible data format)
    // Has to be sent dominant when FD frame, has to be sent recessive when classic CAN frame.
    if (bitnum == 14){
		m_isFlexibleData = false;
		if (_can_rx) m_isFlexibleData = true;
		if (m_isFlexibleData){
			m_tempResult.push_back(OutputPacket{RESERV_BIT_FLEX, _can_rx, 1});
			m_tempResult.push_back(OutputPacket{RESERV_BIT, _can_rx, 1});
		}else{
			m_tempResult.push_back(OutputPacket{RESERV_BIT, _can_rx, 1});
		}
		if (m_isFlexibleData){
			//  Bit 12: Substitute remote request (SRR) bit
			// m_Result.push_back(OutputPacket{SRR, _can_rx, 1}); // This mode default by FlexibleData
			m_dlcStart = 18;
		}else{
         	//  Bit 12: Remote transmission request (RTR) bit
            //  Data frame: dominant, remote frame: recessive
            //  Remote frames do not contain a data field.
			m_tempResult.push_back(OutputPacket{RTR, m_bits[12], 1});
			m_dlcStart = 15;
		}
	}

	if (bitnum == 15 && m_isFlexibleData){
        m_tempResult.push_back(OutputPacket{RESERV_BIT, _can_rx, 1});
	}


	if (bitnum == 16 && m_isFlexibleData){
		m_tempResult.push_back(OutputPacket{BRS, _can_rx, 1});
	}

    if (bitnum == 17 && m_isFlexibleData){
		m_tempResult.push_back(OutputPacket{ESI, _can_rx, 1});
 	}

	// Remember start of DLC (see below).
    if (bitnum == m_dlcStart){
		m_ssBlock = m_samplenum;
	}

	// Bits 15-18: Data length code (DLC), in number of bytes (0-8).
    if (bitnum == m_dlcStart + 3){
		std::vector<bool> dlc_bits(m_bits.begin() + m_dlcStart, m_bits.begin() + m_dlcStart + 4);
		m_dataLenghtCode = convert(dlc_bits);
		m_tempResult.push_back(OutputPacket{DLC, m_dataLenghtCode, 1});
		m_lastDatabit = m_dlcStart + 3 + dlc2Len(m_dataLenghtCode) * 8;
		if (m_dataLenghtCode > 8 && !m_isFlexibleData){
			m_tempResult.push_back(OutputPacket{ERROR_2, m_dataLenghtCode, 1});
		}
	}

	// Remember all databyte bits, except the very last one.
    if (bitnum >= m_dlcStart + 4 && bitnum <= m_lastDatabit){
		m_dataBits.push_back(m_samplenum);
		if (m_dataBits.size() == 8){
			std::vector<bool> data_bits(m_bits.end() - 8, m_bits.end());
			uint32_t value = convert(data_bits);
			m_tempResult.push_back(OutputPacket{PAYLOAD_DATA, value, 1});
		}
	}

	if (bitnum > m_lastDatabit){
        return decodeEndFrame(_can_rx, bitnum);
	}

	return false;
}

bool CANDecoder::Impl::decodeExtendedFrame(const bool _can_rx,uint32_t bitnum){
	// Remember start of EID (see below).
    if (bitnum == 14){
        m_ssBlock = m_samplenum;
		m_isFlexibleData = false;
		m_dlcStart = 35;
	}

	// Bits 14-31: Extended identifier (EID[17..0])
    if (bitnum == 31){
		std::vector<bool> id_bits(m_bits.begin() + 14, m_bits.end());
		uint32_t eid = convert(id_bits);
		m_tempResult.push_back(OutputPacket{EXT_ID, eid, 1});
		uint32_t fullid = m_id << 18 | eid;
		m_tempResult.push_back(OutputPacket{FULL_ID, fullid, 1});
		// Bit 12: Substitute remote request (SRR) bit
		m_tempResult.push_back(OutputPacket{SRR, m_bits[12], 1});
	}

	// Bit 32: Remote transmission request (RTR) bit
    // Data frame: dominant, remote frame: recessive
    // Remote frames do not contain a data field.

    // Remember start of RTR (see below).
    if (bitnum == 32) {
		m_ssBit12 = m_samplenum;
		m_rtr = _can_rx;
		if (!m_isFlexibleData){
			m_tempResult.push_back(OutputPacket{RTR, _can_rx, 1});
		}
	}


	// Bit 33: RB1 (reserved bit)
    if (bitnum == 33){
		m_isFlexibleData = false;
		if (_can_rx) m_isFlexibleData = true;
		if (m_isFlexibleData) {
			m_dlcStart = 37;
			m_tempResult.push_back(OutputPacket{RESERV_BIT_FLEX, _can_rx, 1});
			m_tempResult.push_back(OutputPacket{RESERV_BIT, _can_rx, 1});
		}else{
			m_tempResult.push_back(OutputPacket{RESERV_BIT, _can_rx, 1});
		}
	}


	// Bit 34: RB0 (reserved bit)
    if (bitnum == 34){
		m_tempResult.push_back(OutputPacket{RESERV_BIT, _can_rx, 1});
	}

	if (bitnum == 35 && m_isFlexibleData){
		m_tempResult.push_back(OutputPacket{BRS, _can_rx, 1});
	}

    if (bitnum == 36 && m_isFlexibleData){
		m_tempResult.push_back(OutputPacket{ESI, _can_rx, 1});
 	}

	// Remember start of DLC (see below).
    if (bitnum == m_dlcStart){
		m_ssBlock = m_samplenum;
	}

 	// Bits 35-38: Data length code (DLC), in number of bytes (0-8).
	 if (bitnum == m_dlcStart + 3){
		std::vector<bool> dlc_bits(m_bits.begin() + m_dlcStart, m_bits.begin() + m_dlcStart + 4);
		m_dataLenghtCode = convert(dlc_bits);
		m_tempResult.push_back(OutputPacket{DLC, m_dataLenghtCode, 1});
		m_lastDatabit = m_dlcStart + 3 + dlc2Len(m_dataLenghtCode) * 8;
		if (m_dataLenghtCode > 8 && !m_isFlexibleData){
			m_tempResult.push_back(OutputPacket{ERROR_2, m_dataLenghtCode, 1});
		}
	}

	// Remember all databyte bits, except the very last one.
    if (bitnum >= m_dlcStart + 4 && bitnum <= m_lastDatabit){
		m_dataBits.push_back(m_samplenum);
		if (m_dataBits.size() == 8){
			std::vector<bool> data_bits(m_bits.end() - 8, m_bits.end());
			uint32_t value = convert(data_bits);
			m_tempResult.push_back(OutputPacket{PAYLOAD_DATA, value, 1});
		}
	}

	if (bitnum > m_lastDatabit){
        return decodeEndFrame(_can_rx, bitnum);
	}

	return false;
}

bool CANDecoder::Impl::decodeEndFrame(const bool _can_rx,uint32_t bitnum){
 	// Remember start of CRC sequence (see below).
    if (bitnum == m_lastDatabit + 1){
		m_ssBlock = m_samplenum;
		if (m_isFlexibleData) {
			if (dlc2Len(m_dataLenghtCode) < 16) {
				m_crcLen = 27; // 17 + SBC + stuff bits
			}else{
				m_crcLen = 32; // 21 + SBC + stuff bits
			}
		}else{
			m_crcLen = 15;
		}
	}


	// CRC sequence (15 bits, 17 bits or 21 bits)
	if (bitnum == m_lastDatabit + m_crcLen){
		if (m_isFlexibleData) {
			if (dlc2Len(m_dataLenghtCode) < 16) {
				m_tempResult.push_back(OutputPacket{CRC_LEN, 17, 1});
			}else{
				m_tempResult.push_back(OutputPacket{CRC_LEN, 21, 1});
			}
		}else{
			m_tempResult.push_back(OutputPacket{CRC_LEN, 15, 1});
		}

		uint32_t x = m_lastDatabit + 1;
		std::vector<bool> crc_bits(m_bits.begin() + x, m_bits.begin() + x + m_crcLen);
		m_crc = convert(crc_bits);
		m_tempResult.push_back(OutputPacket{CRC_SEQ, m_crc, 1});
	}

    // CRC delimiter bit (recessive)
    if (bitnum == m_lastDatabit + m_crcLen + 1){
		m_tempResult.push_back(OutputPacket{CRC_DELIMITER, _can_rx, 1});
		if (_can_rx != 1) {
			m_tempResult.push_back(OutputPacket{WARNING_2, _can_rx, 1});
		}
		if (m_isFlexibleData) {
			setNominalBitrate();
		}
	}

 	// ACK slot bit (dominant: ACK, recessive: NACK)
    if (bitnum == m_lastDatabit + m_crcLen + 2){
		m_tempResult.push_back(OutputPacket{ACK_SLOT, _can_rx, 1});
	}

 	// ACK delimiter bit (recessive)
    if (bitnum == m_lastDatabit + m_crcLen + 3){
		m_tempResult.push_back(OutputPacket{ACK_DELIMITER, _can_rx, 1});
		if (_can_rx != 1) {
			m_tempResult.push_back(OutputPacket{WARNING_3, _can_rx, 1});
		}
	}

    // Remember start of EOF (see below).
    if (bitnum == m_lastDatabit + m_crcLen + 4){
		m_ssBlock = m_samplenum;
	}

	// End of frame (EOF), 7 recessive bits
    if (bitnum == m_lastDatabit + m_crcLen + 10){
		m_tempResult.push_back(OutputPacket{END_OF_FRAME, _can_rx, 1});
		std::vector<uint32_t> last_7_bits(m_rawBits.end() - 7, m_rawBits.end());
		std::vector<uint32_t> data { 1, 1, 1, 1, 1, 1, 1 };
		if (last_7_bits != data){
			m_tempResult.push_back(OutputPacket{ERROR_3, 0, 1});
		}
		resetState();
		return true;
	}
	return false;
}

bool CANDecoder::Impl::isStuffBit(){
	// Don't chack end of frame.
	if (m_bits.size() > m_lastDatabit + 17)
		return false;

	std::vector<bool> last_6_bits(m_rawBits.end() - 6, m_rawBits.end());
	std::vector<bool> data_1 { 0, 0, 0, 0, 0, 1 };
	std::vector<bool> data_2 { 1, 1, 1, 1, 1, 0 };
	if (last_6_bits != data_1 && last_6_bits != data_2){
		return false;
	}
	//  Stuff bit. Keep it in self.rawbits, but drop it from self.bits.
	m_bits.pop_back();
	return true;
}

uint32_t CANDecoder::Impl::convert(const std::vector<bool> &_bits){
	if (_bits.size() > 32)
		FATAL("Bit array greater than 32")
	uint32_t x = 0;
	for (size_t i = 0 ;i < _bits.size() ; i++){
    	if (_bits[i] > 0 ) {
			x = x | (_bits[i] << (_bits.size() - i - 1));
		}
	}
	return x;
}

uint8_t CANDecoder::Impl::dlc2Len(uint8_t _dlc){
	uint8_t code_page[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
	return code_page[_dlc];
}