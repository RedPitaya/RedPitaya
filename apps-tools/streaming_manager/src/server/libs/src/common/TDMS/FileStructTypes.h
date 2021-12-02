#pragma once
#include <string>
#include <stdint.h>
#include <vector>
#include <map>

#include "common/TDMS/DataType.h"

using namespace std;

namespace TDMS
{

	struct TableOfContents
	{
		bool HasMetaData;
		bool HasRawData;
		bool HasDaqMxData;
		bool RawDataIsInterleaved;
		bool NumbersAreBigEndian;
		bool ContainsNewObjects;
	};

	struct Segment
	{
		const long Length = 28;
		long Offset;
		long MetadataOffset;
		long RawDataOffset;
		long NextSegmentOffset;
		string Identifier;
		TDMS::TableOfContents TableOfContents;
		int Version;
	};

	struct RawData
	{
		long Offset = 0;
		TDMS::DataType DataType;
		int Dimension = 0;
		long Count = 0;
		long Size = 0;
		bool IsInterleaved = false;
		int InterleaveStride = 0;
	};

	struct Metadata
	{
        TDMS::TableOfContents TableOfContents;
        int Version = 0;
		string PathStr = "";
		vector<string> Path;
		TDMS::RawData RawData;
		map<string, DataType> Properties;
	};

}