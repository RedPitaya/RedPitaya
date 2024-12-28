#include "spi_decoder.h"
#include <limits>
#include <climits>
#include <cassert>
#include "rp.h"


using namespace spi;


class SPIDecoder::Impl{

	enum State
	{
		FIND_DATA,
		COLLECT_DATA
	};

	public:

	Impl();

	SPIParameters m_options;

	uint32_t m_bitcount = 0;
	uint32_t m_data = 0;
	std::string m_line = "";
	int m_oldclk = -1;
	int m_oldcs = -1;
	int m_oldpins = -1;
	bool m_have_cs = false;
	int m_cs_was_deasserted = -1;
	uint32_t m_samplenum = 0;
	uint32_t m_startSamplenum = 0;
	uint32_t m_nothing_count = 0;

	std::vector<OutputPacket> m_result;
	State m_state;

	void resetDecoder();
    void decode(const uint8_t* _input, uint32_t _size);

	bool findClkEdge(bool data, bool clk, bool cs);
	void handleBit(bool data, bool clk, bool cs);
	bool csAsserted(bool cs);
	void resetDecoderState();
};

SPIDecoder::Impl::Impl(){

}

SPIDecoder::SPIDecoder(int decoderType, const std::string& _name){
	m_decoderType = decoderType;
	m_name = _name;
	m_impl_miso = new Impl();
	m_impl_mosi = new Impl();
	m_impl_miso->m_line = "miso";
	m_impl_mosi->m_line = "mosi";
	m_impl_miso->resetDecoder();
	m_impl_mosi->resetDecoder();
	setParameters(spi::SPIParameters());
}

SPIDecoder::~SPIDecoder(){
	delete m_impl_miso;
	delete m_impl_mosi;
}

auto SPIDecoder::reset() -> void{
	m_impl_miso->resetDecoder();
	m_impl_mosi->resetDecoder();
}

auto SPIDecoder::getMemoryUsage() -> uint64_t{
	uint64_t size = sizeof(Impl);
	size += m_impl_miso->m_result.size() * sizeof(OutputPacket);
	size += m_impl_mosi->m_result.size() * sizeof(OutputPacket);
	return size;
}

void SPIDecoder::setParameters(const SPIParameters& _new_params){
	m_impl_miso->m_options = _new_params;
	m_impl_mosi->m_options = _new_params;
}

auto SPIDecoder::setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool{
	auto opt = m_impl_miso->m_options;
	if (opt.setDecoderSettingsUInt(key,value)){
		setParameters(opt);
		return true;
	}
	return false;
}

auto SPIDecoder::getDecoderSettingsUInt(std::string& key, uint32_t *value) -> bool{
	return m_impl_miso->m_options.getDecoderSettingsUInt(key,value);
}

auto SPIDecoder::getParametersInJSON() -> std::string{
	return m_impl_miso->m_options.toJson();
}

auto SPIDecoder::setParametersInJSON(const std::string &parameter) -> void{
	SPIParameters param;
	if (param.fromJson(parameter)){
		setParameters(param);
	}else{
		ERROR_LOG("Error set parameters %s", parameter.c_str())
	}
}

std::vector<OutputPacket> SPIDecoder::getSignal(){
	auto vec = m_impl_miso->m_result;
	auto vec2 = m_impl_mosi->m_result;
	vec.insert(vec.end(), vec2.begin(), vec2.end());
	return vec;
}

void SPIDecoder::Impl::resetDecoder(){
	m_oldclk = -1;
	m_bitcount = 0;
	m_data = 0;
	m_oldcs = -1;
	m_oldpins = -1;
	m_have_cs = false;
	m_cs_was_deasserted = -1;
	m_nothing_count = 0;
	m_samplenum = 0;
	// m_oldSamplenum = 0;
	m_startSamplenum = 0;
	m_state = FIND_DATA;
	m_result.clear();
}

void SPIDecoder::decode(const uint8_t* _input, uint32_t _size){
	if (m_impl_miso->m_options.m_miso != 0)
		m_impl_miso->decode(_input,_size);
	if (m_impl_mosi->m_options.m_mosi != 0)
		m_impl_mosi->decode(_input,_size);
}

void SPIDecoder::Impl::decode(const uint8_t* _input, uint32_t _size){

	if (!_input) FATAL("Input value is null")
	if (_size == 0) FATAL("Input value size == 0")
	if (_size & 0x1) FATAL("Input value is odd")

	if (m_options.m_cpol > 1) {
		ERROR_LOG("incorrect cpol")
		return;
	}

	if (m_options.m_cpha > 1) {
		ERROR_LOG("incorrect cpha")
		return;
	}

	if (m_options.m_clk == 0 || m_options.m_clk > 8){
		ERROR_LOG("CLK not specified. Valid values from 1 to 8")
		return;
	}

	if (m_options.m_cs > 8){
		ERROR_LOG("CS not specified. Valid values from 0 to 8. 0 - Disabled")
		return;
	}
	// Either MISO or MOSI can be omitted (but not both). CS# is optional.

	uint8_t data_line = 0;
	if (m_line == "miso"){
		data_line = m_options.m_miso;
	}

	if (m_line == "mosi"){
		data_line = m_options.m_mosi;
	}

	if (data_line == 0 || data_line > 7) {
		ERROR_LOG("MISO or MOSI not specified. Valid values from 1 to 8")
		return;
	}
	resetDecoder();
	resetDecoderState();
	m_have_cs = m_options.m_cs != 0;
	uint8_t clk_line = m_options.m_clk - 1;
	uint8_t cs_line = m_options.m_cs - 1;
	data_line--;
	bool initCLK_CS = false;
	for (size_t i = 0; i < _size; i += 2)
	{
		const uint8_t count = _input[i]; // 0 = 1 count, 1 = 2, etc.
		const uint8_t data = _input[i + 1];
		for (uint16_t j = 0; j < count + 1; ++j){
			assert(m_samplenum < std::numeric_limits<decltype(m_samplenum)>::max() && "m_samplenum overflow");

			bool clk = (data & 1 << clk_line);
			bool miso_mosi_data = (data & 1 << data_line);
			bool cs = (data & 1 << cs_line);

			if (m_options.m_invert_bit != 0){
				clk = !clk;
				miso_mosi_data = !miso_mosi_data;
				cs = !cs;
			}

			if (!initCLK_CS){
				m_oldclk = clk;
				m_oldcs = cs;
				initCLK_CS = true;
			}

			if (!findClkEdge(miso_mosi_data, clk, cs)){
				m_nothing_count++;
			}
			++m_samplenum;
		}
	}
}

bool SPIDecoder::Impl::findClkEdge(bool data, bool clk, bool cs){
	if (m_have_cs && m_oldcs != cs){
		// Send all CS# pin value changes.
		m_oldcs = cs;
		// Reset decoder state when CS# changes (and the CS# pin is used).
		resetDecoderState();
	}

	// We only care about samples if CS# is asserted.

	if (m_have_cs && !csAsserted(cs))
		return false;
	// Ignore sample if the clock pin hasn't changed.
	if (clk == m_oldclk)
		return false;

	m_oldclk = clk;

	if(m_options.m_cpol == 0 && m_options.m_cpha == 1 && clk == 1)
		return false;
	else if(m_options.m_cpol == 0 && m_options.m_cpha == 0 && clk == 0)
		return false;
	else if(m_options.m_cpol == 1 && m_options.m_cpha == 1 && clk == 0)
		return false;
	else if(m_options.m_cpol == 1 && m_options.m_cpha == 0 && clk == 1)
		return false;
	TRACE_SHORT("State %d m_nothing_count %d", m_state,m_nothing_count)
	if (m_nothing_count > 0 && m_state == FIND_DATA){
		// m_result.push_back(OutputPacket{m_line , NOTHING, 0, m_nothing_count, 0 , m_startSamplenum});
		m_nothing_count = 0;
	}

	// Found the correct clock edge, now get the SPI bit(s).
	handleBit(data, clk, cs);
	return true;
}

void SPIDecoder::Impl::handleBit(bool data, bool clk, bool cs)
{
	if (m_state == FIND_DATA){
	 	m_startSamplenum = m_samplenum;
		m_state = COLLECT_DATA;
	}

	// Receive MISO bit into our shift register.
	assert((m_options.m_word_size - 1 - m_bitcount) <= CHAR_BIT && "long shift");

    if(m_options.m_bit_order == MsbFirst){
        m_data <<= 1;
        m_data |= (data << 0);
    }
    else if(m_options.m_bit_order == LsbFirst){
        m_data >>= 1;
        m_data |= (data << (m_options.m_word_size - 1));
    }

	m_bitcount += 1;
	TRACE_SHORT("Data %d bit %d m_bitcount %d sam %d sstart %d", m_data, data ,m_bitcount,m_samplenum,m_startSamplenum)
	// Continue to receive if not enough bits were received, yet.
	if (m_bitcount != m_options.m_word_size)
		return;

	// m_oldSamplenum = m_samplenum;

	auto data_len = m_samplenum - m_startSamplenum;
	m_result.push_back(OutputPacket{m_line , DATA, m_data, data_len, (float)m_bitcount, m_startSamplenum});
	resetDecoderState();
}

bool SPIDecoder::Impl::csAsserted(bool cs){
	return m_options.m_cs_polarity == ActiveLow ? (cs == 0) : (cs == 1);
}

void SPIDecoder::Impl::resetDecoderState(){
	m_data = 0;
	m_bitcount = 0;
	m_state = FIND_DATA;
}
