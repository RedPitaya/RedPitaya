#include <memory>
#include <chrono>
#include <thread>

#include "common.h"
#include "config_net_lib/client_net_config_manager.h"
#include "logger_lib/file_logger.h"

std::mutex g_rmutex;

auto getTS(std::string suffix) -> std::string
{
	using namespace std;
	using namespace std::chrono;
	system_clock::time_point timeNow = system_clock::now();
	auto ttime_t = system_clock::to_time_t(timeNow);
	auto tp_sec = system_clock::from_time_t(ttime_t);
	milliseconds ms = duration_cast<milliseconds>(timeNow - tp_sec);

	std::tm *ttm = localtime(&ttime_t);

	char date_time_format[] = "%Y.%m.%d-%H.%M.%S";

	char time_str[] = "yyyy.mm.dd.HH-MM.SS.fff";

	strftime(time_str, strlen(time_str), date_time_format, ttm);

	string result(time_str);
	result.append(".");
	result.append(to_string(ms.count()));
	result.append(suffix);
	return result;
}

auto sleepMs(int ms) -> void
{
	std::this_thread::sleep_for(std::chrono::microseconds(1000 * ms));
}

auto search() -> std::string
{
	ClientNetConfigManager client("", false);
	client.startBroadcast("127.0.0.1", NET_BROADCAST_PORT);
	int timout = 1;

	std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
	auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNow);
	auto value = curTime.time_since_epoch();
	auto timeBegin = value.count();
	while ((value.count() - timeBegin) < timout * 1000) {
		sleepMs(100);
		value = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch();
		auto list = client.getBroadcastClients();
		if (list.size() > 0)
			return list.front().host;
	}
	return "";
}

auto requestMemoryBlockSize(ClientNetConfigManager::Ptr cl,
							const std::list<std::string> &hosts,
							std::map<std::string, uint32_t> *sizes,
							bool verbose) -> bool
{
	std::atomic<int> rstart_counter;

	cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, error_code err) {
		const std::lock_guard<std::mutex> lock(g_rmutex);
		if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
			aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
			rstart_counter--;
		}
	});

	cl->getMemBlockSizeNofiy.connect([&](std::string host, std::string size) {
		const std::lock_guard<std::mutex> lock(g_rmutex);
		if (verbose)
			aprintf(stdout, "%s Memory block size of %s is %s\n", getTS(": ").c_str(), host.c_str(), size.c_str());
		rstart_counter--;
		if (sizes)
			(*sizes)[host] = atoi(size.c_str());
	});

	rstart_counter = hosts.size();
	for (auto &host : hosts) {
		if (verbose)
			aprintf(stdout, "%s Request for memory block size sent : %s\n", getTS(": ").c_str(), host.c_str());
		if (!cl->requestMemoryBlockSize(host)) {
			rstart_counter--;
		}
	}
	auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	auto timeout = false;
	while (!timeout && rstart_counter > 0) {
		sleepMs(100);
		timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime  > 5000);
	}

	cl->removeHadlers();
	return !timeout;
}

auto requestActiveChannels(ClientNetConfigManager::Ptr cl,
						   const std::list<std::string> &hosts,
						   std::map<std::string, uint32_t> *channels,
						   bool verbose) -> bool
{
	std::atomic<int> rstart_counter;

	cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, error_code err) {
		const std::lock_guard<std::mutex> lock(g_rmutex);
		if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
			aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
			rstart_counter--;
		}
	});

	cl->getActiveChannelsNofiy.connect([&](std::string host, std::string size) {
		const std::lock_guard<std::mutex> lock(g_rmutex);
		if (verbose)
			aprintf(stdout, "%s Active channels %s for %s\n", getTS(": ").c_str(), size.c_str(),host.c_str());
		rstart_counter--;
		if (channels)
			(*channels)[host] = atoi(size.c_str());
	});

	rstart_counter = hosts.size();
	for (auto &host : hosts) {
		if (verbose)
			aprintf(stdout, "%s Request for active channels sent : %s\n", getTS(": ").c_str(), host.c_str());
		if (!cl->requestActiveChannels(host)) {
			rstart_counter--;
		}
	}
	auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	auto timeout = false;
	while (!timeout && rstart_counter > 0) {
		sleepMs(100);
		timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime  > 5000);
	}

	cl->removeHadlers();
	return !timeout;
}

auto requestStartStreaming(ClientNetConfigManager::Ptr cl,
					std::list<std::string> masterHosts,
					std::list<std::string> slaveHosts,
					std::map<std::string, StateRunnedHosts> *runned_hosts,
					bool verbous) -> bool
{
	std::atomic<int> rstart_counter;

	cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, error_code err) {
		const std::lock_guard lock(g_rmutex);
		if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
			aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
			rstart_counter--;
			masterHosts.remove(host);
			slaveHosts.remove(host);
		}
	});

	cl->configFileMissedNotify.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s Streaming started: %s config file is missed [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		masterHosts.remove(host);
		slaveHosts.remove(host);
	});

	cl->serverStoppedMemErrorNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s Streaming started: %s memory error [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		masterHosts.remove(host);
		slaveHosts.remove(host);
	});

	cl->serverStoppedNoActiveChannelsNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s Streaming started: %s No active channels [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		masterHosts.remove(host);
		slaveHosts.remove(host);
	});

	cl->serverStoppedMemModifyNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s Streaming started: %s memory modify [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		masterHosts.remove(host);
		slaveHosts.remove(host);
	});

	cl->serverStartedTCPNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s Streaming started: %s TCP mode [OK]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		if (runned_hosts)
			(*runned_hosts)[host] = StateRunnedHosts::TCP;
	});

	cl->serverStartedSDNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s Streaming started: %s Local mode [OK]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		if (runned_hosts)
			(*runned_hosts)[host] = StateRunnedHosts::LOCAL;
	});

	rstart_counter = slaveHosts.size();
	for (auto &host : slaveHosts) {
		if (verbous)
			aprintf(stdout, "%s Send start command to slave board: %s\n", getTS(": ").c_str(), host.c_str());
		if (!cl->sendADCServerStart(host)) {
			rstart_counter--;
		}
	}

	auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	auto timeout = false;
	while (!timeout && rstart_counter > 0) {
		sleepMs(100);
		timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime  > 5000);
	}

	if (timeout){
		cl->removeHadlers();
		return false;
	}

	rstart_counter = masterHosts.size();
	for (auto &host : masterHosts) {
		if (verbous)
			aprintf(stdout, "%s Send start command to master board: %s\n", getTS(": ").c_str(), host.c_str());
		if (!cl->sendADCServerStart(host)) {
			rstart_counter--;
		}
	}

	beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	timeout = false;
	while (!timeout && rstart_counter > 0) {
		sleepMs(100);
		timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime  > 5000);
	}

	cl->removeHadlers();
	return !timeout;
}


auto requestStopStreaming(ClientNetConfigManager::Ptr cl,
				   std::list<std::string> masterHosts,
				   std::list<std::string> slaveHosts,
				   bool verbous) -> bool
{
	std::atomic<int> rstop_counter;

	cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, error_code err) {
		const std::lock_guard<std::mutex> lock(g_rmutex);
		if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
			aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
			rstop_counter--;
			masterHosts.remove(host);
			slaveHosts.remove(host);
		}
	});

	cl->serverStoppedNofiy.connect([&](std::string host) {
		const std::lock_guard<std::mutex> lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s Streaming stopped: %s [OK]\n", getTS(": ").c_str(), host.c_str());
		masterHosts.remove(host);
		slaveHosts.remove(host);
		rstop_counter--;
	});

	cl->serverStoppedSDFullNofiy.connect([&](std::string host) {
		const std::lock_guard<std::mutex> lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s Streaming stopped: %s SD card is full [OK]\n", getTS(": ").c_str(), host.c_str());
		masterHosts.remove(host);
		slaveHosts.remove(host);
		rstop_counter--;
	});

	cl->serverStoppedSDDoneNofiy.connect([&](std::string host) {
		const std::lock_guard<std::mutex> lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s Streaming stopped: %s All samples are recorded on the SD card [OK]\n", getTS(": ").c_str(), host.c_str());
		masterHosts.remove(host);
		slaveHosts.remove(host);
		rstop_counter--;
	});

	rstop_counter = masterHosts.size();
	for (auto &host : masterHosts) {
		if (verbous)
			aprintf(stdout, "%s Send stop command to master board %s\n", getTS(": ").c_str(), host.c_str());
		if (!cl->sendADCServerStop(host)) {
			rstop_counter--;
		}
	}

	auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	auto timeout = false;
	while (!timeout && rstop_counter > 0) {
		sleepMs(100);
		timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime  > 5000);
	}

	if (timeout){
		cl->removeHadlers();
		return false;
	}

	rstop_counter = slaveHosts.size();
	for (auto &host : slaveHosts) {
		if (verbous)
			aprintf(stdout, "%s Send stop command to slave board %s\n", getTS(": ").c_str(), host.c_str());
		if (!cl->sendADCServerStop(host)) {
			rstop_counter--;
		}
	}

	beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	timeout = false;
	while (!timeout && rstop_counter > 0) {
		sleepMs(100);
		timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime  > 5000);
	}

	cl->removeHadlers();
	return !timeout;
}


auto requestStartADC(ClientNetConfigManager::Ptr cl,
			  std::list<std::string> masterHosts,
			  std::list<std::string> slaveHosts,
			  std::map<std::string, StateRunnedHosts> *runned_hosts,
			  bool verbous) -> bool
{
	std::atomic<int> rstart_counter;

	cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, error_code err) {
		const std::lock_guard lock(g_rmutex);
		if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
			aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
			rstart_counter--;
			masterHosts.remove(host);
			slaveHosts.remove(host);
		}
	});

	cl->serverStoppedMemErrorNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s Streaming started: %s memory error [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		masterHosts.remove(host);
		slaveHosts.remove(host);
	});

	cl->serverStoppedMemModifyNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s Streaming started: %s memory modify [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		masterHosts.remove(host);
		slaveHosts.remove(host);
	});

	cl->startADCDoneNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s ADC is run: %s\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		if (runned_hosts)
			(*runned_hosts)[host] = StateRunnedHosts::NONE;
	});

	rstart_counter = slaveHosts.size();

	for (auto &host : slaveHosts) {
		if (verbous)
			aprintf(stdout, "%s Send start ADC command to slave board: %s\n", getTS(": ").c_str(), host.c_str());
		if (!cl->sendADCFPGAStart(host)) {
			rstart_counter--;
		}
	}

	auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	auto timeout = false;
	while (!timeout && rstart_counter > 0) {
		sleepMs(100);
		timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime  > 5000);
	}

	if (timeout){
		cl->removeHadlers();
		return false;
	}

	rstart_counter = masterHosts.size();
	for (auto &host : masterHosts) {
		if (verbous)
			aprintf(stdout, "%s Send start ADC command to master board: %s\n", getTS(": ").c_str(), host.c_str());
		if (!cl->sendADCFPGAStart(host)) {
			rstart_counter--;
		}
	}

	beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	timeout = false;
	while (!timeout && rstart_counter > 0) {
		sleepMs(100);
		timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime  > 5000);
	}

	cl->removeHadlers();
	return !timeout;
}

auto requestStartDACStreaming(ClientNetConfigManager::Ptr cl, std::string host, uint8_t ac, StateRunnedHosts *runned_host, bool verbous)
	-> bool
{
	std::atomic<int> rstart_counter;

	cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, error_code err) {
		const std::lock_guard lock(g_rmutex);
		if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
			aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
			rstart_counter--;
		}
	});

	cl->serverDacStoppedMemErrorNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s Streaming started: %s memory error [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
	});

	cl->serverDacStoppedMemModifyNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s Streaming started: %s memory modify [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
	});

	cl->serverDacStoppedConfigErrorNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s Streaming started: %s config error [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
	});

	cl->configFileMissedNotify.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s Streaming started: %s config file is missed [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
	});

	cl->serverDacStartedNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s DAC streaming started: %s TCP mode [OK]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		*runned_host = StateRunnedHosts::TCP;
	});

	cl->serverDacStartedSDNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (verbous)
			aprintf(stdout, "%s DAC streaming started: %s Local mode [OK]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		*runned_host = StateRunnedHosts::LOCAL;
	});

	rstart_counter = 1;
	if (verbous)
		aprintf(stdout, "%s Send start command to master board: %s\n", getTS(": ").c_str(), host.c_str());
	if (!cl->sendDACServerStart(host, std::to_string(ac))) {
		rstart_counter--;
	}
	auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	auto timeout = false;
	while (!timeout && rstart_counter > 0) {
		sleepMs(100);
		timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count()
					   - beginTime
				   > 5000);
	}

	cl->removeHadlers();
	return !timeout;
}
