#include "spi_decoder.h"

#include <cstdio>
#include <limits>
#include <climits>
#include <memory>
#include <algorithm>
#include <cassert>

SpiDecoder::SpiDecoder(const std::string& _name)
#ifndef CLI
	:
	m_Name(_name),
	m_ParametersUpdated(false),
	m_Signal(m_Name + "_signal", 1, {0,0,0}),
	m_Parameters(m_Name + "_parameters", CBaseParameter::RW, m_Options, 0)
#endif
{

}

void SpiDecoder::ResetDecoder()
{
	m_Oldclk = 1;
	m_Bitcount = 0;
	m_Data = 0;
	m_Oldcs = -1;
	m_Oldpins = -1;
	m_Have_cs = -1;
	m_Cs_was_deasserted = -1;
	m_Samplenum = -1;
	m_OldSamplenum = 0;
	m_State = FIND_DATA;
	m_Result.clear();
}

void SpiDecoder::UpdateParameters()
{
#ifndef CLI
	m_Parameters.Update();
	SetParameters(m_Parameters.Value());
#endif
	m_ParametersUpdated = true;
}

void SpiDecoder::UpdateSignals()
{
#ifndef CLI
	m_Signal.Update();
#endif
}

void SpiDecoder::SetParameters(const SpiParameters& _new_params)
{
//FIXME mb
	m_Options = _new_params;

	--m_Options.cs;
	--m_Options.data;
	--m_Options.clk;

	assert((m_Options.cs == 255 || (m_Options.cs >= 0 && m_Options.cs <= 7)) && "incorrect cs");
	assert(m_Options.clk >= 0 && m_Options.clk <= 7 && "incorrect clk");
	assert(m_Options.data >= 0 && m_Options.data <= 7 && "incorrect data(miso/mosi)");
	assert(m_Options.cpol >= 0 && m_Options.cpol <= 1 && "incorrect cpol)");
	assert(m_Options.cpha >= 0 && m_Options.cpha <= 1 && "incorrect cpha)");
}

bool SpiDecoder::IsParametersChanged()
{
#ifndef CLI
	return (m_Parameters.IsNewValue()) || m_ParametersUpdated;
#else
    return false;
#endif // CLI
}

void SpiDecoder::Decode(const uint8_t* _input, uint32_t _size)
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
#endif
	ResetDecoder();

	m_ParametersUpdated = false;

	// Either MISO or MOSI can be omitted (but not both). CS# is optional.
	m_Have_cs = m_Options.cs != -1;
	for (size_t i = 0; i < _size; i += 2)
	{
		const uint8_t count = _input[i]; // 0 = 1 count, 1 = 2, etc.
		const uint8_t data = _input[i + 1];
		for (uint16_t j = 0; j < count + 1; ++j)
		{
			assert(m_Samplenum < std::numeric_limits<decltype(m_Samplenum)>::max() && "m_Samplenum overflow");
			++m_Samplenum;

			bool clk = (data & 1 << m_Options.clk) >> m_Options.clk;
			bool miso_mosi_data = (data & 1 << m_Options.data) >> m_Options.data;
			bool cs = (data & 1 << m_Options.cs) >> m_Options.cs;

			if (m_Options.invert_bit != 0){
				clk = !clk;
				miso_mosi_data = !miso_mosi_data;
				cs = !cs;
			}

			FindClkEdge(miso_mosi_data, clk, cs);
		}
	}

#ifndef CLI
	m_Signal.Set(std::move(m_Result));
	pthread_mutex_unlock(&m_mutex);
#endif
}

void SpiDecoder::FindClkEdge(bool data, bool clk, bool cs)
{
	if (m_Have_cs && m_Oldcs != cs)
	{
		// Send all CS# pin value changes.
		m_Oldcs = cs;

		// Reset decoder state when CS# changes (and the CS# pin is used).
		ResetDecoderState();
	}

	// We only care about samples if CS# is asserted.
	if (m_Have_cs && !CsAsserted(cs))
		return;
	// Ignore sample if the clock pin hasn't changed.
	if (clk == m_Oldclk)
		return;

	m_Oldclk = clk;

	if(m_Options.cpol == 0 && m_Options.cpha == 0 && clk == 1)
		return;
	else if(m_Options.cpol == 0 && m_Options.cpha == 1 && clk == 0)
		return;
	else if(m_Options.cpol == 1 && m_Options.cpha == 0 && clk == 0)
		return;
	else if(m_Options.cpol == 1 && m_Options.cpha == 1 && clk == 1)
		return;

	// Found the correct clock edge, now get the SPI bit(s).
	HandleBit(data, clk, cs);
}

void SpiDecoder::HandleBit(bool data, bool clk, bool cs)
{
	if (m_State == FIND_DATA)
	{
		m_StartDataSamplenum = m_Samplenum;
		m_State = COLLECT_DATA;
	}

	// Receive MISO bit into our shift register.
	assert((m_Options.word_size - 1 - m_Bitcount) <= CHAR_BIT && "long shift");
	if (m_Options.bit_order == MsbFirst)
		m_Data |= data << (m_Options.word_size - 1 - m_Bitcount);
	else
		m_Data |= data << m_Bitcount;

	m_Bitcount += 1;

	// Continue to receive if not enough bits were received, yet.
	if (m_Bitcount != m_Options.word_size)
		return;

	const auto size16 = std::numeric_limits<uint16_t>::max();
	int nothing_count = (int)m_Samplenum - (int)m_OldSamplenum - ((int)m_Samplenum - m_StartDataSamplenum);
	if (nothing_count < 0)
		nothing_count = 0;
	while (nothing_count)
	{
		m_Result.push_back(OutputPacket{NOTHING, 0, nothing_count >= size16 ? size16 : (uint16_t)nothing_count});
		nothing_count -= std::min<uint32_t>(nothing_count, size16);
	}
	m_OldSamplenum = m_Samplenum;

	const uint16_t data_len = m_Samplenum - m_StartDataSamplenum;
	m_Result.push_back({(uint8_t)0, (uint8_t)m_Data, data_len});
	m_State = FIND_DATA;

	ResetDecoderState();
}

bool SpiDecoder::CsAsserted(bool cs)
{
	return m_Options.cs_polarity == ActiveLow ? (cs == 0) : (cs == 1);
}

void SpiDecoder::ResetDecoderState()
{
	m_Data = 0;
	m_Bitcount = 0;
	m_State = FIND_DATA;
}

SpiDecoder::~SpiDecoder()
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
