#ifndef GPIO_STREAMING_LIB_GPIO_STREAM_SERVER_H
#define GPIO_STREAMING_LIB_GPIO_STREAM_SERVER_H

#include <memory>
#include <string>

#include "settings_lib/stream_settings.h"
#include "uio_lib/hardware.h"

namespace gpio_streaming_lib {

class GPIOStreamServer;

enum class GPIOStopReason { NORMAL, MEM_ERROR, MEM_MODIFY, CONFIG_ERROR };

// Events of the server, called from its threads; the default does nothing
class GPIOServerCallback {
   public:
    virtual ~GPIOServerCallback() {}
    virtual void started(GPIOStreamServer*) {}
    // after every stop, also of a server that was not running (a client stops before it starts)
    virtual void stopped(GPIOStreamServer*, GPIOStopReason) {}
};

struct GPIOStreamOptions {
    std::string host = "127.0.0.1";  // the address the data servers listen on
};

// The GPIO streaming of a board (DIO0..7 P and N) through CHardware, over the network: the input
// lines go out on NET_GPIO_IN_STREAMING_PORT, the output lines come in on
// NET_GPIO_OUT_STREAMING_PORT; the settings choose either or both. A buffer is one pack of one
// channel of 32 bit words with a DataLib::GPIOHeader.
// start() sets everything up and opens the data servers. The input starts at once; with the
// output the block starts when the first output buffer has come (the input with it, so both
// begin at the same clock). The memory for the GPIO (CMemoryManager MM_GPIO) is set by the
// caller beforehand; with both directions each gets half of it.
class GPIOStreamServer {
   public:
    using Ptr = std::shared_ptr<GPIOStreamServer>;

    explicit GPIOStreamServer(uio_lib::CHardware::Ptr hw);
    ~GPIOStreamServer();

    auto start(const CStreamSettings& settings, const GPIOStreamOptions& options = {}) -> bool;
    auto stop(GPIOStopReason reason) -> void;
    auto stopNonBlocking(GPIOStopReason reason) -> void;
    // started and not stopped (the block itself may still wait for the first output buffer)
    auto isRunning() -> bool;

    auto setVerbose(bool enable) -> void;
    auto setCallback(std::shared_ptr<GPIOServerCallback> callback) -> void;
    auto removeCallback() -> void;

   private:
    GPIOStreamServer(const GPIOStreamServer&) = delete;
    GPIOStreamServer& operator=(const GPIOStreamServer&) = delete;

    struct Impl;
    std::shared_ptr<Impl> m_pimpl;  // shared with the threads of stopNonBlocking()
};

}  // namespace gpio_streaming_lib

#endif
