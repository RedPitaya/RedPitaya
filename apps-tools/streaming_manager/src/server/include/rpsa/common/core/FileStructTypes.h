#pragma once
#include <string>
#include <stdint.h>
#include <vector>
#include <map>

#include "DataType.h"

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
		long Offset;
		TDMS::DataType DataType;
		int Dimension;
		long Count;
		long Size;
		bool IsInterleaved;
		int InterleaveStride;
	};

	struct Metadata
	{
        TDMS::TableOfContents TableOfContents;
        int Version;
		string PathStr;
		vector<string> Path;
		TDMS::RawData RawData;
		map<string, DataType> Properties;
	};

	
}