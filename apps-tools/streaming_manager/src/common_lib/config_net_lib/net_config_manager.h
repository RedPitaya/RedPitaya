#ifndef CONFIG_NET_LIB_NCM_H
#define CONFIG_NET_LIB_NCM_H

#include <mutex>
#include "data_lib/signal.hpp"
#include "net_lib/asio_common.h"
#include "net_lib/asio_net_simple.h"

class CNetConfigManager : public std::enable_shared_from_this<CNetConfigManager> {
   public:
    // The numbers are the wire protocol shared with older clients and servers: never change or
    // reuse a number, add new commands at the end with the next free one. A side that does not
    // know a command ignores it.
    enum ECommands : uint32_t {
        CS_RESPONSE_SETTINGS_GET_SUCCESS = 0,
        CS_RESPONSE_SETTINGS_GET_FAIL = 1,

        CS_REQUEST_SERVER_SETTINGS = 2,
        CS_REQUEST_SERVER_SETTINGS_VARIABLE = 3,
        CS_RESPONSE_CONFIG_FILE_MISSED = 4,

        CS_REQUEST_SERVER_MEM_BLOCK_SIZE = 5,
        CS_RESPONSE_MEM_BLOCK_SIZE = 6,
        CS_REQUEST_SERVER_ACTIVE_CHANNELS = 7,
        CS_RESPONSE_ACTIVE_CHANNELS = 8,

        CS_REQUEST_SAVE_SETTING_TO_FILE = 9,
        CS_RESPONSE_SAVE_SETTING_TO_FILE_SUCCES = 10,
        CS_RESPONSE_SAVE_SETTING_TO_FILE_FAIL = 11,

        CS_REQUEST_LOAD_SETTING_FROM_FILE = 12,
        CS_RESPONSE_LOAD_SETTING_FROM_FILE_SUCCES = 13,
        CS_RESPONSE_LOAD_SETTING_FROM_FILE_FAIL = 14,

        CS_RESPONSE_MASTER_CONNECTED = 15,
        CS_RESPONSE_SLAVE_CONNECTED = 16,

        CS_REQUEST_ADC_SERVER_START = 17,
        CS_REQUEST_ADC_SERVER_STOP = 18,

        CS_RESPONSE_ADC_SERVER_STOPPED = 19,
        CS_RESPONSE_ADC_SERVER_STOPPED_NO_CHANNELS = 20,
        CS_RESPONSE_ADC_SERVER_STOPPED_MEM_ERROR = 21,
        CS_RESPONSE_ADC_SERVER_STOPPED_MEM_MODIFY = 22,
        CS_RESPONSE_ADC_SERVER_STOPPED_SD_FULL = 23,
        CS_RESPONSE_ADC_SERVER_STOPPED_SD_DONE = 24,
        CS_RESPONSE_ADC_SERVER_STARTED_TCP = 25,
        CS_RESPONSE_ADC_SERVER_STARTED_SD = 26,

        CS_REQUEST_ADC_FPGA_START = 27,
        CS_RESPONSE_ADC_FPGA_START_ADC_DONE = 28,

        CS_REQUEST_DAC_FPGA_START = 29,
        CS_RESPONSE_DAC_FPGA_START_DONE = 30,

        CS_REQUEST_DAC_SERVER_START = 31,
        CS_REQUEST_DAC_SERVER_STOP = 32,

        CS_RESPONSE_DAC_SERVER_STOPPED = 33,
        CS_RESPONSE_DAC_SERVER_STOPPED_MEM_ERROR = 34,
        CS_RESPONSE_DAC_SERVER_STOPPED_MEM_MODIFY = 35,
        CS_RESPONSE_DAC_SERVER_STOPPED_CONFIG_ERROR = 36,
        CS_RESPONSE_DAC_SERVER_STOPPED_SD_DONE = 37,
        CS_RESPONSE_DAC_SERVER_STOPPED_SD_EMPTY = 38,
        CS_RESPONSE_DAC_SERVER_STOPPED_SD_BROKEN = 39,
        CS_RESPONSE_DAC_SERVER_STOPPED_SD_MISSING = 40,
        CS_RESPONSE_DAC_SERVER_STARTED = 41,
        CS_RESPONSE_DAC_SERVER_STARTED_SD = 42,

        CS_REQUEST_GET_SERVER_MODE = 43,
        CS_RESPONSE_SERVER_MODE_TCP = 44,
        CS_RESPONSE_SERVER_MODE_SD = 45,

        CS_RESPONSE_ADC_SERVER_STOPPED_CONFIG_ERROR = 46,

        // tag: ETriggerTarget as a decimal number
        CS_REQUEST_SOFTWARE_TRIGGER = 47,
        CS_RESPONSE_SOFTWARE_TRIGGER_DONE = 48,
        CS_RESPONSE_SOFTWARE_TRIGGER_FAIL = 49,

        // response tag: TriggerStatus::toJson(), empty if the server has no status to give
        CS_REQUEST_TRIGGER_STATUS = 50,
        CS_RESPONSE_TRIGGER_STATUS = 51,

        CS_REQUEST_GPIO_SERVER_START = 52,
        CS_REQUEST_GPIO_SERVER_STOP = 53,
        CS_RESPONSE_GPIO_SERVER_STARTED = 54,
        CS_RESPONSE_GPIO_SERVER_STOPPED = 55,
        CS_RESPONSE_GPIO_SERVER_STOPPED_MEM_ERROR = 56,
        CS_RESPONSE_GPIO_SERVER_STOPPED_CONFIG_ERROR = 57
    };

    // Block that a software trigger is sent to; wire values like ECommands.
    enum ETriggerTarget : uint32_t { TT_ADC = 0, TT_DAC = 1, TT_GPIO = 2 };

    // The tag of the trigger commands: the decimal number of a known target, nothing else.
    static auto parseTriggerTarget(const std::string& tag, ETriggerTarget& target) -> bool;

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

    sigslot::signal<std::string> connectNotify;
    sigslot::signal<std::string> disconnectNotify;

    sigslot::signal<std::error_code> errorNotify;
    sigslot::signal<std::error_code> connectTimeoutNotify;
    sigslot::signal<std::error_code, size_t> sendNotify;

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
        void clear();
    };

    auto start() -> bool;
    auto net() -> std::shared_ptr<net_lib::CAsioNetSimple>;
    // Throws on a packet this protocol does not produce: a peer of another version must not be served.
    auto receiveHandler(std::error_code error, uint8_t*, size_t) -> void;

    std::string m_host;
    uint16_t m_port;
    std::shared_ptr<net_lib::CAsioNetSimple> m_asionet;
    std::mutex m_netMtx;
    net_lib::EMode m_mode;
    dyn_buffer m_buffers;
};

static_assert(CNetConfigManager::CS_RESPONSE_SETTINGS_GET_SUCCESS == 0);
static_assert(CNetConfigManager::CS_REQUEST_ADC_SERVER_START == 17);
static_assert(CNetConfigManager::CS_REQUEST_DAC_SERVER_START == 31);
static_assert(CNetConfigManager::CS_RESPONSE_SERVER_MODE_SD == 45);
static_assert(CNetConfigManager::CS_RESPONSE_GPIO_SERVER_STOPPED_CONFIG_ERROR == 57);

#endif
