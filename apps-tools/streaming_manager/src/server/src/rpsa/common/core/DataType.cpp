#include <ctime>
#include <cmath>
#include <iostream>
#include <ctime>
#include <cstring>
#include <sstream>
#include <locale>
#include <iomanip>
#include "types.h"
#include "rpsa/common/core/DataType.h"

namespace {
static time_t GetTime1904()
{
    std::stringstream stream("1904-01-01 00:00:00");
    stream.imbue(std::locale::classic());

    std::tm time_point;
    std::memset(&time_point, 0, sizeof(std::tm));

    stream >> std::get_time(&time_point, "%Y-%m-%d %H:%M:%S");
    return std::mktime(&time_point);
}

static const time_t time_1904 = GetTime1904();
}

namespace TDMS {



	DataType::DataType():
    m_dataType(-1),
    m_dataStringLenght(-1),
    m_rawData(nullptr),
    m_vectorData()
	{
		
	}

	DataType::DataType(const DataType& tmp) {
	    m_rawData = nullptr;
		m_dataType = tmp.m_dataType;
		m_dataStringLenght = tmp.m_dataStringLenght;
		uint32_t bufSize = GetLength();
		if (tmp.m_rawData!= nullptr) {
            m_rawData = new char[bufSize];
            memcpy(m_rawData, tmp.m_rawData, bufSize);
        }
		m_vectorData = tmp.m_vectorData;
	}

	DataType& DataType::operator=(const DataType& tmp) {
        m_rawData = nullptr;
		m_dataType = tmp.m_dataType;
		m_dataStringLenght = tmp.m_dataStringLenght;
		uint32_t bufSize = GetLength();
        if (tmp.m_rawData!= nullptr) {
            m_rawData = new char[bufSize];
            memcpy(m_rawData, tmp.m_rawData, bufSize);
        }
		m_vectorData = tmp.m_vectorData;
		return *this;
	}

	DataType::DataType(DataType&& tmp) {
		m_dataType = tmp.m_dataType;
		m_rawData = tmp.m_rawData;
		m_dataStringLenght = tmp.m_dataStringLenght;
		tmp.m_rawData = nullptr;
		tmp.m_dataType = DataType::Empty;
		m_vectorData = tmp.m_vectorData;
		tmp.m_vectorData.clear();
	}

	DataType& DataType::operator=(DataType&& tmp) {
		this->~DataType();
		m_dataType = tmp.m_dataType;
		m_rawData = tmp.m_rawData;
		m_dataStringLenght = tmp.m_dataStringLenght;
		tmp.m_rawData = nullptr;
		m_vectorData = tmp.m_vectorData;
		tmp.m_vectorData.clear();
		return *this;
	}

	void DataType::InitDataType(uint32_t dataType, void *rawData) {
		m_dataType = dataType;
		m_rawData = rawData;
	}

    void DataType::InitDataType(uint32_t dataType, std::vector<std::shared_ptr<DataType::Raw>> vec){
        m_dataType = dataType;
        m_vectorData = vec;
	}

	void DataType::InitStringType(uint32_t length, void *rawData) {
		m_dataType = DataType::String;
		m_dataStringLenght = length;
		m_rawData = rawData;
	}

    void DataType::InitRaw(uint32_t dataType,uint64_t count,void *rawData){
        m_dataType = dataType;
        std::shared_ptr<Raw> raw = std::make_shared<Raw>();
        raw->data = (uint8_t*)rawData;
        raw->size = count * GetLength();
        this->m_vectorData.push_back(raw);
	}

	DataType::~DataType()
	{
	    if (m_rawData != nullptr)
    	    delete  [] m_rawData;
	}


	uint32_t DataType::GetDataType() {
		return m_dataType;

	}

    std::vector<std::shared_ptr<DataType::Raw>> DataType::GetRawVector(){
        return  m_vectorData;
	}

	const  void*    DataType::GetRawData() {
		return m_rawData;
	}


	uint32_t DataType::GetLength() {
		if (m_dataType == DataType::String)
			return this->m_dataStringLenght;
		return DataType::GetLength(this->m_dataType);
	}

	uint32_t DataType::GetLength(uint32_t dataType) {
		
		switch (dataType)
		{
			case DataType::Empty: return 0;
			case Void: return 1;
			case Integer8: return 1;
			case Integer16: return 2;
			case Integer32: return 4;
			case Integer64: return 8;
			case UnsignedInteger8: return 1;
			case UnsignedInteger16: return 2;
			case UnsignedInteger32: return 4;
			case UnsignedInteger64: return 8;
			case SingleFloat:
			case SingleFloatWithUnit: return 4;
			case DoubleFloat:
			case DoubleFloatWithUnit: return 8;
			case Boolean: return 1;
			case TimeStamp: return 16;
			case String: return -1;
			default: {
				std::string message = "Cannot determine size of data type: ";
				message += dataType;
                std::cout << "DataType Error: " << message << std::endl;
			//	throw std::invalid_argument(message.c_str());
			}
		}	
        return 0;	
	}
	
	uint64_t DataType::GetArrayLength(uint32_t dataType, uint64_t size)
	{
		return GetLength(dataType) * size;
	}

    std::string  DataType::ToString(){

        char cstr[22];

        switch (m_dataType)
        {
            case DataType::Empty: return "Empty";
            case Void: return "Void";
            case Integer8: sprintf(cstr,"%i", this->GetData<int8_t>()); break;
            case Integer16: sprintf(cstr,"%i", this->GetData<int16_t>()); break;
            case Integer32: sprintf(cstr,"%i", this->GetData<int32_t>()); break;
            case Integer64: sprintf(cstr,"%lld", this->GetData<int64_t>()); break;
            case UnsignedInteger8: sprintf(cstr,"%u", this->GetData<u_int8_t>()); break;
            case UnsignedInteger16: sprintf(cstr,"%u", this->GetData<u_int16_t>()); break;
            case UnsignedInteger32: sprintf(cstr,"%u", this->GetData<u_int32_t>()); break;
            case UnsignedInteger64: sprintf(cstr,"%llu", this->GetData<u_int64_t>()); break;
            case SingleFloat:
            case SingleFloatWithUnit: sprintf(cstr,"%f", this->GetData<float>()); break;
            case DoubleFloat:
            case DoubleFloatWithUnit:sprintf(cstr,"%lf", this->GetData<double>()); break;
            case Boolean: sprintf(cstr,"%s", this->GetData<uint8_t>()?"true":"false"); break;
            case TimeStamp:
            {
                uint64_t *t = (uint64_t*)GetRawData();
                double v1 = (double)t[0] / std::pow(2., 64.);

                std::tm time_point;
                std::memset(&time_point, 0, sizeof(std::tm));
                time_t time = t[1] + time_1904;
#ifdef _WIN32
				gmtime_s(&time_point, &time);
#else
				gmtime_r(&time, &time_point);
#endif // _WIN32

                std::stringstream stream;
                stream.imbue(std::locale::classic());
                stream << std::put_time(&time_point, "day: %d month: %m year: %Y time:%T ");

                sprintf(cstr,"%lf", v1);
                return stream.str() + std::string(cstr);

            }
            case String: {
                return GetDataString();
            }
            default: {
                std::string message = "Cannot determine of data type ";
             //   throw std::invalid_argument(message.c_str());
            }
        }
        return std::string(cstr);
	}


    std::string  DataType::ToTypeString(){


        switch (m_dataType)
        {
            case Empty: return "Empty";
            case Void: return "Void";
            case Integer8:
				return "Integer8";
            case Integer16: return "Integer16";
            case Integer32: return "Integer32";
            case Integer64: return "Integer64";
            case UnsignedInteger8: return "UInteger8";
            case UnsignedInteger16: return "UInteger16";
            case UnsignedInteger32: return "UInteger32";
            case UnsignedInteger64: return "UInteger64";
            case SingleFloat: return "SingleFloat";
            case SingleFloatWithUnit: return "SingleFloatWithUnit";
            case DoubleFloat: return "DoubleFloat";
            case DoubleFloatWithUnit: return "DoubleFloatWithUnit";
            case Boolean: return "Boolean";
            case TimeStamp: return "TimeStamp";

            case String: return "String";

        }
        return "Error";
    }

    void DataType::PrintVector(int limitDataSize){

		int i =0;
	    for(auto &r : m_vectorData){
            if (m_dataType == DataType::String){
				printf("\t\t\t Raw val[%i]:",i++);
                for(int j=0;j<r->size;j++)
                {
                    printf("%c",r->data[j]);
                }
                printf("\n");
            }
            else
            {
                printf("\t\t\t Raw val[%i]:\n",i++);
                bool Trunc = false;
                long DataSize = this->GetLength();
                if (m_dataType == DataType::TimeStamp)
                    DataSize /= 2;
                long count = r->size / DataSize;
                if (limitDataSize!= -1){
                    if (count > limitDataSize) {
                        count = limitDataSize;
                        Trunc = true;
                    }
                }

                for(int j=0;j<count;j++)
                {
                    char cstr[22];

                    printf("\t");
                    switch (m_dataType) {
                        case DataType::Empty:
                            printf("\t\t\t- Empty\n");
                            break;
                        case Void:
                            printf("\t\t\t- Void\n");
                            break;
                        case Integer8:
                            printf("\t\t\t- %i\n",((int8_t*)r->data)[j]);
                            break;
                        case Integer16:
                            printf("\t\t\t- %i\n",((int16_t*)r->data)[j]);
                            break;
                        case Integer32:
                            printf("\t\t\t- %i\n",((int32_t*)r->data)[j]);
                            break;
                        case Integer64:
                            printf("\t\t\t- %lld\n",((int64_t*)r->data)[j]);
                            break;
                        case UnsignedInteger8:
                            printf("\t\t\t- %u\n",((u_int8_t *)r->data)[j]);
                            break;
                        case UnsignedInteger16:
                            printf("\t\t\t- %u\n",((u_int16_t *)r->data)[j]);
                            break;
                        case UnsignedInteger32:
                            printf("\t\t\t- %u\n",((u_int32_t *)r->data)[j]);
                            break;
                        case UnsignedInteger64:
                            printf("\t\t\t- %llu\n",((u_int64_t *)r->data)[j]);
                            break;
                        case SingleFloat:
                        case SingleFloatWithUnit:
                            printf("\t\t\t- %f\n",((float_t *)r->data)[j]);
                            break;
                        case DoubleFloat:
                        case DoubleFloatWithUnit:
                            printf("\t\t\t- %lf\n",((double_t *)r->data)[j]);
                            break;
                        case Boolean:
                            printf("\t\t\t- %s\n",(((uint8_t*)r->data)[j]?"true" : "false"));
                            break;
                        case TimeStamp: {
                            uint64_t t1  =  ((uint64_t *)r->data)[j++];
                            uint64_t v2  =  ((uint64_t *)r->data)[j];
                            double v1 = (double) t1 / std::pow(2., 64.);

                            std::tm time_point;
                            std::memset(&time_point, 0, sizeof(std::tm));
                            time_t time = v2 + time_1904;
#ifdef _WIN32
							gmtime_s(&time_point, &time);
#else
							gmtime_r(&time, &time_point);
#endif // _WIN32

                            std::stringstream stream;
                            stream.imbue(std::locale::classic());
                            stream << std::put_time(&time_point, "day: %d month: %m year: %Y time:%T ");

                            sprintf(cstr, "%lf", v1);
                            printf("\t\t\t- %s\n",(stream.str() + std::string(cstr)).c_str());
                            break;

                        }
                    }
                }
                if (Trunc){
                    printf("\t\t\t- ........\n");
                }
            }

        }
	}

    uint64_t*  DataType::GetRawTimeValue(time_t time_val){
	    uint64_t  *val = new uint64_t[2];
	    val[0] = 0; // Subseconds
	    val[1] = time_val - time_1904;
        return val;
	}
}
