#ifndef TDMS_LIB_BINARYSTREAM_H
#define TDMS_LIB_BINARYSTREAM_H

#include <iostream>
#include "data_type.h"


using namespace std;

namespace TDMS {
class BinaryStream {
   public:
    BinaryStream();
    ~BinaryStream();

    static auto ReadLengthPrefixedString(iostream& reader) -> DataType;
    static auto ReadString(iostream& reader, int length) -> DataType;
    static auto Read(iostream& reader, TDMSType dataType) -> DataType;
    static auto ReadArray(iostream& reader, long size, int offset) -> std::shared_ptr<uint8_t[]>;
    static auto ReadArray(iostream& reader, long dataSize, long Count, int offset, int interleaveSkip) -> std::shared_ptr<uint8_t[]>;
    static auto Write(iostream& writer, DataType& data) -> void;

    template <typename T>
    static auto Read(iostream& reader, TDMSType dataType) -> T {
        DataType data = BinaryStream::Read(reader, dataType);
        return data.GetData<T>();
    }
};
}  // namespace TDMS

#endif
