#include "server_net_config_manager.h"
#include "logger_lib/file_logger.h"

ServerNetConfigManager::ServerNetConfigManager(std::string defualt_file_settings_path, broadcast_lib::EMode mode, std::string host, uint16_t port)
    : m_pBroadcast(nullptr), m_pNetConfManager(nullptr), m_file_settings(defualt_file_settings_path), m_mode(mode) {
    m_pNetConfManager = std::make_shared<CNetConfigManager>();
    m_pNetConfManager->receivedCommandNotify.connect(&ServerNetConfigManager::receiveCommand, this);
    m_pNetConfManager->receivedStringNotify.connect(&ServerNetConfigManager::receiveValueStr, this);
    m_pNetConfManager->receivedConfigNotify.connect(&ServerNetConfigManager::receiveConfig, this);
    m_pNetConfManager->connectNotify.connect(&ServerNetConfigManager::connected, this);
    m_pNetConfManager->disconnectNotify.connect(&ServerNetConfigManager::disconnected, this);
    m_pNetConfManager->errorNotify.connect(&ServerNetConfigManager::serverError, this);

    startServer(host, port);
    if (!m_settings.readFromFile(defualt_file_settings_path)) {
        m_settings.resetDefault();
        m_settings.writeToFile(defualt_file_settings_path);
    }
}

ServerNetConfigManager::~ServerNetConfigManager() {
    stop();
}

auto ServerNetConfigManager::stop() -> void {
    if (m_pBroadcast)
        m_pBroadcast->closeSocket();
    m_pNetConfManager->stopAsioNet();
}

auto ServerNetConfigManager::startServer(std::string host, uint16_t port) -> void {
    m_pNetConfManager->startAsioNet(net_lib::M_SERVER, host, port);
}

auto ServerNetConfigManager::startBroadcast(broadcast_lib::EModel model, std::string host, uint16_t port) -> void {
    m_pBroadcast = broadcast_lib::CAsioBroadcastSocket::create(model, host, port);
    m_pBroadcast->initServer(m_mode);
    m_pBroadcast->errorNotify.connect([=, this](std::error_code er) {
        ERROR_LOG("Broadcast server error: %s (%d)", er.message().c_str(), er.value());
        errorNofiy(Errors::BROADCAST_ERROR);
    });
}

auto ServerNetConfigManager::setBroadcastAddress(std::string host) -> void {
    m_pBroadcast->setIPAddress(host);
}

auto ServerNetConfigManager::isConnected() -> bool {
    return m_pNetConfManager->isConnected();
}

auto ServerNetConfigManager::connected(std::string) -> void {
    clientConnectedNofiy();
    if (m_mode == broadcast_lib::AB_SERVER_MASTER)
        m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_MASTER_CONNECTED);
    if (m_mode == broadcast_lib::AB_SERVER_SLAVE)
        m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SLAVE_CONNECTED);
}

auto ServerNetConfigManager::disconnected(std::string) -> void {
    clientDisconnectedNofiy();
}

auto ServerNetConfigManager::receiveCommand(uint32_t command, std::string tag) -> void {
    auto c = static_cast<CNetConfigManager::ECommands>(command);

    if (c == CNetConfigManager::ECommands::CS_REQUEST_ADC_FPGA_START) {
        startADCNofiy();
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_DAC_FPGA_START) {
        startDACNofiy();
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_ADC_SERVER_START) {
        startStreamingNofiy();
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_ADC_SERVER_STOP) {
        stopStreamingNofiy();
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_DAC_SERVER_START) {
        startDacStreamingNofiy(tag);
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_DAC_SERVER_STOP) {
        stopDacStreamingNofiy();
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_GET_SERVER_MODE) {
        if (m_settings.getADCPassMode().value == CStreamSettings::PassMode::FILE) {
            sendADCServerModeSD();
        } else if (m_settings.getADCPassMode().value == CStreamSettings::PassMode::NET) {
            sendADCServerModeTCP();
        }
        getServerModeNofiy();
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_SERVER_MEM_BLOCK_SIZE) {
        m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_MEM_BLOCK_SIZE, std::to_string(m_settings.getMemoryBlockSize()));
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_SERVER_ACTIVE_CHANNELS) {
        uint8_t ac = 0;
        for (int i = 1; i <= 4; i++) {
            if (m_settings.getADCChannels(i).value == CStreamSettings::State::ON) {
                ac++;
            }
        }
        m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_ACTIVE_CHANNELS, std::to_string(ac));
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_LOAD_SETTING_FROM_FILE) {
        if (m_settings.readFromFile(m_file_settings)) {
            getNewSettingsNofiy();
            memoryBlockSizeChangeNofiy();
            m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_LOAD_SETTING_FROM_FILE_SUCCES);
        } else {
            m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_LOAD_SETTING_FROM_FILE_FAIL);
        }
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_SAVE_SETTING_TO_FILE) {
        if (m_settings.writeToFile(m_file_settings)) {
            m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SAVE_SETTING_TO_FILE_SUCCES);
        } else {
            m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SAVE_SETTING_TO_FILE_FAIL);
        }
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_SERVER_SETTINGS) {
        m_pNetConfManager->sendConfig(m_settings.toJson(), false);
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_SERVER_SETTINGS_VARIABLE) {
        m_pNetConfManager->sendData(tag, m_settings.getValue(tag));
    }
}

auto ServerNetConfigManager::receiveValueStr(std::string key, std::string value) -> void {
    if (setSettingsParameter(key, value)) {
        m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SETTINGS_GET_SUCCESS);
        getNewSettingsNofiy();
    } else {
        m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SETTINGS_GET_FAIL);
    }
}

auto ServerNetConfigManager::setSettingsParameter(const std::string& key, const std::string& value) -> bool {
    bool state = m_settings.setValue(key, value);
    if (state && key == "block_size") {
        memoryBlockSizeChangeNofiy();
    }
    return state;
}

auto ServerNetConfigManager::receiveConfig(std::string value) -> void {
    CStreamSettings settings;
    if (settings.parseJson(value)) {
        m_settings = settings;
        m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SETTINGS_GET_SUCCESS);
        getNewSettingsNofiy();
        memoryBlockSizeChangeNofiy();
    } else {
        m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SETTINGS_GET_FAIL);
    }
}

auto ServerNetConfigManager::serverError(std::error_code) -> void {
    errorNofiy(Errors::SERVER_INTERNAL);
}

auto ServerNetConfigManager::sendConfigFileMissed() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_CONFIG_FILE_MISSED);
}

auto ServerNetConfigManager::sendADCStarted() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_ADC_FPGA_START_ADC_DONE);
}

auto ServerNetConfigManager::sendDACStarted() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_ADC_FPGA_START_ADC_DONE);
}

auto ServerNetConfigManager::sendADCServerStartedTCP() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STARTED_TCP);
}

auto ServerNetConfigManager::sendADCServerStartedSD() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STARTED_SD);
}

auto ServerNetConfigManager::sendADCServerModeTCP() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SERVER_MODE_TCP);
}

auto ServerNetConfigManager::sendADCServerModeSD() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SERVER_MODE_SD);
}

auto ServerNetConfigManager::sendDACServerStarted() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STARTED);
}

auto ServerNetConfigManager::sendDACServerStartedSD() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STARTED_SD);
}

auto ServerNetConfigManager::sendServerStopped() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STOPPED);
}

auto ServerNetConfigManager::sendServerNoChannelsStopped() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STOPPED_NO_CHANNELS);
}

auto ServerNetConfigManager::sendServerMemoryErrorStopped() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STOPPED_MEM_ERROR);
}

auto ServerNetConfigManager::sendServerMemoryModifyStopped() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STOPPED_MEM_MODIFY);
}

auto ServerNetConfigManager::sendServerStoppedSDFull() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STOPPED_SD_FULL);
}

auto ServerNetConfigManager::sendServerStoppedDone() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STOPPED_SD_DONE);
}

auto ServerNetConfigManager::sendDACServerStopped() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STOPPED);
}

auto ServerNetConfigManager::sendDACServerMemoryErrorStopped() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STOPPED_MEM_ERROR);
}

auto ServerNetConfigManager::sendDACServerMemoryModifyStopped() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STOPPED_MEM_MODIFY);
}

auto ServerNetConfigManager::sendDACServerConfigErrorStopped() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STOPPED_CONFIG_ERROR);
}

auto ServerNetConfigManager::sendDACServerStoppedSDDone() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STOPPED_SD_DONE);
}

auto ServerNetConfigManager::sendDACServerStoppedSDEmpty() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STOPPED_SD_EMPTY);
}

auto ServerNetConfigManager::sendDACServerStoppedSDBroken() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STOPPED_SD_BROKEN);
}

auto ServerNetConfigManager::sendDACServerStoppedSDMissingFile() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_DAC_SERVER_STOPPED_SD_MISSING);
}

auto ServerNetConfigManager::getSettings() -> const CStreamSettings {
    return m_settings;
}

auto ServerNetConfigManager::getSettingsRef() -> CStreamSettings& {
    return m_settings;
}

auto ServerNetConfigManager::setMode(broadcast_lib::EMode mode) -> void {
    m_mode = mode;
}
