#include <chrono>

#include "common.h"
#include "config.h"
#include "config_net_lib/client_net_config_manager.h"
#include "logger_lib/file_logger.h"

std::mutex g_smutex;

auto sendConfigCommon(ClientNetConfigManager::Ptr cl, std::string key, std::string value, bool verb) -> bool
{
	auto connected_hosts = cl->getHosts();
	if (connected_hosts.size() != 1) {
		const char *msg = connected_hosts.size() == 0 ? "The server is not connected" : "More than 1 server connected";
		aprintf(stderr, "%s %s\n", getTS(": ").c_str(), msg);
		return false;
	} else {
		return sendConfigCommon(cl, connected_hosts.front(), key, value, verb);
	};
}

auto sendConfigCommon(ClientNetConfigManager::Ptr cl, std::string host, std::string key, std::string value, bool verb) -> bool
{
	std::atomic<int> set_counter;
	cl->successSendConfigNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_smutex);
		if (verb) {
			aprintf(stdout, "%s SET: %s [OK]\n", getTS(": ").c_str(), host.c_str());
			aprintf(stdout, "%s Send configuration save command to: %s\n", getTS(": ").c_str(), host.c_str());
		}
		cl->sendSaveToFile(host);
	});

	cl->failSendConfigNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_smutex);
		if (verb)
			aprintf(stdout, "%s SET: %s [FAIL]\n", getTS(": ").c_str(), host.c_str());
		set_counter--;
	});

	cl->successSaveConfigNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_smutex);
		if (verb)
			aprintf(stdout, "%s SAVE TO FILE: %s [OK]\n", getTS(": ").c_str(), host.c_str());
		set_counter--;
	});

	cl->failSaveConfigNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_smutex);
		if (verb)
			aprintf(stdout, "%s SAVE TO FILE: %s [FAIL]\n", getTS(": ").c_str(), host.c_str());
		set_counter--;
	});

	cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, error_code err) {
		const std::lock_guard lock(g_smutex);
		if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
			aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
			set_counter--;
		}

		if (errors == ClientNetConfigManager::Errors::ERROR_SEND_CONFIG) {
			if (verb)
				aprintf(stderr, "%s Error send configuration: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
			set_counter--;
		}
	});

	set_counter = 1;
	if (verb)
		aprintf(stdout, "%s Send configuration to: %s\n", getTS(": ").c_str(), host.c_str());
	if (!cl->sendConfigVariable(host, key, value)) {
		set_counter--;
	}

	auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	auto timeout = false;
	while (!timeout && set_counter > 0) {
		sleepMs(100);
		timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count()
					   - beginTime
				   > 5000);
	}
	cl->removeHadlers();
	return !timeout;
}

auto getConfigCommon(ClientNetConfigManager::Ptr cl, std::string key, bool verb) -> std::string
{
	auto connected_hosts = cl->getHosts();
	if (connected_hosts.size() != 1) {
		const char *msg = connected_hosts.size() == 0 ? "The server is not connected" : "More than 1 server connected";
		aprintf(stderr, "%s %s\n", getTS(": ").c_str(), msg);
		return "";
	} else {
		return getConfigCommon(cl, connected_hosts.front(), key, verb);
	};
}

auto getConfigCommon(ClientNetConfigManager::Ptr cl, std::string host, std::string key, bool verb) -> std::string
{
	std::string config;
	std::atomic<int> get_counter;
	cl->getNewSettingsItemNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_smutex);
		if (verb)
			aprintf(stdout, "%s Get settings from: %s\n", getTS(": ").c_str(), host.c_str());
		CStreamSettings *s = cl->getLocalSettingsOfHost(host);
		config = s->getValue(key);
		get_counter--;
	});

	cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, error_code err) {
		const std::lock_guard lock(g_smutex);
		if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
			aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
			get_counter--;
		}

		if (errors == ClientNetConfigManager::Errors::CANNT_SET_DATA_TO_CONFIG) {
			if (verb)
				aprintf(stderr, "%s Error get settings from: %s\n", getTS(": ").c_str(), host.c_str());
			get_counter--;
		}
	});

	get_counter = 1;
	if (verb)
		aprintf(stdout, "%s Send configuration request: %s\n", getTS(": ").c_str(), host.c_str());
	if (!cl->requestConfigVariable(host, key)) {
		get_counter--;
	}

	auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	auto timeout = false;
	while (!timeout && get_counter > 0) {
		sleepMs(100);
		timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count()
					   - beginTime
				   > 5000);
	}
	cl->removeHadlers();
	return config;
}

auto sendFileConfigCommon(ClientNetConfigManager::Ptr cl, std::string config, bool verb) -> bool
{
	auto connected_hosts = cl->getHosts();
	if (connected_hosts.size() != 1) {
		const char *msg = connected_hosts.size() == 0 ? "The server is not connected" : "More than 1 server connected";
		aprintf(stderr, "%s %s\n", getTS(": ").c_str(), msg);
		return false;
	} else {
		return sendFileConfigCommon(cl, connected_hosts.front(), config, verb);
	}
}

auto sendFileConfigCommon(ClientNetConfigManager::Ptr cl, std::string host, std::string config, bool verb) -> bool
{
	if (!cl->parseJson(config)) {
		aprintf(stdout, "%s Error applying settings for host: %s\n", getTS(": ").c_str(), host.c_str());
		return false;
	}

	std::atomic<int> set_counter;
	cl->successSendConfigNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_smutex);
		if (verb) {
			aprintf(stdout, "%s SET: %s [OK]\n", getTS(": ").c_str(), host.c_str());
			aprintf(stdout, "%s Send configuration save command to: %s\n", getTS(": ").c_str(), host.c_str());
		}
		cl->sendSaveToFile(host);
	});

	cl->failSendConfigNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_smutex);
		if (verb)
			aprintf(stdout, "%s SET: %s [FAIL]\n", getTS(": ").c_str(), host.c_str());
		set_counter--;
	});

	cl->successSaveConfigNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_smutex);
		if (verb)
			aprintf(stdout, "%s SAVE TO FILE: %s [OK]\n", getTS(": ").c_str(), host.c_str());
		set_counter--;
	});

	cl->failSaveConfigNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_smutex);
		if (verb)
			aprintf(stdout, "%s SAVE TO FILE: %s [FAIL]\n", getTS(": ").c_str(), host.c_str());
		set_counter--;
	});

	cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, error_code err) {
		const std::lock_guard lock(g_smutex);
		if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
			aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
			set_counter--;
		}

		if (errors == ClientNetConfigManager::Errors::ERROR_SEND_CONFIG) {
			if (verb)
				aprintf(stderr, "%s Error send configuration: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
			set_counter--;
		}
	});

	set_counter = 1;
	if (verb)
		aprintf(stdout, "%s Send configuration to: %s\n", getTS(": ").c_str(), host.c_str());
	if (!cl->sendConfig(host)) {
		set_counter--;
	}

	auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	auto timeout = false;
	while (!timeout && set_counter > 0) {
		sleepMs(100);
		timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count()
					   - beginTime
				   > 5000);
	}
	cl->removeHadlers();
	return !timeout;
}

auto getFileConfigCommon(ClientNetConfigManager::Ptr cl, bool verb) -> std::string
{
	auto connected_hosts = cl->getHosts();
	if (connected_hosts.size() != 1) {
		const char *msg = connected_hosts.size() == 0 ? "The server is not connected" : "More than 1 server connected";
		aprintf(stderr, "%s %s\n", getTS(": ").c_str(), msg);
		return "";
	} else {
		return getFileConfigCommon(cl, connected_hosts.front(), verb);
	}
}

auto getFileConfigCommon(ClientNetConfigManager::Ptr cl, std::string host, bool verb) -> std::string
{
	std::atomic<int> get_counter;
	std::string config = "";
	cl->getNewSettingsNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_smutex);
		if (verb)
			aprintf(stdout, "%s Get settings from: %s\n", getTS(": ").c_str(), host.c_str());
		CStreamSettings *s = cl->getLocalSettingsOfHost(host);
		config = s->toJson();
		get_counter--;
	});

	cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, error_code err) {
		const std::lock_guard lock(g_smutex);
		if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
			aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
			get_counter--;
		}
		if (errors == ClientNetConfigManager::Errors::CANNT_SET_DATA_TO_CONFIG) {
			if (verb)
				aprintf(stderr, "%s Error get settings from: %s\n", getTS(": ").c_str(), host.c_str());
			get_counter--;
		}
	});

	get_counter = 1;
	if (verb)
		aprintf(stdout, "%s Send configuration request: %s\n", getTS(": ").c_str(), host.c_str());
	if (!cl->requestConfig(host)) {
		get_counter--;
	}

	auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	auto timeout = false;
	while (!timeout && get_counter > 0) {
		sleepMs(100);
		timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count()
					   - beginTime
				   > 5000);
	}
	cl->removeHadlers();
	return config;
}
