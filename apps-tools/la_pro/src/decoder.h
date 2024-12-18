#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <typeinfo>

#include "DataManager.h"
#include "CustomParameters.h"
#include "rp_la.h"

using namespace rp_la;

//template for signals
template <>
class CCustomSignal<OutputPacket> : public CParameter<OutputPacket, std::vector<OutputPacket>>
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
			node.push_back(JSONNode("c", res.control));
			node.push_back(JSONNode("d", res.data));
			node.push_back(JSONNode("l", res.length));
			node.push_back(JSONNode("ln", res.line_name));
			node.push_back(JSONNode("b", res.bitsInPack));
			node.push_back(JSONNode("s", res.sampleStart));

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

	bool ForceSend()
	{
		m_Dirty = true;
		return true;
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
			node.push_back(JSONNode("c", res.control));
			node.push_back(JSONNode("d", res.data));
			node.push_back(JSONNode("l", res.length));
			node.push_back(JSONNode("ln", res.line_name));
			node.push_back(JSONNode("b", res.bitsInPack));
			node.push_back(JSONNode("s", res.sampleStart));

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
		uint8_t control = i->at("c").as_int();
		uint32_t data = i->at("d").as_int();
		uint32_t length = i->at("l").as_int();
		std::string line_name = i->at("ln").as_string();
		float bits = i->at("b").as_float();
		uint32_t sampleStart = i->at("s").as_float();

		res.push_back(OutputPacket{line_name , control, data, length, bits, sampleStart});
	}

	return res;
}
