#include "rpsa/common/core/BinaryStream.h"

namespace TDMS
{
	BinaryStream::BinaryStream()
	{
	}

	BinaryStream::~BinaryStream()
	{

	}

	DataType BinaryStream::ReadLengthPrefixedString(iostream &reader)
	{
		return  BinaryStream::Read(reader, DataType::String);
	}


	DataType BinaryStream::ReadString(iostream &reader, int length)
	{
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

	DataType BinaryStream::Read(iostream &reader, int dataType)
	{
		DataType data;
		switch (dataType)
		{
			case DataType::Empty: 
				return data;
			case DataType::Void: 
				reader.peek();
				return data;
			case DataType::Boolean:
			case DataType::Integer8: 
			case DataType::Integer16:
			case DataType::Integer32:
			case DataType::Integer64:
			case DataType::UnsignedInteger8:
			case DataType::UnsignedInteger16:
			case DataType::UnsignedInteger32:
			case DataType::UnsignedInteger64:
			case DataType::SingleFloat:
			case DataType::SingleFloatWithUnit:
			case DataType::DoubleFloat:
			case DataType::DoubleFloatWithUnit:
			case DataType::TimeStamp:
			{
				uint32_t length = DataType::GetLength(dataType);
				char *buff = new char[length];
				reader.read(buff, length);
				data.InitDataType(dataType, buff);
				return data;
			}
			case DataType::String: 
			{
				DataType dataPrefix = BinaryStream::Read(reader, DataType::Integer32);
				uint32_t prefix = dataPrefix.GetData<uint32_t>();
				DataType stringData = BinaryStream::ReadString(reader, prefix);
				return stringData;
			}
			default: {
				std::string message = "Cannot determine size of data type: ";
				message += dataType;
			//	throw std::invalid_argument(message.c_str());
			}
		}
		return DataType();
	}

	uint8_t* BinaryStream::ReadArray(iostream &reader, long size,int offset){
		uint8_t *buff = new uint8_t[size];
		std::ios::pos_type pos = reader.tellg();
		reader.seekg(offset, ios::beg);
		reader.read((char*)buff,size);
		reader.seekg(pos);
		return buff;
	}

	uint8_t* BinaryStream::ReadArray(iostream &reader, long dataSize, long Count ,int offset,int interleaveSkip){
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

	void    BinaryStream::Write(iostream &writer,DataType& data) {
		switch (data.GetDataType())
		{
			case DataType::Empty: break;
			case DataType::Void:
			{
				uint8_t  buf = 0;
				writer.write((char*)&buf, sizeof(buf));
				break;
			}
			case DataType::Boolean:
			case DataType::Integer8:
			case DataType::Integer16:
			case DataType::Integer32:
			case DataType::Integer64:
			case DataType::UnsignedInteger8:
			case DataType::UnsignedInteger16:
			case DataType::UnsignedInteger32:
			case DataType::UnsignedInteger64:
			case DataType::SingleFloat:
			case DataType::SingleFloatWithUnit:
			case DataType::DoubleFloat:
			case DataType::DoubleFloatWithUnit:
			case DataType::TimeStamp: {
				writer.write((char*)data.GetRawData(), data.GetLength());
				break;
			}
			case DataType::String:
			{
				uint32_t strLen = data.GetLength();
				writer.write((char *)&strLen, sizeof(strLen));
				writer.write((char *)data.GetRawData(), data.GetLength());
				break;
			}
			default: {
				std::string message = "Cannot determine size of data type: ";
				message += data.GetDataType();
				throw std::invalid_argument(message.c_str());
			}
		}
	}
}