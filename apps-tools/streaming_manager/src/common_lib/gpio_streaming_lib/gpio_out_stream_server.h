#ifndef GPIO_STREAMING_LIB_GPIO_OUT_STREAM_SERVER_H
#define GPIO_STREAMING_LIB_GPIO_OUT_STREAM_SERVER_H

#include <memory>
#include <string>

#include "gpio_in_stream_server.h"
#include "gpio_stop_reason.h"
#include "settings_lib/stream_settings.h"
#include "uio_lib/hardware.h"

namespace gpio_streaming_lib {

class GPIOOutStreamServer;

// Events of the server, called from its threads; the default does nothing
class GPIOOutServerCallback {
   public:
    virtual ~GPIOOutServerCallback() {}
    virtual void started(GPIOOutStreamServer*) {}
    // the file mode: the output plays the file (instead of started)
    virtual void startedSD(GPIOOutStreamServer*) {}
    // after every stop, also of a server that was not running (a client stops before it starts);
    // waitClocks: the clocks the output waited for a buffer of the client since it began to play
    // (the network was too slow), the time after the last buffer up to the stop included
    virtual void stopped(GPIOOutStreamServer*, GPIOStopReason, uint64_t /* waitClocks */) {}
};

// The GPIO output of a board (DIO0..7 P and N) through CHardware: the client's words {P lines,
// N lines, 8 bit unused, count} come on NET_GPIO_OUT_STREAMING_PORT, a buffer is one pack of one
// channel with a DataLib::GPIOHeader (its dataSize: the last one may be filled in part). It runs
// on its own, the input may run or not. start() sets everything up and opens the data server; the
// output plays once the client's first buffers have filled the ring (or 100 ms after the last
// came), a reserve for a client that sends unevenly. When the client is slower than the output,
// the lines keep the last word and the clocks are counted (waitClocks). The memory (CMemoryManager
// MM_GPIO_OUT) is set by the caller beforehand. Needs an FPGA whose output tells its free
// buffers (HardwareFeatures::gpioOutRefill).
class GPIOOutStreamServer {
   public:
    using Ptr = std::shared_ptr<GPIOOutStreamServer>;

    explicit GPIOOutStreamServer(uio_lib::CHardware::Ptr hw);
    ~GPIOOutStreamServer();

    auto start(const CStreamSettings& settings, const GPIOStreamOptions& options = {}) -> bool;
    auto stop(GPIOStopReason reason) -> void;
    auto stopNonBlocking(GPIOStopReason reason) -> void;
    // started and not stopped (the output itself may still fill its ring)
    auto isRunning() -> bool;
    // the clocks the output waited for a buffer so far
    auto waitClocks() -> uint64_t;

    auto setVerbose(bool enable) -> void;
    auto setCallback(std::shared_ptr<GPIOOutServerCallback> callback) -> void;
    auto removeCallback() -> void;

   private:
    GPIOOutStreamServer(const GPIOOutStreamServer&) = delete;
    GPIOOutStreamServer& operator=(const GPIOOutStreamServer&) = delete;

    struct Impl;
    std::shared_ptr<Impl> m_pimpl;  // shared with the threads of stopNonBlocking()
};

}  // namespace gpio_streaming_lib

#endif
