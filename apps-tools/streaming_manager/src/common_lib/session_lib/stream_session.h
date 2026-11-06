#ifndef SESSION_LIB_STREAM_SESSION_H
#define SESSION_LIB_STREAM_SESSION_H

#include <functional>
#include <memory>
#include <string>

#include "config_net_lib/server_net_config_manager.h"
#include "dac_streaming_lib/dac_stream_server.h"
#include "gpio_streaming_lib/gpio_in_stream_server.h"
#include "gpio_streaming_lib/gpio_out_stream_server.h"
#include "net_lib/asio_common.h"
#include "streaming_lib/adc_stream_server.h"
#include "uio_lib/hardware.h"

namespace session_lib {

struct SessionOptions {
    std::string configFile;           // the settings, read at the start; made with the defaults if missing
    std::string host;                 // the config server listens on it; empty - every interface
    uint16_t configPort = NET_CONFIG_PORT;
    std::string adcHost;              // the ADC data server listens on it; empty - its default
    bool broadcast = true;            // announce the board on the network
    uint8_t broadcastModel = 0;       // the board model the announcement carries
    std::string broadcastMac = "NONE";
    std::string broadcastHost;        // the address announced; empty - host, or 127.0.0.1
    uint16_t broadcastPort = NET_BROADCAST_PORT;
    bool verbose = false;
    bool adcTestMode = false;         // the ADC benchmark: no DMA addresses
};

// The events of the streams for the application (the web page); the config protocol is answered
// by the session itself. Called on the threads of the streams and of the config channel.
class SessionCallback {
   public:
    virtual ~SessionCallback() {}
    virtual void settingsChanged(const CStreamSettings& /* before */, const CStreamSettings& /* after */) {}
    virtual void adcStarted(bool /* file */) {}
    virtual void adcRunning() {}  // the scope runs (the second start command)
    virtual void adcStopped(streaming_lib::ADCStopReason) {}
    virtual void dacStarted(bool /* file */) {}
    virtual void dacStopped(dac_streaming_lib::DACStopReason) {}
    virtual void gpioInStarted() {}
    virtual void gpioInStopped(gpio_streaming_lib::GPIOStopReason) {}
    virtual void gpioOutStarted() {}
    virtual void gpioOutStopped(gpio_streaming_lib::GPIOStopReason, uint64_t /* waitClocks */) {}
};

// The streaming of one board: the settings and the config server with its broadcast, the ADC, DAC,
// GPIO input and GPIO output servers, the memory of the streams and the trigger commands.
// streaming-server and the web application are this and their own front end.
//
// The streams run on their own: one client may start the ADC and the DAC over one config
// connection (adc_dac_example_1). A change of the settings stops nothing but for a new block
// size, which stops every stream (MEM_MODIFY) to split the memory again; the memory size of a
// stream takes effect at its next start.
class StreamSession {
   public:
    using Ptr = std::shared_ptr<StreamSession>;

    // Starts the config server (and the broadcast) on hw. Throws if the server cannot start.
    static auto create(uio_lib::CHardware::Ptr hw, const SessionOptions& options) -> Ptr;
    ~StreamSession();

    // Stops the streams (NORMAL), the config server and the broadcast. Once; also the destructor's.
    auto stop() -> void;

    auto isMaster() const -> bool;
    auto hardware() const -> uio_lib::CHardware::Ptr;
    auto config() const -> ServerNetConfigManager::Ptr;

    auto settings() -> CStreamSettings;
    // The application's change of the settings, checked like the network's
    auto updateSettings(const std::function<bool(CStreamSettings&)>& change) -> bool;
    auto saveSettings() -> bool;
    auto setBroadcastAddress(const std::string& host) -> void;

    // The streams. The starts take the settings of the moment.
    auto startADCStreaming() -> bool;
    auto startADC() -> void;  // the scope, after startADCStreaming
    auto stopADCStreaming(streaming_lib::ADCStopReason reason, bool wait = false) -> void;
    auto startDAC(dac_channels_t channels = dac_channels_t()) -> bool;
    auto stopDAC(dac_streaming_lib::DACStopReason reason, bool wait = false) -> void;
    auto startGPIOIn() -> bool;
    auto stopGPIOIn(gpio_streaming_lib::GPIOStopReason reason, bool wait = false) -> void;
    auto startGPIOOut() -> bool;
    auto stopGPIOOut(gpio_streaming_lib::GPIOStopReason reason, bool wait = false) -> void;

    auto adcPassSamples() -> uint64_t;
    auto adcWrittenSize() -> uint64_t;

    auto setCallback(std::shared_ptr<SessionCallback> callback) -> void;
    auto removeCallback() -> void;

    StreamSession(const StreamSession&) = delete;
    StreamSession& operator=(const StreamSession&) = delete;

    struct Impl;

   private:
    explicit StreamSession(std::shared_ptr<Impl> impl);
    std::shared_ptr<Impl> m_pimpl;
};

}  // namespace session_lib

#endif
