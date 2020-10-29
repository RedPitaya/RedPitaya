#include "rpsa/common/core/Reader.h"

namespace TDMS
{
	Reader::Reader(iostream &fileStream, uint64_t fileSize)
	{
		m_fileStream = &fileStream;
		m_fileSize = fileSize;
	}


	Reader::~Reader()
	{
	}

    uint64_t Reader::GetFileSize(){
        return m_fileSize;
	}

    shared_ptr<Segment> Reader::ReadFirstSegment()
	{
		return ReadSegment(0);
	}

    shared_ptr<Segment> Reader::ReadSegment(uint64_t offset)
	{
		if (offset >= m_fileSize)
			return NULL;

		m_fileStream->seekg(offset, m_fileStream->beg);

        shared_ptr<Segment> leadin = make_shared<Segment>();
		leadin->Offset = offset; 
		leadin->MetadataOffset = offset + leadin->Length;
		DataType ident = m_bstream.ReadString(*m_fileStream, 4);
		leadin->Identifier = string(ident.GetDataString());

		uint32_t tableOfContentsMask = m_bstream.Read<uint32_t>(*m_fileStream, DataType::UnsignedInteger32);


		leadin->TableOfContents.ContainsNewObjects = ((tableOfContentsMask >> 2) & 1) == 1;
		leadin->TableOfContents.HasDaqMxData = ((tableOfContentsMask >> 7) & 1) == 1;
		leadin->TableOfContents.HasMetaData = ((tableOfContentsMask >> 1) & 1) == 1;
		leadin->TableOfContents.HasRawData = ((tableOfContentsMask >> 3) & 1) == 1;
		leadin->TableOfContents.NumbersAreBigEndian = ((tableOfContentsMask >> 6) & 1) == 1;
		leadin->TableOfContents.RawDataIsInterleaved = ((tableOfContentsMask >> 5) & 1) == 1;
		
		leadin->Version = m_bstream.Read<int32_t>(*m_fileStream, DataType::Integer32);
		
		int64_t nextsegment = m_bstream.Read<int64_t>(*m_fileStream, DataType::Integer64);
		if (nextsegment >= (int64_t)m_fileSize)
			nextsegment = -1;
		if (nextsegment != -1)
		    nextsegment +=  offset + leadin->Length;
		leadin->NextSegmentOffset = nextsegment;
		int64_t rawdataoffset = m_bstream.Read<int64_t>(*m_fileStream, DataType::Integer64);
		if (rawdataoffset !=0)
		    rawdataoffset +=   offset + leadin->Length;
		leadin->RawDataOffset = rawdataoffset;
		cout << "Segment offset :" << offset << "\n";
		return leadin;
	}

	vector<shared_ptr<Metadata>> Reader::ReadMetadata(shared_ptr<Segment> segment) {
		vector<shared_ptr<Metadata>> metadatas;

		cout << "Metadata offset: " << segment->MetadataOffset << "\n";
        cout << "Raw offset: " << segment->RawDataOffset << "\n";
		m_fileStream->seekg(segment->MetadataOffset, ios::beg);
		int32_t objectCount = m_bstream.Read<int32_t>(*m_fileStream, DataType::Integer32);	
		long rawDataOffset = segment->RawDataOffset;
		bool isInterleaved = segment->TableOfContents.RawDataIsInterleaved;
		int interleaveStride = 0;
		for (int32_t x = 0; x < objectCount; x++)
		{
            cout << "Metadata offset position: " << m_fileStream->tellg() << "\n";
            shared_ptr<Metadata> metadata = std::make_shared<Metadata>();
            metadata->TableOfContents = segment->TableOfContents;
            metadata->Version = segment->Version;
			metadata->PathStr = m_bstream.ReadLengthPrefixedString(*m_fileStream).GetDataString();

            std::regex r("'(.*?)'");
            std::sregex_iterator next(metadata->PathStr.begin(), metadata->PathStr.end(), r);
            std::sregex_iterator end;
            while (next != end) {
                std::smatch match = *next;
                metadata->Path.push_back(match.str());
                next++;
            }

			auto  rawDataIndexLength = m_bstream.Read<int32_t>(*m_fileStream, DataType::Integer32);
			if (rawDataIndexLength > 0)
			{
				metadata->RawData.Offset = rawDataOffset;
				cout << "RawData.Offset " << rawDataOffset << endl;
				metadata->RawData.IsInterleaved = segment->TableOfContents.RawDataIsInterleaved;

				int32_t dataType = m_bstream.Read<int32_t>(*m_fileStream, DataType::Integer32);

                metadata->RawData.DataType.InitDataType(dataType, NULL);

				metadata->RawData.Dimension = m_bstream.Read<int32_t>(*m_fileStream, DataType::Integer32);
				metadata->RawData.Count = (long)m_bstream.Read<int64_t>(*m_fileStream, DataType::Integer64);

				metadata->RawData.Size = rawDataIndexLength == 28 ? (long)m_bstream.Read<int64_t>(*m_fileStream, DataType::Integer64) :
					(long)DataType::GetArrayLength(metadata->RawData.DataType.GetDataType(), metadata->RawData.Count);

                vector<shared_ptr<DataType::Raw>> raw = ReadRawData(metadata->RawData);
                cout << "RawData.Size " << metadata->RawData.Size << endl;
                metadata->RawData.DataType.InitDataType(dataType, raw);
				if (isInterleaved)
				{
					//fixed error. The interleave stride is the sum of all channel (type) dataSizes
					rawDataOffset += DataType::GetLength(metadata->RawData.DataType.GetDataType());
					interleaveStride += DataType::GetLength(metadata->RawData.DataType.GetDataType());
				}
				else
					rawDataOffset += metadata->RawData.Size;
			}
            cout << "Property offset position: " << m_fileStream->tellg() << "\n";
			auto propertyCount = m_bstream.Read<int32_t>(*m_fileStream, DataType::Integer32);
			for (auto y = 0; y < propertyCount; y++)
			{
				auto key = m_bstream.ReadLengthPrefixedString(*m_fileStream).GetDataString();
				auto value = m_bstream.Read(*m_fileStream, m_bstream.Read<int32_t>(*m_fileStream, DataType::Integer32));
				metadata->Properties.insert(std::pair<string, DataType>(key, value));
			}
			metadatas.push_back(metadata);
		}

		if (isInterleaved)
		{
			for (auto &metadata : metadatas)
			{
				
				metadata->RawData.InterleaveStride = interleaveStride;

				metadata->RawData.Count = segment->NextSegmentOffset > 0
					? (segment->NextSegmentOffset - metadata->RawData.Offset + interleaveStride - 1) / interleaveStride
					: (m_fileSize - metadata->RawData.Offset + interleaveStride - 1) / interleaveStride;
				
			}
		}

		return metadatas;
	}

    vector<shared_ptr<DataType::Raw>> Reader::ReadRawData(RawData &rawData)
    {
        if (rawData.IsInterleaved)
            return ReadRawInterleaved(rawData.Offset, rawData.Count, rawData.DataType.GetDataType(), rawData.InterleaveStride - rawData.DataType.GetLength());    //fixed error

        return rawData.DataType.GetDataType() == DataType::String ?
               ReadRawStrings(rawData.Offset, rawData.Count) :
               ReadRawFixed(rawData.Offset, rawData.Count, rawData.DataType.GetDataType());
    }

    vector<shared_ptr<DataType::Raw>> Reader::ReadRawFixed(long offset, long count, uint32_t dataType)
    {
        long  sizeread =  DataType::GetLength(dataType) * count;
        uint8_t  *buff = m_bstream.ReadArray(*m_fileStream, sizeread, offset);
        vector<shared_ptr<DataType::Raw>> vec;
        shared_ptr<DataType::Raw> raw = make_shared<DataType::Raw>();
        raw->data = buff;
        raw->size = sizeread;
        vec.push_back(raw);
        return  vec;
    }

    vector<shared_ptr<DataType::Raw>> Reader::ReadRawInterleaved(long offset, long count, uint32_t dataType, int interleaveSkip)
    {
        long  sizeread =  DataType::GetLength(dataType);
        vector<shared_ptr<DataType::Raw>> vec;
        uint8_t  *buff = m_bstream.ReadArray(*m_fileStream, sizeread , count, offset,interleaveSkip);
        shared_ptr<DataType::Raw> raw = make_shared<DataType::Raw>();
        raw->data = buff;
        raw->size = sizeread;
        vec.push_back(raw);
        return vec;

    }

    vector<shared_ptr<DataType::Raw>> Reader::ReadRawStrings(long offset, long count)
    {
        vector<shared_ptr<DataType::Raw>> vec;
        std::ios::pos_type pos = m_fileStream->tellg();

        m_fileStream->seekg(offset, ios::beg);

        long dataOffset = offset + (count * 4);
        std::ios::pos_type indexPosition;
        long dataPosition = dataOffset;

        for (long x = 0; x < count; x++)
        {
            uint32_t endOfString = m_bstream.Read<uint32_t>(*m_fileStream, DataType::UnsignedInteger32);
            indexPosition =  m_fileStream->tellg();

            m_fileStream->seekg(dataPosition, ios::beg);

            uint8_t *buff = new uint8_t[(int)((dataOffset + endOfString) - dataPosition)];
            m_fileStream->read((char*)buff,(int)((dataOffset + endOfString) - dataPosition));
            shared_ptr<DataType::Raw> raw = make_shared<DataType::Raw>();
            raw->data = buff;
            raw->size = (int)((dataOffset + endOfString) - dataPosition);
            vec.push_back(raw);
            dataPosition = dataOffset + endOfString;
            m_fileStream->seekg(indexPosition);
        }

        m_fileStream->seekg(pos);
        return vec;
    }

}
