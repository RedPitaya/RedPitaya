#pragma once
#include <ctime>
#include "common/stream_settings.h"
#include "AsioBroadcastSocket.h"
#include "NetConfigManager.h"
#include "EventHandlers.h"

class ClientNetConfigManager : public  CStreamSettings{

public:
    struct BroadCastClients{
        std::string host;
        asionet_broadcast::CAsioBroadcastSocket::ABMode mode;
        asionet_broadcast::CAsioBroadcastSocket::Model  model;
        std::time_t ts;
    };

    enum class Errors{
        CANNT_SET_DATA_TO_CONFIG,
        ERROR_SEND_CONFIG,
        BREAK_RECEIVE_SETTINGS,
        SERVER_INTERNAL,
        BROADCAST_ERROR,
        BROADCAST_ERROR_PARSE,
        CONNECT_TIMEOUT
    };

    enum class Events{
        BROADCAST_NEW_CLIENT,
        SERVER_CONNECTED,
        GET_NEW_SETTING
    };
    ClientNetConfigManager(std::string default_file_settings_path,bool loadConfig = true);
    ~ClientNetConfigManager();

    auto startBroadcast(std::string host,std::string port) -> void;
    auto getBroadcastClients() -> const std::list<BroadCastClients>;
    auto connectToServers(std::vector<std::string> _hosts,std::string port) -> void;
    auto isServersConnected() -> bool;
    auto sendConfigAll() -> void;
    auto sendConfig(std::string host) -> bool;
    auto requestConfig(std::string host) -> bool;
    auto getLocalSettingsOfHost(std::string host) -> CStreamSettings*;

    auto addHandlerError(std::function<void(ClientNetConfigManager::Errors,std::string)> _func) -> void;
    auto addHandler(ClientNetConfigManager::Events event, std::function<void(std::string)> _func) -> void;
private:
    struct Clients{
        enum class States{
            NORMAL,
            GET_DATA
        };
        States m_current_state = States::NORMAL;
        CStreamSettings m_client_settings;
        std::shared_ptr<CNetConfigManager> m_manager;
        asionet_broadcast::CAsioBroadcastSocket::ABMode m_mode;
    };

    std::shared_ptr<asionet_broadcast::CAsioBroadcastSocket> m_pBroadcast;

    auto receiveCommand(uint32_t command,std::shared_ptr<Clients> sender) -> void;
    auto receiveValueStr(std::string key,std::string value,std::shared_ptr<Clients> sender) -> void;
    auto receiveValueInt(std::string key,uint32_t value,std::shared_ptr<Clients> sender) -> void;
    auto receiveValueDouble(std::string key,double value,std::shared_ptr<Clients> sender) -> void;
    auto serverError(std::error_code error,std::shared_ptr<Clients> sender) -> void;
    auto connectTimeoutError(std::error_code error,std::shared_ptr<Clients> sender) -> void;
    auto sendConfig(std::shared_ptr<Clients> _client, bool _async) -> bool;

    EventList<Errors,std::string> m_errorCallback;
    EventList<std::string> m_callbacksStr;
    std::string m_file_settings;
    std::list<BroadCastClients> m_broadcastClients;
    std::list<std::shared_ptr<Clients>> m_clients;
};