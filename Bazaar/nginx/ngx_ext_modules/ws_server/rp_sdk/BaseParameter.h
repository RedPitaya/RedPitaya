#pragma once

#include <libjson.h>

class CBaseParameter  //base class for parameter and signal
{
   public:
    enum AccessMode {
        RW = 0,
        RO,
        WO,

        RWSA,  // send value always
        ROSA,  // send value always

        AccessModes
    };
    enum ParameterType { PARAM = 0, SIGNAL = 1, BIN_SIGNAL = 2 };

    enum BinarySignalType { UNDEFINED = 0, INT8 = 1, INT16 = 2, INT32 = 3, UINT8 = 4, UINT16 = 5, UINT32 = 6, FLOAT = 7, DOUBLE = 8 };

    explicit CBaseParameter(ParameterType pType) : m_paramType(pType){};
    virtual ~CBaseParameter(){};
    virtual const char* GetName() const = 0;
    virtual void Update() = 0;                          //apply change of value
    virtual JSONNode GetJSONObject() = 0;               //get JSON-formatted string with parameters or signals
    virtual void SetValueFromJSON(JSONNode _node) = 0;  // set the m_TmpValue->value from JSON object
    virtual AccessMode GetAccessMode() const = 0;
    virtual bool IsValueChanged() const = 0;
    virtual bool IsNewValue() const = 0;
    virtual void ClearNewValue() = 0;
    virtual bool NeedSend(bool _no_need = false) const { return _no_need; };
    virtual size_t GetSizeInBytes() = 0;
    virtual BinarySignalType GetDataType() { return UNDEFINED; };
    virtual const void* GetDataVoidPtr() { return NULL; };

    ParameterType GetParameterType() { return m_paramType; };

   private:
    ParameterType m_paramType;
};
