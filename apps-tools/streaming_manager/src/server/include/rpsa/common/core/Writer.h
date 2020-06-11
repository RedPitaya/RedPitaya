//
// Created by user on 15.11.18.
//

#ifndef TDMS_LIB_WRITER_H
#define TDMS_LIB_WRITER_H


#include <memory>
#include <fstream>
#include "DataType.h"
#include "FileStructTypes.h"
#include "BinaryStream.h"

using  namespace std;

namespace TDMS
{
    class WriterSegment {
        vector<shared_ptr<Metadata>> m_nodes;
    public:
        void LoadMetadata(vector<shared_ptr<Metadata>> data);
        bool IsRootNodePresent();
        shared_ptr<Metadata> GenerateRoot();
        shared_ptr<Metadata> GenerateGroup(string groupName);
        shared_ptr<Metadata> GenerateChannel(string groupName,string channelName);
        void                 AddProperties(shared_ptr<Metadata> metadata,string key,DataType value);
        void                 AddRaw(shared_ptr<Metadata> metadata,uint32_t type,int64_t count, void *rawData);
        shared_ptr<Metadata> GetRoot();
        vector<shared_ptr<Metadata>> GetNodes();

    };

    class Writer {
        iostream *m_fileStream;
        BinaryStream m_bstream;
        bool m_is_append;
        vector<std::ios::pos_type> m_stek_pos_g;
        vector<std::ios::pos_type> m_stek_pos_p;
    public:
        Writer(iostream &fileStream, bool append);
        void Write(WriterSegment &segment);
        uint64_t GetFileSize();
    private:
        void WriteSegment(long offset, shared_ptr<Metadata> leadin);
        int64_t  WriteRawHeader(shared_ptr<Metadata> metadata);
        void WriteNextSegmentAddress(int64_t offset,int64_t address);
        void WriteRawSegmentAddress(int64_t offset,int64_t address);

        void PushGPosition();
        void PopGPosition();
        void PushPPosition();
        void PopPPosition();
    };


}

#endif //TDMS_LIB_WRITER_H
