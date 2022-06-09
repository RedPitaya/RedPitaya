#ifndef TDMS_LIB_READER_H
#define TDMS_LIB_READER_H

#include <fstream>
#include <regex>
#include "file_struct_types.h"
#include "binary_stream.h"
#include "data_type.h"

using namespace std;

namespace TDMS
{
	class Reader
	{
        public:
            Reader(iostream &fileStream,uint64_t fileSize);
            ~Reader();
            auto GetFileSize() -> uint64_t;
            auto ReadFirstSegment() -> shared_ptr<Segment>;
            auto ReadSegment(uint64_t offset) -> shared_ptr<Segment>;
            auto ReadMetadata(shared_ptr<Segment> segment) -> vector<shared_ptr<Metadata>>;
            auto ReadRawData(RawData &rawData) -> vector<shared_ptr<DataType::Raw>>;
            auto ReadRawFixed(long offset, long count, TDMSType dataType) -> vector<shared_ptr<DataType::Raw>>;
            auto ReadRawInterleaved(long offset, long count, TDMSType dataType, int interleaveSkip) -> vector<shared_ptr<DataType::Raw>> ;
            auto ReadRawStrings(long offset, long count) -> vector<shared_ptr<DataType::Raw>>;

        private:
            iostream*    m_fileStream;
            uint64_t     m_fileSize;
            BinaryStream m_bstream;
	};
}

#endif
