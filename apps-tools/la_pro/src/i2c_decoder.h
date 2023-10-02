#pragma once

#include "decoder.h"

#ifndef CLI
#include "DataManager.h"
#include "CustomParameters.h"
#include <pthread.h>
#endif

#include <cstring>
#include <string>
#include <vector>
#include <deque>

enum AddressFormat { Shifted, Unshifted };
struct I2CParameters
{
	uint8_t scl; // 0...7
	uint8_t sda; // 0...7
	uint32_t acq_speed;
	AddressFormat address_format;
	uint8_t  invert_bit; 
};

#ifndef CLI
//custom CI2CParameter
class CI2CParameter : public CDecoderParameter<I2CParameters>
{
public:
	CI2CParameter(std::string _name, CBaseParameter::AccessMode _access_mode, I2CParameters _value, int _fpga_update)
		:CDecoderParameter(_name, _access_mode, _value, _fpga_update, 0, 0){};

	JSONNode GetJSONObject()
	{
		JSONNode n(JSON_NODE);
		n.set_name(this->m_Value.name);

		JSONNode val(JSON_NODE);
		val.set_name("value");
		val.push_back(JSONNode("scl", this->m_Value.value.scl));
		val.push_back(JSONNode("sda", this->m_Value.value.sda));
		val.push_back(JSONNode("acq_speed", this->m_Value.value.acq_speed));
		val.push_back(JSONNode("address_format", (int)this->m_Value.value.address_format));
		val.push_back(JSONNode("invert_bit", this->m_Value.value.invert_bit));	
		n.push_back(val);
		n.push_back(JSONNode("access_mode", this->m_Value.access_mode));

		return n;
	}
};


//I2CParameters specialization of function
template <>
inline I2CParameters GetValueFromJSON<I2CParameters>(JSONNode _node, const char* _at)
{
	uint8_t scl = _node.at(_at).at("scl").as_int();
	uint8_t sda = _node.at(_at).at("sda").as_int();
	uint32_t acq_speed = _node.at(_at).at("acq_speed").as_int();
	uint8_t invert_bit = _node.at(_at).at("invert_bit").as_int();
	AddressFormat address_format = (AddressFormat)_node.at(_at).at("address_format").as_int();

	return {scl, sda, acq_speed, address_format,invert_bit};
}
#endif

class I2CDecoder : public Decoder
{
public:
    I2CDecoder(const std::string& _name = "i2c");
    ~I2CDecoder();

    void SetParameters(const I2CParameters& _new_params);
    void Decode(const uint8_t* _input, uint32_t _size) override;
    bool IsParametersChanged() override;
    void UpdateParameters() override;
    void UpdateSignals() override;

    std::vector<OutputPacket> GetSignal() { return m_Result; }

private:
    bool m_ParametersUpdated;

	enum States
	{
		FIND_START,
		FIND_ADDRESS,
		FIND_DATA,
		FIND_ACK
	};

	// this enum for Control byte in output structure
	enum I2CAnnotations
	{
		START = 1,
		REPEAT_START,
		STOP,
		ACK,
		NACK,
		READ_ADDRESS,
		WRITE_ADDRESS,
		DATA_READ,
		DATA_WRITE,
		NOTHING
	};

	void ResetDecoder();
	void AddNothing();

	bool IsStartCondition(bool scl, bool sda) const;
	bool IsDataBit(bool scl, bool sda) const;
	bool IsStopCondition(bool scl, bool sda) const;

	void GetAck(bool scl, bool sda);
	void FoundStart(bool scl, bool sda);
	void FoundAddressOrData(bool scl, bool sda);
	void FoundStop(bool scl, bool sda);

	bool m_OldScl;
	bool m_OldSda;
	bool m_OldPins;
	uint32_t m_Ss;
	uint32_t m_Es;
	uint32_t m_Ss_byte;
	uint8_t m_Databyte;

	uint32_t m_Bitwidth;
	uint32_t m_Samplerate;
	uint32_t m_Samplenum;
	uint32_t m_OldSamplenum;
	uint32_t m_Bitcount;

	int m_Wr;
	uint32_t m_Is_repeat_start;
	uint32_t m_Pdu_start;
	uint32_t m_Pdu_bits;

	I2CAnnotations m_Cmd;
	States m_State;
	States m_OldState;

	I2CParameters m_Options;
	std::deque<std::vector<uint32_t>> m_Bits;
	std::vector<OutputPacket> m_Result;
	std::string m_Name;

#ifndef CLI
	CDecodedSignal m_Signal;
	CI2CParameter m_Parameters;
	pthread_mutex_t  m_mutex = PTHREAD_MUTEX_INITIALIZER;
	bool m_Start = false;
#endif
};
