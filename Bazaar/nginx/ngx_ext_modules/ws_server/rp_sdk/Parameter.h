#pragma once

#include <memory>

#include "DataManager.h"
#include "BaseParameter.h"
#include "misc.h"

template <typename T, typename ValueT>
class CParameter: public CBaseParameter //class for parameter and signal
{
public:
	CParameter(std::string _name, AccessMode _access_mode, ValueT _value, int _fpga_update, T _min, T _max); //parameter constructor
	CParameter(std::string _name, AccessMode _access_mode, const ValueT& _value); //signal constructor

	const char* GetName() const;
	virtual void Set(const ValueT& _value) = 0; //set the m_Value.value

	ValueT& Value(); // access the value
	const ValueT& Value() const; // access the value
	const ValueT& NewValue();  //access the value is stored in temp storage

	virtual void Update(); //apply change of value

	virtual JSONNode GetJSONObject() = 0; //get JSON-formatted string with parameters or signals
	void SetValueFromJSON(JSONNode _node);// set the m_TmpValue->value from JSON object

	AccessMode GetAccessMode() const;

	virtual bool IsValueChanged() const = 0;
	virtual bool IsNewValue() const;
	void ClearNewValue();

protected:
	TParam<T, ValueT> m_Value; //parameter or signal struct data
	std::shared_ptr<TParam<T, ValueT>> m_TmpValue; //temp storage of parameter or signal data received from server

};

template <typename T, typename ValueT>
inline const char* CParameter<T, ValueT>::GetName() const
{
	return m_Value.name.c_str();
}

template <typename T, typename ValueT>
inline CParameter<T, ValueT>::CParameter(std::string _name, AccessMode _access_mode, ValueT _value, int _fpga_update, T _min, T _max)
{
	m_Value.name = _name;
	m_Value.value = _value;
	m_Value.min = _min;
	m_Value.max = _max;
	m_Value.access_mode = _access_mode;
	m_Value.fpga_update = _fpga_update;

	CDataManager * man = CDataManager::GetInstance();
	if(man)
		man->RegisterParam(this);
}

template <typename T, typename ValueT>
inline CParameter<T, ValueT>::CParameter(std::string _name, AccessMode _access_mode, const ValueT& _value)
{
	m_Value.name = _name;
	m_Value.value = _value;
//	m_Value.min;
//	m_Value.max;
	m_Value.access_mode = _access_mode;
//	m_Value.fpga_update;

	CDataManager * man = CDataManager::GetInstance();
	if(man)
		man->RegisterSignal(this);
}

template <typename T, typename ValueT>
inline ValueT& CParameter<T, ValueT>::Value()
{
	return m_Value.value;
}

template <typename T, typename ValueT>
inline const ValueT& CParameter<T, ValueT>::Value() const
{
	return m_Value.value;
}

template <typename T, typename ValueT>
inline const ValueT& CParameter<T, ValueT>::NewValue()
{
	if(m_TmpValue.get() != nullptr)
	{
		return m_TmpValue.get()->value;
	}
	else
	{
		return Value();
	}
}

template <typename T, typename ValueT>
inline void CParameter<T, ValueT>::Update()
{
	if(m_TmpValue.get() != nullptr)
	{
		Set(m_TmpValue.get()->value);
		m_TmpValue.reset();
	}
}

template <typename T, typename ValueT>
inline void CParameter<T, ValueT>::SetValueFromJSON(JSONNode _node)
{
	ValueT value = GetValueFromJSON<ValueT>(_node, "value");
	m_TmpValue.reset(new TParam<T, ValueT>);
	m_TmpValue.get()->value = value;
}

template <typename T, typename ValueT>
inline CBaseParameter::AccessMode CParameter<T, ValueT>::GetAccessMode() const
{
	return (CBaseParameter::AccessMode)m_Value.access_mode;
}

// template <typename T, typename ValueT>
// inline bool CParameter<T, ValueT>::IsValueChanged() const
// {
// 	return true;
// }

template <typename T, typename ValueT>
inline bool CParameter<T, ValueT>::IsNewValue() const
{
	return (m_TmpValue.get() != nullptr);
}

template <typename T, typename ValueT>
inline void CParameter<T, ValueT>::ClearNewValue()
{
	m_TmpValue.reset();
}
