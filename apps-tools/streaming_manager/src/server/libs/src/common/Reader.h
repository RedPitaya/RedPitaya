#pragma once

#include <fstream>
#include <regex>
#include "FileStructTypes.h"
#include "BinaryStream.h"
#include "DataType.h"

using namespace std;

namespace TDMS
{
	class Reader
	{
	private:
		iostream *m_fileStream;
		uint64_t  m_fileSize;
		BinaryStream m_bstream;
	public:
		Reader(iostream &fileStream,uint64_t fileSize);
		~Reader();
		uint64_t GetFileSize();
		shared_ptr<Segment>  ReadFirstSegment();
        shared_ptr<Segment>  ReadSegment(uint64_t offset);
		vector<shared_ptr<Metadata>> ReadMetadata(shared_ptr<Segment> segment);
        vector<shared_ptr<DataType::Raw>> ReadRawData(RawData &rawData);
        vector<shared_ptr<DataType::Raw>> ReadRawFixed(long offset, long count, uint32_t dataType);
        vector<shared_ptr<DataType::Raw>> ReadRawInterleaved(long offset, long count, uint32_t dataType, int interleaveSkip);
        vector<shared_ptr<DataType::Raw>> ReadRawStrings(long offset, long count);
	};
}
