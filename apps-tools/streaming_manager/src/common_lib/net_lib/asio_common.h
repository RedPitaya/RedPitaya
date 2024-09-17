#ifndef NET_LIB_ASIO_COMMON_H
#define NET_LIB_ASIO_COMMON_H

#include <list>
#include <memory>
#include <stdint.h>

#define NET_ADC_STREAMING_PORT 18900
#define NET_CONFIG_PORT 18901
#define NET_BROADCAST_PORT 18902
#define NET_DAC_STREAMING_PORT 18903

namespace net_lib {

enum EMode {
	M_SERVER,
	M_CLIENT
};

typedef std::shared_ptr<uint8_t[]> net_buffer;
typedef std::list<std::pair<net_buffer, size_t>> net_list;

auto createBuffer(const char *buffer, size_t size) -> net_buffer;
auto createBuffer(uint64_t size) -> net_buffer;
} // namespace net_lib

#endif
