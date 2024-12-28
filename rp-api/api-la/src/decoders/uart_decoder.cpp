#include "uart_decoder.h"

#include <limits>
#include <cassert>
#include <math.h>
#include "rp.h"


using namespace uart;


class UARTDecoder::Impl{

	enum State
	{
		WAIT_FOR_START_BIT,
		GET_START_BIT,
		GET_DATA_BITS,
		GET_PARITY_BIT,
		GET_STOP_BITS
	};

	public:

	Impl();

    bool        m_isEnd;
    bool        m_oldBit;
    uint32_t    m_prevBitStart;
    uint32_t    m_startDataBits;
    uint8_t     m_curDataBit;
    uint16_t    m_dataByte;
    uint8_t     m_parityBit;
    bool        m_parityOk;
	uint32_t    m_samplenum;
	uint32_t    m_silenceLength;
    uint16_t    m_bitAccumulate;

	double      m_bitWidth;

	State       m_state;

	UARTParameters m_options;

    std::string m_line;

	std::string m_name;
	std::vector<OutputPacket> m_result;

    void resetDecoder();
    void decode(const uint8_t* _input, uint32_t _size);

	void waitForStartBit(bool bit, uint32_t sampleNum);
    void getStartBit(bool bit, uint32_t sampleNum);
    void getDataBits(bool bit, uint32_t sampleNum);
    void getParityBit(bool bit, uint32_t sampleNum);
    void getStopBits(bool bit, uint32_t sampleNum);
    bool parityOk();
};

UARTDecoder::Impl::Impl(){

}

UARTDecoder::UARTDecoder(int decoderType, const std::string& _name){
    m_decoderType = decoderType;
	m_name = _name;
	m_impl_rx = new Impl();
	m_impl_tx = new Impl();
    m_impl_rx->m_line = "rx";
    m_impl_tx->m_line = "tx";
	m_impl_rx->resetDecoder();
	m_impl_tx->resetDecoder();
    setParameters(uart::UARTParameters());
}

UARTDecoder::~UARTDecoder(){
	delete m_impl_rx;
	delete m_impl_tx;
}

auto UARTDecoder::reset() -> void{
	m_impl_tx->resetDecoder();
	m_impl_rx->resetDecoder();
}

auto UARTDecoder::getMemoryUsage() -> uint64_t{
	uint64_t size = sizeof(Impl);
	size += m_impl_rx->m_result.size() * sizeof(OutputPacket);
	size += m_impl_tx->m_result.size() * sizeof(OutputPacket);
	return size;
}

void UARTDecoder::setParameters(const UARTParameters& _new_params)
{
    if (_new_params.m_baudrate == 0) FATAL("baudrate should not be equal to 0")
	m_impl_rx->m_options = _new_params;
	m_impl_tx->m_options = _new_params;
    m_impl_rx->m_bitWidth = (double)_new_params.m_samplerate / (double)_new_params.m_baudrate;
    m_impl_tx->m_bitWidth = (double)_new_params.m_samplerate / (double)_new_params.m_baudrate;
}

auto UARTDecoder::setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool{
	auto opt_tx = m_impl_tx->m_options;
    auto opt_rx = m_impl_rx->m_options;
	if (opt_tx.setDecoderSettingsUInt(key,value) && opt_rx.setDecoderSettingsUInt(key,value)){
		setParameters(opt_tx);
		return true;
	}
	return false;
}

auto UARTDecoder::getDecoderSettingsUInt(std::string& key, uint32_t *value) -> bool{
	return m_impl_tx->m_options.getDecoderSettingsUInt(key,value);
}

auto UARTDecoder::getParametersInJSON() -> std::string{
	return m_impl_rx->m_options.toJson();
}

auto UARTDecoder::setParametersInJSON(const std::string &parameter) -> void{
	UARTParameters param;
	if (param.fromJson(parameter)){
		setParameters(param);
	}else{
		ERROR_LOG("Error set parameters %s", parameter.c_str())
	}
}

std::vector<OutputPacket> UARTDecoder::getSignal(){
	auto vec = m_impl_rx->m_result;
	auto vec2 = m_impl_tx->m_result;
	vec.insert(vec.end(), vec2.begin(), vec2.end());
	return vec;
}

void UARTDecoder::Impl::resetDecoder()
{
    m_samplenum = 0;
    m_state = WAIT_FOR_START_BIT;
    m_startDataBits = 0;
    m_prevBitStart = 0;
    m_curDataBit = 0;
    m_dataByte = 0;
    m_parityBit = 0;
    m_parityOk = false;
    m_oldBit = 1;
    m_silenceLength = 0;
    m_bitAccumulate = 0;
    m_isEnd = 0;
	m_result.clear();
}

void UARTDecoder::decode(const uint8_t* _input, uint32_t _size)
{
    if (m_impl_rx->m_options.m_rx != 0)
        m_impl_rx->decode(_input,_size);
    if (m_impl_tx->m_options.m_tx != 0)
        m_impl_tx->decode(_input,_size);
}

void UARTDecoder::Impl::decode(const uint8_t* _input, uint32_t _size)
{
	if (!_input) FATAL("Input value is null")
	if (_size == 0) FATAL("Input value size == 0")
	if (_size & 0x1) FATAL("Input value is odd")
	if (m_options.m_rx > 8) {
        ERROR_LOG("RX not specified. Valid values from 0 to 8")
        return;
    }

	if (m_options.m_tx > 8) {
        ERROR_LOG("TX not specified. Valid values from 0 to 8")
        return;
    }

	if (m_options.m_rx == 0 && m_options.m_tx == 0) {
        ERROR_LOG("RX or TX not specified. Valid values from 0 to 8")
        return;
    }

    resetDecoder();

    uint32_t savedSampleNum = 0;

    uint8_t rx_line = 0;

    if (m_line == "rx")
        rx_line = m_options.m_rx;

    if (m_line == "tx")
        rx_line = m_options.m_tx;

    rx_line--;

    for (uint32_t i = 0; i < _size; i += 2)
    {
        // Read count and data for decode RLE
        const uint8_t count = _input[i];
        const uint8_t data = _input[i + 1];



        for(uint16_t j = 0; j < count + 1; ++j)
        {
            bool newBit = data & (1 << (rx_line));
            m_isEnd = (i + 2 >= _size) && j == count;
            uint32_t sampleNum = savedSampleNum;
            assert(m_samplenum < std::numeric_limits<decltype(m_samplenum)>::max() && "m_samplenum overflow");
            savedSampleNum++;

            if(m_options.m_invert)
                newBit = !newBit;

            // State machine for RX line
            if(m_state == WAIT_FOR_START_BIT)
                waitForStartBit(newBit, sampleNum);
            else if(m_state == GET_START_BIT)
                getStartBit(newBit, sampleNum);
            else if(m_state == GET_DATA_BITS)
                getDataBits(newBit, sampleNum);
            else if(m_state == GET_PARITY_BIT)
                getParityBit(newBit, sampleNum);
            else if(m_state == GET_STOP_BITS)
                getStopBits(newBit, sampleNum);
            // Save current values of lines for next round
            m_oldBit = newBit;
        }
    }
}

void UARTDecoder::Impl::waitForStartBit(bool bit, uint32_t sampleNum)
{
    // The start bit is always 0 (low). As the idle UART (and the stop bit)
    // level is 1 (high), the beginning of a start bit is a falling edge.
    if(bit != 0){
        // Silence length calculating and return.
        // Write silence length to output if need.
        m_silenceLength += 1;
        if(m_silenceLength == 0x7FFFFFFF){
            //m_result.push_back({m_line ,NOTHING, 0, m_silenceLength, 0, sampleNum - m_silenceLength});
            m_silenceLength = 0;
        }
        return;
    }

    if(m_silenceLength != 0){
        //m_result.push_back({m_line ,NOTHING, 0, m_silenceLength, 0 , sampleNum - m_silenceLength});
        m_silenceLength = 0;
    }

    // Save the sample number where the start bit begins.
    m_prevBitStart = sampleNum;
    m_silenceLength = 0;
    m_bitAccumulate = 0;
    m_startDataBits = 0;
    m_state = GET_START_BIT;
}

void UARTDecoder::Impl::getStartBit(bool bit, uint32_t sampleNum)
{
    m_bitAccumulate += bit;
    if (m_oldBit == bit){
        if (((double)sampleNum < m_bitWidth + (double)m_prevBitStart) && !m_isEnd){
            return;
        }
    }else{
        m_bitAccumulate -= bit;
    }

    int bitValue = round((float)m_bitAccumulate / (float)(sampleNum - m_prevBitStart));
    // The startbit must be 0. If not, we report an error.
    if(bitValue != 0){
        // START-bit error
        m_result.push_back(OutputPacket{m_line ,START_BIT_ERR, 0, sampleNum - m_prevBitStart, 0, m_prevBitStart});
        m_state = WAIT_FOR_START_BIT;
        return;
    }

    m_result.push_back(OutputPacket{m_line ,START_BIT, 0, sampleNum - m_prevBitStart, 1, m_prevBitStart});
    m_curDataBit = 0;
    m_dataByte = 0;
    m_bitAccumulate = 0;
    m_prevBitStart = sampleNum;
    m_startDataBits = sampleNum;
    m_state = GET_DATA_BITS;
}

void UARTDecoder::Impl::getDataBits(bool bit, uint32_t sampleNum)
{
    m_bitAccumulate += bit;
    if (m_oldBit == bit){
        if (((double)sampleNum < m_bitWidth + (double)m_prevBitStart) && !m_isEnd){
            return;
        }
    }else{
        m_bitAccumulate -= bit;
    }

    int bitValue = round((float)m_bitAccumulate / (float)(sampleNum - m_prevBitStart));

    if(m_options.m_bitOrder == MSB_FIRST){
        m_dataByte <<= 1;
        m_dataByte |= (bitValue << 0);
    }
    else if(m_options.m_bitOrder == LSB_FIRST){
        m_dataByte >>= 1;
        m_dataByte |= (bitValue << (m_options.m_num_data_bits - 1));
    }

    // Return here, unless we already received all data bits.
    if(m_curDataBit < (m_options.m_num_data_bits - 1)){
        m_bitAccumulate = 0;
        m_curDataBit++;
        m_prevBitStart = sampleNum;
        return;
    }

    m_result.push_back({m_line, DATA, m_dataByte, sampleNum - m_startDataBits, (float)m_curDataBit + 1, m_startDataBits});
    m_bitAccumulate = 0;
    m_prevBitStart = sampleNum;

    if(m_options.m_parity == NONE)
        m_state = GET_STOP_BITS;
    else
        m_state = GET_PARITY_BIT;
}

void UARTDecoder::Impl::getParityBit(bool bit, uint32_t sampleNum){

    m_bitAccumulate += bit;
    if (m_oldBit == bit){
        if (((double)sampleNum < m_bitWidth + (double)m_prevBitStart) && !m_isEnd){
            return;
        }
    }else{
        m_bitAccumulate -= bit;
    }

    int bitValue = round((float)m_bitAccumulate / (float)(sampleNum - m_prevBitStart));
    m_parityBit = bitValue;
    m_parityOk = parityOk();
    if (m_parityOk)
        m_result.push_back(OutputPacket{m_line, PARITY_BIT, 0, sampleNum - m_prevBitStart, 1, m_prevBitStart});
    else
        m_result.push_back(OutputPacket{m_line, PARITY_ERR, 0, sampleNum - m_prevBitStart, 1, m_prevBitStart});
    m_bitAccumulate = 0;
    m_prevBitStart = sampleNum;
    m_state = GET_STOP_BITS;
}

void UARTDecoder::Impl::getStopBits(bool bit, uint32_t sampleNum){
    State nextState = WAIT_FOR_START_BIT;
    float bitWait = 1;

    switch (m_options.m_num_stop_bits)
    {
        case STOP_BITS_05:
            bitWait = 0.5;
        break;

        case STOP_BITS_10:
            bitWait = 1;
        break;

        case STOP_BITS_15:
            bitWait = 1.5;
        break;

        case STOP_BITS_20:
            bitWait = 2;
        break;

        case STOP_BITS_NO:
            m_prevBitStart = sampleNum;
            return;

    default:
        FATAL("Unknown mode %d",m_options.m_num_stop_bits)
    }


    m_bitAccumulate += bit;
    if (m_oldBit == bit){
        if (((double)sampleNum < m_bitWidth * bitWait + (double)m_prevBitStart) && !m_isEnd){
            return;
        }
    }else{
        m_bitAccumulate -= bit;
    }

    int bitValue = round((float)m_bitAccumulate / (float)(sampleNum - m_prevBitStart));

    if(bitValue != 1){
        // STOP-bit error
        m_result.push_back({m_line, STOP_BIT_ERR, 0, sampleNum - m_prevBitStart, 1, m_prevBitStart});
        m_state = WAIT_FOR_START_BIT;
        m_bitAccumulate = 0;
        m_prevBitStart = sampleNum;
        return;
    }


    m_result.push_back({m_line, STOP_BIT, 0, sampleNum - m_prevBitStart, bitWait, m_prevBitStart});
    m_state = nextState;
    m_prevBitStart = sampleNum;
}

bool UARTDecoder::Impl::parityOk(){
    int sumOfBits = 0;

    for(int i = 0; i < m_options.m_num_data_bits; i++)
        sumOfBits += (int)((m_dataByte >> i) & 0x01);

    sumOfBits += m_parityBit;

    switch (m_options.m_parity )
    {
        case ODD:
            return (sumOfBits % 2) == 1;
        case EVEN:
            return (sumOfBits % 2) == 0;
        case ALWAYS_0:
            return m_parityBit == 0;
        case ALWAYS_1:
            return m_parityBit == 1;
    default:
        ERROR_LOG("Unknown mode parity %d",m_options.m_parity)
        break;
    }
    return false;
}
