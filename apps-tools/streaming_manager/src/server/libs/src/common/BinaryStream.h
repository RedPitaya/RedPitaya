#pragma once
#pragma GCC diagnostic ignored "-Wpedantic"

#include <fstream>
#include <iostream>
#include "DataType.h"

using namespace std;

namespace TDMS
{
	class BinaryStream
	{
	public:
		BinaryStream();
		~BinaryStream();

		static DataType ReadLengthPrefixedString(iostream &reader);
		static DataType ReadString(iostream &reader, int length);
		static DataType Read(iostream &reader, int dataType);
		static uint8_t* ReadArray(iostream &reader, long size,int offset);
        static uint8_t* ReadArray(iostream &reader, long dataSize, long Count ,int offset,int interleaveSkip);
		template<typename T>
		static T Read(iostream &reader, int dataType) {
		 	DataType data = BinaryStream::Read(reader, dataType);
			return data.GetData<T>();
		};

		static void     Write(iostream &writer,DataType& data);

	};
}

