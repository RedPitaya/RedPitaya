#ifndef CONFIG_NET_LIB_CNCM_H
#define CONFIG_NET_LIB_CNCM_H

#include <ctime>
#include "settings_lib/stream_settings.h"
#include "broadcast_lib/asio_broadcast_socket.h"
//#include "net_lib/event_handlers.h"
#include "net_config_manager.h"
#include "data_lib/signal.hpp"

class ClientNetConfigManager : public  CStreamSettings{

public:
    struct BroadCastClients{
        std::string host;
        broadcast_lib::EMode mode;
        broadcast_lib::EModel  model;
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

    using Ptr = std::shared_ptr<ClientNetConfigManager>;

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
    auto sendStartADC(const std::string &host) -> bool;
    auto sendStartDAC(const std::string &host) -> bool;
    auto sendGetServerMode(const std::string &host) -> bool;
    auto sendGetServerTestMode(const std::string &host) -> bool;
    auto requestConfig(const std::string &host) -> bool;
    auto requestTestConfig(const std::string &host) -> bool;
    auto getModeByHost(const std::string &host) -> broadcast_lib::EMode;
    auto getLocalSettingsOfHost(const std::string &host) -> CStreamSettings*;
    auto getLocalTestSettingsOfHost(const std::string &host) -> CStreamSettings*;

    auto sendLoopbackStart(const std::string &host) -> bool;
    auto sendLoopbackStop(const std::string &host) -> bool;

    auto sendLoopbackDACSpeed(const std::string &host,int32_t value,bool _async = true) -> bool;
    auto sendLoopbackMode(const std::string &host,int32_t value,bool _async = true) -> bool;
    auto sendLoopbackChannels(const std::string &host,int32_t value,bool _async = true) -> bool;
    auto sendLoopbackTimeout(const std::string &host,int32_t value,bool _async = true) -> bool;

    auto removeHadlers() -> void;

    sigslot::signal<std::string&> broadCastNewClientNofiy;
    sigslot::signal<std::string&> serverConnectedNofiy;
    sigslot::signal<std::string&> getNewSettingsNofiy;
    sigslot::signal<std::string&> successSendConfigNofiy;
    sigslot::signal<std::string&> failSendConfigNofiy;
    sigslot::signal<std::string&> successSaveConfigNofiy;
    sigslot::signal<std::string&> failSaveConfigNofiy;

    sigslot::signal<std::string&> serverStartedTCPNofiy;
    sigslot::signal<std::string&> serverStartedUDPNofiy;
    sigslot::signal<std::string&> serverStartedSDNofiy;
    sigslot::signal<std::string&> serverStoppedNofiy;
    sigslot::signal<std::string&> serverStoppedSDFullNofiy;
    sigslot::signal<std::string&> serverStoppedSDDoneNofiy;

    sigslot::signal<std::string&> serverDacStartedNofiy;
    sigslot::signal<std::string&> serverDacStartedSDNofiy;
    sigslot::signal<std::string&> serverDacStoppedNofiy;
    sigslot::signal<std::string&> serverDacStoppedSDDoneNofiy;
    sigslot::signal<std::string&> serverDacStoppedSDEmptyNofiy;
    sigslot::signal<std::string&> serverDacStoppedSDBrokenNofiy;
    sigslot::signal<std::string&> serverDacStoppedSDMissingNofiy;

    sigslot::signal<std::string&> serverLoopbackStartedNofiy;
    sigslot::signal<std::string&> serverLoopbackStoppedNofiy;
    sigslot::signal<std::string&> serverLoopbackBusyNofiy;
    sigslot::signal<std::string&> serverLoopbackCopySettingsDoneNofiy;

    sigslot::signal<std::string&> serverModeTCPNofiy;
    sigslot::signal<std::string&> serverModeUDPNofiy;
    sigslot::signal<std::string&> serverModeSDNofiy;

    sigslot::signal<std::string&> startADCDoneNofiy;
    sigslot::signal<std::string&> startDACDoneNofiy;

    sigslot::signal<std::string&> configFileMissedNotify;


    sigslot::signal<ClientNetConfigManager::Errors,std::string,error_code> errorNofiy;


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
        broadcast_lib::EMode m_mode;
    };

    std::shared_ptr<broadcast_lib::CAsioBroadcastSocket> m_pBroadcast;

    auto receiveCommand(uint32_t command,std::weak_ptr<Clients> sender) -> void;
    auto receiveValueStr(std::string key,std::string value,std::weak_ptr<Clients> sender) -> void;
    auto receiveValueInt(std::string key,uint32_t value,std::weak_ptr<Clients> sender) -> void;
    auto receiveValueDouble(std::string key,double value,std::weak_ptr<Clients> sender) -> void;
    auto serverError(std::error_code error,std::weak_ptr<Clients> sender) -> void;
    auto connectTimeoutError(std::error_code error,std::weak_ptr<Clients> sender) -> void;
    auto sendConfig(std::shared_ptr<Clients> _client, bool _async) -> bool;
    auto sendTestConfig(std::shared_ptr<Clients> _client, bool _async,const CStreamSettings &settings) -> bool;

    std::string m_file_settings;
    std::list<BroadCastClients> m_broadcastClients;
    std::list<std::shared_ptr<Clients>> m_clients;
};

#endif
