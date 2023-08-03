#pragma once

#include "decoder.h"

#ifndef CLI
#include "DataManager.h"
#include "CustomParameters.h"
#include <pthread.h>
#endif

#include <vector>
#include <cstdint>
#include <string>


enum UartBitOrder
{
	LSB_FIRST,
	MSB_FIRST
};

enum NumDataBits
{
	DATA_BITS_5 = 5,
	DATA_BITS_6,
	DATA_BITS_7,
	DATA_BITS_8,
	DATA_BITS_9
};

enum Parity
{
	NONE,
	ODD,
	EVEN
};

enum NumStopBits
{
	STOP_BITS_NO = -1,
	STOP_BITS_05,
	STOP_BITS_10,
	STOP_BITS_15,
	STOP_BITS_20
};

struct UARTParameters
{
	uint8_t rx; // 1..8
	uint32_t baudrate;
	bool invert_rx;
    UartBitOrder bitOrder;
    NumDataBits num_data_bits; // 5..9
    Parity parity;
    NumStopBits num_stop_bits;
	uint32_t samplerate;
};

#ifndef CLI
class CUARTParameter : public CDecoderParameter<UARTParameters>
{
public:
	CUARTParameter(std::string _name, CBaseParameter::AccessMode _access_mode, UARTParameters _value, int _fpga_update)
		:CDecoderParameter(_name, _access_mode, _value, _fpga_update, 0, 0){};

	JSONNode GetJSONObject()
	{
		JSONNode n(JSON_NODE);
		n.set_name(this->m_Value.name);

		JSONNode val(JSON_NODE);
		val.set_name("value");
		val.push_back(JSONNode("rx", this->m_Value.value.rx));
		val.push_back(JSONNode("baudrate", this->m_Value.value.baudrate));
		val.push_back(JSONNode("invert_rx", this->m_Value.value.invert_rx));
		val.push_back(JSONNode("bitOrder", (int)this->m_Value.value.bitOrder));
		val.push_back(JSONNode("num_data_bits", (int)this->m_Value.value.num_data_bits));
		val.push_back(JSONNode("parity", (int)this->m_Value.value.parity));
		val.push_back(JSONNode("num_stop_bits", (int)this->m_Value.value.num_stop_bits));
		val.push_back(JSONNode("samplerate", (int)this->m_Value.value.samplerate));

		n.push_back(val);
		n.push_back(JSONNode("access_mode", this->m_Value.access_mode));

		return n;
	}
};

//UARTParameters specialization of function
template <>
inline UARTParameters GetValueFromJSON<UARTParameters>(JSONNode _node, const char* _at)
{
	UARTParameters p;
	p.rx = _node.at(_at).at("rx").as_int();
	p.baudrate = _node.at(_at).at("baudrate").as_int();
	p.invert_rx = _node.at(_at).at("invert_rx").as_int();
	p.bitOrder = (UartBitOrder)_node.at(_at).at("bitOrder").as_int();
	p.num_data_bits = (NumDataBits)_node.at(_at).at("num_data_bits").as_int();
	p.parity = (Parity)_node.at(_at).at("parity").as_int();
	p.num_stop_bits = (NumStopBits)_node.at(_at).at("num_stop_bits").as_int();
	p.samplerate = _node.at(_at).at("samplerate").as_int();

	return p;
}
#endif

class UARTDecoder : public Decoder
{
public:
    UARTDecoder(const std::string& _name = "uart");
    ~UARTDecoder();

    void SetParameters(const UARTParameters& _new_params);
    UARTParameters GetParameters();
    void Decode(const uint8_t* _input, uint32_t _size) override;
    bool IsParametersChanged() override;
    void UpdateParameters() override;
    void UpdateSignals() override;
    std::vector<OutputPacket> GetSignal() { return m_Result; }

    void printVector(){
    	printf("%02X -\n", m_Result[1].data);
    }

private:
    bool m_ParametersUpdated;

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

	// this enum for Control byte in output structure
	enum UARTAnnotations
	{
		NO,
		RX_DATA,
		RX_START,
		RX_PARITY_OK,
		RX_PARITY_ERR,
		RX_STOP,
		RX_WARNING,
		RX_DATA_BITS
	};

	void ResetDecoder();

	void WaitForStartBit(bool bit, uint32_t sampleNum);
    void GetStartBit(bool bit, uint32_t sampleNum);
    void GetDataBits(bool bit, uint32_t sampleNum);
    void GetParityBit(bool bit, uint32_t sampleNum);
    void GetStopBits(bool bit, uint32_t sampleNum);
    bool BitRiched(uint8_t bitNum, uint32_t sampleNum);
    bool ParityOk();
	bool StopBit05Reached(uint8_t bitNum, uint32_t sampleNum);
	bool StopBit10Reached(uint8_t bitNum, uint32_t sampleNum);
	void WaitEndOfStop(uint8_t bitNum, uint32_t sampleNum);

    bool m_OldBit;
    uint32_t m_FrameStart;
    uint32_t m_FrameStop;
    uint8_t m_StartBit;
    uint8_t m_CurDataBit;
    uint16_t m_DataByte;
    uint8_t m_ParityBit;
    bool m_ParityOk;
    uint8_t m_StopBit1;
    uint8_t m_StopBit2;
	uint32_t m_Samplenum;
	uint16_t m_SilenceLength;

	bool m_StopBit1_Get;

	float m_bitWidth;
	uint16_t m_CountOfStop;

	State m_State;

	UARTParameters m_Options;
	std::string m_Name;

	std::vector<OutputPacket> m_Result;

#ifndef CLI
	CDecodedSignal m_Signal;
	CUARTParameter m_Parameters;
	pthread_mutex_t  m_mutex = PTHREAD_MUTEX_INITIALIZER;
	bool m_Start = false;
#endif
};
