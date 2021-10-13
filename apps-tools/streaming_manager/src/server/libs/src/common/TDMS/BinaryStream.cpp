#include "BinaryStream.h"

using namespace  TDMS;

BinaryStream::BinaryStream()
{}

BinaryStream::~BinaryStream()
{}

auto BinaryStream::ReadLengthPrefixedString(iostream &reader) -> DataType{
    return  BinaryStream::Read(reader, TDMSType::String);
}


auto BinaryStream::ReadString(iostream &reader, int length) -> DataType{
    DataType datatype;
    uint8_t *buffer = new uint8_t[length];
    try
    {
        reader.read((char*)buffer, length);
        datatype.InitStringType(length, buffer);
        return datatype;
    }
    catch (std::exception e) {
        free(buffer);
        cout << e.what() << endl;
    }
    return datatype;
}

auto BinaryStream::Read(iostream &reader, TDMSType dataType) -> DataType{
    DataType data;
    switch (dataType)
    {
        case TDMSType::Empty:
            return data;
        case TDMSType::Void:
            reader.peek();
            return data;
        case TDMSType::Boolean:
        case TDMSType::Integer8:
        case TDMSType::Integer16:
        case TDMSType::Integer32:
        case TDMSType::Integer64:
        case TDMSType::UnsignedInteger8:
        case TDMSType::UnsignedInteger16:
        case TDMSType::UnsignedInteger32:
        case TDMSType::UnsignedInteger64:
        case TDMSType::SingleFloat:
        case TDMSType::SingleFloatWithUnit:
        case TDMSType::DoubleFloat:
        case TDMSType::DoubleFloatWithUnit:
        case TDMSType::TimeStamp:
        {
            uint32_t length = DataType::GetLength(dataType);
            char *buff = new char[length];
            reader.read(buff, length);
            data.InitDataType(dataType, buff);
            return data;
        }
        case TDMSType::String:
        {
            DataType dataPrefix = BinaryStream::Read(reader, TDMSType::Integer32);
            uint32_t prefix = dataPrefix.GetData<uint32_t>();
            DataType stringData = BinaryStream::ReadString(reader, prefix);
            return stringData;
        }
        default: {
            std::string message = "Cannot determine size of data type: ";
            message += static_cast<uint32_t>(dataType);
        }
    }
    return DataType();
}

auto BinaryStream::ReadArray(iostream &reader, long size,int offset) -> uint8_t*{
    uint8_t *buff = new uint8_t[size];
    std::ios::pos_type pos = reader.tellg();
    reader.seekg(offset, ios::beg);
    reader.read((char*)buff,size);
    reader.seekg(pos);
    return buff;
}

auto BinaryStream::ReadArray(iostream &reader, long dataSize, long Count ,int offset,int interleaveSkip) -> uint8_t*{
    uint8_t *buff = new uint8_t[dataSize * Count];
    std::ios::pos_type pos = reader.tellg();
    reader.seekg(offset, ios::beg);
    for(long x = 0 ; x < Count ; x++) {
        reader.read((char *) buff, dataSize);
        reader.seekg(interleaveSkip,ios::cur);
    }
    reader.seekg(pos);
    return buff;
}

auto BinaryStream::Write(iostream &writer,DataType& data) -> void{
    switch (data.GetDataType())
    {
        case TDMSType::Empty: break;
        case TDMSType::Void:
        {
            uint8_t  buf = 0;
            writer.write((char*)&buf, sizeof(buf));
            break;
        }
        case TDMSType::Boolean:
        case TDMSType::Integer8:
        case TDMSType::Integer16:
        case TDMSType::Integer32:
        case TDMSType::Integer64:
        case TDMSType::UnsignedInteger8:
        case TDMSType::UnsignedInteger16:
        case TDMSType::UnsignedInteger32:
        case TDMSType::UnsignedInteger64:
        case TDMSType::SingleFloat:
        case TDMSType::SingleFloatWithUnit:
        case TDMSType::DoubleFloat:
        case TDMSType::DoubleFloatWithUnit:
        case TDMSType::TimeStamp: {
            writer.write((char*)data.GetRawData(), data.GetLength());
            break;
        }
        case TDMSType::String:
        {
            uint32_t strLen = data.GetLength();
            writer.write((char *)&strLen, sizeof(strLen));
            writer.write((char *)data.GetRawData(), data.GetLength());
            break;
        }
        default: {
            std::string message = "Cannot determine size of data type: ";
            message += static_cast<uint32_t>(data.GetDataType());
            throw std::invalid_argument(message.c_str());
        }
    }
}