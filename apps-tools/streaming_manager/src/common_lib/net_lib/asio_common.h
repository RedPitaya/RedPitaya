#ifndef NET_LIB_ASIO_COMMON_H
#define NET_LIB_ASIO_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <memory>
#include <list>
#include <utility>

#include "data_lib/buffers_pack.h"

namespace net_lib {

typedef std::shared_ptr<uint8_t[]> net_buffer;
typedef std::list<std::pair<net_buffer,size_t>> net_list;

extern const uint8_t ID_PACK[];
extern const uint8_t ID_PACK_END[];
extern const uint8_t ID_BUFFER[];

enum EProtocol {
    P_TCP = 0,
    P_UDP = 1
};

enum EMode {
    M_NONE   = 0,
    M_SERVER = 1,
    M_CLIENT = 2
};

struct AsioBufferNolder{
    uint8_t  header[100];
    size_t   headerLen;
    uint8_t* dataPtr = nullptr;
    size_t   dataLen;
    DataLib::CDataBuffer::Ptr buffPackOwner;
};

typedef std::list<AsioBufferNolder> net_list_bh;

auto createBuffer(const char *buffer,size_t size) -> net_buffer;

auto createBuffer(uint64_t size) -> net_buffer;

auto buildPack(uint64_t _id,DataLib::CDataBuffersPack::Ptr pack,size_t split_size) -> net_list_bh;

auto extractBeginPack(uint8_t* _buffer,size_t _length,uint64_t *_id,size_t *_allBuffersSize) -> DataLib::CDataBuffersPack::Ptr;
auto extractEndPack(uint8_t* _buffer,size_t _length,uint64_t *_id) -> bool;
auto extractBufferPack(uint8_t* _buffer,size_t _length,uint64_t *_id,uint64_t *_packOrder,DataLib::EDataBuffersPackChannel *_channel) -> DataLib::CDataBuffer::Ptr;

}

#endif
