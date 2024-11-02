#include "uart_decoder.h"

#include <limits>
#include <cassert>
#include "rp.h"


using namespace uart;


class UARTDecoder::Impl{

	enum State
	{
		WAIT_FOR_START_BIT,
		GET_START_BIT,
		GET_DATA_BITS,
		GET_PARITY_BIT,
		GET_STOP_BITS,
		WAIT_END_OF_HALF_STOP,
		WAIT_END_OF_FULL_STOP
	};

	public:

	Impl();


    bool        m_oldBit;
    uint32_t    m_frameStart;
    uint32_t    m_frameStop;
    uint8_t     m_startBit;
    uint8_t     m_curDataBit;
    uint16_t    m_dataByte;
    uint8_t     m_parityBit;
    bool        m_parityOk;
    uint8_t     m_stopBit1;
    uint8_t     m_stopBit2;
	uint32_t    m_samplenum;
	uint16_t    m_silenceLength;

	bool        m_stopBit1_Get;

	float       m_bitWidth;
	uint16_t    m_countOfStop;

	State       m_state;

	UARTParameters m_options;

	std::string m_name;
	std::vector<OutputPacket> m_result;

    void resetDecoder();
    void decode(const uint8_t* _input, uint32_t _size);

	void waitForStartBit(bool bit, uint32_t sampleNum);
    void getStartBit(bool bit, uint32_t sampleNum);
    void getDataBits(bool bit, uint32_t sampleNum);
    void getParityBit(bool bit, uint32_t sampleNum);
    void getStopBits(bool bit, uint32_t sampleNum);
    bool bitRiched(uint8_t bitNum, uint32_t sampleNum);
    bool parityOk();
	bool stopBit05Reached(uint8_t bitNum, uint32_t sampleNum);
	bool stopBit10Reached(uint8_t bitNum, uint32_t sampleNum);
	void waitEndOfStop(uint8_t bitNum, uint32_t sampleNum);
};

UARTDecoder::Impl::Impl(){

}

UARTDecoder::UARTDecoder(const std::string& _name){
	m_impl = new Impl();
	m_impl->m_name = _name;
	m_impl->resetDecoder();
}

UARTDecoder::~UARTDecoder(){
	delete m_impl;
}

void UARTDecoder::setParameters(const UARTParameters& _new_params)
{
    if (_new_params.m_baudrate == 0) FATAL("baudrate should not be equal to 0")
	m_impl->m_options = _new_params;
    m_impl->m_bitWidth = (float)_new_params.m_samplerate / (float)_new_params.m_baudrate;
    m_impl->m_countOfStop = 0;
}

std::vector<OutputPacket> UARTDecoder::getSignal(){
	return m_impl->m_result;
}

void UARTDecoder::Impl::resetDecoder()
{
    m_samplenum = 0;
    //m_bitWidth = 0; Do not reset this in non-constructor methods
    m_state = WAIT_FOR_START_BIT;
    m_frameStart = 0;
    m_startBit = 0;
    m_curDataBit = 0;
    m_dataByte = 0;
    m_parityBit = 0;
    m_parityOk = false;
    m_stopBit1 = 0;
    m_stopBit2 = 0;
    m_oldBit = 1;
    m_silenceLength = 0;
    m_stopBit1_Get = false;

	m_result.clear();
}

void UARTDecoder::decode(const uint8_t* _input, uint32_t _size)
{
    m_impl->decode(_input,_size);
}

void UARTDecoder::Impl::decode(const uint8_t* _input, uint32_t _size)
{
	assert(_input && "null ptr");
	assert(_size > 0 && "input vector size == 0");
	assert(_size % 2 == 0 && "odd size");
	if (m_options.m_rx > 8) FATAL("RX line more than 8")
	if (m_options.m_rx == 0) FATAL("RX line should not be equal to 0")

    resetDecoder();

    uint32_t savedSampleNum = 0;

    for (uint32_t i = 0; i < _size; i += 2)
    {
        // Read count and data for decode RLE
        const uint8_t count = _input[i];
        const uint8_t data = _input[i + 1];

        uint8_t rx_line = m_options.m_rx - 1;

        for(uint16_t j = 0; j < count + 1; ++j)
        {
            bool newBit = data & (1 << (rx_line));
            uint32_t sampleNum = savedSampleNum;
            assert(m_samplenum < std::numeric_limits<decltype(m_samplenum)>::max() && "m_samplenum overflow");
            savedSampleNum += 1;

            if(m_options.m_invert_rx)
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
            else if(m_state == WAIT_END_OF_FULL_STOP || m_state == WAIT_END_OF_HALF_STOP)
            	waitEndOfStop(newBit, sampleNum);

            // Save current values of lines for next round
            m_oldBit = newBit;
        }
    }

    for(uint32_t i=0; i < m_result.size(); i++)
    {
//        if(m_Result[i].control == (1 << 5)) // Start-bit
//            m_Result[i].length -= 2;
        /*else*/
        if(m_result[i].control == (1 << 6)) // Stop-bit
            m_result[i].length -= 1;
        else if(m_result[i].control == (1 << 4)) // Nothing on line
            m_result[i].length -= 1;
    }
}

void UARTDecoder::Impl::waitForStartBit(bool bit, uint32_t sampleNum)
{
    // The start bit is always 0 (low). As the idle UART (and the stop bit)
    // level is 1 (high), the beginning of a start bit is a falling edge.
    if(!(m_oldBit == 1 && bit == 0))
    {
        // Silence length calculating and return.
        // Write silence length to output if need.
        m_silenceLength += 1;

        if(m_silenceLength == 0xFFFF)
        {
            uint8_t controlByteInOutput = NO;
            controlByteInOutput = (1 << 4);
            m_result.push_back(OutputPacket{controlByteInOutput, 0, m_silenceLength});
            m_silenceLength = 0;
        }

        return;
    }

    if(m_silenceLength != 0)
    {
        uint8_t controlByteInOutput = NO;
        controlByteInOutput = (1 << 4);
        m_result.push_back(OutputPacket{controlByteInOutput, 0, m_silenceLength});
        m_silenceLength = 0;
    }

    // Save the sample number where the start bit begins.
    m_frameStart = sampleNum;
    m_stopBit1_Get = false;
    m_silenceLength = 0;

    m_state = GET_START_BIT;
}

void UARTDecoder::Impl::getStartBit(bool bit, uint32_t sampleNum)
{
    uint8_t controlByteInOutput = 0;
    // Skip samples until we're in the middle of the start bit.
    if(!bitRiched(0, sampleNum))
        return;

    m_startBit = bit;

    // The startbit must be 0. If not, we report an error.
    if(m_startBit != 0)
    {
        // START-bit error
        controlByteInOutput |= (1 << 2);
    }

    m_curDataBit = 0;
    m_dataByte = 0;

    controlByteInOutput |= (1 << 5);
    m_result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length

    m_state = GET_DATA_BITS;
}

void UARTDecoder::Impl::getDataBits(bool bit, uint32_t sampleNum)
{
    // Skip samples until we're in the middle of the start bit.
    if(!bitRiched(m_curDataBit + 1, sampleNum))
        return;

    if(m_options.m_bitOrder == MSB_FIRST)
    {
        m_dataByte <<= 1;
        m_dataByte |= (bit << 0);
    }
    else if(m_options.m_bitOrder == LSB_FIRST)
    {
        m_dataByte >>= 1;
        m_dataByte |= (bit << (m_options.m_num_data_bits - 1));
    }

    // Return here, unless we already received all data bits.
    if(m_curDataBit < (m_options.m_num_data_bits - 1))
    {
        m_curDataBit += 1;
        return;
    }

    if(m_options.m_parity == NONE)
        m_state = GET_STOP_BITS;
    else
        m_state = GET_PARITY_BIT;
}

void UARTDecoder::Impl::getParityBit(bool bit, uint32_t sampleNum)
{
    //uint8_t controlByteInOutput = 0;
    // If no parity is used/configured, skip to the next state immediately.
    if(m_options.m_parity == NONE)
    {
        m_state = GET_STOP_BITS;
        return;
    }

    // Skip samples until we're in the middle of the parity bit.
    if(!bitRiched(m_options.m_num_data_bits + 1, sampleNum))
        return;

    m_parityBit = bit;

    m_state = GET_STOP_BITS;

    m_parityOk = parityOk();

    /*controlByteInOutput |= (1 << 7);
    if(!m_ParityOk)
        controlByteInOutput |= (1 << 1);

    m_Result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length*/
}

void UARTDecoder::Impl::waitEndOfStop(uint8_t bitNum, uint32_t sampleNum)
{
	m_state = WAIT_FOR_START_BIT;
	return;

	m_countOfStop++;
	float divider = 2;

	if(m_state == WAIT_END_OF_HALF_STOP)
		divider = 4;
	else if(m_state == WAIT_END_OF_FULL_STOP)
		divider = 2;

	if(m_countOfStop == (uint16_t)(m_bitWidth/divider))
	{
		m_state = WAIT_FOR_START_BIT;
		m_countOfStop = 0;
	}
}

void UARTDecoder::Impl::getStopBits(bool bit, uint32_t sampleNum)
{
    // Skip samples until we're in the middle of the stop bit(s).
    uint8_t skip_parity = (m_options.m_parity == NONE) ? 0 : 1;
    uint8_t bitNum = m_options.m_num_data_bits + 1 + skip_parity;
    uint8_t controlByteInOutput = 0;
    uint8_t controlDataWriting = 0;
    uint16_t length = 0;

    // Get 0.5 Stop Bit
    if(m_options.m_num_stop_bits == STOP_BITS_05)
    {
        if(!stopBit05Reached(bitNum, sampleNum))
            return;

        m_stopBit1 = bit;

        if(m_stopBit1 != 1)
        {
            // STOP-bit error
            controlByteInOutput |= (1 << 3);
        }

        //m_State = WAIT_FOR_START_BIT;
        m_state = WAIT_END_OF_HALF_STOP;

        m_frameStop = sampleNum;

        // After STOP bits receiving we save data.
        if(m_options.m_num_data_bits == DATA_BITS_9)
            controlDataWriting = ((m_dataByte >> 8) & 0x01) | 0x80;  // 0x80 means that 9 bit data received
        length = m_options.m_num_data_bits * (uint16_t)m_bitWidth; // TODO: Data writing need to rewrite
        m_result.push_back(OutputPacket{controlDataWriting, (uint8_t)m_dataByte, length});

        if(m_options.m_parity != NONE)
        {
	    	controlByteInOutput |= (1 << 7);
	    	if(!m_parityOk)
	    	    controlByteInOutput |= (1 << 1);
	    	m_result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
    	}

        controlByteInOutput |= (1 << 6);
        m_result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)(m_bitWidth/2)}); // TODO: Calculate length
    }

    // Get 1.0 Stop Bit
    else if(m_options.m_num_stop_bits == STOP_BITS_10)
    {
        if(!stopBit10Reached(bitNum, sampleNum))
            return;

        m_stopBit1 = bit;

        if(m_stopBit1 != 1)
        {
            // STOP-bit error
            controlByteInOutput |= (1 << 3);
        }

        //m_State = WAIT_FOR_START_BIT;
        m_state = WAIT_END_OF_FULL_STOP;

        m_frameStop = sampleNum;

        // After STOP bits receiving we save data.
        if(m_options.m_num_data_bits == DATA_BITS_9)
            controlDataWriting = ((m_dataByte >> 8) & 0x01) | 0x80;  // 0x80 means that 9 bit data received
        length = m_options.m_num_data_bits * (uint16_t)m_bitWidth; // TODO: Data writing need to rewrite
        m_result.push_back(OutputPacket{controlDataWriting, (uint8_t)m_dataByte, length});

        if(m_options.m_parity != NONE)
        {
	    	controlByteInOutput |= (1 << 7);
	    	if(!m_parityOk)
	    	    controlByteInOutput |= (1 << 1);
	    	m_result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
    	}

        controlByteInOutput |= (1 << 6);
        m_result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
    }

    // Get 1.5 Stop Bits
    else if(m_options.m_num_stop_bits == STOP_BITS_15)
    {
        // Get first full stop-bit
        if(!m_stopBit1_Get)
        {
            if(!stopBit10Reached(bitNum, sampleNum))
                return;

            m_stopBit1 = bit;
            m_stopBit1_Get = true;

            if(m_stopBit1 != 1)
            {
                // STOP-bit error
                controlByteInOutput |= (1 << 3);

                m_state = WAIT_END_OF_HALF_STOP;

                m_frameStop = sampleNum;

                // After STOP bits receiving we save data.
                if(m_options.m_num_data_bits == DATA_BITS_9)
                    controlDataWriting = ((m_dataByte >> 8) & 0x01) | 0x80;  // 0x80 means that 9 bit data received
                length = m_options.m_num_data_bits * (uint16_t)m_bitWidth; // TODO: Data writing need to rewrite
                m_result.push_back(OutputPacket{controlDataWriting, (uint8_t)m_dataByte, length});

		        if(m_options.m_parity != NONE){
			    	controlByteInOutput |= (1 << 7);
			    	if(!m_parityOk)
			    	    controlByteInOutput |= (1 << 1);
			    	m_result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
		    	}

                controlByteInOutput |= (1 << 6);
                m_result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
            }
        }
        // Get second half stop-bit
        else
        {
            if(!stopBit05Reached(bitNum+1, sampleNum))
                return;

            m_stopBit2 = bit;

            if(m_stopBit2 != 1)
            {
                // STOP-bit error
                controlByteInOutput |= (1 << 3);
            }

            //m_State = WAIT_FOR_START_BIT;
        	m_state = WAIT_END_OF_HALF_STOP;

            m_frameStop = sampleNum;

            // After STOP bits receiving we save data.
            if(m_options.m_num_data_bits == DATA_BITS_9)
                controlDataWriting = ((m_dataByte >> 8) & 0x01) | 0x80;  // 0x80 means that 9 bit data received
            length = m_options.m_num_data_bits * (uint16_t)m_bitWidth; // TODO: Data writing need to rewrite
            m_result.push_back(OutputPacket{controlDataWriting, (uint8_t)m_dataByte, length});

	        if(m_options.m_parity != NONE){
		    	controlByteInOutput |= (1 << 7);
		    	if(!m_parityOk)
		    	    controlByteInOutput |= (1 << 1);
		    	m_result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
	    	}

            controlByteInOutput |= (1 << 6);
            m_result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)(m_bitWidth/2)}); // TODO: Calculate length
        }
    }

    // Get 2.0 Stop Bits
    else if(m_options.m_num_stop_bits == STOP_BITS_20)
    {
        // Get first full stop-bit
        if(!m_stopBit1_Get){
            if(!stopBit10Reached(bitNum, sampleNum))
                return;

            m_stopBit1 = bit;
            m_stopBit1_Get = true;

            if(m_stopBit1 != 1)
            {
                // STOP-bit error
                controlByteInOutput |= (1 << 3);

                m_state = WAIT_END_OF_FULL_STOP;

                m_frameStop = sampleNum;

                // After STOP bits receiving we save data.
                if(m_options.m_num_data_bits == DATA_BITS_9)
                    controlDataWriting = ((m_dataByte >> 8) & 0x01) | 0x80;  // 0x80 means that 9 bit data received
                length = m_options.m_num_data_bits * (uint16_t)m_bitWidth; // TODO: Data writing need to rewrite
                m_result.push_back(OutputPacket{controlDataWriting, (uint8_t)m_dataByte, length});

		        if(m_options.m_parity != NONE){
			    	controlByteInOutput |= (1 << 7);
			    	if(!m_parityOk)
			    	    controlByteInOutput |= (1 << 1);
			    	m_result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
		    	}

                controlByteInOutput |= (1 << 6);
                m_result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
            }
        }
        // Get second full stop-bit
        else
        {
            if(!stopBit10Reached(bitNum+1, sampleNum))
                return;

            m_stopBit2 = bit;

            if(m_stopBit2 != 1){
                // STOP-bit error
                controlByteInOutput |= (1 << 3);
            }

            //m_State = WAIT_FOR_START_BIT;
            m_state = WAIT_END_OF_FULL_STOP;

            m_frameStop = sampleNum;

            // After STOP bits receiving we save data.
            if(m_options.m_num_data_bits == DATA_BITS_9)
                controlDataWriting = ((m_dataByte >> 8) & 0x01) | 0x80;  // 0x80 means that 9 bit data received
            length = m_options.m_num_data_bits * (uint16_t)m_bitWidth; // TODO: Data writing need to rewrite
            m_result.push_back(OutputPacket{controlDataWriting, (uint8_t)m_dataByte, length});

	        if(m_options.m_parity != NONE)
	        {
		    	controlByteInOutput |= (1 << 7);
		    	if(!m_parityOk)
		    	    controlByteInOutput |= (1 << 1);
		    	m_result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
	    	}

            controlByteInOutput |= (1 << 6);
            m_result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
        }
    }

    // No STOP bits
    else if(m_options.m_num_stop_bits == STOP_BITS_NO)
    {
    	m_state = WAIT_FOR_START_BIT;
        m_frameStop = sampleNum;

        // Save data.
        if(m_options.m_num_data_bits == DATA_BITS_9)
            controlDataWriting = ((m_dataByte >> 8) & 0x01) | 0x80;  // 0x80 means that 9 bit data received
        length = m_options.m_num_data_bits * (uint16_t)m_bitWidth; // TODO: Data writing need to rewrite
        m_result.push_back(OutputPacket{controlDataWriting, (uint8_t)m_dataByte, length});

        if(m_options.m_parity != NONE)
        {
	    	controlByteInOutput |= (1 << 7);
	    	if(!m_parityOk)
	    	    controlByteInOutput |= (1 << 1);
	    	m_result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
    	}
    }
}

bool UARTDecoder::Impl::stopBit05Reached(uint8_t bitNum, uint32_t sampleNum){
    // Return true if we reached the middle of the desired bit, false otherwise.
    uint32_t bitPos = m_frameStart + (m_bitWidth - 1) / 4;
    bitPos += bitNum * m_bitWidth;

    if(sampleNum >= bitPos)
        return true;

    return false;
}

bool UARTDecoder::Impl::stopBit10Reached(uint8_t bitNum, uint32_t sampleNum){
    // Return true if we reached the middle of the desired bit, false otherwise.
    uint32_t bitPos = m_frameStart + (m_bitWidth - 1) / 2;
    bitPos += bitNum * m_bitWidth;

    if(sampleNum >= bitPos)
        return true;

    return false;
}

bool UARTDecoder::Impl::bitRiched(uint8_t bitNum, uint32_t sampleNum){
    // Return true if we reached the middle of the desired bit, false otherwise.
    uint32_t bitPos = m_frameStart + (m_bitWidth - 1) / 2;
    bitPos += bitNum * m_bitWidth;

    if(sampleNum >= bitPos)
        return true;

    return false;
}

bool UARTDecoder::Impl::parityOk(){
    int sumOfBits = 0;

    for(int i = 0; i < m_options.m_num_data_bits; i++)
        sumOfBits += (int)((m_dataByte >> i) & 0x01);

    sumOfBits += m_parityBit;

    if(m_options.m_parity == ODD)
        return (sumOfBits % 2) == 1;
    else if(m_options.m_parity == EVEN)
        return (sumOfBits % 2) == 0;

    return false;
}
