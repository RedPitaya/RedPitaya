#include "i2c_decoder.h"

#include <initializer_list>
#include <string>
#include <cstdio>
#include <limits>
#include <algorithm>
#include <cassert>

#include "rp.h"

using namespace i2c;


class I2CDecoder::Impl{

	enum States
	{
		FIND_START,
		FIND_ADDRESS,
		FIND_DATA,
		FIND_ACK
	};

	public:

	Impl();

	I2CParameters m_options;

	bool m_oldScl;
	bool m_oldSda;
	bool m_oldPins;
	bool m_needPush;
	uint32_t m_ss;
	uint32_t m_es;
	uint32_t m_ssByte;
	uint8_t m_dataByte;

	uint32_t m_bitwidth;
	uint32_t m_samplerate;
	uint32_t m_samplenum;
	uint32_t m_oldSamplenum;
	uint32_t m_bitcount;
	float    m_bitCountDetect;
	uint32_t m_nothing;
	uint32_t m_nothingStart;

	int m_wr;
	uint32_t m_is_repeat_start;
	uint32_t m_pdu_start;
	uint32_t m_pdu_bits;

	I2CAnnotations m_cmd;
	States m_state;
	States m_oldState;

	std::deque<std::vector<uint32_t>> m_bits;
	std::vector<OutputPacket> m_result;

	void resetDecoder();
	void addNothing();
    void decode(const uint8_t* _input, uint32_t _size);

	inline bool isStartCondition(bool scl, bool sda) const;
	inline bool isDataBit(bool scl, bool sda) const;
	inline bool isStopCondition(bool scl, bool sda) const;

	void getAck(bool scl, bool sda);
	void foundStart(bool scl, bool sda);
	bool foundAddressOrData(bool scl, bool sda);
	void foundStop(bool scl, bool sda);

};

I2CDecoder::Impl::Impl(){

}

I2CDecoder::I2CDecoder(int decoderType, const std::string& _name){
	m_decoderType = decoderType;
	m_name = _name;
	m_impl = new Impl();
	m_impl->resetDecoder();
	setParameters(i2c::I2CParameters());
}

I2CDecoder::~I2CDecoder(){
	delete m_impl;
}

auto I2CDecoder::reset() -> void{
	m_impl->resetDecoder();
}

auto I2CDecoder::getMemoryUsage() -> uint64_t{
	uint64_t size = sizeof(Impl);
	size += m_impl->m_bits.size() * sizeof(std::deque<std::vector<uint32_t>>);
	for(auto itm : m_impl->m_bits){
		size += itm.size() * sizeof(uint32_t);
	}
	size += m_impl->m_result.size() * sizeof(OutputPacket);
	return size;
}

void I2CDecoder::setParameters(const I2CParameters& _new_params){
	m_impl->m_options = _new_params;
}


auto I2CDecoder::setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool{
	auto opt = m_impl->m_options;
	if (opt.setDecoderSettingsUInt(key,value)){
		setParameters(opt);
		return true;
	}
	return false;
}

auto I2CDecoder::getDecoderSettingsUInt(std::string& key, uint32_t *value) -> bool{
	return m_impl->m_options.getDecoderSettingsUInt(key,value);
}

auto I2CDecoder::getParametersInJSON() -> std::string{
	return m_impl->m_options.toJson();
}

auto I2CDecoder::setParametersInJSON(const std::string &parameter) -> void{
	I2CParameters param;
	if (param.fromJson(parameter)){
		setParameters(param);
	}else{
		ERROR_LOG("Error set parameters %s", parameter.c_str())
	}
}

std::vector<OutputPacket> I2CDecoder::getSignal(){
	return m_impl->m_result;
}

void I2CDecoder::Impl::resetDecoder(){
	m_oldScl = 1;
	m_oldSda = 1;
	m_oldPins = 0;
	m_nothing = 0;
	m_nothingStart = 0;
	m_ss = -1;
	m_es = -1;
	m_ssByte = -1;
	m_needPush = false;
	m_dataByte = 0;
	m_samplerate = 0;
	m_samplenum = 0;
	m_oldSamplenum = 0;
	m_bitCountDetect = 0;
	m_bitcount = 0;
	m_wr = 1;
	m_is_repeat_start = 0;
	m_pdu_start = 0;
	m_pdu_bits = 0;
	m_state = FIND_START;
	m_oldState = FIND_START;

	m_result.clear();
}

void I2CDecoder::Impl::addNothing(){
	// m_result.push_back(OutputPacket{"sda", NOTHING, 0, m_nothing, 0 , m_nothingStart});
	m_nothing = 0;
}

void I2CDecoder::decode(const uint8_t* _input, uint32_t _size){
	m_impl->decode(_input,_size);
}

void I2CDecoder::Impl::decode(const uint8_t* _input, uint32_t _size)
{

	auto push = [&](){
		m_result.push_back(OutputPacket{"sda", (uint8_t)m_cmd, m_dataByte, (m_es - m_ss),m_bitCountDetect, m_ss});
		m_bitCountDetect = 0;
		m_dataByte = 0;
		m_needPush = false;
	};

	if (!_input) FATAL("Input value is null")
	if (_size == 0) FATAL("Input value size == 0")
	if (_size & 0x1) FATAL("Input value is odd")
	if (m_options.m_scl == 0 || m_options.m_sda == 0 || m_options.m_scl > 8 || m_options.m_sda > 8){
		ERROR_LOG("SCL and SDA not specified. Valid values from 1 to 8")
		return;
	}

	TRACE_SHORT("Input data size %d",_size)

	resetDecoder();

	uint8_t scl_line = m_options.m_scl - 1;
	uint8_t sda_line = m_options.m_sda - 1;
	bool needInit = true;

	for (size_t i = 0; i < _size; i += 2)
	{
		const uint8_t count = _input[i]; // 0 = 1 count, 1 = 2, etc.
		const uint8_t data = _input[i + 1];
		for (uint16_t j = 0; j < count + 1; ++j)
		{
			bool isEnd = (i + 2 >= _size) && j == count;
			bool scl = (data & 1 << scl_line);
			bool sda = (data & 1 << sda_line);

			if (m_options.m_invert_bit != 0){
				scl = !scl;
				sda = !sda;
			}

			if (needInit){
				m_oldScl = scl;
				m_oldSda = sda;
				needInit = false;
			}

			// state machine
			if (m_state == FIND_START)
			{
				if (isStartCondition(scl, sda)){
					if (m_nothing > 0){
						addNothing();
					}
					foundStart(scl, sda);
					m_needPush = true;
				}
				else{
					m_nothing++;
				}
			}
			else if (m_state == FIND_ADDRESS){
				if (isDataBit(scl, sda)){
					if (m_needPush){
						push();
					}
					if (foundAddressOrData(scl, sda)){
						m_needPush = true;
					}
				}
				else
					m_es++;
			}
			else if (m_state == FIND_DATA){
				if (isDataBit(scl, sda)){
					if (m_needPush){
						push();

					}
					if (foundAddressOrData(scl, sda)){
						m_needPush = true;
					}
				}
				else if (isStartCondition(scl, sda)){
					if (m_needPush){
						push();
					}
					foundStart(scl, sda);
					m_needPush = true;
				}
				else if (isStopCondition(scl, sda)){
					if (m_needPush){
						push();
					}
					foundStop(scl, sda);
					push();
				}
				else
					m_es++;
			}
			else if (m_state == FIND_ACK)
			{
				if (isDataBit(scl, sda)){
					if (m_needPush){
						push();
					}
					getAck(scl, sda);
					m_needPush = true;
				}else{
					m_es++;
				}
			}

			if (m_state != m_oldState){
				m_oldState = m_state;
				m_oldSamplenum = m_samplenum;
			}

			// save current SDA/SCL values for the next round.
			m_oldScl = scl;
			m_oldSda = sda;

			assert(m_samplenum < std::numeric_limits<decltype(m_samplenum)>::max() && "m_samplenum overflow");
			++m_samplenum;

			if (m_needPush && isEnd){
				push();
			}
		}
	}
}

bool I2CDecoder::Impl::isStartCondition(bool scl, bool sda) const{
	return (m_oldSda == 1 && sda == 0) && scl == 1;
}

bool I2CDecoder::Impl::isDataBit(bool scl, bool sda) const{
	return m_oldScl == 0 && scl == 1;
}

bool I2CDecoder::Impl::isStopCondition(bool scl, bool sda) const{
	return (m_oldSda == 0 && sda == 1) && scl == 1;
}

void I2CDecoder::Impl::getAck(bool scl, bool sda){
	m_es = m_ss = m_samplenum;
	m_cmd = (sda == 1) ? NACK : ACK;
	// There could be multiple data bytes in a row, so either find
	// another data byte or a STOP condition next.
	m_state = FIND_DATA;
}

void I2CDecoder::Impl::foundStart(bool scl, bool sda){
	m_nothingStart = m_es;
	m_ss = m_es = m_samplenum;

	m_pdu_start = m_samplenum;
	m_pdu_bits = 0;
	m_cmd = (m_is_repeat_start == 1) ? REPEAT_START : START;
	m_state = FIND_ADDRESS;
	m_bitcount = m_dataByte = 0;
	m_is_repeat_start = 1;
	m_wr = -1;
	m_bits.clear();
}

bool I2CDecoder::Impl::foundAddressOrData(bool scl, bool sda){
	// Address and data are transmitted MSB-first.
	m_dataByte <<= 1;
	m_dataByte |= sda;

	// Remember the start of the first data/address bit.
	if (m_bitcount == 0)
		m_ssByte = m_samplenum;

	// Store individual bits and their start/end samplenumbers.
	// In the list, index 0 represents the LSB (I²C transmits MSB-first).

	m_bits.insert(m_bits.begin(), std::vector<uint32_t>{sda, m_samplenum, m_samplenum});
	if (m_bitcount > 0)
	{
		assert(m_bits.size() >= 2 && m_bits[1].size() >= 3 && "m_Bits out of range");
		m_bits[1][2] = m_samplenum;
	}
	if (m_bitcount == 7)
	{
		assert(m_bits[0].size() >= 3 && m_bits[2].size() >= 3 && "m_Bits out of range");
		m_bitwidth = m_bits[1][2] - m_bits[2][2];
		m_bits[0][2] += m_bitwidth;
	}

	// Return if we haven't collected all 8 + 1 bits, yet.

	if (m_bitcount < 7){
		m_bitcount += 1;
		return false;
	}

	if (m_state == FIND_ADDRESS)
	{
		// The READ/WRITE bit is only in address bytes, not data bytes.
		m_wr = (m_dataByte & 1) ? 0 : 1;
		if (m_options.m_address_format == Shifted)
			m_dataByte >>= 1;
	}

	m_cmd = WRITE_ADDRESS;
	if (m_state == FIND_ADDRESS && m_wr == 1){
		m_cmd = WRITE_ADDRESS;
	}
	else if (m_state == FIND_ADDRESS && m_wr == 0){
		m_cmd = READ_ADDRESS;
	}
	else if (m_state == FIND_DATA && m_wr == 1){
		m_cmd = DATA_WRITE;
	}
	else if (m_state == FIND_DATA && m_wr == 0){
		m_cmd = DATA_READ;
	}
	else{
		FATAL("incorrect i2c decoder state");
	}
	m_bitCountDetect = m_bitcount + 1;
	m_ss = m_ssByte;
	m_es = m_samplenum;
	// Done with this packet.
	m_bitcount = 0;
	m_bits.clear();
	m_state = FIND_ACK;
	return true;
}

void I2CDecoder::Impl::foundStop(bool scl, bool sda){
	m_cmd = STOP;
	m_ss = m_es = m_samplenum;
	m_state = FIND_START;
	m_is_repeat_start = 0;
	m_wr = -1;
	m_bits.clear();
}

