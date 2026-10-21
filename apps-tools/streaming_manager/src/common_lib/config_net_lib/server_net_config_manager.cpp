#include "server_net_config_manager.h"
#include "config_apply.h"
#include "logger_lib/file_logger.h"

ServerNetConfigManager::ServerNetConfigManager(std::string defualt_file_settings_path, broadcast_lib::EMode mode, std::string host, uint16_t port, uint8_t maxAdcChannels)
    : m_pBroadcast(nullptr), m_pNetConfManager(nullptr), m_file_settings(defualt_file_settings_path), m_mode(mode), m_maxADCChannels(maxAdcChannels) {
    m_pNetConfManager = std::make_shared<CNetConfigManager>();
    m_pNetConfManager->receivedCommandNotify.connect(&ServerNetConfigManager::receiveCommand, this);
    m_pNetConfManager->receivedStringNotify.connect(&ServerNetConfigManager::receiveValueStr, this);
    m_pNetConfManager->receivedConfigNotify.connect(&ServerNetConfigManager::receiveConfig, this);
    m_pNetConfManager->connectNotify.connect(&ServerNetConfigManager::connected, this);
    m_pNetConfManager->disconnectNotify.connect(&ServerNetConfigManager::disconnected, this);
    m_pNetConfManager->errorNotify.connect(&ServerNetConfigManager::serverError, this);

    // The settings are in place before a client can ask for them.
    if (!m_settings.readFromFile(defualt_file_settings_path)) {
        m_settings.resetDefault();
        m_settings.writeToFile(defualt_file_settings_path);
    }
    startServer(host, port);
}

ServerNetConfigManager::~ServerNetConfigManager() {
    stop();
}

auto ServerNetConfigManager::stop() -> void {
    if (m_pBroadcast) {
        m_pBroadcast->errorNotify.disconnect_all();
        m_pBroadcast->closeSocket();
        // Joins the broadcast thread, so its handler is not running into this object later.
        m_pBroadcast.reset();
    }
    m_pNetConfManager->stopAsioNet();
}

auto ServerNetConfigManager::startServer(std::string host, uint16_t port) -> void {
    m_pNetConfManager->startAsioNet(net_lib::M_SERVER, host, port);
}

auto ServerNetConfigManager::startBroadcast(uint8_t model, std::string mac, std::string host, uint16_t port) -> void {
    m_pBroadcast = broadcast_lib::CAsioBroadcastSocket::create(model, mac, host, port);
    m_pBroadcast->initServer(m_mode);
    m_pBroadcast->errorNotify.connect([=, this](std::error_code er) {
        ERROR_LOG("Broadcast server error: %s (%d)", er.message().c_str(), er.value());
        errorNofiy(Errors::BROADCAST_ERROR);
    });
}

auto ServerNetConfigManager::setBroadcastAddress(std::string host) -> void {
    if (m_pBroadcast)
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
		dac_channels_t ac = dac_channels_t::fromString(tag);
		startDacStreamingNofiy(ac);
	}

	if (c == CNetConfigManager::ECommands::CS_REQUEST_DAC_SERVER_STOP) {
        stopDacStreamingNofiy();
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_GPIO_SERVER_START) {
        if (startGpioStreamingNofiy.slot_count() == 0) {
            sendGPIOServerConfigErrorStopped();
        } else {
            startGpioStreamingNofiy();
        }
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_GPIO_SERVER_STOP) {
        if (stopGpioStreamingNofiy.slot_count() == 0) {
            sendGPIOServerStopped();
        } else {
            stopGpioStreamingNofiy();
        }
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_SOFTWARE_TRIGGER) {
        CNetConfigManager::ETriggerTarget target;
        if (!CNetConfigManager::parseTriggerTarget(tag, target)) {
            m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SOFTWARE_TRIGGER_FAIL, tag);
        } else if (softwareTriggerNofiy.slot_count() == 0) {
            sendSoftwareTriggerFail(target);
        } else {
            softwareTriggerNofiy(target);
        }
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_TRIGGER_STATUS) {
        if (triggerStatusRequestNofiy.slot_count() == 0) {
            m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_TRIGGER_STATUS, "");
        } else {
            triggerStatusRequestNofiy();
        }
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_GET_SERVER_MODE) {
        const auto mode = getSettings().getADCPassMode().value;
        if (mode == CStreamSettings::PassMode::FILE) {
            sendADCServerModeSD();
        } else if (mode == CStreamSettings::PassMode::NET) {
            sendADCServerModeTCP();
        }
        getServerModeNofiy();
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_SERVER_MEM_BLOCK_SIZE) {
        m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_MEM_BLOCK_SIZE, std::to_string(getSettings().getMemoryBlockSize()));
    }

    if (c == CNetConfigManager::ECommands::CS_REQUEST_SERVER_ACTIVE_CHANNELS) {
        const auto settings = getSettings();
		adc_channels_t ac;
		for (int i = 1; i <= 4 && i <= m_maxADCChannels; i++) {
			if (settings.getADCChannels(i).value == CStreamSettings::State::ON) {
				ac.enable((ADCChannels) (i - 1));
			}
		}
		m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_ACTIVE_CHANNELS, ac.toString());
	}

	if (c == CNetConfigManager::ECommands::CS_REQUEST_LOAD_SETTING_FROM_FILE) {
        bool loaded = false;
        {
            std::lock_guard lock(m_settingsMtx);
            loaded = m_settings.readFromFile(m_file_settings);
        }
		if (loaded) {
			getNewSettingsNofiy();
			memoryBlockSizeChangeNofiy();
            m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_LOAD_SETTING_FROM_FILE_SUCCES);
		} else {
			m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_LOAD_SETTING_FROM_FILE_FAIL);
		}
	}

	if (c == CNetConfigManager::ECommands::CS_REQUEST_SAVE_SETTING_TO_FILE) {
		auto settings = getSettings();
		if (settings.writeToFile(m_file_settings)) {
			m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SAVE_SETTING_TO_FILE_SUCCES);
		} else {
			m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SAVE_SETTING_TO_FILE_FAIL);
		}
	}

	if (c == CNetConfigManager::ECommands::CS_REQUEST_SERVER_SETTINGS) {
		m_pNetConfManager->sendConfig(getSettings().toJson(), false);
	}

	if (c == CNetConfigManager::ECommands::CS_REQUEST_SERVER_SETTINGS_VARIABLE) {
        std::string value;
        {
            std::lock_guard lock(m_settingsMtx);
            value = m_settings.getValue(tag);
        }
        m_pNetConfManager->sendData(tag, value);
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
    bool state = false;
    {
        std::lock_guard lock(m_settingsMtx);
        state = m_settings.setValue(key, value);
    }
    if (state && key == "block_size") {
        memoryBlockSizeChangeNofiy();
    }
    return state;
}

auto ServerNetConfigManager::receiveConfig(std::string value) -> void {
    // Applied over the current settings: a client that does not know some keys leaves them as
    // they are. A value that is not taken fails the whole document.
    bool applied = false;
    {
        std::lock_guard lock(m_settingsMtx);
        applied = applyConfigJson(value, m_settings);
    }
    if (applied) {
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
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_DAC_FPGA_START_DONE);
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

auto ServerNetConfigManager::sendServerConfigErrorStopped() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_ADC_SERVER_STOPPED_CONFIG_ERROR);
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

auto ServerNetConfigManager::sendGPIOServerStarted() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_GPIO_SERVER_STARTED);
}

auto ServerNetConfigManager::sendGPIOServerStopped() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_GPIO_SERVER_STOPPED);
}

auto ServerNetConfigManager::sendGPIOServerMemoryErrorStopped() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_GPIO_SERVER_STOPPED_MEM_ERROR);
}

auto ServerNetConfigManager::sendGPIOServerConfigErrorStopped() -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_GPIO_SERVER_STOPPED_CONFIG_ERROR);
}

auto ServerNetConfigManager::sendSoftwareTriggerDone(CNetConfigManager::ETriggerTarget target) -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SOFTWARE_TRIGGER_DONE, std::to_string(target));
}

auto ServerNetConfigManager::sendSoftwareTriggerFail(CNetConfigManager::ETriggerTarget target) -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SOFTWARE_TRIGGER_FAIL, std::to_string(target));
}

auto ServerNetConfigManager::sendTriggerStatus(const TriggerStatus& status) -> bool {
    return m_pNetConfManager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_TRIGGER_STATUS, status.toJson());
}

auto ServerNetConfigManager::getSettings() -> const CStreamSettings {
    std::lock_guard lock(m_settingsMtx);
    return m_settings;
}

auto ServerNetConfigManager::getSettingsRef() -> CStreamSettings& {
    return m_settings;
}

auto ServerNetConfigManager::setMode(broadcast_lib::EMode mode) -> void {
    m_mode = mode;
}
