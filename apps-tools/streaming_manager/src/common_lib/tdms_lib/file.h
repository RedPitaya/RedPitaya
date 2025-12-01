#pragma once
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "file_struct_types.h"
#include "reader.h"
#include "writer.h"

using namespace std;

namespace TDMS {
class File {
   public:
    File();
    ~File();

    auto ReadFile(string m_fileName) -> vector<shared_ptr<Metadata>>;
    auto ReadFileWithoutClose(string m_fileName) -> vector<shared_ptr<Segment>>;
    auto Close() -> bool;
    auto GetMetadata(shared_ptr<Segment>) -> vector<shared_ptr<Metadata>>;
    auto WriteFile(string m_fileName, WriterSegment& segment, bool Append) -> void;
    auto WriteMemory(std::iostream& stream, WriterSegment& segment) -> void;
    auto Print(vector<shared_ptr<Metadata>>& data, bool PrintRaw, long limitData) -> void;
    auto clearPrevMetadata() -> void;

   private:
    auto LoadMetadata(Reader& reader) -> vector<shared_ptr<Metadata>>;
    auto GetSegments(Reader& reader) -> vector<shared_ptr<Segment>>;
    auto GetMetadataItem(Reader& reader, shared_ptr<Segment> segment,
                         map<string, map<string, shared_ptr<Metadata>>>& prevMetaDataLookup) -> vector<shared_ptr<Metadata>>;

    std::fstream m_read_fs;
    Reader* m_reader;
    map<string, map<string, shared_ptr<Metadata>>> m_prevMetaDataLookup;
};
}  // namespace TDMS
