#include "client_net_config_manager.h"
#include "logger_lib/file_logger.h"
#include <algorithm>
#include <functional>
#include <mutex>

// IN seconds
#define BROADCAST_TIMEOUT 10

std::mutex g_broadcast_mutex;
std::mutex g_client_mutex;

ClientNetConfigManager::ClientNetConfigManager(std::string default_file_settings_path, bool loadConfig)
	: CStreamSettings()
	, m_pBroadcast(nullptr)
	, m_file_settings(default_file_settings_path)
	, m_broadcastClients()
{
	if (loadConfig)
		readFromFile(default_file_settings_path);
}

ClientNetConfigManager::~ClientNetConfigManager()
{
	removeHadlers();
	for (auto &cli : m_clients) {
		cli->m_manager->stopAsioNet();
	}
}

auto ClientNetConfigManager::removeHadlers() -> void
{
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
	serverStoppedMemErrorNofiy.disconnect_all();
	serverStoppedMemModifyNofiy.disconnect_all();
	serverStoppedSDFullNofiy.disconnect_all();
	serverStoppedSDDoneNofiy.disconnect_all();

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

	serverModeTCPNofiy.disconnect_all();
	serverModeSDNofiy.disconnect_all();

	startADCDoneNofiy.disconnect_all();
	startDACDoneNofiy.disconnect_all();

	configFileMissedNotify.disconnect_all();

	getMemBlockSizeNofiy.disconnect_all();
	getActiveChannelsNofiy.disconnect_all();

	errorNofiy.disconnect_all();
}

auto ClientNetConfigManager::startBroadcast(std::string host, uint16_t port) -> void
{
	m_pBroadcast = broadcast_lib::CAsioBroadcastSocket::create(broadcast_lib::CLIENT, host, port);
	m_pBroadcast->initClient();
	m_pBroadcast->errorNotify.connect([=, this](std::error_code er) {
		ERROR_LOG("Broadcast client error: %s (%d)", er.message().c_str(), er.value());
		errorNofiy(Errors::BROADCAST_ERROR, host, er);
	});

	m_pBroadcast->receivedNotify.connect([this, host](std::error_code er, uint8_t *buf, size_t size) {
		bool riseEmit = false;
		std::string h = "";
		if (!er) {
			const std::lock_guard<std::mutex> lock(g_broadcast_mutex);
			BroadCastClients cl;
			cl.mode = broadcast_lib::AB_NONE;
			cl.ts = std::time(0);
			std::string s = std::string((char *) buf, size);
			uint8_t model = s[s.size() - 1] - '0';
			cl.model = static_cast<broadcast_lib::EModel>(model);
			s.pop_back();
			if (s[s.size() - 1] == 'M') {
				cl.mode = broadcast_lib::AB_SERVER_MASTER;
			} else if (s[s.size() - 1] == 'S') {
				cl.mode = broadcast_lib::AB_SERVER_SLAVE;
			} else {
				errorNofiy(Errors::BROADCAST_ERROR_PARSE, host, er);
			}
			s.pop_back();
			cl.host = h = s;
			auto find = std::find_if(std::begin(m_broadcastClients), std::end(m_broadcastClients), [&cl](const BroadCastClients &c) {
				return c.host == cl.host;
			});
			if (find == std::end(m_broadcastClients)) {
				m_broadcastClients.push_back(cl);
				riseEmit = true;
			} else {
				find->ts = std::time(0);
			}
			m_broadcastClients.remove_if([](const BroadCastClients &c) { return (std::time(0) - c.ts > BROADCAST_TIMEOUT); });
		} else {
			ERROR_LOG("Broadcast client error: %s (%d)", er.message().c_str(), er.value());
			errorNofiy(Errors::BROADCAST_ERROR, host, er);
		}

		if (riseEmit)
			broadCastNewClientNofiy(h);
	});
}

auto ClientNetConfigManager::getBroadcastClients() -> const std::list<ClientNetConfigManager::BroadCastClients>
{
	const std::lock_guard<std::mutex> lock(g_broadcast_mutex);
	m_broadcastClients.remove_if([](const BroadCastClients &c) { return (std::time(0) - c.ts > BROADCAST_TIMEOUT); });
	auto copyClients = m_broadcastClients;
	return copyClients;
}

auto ClientNetConfigManager::getHosts() -> std::list<std::string>
{
	std::list<std::string> list;
	for (auto &c : m_clients) {
		if (c->m_manager->isConnected()) {
			list.push_back(c->m_manager->getHost());
		}
	}
	return list;
}

auto ClientNetConfigManager::connectToServers(std::vector<std::string> _hosts, uint16_t port) -> void
{
	m_clients.clear();
	for (std::string &host : _hosts) {
		auto cl = std::make_shared<Clients>();
		cl->m_manager = std::make_shared<CNetConfigManager>();
		cl->m_mode = broadcast_lib::AB_NONE;

		auto cl_weak = std::weak_ptr<Clients>(cl);

		cl->m_manager->receivedCommandNotify.connect(
			std::bind(&ClientNetConfigManager::receiveCommand, this, std::placeholders::_1, std::placeholders::_2, cl_weak));
		cl->m_manager->receivedStringNotify.connect(
			std::bind(&ClientNetConfigManager::receiveStrStr, this, std::placeholders::_1, std::placeholders::_2, cl_weak));
		cl->m_manager->receivedConfigNotify.connect(std::bind(&ClientNetConfigManager::receiveConfig, this, std::placeholders::_1, cl_weak));

		cl->m_manager->errorNotify.connect(std::bind(&ClientNetConfigManager::serverError, this, std::placeholders::_1, cl_weak));
		cl->m_manager->connectTimeoutNotify.connect(
			std::bind(&ClientNetConfigManager::connectTimeoutError, this, std::placeholders::_1, cl_weak));

		cl->m_manager->startAsioNet(net_lib::EMode::M_CLIENT, host, port);
		m_clients.push_back(cl);
	}
}

auto ClientNetConfigManager::diconnectAll() -> void
{
	const std::lock_guard lock(g_client_mutex);
	for (const auto &c : m_clients) {
		c->m_manager->stopAsioNet();
	}
}

auto ClientNetConfigManager::receiveCommand(uint32_t command, std::string tag, std::weak_ptr<Clients> cl) -> void
{
	auto sender = cl.lock();
	if (!sender)
		return;

	CNetConfigManager::ECommands c = static_cast<CNetConfigManager::ECommands>(command);
	if (c == CNetConfigManager::ECommands::CS_RESPONSE_MASTER_CONNECTED) {
		g_client_mutex.lock();
		sender->m_mode = broadcast_lib::AB_SERVER_MASTER;
		g_client_mutex.unlock();
		serverConnectedNofiy(sender->m_manager->getHost());
	}

	if (c == CNetConfigManager::ECommands::CS_RESPONSE_SLAVE_CONNECTED) {
		g_client_mutex.lock();
		sender->m_mode = broadcast_lib::AB_SERVER_SLAVE;
		g_client_mutex.unlock();
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
		getActiveChannelsNofiy(sender->m_manager->getHost(), tag);
	}

	if (c == CNetConfigManager::ECommands::CS_RESPONSE_SERVER_MODE_TCP) {
		serverModeSDNofiy(sender->m_manager->getHost());
	}

	if (c == CNetConfigManager::ECommands::CS_RESPONSE_SERVER_MODE_SD) {
		serverModeTCPNofiy(sender->m_manager->getHost());
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

auto ClientNetConfigManager::isServersConnected() -> bool
{
	const std::lock_guard lock(g_client_mutex);
	bool ret = true;
	for (auto &c : m_clients) {
		if (c->m_mode == broadcast_lib::AB_NONE && c->m_manager->isConnected()) {
			ret = false;
			break;
		}
	}
	return ret;
}

auto ClientNetConfigManager::receiveStrStr(std::string key, std::string value, std::weak_ptr<Clients> cl) -> void
{
	auto sender = cl.lock();
	if (!sender)
		return;
	if (!sender->m_client_settings.setValue(key, value)) {
		errorNofiy(Errors::CANNT_SET_DATA_TO_CONFIG, sender->m_manager->getHost(), std::error_code());
	} else {
		getNewSettingsItemNofiy(sender->m_manager->getHost());
	}
}

auto ClientNetConfigManager::receiveConfig(std::string value, std::weak_ptr<Clients> cl) -> void
{
	auto sender = cl.lock();
	if (!sender)
		return;
	CStreamSettings settings;
	if (settings.parseJson(value)) {
		sender->m_client_settings = settings;
		sender->m_manager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SETTINGS_GET_SUCCESS);
		getNewSettingsNofiy(sender->m_manager->getHost());
	} else {
		sender->m_manager->sendCommand(CNetConfigManager::ECommands::CS_RESPONSE_SETTINGS_GET_FAIL);
		errorNofiy(Errors::CANNT_SET_DATA_TO_CONFIG, sender->m_manager->getHost(), std::error_code());
	}
}

auto ClientNetConfigManager::serverError(std::error_code error, std::weak_ptr<Clients> cl) -> void
{
	TRACE_SHORT("Server error: %s (%d)", error.message().c_str(), error.value())
	auto sender = cl.lock();
	if (!sender)
		return;
	errorNofiy(Errors::SERVER_INTERNAL, sender->m_manager->getHost(), error);
}

auto ClientNetConfigManager::connectTimeoutError(std::error_code error, std::weak_ptr<Clients> cl) -> void
{
	TRACE_SHORT("Connection timeout error: %s (%d)", error.message().c_str(), error.value())
	auto sender = cl.lock();
	if (!sender)
		return;
	errorNofiy(Errors::CONNECT_TIMEOUT, sender->m_manager->getHost(), error);
}

auto ClientNetConfigManager::sendConfigVariable(const std::string &host, const std::string key, const std::string value) -> bool
{
	const std::lock_guard lock(g_client_mutex);
	for (const auto &c : m_clients) {
		if (c->m_mode != broadcast_lib::AB_NONE && host == c->m_manager->getHost()) {
			auto ret = c->m_manager->sendData(key, value);
			if (!ret) {
				c->m_manager->stopAsioNet();
			}
			return ret;
		}
	}
	return false;
}

auto ClientNetConfigManager::sendConfig(const std::string &host) -> bool
{
	const std::lock_guard lock(g_client_mutex);
	for (const auto &c : m_clients) {
		if (c->m_mode != broadcast_lib::AB_NONE && host == c->m_manager->getHost()) {
			auto ret = c->m_manager->sendConfig(toJson(), true);
			if (!ret) {
				c->m_manager->stopAsioNet();
			}
			return ret;
		}
	}
	return false;
}

auto ClientNetConfigManager::requestConfig(const std::string &host) -> bool
{
	auto it = std::find_if(std::begin(m_clients), std::end(m_clients), [&host](const std::shared_ptr<Clients> c) {
		return c->m_manager->getHost() == host;
	});
	if (it != std::end(m_clients)) {
		return it->operator->()->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_SERVER_SETTINGS);
	}
	return false;
}

auto ClientNetConfigManager::requestConfigVariable(const std::string &host, const std::string &variable) -> bool
{
	auto it = std::find_if(std::begin(m_clients), std::end(m_clients), [&host](const std::shared_ptr<Clients> c) {
		return c->m_manager->getHost() == host;
	});
	if (it != std::end(m_clients)) {
		return it->operator->()->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_SERVER_SETTINGS_VARIABLE, variable);
	}
	return false;
}

auto ClientNetConfigManager::requestMemoryBlockSize(const std::string &host) -> bool
{
	auto it = std::find_if(std::begin(m_clients), std::end(m_clients), [&host](const std::shared_ptr<Clients> c) {
		return c->m_manager->getHost() == host;
	});
	if (it != std::end(m_clients)) {
		return it->operator->()->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_SERVER_MEM_BLOCK_SIZE);
	}
	return false;
}

auto ClientNetConfigManager::requestActiveChannels(const std::string &host) -> bool
{
	auto it = std::find_if(std::begin(m_clients), std::end(m_clients), [&host](const std::shared_ptr<Clients> c) {
		return c->m_manager->getHost() == host;
	});
	if (it != std::end(m_clients)) {
		return it->operator->()->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_SERVER_ACTIVE_CHANNELS);
	}
	return false;
}

auto ClientNetConfigManager::getLocalSettingsOfHost(const std::string &host) -> CStreamSettings *
{
	auto it = std::find_if(std::begin(m_clients), std::end(m_clients), [&host](const std::shared_ptr<Clients> c) {
		return c->m_manager->getHost() == host;
	});
	if (it != std::end(m_clients)) {
		return (CStreamSettings *) &(it->operator->()->m_client_settings);
	}
	return nullptr;
}

auto ClientNetConfigManager::sendSaveToFile(const std::string &host) -> bool
{
	auto it = std::find_if(std::begin(m_clients), std::end(m_clients), [&host](const std::shared_ptr<Clients> c) {
		return c->m_manager->getHost() == host;
	});
	if (it != std::end(m_clients)) {
		return it->operator->()->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_SAVE_SETTING_TO_FILE);
	}
	return false;
}

auto ClientNetConfigManager::sendADCServerStart(const std::string &host) -> bool
{
	auto it = std::find_if(std::begin(m_clients), std::end(m_clients), [&host](const std::shared_ptr<Clients> c) {
		return c->m_manager->getHost() == host;
	});
	if (it != std::end(m_clients)) {
		return it->operator->()->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_ADC_SERVER_START);
	}
	return false;
}

auto ClientNetConfigManager::sendADCServerStop(const std::string &host) -> bool
{
	auto it = std::find_if(std::begin(m_clients), std::end(m_clients), [&host](const std::shared_ptr<Clients> c) {
		return c->m_manager->getHost() == host;
	});
	if (it != std::end(m_clients)) {
		return it->operator->()->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_ADC_SERVER_STOP);
	}
	return false;
}

auto ClientNetConfigManager::sendDACServerStart(const std::string &host, const std::string activeChannels) -> bool
{
	auto it = std::find_if(std::begin(m_clients), std::end(m_clients), [&host](const std::shared_ptr<Clients> c) {
		return c->m_manager->getHost() == host;
	});
	if (it != std::end(m_clients)) {
		return it->operator->()->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_DAC_SERVER_START, activeChannels);
	}
	return false;
}

auto ClientNetConfigManager::sendDACServerStop(const std::string &host) -> bool
{
	auto it = std::find_if(std::begin(m_clients), std::end(m_clients), [&host](const std::shared_ptr<Clients> c) {
		return c->m_manager->getHost() == host;
	});
	if (it != std::end(m_clients)) {
		return it->operator->()->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_DAC_SERVER_STOP);
	}
	return false;
}

auto ClientNetConfigManager::sendADCFPGAStart(const std::string &host) -> bool
{
	auto it = std::find_if(std::begin(m_clients), std::end(m_clients), [&host](const std::shared_ptr<Clients> c) {
		return c->m_manager->getHost() == host;
	});
	if (it != std::end(m_clients)) {
		return it->operator->()->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_ADC_FPGA_START);
	}
	return false;
}

auto ClientNetConfigManager::sendDACFPGAStart(const std::string &host) -> bool
{
	auto it = std::find_if(std::begin(m_clients), std::end(m_clients), [&host](const std::shared_ptr<Clients> c) {
		return c->m_manager->getHost() == host;
	});
	if (it != std::end(m_clients)) {
		return it->operator->()->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_DAC_FPGA_START);
	}
	return false;
}

auto ClientNetConfigManager::sendGetServerMode(const std::string &host) -> bool
{
	auto it = std::find_if(std::begin(m_clients), std::end(m_clients), [&host](const std::shared_ptr<Clients> c) {
		return c->m_manager->getHost() == host;
	});
	if (it != std::end(m_clients)) {
		return it->operator->()->m_manager->sendCommand(CNetConfigManager::ECommands::CS_REQUEST_GET_SERVER_MODE);
	}
	return false;
}

auto ClientNetConfigManager::getModeByHost(const std::string &host) -> broadcast_lib::EMode
{
	const std::lock_guard<std::mutex> lock(g_client_mutex);
	auto it = std::find_if(std::begin(m_clients), std::end(m_clients), [&host](const std::shared_ptr<Clients> c) {
		return c->m_manager->getHost() == host;
	});
	if (it != std::end(m_clients)) {
		return it->operator->()->m_mode;
	}
	return broadcast_lib::AB_NONE;
}
