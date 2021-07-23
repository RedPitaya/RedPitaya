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
        std::time_t ts;
    };

    enum class Errors{
        CANNT_SET_DATA,
        SERVER_INTERNAL,
        BREAK_RECEIVE_SETTINGS,
        BROADCAST_ERROR,
        BROADCAST_ERROR_PARSE
    };

    enum class Commands{
        BROADCAST_NEW_CLIENT,
        SERVER_CONNECTED
    };
    ClientNetConfigManager(std::string default_file_settings_path);
    ~ClientNetConfigManager();

    auto startBroadcast(std::string host,std::string port) -> void;
    auto getBroadcastClients() -> const std::list<BroadCastClients>;
    auto connectToServers(std::vector<std::string> _hosts,std::string port) -> void;
    auto isServersConnected() -> bool;

    auto addHandlerError(std::function<void(ClientNetConfigManager::Errors)> _func) -> void;
    auto addHandler(ClientNetConfigManager::Commands event,std::function<void()> _func) -> void;
private:
    struct Clients{
        std::shared_ptr<CNetConfigManager> m_manager;
        asionet_broadcast::CAsioBroadcastSocket::ABMode m_mode;
    };

    std::shared_ptr<asionet_broadcast::CAsioBroadcastSocket> m_pBroadcast;

    auto receiveCommand(uint32_t command,std::shared_ptr<CNetConfigManager> sender) -> void;
    auto receiveValueStr(std::string key,std::string value,std::shared_ptr<CNetConfigManager> sender) -> void;
    auto receiveValueInt(std::string key,uint32_t value,std::shared_ptr<CNetConfigManager> sender) -> void;
    auto receiveValueDouble(std::string key,double value,std::shared_ptr<CNetConfigManager> sender) -> void;
    auto serverError(std::error_code error,std::shared_ptr<CNetConfigManager> sender) -> void;

    EventList<Errors> m_errorCallback;
    EventList<> m_callbacks;
    std::string m_file_settings;
    std::list<BroadCastClients> m_broadcastClients;
    std::list<Clients> m_clients;
};