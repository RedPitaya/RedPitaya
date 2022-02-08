#pragma once
#include "common/stream_settings.h"
#include "AsioBroadcastSocket.h"
#include "NetConfigManager.h"
#include "EventHandlers.h"

class ServerNetConfigManager{
public:
    enum class Errors{
        CANNT_SET_DATA,
        SERVER_INTERNAL,
        BREAK_RECEIVE_SETTINGS,
        BROADCAST_ERROR
    };

    enum class Events{
        GET_NEW_SETTING,
        GET_NEW_TEST_SETTING,        
        STOP_STREAMING,
        START_STREAMING,
        START_STREAMING_TEST,
        CLIENT_CONNECTED,
        CLIENT_DISCONNECTED,
        START_DAC_STREAMING,
        START_DAC_STREAMING_TEST,
        STOP_DAC_STREAMING,
        START_LOOPBACK_MODE,
        STOP_LOOPBACK_MODE,
        GET_SERVER_MODE,
        GET_SERVER_TEST_MODE
    };
    ServerNetConfigManager(std::string defualt_file_settings_path,asionet_broadcast::CAsioBroadcastSocket::ABMode mode, std::string host,std::string port);
    ~ServerNetConfigManager();

    auto startServer(std::string host,std::string port) -> void;
    auto startBroadcast(asionet_broadcast::CAsioBroadcastSocket::Model model,std::string host,std::string port) -> void;
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
    
    auto addHandlerError(std::function<void(ServerNetConfigManager::Errors)> _func) -> void;
    auto addHandler(ServerNetConfigManager::Events event, std::function<void()> _func) -> void;

private:
    enum class States{
        NORMAL,
        GET_DATA,
        GET_TEMP_DATA
    };

    std::shared_ptr<asionet_broadcast::CAsioBroadcastSocket> m_pBroadcast;
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
    EventList<Errors> m_errorCallback;
    EventList<> m_callbacks;
    std::string m_file_settings;
    asionet_broadcast::CAsioBroadcastSocket::ABMode m_mode;
    CStreamSettings m_settings;
    CStreamSettings m_testSettings;
};
