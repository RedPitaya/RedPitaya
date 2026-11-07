#ifndef DAC_STREAMING_LIB_DAC_STREAM_SERVER_H
#define DAC_STREAMING_LIB_DAC_STREAM_SERVER_H

#include <memory>
#include <string>

#include "settings_lib/channels.hpp"
#include "settings_lib/stream_settings.h"
#include "uio_lib/hardware.h"

namespace dac_streaming_lib {

class DACStreamServer;

// DONE: the file has been played; EMPTY, BROKEN, MISSING_FILE: the file cannot be played
// DISCONNECTED: the network client that sends the data has gone
enum class DACStopReason { NORMAL, DONE, EMPTY, BROKEN, MISSING_FILE, MEM_ERROR, MEM_MODIFY, NO_CHANNELS, CONFIG_ERROR, DISCONNECTED };

// Events of the server, called from its threads; the default does nothing
class DACServerCallback {
   public:
    virtual ~DACServerCallback() {}
    virtual void startedTCP(DACStreamServer*) {}
    virtual void startedSD(DACStreamServer*) {}
    // after every stop, also of a server that was not running (a client stops before it starts)
    virtual void stopped(DACStreamServer*, DACStopReason) {}
};

struct DACStreamOptions {
    std::string host;  // the address the data server listens on; empty - every interface
    std::string dirPath;             // the directory of the files, CStreamSettings::getDACDirPath() when empty
};

// The DAC streaming of a board: the generator through CHardware, the buffer ring, the network data
// server or the file on the SD card. start() sets everything up and starts the playback; in the
// network mode it plays what the client sends. The memory for the DAC (CMemoryManager MM_DAC) is
// set by the caller beforehand.
class DACStreamServer {
   public:
    using Ptr = std::shared_ptr<DACStreamServer>;

    explicit DACStreamServer(uio_lib::CHardware::Ptr hw);
    ~DACStreamServer();

    // channels: the channels the client sends in the network mode; a file brings its own
    auto start(const CStreamSettings& settings, dac_channels_t channels, const DACStreamOptions& options = {}) -> bool;
    auto stop(DACStopReason reason) -> void;
    auto stopNonBlocking(DACStopReason reason) -> void;
    auto isRunning() -> bool;

    auto setVerbose(bool enable) -> void;
    auto setCallback(std::shared_ptr<DACServerCallback> callback) -> void;
    auto removeCallback() -> void;

   private:
    DACStreamServer(const DACStreamServer&) = delete;
    DACStreamServer& operator=(const DACStreamServer&) = delete;

    struct Impl;
    std::shared_ptr<Impl> m_pimpl;  // shared with the threads of stopNonBlocking()
};

}  // namespace dac_streaming_lib

#endif
