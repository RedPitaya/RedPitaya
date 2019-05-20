#pragma once
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include "DataType.h"
#include "FileStructTypes.h"
#include "Reader.h"
#include "Writer.h"

using namespace std;

namespace TDMS
{
	class File
	{
	private:

	public:
		File();
		~File();
        vector<shared_ptr<Metadata>> ReadFile(string m_fileName);
        void              WriteFile(string m_fileName,WriterSegment &segment,bool Append);
		void              WriteMemory(std::iostream& stream,WriterSegment &segment);

        void Print(vector<shared_ptr<Metadata>> &data,bool PrintRaw,long limitData);
	private:
		vector<shared_ptr<Metadata>> LoadMetadata(Reader &reader);
		vector<shared_ptr<Segment>>  GetSegments(Reader &reader);

	};
}
