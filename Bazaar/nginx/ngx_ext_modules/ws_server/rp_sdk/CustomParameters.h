#pragma once

#include <stdio.h>

#include "Parameter.h"

//template for params
template <typename Type> class CCustomParameter : public CParameter<Type, Type>
{
public:
	CCustomParameter(std::string _name, CBaseParameter::AccessMode _access_mode, Type _value, int _fpga_update, Type _min, Type _max)
		: CParameter<Type, Type>(_name, _access_mode, _value, _fpga_update, _min, _max)
		, m_SentValue(_value)
	{}	
	
	~CCustomParameter()
	{
/*		CDataManager * man = CDataManager::GetInstance();
		if(man)	
			man->UnRegisterParam(this->GetName());*/
	}

	JSONNode GetJSONObject()
	{
		JSONNode n(JSON_NODE);
		n.set_name(this->m_Value.name);
		n.push_back(JSONNode("value", this->m_Value.value));
		n.push_back(JSONNode("min", this->m_Value.min));
		n.push_back(JSONNode("max", this->m_Value.max));
		n.push_back(JSONNode("access_mode", this->m_Value.access_mode));
		n.push_back(JSONNode("fpga_update", this->m_Value.fpga_update));

		return n;
	}
	
	Type CheckMinMax(Type _value)
	{
		Type value = _value;
		if(this->m_Value.min > value) 
		{
			dbg_printf("Incorrect parameters value (min value)\n");
//			dbg_printf("Incorrect parameters value: %f (min:%f), "
//			"correcting it\n", (float)value, float(this->m_Value.min));				
		 	value = this->m_Value.min;

		} else if(this->m_Value.max < value) 
		{
			dbg_printf("Incorrect parameters value (max value)\n");
//			dbg_printf("Incorrect parameters value: %f (max:%f), "
//                    	" correcting it\n", (float)value, float(this->m_Value.max));
			value = this->m_Value.max;
		}

		return value;
	}

	void Set(const Type& _value)
	{
		this->m_Value.value = CheckMinMax(_value);
	}

	bool IsValueChanged() const
	{
		bool tmp = (m_SentValue != this->m_Value.value);
		m_SentValue = this->m_Value.value;
		return tmp;
	}

protected:
	mutable Type m_SentValue;

};

//template for signals
template <typename Type> class CCustomSignal : public CParameter<Type, std::vector<Type> >
{
public:
	CCustomSignal(std::string _name, int _size, Type _def_value)
		:CParameter<Type, std::vector<Type> >(_name, CBaseParameter::RO, std::vector<Type>(_size, _def_value)){}	

	CCustomSignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, Type _def_value)
		:CParameter<Type, std::vector<Type> >(_name, _access_mode, std::vector<Type>(_size, _def_value)){}	
		
	~CCustomSignal()
	{
/*		CDataManager * man = CDataManager::GetInstance();
		if(man)	
			man->UnRegisterSignal(this->GetName());*/
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
			Type res = this->m_Value.value.at(i);			
			child.push_back(JSONNode("", res));
		}		
		n.push_back(child);
		return n;
	}

	const Type& operator [](int _index) const
	{
		return this->m_Value.value.at(_index);
	}

	Type& operator [](int _index)
	{
		return this->m_Value.value.at(_index);
	}

	void Set(const std::vector<Type>& _value)
	{
		this->m_Value.value = _value;
	}

	void Resize(int _new_size)
	{
		this->m_Value.value.resize(_new_size);
	}

	int GetSize()
	{
		return this->m_Value.value.size();
	}
};

//custom CIntParameter 
class CIntParameter : public CCustomParameter<int>
{
public:
	CIntParameter(std::string _name, CBaseParameter::AccessMode _access_mode, int _value, int _fpga_update, int _min, int _max)
		:CCustomParameter(_name, _access_mode, _value, _fpga_update, _min, _max){};	
};

//custom CFloatParameter
class CFloatParameter : public CCustomParameter<float>
{
public:
	CFloatParameter(std::string _name, CBaseParameter::AccessMode _access_mode, float _value, int _fpga_update, float _min, float _max)
		:CCustomParameter(_name, _access_mode, _value, _fpga_update, _min, _max){};	
};

//custom CDoubleParameter
class CDoubleParameter : public CCustomParameter<double>
{
public:
	CDoubleParameter(std::string _name, CBaseParameter::AccessMode _access_mode, double _value, int _fpga_update, double _min, double _max)
		:CCustomParameter(_name, _access_mode, _value, _fpga_update, _min, _max){};	
};

//custom CBooleanParameter
class CBooleanParameter : public CCustomParameter<bool>
{
public:
	CBooleanParameter(std::string _name, CBaseParameter::AccessMode _access_mode, bool _value, int _fpga_update)
		:CCustomParameter(_name, _access_mode, _value, _fpga_update, false, true){};
};

//custom CStringParameter
class CStringParameter : public CCustomParameter<std::string>
{
public:
	CStringParameter(std::string _name, CBaseParameter::AccessMode _access_mode, std::string _value, int _fpga_update)
		:CCustomParameter(_name, _access_mode, _value, _fpga_update, "", ""){};

	void Set(const std::string& _value)
	{
		this->m_Value.value = _value;
	}
};

//custom CIntSignal
class CIntSignal : public CCustomSignal<int>
{
public:
	CIntSignal(std::string _name, int _size, int _def_value)
		:CCustomSignal(_name, _size, _def_value){};

	CIntSignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, int _def_value)
		:CCustomSignal(_name, _access_mode, _size, _def_value){};
};

//custom CFloatSignal
class CFloatSignal : public CCustomSignal<float>
{
public:
	CFloatSignal(std::string _name, int _size, float _def_value)
		:CCustomSignal(_name, _size, _def_value){};

	CFloatSignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, float _def_value)
		:CCustomSignal(_name, _access_mode, _size, _def_value){};
};

//custom CDoubleSignal
class CDoubleSignal : public CCustomSignal<double>
{
public:
	CDoubleSignal(std::string _name, int _size, double _def_value)
		:CCustomSignal(_name, _size, _def_value){};

	CDoubleSignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, double _def_value)
		:CCustomSignal(_name, _access_mode, _size, _def_value){};
};

extern CBooleanParameter IsDemoParam;		// special default parameter to check mode (demo or not)
extern CStringParameter InCommandParam;		// special default parameter to receive a string command from WEB UI
extern CStringParameter OutCommandParam;	// special default parameter to send a string command to WEB UI
