#pragma once

#include <vector>
#include <deque>
#include <string>
#include <cstdint>
#include <cstdlib>

#ifndef CLI
#include "DataManager.h"
#include "CustomParameters.h"
#include <pthread.h>
#endif

#include "decoder.h"

enum CsPolartiy { ActiveLow, ActiveHigh };
enum BitOrder { MsbFirst, LsbFirst };

const uint8_t DATA_ERROR = 0x80; // For control byte

struct SpiParameters
{
    uint8_t clk; // 0...7 - ch Number
    uint8_t data; // 0...7
    uint8_t cs;  // -1...7, -1 if is not set
    uint8_t cpol = 0;
    uint8_t cpha = 0;
    uint8_t word_size = 8;
	uint32_t acq_speed;
    CsPolartiy cs_polarity = ActiveLow;
    BitOrder bit_order = MsbFirst;
	uint8_t  invert_bit; 
};

#ifndef CLI
//custom CI2CParameter
class SpiParameter : public CDecoderParameter<SpiParameters>
{
public:
	SpiParameter(std::string _name, CBaseParameter::AccessMode _access_mode, SpiParameters _value, int _fpga_update)
		:CDecoderParameter(_name, _access_mode, _value, _fpga_update, 0, 0){};

	JSONNode GetJSONObject()
	{
		JSONNode n(JSON_NODE);
		n.set_name(this->m_Value.name);

		JSONNode val(JSON_NODE);
		val.set_name("value");
		val.push_back(JSONNode("clk", this->m_Value.value.clk));
		val.push_back(JSONNode("data", this->m_Value.value.data));
		val.push_back(JSONNode("cs", this->m_Value.value.cs));
		val.push_back(JSONNode("cpol", this->m_Value.value.cpol));
		val.push_back(JSONNode("cpha", this->m_Value.value.cpha));
		val.push_back(JSONNode("word_size", this->m_Value.value.word_size));
		val.push_back(JSONNode("acq_speed", this->m_Value.value.acq_speed));
		val.push_back(JSONNode("cs_polarity", (int)this->m_Value.value.cs_polarity));
		val.push_back(JSONNode("bit_order", (int)this->m_Value.value.bit_order));
		val.push_back(JSONNode("invert_bit", this->m_Value.value.invert_bit));	
		n.push_back(val);
		n.push_back(JSONNode("access_mode", this->m_Value.access_mode));

		return n;
	}
};


//I2CParameters specialization of function
template <>
inline SpiParameters GetValueFromJSON<SpiParameters>(JSONNode _node, const char* _at)
{
	SpiParameters p;
	p.clk = _node.at(_at).at("clk").as_int();
	p.data = _node.at(_at).at("data").as_int();
	p.cs = _node.at(_at).at("cs").as_int();
	p.cpol = _node.at(_at).at("cpol").as_int();
	p.cpha = _node.at(_at).at("cpha").as_int();
	p.word_size = _node.at(_at).at("word_size").as_int();
	p.acq_speed = _node.at(_at).at("acq_speed").as_int();
	p.cs_polarity = (CsPolartiy)_node.at(_at).at("cs_polarity").as_int();
	p.bit_order = (BitOrder)_node.at(_at).at("bit_order").as_int();
	p.invert_bit = _node.at(_at).at("invert_bit").as_int();
	//fprintf(stderr, "%s\n", _node.write_formatted().c_str());

	return p;
}
#endif

class SpiDecoder : public Decoder
{
public:
	SpiDecoder(const std::string& _name = "spi");
	~SpiDecoder();

    void SetParameters(const SpiParameters& _new_params);
    void Decode(const uint8_t* _input, uint32_t _size) override;
    bool IsParametersChanged() override;
    void UpdateParameters() override;
    void UpdateSignals() override;
    std::vector<OutputPacket> GetSignal() { return m_Result; }
    //  NOTE: When CS is not set try to look at first bits only then generate error
private:
    bool m_ParametersUpdated;

	enum
	{
		NOTHING = 10
	};

	enum State
	{
		FIND_DATA,
		COLLECT_DATA
	};

	void ResetDecoder();

	void FindClkEdge(bool data, bool clk, bool cs);
	void HandleBit(bool data, bool clk, bool cs);
	bool CsAsserted(bool cs);
	void ResetDecoderState();

	int m_Oldclk = 1;
	uint32_t m_Bitcount = 0;
	uint32_t m_Data = 0;
	int m_Oldcs = -1;
	int m_Oldpins = -1;
	int m_Have_cs = -1;
	int m_Cs_was_deasserted = -1;
	int m_Samplenum = -1;
	int m_OldSamplenum = 0;
	int m_StartDataSamplenum = 0;

	SpiParameters m_Options;
	std::vector<OutputPacket> m_Result;

	std::string m_Name;

	State m_State;
#ifndef CLI
	CDecodedSignal m_Signal;
	SpiParameter m_Parameters;
	pthread_mutex_t  m_mutex = PTHREAD_MUTEX_INITIALIZER;
	bool m_Start = false;
#endif
};
