#include "uart_decoder.h"

#include <initializer_list>
#include <string>
#include <cstdio>
#include <cassert>
#include <limits>
#include <climits>


UARTDecoder::UARTDecoder(const std::string& _name)
#ifndef CLI
:
	m_Name(_name),
    m_ParametersUpdated(false),
	m_Signal(m_Name + "_signal", 1, {0, 0, 0}),
	m_Parameters(m_Name + "_parameters", CBaseParameter::RW, m_Options, 0)
#endif
{
	ResetDecoder();
}

void UARTDecoder::ResetDecoder()
{
    m_Samplenum = 0;
    //m_bitWidth = 0; Do not reset this in non-constructor methods
    m_State = WAIT_FOR_START_BIT;
    m_FrameStart = 0;
    m_StartBit = 0;
    m_CurDataBit = 0;
    m_DataByte = 0;
    m_ParityBit = 0;
    m_ParityOk = false;
    m_StopBit1 = 0;
    m_StopBit2 = 0;
    m_OldBit = 1;
    m_SilenceLength = 0;
    m_StopBit1_Get = false;

	m_Result.clear();
}

void UARTDecoder::UpdateParameters()
{
#ifndef CLI
    m_Parameters.Update();
    SetParameters(m_Parameters.Value());
#endif
    m_ParametersUpdated = true;
}

void UARTDecoder::UpdateSignals()
{
#ifndef CLI
    m_Signal.Update();
#endif
}

void UARTDecoder::SetParameters(const UARTParameters& _new_params)
{
    m_Options = _new_params;
    m_bitWidth = (float)m_Options.samplerate / (float)m_Options.baudrate;
    m_CountOfStop = 0;
}

bool UARTDecoder::IsParametersChanged()
{
#ifndef CLI
    return (m_Parameters.IsNewValue()) || m_ParametersUpdated;
#else
    return false;
#endif // CLI
}

UARTParameters UARTDecoder::GetParameters()
{
	return m_Options;
}

void UARTDecoder::Decode(const uint8_t* _input, uint32_t _size)
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

    uint32_t savedSampleNum = 0;

    for (uint32_t i = 0; i < _size; i += 2)
    {
        // Read count and data for decode RLE
        const uint8_t count = _input[i];
        const uint8_t data = _input[i + 1];

        for(uint16_t j = 0; j < count + 1; ++j)
        {
            bool newBit = data & (1 << (m_Options.rx - 1));
            uint32_t sampleNum = savedSampleNum;
            assert(m_Samplenum < std::numeric_limits<decltype(m_Samplenum)>::max() && "m_Samplenum overflow");
            savedSampleNum += 1;

            if(m_Options.invert_rx)
                newBit = !newBit;

            // State machine for RX line
            if(m_State == WAIT_FOR_START_BIT)
                WaitForStartBit(newBit, sampleNum);
            else if(m_State == GET_START_BIT)
                GetStartBit(newBit, sampleNum);
            else if(m_State == GET_DATA_BITS)
                GetDataBits(newBit, sampleNum);
            else if(m_State == GET_PARITY_BIT)
                GetParityBit(newBit, sampleNum);
            else if(m_State == GET_STOP_BITS)
                GetStopBits(newBit, sampleNum);
            else if(m_State == WAIT_END_OF_FULL_STOP || m_State == WAIT_END_OF_HALF_STOP)
            	WaitEndOfStop(newBit, sampleNum);

            // Save current values of lines for next round
            m_OldBit = newBit;
        }
    }

    for(uint32_t i=0; i<m_Result.size(); i++)
    {
//        if(m_Result[i].control == (1 << 5)) // Start-bit
//            m_Result[i].length -= 2;
        /*else*/ if(m_Result[i].control == (1 << 6)) // Stop-bit
            m_Result[i].length -= 1;
        else if(m_Result[i].control == (1 << 4)) // Nothing on line
            m_Result[i].length -= 1;
    }

#ifndef CLI
	m_Signal.Set(std::move(m_Result));
    pthread_mutex_unlock(&m_mutex);
#endif
}

void UARTDecoder::WaitForStartBit(bool bit, uint32_t sampleNum)
{
    // The start bit is always 0 (low). As the idle UART (and the stop bit)
    // level is 1 (high), the beginning of a start bit is a falling edge.
    if(!(m_OldBit == 1 && bit == 0))
    {
        // Silence length calculating and return.
        // Write silence length to output if need.
        m_SilenceLength += 1;

        if(m_SilenceLength == 0xFFFF)
        {
            uint8_t controlByteInOutput = NO;
            controlByteInOutput = (1 << 4);
            m_Result.push_back(OutputPacket{controlByteInOutput, 0, m_SilenceLength});
            m_SilenceLength = 0;
        }

        return;
    }

    if(m_SilenceLength != 0)
    {
        uint8_t controlByteInOutput = NO;
        controlByteInOutput = (1 << 4);
        m_Result.push_back(OutputPacket{controlByteInOutput, 0, m_SilenceLength});
        m_SilenceLength = 0;
    }

    // Save the sample number where the start bit begins.
    m_FrameStart = sampleNum;
    m_StopBit1_Get = false;
    m_SilenceLength = 0;

    m_State = GET_START_BIT;
}

void UARTDecoder::GetStartBit(bool bit, uint32_t sampleNum)
{
    uint8_t controlByteInOutput = 0;
    // Skip samples until we're in the middle of the start bit.
    if(!BitRiched(0, sampleNum))
        return;

    m_StartBit = bit;

    // The startbit must be 0. If not, we report an error.
    if(m_StartBit != 0)
    {
        // START-bit error
        controlByteInOutput |= (1 << 2);
    }

    m_CurDataBit = 0;
    m_DataByte = 0;

    controlByteInOutput |= (1 << 5);
    m_Result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length

    m_State = GET_DATA_BITS;
}

void UARTDecoder::GetDataBits(bool bit, uint32_t sampleNum)
{
    // Skip samples until we're in the middle of the start bit.
    if(!BitRiched(m_CurDataBit + 1, sampleNum))
        return;

    if(m_Options.bitOrder == MSB_FIRST)
    {
        m_DataByte <<= 1;
        m_DataByte |= (bit << 0);
    }
    else if(m_Options.bitOrder == LSB_FIRST)
    {
        m_DataByte >>= 1;
        m_DataByte |= (bit << (m_Options.num_data_bits - 1));
    }

    // Return here, unless we already received all data bits.
    if(m_CurDataBit < (m_Options.num_data_bits - 1))
    {
        m_CurDataBit += 1;
        return;
    }

    if(m_Options.parity == NONE)
        m_State = GET_STOP_BITS;
    else
        m_State = GET_PARITY_BIT;
}

void UARTDecoder::GetParityBit(bool bit, uint32_t sampleNum)
{
    //uint8_t controlByteInOutput = 0;
    // If no parity is used/configured, skip to the next state immediately.
    if(m_Options.parity == NONE)
    {
        m_State = GET_STOP_BITS;
        return;
    }

    // Skip samples until we're in the middle of the parity bit.
    if(!BitRiched(m_Options.num_data_bits + 1, sampleNum))
        return;

    m_ParityBit = bit;

    m_State = GET_STOP_BITS;

    m_ParityOk = ParityOk();

    /*controlByteInOutput |= (1 << 7);
    if(!m_ParityOk)
        controlByteInOutput |= (1 << 1);

    m_Result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length*/
}

void UARTDecoder::WaitEndOfStop(uint8_t bitNum, uint32_t sampleNum)
{
	m_State = WAIT_FOR_START_BIT;
	return;

	m_CountOfStop++;
	float divider = 2;

	if(m_State == WAIT_END_OF_HALF_STOP)
		divider = 4;
	else if(m_State == WAIT_END_OF_FULL_STOP)
		divider = 2;

	if(m_CountOfStop == (uint16_t)(m_bitWidth/divider))
	{
		m_State = WAIT_FOR_START_BIT;
		m_CountOfStop = 0;
	}
}

void UARTDecoder::GetStopBits(bool bit, uint32_t sampleNum)
{
    // Skip samples until we're in the middle of the stop bit(s).
    uint8_t skip_parity = (m_Options.parity == NONE) ? 0 : 1;
    uint8_t bitNum = m_Options.num_data_bits + 1 + skip_parity;
    uint8_t controlByteInOutput = 0;
    uint8_t controlDataWriting = 0;
    uint16_t length = 0;

    // Get 0.5 Stop Bit
    if(m_Options.num_stop_bits == STOP_BITS_05)
    {
        if(!StopBit05Reached(bitNum, sampleNum))
            return;

        m_StopBit1 = bit;

        if(m_StopBit1 != 1)
        {
            // STOP-bit error
            controlByteInOutput |= (1 << 3);
        }

        //m_State = WAIT_FOR_START_BIT;
        m_State = WAIT_END_OF_HALF_STOP;

        m_FrameStop = sampleNum;

        // After STOP bits receiving we save data.
        if(m_Options.num_data_bits == DATA_BITS_9)
            controlDataWriting = ((m_DataByte >> 8) & 0x01) | 0x80;  // 0x80 means that 9 bit data received
        length = m_Options.num_data_bits * (uint16_t)m_bitWidth; // TODO: Data writing need to rewrite
        m_Result.push_back(OutputPacket{controlDataWriting, (uint8_t)m_DataByte, length});

        if(m_Options.parity != NONE)
        {
	    	controlByteInOutput |= (1 << 7);
	    	if(!m_ParityOk)
	    	    controlByteInOutput |= (1 << 1);
	    	m_Result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
    	}

        controlByteInOutput |= (1 << 6);
        m_Result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)(m_bitWidth/2)}); // TODO: Calculate length
    }

    // Get 1.0 Stop Bit
    else if(m_Options.num_stop_bits == STOP_BITS_10)
    {
        if(!StopBit10Reached(bitNum, sampleNum))
            return;

        m_StopBit1 = bit;

        if(m_StopBit1 != 1)
        {
            // STOP-bit error
            controlByteInOutput |= (1 << 3);
        }

        //m_State = WAIT_FOR_START_BIT;
        m_State = WAIT_END_OF_FULL_STOP;

        m_FrameStop = sampleNum;

        // After STOP bits receiving we save data.
        if(m_Options.num_data_bits == DATA_BITS_9)
            controlDataWriting = ((m_DataByte >> 8) & 0x01) | 0x80;  // 0x80 means that 9 bit data received
        length = m_Options.num_data_bits * (uint16_t)m_bitWidth; // TODO: Data writing need to rewrite
        m_Result.push_back(OutputPacket{controlDataWriting, (uint8_t)m_DataByte, length});

        if(m_Options.parity != NONE)
        {
	    	controlByteInOutput |= (1 << 7);
	    	if(!m_ParityOk)
	    	    controlByteInOutput |= (1 << 1);
	    	m_Result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
    	}

        controlByteInOutput |= (1 << 6);
        m_Result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
    }

    // Get 1.5 Stop Bits
    else if(m_Options.num_stop_bits == STOP_BITS_15)
    {
        // Get first full stop-bit
        if(!m_StopBit1_Get)
        {
            if(!StopBit10Reached(bitNum, sampleNum))
                return;

            m_StopBit1 = bit;
            m_StopBit1_Get = true;

            if(m_StopBit1 != 1)
            {
                // STOP-bit error
                controlByteInOutput |= (1 << 3);

                m_State = WAIT_END_OF_HALF_STOP;

                m_FrameStop = sampleNum;

                // After STOP bits receiving we save data.
                if(m_Options.num_data_bits == DATA_BITS_9)
                    controlDataWriting = ((m_DataByte >> 8) & 0x01) | 0x80;  // 0x80 means that 9 bit data received
                length = m_Options.num_data_bits * (uint16_t)m_bitWidth; // TODO: Data writing need to rewrite
                m_Result.push_back(OutputPacket{controlDataWriting, (uint8_t)m_DataByte, length});

		        if(m_Options.parity != NONE)
		        {
			    	controlByteInOutput |= (1 << 7);
			    	if(!m_ParityOk)
			    	    controlByteInOutput |= (1 << 1);
			    	m_Result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
		    	}

                controlByteInOutput |= (1 << 6);
                m_Result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
            }
        }
        // Get second half stop-bit
        else
        {
            if(!StopBit05Reached(bitNum+1, sampleNum))
                return;

            m_StopBit2 = bit;

            if(m_StopBit2 != 1)
            {
                // STOP-bit error
                controlByteInOutput |= (1 << 3);
            }

            //m_State = WAIT_FOR_START_BIT;
        	m_State = WAIT_END_OF_HALF_STOP;

            m_FrameStop = sampleNum;

            // After STOP bits receiving we save data.
            if(m_Options.num_data_bits == DATA_BITS_9)
                controlDataWriting = ((m_DataByte >> 8) & 0x01) | 0x80;  // 0x80 means that 9 bit data received
            length = m_Options.num_data_bits * (uint16_t)m_bitWidth; // TODO: Data writing need to rewrite
            m_Result.push_back(OutputPacket{controlDataWriting, (uint8_t)m_DataByte, length});

	        if(m_Options.parity != NONE)
	        {
		    	controlByteInOutput |= (1 << 7);
		    	if(!m_ParityOk)
		    	    controlByteInOutput |= (1 << 1);
		    	m_Result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
	    	}

            controlByteInOutput |= (1 << 6);
            m_Result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)(m_bitWidth/2)}); // TODO: Calculate length
        }
    }

    // Get 2.0 Stop Bits
    else if(m_Options.num_stop_bits == STOP_BITS_20)
    {
        // Get first full stop-bit
        if(!m_StopBit1_Get)
        {
            if(!StopBit10Reached(bitNum, sampleNum))
                return;

            m_StopBit1 = bit;
            m_StopBit1_Get = true;

            if(m_StopBit1 != 1)
            {
                // STOP-bit error
                controlByteInOutput |= (1 << 3);

                m_State = WAIT_END_OF_FULL_STOP;

                m_FrameStop = sampleNum;

                // After STOP bits receiving we save data.
                if(m_Options.num_data_bits == DATA_BITS_9)
                    controlDataWriting = ((m_DataByte >> 8) & 0x01) | 0x80;  // 0x80 means that 9 bit data received
                length = m_Options.num_data_bits * (uint16_t)m_bitWidth; // TODO: Data writing need to rewrite
                m_Result.push_back(OutputPacket{controlDataWriting, (uint8_t)m_DataByte, length});

		        if(m_Options.parity != NONE)
		        {
			    	controlByteInOutput |= (1 << 7);
			    	if(!m_ParityOk)
			    	    controlByteInOutput |= (1 << 1);
			    	m_Result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
		    	}

                controlByteInOutput |= (1 << 6);
                m_Result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
            }
        }
        // Get second full stop-bit
        else
        {
            if(!StopBit10Reached(bitNum+1, sampleNum))
                return;

            m_StopBit2 = bit;

            if(m_StopBit2 != 1)
            {
                // STOP-bit error
                controlByteInOutput |= (1 << 3);
            }

            //m_State = WAIT_FOR_START_BIT;
            m_State = WAIT_END_OF_FULL_STOP;

            m_FrameStop = sampleNum;

            // After STOP bits receiving we save data.
            if(m_Options.num_data_bits == DATA_BITS_9)
                controlDataWriting = ((m_DataByte >> 8) & 0x01) | 0x80;  // 0x80 means that 9 bit data received
            length = m_Options.num_data_bits * (uint16_t)m_bitWidth; // TODO: Data writing need to rewrite
            m_Result.push_back(OutputPacket{controlDataWriting, (uint8_t)m_DataByte, length});

	        if(m_Options.parity != NONE)
	        {
		    	controlByteInOutput |= (1 << 7);
		    	if(!m_ParityOk)
		    	    controlByteInOutput |= (1 << 1);
		    	m_Result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
	    	}

            controlByteInOutput |= (1 << 6);
            m_Result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
        }
    }

    // No STOP bits
    else if(m_Options.num_stop_bits == STOP_BITS_NO)
    {
    	m_State = WAIT_FOR_START_BIT;m_FrameStop = sampleNum;

        // Save data.
        if(m_Options.num_data_bits == DATA_BITS_9)
            controlDataWriting = ((m_DataByte >> 8) & 0x01) | 0x80;  // 0x80 means that 9 bit data received
        length = m_Options.num_data_bits * (uint16_t)m_bitWidth; // TODO: Data writing need to rewrite
        m_Result.push_back(OutputPacket{controlDataWriting, (uint8_t)m_DataByte, length});

        if(m_Options.parity != NONE)
        {
	    	controlByteInOutput |= (1 << 7);
	    	if(!m_ParityOk)
	    	    controlByteInOutput |= (1 << 1);
	    	m_Result.push_back(OutputPacket{controlByteInOutput, 0, (uint16_t)m_bitWidth}); // TODO: Calculate length
    	}
    }
}

bool UARTDecoder::StopBit05Reached(uint8_t bitNum, uint32_t sampleNum)
{
    // Return true if we reached the middle of the desired bit, false otherwise.
    uint32_t bitPos = m_FrameStart + (m_bitWidth - 1) / 4;
    bitPos += bitNum * m_bitWidth;

    if(sampleNum >= bitPos)
        return true;

    return false;
}

bool UARTDecoder::StopBit10Reached(uint8_t bitNum, uint32_t sampleNum)
{
    // Return true if we reached the middle of the desired bit, false otherwise.
    uint32_t bitPos = m_FrameStart + (m_bitWidth - 1) / 2;
    bitPos += bitNum * m_bitWidth;

    if(sampleNum >= bitPos)
        return true;

    return false;
}

bool UARTDecoder::BitRiched(uint8_t bitNum, uint32_t sampleNum)
{
    // Return true if we reached the middle of the desired bit, false otherwise.
    uint32_t bitPos = m_FrameStart + (m_bitWidth - 1) / 2;
    bitPos += bitNum * m_bitWidth;

    if(sampleNum >= bitPos)
        return true;

    return false;
}

bool UARTDecoder::ParityOk()
{
    int sumOfBits = 0;

    for(int i = 0; i < m_Options.num_data_bits; i++)
        sumOfBits += (int)((m_DataByte >> i) & 0x01);

    sumOfBits += m_ParityBit;

    if(m_Options.parity == ODD)
        return (sumOfBits % 2) == 1;
    else if(m_Options.parity == EVEN)
        return (sumOfBits % 2) == 0;

    return false;
}

UARTDecoder::~UARTDecoder()
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
