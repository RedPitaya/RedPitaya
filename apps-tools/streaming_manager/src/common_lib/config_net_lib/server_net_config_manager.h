#ifndef CONFIG_NET_LIB_SNCM_H
#define CONFIG_NET_LIB_SNCM_H

#include <mutex>
#include "broadcast_lib/asio_broadcast_socket.h"
#include "data_lib/signal.hpp"
#include "net_config_manager.h"
#include "settings_lib/channels.hpp"
#include "settings_lib/stream_settings.h"
#include "trigger_status.h"

class ServerNetConfigManager {
   public:
    enum Errors { CANNT_SET_DATA = 0, SERVER_INTERNAL = 1, BROADCAST_ERROR = 2 };

    enum EStopReason {
        NORMAL = 0,
        SD_FULL = 1,
        DONE = 2,
        MEM_ERROR = 3,
        MEM_MODIFY = 4,
        NO_CHANNELS = 5,
        CONFIG_ERROR = 6
    };

    using Ptr = std::shared_ptr<ServerNetConfigManager>;

    ServerNetConfigManager(std::string defualt_file_settings_path, broadcast_lib::EMode mode, std::string host, uint16_t port, uint8_t maxAdcChannels);
    ~ServerNetConfigManager();

    auto startServer(std::string host, uint16_t port) -> void;
    auto startBroadcast(uint8_t model, std::string mac, std::string host, uint16_t port) -> void;
    auto setBroadcastAddress(std::string host) -> void;
    auto setMode(broadcast_lib::EMode mode) -> void;
    auto stop() -> void;
    auto isConnected() -> bool;

    auto sendADCServerStartedTCP() -> bool;
    auto sendADCServerStartedSD() -> bool;
    auto sendADCServerModeTCP() -> bool;
    auto sendADCServerModeSD() -> bool;
    auto sendADCStarted() -> bool;

    auto sendDACServerStarted() -> bool;
    auto sendDACServerStartedSD() -> bool;
    auto sendDACStarted() -> bool;

    auto sendServerStopped() -> bool;
    auto sendServerStoppedSDFull() -> bool;
    auto sendServerStoppedDone() -> bool;
    auto sendServerNoChannelsStopped() -> bool;
    auto sendServerMemoryErrorStopped() -> bool;
    auto sendServerMemoryModifyStopped() -> bool;
    auto sendServerConfigErrorStopped() -> bool;

    auto sendDACServerStopped() -> bool;
    auto sendDACServerMemoryErrorStopped() -> bool;
    auto sendDACServerMemoryModifyStopped() -> bool;
    auto sendDACServerConfigErrorStopped() -> bool;
    auto sendDACServerStoppedSDDone() -> bool;
    auto sendDACServerStoppedSDEmpty() -> bool;
    auto sendDACServerStoppedSDBroken() -> bool;
    auto sendDACServerStoppedSDMissingFile() -> bool;

    auto sendGPIOServerStarted() -> bool;
    auto sendGPIOServerStopped() -> bool;
    auto sendGPIOServerMemoryErrorStopped() -> bool;
    auto sendGPIOServerConfigErrorStopped() -> bool;

    auto sendSoftwareTriggerDone(CNetConfigManager::ETriggerTarget target) -> bool;
    auto sendSoftwareTriggerFail(CNetConfigManager::ETriggerTarget target) -> bool;
    auto sendTriggerStatus(const TriggerStatus& status) -> bool;

    auto sendConfigFileMissed() -> bool;

    // Not synchronized with the config channel: use only while no client changes the settings.
    auto getSettingsRef() -> CStreamSettings&;
    auto setSettingsParameter(const std::string& key, const std::string& value) -> bool;
    auto getSettings() -> const CStreamSettings;

    sigslot::signal<> getNewSettingsNofiy;

    sigslot::signal<> stopStreamingNofiy;
    sigslot::signal<> startStreamingNofiy;

    sigslot::signal<> clientConnectedNofiy;
    sigslot::signal<> clientDisconnectedNofiy;

	sigslot::signal<dac_channels_t> startDacStreamingNofiy;
	sigslot::signal<> stopDacStreamingNofiy;

    sigslot::signal<> getServerModeNofiy;

    sigslot::signal<> startADCNofiy;
    sigslot::signal<> startDACNofiy;

    sigslot::signal<> memoryBlockSizeChangeNofiy;

    sigslot::signal<> startGpioStreamingNofiy;
    sigslot::signal<> stopGpioStreamingNofiy;

    // The handler answers with sendSoftwareTriggerDone/Fail and sendTriggerStatus. Without a
    // handler the request is answered here: trigger fail, empty status, GPIO config error.
    sigslot::signal<CNetConfigManager::ETriggerTarget> softwareTriggerNofiy;
    sigslot::signal<> triggerStatusRequestNofiy;

    sigslot::signal<ServerNetConfigManager::Errors> errorNofiy;

   private:
    enum class States { NORMAL, GET_DATA, GET_TEMP_DATA };

    std::shared_ptr<broadcast_lib::CAsioBroadcastSocket> m_pBroadcast;
    std::shared_ptr<CNetConfigManager> m_pNetConfManager;

    auto receiveCommand(uint32_t command, std::string tag) -> void;
    auto receiveValueStr(std::string key, std::string value) -> void;
    auto receiveConfig(std::string value) -> void;
    auto connected(std::string host) -> void;
    auto disconnected(std::string host) -> void;
    auto serverError(std::error_code error) -> void;
    auto sendConfig(bool _async) -> bool;

    std::string m_file_settings;
    broadcast_lib::EMode m_mode;
    CStreamSettings m_settings;
    // m_settings is changed on the asio thread and read from the streaming threads.
    std::mutex m_settingsMtx;
    uint8_t m_maxADCChannels = 0;
};

#endif
