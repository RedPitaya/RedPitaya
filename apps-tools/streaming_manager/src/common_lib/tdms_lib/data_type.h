#ifndef TDMS_LIB_DATA_TYPE_H
#define TDMS_LIB_DATA_TYPE_H


#include <stdint.h>
#include <vector>
#include <stdexcept>
#include <string>
#include <cstring>
#include <memory>
#include <iostream>

using namespace std;

namespace TDMS
{
    enum class TDMSType{
        Empty                   = 0x0000000F,
        Void                    = 0x00000000,
        Integer8                = 0x00000001,
        Integer16               = 0x00000002,
        Integer32               = 0x00000003,
        Integer64               = 0x00000004,
        UnsignedInteger8        = 0x00000005,
        UnsignedInteger16       = 0x00000006,
        UnsignedInteger32       = 0x00000007,
        UnsignedInteger64       = 0x00000008,
        SingleFloat             = 0x00000009,
        DoubleFloat             = 0x0000000A,
        ExtendedFloat           = 0x0000000B,
        SingleFloatWithUnit     = 0x00000019,
        DoubleFloatWithUnit     = 0x0000001A,
        ExtendedFloatWithUnit   = 0x0000001B,
        String                  = 0x00000020,
        Boolean                 = 0x00000021,
        TimeStamp               = 0x00000044
    };

	class DataType
	{
    	public:
            struct Raw{
                std::shared_ptr<uint8_t[]> data;
                uint64_t size;
                TDMSType dataType;
                Raw():data(nullptr),size(0), dataType(TDMSType::Empty){}
                ~Raw();
            };

    	protected:
	        TDMSType m_dataType;
		    uint32_t m_dataStringLenght;
		    void*    m_rawData;
            vector<shared_ptr<DataType::Raw>> m_vectorData;

	    public:
            DataType();
            ~DataType() noexcept(false);
            DataType(const DataType& tmp);

            DataType& operator=(const DataType& tmp);
            DataType(DataType&& tmp);
            DataType& operator=(DataType&& tmp);

            auto InitDataType(TDMSType dataType, void *rawData) -> void;
            auto InitDataType(TDMSType dataType, std::vector<std::shared_ptr<DataType::Raw>> vec)  -> void;
            auto InitStringType(uint32_t length, void *rawData)  -> void;
            auto InitRaw(TDMSType dataType,uint64_t count,std::shared_ptr<uint8_t[]> rawData)  -> void;

            auto GetDataType() -> TDMSType;
            auto ToString() -> string;
            auto ToTypeString() -> string;
            auto GetRawVector() -> vector<shared_ptr<DataType::Raw>>;
            auto GetRawData() const -> void*;
            auto GetDataString() -> string;
            auto GetLength() -> uint32_t;
            auto PrintVector(int limitDataSize) -> void;

    		template<typename T>
    		auto GetData() -> T {
			    T* val = static_cast<T*>(m_rawData);
				 return val[0];
			}

            template<typename T>
            static auto MakeData(T value) -> void*{
                uint8_t *buff = new uint8_t[sizeof(T)];
                memcpy(buff,&value,sizeof(T));
                return  buff;
            }

            static auto GetLength(TDMSType dataType) -> uint32_t;
    		static auto GetArrayLength(TDMSType dataType, uint64_t size) -> uint64_t;
    		static auto GetRawTimeValue(time_t time_val) -> uint64_t* ;
	};
}

#endif
