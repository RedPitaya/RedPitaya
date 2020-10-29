#include "rpsa/common/core/File.h"

namespace TDMS
{
    template <typename T, typename Key>
    bool key_exists(const T& container, const Key& key)
    {
        return (container.find(key) != std::end(container));
    }

	File::File()
	{
	}


	File::~File()
	{
	}


    void File::Print(vector<shared_ptr<Metadata>> &data,bool PrintRaw,long limitData){
        for (auto &m : data){
            cout << "Path: " <<  m->PathStr << endl;
            cout << "\tProperties:" <<  m->Properties.size() << endl;
            for(auto &p : m->Properties){
                cout << "\t\tKey: " << p.first << "\tValue:" << p.second.ToString() << endl;
            }

            cout << "\tRaw Data:" <<  m->RawData.Size << endl;
            if (m->RawData.Size>0) {
                cout << "\t\t- Type:" << m->RawData.DataType.ToTypeString() << endl;
                cout << "\t\t- Count:" << m->RawData.Count << endl;
                cout << "\t\t- IsInterleaved:" << m->RawData.IsInterleaved << endl;
                cout << "\t\t- Dimension:" << m->RawData.Dimension << endl;
                cout << "\t\t- InterleaveStride:" << m->RawData.InterleaveStride << endl;
                cout << "\t\t- Offset:" << m->RawData.Offset << endl;
                if (PrintRaw) {
                    cout << "\t\t\tRAW DATA:" << endl;
                    m->RawData.DataType.PrintVector(limitData);
                }
            }
        }
    }

    vector<shared_ptr<Metadata>> File::ReadFile(string m_fileName) {
		std::fstream ifs;
		ifs.open(m_fileName, ios::binary | std::ifstream::in );
		if (ifs.fail()) {
			cout << "File " << m_fileName << " not exist" << std::endl;
			return vector<shared_ptr<Metadata>>();
		}
		ifs.seekg(0, ios::beg);
		std::streampos fsize = 0;
		fsize = ifs.tellg();
		ifs.seekg(0, ios::end);
		fsize = ifs.tellg() - fsize;
		ifs.seekg(0, ios::beg);
		Reader reader(ifs, fsize);
		vector<shared_ptr<Metadata>> metadata = LoadMetadata(reader);

		ifs.close();
        return  metadata;
	}

    void  File::WriteFile(string m_fileName,WriterSegment &segment,bool Append){
        std::fstream ifs;
        ifs.open(m_fileName, ios::binary | std::ofstream::out| std::ofstream::in | (Append? std::ofstream::binary  : std::ofstream::trunc));
        if (ifs.fail()) {
            ifs.open(m_fileName, ios::binary | std::ofstream::out| std::ofstream::in |  std::ofstream::trunc);
            if (ifs.fail()) {
                cout << "File " << m_fileName << " not exist" << std::endl;
                return;
            }
        }
        Writer writer(ifs,Append);
        writer.Write(segment);
        ifs.close();
    }

    void  File::WriteMemory(std::iostream& stream,WriterSegment &segment){

        Writer writer(stream, true);
        writer.Write(segment);

    }



    vector<shared_ptr<Metadata>> File::LoadMetadata(Reader &reader) {
		vector<shared_ptr<Segment>> segments = GetSegments(reader);
		vector<shared_ptr<Metadata>> metadataRet;
		map<string, map<string, shared_ptr<Metadata>>> prevMetaDataLookup;
		for (auto &segment : segments)
		{
			if (!(segment->TableOfContents.ContainsNewObjects ||
				segment->TableOfContents.HasDaqMxData ||
				segment->TableOfContents.HasMetaData ||
				segment->TableOfContents.HasRawData)) {
				continue;
			}

            vector<shared_ptr<Metadata>> metadatas = reader.ReadMetadata(segment);
			long rawDataSize = 0;
			long nextOffset = segment->RawDataOffset;


            for (auto &metadata : metadatas)
            {
                if (metadata->RawData.Count == 0 && metadata->Path.size() > 1)
                {
                    // apply previous metadata if available
                    auto  prevMetadataPair = prevMetaDataLookup.find(metadata->Path[0]);
                    if (prevMetadataPair!= prevMetaDataLookup.end()) {
                        map<string, shared_ptr<Metadata>> prevMetadataMap = prevMetadataPair->second;
                        auto prevMetaDataPair2 = prevMetadataMap.find(metadata->Path[1]);
                        if (prevMetaDataPair2!= prevMetadataMap.end()) {
                             auto prevMetaData = prevMetaDataPair2->second;

                             metadata->RawData.Count = segment->TableOfContents.HasRawData
                                               ? prevMetaData->RawData.Count : 0;
                             metadata->RawData.DataType = prevMetaData->RawData.DataType;
                             metadata->RawData.Offset = segment->RawDataOffset + rawDataSize;
                             metadata->RawData.IsInterleaved = prevMetaData->RawData.IsInterleaved;
                             metadata->RawData.InterleaveStride = prevMetaData->RawData.InterleaveStride;
                             metadata->RawData.Size = prevMetaData->RawData.Size;
                             metadata->RawData.Dimension = prevMetaData->RawData.Dimension;

                        }
                    }
                }
                if (metadata->RawData.IsInterleaved && segment->NextSegmentOffset <= 0)
                {
                    metadata->RawData.Count = segment->NextSegmentOffset > 0
                                      ? (segment->NextSegmentOffset - metadata->RawData.Offset + metadata->RawData.InterleaveStride - 1)/
                                                      metadata->RawData.InterleaveStride
                                      : (reader.GetFileSize() - metadata->RawData.Offset + metadata->RawData.InterleaveStride - 1)/
                                                      metadata->RawData.InterleaveStride;
                }

                if (metadata->Path.size() > 1)
                {
                    rawDataSize += metadata->RawData.Size;
                    nextOffset += metadata->RawData.Size;
                }
            }

            vector<shared_ptr<Metadata>> implicitMetadatas;
            bool Check= true;
            for (auto &metadata : metadatas)
            {
                if (!(!metadata->RawData.IsInterleaved && metadata->RawData.Size > 0))
                    Check = false;
            }

            if (Check && segment->TableOfContents.HasRawData)
            {
                while (nextOffset < segment->NextSegmentOffset   ||
                       (segment->NextSegmentOffset == -1 &&  nextOffset < (long)reader.GetFileSize()))
                {
                    // Incremental Meta Data see http://www.ni.com/white-paper/5696/en/#toc1
                    for (auto &metadata : metadatas)
                    {
                        if (metadata->Path.size() > 1)
                        {
                            shared_ptr<Metadata> implicitMetadata;

                            implicitMetadata->Path = metadata->Path;
                            implicitMetadata->RawData.Count = metadata->RawData.Count;
                            implicitMetadata->RawData.DataType = metadata->RawData.DataType;
                            implicitMetadata->RawData.Offset = nextOffset;
                            implicitMetadata->RawData.IsInterleaved = metadata->RawData.IsInterleaved;
                            implicitMetadata->RawData.Size = metadata->RawData.Size;
                            implicitMetadata->RawData.Dimension = metadata->RawData.Dimension;

                            implicitMetadata->Properties = metadata->Properties;

                            implicitMetadatas.push_back(implicitMetadata);
                            nextOffset += implicitMetadata->RawData.Size;
                        }
                    }
                }
            }

            vector<shared_ptr<Metadata>> metadataWithImplicit;

            metadataWithImplicit.insert(std::end(metadataWithImplicit), std::begin(metadatas), std::end(metadatas));
            metadataWithImplicit.insert(std::end(metadataWithImplicit), std::begin(implicitMetadatas), std::end(implicitMetadatas));

            for (auto &metadata : metadataWithImplicit)
            {
                if (metadata->Path.size() == 2)
                {
                    if (!key_exists<map<string, map<string, shared_ptr<Metadata>>>,string>(prevMetaDataLookup,metadata->Path[0]))
                    {
                        auto pair_data =  std::pair<string,map<string, shared_ptr<Metadata>>>(metadata->Path[0], map<string, shared_ptr<Metadata>>());
                        prevMetaDataLookup.insert(pair_data);
                    }

                    prevMetaDataLookup[metadata->Path[0]][metadata->Path[1]] = metadata;

                }
                metadataRet.push_back(metadata);
            }
		}

		return metadataRet;
	}



	vector<shared_ptr<Segment>> File::GetSegments(Reader &reader)
	{
		vector<shared_ptr<Segment>> list;
        shared_ptr<Segment> segment = reader.ReadFirstSegment();
		while (segment != NULL)
		{
			list.push_back(segment);
			segment = reader.ReadSegment(segment->NextSegmentOffset);
		}
		return list;

	}
}
