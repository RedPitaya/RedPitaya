#pragma once
#include <fstream>
#include <string>
#include <map>
#include <vector>

#include "data_type.h"
#include "file_struct_types.h"
#include "writer.h"

using namespace std;

namespace rp_formatter_api::TDMS
{
	class File
	{
    	public:
            File();
            ~File();

            auto WriteFile(string m_fileName,WriterSegment &segment,bool Append) -> void;
            auto WriteMemory(std::iostream& stream,WriterSegment &segment) -> void;
            auto Print(vector<shared_ptr<Metadata>> &data,bool PrintRaw,long limitData) -> void;
            auto clearPrevMetadata() -> void;

    private:
    	    map<string, map<string, shared_ptr<Metadata>>> m_prevMetaDataLookup;
	};
}
