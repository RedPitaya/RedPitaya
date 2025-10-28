#pragma once

#include <stdio.h>
#include <string.h>

#include "Parameter.h"

#define CONFIG_VAR 1

//template for params
template <typename Type>
class CCustomParameter : public CParameter<Type, Type> {
   public:
    CCustomParameter(std::string _name, CBaseParameter::AccessMode _access_mode, Type _value, int _fpga_update, Type _min, Type _max)
        : CParameter<Type, Type>(CParameter<Type, Type>::PARAM, _name, _access_mode, _value, _fpga_update, _min, _max),
          m_SentValue(_value),
          m_Dirty(false),
          m_NeedUnregister(false),
          m_NeedSend(false),
          m_Tag(0) {}

    CCustomParameter(std::string _name, CBaseParameter::AccessMode _access_mode, Type _value, int _fpga_update, Type _min, Type _max, int _tag)
        : CParameter<Type, Type>(CParameter<Type, Type>::PARAM, _name, _access_mode, _value, _fpga_update, _min, _max),
          m_SentValue(_value),
          m_Dirty(false),
          m_NeedUnregister(false),
          m_NeedSend(false),
          m_Tag(_tag) {}

    ~CCustomParameter() {
        /*		CDataManager * man = CDataManager::GetInstance();
		if(man)
			man->UnRegisterParam(this->GetName());*/
        if (m_NeedUnregister) {
            CDataManager* man = CDataManager::GetInstance();
            if (man)
                man->UnRegisterParam(this->GetName());
        }
    }

    JSONNode GetJSONObject() {
        JSONNode n(JSON_NODE);
        n.set_name(this->m_Value.name);
        n.push_back(JSONNode("value", this->m_Value.value));
        n.push_back(JSONNode("min", this->m_Value.min));
        n.push_back(JSONNode("max", this->m_Value.max));
        n.push_back(JSONNode("access_mode", this->m_Value.access_mode));
        n.push_back(JSONNode("fpga_update", this->m_Value.fpga_update));

        return n;
    }

    Type CheckMinMax(Type _value) {
        Type value = _value;
        if (this->m_Value.min > value) {
            value = this->m_Value.min;

        } else if (this->m_Value.max < value) {
            value = this->m_Value.max;
        }

        return value;
    }

    void Set(const Type& _value) { this->m_Value.value = CheckMinMax(_value); }

    void SetMax(const Type& _max) {
        this->m_Value.max = _max;
        this->m_Value.value = CheckMinMax(this->m_Value.value);
    }

    const Type& GetMax() { return this->m_Value.max; }

    void SetMin(const Type& _min) {
        this->m_Value.min = _min;
        this->m_Value.value = CheckMinMax(this->m_Value.value);
    }

    const Type& GetMin() { return this->m_Value.min; }

    // void SendValue(const Type& _value)
    // {
    // 	this->m_Value.value = CheckMinMax(_value);
    // 	m_Dirty = true;
    // }

    bool IsValueChanged() const {
        bool tmp = (m_SentValue != this->m_Value.value) || m_Dirty;
        m_SentValue = this->m_Value.value;
        m_Dirty = false;

        return tmp;
    }

    void NeedUnregister(bool _value) { m_NeedUnregister = _value; }

    void SendValue(const Type& _value) {
        this->m_Value.value = CheckMinMax(_value);
        m_NeedSend = true;
    }

    bool NeedSend(bool _no_need = false) const {
        bool tmp = m_NeedSend;
        if (_no_need)
            m_NeedSend = false;
        return tmp;
    }

    int Tag() const { return m_Tag; }

    size_t GetSizeInBytes() { return sizeof(Type); }

   protected:
    mutable Type m_SentValue;
    mutable bool m_Dirty;
    bool m_NeedUnregister;
    mutable bool m_NeedSend;
    mutable int m_Tag;
};

//template for signals
template <typename Type>
class CCustomSignal : public CParameter<Type, std::vector<Type>> {
   public:
    CCustomSignal(std::string _name, int _size, Type _def_value)
        : CParameter<Type, std::vector<Type>>(CParameter<Type, Type>::SIGNAL, _name, CBaseParameter::RO, std::vector<Type>(_size, _def_value)), m_Dirty(true) {}

    CCustomSignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, Type _def_value)
        : CParameter<Type, std::vector<Type>>(CParameter<Type, Type>::SIGNAL, _name, _access_mode, std::vector<Type>(_size, _def_value)), m_Dirty(true) {}

    ~CCustomSignal() {
        /*		CDataManager * man = CDataManager::GetInstance();
		if(man)
			man->UnRegisterSignal(this->GetName());*/
    }

    JSONNode GetJSONObject() {
        JSONNode n(JSON_NODE);
        n.set_name(this->m_Value.name);
        n.push_back(JSONNode("size", this->m_Value.value.size()));

        JSONNode child(JSON_ARRAY);
        child.set_name("value");
        for (unsigned int i = 0; i < this->m_Value.value.size(); i++) {
            Type res = this->m_Value.value.at(i);
            child.push_back(JSONNode("", res));
        }
        n.push_back(child);
        return n;
    }

    const Type& operator[](int _index) const { return this->m_Value.value.at(_index); }

    Type& operator[](int _index) {
        m_Dirty = true;
        return this->m_Value.value.at(_index);
    }

    void Set(const std::vector<Type>& _value) {
        m_Dirty = true;
        this->m_Value.value = _value;
    }

    void Set(const Type* _value, size_t _size) {
        m_Dirty = true;
        this->m_Value.value.assign(_value, _value + _size);
    }

    void Resize(int _new_size) {
        m_Dirty = true;
        this->m_Value.value.resize(_new_size);
    }

    void Update() { m_Dirty = false; }

    int GetSize() { return this->m_Value.value.size(); }

    std::vector<Type> GetData() { return this->m_Value.value; }

    bool IsValueChanged() const { return m_Dirty; }

    bool ForceSend() {
        m_Dirty = true;
        return true;
    }

    size_t GetSizeInBytes() { return GetSize() * sizeof(Type); }

   private:
    bool m_Dirty;
};

template <typename Type>
class CCustomBase64Signal : public CParameter<Type, std::vector<Type>> {
   public:
    CCustomBase64Signal(std::string _name, int _size, Type _def_value)
        : CParameter<Type, std::vector<Type>>(CParameter<Type, Type>::SIGNAL, _name, CBaseParameter::RO, std::vector<Type>(_size, _def_value)), m_Dirty(true) {}

    CCustomBase64Signal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, Type _def_value)
        : CParameter<Type, std::vector<Type>>(CParameter<Type, Type>::SIGNAL, _name, _access_mode, std::vector<Type>(_size, _def_value)), m_Dirty(true) {}

    ~CCustomBase64Signal() {
        /*		CDataManager * man = CDataManager::GetInstance();
		if(man)
			man->UnRegisterSignal(this->GetName());*/
    }

    JSONNode GetJSONObject() {
        JSONNode n(JSON_NODE);
        n.set_name(this->m_Value.name);
        n.push_back(JSONNode("size", this->m_Value.value.size()));
        n.push_back(JSONNode("type", typeid(Type).name()));

        JSONNode child(JSON_STRING);
        child.set_name("value");
        child.set_binary(reinterpret_cast<const unsigned char*>(this->m_Value.value.data()), this->m_Value.value.size() * sizeof(Type));
        n.push_back(child);
        return n;
    }

    const Type& operator[](int _index) const { return this->m_Value.value.at(_index); }

    Type& operator[](int _index) {
        m_Dirty = true;
        return this->m_Value.value.at(_index);
    }

    void Set(const std::vector<Type>& _value) {
        m_Dirty = true;
        this->m_Value.value = _value;
    }

    void Set(const Type* _value, size_t _size) {
        m_Dirty = true;
        this->m_Value.value.assign(_value, _value + _size);
    }

    void Resize(int _new_size) {
        m_Dirty = true;
        this->m_Value.value.resize(_new_size);
    }

    void Reserve(int _new_size) { this->m_Value.value.reserve(_new_size); }

    void Update() { m_Dirty = false; }

    int GetSize() { return this->m_Value.value.size(); }

    std::vector<Type> GetData() { return this->m_Value.value; }

    std::vector<Type>* GetDataPtr() { return &this->m_Value.value; }

    bool IsValueChanged() const { return m_Dirty; }

    bool ForceSend() {
        m_Dirty = true;
        return true;
    }

    size_t GetSizeInBytes() { return GetSize() * sizeof(Type); }

   private:
    bool m_Dirty;
};

template <typename Type>
class CCustomBinarySignal : public CParameter<Type, std::vector<Type>> {
   public:
    CCustomBinarySignal(std::string _name, int _size, Type _def_value)
        : CParameter<Type, std::vector<Type>>(CParameter<Type, Type>::BIN_SIGNAL, _name, CBaseParameter::RO, std::vector<Type>(_size, _def_value)), m_Dirty(true) {}

    CCustomBinarySignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, Type _def_value)
        : CParameter<Type, std::vector<Type>>(CParameter<Type, Type>::BIN_SIGNAL, _name, _access_mode, std::vector<Type>(_size, _def_value)), m_Dirty(true) {}

    ~CCustomBinarySignal() {
        /*		CDataManager * man = CDataManager::GetInstance();
		if(man)
			man->UnRegisterSignal(this->GetName());*/
    }

    JSONNode GetJSONObject() {
        JSONNode n(JSON_NODE);
        return n;
    }

    const Type& operator[](int _index) const { return this->m_Value.value.at(_index); }

    Type& operator[](int _index) {
        m_Dirty = true;
        return this->m_Value.value.at(_index);
    }

    void Set(const std::vector<Type>& _value) {
        m_Dirty = true;
        this->m_Value.value = _value;
    }

    void Set(const Type* _value, size_t _size) {
        m_Dirty = true;
        this->m_Value.value.assign(_value, _value + _size);
    }

    void Resize(int _new_size) {
        m_Dirty = true;
        this->m_Value.value.resize(_new_size);
    }

    void Reserve(int _new_size) { this->m_Value.value.reserve(_new_size); }

    void Update() { m_Dirty = false; }

    int GetSize() { return this->m_Value.value.size(); }

    std::vector<Type> GetData() { return this->m_Value.value; }

    std::vector<Type>* GetDataPtr() { return &this->m_Value.value; }

    const void* GetDataVoidPtr() { return this->m_Value.value.data(); }

    bool IsValueChanged() const { return m_Dirty; }

    bool ForceSend() {
        m_Dirty = true;
        return true;
    }

    CBaseParameter::BinarySignalType GetDataType() {
        if constexpr (std::is_same_v<Type, int8_t>) {
            return CBaseParameter::BinarySignalType::INT8;
        } else if constexpr (std::is_same_v<Type, int16_t>) {
            return CBaseParameter::BinarySignalType::INT16;
        } else if constexpr (std::is_same_v<Type, int32_t>) {
            return CBaseParameter::BinarySignalType::INT32;
        } else if constexpr (std::is_same_v<Type, uint8_t>) {
            return CBaseParameter::BinarySignalType::UINT8;
        } else if constexpr (std::is_same_v<Type, uint16_t>) {
            return CBaseParameter::BinarySignalType::UINT16;
        } else if constexpr (std::is_same_v<Type, uint32_t>) {
            return CBaseParameter::BinarySignalType::UINT32;
        } else if constexpr (std::is_same_v<Type, float>) {
            return CBaseParameter::BinarySignalType::FLOAT;
        } else if constexpr (std::is_same_v<Type, double>) {
            return CBaseParameter::BinarySignalType::DOUBLE;
        } else {
            return CBaseParameter::BinarySignalType::UNDEFINED;
        }
    }

    size_t GetSizeInBytes() { return GetSize() * sizeof(Type); }

   private:
    bool m_Dirty;
};

//custom CIntParameter
class CIntParameter : public CCustomParameter<int> {
   public:
    CIntParameter(std::string _name, CBaseParameter::AccessMode _access_mode, int _value, int _fpga_update, int _min, int _max)
        : CCustomParameter(_name, _access_mode, _value, _fpga_update, _min, _max){};

    CIntParameter(std::string _name, CBaseParameter::AccessMode _access_mode, int _value, int _fpga_update, int _min, int _max, int _tag)
        : CCustomParameter(_name, _access_mode, _value, _fpga_update, _min, _max, _tag){};
};

//custom CIntParameter
class CUIntParameter : public CCustomParameter<uint32_t> {
   public:
    CUIntParameter(std::string _name, CBaseParameter::AccessMode _access_mode, uint32_t _value, int _fpga_update, uint32_t _min, uint32_t _max)
        : CCustomParameter(_name, _access_mode, _value, _fpga_update, _min, _max){};

    CUIntParameter(std::string _name, CBaseParameter::AccessMode _access_mode, uint32_t _value, int _fpga_update, uint32_t _min, uint32_t _max, int _tag)
        : CCustomParameter(_name, _access_mode, _value, _fpga_update, _min, _max, _tag){};
};

//custom CFloatParameter
class CFloatParameter : public CCustomParameter<float> {
   public:
    CFloatParameter(std::string _name, CBaseParameter::AccessMode _access_mode, float _value, int _fpga_update, float _min, float _max)
        : CCustomParameter(_name, _access_mode, _value, _fpga_update, _min, _max){};

    CFloatParameter(std::string _name, CBaseParameter::AccessMode _access_mode, float _value, int _fpga_update, float _min, float _max, int _tag)
        : CCustomParameter(_name, _access_mode, _value, _fpga_update, _min, _max, _tag){};
};

//custom CDoubleParameter
class CDoubleParameter : public CCustomParameter<double> {
   public:
    CDoubleParameter(std::string _name, CBaseParameter::AccessMode _access_mode, double _value, int _fpga_update, double _min, double _max)
        : CCustomParameter(_name, _access_mode, _value, _fpga_update, _min, _max){};

    CDoubleParameter(std::string _name, CBaseParameter::AccessMode _access_mode, double _value, int _fpga_update, double _min, double _max, int _tag)
        : CCustomParameter(_name, _access_mode, _value, _fpga_update, _min, _max, _tag){};
};

//custom CBooleanParameter
class CBooleanParameter : public CCustomParameter<bool> {
   public:
    CBooleanParameter(std::string _name, CBaseParameter::AccessMode _access_mode, bool _value, int _fpga_update)
        : CCustomParameter(_name, _access_mode, _value, _fpga_update, false, true){};

    CBooleanParameter(std::string _name, CBaseParameter::AccessMode _access_mode, bool _value, int _fpga_update, int _tag)
        : CCustomParameter(_name, _access_mode, _value, _fpga_update, false, true, _tag){};
};

//custom CStringParameter
class CStringParameter : public CCustomParameter<std::string> {
   public:
    CStringParameter(std::string _name, CBaseParameter::AccessMode _access_mode, std::string _value, int _fpga_update)
        : CCustomParameter(_name, _access_mode, _value, _fpga_update, "", ""){};

    CStringParameter(std::string _name, CBaseParameter::AccessMode _access_mode, std::string _value, int _fpga_update, int _tag)
        : CCustomParameter(_name, _access_mode, _value, _fpga_update, "", "", _tag){};

    void Set(const std::string& _value) { this->m_Value.value = _value; }

    void SendValue(const std::string& _value) {
        this->m_Value.value = _value;
        m_NeedSend = true;
    }
};

//custom CIntSignal
class CIntSignal : public CCustomSignal<int> {
   public:
    CIntSignal(std::string _name, int _size, int _def_value) : CCustomSignal(_name, _size, _def_value){};

    CIntSignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, int _def_value) : CCustomSignal(_name, _access_mode, _size, _def_value){};
};

//custom CByteSignal
class CByteSignal : public CCustomSignal<uint8_t> {
   public:
    CByteSignal(std::string _name, int _size, uint8_t _def_value) : CCustomSignal(_name, _size, _def_value){};

    CByteSignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, uint8_t _def_value) : CCustomSignal(_name, _access_mode, _size, _def_value){};
};

//custom CFloatSignal
class CFloatSignal : public CCustomSignal<float> {
   public:
    CFloatSignal(std::string _name, int _size, float _def_value) : CCustomSignal(_name, _size, _def_value){};

    CFloatSignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, float _def_value) : CCustomSignal(_name, _access_mode, _size, _def_value){};
};

//custom CDoubleSignal
class CDoubleSignal : public CCustomSignal<double> {
   public:
    CDoubleSignal(std::string _name, int _size, double _def_value) : CCustomSignal(_name, _size, _def_value){};

    CDoubleSignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, double _def_value) : CCustomSignal(_name, _access_mode, _size, _def_value){};
};

class CIntBase64Signal : public CCustomBase64Signal<int> {
   public:
    CIntBase64Signal(std::string _name, int _size, float _def_value) : CCustomBase64Signal(_name, _size, _def_value){};

    CIntBase64Signal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, float _def_value) : CCustomBase64Signal(_name, _access_mode, _size, _def_value){};
};

class CByteBase64Signal : public CCustomBase64Signal<uint8_t> {
   public:
    CByteBase64Signal(std::string _name, int _size, float _def_value) : CCustomBase64Signal(_name, _size, _def_value){};

    CByteBase64Signal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, float _def_value) : CCustomBase64Signal(_name, _access_mode, _size, _def_value){};
};

class CFloatBase64Signal : public CCustomBase64Signal<float> {
   public:
    CFloatBase64Signal(std::string _name, int _size, float _def_value) : CCustomBase64Signal(_name, _size, _def_value){};

    CFloatBase64Signal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, float _def_value) : CCustomBase64Signal(_name, _access_mode, _size, _def_value){};
};

class CDoubleBase64Signal : public CCustomBase64Signal<double> {
   public:
    CDoubleBase64Signal(std::string _name, int _size, float _def_value) : CCustomBase64Signal(_name, _size, _def_value){};

    CDoubleBase64Signal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, float _def_value) : CCustomBase64Signal(_name, _access_mode, _size, _def_value){};
};

class CInt8BinarySignal : public CCustomBinarySignal<int8_t> {
   public:
    CInt8BinarySignal(std::string _name, int _size, float _def_value) : CCustomBinarySignal(_name, _size, _def_value){};

    CInt8BinarySignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, float _def_value) : CCustomBinarySignal(_name, _access_mode, _size, _def_value){};
};

class CUInt8BinarySignal : public CCustomBinarySignal<uint8_t> {
   public:
    CUInt8BinarySignal(std::string _name, int _size, float _def_value) : CCustomBinarySignal(_name, _size, _def_value){};

    CUInt8BinarySignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, float _def_value) : CCustomBinarySignal(_name, _access_mode, _size, _def_value){};
};

class CInt16BinarySignal : public CCustomBinarySignal<int16_t> {
   public:
    CInt16BinarySignal(std::string _name, int _size, float _def_value) : CCustomBinarySignal(_name, _size, _def_value){};

    CInt16BinarySignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, float _def_value) : CCustomBinarySignal(_name, _access_mode, _size, _def_value){};
};

class CUInt16BinarySignal : public CCustomBinarySignal<uint16_t> {
   public:
    CUInt16BinarySignal(std::string _name, int _size, float _def_value) : CCustomBinarySignal(_name, _size, _def_value){};

    CUInt16BinarySignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, float _def_value) : CCustomBinarySignal(_name, _access_mode, _size, _def_value){};
};

class CInt32BinarySignal : public CCustomBinarySignal<int32_t> {
   public:
    CInt32BinarySignal(std::string _name, int _size, float _def_value) : CCustomBinarySignal(_name, _size, _def_value){};

    CInt32BinarySignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, float _def_value) : CCustomBinarySignal(_name, _access_mode, _size, _def_value){};
};

class CUInt32BinarySignal : public CCustomBinarySignal<uint32_t> {
   public:
    CUInt32BinarySignal(std::string _name, int _size, float _def_value) : CCustomBinarySignal(_name, _size, _def_value){};

    CUInt32BinarySignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, float _def_value) : CCustomBinarySignal(_name, _access_mode, _size, _def_value){};
};

class CFloatBinarySignal : public CCustomBinarySignal<float> {
   public:
    CFloatBinarySignal(std::string _name, int _size, float _def_value) : CCustomBinarySignal(_name, _size, _def_value){};

    CFloatBinarySignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, float _def_value) : CCustomBinarySignal(_name, _access_mode, _size, _def_value){};
};

class CDoubleBinarySignal : public CCustomBinarySignal<double> {
   public:
    CDoubleBinarySignal(std::string _name, int _size, float _def_value) : CCustomBinarySignal(_name, _size, _def_value){};

    CDoubleBinarySignal(std::string _name, CBaseParameter::AccessMode _access_mode, int _size, float _def_value) : CCustomBinarySignal(_name, _access_mode, _size, _def_value){};
};

extern CBooleanParameter IsDemoParam;     // special default parameter to check mode (demo or not)
extern CStringParameter InCommandParam;   // special default parameter to receive a string command from WEB UI
extern CStringParameter OutCommandParam;  // special default parameter to send a string command to WEB UI
