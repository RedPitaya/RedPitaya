#ifndef TDMS_LIB_WRITER_H
#define TDMS_LIB_WRITER_H

#include <memory>
#include <fstream>
#include <list>
#include "data_type.h"
#include "file_struct_types.h"
#include "binary_stream.h"

using  namespace std;

namespace TDMS
{
    class WriterSegment
    {
        public:
            auto AddProperties(shared_ptr<Metadata> metadata,string key,DataType value) -> void;
            auto AddRaw(shared_ptr<Metadata> metadata,TDMSType type,int64_t count, uint8_t* rawData) -> void;
            auto LoadMetadata(vector<shared_ptr<Metadata>> data) -> void;
            auto IsRootNodePresent() -> bool;
            auto GenerateRoot() -> shared_ptr<Metadata>;
            auto GenerateGroup(string groupName) -> shared_ptr<Metadata>;
            auto GenerateChannel(string groupName,string channelName) -> shared_ptr<Metadata>;
            auto GetRoot() -> shared_ptr<Metadata>;
            auto GetNodes() -> vector<shared_ptr<Metadata>>;

        private:
            vector<shared_ptr<Metadata>> m_nodes;
    };

    class Writer
    {
        public:
            Writer(iostream &fileStream, bool append);
            auto Write(WriterSegment &segment) -> void;
            auto GetFileSize() -> uint64_t;

        private:
            auto WriteSegment(long offset, shared_ptr<Metadata> leadin) -> void;
            auto WriteRawHeader(shared_ptr<Metadata> metadata) -> int64_t;
            auto WriteNextSegmentAddress(int64_t offset,int64_t address) -> void;
            auto WriteRawSegmentAddress(int64_t offset,int64_t address) -> void;

            auto PushGPosition() -> void;
            auto PopGPosition() -> void;
            auto PushPPosition() -> void;
            auto PopPPosition() -> void;

            iostream*                  m_fileStream;
            BinaryStream               m_bstream;
            bool                       m_is_append;
            list<std::ios::pos_type>   m_stek_pos_g;
            list<std::ios::pos_type>   m_stek_pos_p;
    };
}

#endif //TDMS_LIB_WRITER_H
