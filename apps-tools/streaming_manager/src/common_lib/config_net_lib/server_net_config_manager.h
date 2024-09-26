#ifndef CONFIG_NET_LIB_SNCM_H
#define CONFIG_NET_LIB_SNCM_H

#include "settings_lib/stream_settings.h"
#include "broadcast_lib/asio_broadcast_socket.h"
#include "net_config_manager.h"
#include "data_lib/signal.hpp"

class ServerNetConfigManager
{
public:
	enum Errors {
		CANNT_SET_DATA,
		SERVER_INTERNAL,
		BROADCAST_ERROR
	};

	enum EStopReason {
		NORMAL = 0,
		SD_FULL = 1,
		DONE = 2,
		MEM_ERROR = 3,
		MEM_MODIFY = 4,
		NO_CHANNELS = 5

	};

	using Ptr = std::shared_ptr<ServerNetConfigManager>;

	ServerNetConfigManager(std::string defualt_file_settings_path, broadcast_lib::EMode mode, std::string host, uint16_t port);
	~ServerNetConfigManager();

	auto startServer(std::string host, uint16_t port) -> void;
	auto startBroadcast(broadcast_lib::EModel model, std::string host, uint16_t port) -> void;
	auto setMode(broadcast_lib::EMode mode) -> void;
	auto stop() -> void;
	auto isConnected() -> bool;

	auto sendADCServerStartedTCP() -> bool;
	auto sendADCServerStartedSD() -> bool;
	auto sendADCServerModeTCP() -> bool;
	auto sendADCServerModeSD() -> bool;
	auto sendADCStarted() -> bool;

	auto sendDACServerStarted() -> bool;
	auto sendDACServerStartedSD() -> bool;
	auto sendDACStarted() -> bool;

	auto sendServerStopped() -> bool;
	auto sendServerStoppedSDFull() -> bool;
	auto sendServerStoppedDone() -> bool;
	auto sendServerNoChannelsStopped() -> bool;
	auto sendServerMemoryErrorStopped() -> bool;
	auto sendServerMemoryModifyStopped() -> bool;

	auto sendDACServerStopped() -> bool;
	auto sendDACServerMemoryErrorStopped() -> bool;
	auto sendDACServerMemoryModifyStopped() -> bool;
	auto sendDACServerStoppedSDDone() -> bool;
	auto sendDACServerStoppedSDEmpty() -> bool;
	auto sendDACServerStoppedSDBroken() -> bool;
	auto sendDACServerStoppedSDMissingFile() -> bool;

	auto sendConfigFileMissed() -> bool;

	auto getSettingsRef() -> CStreamSettings &;
	auto setSettingsParameter(const std::string &key, const std::string &value) -> bool;
	auto getSettings() -> const CStreamSettings;

	sigslot::signal<> getNewSettingsNofiy;

	sigslot::signal<> stopStreamingNofiy;
	sigslot::signal<> startStreamingNofiy;

	sigslot::signal<> clientConnectedNofiy;
	sigslot::signal<> clientDisconnectedNofiy;

	sigslot::signal<const std::string&> startDacStreamingNofiy;
	sigslot::signal<> stopDacStreamingNofiy;

	sigslot::signal<> getServerModeNofiy;

	sigslot::signal<> startADCNofiy;
	sigslot::signal<> startDACNofiy;

	sigslot::signal<> memoryBlockSizeChangeNofiy;

	sigslot::signal<ServerNetConfigManager::Errors> errorNofiy;

private:
	enum class States {
		NORMAL,
		GET_DATA,
		GET_TEMP_DATA
	};

	std::shared_ptr<broadcast_lib::CAsioBroadcastSocket> m_pBroadcast;
	std::shared_ptr<CNetConfigManager> m_pNetConfManager;

	auto receiveCommand(uint32_t command, std::string tag) -> void;
	auto receiveValueStr(std::string key, std::string value) -> void;
	auto receiveConfig(std::string value) -> void;
	auto connected(std::string host) -> void;
	auto disconnected(std::string host) -> void;
	auto serverError(std::error_code error) -> void;
	auto sendConfig(bool _async) -> bool;

	std::string m_file_settings;
	broadcast_lib::EMode m_mode;
	CStreamSettings m_settings;
};

#endif
