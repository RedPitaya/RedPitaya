#pragma once
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include "common/TDMS/DataType.h"
#include "FileStructTypes.h"
#include "Reader.h"
#include "Writer.h"

using namespace std;

namespace TDMS
{
	class File
	{
    	public:
            File();
            ~File();

            auto ReadFile(string m_fileName) -> vector<shared_ptr<Metadata>>;
            auto WriteFile(string m_fileName,WriterSegment &segment,bool Append) -> void;
            auto WriteMemory(std::iostream& stream,WriterSegment &segment) -> void;
            auto Print(vector<shared_ptr<Metadata>> &data,bool PrintRaw,long limitData) -> void;

        private:
    	    auto LoadMetadata(Reader &reader) -> vector<shared_ptr<Metadata>>;
    	    auto GetSegments(Reader &reader) -> vector<shared_ptr<Segment>>;
	};
}
