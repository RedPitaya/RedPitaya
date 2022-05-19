#ifndef CONFIG_NET_LIB_NCM_H
#define CONFIG_NET_LIB_NCM_H

#include <list>
#include "net_lib/asio_common.h"
#include "net_lib/asio_net_simple.h"
#include "data_lib/signal.hpp"
#include "data_lib/thread_cout.h"

class CNetConfigManager
{
public:
    enum ECommands{

        BEGIN_SEND_SETTING                  =   0,
        END_SEND_SETTING                    =   1,
        SETTING_GET_SUCCESS                 =   2,
        SETTING_GET_FAIL                    =   3,
        
        BEGIN_SEND_TEST_SETTING             =   4,
        END_SEND_TEST_SETTING               =   5,
        TEST_SETTING_GET_SUCCESS            =   6,
        TEST_SETTING_GET_FAIL               =   7,
        
        REQUEST_SERVER_SETTINGS             =   8,
        REQUEST_SERVER_TEST_SETTINGS        =   9,

        STOP_STREAMING                      =   10,
        START_STREAMING                     =   11,
        START_STREAMING_TEST                =   12,
        SERVER_STOPPED                      =   13,
        SERVER_STOPPED_SD_FULL              =   14,
        SERVER_STOPPED_SD_DONE              =   15,
        SERVER_STARTED_TCP                  =   16,
        SERVER_STARTED_UDP                  =   17,
        SERVER_STARTED_SD                   =   18,

        START_ADC                           =   19,
        START_ADC_DONE                      =   20,

        START_DAC                           =   21,
        START_DAC_DONE                      =   22,

        SAVE_SETTING_TO_FILE                =   23,
        SAVE_TO_FILE_SUCCES                 =   24,
        SAVE_TO_FILE_FAIL                   =   25,
        LOAD_SETTING_FROM_FILE              =   26,
        LOAD_FROM_FILE_SUCCES               =   27,
        LOAD_FROM_FILE_FAIL                 =   28,
        COPY_SETTINGS_TO_TEST_SETTINGS      =   29,
        COPY_SETTINGS_TO_TEST_SETTINGS_DONE =   30,

        MASTER_CONNETED                     =   31,
        SLAVE_CONNECTED                     =   32,
        START_DAC_STREAMING                 =   33,
        START_DAC_STREAMING_TEST            =   34,
        STOP_DAC_STREAMING                  =   35,
        SERVER_DAC_STOPPED                  =   36,
        SERVER_DAC_STOPPED_SD_DONE          =   37,
        SERVER_DAC_STOPPED_SD_EMPTY         =   38,
        SERVER_DAC_STOPPED_SD_BROKEN        =   39,
        SERVER_DAC_STOPPED_SD_MISSING       =   40,
        SERVER_DAC_STARTED                  =   41,
        SERVER_DAC_STARTED_SD               =   42,

        // Loopback commands
        SERVER_LOOPBACK_START               =   43,
        SERVER_LOOPBACK_STOP                =   44,
        SERVER_LOOPBACK_STARTED             =   45,
        SERVER_LOOPBACK_STOPPED             =   46,
        SERVER_LOOPBACK_BUSY                =   47,        // Streaming server in active streaming mode

        SERVER_MODE_TCP                     =   48,
        SERVER_MODE_UDP                     =   49,
        SERVER_MODE_SD                      =   50,
        GET_SERVER_MODE                     =   51,
        GET_SERVER_TEST_MODE                =   52,

        CONFIG_FILE_MISSED                  =   53
    };

    using Ptr = std::shared_ptr<CNetConfigManager>;

    CNetConfigManager();
    ~CNetConfigManager();


    auto startAsioNet(net_lib::EMode _mode, std::string _host,std::string _port) -> bool;
    auto stopAsioNet() -> bool;
    auto isConnected() -> bool;
    auto getHost() -> std::string;
    auto getPort() -> std::string;

    auto sendData(std::string key,std::string value,bool async = true) -> bool;
    auto sendData(std::string key,uint32_t value,bool async = true) -> bool;
    auto sendData(std::string key,double value,bool async = true) -> bool;
    auto sendData(uint32_t command,bool async = true) -> bool;
    auto sendData(ECommands command,bool async = true) -> bool;

    sigslot::signal<string>    connectNotify;
    sigslot::signal<string>    disconnectNotify;

    sigslot::signal<error_code> errorNotify;
    sigslot::signal<error_code> connectTimeoutNotify;
    sigslot::signal<error_code,size_t> sendNotify;

    sigslot::signal<std::string,std::string> receivedStringNotify;
    sigslot::signal<std::string,uint32_t> receivedIntNotify;
    sigslot::signal<std::string,double> receivedDoubleNotify;
    sigslot::signal<uint32_t> receivedCommandNotify;

private:

    CNetConfigManager(const CNetConfigManager &) = delete;
    CNetConfigManager(CNetConfigManager &&) = delete;
    CNetConfigManager& operator=(const CNetConfigManager&) =delete;
    CNetConfigManager& operator=(const CNetConfigManager&&) =delete;

    struct dyn_buffer{
        uint8_t* m_buffers = nullptr;
        uint32_t      m_size = 0;
        uint32_t      m_data_size = 0;
        void push_back(uint8_t *_src,uint32_t _size);
        void resize(uint32_t _size);
        void removeAtStart(uint32_t _size);
    };

    auto start() -> bool;
    auto receiveHandler(std::error_code error,uint8_t*,size_t) -> void;

    std::string                      m_host;
    std::string                      m_port;
    net_lib::CAsioNetSimple*         m_asionet;
    net_lib::EMode                   m_mode;
    dyn_buffer                       m_buffers;
};

#endif
