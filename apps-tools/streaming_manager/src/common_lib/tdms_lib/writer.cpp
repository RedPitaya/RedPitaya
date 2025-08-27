#include "writer.h"
#include <iostream>

#define OFFSET_NEXT_SEGMENT 12
#define OFFSET_RAW_DATA OFFSET_NEXT_SEGMENT + 8

using namespace TDMS;

void print_state(const std::ios& stream) {
    std::cout << " good()=" << stream.good();
    std::cout << " eof()=" << stream.eof();
    std::cout << " fail()=" << stream.fail();
    std::cout << " bad()=" << stream.bad();
}

auto WriterSegment::LoadMetadata(vector<shared_ptr<Metadata>> data) -> void {
    m_nodes = data;
}

auto WriterSegment::IsRootNodePresent() -> bool {
    for (auto& n : m_nodes) {
        if (n->Path.size() == 0)
            return true;
    }
    return false;
}

auto WriterSegment::GetRoot() -> shared_ptr<Metadata> {
    for (auto& n : m_nodes) {
        if (n->Path.size() == 0)
            return n;
    }
    return nullptr;
}

auto WriterSegment::GetNodes() -> vector<shared_ptr<Metadata>> {
    return m_nodes;
}

auto WriterSegment::GenerateRoot() -> shared_ptr<Metadata> {
    shared_ptr<Metadata> metadata = make_shared<Metadata>();
    metadata->Version = 4713;  // This version of TDMS2.0
    metadata->PathStr = "/";
    metadata->TableOfContents.ContainsNewObjects = false;
    metadata->TableOfContents.HasDaqMxData = false;
    metadata->TableOfContents.HasMetaData = false;
    metadata->TableOfContents.HasRawData = false;
    metadata->TableOfContents.NumbersAreBigEndian = false;
    metadata->TableOfContents.RawDataIsInterleaved = false;
    return metadata;
}

auto WriterSegment::GenerateGroup(string groupName) -> shared_ptr<Metadata> {
    shared_ptr<Metadata> metadata = make_shared<Metadata>();
    metadata->PathStr = "/'" + groupName + "'";
    metadata->Path.push_back(groupName);
    return metadata;
}

auto WriterSegment::GenerateChannel(string groupName, string channelName) -> shared_ptr<Metadata> {
    shared_ptr<Metadata> metadata = make_shared<Metadata>();
    metadata->PathStr = "/'" + groupName + "'/'" + channelName + "'";
    metadata->Path.push_back(groupName);
    metadata->Path.push_back(channelName);
    return metadata;
}

auto WriterSegment::AddProperties(shared_ptr<Metadata> metadata, string key, DataType value) -> void {
    metadata->Properties[key] = value;
}

auto WriterSegment::AddRaw(shared_ptr<Metadata> metadata, TDMSType type, int64_t count, std::shared_ptr<uint8_t[]> rawData) -> void {
    if (type == TDMSType::String)
        throw std::invalid_argument("[ERROR] Set string raw data not implemented!");

    metadata->RawData.DataType.InitRaw(type, count, rawData);
    metadata->RawData.Count = count;
    metadata->RawData.Dimension = 1;
    metadata->RawData.IsInterleaved = false;
    metadata->RawData.Size = count * metadata->RawData.DataType.GetLength();
    metadata->RawData.Offset = 0;
}

auto Writer::GetFileSize() -> uint64_t {
    m_fileStream->seekg(0, ios::beg);
    std::streampos fsize = 0;
    fsize = m_fileStream->tellg();
    m_fileStream->seekg(0, ios::end);
    fsize = m_fileStream->tellg() - fsize;
    m_fileStream->seekg(0, ios::beg);
    return fsize;
}

auto Writer::Write(WriterSegment& segment) -> void {
    auto root = segment.GetRoot();
    if (root == nullptr) {
        cout << "[Error] No root metadata\n";
        return;
    }
    m_fileStream->seekp(0, ios::end);
    auto posSegmentBegin = m_fileStream->tellp();
    WriteSegment(posSegmentBegin, root);
    auto nodes = segment.GetNodes();
    int32_t metadatacount = nodes.size() - 1;
    m_fileStream->write(reinterpret_cast<char*>(&metadatacount), sizeof(metadatacount));

    for (auto& n : nodes) {
        if (n != root) {
            int32_t path_len = n->PathStr.size();
            m_fileStream->write(reinterpret_cast<char*>(&path_len), sizeof(path_len));
            m_fileStream->write(n->PathStr.data(), n->PathStr.size());
            WriteRawHeader(n);
            int32_t property_count = n->Properties.size();
            m_fileStream->write(reinterpret_cast<char*>(&property_count), sizeof(property_count));
            for (const auto& kv : n->Properties) {
                int32_t key_len = kv.first.size();
                m_fileStream->write(reinterpret_cast<char*>(&key_len), sizeof(key_len));
                m_fileStream->write(kv.first.data(), kv.first.size());
                DataType value = kv.second;
                TDMSType typeValue = value.GetDataType();
                uint32_t typeV = static_cast<uint32_t>(typeValue);
                m_fileStream->write(reinterpret_cast<char*>(&typeV), sizeof(typeV));
                m_bstream.Write(*m_fileStream, value);
            }
        }
    }

    if (root->TableOfContents.HasRawData) {
        auto posMetadataEnd = m_fileStream->tellp();
        WriteRawSegmentAddress(posSegmentBegin, posMetadataEnd - posSegmentBegin - 28);
    }

    for (auto& n : nodes) {
        if (n != root) {
            auto rawVector = n->RawData.DataType.GetRawVector();
            for (auto& r : rawVector) {
                m_fileStream->write((const char*)r->data.get(), r->size);
            }
        }
    }

    auto posSegmentEnd = m_fileStream->tellp();
    WriteNextSegmentAddress(posSegmentBegin, posSegmentEnd - posSegmentBegin - 28);
}

auto Writer::WriteRawHeader(shared_ptr<Metadata> metadata) -> int64_t {
    if (metadata->RawData.DataType.GetDataType() == TDMSType::String)
        throw std::invalid_argument("[ERROR] Save string raw data not implemented!");
    if (metadata->RawData.IsInterleaved)
        throw std::invalid_argument("[ERROR] Interleaved raw data not implemented!");

    auto rawVector = metadata->RawData.DataType.GetRawVector();
    if (rawVector.size() == 0) {
        // Write INDEX of raw header
        int32_t raw_index = -1;
        m_fileStream->write(reinterpret_cast<char*>(&raw_index), sizeof(raw_index));
        return 0;
    } else {
        // Write INDEX of raw header
        int32_t raw_index = 20;
        m_fileStream->write(reinterpret_cast<char*>(&raw_index), sizeof(raw_index));

        // Write data type of raw header
        TDMSType raw_datatype = metadata->RawData.DataType.GetDataType();
        uint32_t typeV = static_cast<uint32_t>(raw_datatype);
        m_fileStream->write(reinterpret_cast<char*>(&typeV), sizeof(typeV));

        // Write demension type of raw header (for ver 2.0, 1 is the only valid value)
        int32_t raw_demension = 1;
        m_fileStream->write(reinterpret_cast<char*>(&raw_demension), sizeof(raw_demension));

        // Write count values
        uint64_t raw_count_values = metadata->RawData.Count;
        m_fileStream->write(reinterpret_cast<char*>(&raw_count_values), sizeof(raw_count_values));

        return DataType::GetArrayLength(raw_datatype, raw_count_values);
    }
}

Writer::Writer(iostream& fileStream, bool append) {
    m_fileStream = &fileStream;
    m_is_append = append;
    m_stek_pos_g.clear();
    m_stek_pos_p.clear();
}

// Full lenght 28 bytes;

auto Writer::WriteSegment(long offset, shared_ptr<Metadata> leadin) -> void {
    if (leadin->TableOfContents.RawDataIsInterleaved)
        throw std::invalid_argument("[ERROR] Interleaved data not implemented!");
    if (leadin->TableOfContents.HasDaqMxData)
        throw std::invalid_argument("[ERROR] DaqMx data not implemented!");

    m_fileStream->seekp(offset, ios::beg);
    m_fileStream->write("TDSm", 4);
    int32_t tableOfContentsMask = 0;
    if (leadin->TableOfContents.ContainsNewObjects)
        tableOfContentsMask |= 1 << 2;
    if (leadin->TableOfContents.HasDaqMxData)
        tableOfContentsMask |= 1 << 7;
    if (leadin->TableOfContents.HasMetaData)
        tableOfContentsMask |= 1 << 1;
    if (leadin->TableOfContents.HasRawData)
        tableOfContentsMask |= 1 << 3;
    if (leadin->TableOfContents.NumbersAreBigEndian)
        tableOfContentsMask |= 1 << 6;
    if (leadin->TableOfContents.RawDataIsInterleaved)
        tableOfContentsMask |= 1 << 5;

    m_fileStream->write(reinterpret_cast<char*>(&tableOfContentsMask), sizeof(tableOfContentsMask));
    int32_t version = leadin->Version;
    m_fileStream->write(reinterpret_cast<char*>(&version), sizeof(version));
    int64_t nextsegment = -1;
    m_fileStream->write(reinterpret_cast<char*>(&nextsegment), sizeof(nextsegment));
    int64_t data_offset = 0;
    m_fileStream->write(reinterpret_cast<char*>(&data_offset), sizeof(data_offset));
}

auto Writer::WriteNextSegmentAddress(int64_t offset, int64_t address) -> void {
    PushPPosition();
    m_fileStream->seekp(offset + OFFSET_NEXT_SEGMENT, ios::beg);
    m_fileStream->write(reinterpret_cast<const char*>(&address), sizeof(address));
    PopPPosition();
}

auto Writer::WriteRawSegmentAddress(int64_t offset, int64_t address) -> void {
    PushPPosition();
    m_fileStream->seekp(offset + OFFSET_RAW_DATA, ios::beg);
    m_fileStream->write(reinterpret_cast<char*>(&address), sizeof(address));
    PopPPosition();
}

auto Writer::PushGPosition() -> void {
    std::ios::pos_type pos = m_fileStream->tellg();
    m_stek_pos_g.push_back(pos);
}

auto Writer::PopGPosition() -> void {
    std::ios::pos_type pos = m_stek_pos_g.back();
    m_fileStream->seekg(pos);
    m_stek_pos_g.pop_back();
}

auto Writer::PushPPosition() -> void {
    std::ios::pos_type pos = m_fileStream->tellp();
    m_stek_pos_p.push_back(pos);
}

auto Writer::PopPPosition() -> void {
    std::ios::pos_type pos = m_stek_pos_p.back();
    m_fileStream->seekp(pos);
    m_stek_pos_p.pop_back();
}
