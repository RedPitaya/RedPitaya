#ifndef CONFIG_NET_LIB_NCM_H
#define CONFIG_NET_LIB_NCM_H

#include "data_lib/signal.hpp"
#include "net_lib/asio_common.h"
#include "net_lib/asio_net_simple.h"

class CNetConfigManager {
   public:
    enum ECommands {
        CS_RESPONSE_SETTINGS_GET_SUCCESS,
        CS_RESPONSE_SETTINGS_GET_FAIL,

        CS_REQUEST_SERVER_SETTINGS,
        CS_REQUEST_SERVER_SETTINGS_VARIABLE,
        CS_RESPONSE_CONFIG_FILE_MISSED,

        CS_REQUEST_SERVER_MEM_BLOCK_SIZE,
        CS_RESPONSE_MEM_BLOCK_SIZE,
        CS_REQUEST_SERVER_ACTIVE_CHANNELS,
        CS_RESPONSE_ACTIVE_CHANNELS,

        CS_REQUEST_SAVE_SETTING_TO_FILE,
        CS_RESPONSE_SAVE_SETTING_TO_FILE_SUCCES,
        CS_RESPONSE_SAVE_SETTING_TO_FILE_FAIL,

        CS_REQUEST_LOAD_SETTING_FROM_FILE,
        CS_RESPONSE_LOAD_SETTING_FROM_FILE_SUCCES,
        CS_RESPONSE_LOAD_SETTING_FROM_FILE_FAIL,

        CS_RESPONSE_MASTER_CONNECTED,
        CS_RESPONSE_SLAVE_CONNECTED,

        CS_REQUEST_ADC_SERVER_START,
        CS_REQUEST_ADC_SERVER_STOP,

        CS_RESPONSE_ADC_SERVER_STOPPED,
        CS_RESPONSE_ADC_SERVER_STOPPED_NO_CHANNELS,
        CS_RESPONSE_ADC_SERVER_STOPPED_MEM_ERROR,
        CS_RESPONSE_ADC_SERVER_STOPPED_MEM_MODIFY,
        CS_RESPONSE_ADC_SERVER_STOPPED_SD_FULL,
        CS_RESPONSE_ADC_SERVER_STOPPED_SD_DONE,
        CS_RESPONSE_ADC_SERVER_STARTED_TCP,
        CS_RESPONSE_ADC_SERVER_STARTED_SD,

        CS_REQUEST_ADC_FPGA_START,
        CS_RESPONSE_ADC_FPGA_START_ADC_DONE,

        CS_REQUEST_DAC_FPGA_START,
        CS_RESPONSE_DAC_FPGA_START_DONE,

        CS_REQUEST_DAC_SERVER_START,
        CS_REQUEST_DAC_SERVER_STOP,

        CS_RESPONSE_DAC_SERVER_STOPPED,
        CS_RESPONSE_DAC_SERVER_STOPPED_MEM_ERROR,
        CS_RESPONSE_DAC_SERVER_STOPPED_MEM_MODIFY,
        CS_RESPONSE_DAC_SERVER_STOPPED_CONFIG_ERROR,
        CS_RESPONSE_DAC_SERVER_STOPPED_SD_DONE,
        CS_RESPONSE_DAC_SERVER_STOPPED_SD_EMPTY,
        CS_RESPONSE_DAC_SERVER_STOPPED_SD_BROKEN,
        CS_RESPONSE_DAC_SERVER_STOPPED_SD_MISSING,
        CS_RESPONSE_DAC_SERVER_STARTED,
        CS_RESPONSE_DAC_SERVER_STARTED_SD,

        CS_REQUEST_GET_SERVER_MODE,
        CS_RESPONSE_SERVER_MODE_TCP,
        CS_RESPONSE_SERVER_MODE_SD
    };

    using Ptr = std::shared_ptr<CNetConfigManager>;

    CNetConfigManager();
    ~CNetConfigManager();

    auto startAsioNet(net_lib::EMode _mode, std::string _host, uint16_t _port) -> bool;
    auto stopAsioNet() -> bool;
    auto isConnected() -> bool;
    auto getHost() -> std::string;
    auto getPort() -> uint16_t;

    auto sendData(std::string key, std::string value, bool async = true) -> bool;
    auto sendConfig(std::string value, bool async = true) -> bool;
    auto sendCommand(ECommands command, const std::string& tag = "", bool async = true) -> bool;

    sigslot::signal<string> connectNotify;
    sigslot::signal<string> disconnectNotify;

    sigslot::signal<error_code> errorNotify;
    sigslot::signal<error_code> connectTimeoutNotify;
    sigslot::signal<error_code, size_t> sendNotify;

    sigslot::signal<std::string, std::string> receivedStringNotify;
    sigslot::signal<std::string> receivedConfigNotify;
    sigslot::signal<uint32_t, std::string> receivedCommandNotify;

   private:
    CNetConfigManager(const CNetConfigManager&) = delete;
    CNetConfigManager(CNetConfigManager&&) = delete;
    CNetConfigManager& operator=(const CNetConfigManager&) = delete;
    CNetConfigManager& operator=(const CNetConfigManager&&) = delete;

    struct dyn_buffer {
        uint8_t* m_buffers = nullptr;
        uint32_t m_size = 0;
        uint32_t m_data_size = 0;
        void push_back(uint8_t* _src, uint32_t _size);
        void resize(uint32_t _size);
        void removeAtStart(uint32_t _size);
    };

    auto start() -> bool;
    auto receiveHandler(std::error_code error, uint8_t*, size_t) -> void;

    std::string m_host;
    uint16_t m_port;
    net_lib::CAsioNetSimple* m_asionet;
    net_lib::EMode m_mode;
    dyn_buffer m_buffers;
};

#endif
