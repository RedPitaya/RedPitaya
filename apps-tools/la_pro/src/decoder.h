#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <typeinfo>

#ifndef CLI
#include "DataManager.h"
#include "CustomParameters.h"
#endif

struct OutputPacket
{
    uint8_t control; // 0 when data, elsewise represents specific state
					 // anyway control byte specifies meaning of the “data” byte
    uint32_t data;
	uint16_t length; // RLE, how many counts takes this byte

};

class Decoder
{
public:
    virtual void Decode(const uint8_t* _input, uint32_t _size) = 0;
    virtual ~Decoder() {}
    virtual bool IsParametersChanged() = 0;
    virtual void UpdateParameters() = 0;
    virtual void UpdateSignals() = 0;
};

#ifndef CLI
//template for signals
template <>
class CCustomSignal<OutputPacket> : public CParameter<OutputPacket, std::vector<OutputPacket> >
{
public:
	CCustomSignal(std::string _name, int _size, OutputPacket _def_value)
		:CParameter<OutputPacket, std::vector<OutputPacket> >(_name, CBaseParameter::RO, std::vector<OutputPacket>(_size, _def_value)),
		m_Dirty(true){}

	CCustomSignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, OutputPacket _def_value)
		:CParameter<OutputPacket, std::vector<OutputPacket> >(_name, _access_mode, std::vector<OutputPacket>(_size, _def_value)),
		m_Dirty(true) {}

	virtual ~CCustomSignal()
	{
	}

	JSONNode GetJSONObject()
	{
		JSONNode n(JSON_NODE);
		n.set_name(this->m_Value.name);
		n.push_back(JSONNode("size", this->m_Value.value.size()));

		JSONNode child(JSON_ARRAY);
		child.set_name("value");
		for(unsigned int i=0; i < this->m_Value.value.size(); i++)
		{
			OutputPacket res = this->m_Value.value.at(i);

			JSONNode node(JSON_NODE);
			node.push_back(JSONNode("control", res.control));
			node.push_back(JSONNode("data", res.data));
			node.push_back(JSONNode("length", res.length));

			child.push_back(node);
		}
		n.push_back(child);
		return n;
	}

	const OutputPacket& operator [](int _index) const
	{
		return this->m_Value.value.at(_index);
	}

	OutputPacket& operator [](int _index)
	{
		m_Dirty = true;
		return this->m_Value.value.at(_index);
	}

	void Set(const std::vector<OutputPacket>& _value)
	{
		m_Dirty = true;
		this->m_Value.value = _value;
	}

	void Resize(int _new_size)
	{
		m_Dirty = true;
		this->m_Value.value.resize(_new_size);
	}

	void Update()
	{
		m_Dirty = false;
	}

	int GetSize()
	{
		return this->m_Value.value.size();
	}

	bool IsValueChanged() const
	{
		return m_Dirty;
	}
protected:
	bool m_Dirty;
};

class CDecodedSignal : public CCustomSignal<OutputPacket>
{
public:
	CDecodedSignal(std::string _name, int _size, OutputPacket _def_value)
		:CCustomSignal(_name, _size, _def_value){};

	CDecodedSignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, OutputPacket _def_value)
		:CCustomSignal(_name, _access_mode, _size, _def_value){};

	JSONNode GetJSONObject()
	{
		JSONNode n(JSON_NODE);
		n.set_name(this->m_Value.name);
		n.push_back(JSONNode("size", this->m_Value.value.size()));

		JSONNode child(JSON_ARRAY);
		child.set_name("value");
		for(unsigned int i=0; i < this->m_Value.value.size(); i++)
		{
			OutputPacket res = this->m_Value.value.at(i);

			JSONNode node(JSON_NODE);
			node.push_back(JSONNode("control", res.control));
			node.push_back(JSONNode("data", res.data));
			node.push_back(JSONNode("length", res.length));

			child.push_back(node);
		}
		n.push_back(child);
		return n;
	}

	void Set(std::vector<OutputPacket>&& _value)
	{
		m_Dirty = true;
		this->m_Value.value = std::move(_value);
	}

};

//std::vector<OutputPacket> specialization of function
template <>
inline std::vector<OutputPacket> GetValueFromJSON<std::vector<OutputPacket> >(JSONNode _node, const char* _at)
{
	JSONNode n = _node.at(_at);
	std::vector<OutputPacket> res;
	for (auto i = n.begin(); i != n.end(); ++i)
	{
		uint8_t control = i->at("control").as_int();
		uint8_t data = i->at("data").as_int();
		uint16_t length = i->at("length").as_int();

		res.push_back(OutputPacket{control, data, length});
	}

	return res;
}
#endif
