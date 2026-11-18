#ifndef GPIO_STREAMING_LIB_GPIO_IN_STREAM_SERVER_H
#define GPIO_STREAMING_LIB_GPIO_IN_STREAM_SERVER_H

#include <memory>
#include <string>

#include "gpio_stop_reason.h"
#include "settings_lib/stream_settings.h"
#include "uio_lib/hardware.h"

namespace gpio_streaming_lib {

class GPIOInStreamServer;

// Events of the server, called from its threads; the default does nothing
class GPIOInServerCallback {
   public:
    virtual ~GPIOInServerCallback() {}
    virtual void started(GPIOInStreamServer*) {}
    // the file mode: the input writes the file (instead of started)
    virtual void startedSD(GPIOInStreamServer*) {}
    // after every stop, also of a server that was not running (a client stops before it starts)
    virtual void stopped(GPIOInStreamServer*, GPIOStopReason) {}
};

struct GPIOStreamOptions {
    std::string host;  // the address the data server listens on; empty - every interface
    std::string dirPath;             // the file mode: the directory, CStreamSettings::getGPIODirPath() when empty
};

// The GPIO input of a board (DIO0..7 P and N) through CHardware: the lines go to the client on
// NET_GPIO_IN_STREAMING_PORT, a buffer is one pack of one channel of 32 bit words {N count,
// N lines, P count, P lines} with a DataLib::GPIOHeader. With gpio_in_pass_mode FILE they go to
// a file gpio_<UTC time>.bin|csv|vcd instead (CGpioFileWriter of writer_lib). It runs on its own, the output may run
// or not. start() sets everything up, opens the data server and starts the input. The memory
// (CMemoryManager MM_GPIO_IN) is set by the caller beforehand.
class GPIOInStreamServer {
   public:
    using Ptr = std::shared_ptr<GPIOInStreamServer>;

    explicit GPIOInStreamServer(uio_lib::CHardware::Ptr hw);
    ~GPIOInStreamServer();

    auto start(const CStreamSettings& settings, const GPIOStreamOptions& options = {}) -> bool;
    auto stop(GPIOStopReason reason) -> void;
    auto stopNonBlocking(GPIOStopReason reason) -> void;
    auto isRunning() -> bool;
    // the losses of the last run (reset by start, kept after the stop): samples lost on the
    // board (the FPGA found no free buffer) and the samples it gave
    auto getBoardLost() -> uint64_t;
    auto getBoardSamples() -> uint64_t;
    // The recording of the current or last run (kept after the stop): where it goes, the file
    struct Record {
        int mode = 0;  // 0 none yet, 1 network, 2 SD
        bool running = false;
        uint64_t samples = 0;  // the time line of the file in samples (the gaps included)
        uint64_t limit = 0;
        uint64_t written = 0;  // bytes
        double writeRate = 0;
        double queue = 0;
        std::string file;
    };
    auto getRecord() -> Record;
    // the file of the file mode, "" in the network mode
    auto filePath() -> std::string;

    auto setVerbose(bool enable) -> void;
    auto setCallback(std::shared_ptr<GPIOInServerCallback> callback) -> void;
    auto removeCallback() -> void;

   private:
    GPIOInStreamServer(const GPIOInStreamServer&) = delete;
    GPIOInStreamServer& operator=(const GPIOInStreamServer&) = delete;

    struct Impl;
    std::shared_ptr<Impl> m_pimpl;  // shared with the threads of stopNonBlocking()
};

}  // namespace gpio_streaming_lib

#endif
