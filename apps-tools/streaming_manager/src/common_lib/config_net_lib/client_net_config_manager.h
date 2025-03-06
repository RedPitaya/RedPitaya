#ifndef CONFIG_NET_LIB_CNCM_H
#define CONFIG_NET_LIB_CNCM_H

#include <ctime>
#include "broadcast_lib/asio_broadcast_socket.h"
#include "data_lib/signal.hpp"
#include "net_config_manager.h"
#include "settings_lib/stream_settings.h"

class ClientNetConfigManager : public CStreamSettings {
   public:
    struct BroadCastClients {
        std::string host;
        broadcast_lib::EMode mode;
        broadcast_lib::EModel model;
        std::time_t ts;
    };

    enum class Errors { CANNT_SET_DATA_TO_CONFIG, ERROR_SEND_CONFIG, SERVER_INTERNAL, BROADCAST_ERROR, BROADCAST_ERROR_PARSE, CONNECT_TIMEOUT };

    using Ptr = std::shared_ptr<ClientNetConfigManager>;

    ClientNetConfigManager(std::string default_file_settings_path, bool loadConfig = true);
    ~ClientNetConfigManager();

    auto startBroadcast(std::string host, uint16_t port) -> void;
    auto connectToServers(std::vector<std::string> _hosts, uint16_t port) -> void;
    auto isServersConnected() -> bool;
    auto diconnectAll() -> void;

    auto getBroadcastClients() -> const std::list<BroadCastClients>;
    auto getHosts() -> std::list<std::string>;
    auto getModeByHost(const std::string& host) -> broadcast_lib::EMode;
    auto getLocalSettingsOfHost(const std::string& host) -> CStreamSettings*;

    auto sendConfigVariable(const std::string& host, const std::string key, const std::string value) -> bool;
    auto sendConfig(const std::string& host) -> bool;
    auto sendSaveToFile(const std::string& host) -> bool;
    auto sendADCServerStart(const std::string& host) -> bool;
    auto sendADCServerStop(const std::string& host) -> bool;
    auto sendDACServerStart(const std::string& host, const std::string activeChannels) -> bool;
    auto sendDACServerStop(const std::string& host) -> bool;
    auto sendADCFPGAStart(const std::string& host) -> bool;
    auto sendDACFPGAStart(const std::string& host) -> bool;
    auto sendGetServerMode(const std::string& host) -> bool;
    auto sendActiveDACChannels(const std::string& host, std::string& count) -> bool;

    auto requestConfig(const std::string& host) -> bool;
    auto requestConfigVariable(const std::string& host, const std::string& variable) -> bool;
    auto requestMemoryBlockSize(const std::string& host) -> bool;
    auto requestActiveChannels(const std::string& host) -> bool;

    auto removeHadlers() -> void;

    sigslot::signal<std::string&> broadCastNewClientNofiy;
    sigslot::signal<std::string&> serverConnectedNofiy;
    sigslot::signal<std::string&> getNewSettingsNofiy;
    sigslot::signal<std::string&> getNewSettingsItemNofiy;
    sigslot::signal<std::string&> successSendConfigNofiy;
    sigslot::signal<std::string&> failSendConfigNofiy;
    sigslot::signal<std::string&> successSaveConfigNofiy;
    sigslot::signal<std::string&> failSaveConfigNofiy;

    sigslot::signal<std::string&> serverStartedTCPNofiy;
    sigslot::signal<std::string&> serverStartedSDNofiy;
    sigslot::signal<std::string&> serverStoppedNofiy;
    sigslot::signal<std::string&> serverStoppedNoActiveChannelsNofiy;
    sigslot::signal<std::string&> serverStoppedMemErrorNofiy;
    sigslot::signal<std::string&> serverStoppedMemModifyNofiy;
    sigslot::signal<std::string&> serverStoppedSDFullNofiy;
    sigslot::signal<std::string&> serverStoppedSDDoneNofiy;

    sigslot::signal<std::string&> serverDacStartedNofiy;
    sigslot::signal<std::string&> serverDacStartedSDNofiy;
    sigslot::signal<std::string&> serverDacStoppedNofiy;
    sigslot::signal<std::string&> serverDacStoppedMemErrorNofiy;
    sigslot::signal<std::string&> serverDacStoppedMemModifyNofiy;
    sigslot::signal<std::string&> serverDacStoppedConfigErrorNofiy;
    sigslot::signal<std::string&> serverDacStoppedSDDoneNofiy;
    sigslot::signal<std::string&> serverDacStoppedSDEmptyNofiy;
    sigslot::signal<std::string&> serverDacStoppedSDBrokenNofiy;
    sigslot::signal<std::string&> serverDacStoppedSDMissingNofiy;

    sigslot::signal<std::string&> serverModeTCPNofiy;
    sigslot::signal<std::string&> serverModeSDNofiy;

    sigslot::signal<std::string&> startADCDoneNofiy;
    sigslot::signal<std::string&> startDACDoneNofiy;

    sigslot::signal<std::string&> configFileMissedNotify;

    sigslot::signal<std::string&, std::string> getMemBlockSizeNofiy;
    sigslot::signal<std::string&, std::string> getActiveChannelsNofiy;
    sigslot::signal<ClientNetConfigManager::Errors, std::string, error_code> errorNofiy;

   private:
    struct Clients {
        enum class States { NORMAL, GET_DATA };
        CStreamSettings m_client_settings;
        std::shared_ptr<CNetConfigManager> m_manager;
        broadcast_lib::EMode m_mode;
    };

    std::shared_ptr<broadcast_lib::CAsioBroadcastSocket> m_pBroadcast;

    auto receiveCommand(uint32_t command, std::string tag, std::weak_ptr<Clients> sender) -> void;
    auto receiveStrStr(std::string key, std::string value, std::weak_ptr<Clients> sender) -> void;
    auto receiveConfig(std::string value, std::weak_ptr<Clients> sender) -> void;

    auto serverError(std::error_code error, std::weak_ptr<Clients> sender) -> void;
    auto connectTimeoutError(std::error_code error, std::weak_ptr<Clients> sender) -> void;

    std::string m_file_settings;
    std::list<BroadCastClients> m_broadcastClients;
    std::list<std::shared_ptr<Clients>> m_clients;
};

#endif
