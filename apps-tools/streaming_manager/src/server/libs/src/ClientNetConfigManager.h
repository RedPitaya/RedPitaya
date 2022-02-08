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
        GET_NEW_SETTING,
        SUCCESS_SEND_CONFIG,
        FAIL_SEND_CONFIG,
        SUCCESS_SAVE_CONFIG,
        FAIL_SAVE_CONFIG,
        SERVER_STARTED_TCP,
        SERVER_STARTED_UDP,
        SERVER_STARTED_SD,
        SERVER_STOPPED,
        SERVER_STOPPED_SD_FULL,
        SERVER_STOPPED_SD_DONE,
        SERVER_DAC_STARTED,
        SERVER_DAC_STARTED_SD,
        SERVER_DAC_STOPPED,
        SERVER_DAC_STOPPED_SD_DONE,
        SERVER_DAC_STOPPED_SD_EMPTY,
        SERVER_DAC_STOPPED_SD_BROKEN,
        SERVER_DAC_STOPPED_SD_MISSING,

        SERVER_LOOPBACK_STARTED,
        SERVER_LOOPBACK_STOPPED,
        SERVER_LOOPBACK_BUSY,
        COPY_SETTINGS_TO_TEST_SETTINGS_DONE,

        SERVER_MODE_TCP,
        SERVER_MODE_UDP,
        SERVER_MODE_SD,
        GET_SERVER_MODE,
        GET_SERVER_TEST_MODE
    };
    ClientNetConfigManager(std::string default_file_settings_path,bool loadConfig = true);
    ~ClientNetConfigManager();
    auto startBroadcast(std::string host,std::string port) -> void;
    auto getBroadcastClients() -> const std::list<BroadCastClients>;
    auto connectToServers(std::vector<std::string> _hosts,std::string port) -> void;
    auto diconnectAll() -> void;
    auto getHosts() -> std::list<std::string>;
    auto isServersConnected() -> bool;
    auto sendConfig(const std::string &host) -> bool;
    auto sendTestConfig(const std::string &host,const CStreamSettings &settings) -> bool;
    auto sendCopyConfigToTest(const std::string &host) -> bool;
    auto sendSaveToFile(const std::string &host) -> bool;
    auto sendStart(const std::string &host,bool test_mode = false) -> bool;
    auto sendStop(const std::string &host) -> bool;
    auto sendDACStart(const std::string &host,bool test_mode = false) -> bool;
    auto sendDACStop(const std::string &host) -> bool;
    auto sendGetServerMode(const std::string &host) -> bool;
    auto sendGetServerTestMode(const std::string &host) -> bool;
    auto requestConfig(const std::string &host) -> bool;
    auto requestTestConfig(const std::string &host) -> bool;
    auto getModeByHost(const std::string &host) -> asionet_broadcast::CAsioBroadcastSocket::ABMode;
    auto getLocalSettingsOfHost(const std::string &host) -> CStreamSettings*;
    auto getLocalTestSettingsOfHost(const std::string &host) -> CStreamSettings*;

    auto sendLoopbackStart(const std::string &host) -> bool;
    auto sendLoopbackStop(const std::string &host) -> bool;

    auto sendLoopbackDACSpeed(const std::string &host,int32_t value,bool _async = true) -> bool;
    auto sendLoopbackMode(const std::string &host,int32_t value,bool _async = true) -> bool;
    auto sendLoopbackChannels(const std::string &host,int32_t value,bool _async = true) -> bool;
    auto sendLoopbackTimeout(const std::string &host,int32_t value,bool _async = true) -> bool;

    auto addHandlerError(std::function<void(ClientNetConfigManager::Errors,std::string)> _func) -> void;
    auto addHandler(ClientNetConfigManager::Events event, std::function<void(std::string)> _func) -> void;
    auto removeHadlers() -> void;

private:
    struct Clients{
        enum class States{
            NORMAL,
            GET_DATA
        };
        States m_current_state = States::NORMAL;
        CStreamSettings m_client_settings;
        CStreamSettings m_testSettings;
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
    auto sendTestConfig(std::shared_ptr<Clients> _client, bool _async,const CStreamSettings &settings) -> bool;

    EventList<Errors,std::string> m_errorCallback;
    EventList<std::string> m_callbacksStr;
    std::string m_file_settings;
    std::list<BroadCastClients> m_broadcastClients;
    std::list<std::shared_ptr<Clients>> m_clients;
};
