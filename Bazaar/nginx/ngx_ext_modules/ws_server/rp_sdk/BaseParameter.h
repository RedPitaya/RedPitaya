#pragma once

#include <libjson.h>

class CBaseParameter  //base class for parameter and signal
{
public:
	enum AccessMode
	{
		RW = 0,
		RO,
		WO,

		RWSA,	// send value always
		ROSA,	// send value always

		AccessModes
	};

	virtual ~CBaseParameter(){};
	virtual const char* GetName() const = 0;
	virtual void Update() = 0;		//apply change of value
	virtual JSONNode GetJSONObject() = 0;	//get JSON-formatted string with parameters or signals
	virtual void SetValueFromJSON(JSONNode _node) = 0;	// set the m_TmpValue->value from JSON object
	virtual AccessMode GetAccessMode() const = 0;
	virtual bool IsValueChanged() const = 0;
	virtual bool IsNewValue() const = 0;
	virtual void ClearNewValue() = 0;
	virtual bool NeedSend(bool _no_need=false) const { return _no_need; };
};
