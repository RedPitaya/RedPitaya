#include "file.h"

using namespace rp_formatter_api::TDMS;

template <typename T, typename Key>
bool key_exists(const T& container, const Key& key){
    return (container.find(key) != std::end(container));
}

File::File()
{
}


File::~File(){
}

auto File::Print(vector<shared_ptr<Metadata>> &data,bool PrintRaw,long limitData) -> void{
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

auto File::clearPrevMetadata() -> void{
    m_prevMetaDataLookup.clear();
}


auto File::WriteFile(string m_fileName,WriterSegment &segment,bool Append) -> void{
    std::fstream ifs;
    ifs.open(   m_fileName, ios::binary | std::ofstream::out| std::ofstream::in | (Append? std::ofstream::binary  : std::ofstream::trunc));
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

auto File::WriteMemory(std::iostream& stream,WriterSegment &segment) -> void{
    Writer writer(stream, true);
    writer.Write(segment);
}

