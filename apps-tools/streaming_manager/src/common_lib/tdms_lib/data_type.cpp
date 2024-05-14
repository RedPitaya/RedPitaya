#include <ctime>
#include <cmath>
#include <iostream>
#include <ctime>
#include <cstring>
#include <sstream>
#include <locale>
#include <iomanip>
#include "data_type.h"

using namespace TDMS;

time_t rp_GetTime1904(){
    std::stringstream stream("1904-01-01 00:00:00");
    stream.imbue(std::locale::classic());
    std::tm time_point;
    std::memset(&time_point, 0, sizeof(std::tm));
    stream >> std::get_time(&time_point, "%Y-%m-%d %H:%M:%S");
    return std::mktime(&time_point);
}

const time_t time_1904 = rp_GetTime1904();

DataType::DataType():
    m_dataType(TDMSType::Empty),
    m_dataStringLenght(-1),
    m_rawData(nullptr),
    m_vectorData()
{}

DataType::DataType(const DataType& tmp){
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

DataType& DataType::operator=(const DataType& tmp){
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

DataType::DataType(DataType&& tmp){
    m_dataType = tmp.m_dataType;
    m_rawData = tmp.m_rawData;
    m_dataStringLenght = tmp.m_dataStringLenght;
    tmp.m_rawData = nullptr;
    tmp.m_dataType = TDMSType::Empty;
    m_vectorData = tmp.m_vectorData;
    tmp.m_vectorData.clear();
}

DataType& DataType::operator=(DataType&& tmp){
    this->~DataType();
    m_dataType = tmp.m_dataType;
    m_rawData = tmp.m_rawData;
    m_dataStringLenght = tmp.m_dataStringLenght;
    tmp.m_rawData = nullptr;
    m_vectorData = tmp.m_vectorData;
    tmp.m_vectorData.clear();
    return *this;
}

auto DataType::InitDataType(TDMSType dataType, void *rawData) -> void{
    m_dataType = dataType;
    m_rawData = rawData;
}

auto DataType::InitDataType(TDMSType dataType, std::vector<std::shared_ptr<DataType::Raw>> vec) -> void{
    m_dataType = dataType;
    m_vectorData = vec;
}

auto DataType::InitStringType(uint32_t length, void *rawData) -> void{
    m_dataType = TDMSType::String;
    m_dataStringLenght = length;
    m_rawData = rawData;
}

auto DataType::InitRaw(TDMSType dataType,uint64_t count,std::shared_ptr<uint8_t[]> rawData) -> void{
    m_dataType = dataType;
    std::shared_ptr<Raw> raw = std::make_shared<Raw>();
    raw->data = rawData;
    raw->size = count * GetLength();
    raw->dataType = dataType;
    this->m_vectorData.push_back(raw);
}

DataType::~DataType() noexcept(false)
{
    if (m_rawData != nullptr)
        switch (m_dataType)
        {
            case TDMSType::Empty: {
                std::string message = "Cannot delete empty type";
                throw std::runtime_error(message.c_str());
            }
            case TDMSType::Void: {
                 std::string message = "Cannot delete void type";
                throw std::runtime_error(message.c_str());
            }
            case TDMSType::Integer8:  delete[] reinterpret_cast<int8_t*>(m_rawData); break;
            case TDMSType::Integer16: delete[] reinterpret_cast<int16_t*>(m_rawData); break;
            case TDMSType::Integer32: delete[] reinterpret_cast<int32_t*>(m_rawData); break;
            case TDMSType::Integer64: delete[] reinterpret_cast<int64_t*>(m_rawData); break;
            case TDMSType::Boolean:
            case TDMSType::UnsignedInteger8: delete[] reinterpret_cast<uint8_t*>(m_rawData); break;
            case TDMSType::UnsignedInteger16: delete[] reinterpret_cast<uint16_t*>(m_rawData); break;
            case TDMSType::UnsignedInteger32: delete[] reinterpret_cast<uint32_t*>(m_rawData); break;
            case TDMSType::TimeStamp:
            case TDMSType::UnsignedInteger64: delete[] reinterpret_cast<uint64_t*>(m_rawData); break;
            case TDMSType::SingleFloat:
            case TDMSType::SingleFloatWithUnit: delete[] reinterpret_cast<float*>(m_rawData); break;
            case TDMSType::DoubleFloat:
            case TDMSType::DoubleFloatWithUnit:delete[] reinterpret_cast<double*>(m_rawData); break;
            case TDMSType::String:delete[] reinterpret_cast<char*>(m_rawData); break;
            default: {
                std::string message = "Cannot determine of data type ";
                throw std::runtime_error(message.c_str());
            }
        }
}

auto DataType::GetDataType() -> TDMSType{
    return m_dataType;
}

auto DataType::GetRawVector() -> std::vector<std::shared_ptr<DataType::Raw>>{
    return  m_vectorData;
}

auto DataType::GetRawData() const -> void*{
    return m_rawData;
}

auto DataType::GetLength() -> uint32_t {
    if (m_dataType == TDMSType::String)
        return this->m_dataStringLenght;
    return DataType::GetLength(this->m_dataType);
}

auto DataType::GetLength(TDMSType dataType) -> uint32_t{
    switch (dataType)
    {
        case TDMSType::Empty: return 0;
        case TDMSType::Void: return 1;
        case TDMSType::Integer8: return 1;
        case TDMSType::Integer16: return 2;
        case TDMSType::Integer32: return 4;
        case TDMSType::Integer64: return 8;
        case TDMSType::UnsignedInteger8: return 1;
        case TDMSType::UnsignedInteger16: return 2;
        case TDMSType::UnsignedInteger32: return 4;
        case TDMSType::UnsignedInteger64: return 8;
        case TDMSType::SingleFloat:
        case TDMSType::SingleFloatWithUnit: return 4;
        case TDMSType::DoubleFloat:
        case TDMSType::DoubleFloatWithUnit: return 8;
        case TDMSType::Boolean: return 1;
        case TDMSType::TimeStamp: return 16;
        case TDMSType::String: return -1;
        default: {
            std::string message = "Cannot determine size of data type: ";
            message += static_cast<uint32_t>(dataType);
            std::cout << "DataType Error: " << message << std::endl;
        }
    }
    return 0;
}

auto DataType::GetArrayLength(TDMSType dataType, uint64_t size) -> uint64_t{
    return GetLength(dataType) * size;
}

auto DataType::ToString() -> std::string{
    char cstr[22];
    switch (m_dataType)
    {
        case TDMSType::Empty: return "Empty";
        case TDMSType::Void: return "Void";
        case TDMSType::Integer8: sprintf(cstr,"%i", this->GetData<int8_t>()); break;
        case TDMSType::Integer16: sprintf(cstr,"%i", this->GetData<int16_t>()); break;
        case TDMSType::Integer32: sprintf(cstr,"%i", this->GetData<int32_t>()); break;
        case TDMSType::Integer64: sprintf(cstr,"%ld",(long int) this->GetData<int64_t>()); break;
        case TDMSType::UnsignedInteger8: sprintf(cstr,"%u", this->GetData<uint8_t>()); break;
        case TDMSType::UnsignedInteger16: sprintf(cstr,"%u", this->GetData<uint16_t>()); break;
        case TDMSType::UnsignedInteger32: sprintf(cstr,"%u", this->GetData<uint32_t>()); break;
        case TDMSType::UnsignedInteger64: sprintf(cstr,"%lu", (long unsigned int)this->GetData<uint64_t>()); break;
        case TDMSType::SingleFloat:
        case TDMSType::SingleFloatWithUnit: sprintf(cstr,"%f", this->GetData<float>()); break;
        case TDMSType::DoubleFloat:
        case TDMSType::DoubleFloatWithUnit:sprintf(cstr,"%lf", this->GetData<double>()); break;
        case TDMSType::Boolean: sprintf(cstr,"%s", this->GetData<uint8_t>()?"true":"false"); break;
        case TDMSType::TimeStamp:
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
        case TDMSType::String: {
            return GetDataString();
        }
        default: {
            std::string message = "Cannot determine of data type ";
        }
    }
    return std::string(cstr);
}


auto DataType::ToTypeString() -> std::string{
    switch (m_dataType)
    {
        case TDMSType::Empty: return "Empty";
        case TDMSType::Void: return "Void";
        case TDMSType::Integer8:  return "Integer8";
        case TDMSType::Integer16: return "Integer16";
        case TDMSType::Integer32: return "Integer32";
        case TDMSType::Integer64: return "Integer64";
        case TDMSType::UnsignedInteger8: return "UInteger8";
        case TDMSType::UnsignedInteger16: return "UInteger16";
        case TDMSType::UnsignedInteger32: return "UInteger32";
        case TDMSType::UnsignedInteger64: return "UInteger64";
        case TDMSType::SingleFloat: return "SingleFloat";
        case TDMSType::SingleFloatWithUnit: return "SingleFloatWithUnit";
        case TDMSType::DoubleFloat: return "DoubleFloat";
        case TDMSType::DoubleFloatWithUnit: return "DoubleFloatWithUnit";
        case TDMSType::Boolean: return "Boolean";
        case TDMSType::TimeStamp: return "TimeStamp";
        case TDMSType::String: return "String";
        default: ;
    }
    return "Error";
}

auto DataType::PrintVector(int limitDataSize) -> void{
    int i =0;
    for(auto &r : m_vectorData){
        if (m_dataType == TDMSType::String){
            printf("\t\t\t Raw val[%i]:",i++);
            for(auto j= 0u ; j < r->size ; j++){
                printf("%c",r->data.get()[j]);
            }
            printf("\n");
        }
        else
        {
            printf("\t\t\t Raw val[%i]:\n",i++);
            bool Trunc = false;
            long DataSize = this->GetLength();
            if (m_dataType == TDMSType::TimeStamp)
                DataSize /= 2;
            long count = r->size / DataSize;
            if (limitDataSize!= -1){
                if (count > limitDataSize) {
                    count = limitDataSize;
                    Trunc = true;
                }
            }
            for(int j=0 ; j<count ; j++){
                char cstr[22];
                printf("\t");
                switch (m_dataType) {
                    case TDMSType::Empty:
                        printf("\t\t\t- Empty\n");
                        break;
                    case TDMSType::Void:
                        printf("\t\t\t- Void\n");
                        break;
                    case TDMSType::Integer8:
                        printf("\t\t\t- %i\n",((int8_t*)r->data.get())[j]);
                        break;
                    case TDMSType::Integer16:
                        printf("\t\t\t- %i\n",((int16_t*)r->data.get())[j]);
                        break;
                    case TDMSType::Integer32:
                        printf("\t\t\t- %i\n",((int32_t*)r->data.get())[j]);
                        break;
                    case TDMSType::Integer64:
                        printf("\t\t\t- %ld\n",(long int)((int64_t*)r->data.get())[j]);
                        break;
                    case TDMSType::UnsignedInteger8:
                        printf("\t\t\t- %u\n",((uint8_t *)r->data.get())[j]);
                        break;
                    case TDMSType::UnsignedInteger16:
                        printf("\t\t\t- %u\n",((uint16_t *)r->data.get())[j]);
                        break;
                    case TDMSType::UnsignedInteger32:
                        printf("\t\t\t- %u\n",((uint32_t *)r->data.get())[j]);
                        break;
                    case TDMSType::UnsignedInteger64:
                        printf("\t\t\t- %lu\n",(long unsigned int)((uint64_t *)r->data.get())[j]);
                        break;
                    case TDMSType::SingleFloat:
                    case TDMSType::SingleFloatWithUnit:
                        printf("\t\t\t- %f\n",((float_t *)r->data.get())[j]);
                        break;
                    case TDMSType::DoubleFloat:
                    case TDMSType::DoubleFloatWithUnit:
                        printf("\t\t\t- %lf\n",((double_t *)r->data.get())[j]);
                        break;
                    case TDMSType::Boolean:
                        printf("\t\t\t- %s\n",(((uint8_t*)r->data.get())[j]?"true" : "false"));
                        break;
                    case TDMSType::TimeStamp: {
                        uint64_t t1  =  ((uint64_t *)r->data.get())[j++];
                        uint64_t v2  =  ((uint64_t *)r->data.get())[j];
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
                    default:
                        break;
                }
            }
            if (Trunc){
                printf("\t\t\t- ........\n");
            }
        }

    }
}

auto DataType::GetRawTimeValue(time_t time_val) -> uint64_t*{
    uint64_t  *val = new uint64_t[2];
    val[0] = 0; // Subseconds
    val[1] = time_val - time_1904;
    return val;
}

TDMS::DataType::Raw::~Raw(){
}

auto DataType::GetDataString() -> string{
    return string((char*)m_rawData,m_dataStringLenght);
}

