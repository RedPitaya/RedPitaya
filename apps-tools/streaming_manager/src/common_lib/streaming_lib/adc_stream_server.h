#ifndef STREAMING_LIB_ADC_STREAM_SERVER_H
#define STREAMING_LIB_ADC_STREAM_SERVER_H

#include <memory>
#include <string>

#include "settings_lib/stream_settings.h"
#include "uio_lib/hardware.h"

namespace streaming_lib {

class ADCStreamServer;

// DISCONNECTED: the network client of the data has gone
enum class ADCStopReason { NORMAL, SD_FULL, DONE, MEM_ERROR, MEM_MODIFY, NO_CHANNELS, CONFIG_ERROR, DISCONNECTED };

// Events of the server, called from its threads; the default does nothing
class ADCServerCallback {
   public:
    virtual ~ADCServerCallback() {}
    virtual void startedTCP(ADCStreamServer*) {}
    virtual void startedSD(ADCStreamServer*) {}
    // after every stop, also of a server that was not running (a client stops before it starts)
    virtual void stopped(ADCStreamServer*, ADCStopReason) {}
};

struct ADCStreamOptions {
    std::string host;  // the address the data server listens on; empty - every interface
    std::string filePath;            // the directory of the files, FILE_PATH when empty
    bool testMode = false;           // benchmark: no DMA addresses, at most 100 blocks
};

// The ADC streaming of a board: the scope through CHardware, the buffer ring, the network data
// server or the file on the SD card. start() sets everything up and opens the data server or the
// file; startADC() starts the scope (the second command of the protocol).
// The memory for the ADC (CMemoryManager MM_ADC) is set by the caller beforehand.
class ADCStreamServer {
   public:
    using Ptr = std::shared_ptr<ADCStreamServer>;

    explicit ADCStreamServer(uio_lib::CHardware::Ptr hw);
    ~ADCStreamServer();

    auto start(const CStreamSettings& settings, const ADCStreamOptions& options = {}) -> bool;
    auto startADC() -> void;
    auto stop(ADCStopReason reason) -> void;
    auto stopNonBlocking(ADCStopReason reason) -> void;
    auto isRunning() -> bool;

    auto setVerbose(bool enable) -> void;
    auto setCallback(std::shared_ptr<ADCServerCallback> callback) -> void;
    auto removeCallback() -> void;

    // the file being written (statistics of the web application), nullptr in the network mode
    auto getPassSamples() -> uint64_t;
    auto getWritedSize() -> uint64_t;
    // the write speed of the file (bytes per second) and what its queue holds of its limit, 0..1
    auto getWriteRate() -> double;
    // The losses of the last run (reset by start, kept after the stop), samples per channel (the
    // channel with the most): lost on the board (the FPGA found no free buffer: the client, the
    // network or the SD card too slow), given by the FPGA, and not taken by the SD file (its write
    // queue was full; those buffers are missing from the file).
    auto getBoardLost() -> uint64_t;
    auto getBoardSamples() -> uint64_t;
    auto getFileDropped() -> uint64_t;
    auto getQueueFill() -> double;

   private:
    ADCStreamServer(const ADCStreamServer&) = delete;
    ADCStreamServer& operator=(const ADCStreamServer&) = delete;

    struct Impl;
    std::shared_ptr<Impl> m_pimpl;  // shared with the threads of stopNonBlocking()
};

}  // namespace streaming_lib

#endif
