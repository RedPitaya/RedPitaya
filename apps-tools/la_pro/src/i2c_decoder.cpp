#include "i2c_decoder.h"

#include <initializer_list>
#include <string>
#include <cstdio>
#include <limits>
#include <algorithm>
#include <cassert>

I2CDecoder::I2CDecoder(const std::string& _name):
	m_Name(_name),
	m_ParametersUpdated(false)
#ifndef CLI
	,
	m_Signal(m_Name + "_signal", 1, {0,0,0}),
	m_Parameters(m_Name + "_parameters", CBaseParameter::RW, m_Options, 0)
#endif
{
	ResetDecoder();
	m_Options.address_format = Shifted;
}

void I2CDecoder::SetParameters(const I2CParameters& _new_params)
{
	m_Options = _new_params;
	--m_Options.scl;
	--m_Options.sda;
}

bool I2CDecoder::IsParametersChanged()
{
#ifndef CLI
	if(m_Parameters.IsNewValue()){
	}
	return (m_Parameters.IsNewValue()) || m_ParametersUpdated;
#else
    return false;
#endif // CLI
}

void I2CDecoder::UpdateParameters()
{
#ifndef CLI
	m_Parameters.Update();
	SetParameters(m_Parameters.Value());
#endif
	m_ParametersUpdated = true;
}

void I2CDecoder::UpdateSignals()
{
#ifndef CLI
	m_Signal.Update();
#endif
}

void I2CDecoder::ResetDecoder()
{
	m_OldScl = 1;
	m_OldSda = 1;
	m_OldPins = 0;
	m_Ss = -1;
	m_Es = -1;
	m_Ss_byte = -1;
	m_Databyte = 0;
	m_Samplerate = 0;
	m_Samplenum = 0;
	m_OldSamplenum = 0;
	m_Bitcount = 0;
	m_Wr = 1;
	m_Is_repeat_start = 0;
	m_Pdu_start = 0;
	m_Pdu_bits = 0;
	m_State = FIND_START;
	m_OldState = FIND_START;

	m_Result.clear();
}

void I2CDecoder::AddNothing()
{

	int len = (int)m_Samplenum - (int)m_OldSamplenum - (int)(m_Es - m_Ss);
	if(m_State == FIND_ACK)
		--len;
	if (len < 0)
		return;
	while(len > 0xFFFF)
	{
		len -= 0xFFFF;
		m_Result.push_back(OutputPacket{NOTHING, 0, (uint16_t)0xFFFF});
	}

	m_Result.push_back(OutputPacket{NOTHING, 0, (uint16_t)len});
}

void I2CDecoder::Decode(const uint8_t* _input, uint32_t _size)
{
#ifndef CLI
	pthread_mutex_lock(&m_mutex);
#endif
	assert(_input && "null ptr");
	assert(_size > 0 && "input vector size == 0");
	assert(_size % 2 == 0 && "odd size");
#ifndef CLI
	if (m_Parameters.IsNewValue())
	{
		m_Parameters.Update();
		SetParameters(m_Parameters.Value());
		m_Start = true;
	}

	// if (!m_Start)
	// 	return;
#endif
	ResetDecoder();

	m_ParametersUpdated = false;

	for (size_t i = 0; i < _size; i += 2)
	{
		const uint8_t count = _input[i]; // 0 = 1 count, 1 = 2, etc.
		const uint8_t data = _input[i + 1];
		for (uint16_t j = 0; j < count + 1; ++j)
		{
			bool scl = (data & 1 << m_Options.scl) >> m_Options.scl;
			bool sda = (data & 1 << m_Options.sda) >> m_Options.sda;

			if (m_Options.invert_bit != 0){
				scl = !scl;
				sda = !sda;
			}
			
			// state machine
			if (m_State == FIND_START)
			{
				if (IsStartCondition(scl, sda))
					FoundStart(scl, sda);
			}
			else if (m_State == FIND_ADDRESS)
			{
				if (IsDataBit(scl, sda))
					FoundAddressOrData(scl, sda);
			}
			else if (m_State == FIND_DATA)
			{
				if (IsDataBit(scl, sda))
					FoundAddressOrData(scl, sda);
				else if (IsStartCondition(scl, sda))
					FoundStart(scl, sda);
				else if (IsStopCondition(scl, sda))
					FoundStop(scl, sda);
			}
			else if (m_State == FIND_ACK)
			{
				if (IsDataBit(scl, sda))
					GetAck(scl, sda);
			}

			if (m_State != m_OldState)
			{
				AddNothing();
				m_Result.push_back(OutputPacket{m_Cmd, m_Databyte, (uint16_t)(m_Es - m_Ss)});
				m_OldState = m_State;
				m_OldSamplenum = m_Samplenum;
			}

			// save current SDA/SCL values for the next round.
			m_OldScl = scl;
			m_OldSda = sda;

			assert(m_Samplenum < std::numeric_limits<decltype(m_Samplenum)>::max() && "m_Samplenum overflow");
			++m_Samplenum;
		}
	}

#ifndef CLI
	m_Signal.Set(std::move(m_Result));
	pthread_mutex_unlock(&m_mutex);
#endif
}

bool I2CDecoder::IsStartCondition(bool scl, bool sda) const
{
	return (m_OldSda == 1 && sda == 0) && scl == 1;
}

bool I2CDecoder::IsDataBit(bool scl, bool sda) const
{
	return m_OldScl == 0 && scl == 1;
}

bool I2CDecoder::IsStopCondition(bool scl, bool sda) const
{
	return (m_OldSda == 0 && sda == 1) && scl == 1;
}

void I2CDecoder::GetAck(bool scl, bool sda)
{
	m_Ss = m_Samplenum;
	m_Es = m_Samplenum + m_Bitwidth;
	m_Cmd = (sda == 1) ? NACK : ACK;
	// There could be multiple data bytes in a row, so either find
	// another data byte or a STOP condition next.
	m_State = FIND_DATA;

	assert(m_Bitwidth <= std::numeric_limits<uint16_t>::max() && "m_Bitwidth overflow");
}

void I2CDecoder::FoundStart(bool scl, bool sda)
{
	m_Ss = m_Es = m_Samplenum;

	m_Pdu_start = m_Samplenum;
	m_Pdu_bits = 0;
	m_Cmd = (m_Is_repeat_start == 1) ? REPEAT_START : START;
	m_State = FIND_ADDRESS;
	m_Bitcount = m_Databyte = 0;
	m_Is_repeat_start = 1;
	m_Wr = -1;
	m_Bits.clear();
}

void I2CDecoder::FoundAddressOrData(bool scl, bool sda)
{
	// Address and data are transmitted MSB-first.
	m_Databyte <<= 1;
	m_Databyte |= sda;

	// Remember the start of the first data/address bit.
	if (m_Bitcount == 0)
		m_Ss_byte = m_Samplenum;

	// Store individual bits and their start/end samplenumbers.
	// In the list, index 0 represents the LSB (I²C transmits MSB-first).

	m_Bits.insert(m_Bits.begin(), std::vector<uint32_t>{sda, m_Samplenum, m_Samplenum});
	if (m_Bitcount > 0)
	{
		assert(m_Bits.size() >= 2 && m_Bits[1].size() >= 3 && "m_Bits out of range");
		m_Bits[1][2] = m_Samplenum;
	}
	if (m_Bitcount == 7)
	{
		assert(m_Bits[0].size() >= 3 && m_Bits[2].size() >= 3 && "m_Bits out of range");
		m_Bitwidth = m_Bits[1][2] - m_Bits[2][2];
		m_Bits[0][2] += m_Bitwidth;
	}

	// Return if we haven't collected all 8 + 1 bits, yet.

	if (m_Bitcount < 7)
	{
		m_Bitcount += 1;
		return;
	}

	if (m_State == FIND_ADDRESS)
	{
		// The READ/WRITE bit is only in address bytes, not data bytes.
		m_Wr = (m_Databyte & 1) ? 0 : 1;
		if (m_Options.address_format == Shifted)
			m_Databyte >>= 1;
	}

	m_Cmd = WRITE_ADDRESS;
	if (m_State == FIND_ADDRESS && m_Wr == 1)
	{
		m_Cmd = WRITE_ADDRESS;
	}
	else if (m_State == FIND_ADDRESS && m_Wr == 0)
	{
		m_Cmd = READ_ADDRESS;
	}
	else if (m_State == FIND_DATA && m_Wr == 1)
	{
		m_Cmd = DATA_WRITE;
	}
	else if (m_State == FIND_DATA && m_Wr == 0)
	{
		m_Cmd = DATA_READ;
	}
	else
	{
		assert(0 && "incorrect i2c decoder state");
	}

	m_Ss = m_Ss_byte;
	m_Es = m_Samplenum + m_Bitwidth;

	assert(m_Es >= m_Ss && "END < START");

	if (m_Cmd == READ_ADDRESS || m_Cmd == WRITE_ADDRESS)
	{
		m_Es = m_Samplenum;
	}

	// Done with this packet.
	m_Bitcount = 0;
	m_Bits.clear();
	m_State = FIND_ACK;
}

void I2CDecoder::FoundStop(bool scl, bool sda)
{
	m_Cmd = STOP;
	m_Ss = m_Es = m_Samplenum;
	m_State = FIND_START;
	m_Is_repeat_start = 0;
	m_Wr = -1;
	m_Bits.clear();
}

I2CDecoder::~I2CDecoder()
{
#ifndef CLI
	CDataManager* man = CDataManager::GetInstance();
	assert(man && "null ptr");
	if (man)
	{
		man->UnRegisterSignal(m_Signal.GetName());
		man->UnRegisterParam(m_Parameters.GetName());
	}
#endif
}
