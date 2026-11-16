#ifndef NET_LIB_ASIO_COMMON_H
#define NET_LIB_ASIO_COMMON_H

#include <stdint.h>
#include <list>
#include <memory>

#define NET_ADC_STREAMING_PORT 18900
#define NET_CONFIG_PORT 18901
#define NET_BROADCAST_PORT 18902
#define NET_DAC_STREAMING_PORT 18903
#define NET_GPIO_IN_STREAMING_PORT 18904   // the input lines, board to client
#define NET_GPIO_OUT_STREAMING_PORT 18905  // the output lines, client to board

namespace net_lib {

enum EMode { M_SERVER, M_CLIENT };

typedef std::shared_ptr<uint8_t[]> net_buffer;
typedef std::list<std::pair<net_buffer, size_t>> net_list;

auto createBuffer(const char* buffer, size_t size) -> net_buffer;
auto createBuffer(uint64_t size) -> net_buffer;

// The CPU for a thread that sends a stream: the one that takes the fewest interrupts of the
// network interfaces. TCP without segmentation offload costs a core per 1 Gb/s, and a sender on
// the core of the network interrupt shares it with them: about 90 MB/s instead of the line rate
// on a two core Zynq. -1: no choice (one CPU, no network interrupt found, not Linux).
// RP_STREAMING_SENDER_CPU sets the CPU, -1 there leaves the sender to the scheduler.
auto senderCpu() -> int;
// Pins the calling thread to senderCpu(); returns the CPU, or -1 when the thread stays unpinned.
auto pinSenderThread() -> int;
// How many packs of a ring of ringSize may wait for their zero copy completions. The writer holds
// writerHeld packs and needs one more free to hand a full one over, while the sender waiting for
// that pack reads no completions; at most half the ring. 0: every send waits for its completion.
auto zeroCopyMaxPending(size_t ringSize, size_t writerHeld) -> size_t;
}  // namespace net_lib

#endif
