#pragma once
#pragma GCC diagnostic ignored "-Wpedantic"

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
	class DataType
	{
	public:
	    struct Raw{
	        uint8_t *data;
	        long     size;
            Raw():
			data(nullptr),
			size(0){                
            }

	        ~Raw(){
            	delete []  data;
	        }
	    };

		static const uint32_t Empty = 0x0000000F;
		static const uint32_t Void = 0x00000000;
		static const uint32_t Integer8 = 0x00000001;
		static const uint32_t Integer16 = 0x00000002;
		static const uint32_t Integer32 = 0x00000003;
		static const uint32_t Integer64 = 0x00000004;
		static const uint32_t UnsignedInteger8 = 0x00000005;
		static const uint32_t UnsignedInteger16 = 0x00000006;
		static const uint32_t UnsignedInteger32 = 0x00000007;
		static const uint32_t UnsignedInteger64 = 0x00000008;
		static const uint32_t SingleFloat = 0x00000009;
		static const uint32_t DoubleFloat = 0x0000000A;
		static const uint32_t ExtendedFloat = 0x0000000B;
		static const uint32_t SingleFloatWithUnit = 0x00000019;
		static const uint32_t DoubleFloatWithUnit = 0x0000001A;
		static const uint32_t ExtendedFloatWithUnit = 0x0000001B;
		static const uint32_t String = 0x00000020;
		static const uint32_t Boolean = 0x00000021;
		static const uint32_t TimeStamp = 0x00000044;
	
	protected:
		uint32_t m_dataType;
		uint32_t m_dataStringLenght;
		void *m_rawData;
        std::vector<std::shared_ptr<DataType::Raw>> m_vectorData;
	protected:

	public:

		DataType();
		~DataType();
        void DestroyVector();
		DataType(const DataType& tmp);
		DataType& operator=(const DataType& tmp);
		DataType(DataType&& tmp);
		DataType& operator=(DataType&& tmp);

		           void InitDataType(uint32_t dataType, void *rawData);
                   void InitDataType(uint32_t dataType, std::vector<std::shared_ptr<DataType::Raw>> vec);
				   void InitStringType(uint32_t length, void *rawData);
				   void InitRaw(uint32_t dataType,uint64_t count,void *rawData);
		   	   uint32_t GetDataType();
           std::string  ToString();
           std::string  ToTypeString();
        std::vector<std::shared_ptr<DataType::Raw>> GetRawVector();
        const  void*    GetRawData();
		template<typename T>
					  T GetData() {
						  T* val = (T*)m_rawData;
						  return val[0];
					  };

        template<typename T>
        static    void* MakeData(T value){
                        uint8_t *buff = new uint8_t[sizeof(T)];
                        memcpy(buff,&value,sizeof(T));
                        return  buff;
                      };

			std::string GetDataString() {
						  std::string str = std::string((char*)m_rawData,m_dataStringLenght);
						  return str;
					  };

			       void PrintVector(int limitDataSize);


		       uint32_t   GetLength();
		static uint32_t   GetLength(uint32_t dataType);
		static uint64_t   GetArrayLength(uint32_t dataType, uint64_t size);
        static uint64_t*  GetRawTimeValue(time_t time_val);

	};
}
