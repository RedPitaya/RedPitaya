#ifndef CONFIG_NET_LIB_SNCM_H
#define CONFIG_NET_LIB_SNCM_H

#include "settings_lib/stream_settings.h"
#include "broadcast_lib/asio_broadcast_socket.h"
#include "net_config_manager.h"
#include "data_lib/signal.hpp"

class ServerNetConfigManager{
public:
    enum class Errors{
        CANNT_SET_DATA,
        SERVER_INTERNAL,
        BREAK_RECEIVE_SETTINGS,
        BROADCAST_ERROR
    };

    enum EStopReason{
        NORMAL  = 0,
        SD_FULL = 1,
        DONE    = 2
    };

    using Ptr = std::shared_ptr<ServerNetConfigManager>;

    ServerNetConfigManager(std::string defualt_file_settings_path,broadcast_lib::EMode mode, std::string host,std::string port);
    ~ServerNetConfigManager();

    auto startServer(std::string host,std::string port) -> void;
    auto startBroadcast(broadcast_lib::EModel model,std::string host,std::string port) -> void;
    auto stop() -> void;
    auto isConnected() -> bool;
    auto sendServerStartedTCP() -> bool;
    auto sendServerStartedUDP() -> bool;
    auto sendServerStartedSD() -> bool;
    auto sendServerModeTCP() -> bool;
    auto sendServerModeUDP() -> bool;
    auto sendServerModeSD() -> bool;
    auto sendDACServerStarted() -> bool;
    auto sendDACServerStartedSD() -> bool;

    auto sendADCStarted() -> bool;
    auto sendDACStarted() -> bool;

    auto sendConfigFileMissed() -> bool;
    auto sendServerStopped() -> bool;
    auto sendServerStoppedSDFull() -> bool;
    auto sendServerStoppedDone() -> bool;
    auto sendDACServerStopped() -> bool;
    auto sendDACServerStoppedSDDone() -> bool;
    auto sendDACServerStoppedSDEmpty() -> bool;
    auto sendDACServerStoppedSDBroken() -> bool;
    auto sendDACServerStoppedSDMissingFile() -> bool;

    auto sendServerStartedLoopBackMode() -> bool;
    auto sendServerStoppedLoopBackMode() -> bool;
    auto sendStreamServerBusy() -> bool;
    
    auto getSettingsRef() -> CStreamSettings&;
    auto getSettings() -> const CStreamSettings;
    auto getTempSettings() -> const CStreamSettings;
    
    sigslot::signal<> getNewSettingsNofiy;
    sigslot::signal<> getNewTestSettingsNofiy;

    sigslot::signal<> stopStreamingNofiy;
    sigslot::signal<> startStreamingNofiy;
    sigslot::signal<> startStreamingTestNofiy;

    sigslot::signal<> clientConnectedNofiy;
    sigslot::signal<> clientDisconnectedNofiy;

    sigslot::signal<> startDacStreamingNofiy;
    sigslot::signal<> startDacStreamingTestNofiy;
    sigslot::signal<> stopDacStreamingNofiy;

    sigslot::signal<> startLoopbackModeNofiy;
    sigslot::signal<> stopLoopbackModeNofiy;

    sigslot::signal<> getServerModeNofiy;
    sigslot::signal<> getServerModeTestNofiy;

    sigslot::signal<> startADCNofiy;
    sigslot::signal<> startDACNofiy;

    sigslot::signal<ServerNetConfigManager::Errors> errorNofiy;

private:
    enum class States{
        NORMAL,
        GET_DATA,
        GET_TEMP_DATA
    };

    std::shared_ptr<broadcast_lib::CAsioBroadcastSocket> m_pBroadcast;
    std::shared_ptr<CNetConfigManager> m_pNetConfManager;

    auto receiveCommand(uint32_t command) -> void;
    auto receiveValueStr(std::string key,std::string value) -> void;
    auto receiveValueInt(std::string key,uint32_t value) -> void;
    auto receiveValueDouble(std::string key,double value) -> void;
    auto connected(std::string host) -> void;
    auto disconnected(std::string host) -> void;
    auto serverError(std::error_code error) -> void;
    auto sendConfig(bool sendTest, bool _async) -> bool;

    States m_currentState;
//    EventList<Errors> m_errorCallback;
//    EventList<> m_callbacks;
    std::string m_file_settings;
    broadcast_lib::EMode m_mode;
    CStreamSettings m_settings;
    CStreamSettings m_testSettings;
};

#endif
