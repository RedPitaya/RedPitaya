#include "client_net_config_manager.h"
#include <algorithm>
#include <functional>
#include <mutex>
#include "config_apply.h"
#include "logger_lib/file_logger.h"

// IN seconds
#define BROADCAST_TIMEOUT 10

ClientNetConfigManager::ClientNetConfigManager(std::string default_file_settings_path, bool loadConfig)
    : CStreamSettings(), m_pBroadcast(nullptr), m_file_settings(default_file_settings_path), m_broadcastClients() {
    if (loadConfig)
        readFromFile(default_file_settings_path);
}

ClientNetConfigManager::~ClientNetConfigManager() {
    if (m_pBroadcast) {
        m_pBroadcast->receivedNotify.disconnect_all();
        m_pBroadcast->errorNotify.disconnect_all();
        m_pBroadcast->closeSocket();
        // Joins the broadcast thread before the members its handler uses are gone.
        m_pBroadcast.reset();
    }
    removeHadlers();
    for (auto& cli : clientsCopy()) {
        cli->m_manager->stopAsioNet();
    }
}

auto ClientNetConfigManager::removeHadlers() -> void {
    broadCastNewClientNofiy.disconnect_all();
    serverConnectedNofiy.disconnect_all();
    getNewSettingsNofiy.disconnect_all();
    getNewSettingsItemNofiy.disconnect_all();
    successSendConfigNofiy.disconnect_all();
    failSendConfigNofiy.disconnect_all();
    successSaveConfigNofiy.disconnect_all();
    failSaveConfigNofiy.disconnect_all();

    serverStartedTCPNofiy.disconnect_all();
    serverStartedSDNofiy.disconnect_all();
    serverStoppedNofiy.disconnect_all();
    serverStoppedNoActiveChannelsNofiy.disconnect_all();
    serverStoppedMemErrorNofiy.disconnect_all();
    serverStoppedMemModifyNofiy.disconnect_all();
    serverStoppedSDFullNofiy.disconnect_all();
    serverStoppedSDDoneNofiy.disconnect_all();
    serverStoppedConfigErrorNofiy.disconnect_all();

    serverDacStartedNofiy.disconnect_all();
    serverDacStartedSDNofiy.disconnect_all();
    serverDacStoppedNofiy.disconnect_all();
    serverDacStoppedMemErrorNofiy.disconnect_all();
    serverDacStoppedMemModifyNofiy.disconnect_all();
    serverDacStoppedConfigErrorNofiy.disconnect_all();
    serverDacStoppedSDDoneNofiy.disconnect_all();
    serverDacStoppedSDEmptyNofiy.disconnect_all();
    serverDacStoppedSDBrokenNofiy.disconnect_all();
    serverDacStoppedSDMissingNofiy.disconnect_all();

    serverGpioStartedNofiy.disconnect_all();
    serverGpioStoppedNofiy.disconnect_all();
    serverGpioStoppedMemErrorNofiy.disconnect_all();
    serverGpioStoppedConfigErrorNofiy.disconnect_all();

    softwareTriggerDoneNofiy.disconnect_all();
    softwareTriggerFailNofiy.disconnect_all();
    getTriggerStatusNofiy.disconnect_all();
    failTriggerStatusNofiy.disconnect_all();

    serverModeTCPNofiy.disconnect_all();
    serverModeSDNofiy.disconnect_all();

    startADCDoneNofiy.disconnect_all();
    startDACDoneNofiy.disconnect_all();

    configFileMissedNotify.disconnect_all();

    getMemBlockSizeNofiy.disconnect_all();
    getActiveChannelsNofiy.disconnect_all();

    errorNofiy.disconnect_all();
}

auto ClientNetConfigManager::startBroadcast(std::string host, uint16_t port) -> void {
    m_pBroadcast = broadcast_lib::CAsioBroadcastSocket::create(0, "", host, port);
    m_pBroadcast->initClient();
    m_pBroadcast->errorNotify.connect([=, this](std::error_code er) {
        ERROR_LOG("Broadcast client error: %s (%d)", er.message().c_str(), er.value());
        errorNofiy(Errors::BROADCAST_ERROR, host, er);
    });

    m_pBroadcast->receivedNotify.connect([this, host](std::error_code er, uint8_t* buf, size_t size) {
        bool riseEmit = false;
        std::string h = "";
        if (!er) {
            const std::lock_guard<std::mutex> lock(m_broadcastMtx);
            BroadCastClients cl;
            const std::string s = std::string((char*)buf, size);
            if (!parseBroadcast(s, cl)) {
                TRACE_SHORT("[FATAL ERROR] Broadcast parse error \'%s\'", s.c_str())
                errorNofiy(Errors::BROADCAST_ERROR_PARSE, host, er);
            } else {
                h = cl.host;
                auto find = std::find_if(std::begin(m_broadcastClients), std::end(m_broadcastClients), [&cl](const BroadCastClients& c) { return c.host == cl.host; });
                if (find == std::end(m_broadcastClients)) {
                    m_broadcastClients.push_back(cl);
                    riseEmit = true;
                } else {
                    find->ts = std::time(0);
                }
            }

            m_broadcastClients.remove_if([](const BroadCastClients& c) { return (std::time(0) - c.ts > BROADCAST_TIMEOUT); });
        } else {
            ERROR_LOG("Broadcast client error: %s (%d)", er.message().c_str(), er.value());
            errorNofiy(Errors::BROADCAST_ERROR, host, er);
        }

        if (riseEmit)
            broadCastNewClientNofiy(h);
    });
}

auto ClientNetConfigManager::parseBroadcast(const std::string& message, BroadCastClients& client) -> bool {
    std::vector<std::string> fields;
    size_t begin = 0;
    while (begin <= message.size()) {
        size_t end = message.find('\n', begin);
        if (end == std::string::npos)
            end = message.size();
        fields.push_back(message.substr(begin, end - begin));
        begin = end + 1;
    }
    if (!fields.empty() && fields.back().empty() && !message.empty() && message.back() == '\n')
        fields.pop_back();
    if (fields.size() != 4)
        return false;
    client.mac = fields[0];
    client.host = fields[1];
    if (fields[2].size() > 0 && fields[2][0] == 'M') {
        client.mode = broadcast_lib::AB_SERVER_MASTER;
    } else if (fields[2].size() > 0 && fields[2][0] == 'S') {
        client.mode = broadcast_lib::AB_SERVER_SLAVE;
    } else {
        client.mode = broadcast_lib::AB_NONE;
    }
    client.model = atoi(fields[3].c_str());
    client.ts = std::time(0);
    return true;
}

auto ClientNetConfigManager::getBroadcastClients() -> const std::list<ClientNetConfigManager::BroadCastClients> {
    const std::lock_guard<std::mutex> lock(m_broadcastMtx);
    m_broadcastClients.remove_if([](const BroadCastClients& c) { return (std::time(0) - c.ts > BROADCAST_TIMEOUT); });
    auto copyClients = m_broadcastClients;
    return copyClients;
}

auto ClientNetConfigManager::clientsCopy() -> std::list<std::shared_ptr<Clients>> {
    const std::lock_guard lock(m_clientsMtx);
    return m_clients;
}

auto ClientNetConfigManager::findClient(const std::string& host, bool handshake) -> std::shared_ptr<Clients> {
    const std::lock_guard lock(m_clientsMtx);
    for (const auto& c : m_clients) {
        if (c->m_manager->getHost() == host && (!handshake || c->m_mode != broadcast_lib::AB_NONE))
            return c;
    }
    return nullptr;
}

auto ClientNetConfigManager::getHosts() -> std::list<std::string> {
    std::list<std::string> list;
    for (auto& c : clientsCopy()) {
        if (c->m_manager->isConnected()) {
            list.push_back(c->m_manager->getHost());
        }
    }
    return list;
}

auto ClientNetConfigManager::connectToServers(std::vector<std::string> _hosts, uint16_t port) -> void {
    std::list<std::shared_ptr<Clients>> old;
    {
        const std::lock_guard lock(m_clientsMtx);
        old.swap(m_clients);
    }
    // Stopped outside the lock: their handlers take it.
    old.clear();
    for (std::string& host : _hosts) {
        auto cl = std::make_shared<Clients>();
        cl->m_manager = std::make_shared<CNetConfigManager>();
        cl->m_mode = broadcast_lib::AB_NONE;

        auto cl_weak = std::weak_ptr<Clients>(cl);

        cl->m_manager->receivedCommandNotify.connect(std::bind(&ClientNetConfigManager::receiveCommand, this, std::placeholders::_1, std::placeholders::_2, cl_weak));
        cl->m_manager->receivedStringNotify.connect(std::bind(&ClientNetConfigManager::receiveStrStr, this, std::placeholders::_1, std::placeholders::_2, cl_weak));
        cl->m_manager->receivedConfigNotify.connect(std::bind(&ClientNetConfigManager::receiveConfig, this, std::placeholders::_1, cl_weak));

        cl->m_manager->errorNotify.connect(std::bind(&ClientNetConfigManager::serverError, this, std::placeholders::_1, cl_weak));
        cl->m_manager->connectTimeoutNotify.connect(std::bind(&ClientNetConfigManager::connectTimeoutError, this, std::placeholders::_1, cl_weak));

        {
            const std::lock_guard lock(m_clientsMtx);
            m_clients.push_back(cl);
        }
        cl->m_manager->startAsioNet(net_lib::EMode::M_CLIENT, host, port);
    }
}

auto ClientNetConfigManager::diconnectAll() -> void {
    for (const auto& c : clientsCopy()) {
        c->m_manager->stopAsioNet();
    }
}

auto ClientNetConfigManager::receiveCommand(uint32_t command, std::string tag, std::weak_ptr<Clients> cl) -> void {
    auto sender = cl.lock();
    if (!sender)
        return;

    CNetConfigManager::ECommands c = static_cast<CNetConfigManager::ECommands>(command);
    if (c == CNetConfigManager::ECommands::CS_RESPONSE_MASTER_CONNECTED) {
        {
            const std::lock_guard lock(m_clientsMtx);
            sender->m_mode = broadcast_lib::AB_SERVER_MASTER;
        }
        serverConnectedNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_SLAVE_CONNECTED) {
        {
            const std::lock_guard lock(m_clientsMtx);
            sender->m_mode = broadcast_lib::AB_SERVER_SLAVE;
        }
        serverConnectedNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_SETTINGS_GET_SUCCESS) {
        successSendConfigNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_SETTINGS_GET_FAIL) {
        failSendConfigNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_SAVE_SETTING_TO_FILE_SUCCES) {
        successSaveConfigNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_SAVE_SETTING_TO_FILE_FAIL) {
        failSaveConfigNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STARTED_TCP) {
        serverStartedTCPNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STARTED_SD) {
        serverStartedSDNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STOPPED) {
        serverStoppedNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STOPPED_NO_CHANNELS) {
        serverStoppedNoActiveChannelsNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STOPPED_MEM_ERROR) {
        serverStoppedMemErrorNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STOPPED_MEM_MODIFY) {
        serverStoppedMemModifyNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STOPPED_SD_DONE) {
        serverStoppedSDDoneNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STOPPED_SD_FULL) {
        serverStoppedSDFullNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STOPPED_CONFIG_ERROR) {
        serverStoppedConfigErrorNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_GPIO_SERVER_STARTED) {
        serverGpioStartedNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_GPIO_SERVER_STOPPED) {
        serverGpioStoppedNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_GPIO_SERVER_STOPPED_MEM_ERROR) {
        serverGpioStoppedMemErrorNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_GPIO_SERVER_STOPPED_CONFIG_ERROR) {
        serverGpioStoppedConfigErrorNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_SOFTWARE_TRIGGER_DONE || c == CNetConfigManager::ECommands::CS_RESPONSE_SOFTWARE_TRIGGER_FAIL) {
        CNetConfigManager::ETriggerTarget target;
        if (!CNetConfigManager::parseTriggerTarget(tag, target)) {
            errorNofiy(Errors::SERVER_INTERNAL, sender->m_manager->getHost(), std::error_code());
        } else if (c == CNetConfigManager::ECommands::CS_RESPONSE_SOFTWARE_TRIGGER_DONE) {
            softwareTriggerDoneNofiy(sender->m_manager->getHost(), target);
        } else {
            softwareTriggerFailNofiy(sender->m_manager->getHost(), target);
        }
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_TRIGGER_STATUS) {
        TriggerStatus status;
        if (TriggerStatus::fromJson(tag, status)) {
            getTriggerStatusNofiy(sender->m_manager->getHost(), status);
        } else {
            failTriggerStatusNofiy(sender->m_manager->getHost());
        }
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STARTED) {
        serverDacStartedNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STARTED_SD) {
        serverDacStartedSDNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STOPPED) {
        serverDacStoppedNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STOPPED_MEM_ERROR) {
        serverDacStoppedMemErrorNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STOPPED_MEM_MODIFY) {
        serverDacStoppedMemModifyNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STOPPED_CONFIG_ERROR) {
        serverDacStoppedConfigErrorNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STOPPED_SD_DONE) {
        serverDacStoppedSDDoneNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STOPPED_SD_EMPTY) {
        serverDacStoppedSDEmptyNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STOPPED_SD_BROKEN) {
        serverDacStoppedSDBrokenNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STOPPED_SD_MISSING) {
        serverDacStoppedSDMissingNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_MEM_BLOCK_SIZE) {
        getMemBlockSizeNofiy(sender->m_manager->getHost(), tag);
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_ACTIVE_CHANNELS) {
		adc_channels_t ac = adc_channels_t::fromString(tag);
		getActiveChannelsNofiy(sender->m_manager->getHost(), ac);
	}

	if (c == CNetConfigManager::ECommands::CS_RESPONSE_SERVER_MODE_TCP) {
        serverModeTCPNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_SERVER_MODE_SD) {
        serverModeSDNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_ADC_FPGA_START_ADC_DONE) {
        startADCDoneNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_DAC_FPGA_START_DONE) {
        startDACDoneNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CS_RESPONSE_CONFIG_FILE_MISSED) {
        configFileMissedNotify(sender->m_manager->getHost());
    }
}

auto ClientNetConfigManager::isServersConnected() -> bool {
    for (auto& c : clientsCopy()) {
        bool handshake = false;
        {
            const std::lock_guard lock(m_clientsMtx);
            handshake = c->m_mode != broadcast_lib::AB_NONE;
        }
        if (!handshake && c->m_manager->isConnected())
            return false;
    }
    return true;
}

auto ClientNetConfigManager::receiveStrStr(std::string key, std::string value, std::weak_ptr<Clients> cl) -> void {
    auto sender = cl.lock();
    if (!sender)
        return;
    if (!sender->m_client_settings.setValue(key, value)) {
        errorNofiy(Errors::CANNT_SET_DATA_TO_CONFIG, sender->m_manager->getHost(), std::error_code());
    } else {
        getNewSettingsItemNofiy(sender->m_manager->getHost());
    }
}

auto ClientNetConfigManager::receiveConfig(std::string value, std::weak_ptr<Clients> cl) -> void {
    auto sender = cl.lock();
    if (!sender)
        return;
    if (applyConfigJson(value, sender->m_client_settings)) {
        sender->m_manager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SETTINGS_GET_SUCCESS);
        getNewSettingsNofiy(sender->m_manager->getHost());
    } else {
        sender->m_manager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SETTINGS_GET_FAIL);
        errorNofiy(Errors::CANNT_SET_DATA_TO_CONFIG, sender->m_manager->getHost(), std::error_code());
    }
}

auto ClientNetConfigManager::serverError(std::error_code error, std::weak_ptr<Clients> cl) -> void {
    TRACE_SHORT("Server error: %s (%d)", error.message().c_str(), error.value())
    auto sender = cl.lock();
    if (!sender)
        return;
    errorNofiy(Errors::SERVER_INTERNAL, sender->m_manager->getHost(), error);
}

auto ClientNetConfigManager::connectTimeoutError(std::error_code error, std::weak_ptr<Clients> cl) -> void {
    TRACE_SHORT("Connection timeout error: %s (%d)", error.message().c_str(), error.value())
    auto sender = cl.lock();
    if (!sender)
        return;
    errorNofiy(Errors::CONNECT_TIMEOUT, sender->m_manager->getHost(), error);
}

auto ClientNetConfigManager::sendConfigVariable(const std::string& host, const std::string key, const std::string value) -> bool {
    auto c = findClient(host, true);
    if (!c)
        return false;
    auto ret = c->m_manager->sendData(key, value);
    if (!ret) {
        c->m_manager->stopAsioNet();
    }
    return ret;
}

auto ClientNetConfigManager::sendConfig(const std::string& host) -> bool {
    auto c = findClient(host, true);
    if (!c)
        return false;
    auto ret = c->m_manager->sendConfig(toJson(), true);
    if (!ret) {
        c->m_manager->stopAsioNet();
    }
    return ret;
}

auto ClientNetConfigManager::requestConfig(const std::string& host) -> bool {
    auto c = findClient(host, false);
    return c && c->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_SERVER_SETTINGS);
}

auto ClientNetConfigManager::requestConfigVariable(const std::string& host, const std::string& variable) -> bool {
    auto c = findClient(host, false);
    return c && c->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_SERVER_SETTINGS_VARIABLE, variable);
}

auto ClientNetConfigManager::requestMemoryBlockSize(const std::string& host) -> bool {
    auto c = findClient(host, false);
    return c && c->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_SERVER_MEM_BLOCK_SIZE);
}

auto ClientNetConfigManager::requestActiveChannels(const std::string& host) -> bool {
    auto c = findClient(host, false);
    return c && c->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_SERVER_ACTIVE_CHANNELS);
}

auto ClientNetConfigManager::requestTriggerStatus(const std::string& host) -> bool {
    auto c = findClient(host, false);
    return c && c->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_TRIGGER_STATUS);
}

auto ClientNetConfigManager::getLocalSettingsOfHost(const std::string& host) -> CStreamSettings* {
    auto c = findClient(host, false);
    return c ? &c->m_client_settings : nullptr;
}

auto ClientNetConfigManager::sendSaveToFile(const std::string& host) -> bool {
    auto c = findClient(host, false);
    return c && c->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_SAVE_SETTING_TO_FILE);
}

auto ClientNetConfigManager::sendADCServerStart(const std::string& host) -> bool {
    auto c = findClient(host, false);
    return c && c->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_ADC_SERVER_START);
}

auto ClientNetConfigManager::sendADCServerStop(const std::string& host) -> bool {
    auto c = findClient(host, false);
    return c && c->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_ADC_SERVER_STOP);
}

auto ClientNetConfigManager::sendDACServerStart(const std::string &host, const dac_channels_t &activeChannels) -> bool
{
    auto c = findClient(host, false);
    return c && c->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_DAC_SERVER_START, activeChannels.toString());
}

auto ClientNetConfigManager::sendDACServerStop(const std::string& host) -> bool {
    auto c = findClient(host, false);
    return c && c->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_DAC_SERVER_STOP);
}

auto ClientNetConfigManager::sendADCFPGAStart(const std::string& host) -> bool {
    auto c = findClient(host, false);
    return c && c->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_ADC_FPGA_START);
}

auto ClientNetConfigManager::sendDACFPGAStart(const std::string& host) -> bool {
    auto c = findClient(host, false);
    return c && c->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_DAC_FPGA_START);
}

auto ClientNetConfigManager::sendGPIOServerStart(const std::string& host) -> bool {
    auto c = findClient(host, false);
    return c && c->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_GPIO_SERVER_START);
}

auto ClientNetConfigManager::sendGPIOServerStop(const std::string& host) -> bool {
    auto c = findClient(host, false);
    return c && c->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_GPIO_SERVER_STOP);
}

auto ClientNetConfigManager::sendSoftwareTrigger(const std::string& host, CNetConfigManager::ETriggerTarget target) -> bool {
    auto c = findClient(host, false);
    return c && c->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_SOFTWARE_TRIGGER, std::to_string(target));
}

auto ClientNetConfigManager::sendGetServerMode(const std::string& host) -> bool {
    auto c = findClient(host, false);
    return c && c->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_GET_SERVER_MODE);
}

auto ClientNetConfigManager::getModeByHost(const std::string& host) -> broadcast_lib::EMode {
    auto c = findClient(host, false);
    if (!c)
        return broadcast_lib::AB_NONE;
    const std::lock_guard lock(m_clientsMtx);
    return c->m_mode;
}
