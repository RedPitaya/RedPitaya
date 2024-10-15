#include "remote.h"
#include "config.h"
#include "logger_lib/file_logger.h"
#include <mutex>

#ifdef _WIN32
#include <dir.h>
#else
#include <sys/stat.h>
#endif

ClientOpt::Options g_roption;
//std::atomic<int>   g_rstart_counter;
//std::atomic<int>   g_rstop_counter;
std::mutex g_rmutex;
std::atomic<bool> g_rexit_flag;

auto startStreaming(std::shared_ptr<ClientNetConfigManager> cl,
					std::list<std::string> &masterHosts,
					std::list<std::string> &slaveHosts,
					std::map<std::string, StateRunnedHosts> *runned_hosts) -> bool;
auto startDACStreaming(std::shared_ptr<ClientNetConfigManager> cl,
					   std::list<std::string> &masterHosts,
					   std::list<std::string> &slaveHosts,
					   std::map<std::string, uint8_t> *activeChannels,
					   std::map<std::string, StateRunnedHosts> *runned_hosts) -> bool;
auto stopStreaming(std::shared_ptr<ClientNetConfigManager> cl,
				   std::list<std::string> &masterHosts,
				   std::list<std::string> &slaveHosts,
				   std::map<std::string, StateRunnedHosts> *runned_hosts) -> bool;
auto stopDACStreaming(std::shared_ptr<ClientNetConfigManager> cl,
					  std::list<std::string> &masterHosts,
					  std::list<std::string> &slaveHosts,
					  std::map<std::string, StateRunnedHosts> *runned_hosts) -> bool;
auto startStopStreaming(std::shared_ptr<ClientNetConfigManager> cl,
						std::list<std::string> &masterHosts,
						std::list<std::string> &slaveHosts,
						std::map<std::string, StateRunnedHosts> *runned_hosts) -> bool;
auto startStopDACStreaming(std::shared_ptr<ClientNetConfigManager> cl,
						   std::list<std::string> &masterHosts,
						   std::list<std::string> &slaveHosts,
						   std::map<std::string, uint8_t> *activeChannels,
						   std::map<std::string, StateRunnedHosts> *runned_hosts) -> bool;
auto startADC(std::shared_ptr<ClientNetConfigManager> cl,
			  std::list<std::string> &masterHosts,
			  std::list<std::string> &slaveHosts,
			  std::map<std::string, StateRunnedHosts> *runned_hosts) -> bool;

auto startRemote(std::shared_ptr<ClientNetConfigManager> cl,
				 ClientOpt::Options &option,
				 std::map<std::string, uint8_t> *activeChannels,
				 std::map<std::string, StateRunnedHosts> *runned_hosts) -> bool
{
	std::list<std::string> connected_hosts;
	std::list<std::string> slaveHosts;
	std::list<std::string> masterHosts;
	g_rexit_flag = false;
	g_roption = option;

	connected_hosts = cl->getHosts();
	for (auto &host : connected_hosts) {
		switch (cl->getModeByHost(host)) {
			case broadcast_lib::AB_SERVER_MASTER:
				masterHosts.push_back(host);
				break;
			case broadcast_lib::AB_SERVER_SLAVE:
				slaveHosts.push_back(host);
				break;
			default:
				break;
		}
	}
	switch (g_roption.remote_mode) {
		case ClientOpt::RemoteMode::START: {
			return startStreaming(cl, masterHosts, slaveHosts, runned_hosts);
		}

		case ClientOpt::RemoteMode::START_WITH_ADC: {
			if (startStreaming(cl, masterHosts, slaveHosts, runned_hosts)) {
				return startADC(cl, masterHosts, slaveHosts, runned_hosts);
			}
			return false;
		}

		case ClientOpt::RemoteMode::START_DAC: {
			return startDACStreaming(cl, masterHosts, slaveHosts, activeChannels, runned_hosts);
		}

		case ClientOpt::RemoteMode::START_FPGA_ADC: {
			return startADC(cl, masterHosts, slaveHosts, runned_hosts);
		}

		case ClientOpt::RemoteMode::STOP: {
			return stopStreaming(cl, masterHosts, slaveHosts, runned_hosts);
		}

		case ClientOpt::RemoteMode::STOP_DAC: {
			return stopDACStreaming(cl, masterHosts, slaveHosts, runned_hosts);
		}

		case ClientOpt::RemoteMode::START_STOP: {
			return startStopStreaming(cl, masterHosts, slaveHosts, runned_hosts);
		}

		case ClientOpt::RemoteMode::START_STOP_DAC: {
			return startStopDACStreaming(cl, masterHosts, slaveHosts, activeChannels, runned_hosts);
		}

		default: {
			aprintf(stderr, "%s [Fatal] Error mode\n", getTS(": ").c_str());
			return false;
		}
	}
	return true;
}

auto remoteSIGHandler() -> void
{
	g_rexit_flag = true;
}

auto startStreaming(std::shared_ptr<ClientNetConfigManager> cl,
					std::list<std::string> &masterHosts,
					std::list<std::string> &slaveHosts,
					std::map<std::string, StateRunnedHosts> *runned_hosts) -> bool
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
		if (g_roption.verbous)
			aprintf(stdout, "%s Streaming started: %s config file is missed [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		masterHosts.remove(host);
		slaveHosts.remove(host);
	});

	cl->serverStoppedMemErrorNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s Streaming started: %s memory error [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		masterHosts.remove(host);
		slaveHosts.remove(host);
	});

	cl->serverStoppedMemModifyNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s Streaming started: %s memory modify [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		masterHosts.remove(host);
		slaveHosts.remove(host);
	});

	cl->serverStoppedNoActiveChannelsNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stderr, "%s Streaming started: %s. No active channels. [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		masterHosts.remove(host);
		slaveHosts.remove(host);
	});

	cl->serverStartedTCPNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s Streaming started: %s TCP mode [OK]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		if (runned_hosts)
			(*runned_hosts)[host] = StateRunnedHosts::TCP;
	});

	cl->serverStartedSDNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s Streaming started: %s Local mode [OK]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		if (runned_hosts)
			(*runned_hosts)[host] = StateRunnedHosts::LOCAL;
	});

	rstart_counter = slaveHosts.size();
	for (auto &host : slaveHosts) {
		if (g_roption.verbous)
			aprintf(stdout, "%s Send start command to slave board: %s\n", getTS(": ").c_str(), host.c_str());
		if (!cl->sendADCServerStart(host)) {
			rstart_counter--;
		}
	}
	while (rstart_counter > 0) {
		sleepMs(100);
		if (g_rexit_flag) {
			cl->removeHadlers();
			return false;
		}
	}

	rstart_counter = masterHosts.size();
	for (auto &host : masterHosts) {
		if (g_roption.verbous)
			aprintf(stdout, "%s Send start command to master board: %s\n", getTS(": ").c_str(), host.c_str());
		if (!cl->sendADCServerStart(host)) {
			rstart_counter--;
		}
	}

	while (rstart_counter > 0) {
		sleepMs(100);
		if (g_rexit_flag) {
			cl->removeHadlers();
			return false;
		}
	}
	cl->removeHadlers();
	return true;
}

auto requestMemoryBlockSize(std::shared_ptr<ClientNetConfigManager> cl,
							std::list<std::string> &hosts,
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
	while (rstart_counter > 0) {
		sleepMs(100);
		if (g_rexit_flag) {
			cl->removeHadlers();
			return false;
		}
	}
	cl->removeHadlers();
	return true;
}

auto requestActiveChannels(std::shared_ptr<ClientNetConfigManager> cl,
						   std::list<std::string> &hosts,
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
			aprintf(stdout, "%s %s Active channels: %s\n", getTS(": ").c_str(), host.c_str(), size.c_str());
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
	while (rstart_counter > 0) {
		sleepMs(100);
		if (g_rexit_flag) {
			cl->removeHadlers();
			return false;
		}
	}
	cl->removeHadlers();
	return true;
}

auto startADC(std::shared_ptr<ClientNetConfigManager> cl,
			  std::list<std::string> &masterHosts,
			  std::list<std::string> &slaveHosts,
			  std::map<std::string, StateRunnedHosts> *runned_hosts) -> bool
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
		if (g_roption.verbous)
			aprintf(stdout, "%s Streaming started: %s memory error [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		masterHosts.remove(host);
		slaveHosts.remove(host);
	});

	cl->serverStoppedMemModifyNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s Streaming started: %s memory modify [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		masterHosts.remove(host);
		slaveHosts.remove(host);
	});

	cl->serverStoppedNoActiveChannelsNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stderr, "%s Streaming started: %s. No active channels. [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		masterHosts.remove(host);
		slaveHosts.remove(host);
	});

	cl->startADCDoneNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s ADC is run: %s\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		if (runned_hosts)
			(*runned_hosts)[host] = StateRunnedHosts::NONE;
	});

	rstart_counter = slaveHosts.size();

	for (auto &host : slaveHosts) {
		if (g_roption.verbous)
			aprintf(stdout, "%s Send start ADC command to slave board: %s\n", getTS(": ").c_str(), host.c_str());
		if (!cl->sendADCFPGAStart(host)) {
			rstart_counter--;
		}
	}
	while (rstart_counter > 0) {
		sleepMs(100);
		if (g_rexit_flag) {
			cl->removeHadlers();
			return false;
		}
	}

	rstart_counter = masterHosts.size();
	for (auto &host : masterHosts) {
		if (g_roption.verbous)
			aprintf(stdout, "%s Send start ADC command to master board: %s\n", getTS(": ").c_str(), host.c_str());
		if (!cl->sendADCFPGAStart(host)) {
			rstart_counter--;
		}
	}
	while (rstart_counter > 0) {
		sleepMs(100);
		if (g_rexit_flag) {
			cl->removeHadlers();
			return false;
		}
	}
	cl->removeHadlers();
	return true;
}

auto startDACStreaming(std::shared_ptr<ClientNetConfigManager> cl,
					   std::list<std::string> &masterHosts,
					   std::list<std::string> &slaveHosts,
					   std::map<std::string, uint8_t> *activeChannels,
					   std::map<std::string, StateRunnedHosts> *runned_hosts) -> bool
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

	cl->serverDacStoppedMemErrorNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s Streaming started: %s memory error [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		masterHosts.remove(host);
		slaveHosts.remove(host);
	});

	cl->serverDacStoppedMemModifyNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s Streaming started: %s memory modify [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		masterHosts.remove(host);
		slaveHosts.remove(host);
	});

	cl->serverDacStoppedConfigErrorNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s Streaming started: %s config error [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		masterHosts.remove(host);
		slaveHosts.remove(host);
	});

	cl->configFileMissedNotify.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s Streaming started: %s config file is missed [FAIL]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		masterHosts.remove(host);
		slaveHosts.remove(host);
	});

	cl->serverDacStartedNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s DAC streaming started: %s TCP mode [OK]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		if (runned_hosts)
			(*runned_hosts)[host] = StateRunnedHosts::TCP;
	});

	cl->serverDacStartedSDNofiy.connect([&](std::string host) {
		const std::lock_guard lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s DAC streaming started: %s Local mode [OK]\n", getTS(": ").c_str(), host.c_str());
		rstart_counter--;
		if (runned_hosts)
			(*runned_hosts)[host] = StateRunnedHosts::LOCAL;
	});

	rstart_counter = slaveHosts.size();

	for (auto &host : slaveHosts) {
		auto ac = 0;
		if (activeChannels != nullptr) {
			if (activeChannels->count(host)) {
				ac = activeChannels->at(host);
			}
		}
		if (g_roption.verbous)
			aprintf(stdout, "%s Send start DAC command to slave board: %s Active channels: %d\n", getTS(": ").c_str(), host.c_str(), ac);
		if (!cl->sendDACServerStart(host, std::to_string(ac))) {
			rstart_counter--;
		}
	}
	while (rstart_counter > 0) {
		sleepMs(100);
		if (g_rexit_flag) {
			cl->removeHadlers();
			return false;
		}
	}

	rstart_counter = masterHosts.size();
	for (auto &host : masterHosts) {
		auto ac = 0;
		if (activeChannels != nullptr) {
			if (activeChannels->count(host)) {
				ac = activeChannels->at(host);
			}
		}
		if (g_roption.verbous)
			aprintf(stdout, "%s Send start DAC command to master board: %s Active channels: %d\n", getTS(": ").c_str(), host.c_str(), ac);
		if (!cl->sendDACServerStart(host, std::to_string(ac))) {
			rstart_counter--;
		}
	}
	while (rstart_counter > 0) {
		sleepMs(100);
		if (g_rexit_flag) {
			cl->removeHadlers();
			return false;
		}
	}
	cl->removeHadlers();
	return true;
}

auto stopStreaming(std::shared_ptr<ClientNetConfigManager> cl,
				   std::list<std::string> &masterHosts,
				   std::list<std::string> &slaveHosts,
				   std::map<std::string, StateRunnedHosts> *) -> bool
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
		if (g_roption.verbous)
			aprintf(stdout, "%s Streaming stopped: %s [OK]\n", getTS(": ").c_str(), host.c_str());
		masterHosts.remove(host);
		slaveHosts.remove(host);
		rstop_counter--;
	});

	cl->serverStoppedSDFullNofiy.connect([&](std::string host) {
		const std::lock_guard<std::mutex> lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s Streaming stopped: %s SD card is full [OK]\n", getTS(": ").c_str(), host.c_str());
		masterHosts.remove(host);
		slaveHosts.remove(host);
		rstop_counter--;
	});

	cl->serverStoppedSDDoneNofiy.connect([&](std::string host) {
		const std::lock_guard<std::mutex> lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s Streaming stopped: %s All samples are recorded on the SD card [OK]\n", getTS(": ").c_str(), host.c_str());
		masterHosts.remove(host);
		slaveHosts.remove(host);
		rstop_counter--;
	});

	rstop_counter = masterHosts.size();
	for (auto &host : masterHosts) {
		if (g_roption.verbous)
			aprintf(stdout, "%s Send stop command to master board %s\n", getTS(": ").c_str(), host.c_str());
		if (!cl->sendADCServerStop(host)) {
			rstop_counter--;
		}
	}
	while (rstop_counter > 0) {
		sleepMs(100);
		if (g_rexit_flag) {
			cl->removeHadlers();
			return false;
		}
	}

	rstop_counter = slaveHosts.size();
	for (auto &host : slaveHosts) {
		if (g_roption.verbous)
			aprintf(stdout, "%s Send stop command to slave board %s\n", getTS(": ").c_str(), host.c_str());
		if (!cl->sendADCServerStop(host)) {
			rstop_counter--;
		}
	}
	while (rstop_counter > 0) {
		sleepMs(100);
		if (g_rexit_flag) {
			cl->removeHadlers();
			return false;
		}
	}
	cl->removeHadlers();
	return true;
}

auto stopDACStreaming(std::shared_ptr<ClientNetConfigManager> cl,
					  std::list<std::string> &masterHosts,
					  std::list<std::string> &slaveHosts,
					  std::map<std::string, StateRunnedHosts> *) -> bool
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

	cl->serverDacStoppedNofiy.connect([&](std::string host) {
		const std::lock_guard<std::mutex> lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s DAC streaming stopped: %s [OK]\n", getTS(": ").c_str(), host.c_str());
		masterHosts.remove(host);
		slaveHosts.remove(host);
		rstop_counter--;
	});

	cl->serverDacStoppedSDDoneNofiy.connect([&](std::string host) {
		const std::lock_guard<std::mutex> lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s DAC streaming stopped: %s SD done [OK]\n", getTS(": ").c_str(), host.c_str());
		masterHosts.remove(host);
		slaveHosts.remove(host);
		rstop_counter--;
	});

	cl->serverDacStoppedSDBrokenNofiy.connect([&](std::string host) {
		const std::lock_guard<std::mutex> lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s DAC streaming stopped: %s SD File broken [OK]\n", getTS(": ").c_str(), host.c_str());
		masterHosts.remove(host);
		slaveHosts.remove(host);
		rstop_counter--;
	});

	cl->serverDacStoppedSDEmptyNofiy.connect([&](std::string host) {
		const std::lock_guard<std::mutex> lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s DAC streaming stopped: %s SD File empty [OK]\n", getTS(": ").c_str(), host.c_str());
		masterHosts.remove(host);
		slaveHosts.remove(host);
		rstop_counter--;
	});

	cl->serverDacStoppedSDMissingNofiy.connect([&](std::string host) {
		const std::lock_guard<std::mutex> lock(g_rmutex);
		if (g_roption.verbous)
			aprintf(stdout, "%s DAC streaming stopped: %s SD File missing [OK]\n", getTS(": ").c_str(), host.c_str());
		masterHosts.remove(host);
		slaveHosts.remove(host);
		rstop_counter--;
	});

	rstop_counter = masterHosts.size();
	for (auto &host : masterHosts) {
		if (g_roption.verbous)
			aprintf(stdout, "%s Send DAC stop command to master board %s\n", getTS(": ").c_str(), host.c_str());
		if (!cl->sendDACServerStop(host)) {
			rstop_counter--;
		}
	}
	while (rstop_counter > 0) {
		sleepMs(100);
		if (g_rexit_flag) {
			cl->removeHadlers();
			return false;
		}
	}

	rstop_counter = slaveHosts.size();
	for (auto &host : slaveHosts) {
		if (g_roption.verbous)
			aprintf(stdout, "%s Send DAC stop command to slave board %s\n", getTS(": ").c_str(), host.c_str());
		if (!cl->sendDACServerStop(host)) {
			rstop_counter--;
		}
	}
	while (rstop_counter > 0) {
		sleepMs(100);
		if (g_rexit_flag) {
			cl->removeHadlers();
			return false;
		}
	}
	cl->removeHadlers();
	return true;
}

auto startStopStreaming(std::shared_ptr<ClientNetConfigManager> cl,
						std::list<std::string> &masterHosts,
						std::list<std::string> &slaveHosts,
						std::map<std::string, StateRunnedHosts> *runned_hosts) -> bool
{
	if (!startStreaming(cl, masterHosts, slaveHosts, runned_hosts)) {
		return false;
	}

	if (!startADC(cl, masterHosts, slaveHosts, runned_hosts)) {
		return false;
	}

	auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	auto curTime = beginTime;
	g_roption.timeout = g_roption.timeout == 0 ? 1000 : g_roption.timeout;
	while ((curTime - beginTime <= g_roption.timeout) && (masterHosts.size() > 0 || slaveHosts.size() > 0)) {
		sleepMs(1);
		if (g_rexit_flag)
			break;
		curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	}

	g_rexit_flag = false;

	if (!stopStreaming(cl, masterHosts, slaveHosts, runned_hosts)) {
		return false;
	}

	return true;
}

auto startStopDACStreaming(std::shared_ptr<ClientNetConfigManager> cl,
						   std::list<std::string> &masterHosts,
						   std::list<std::string> &slaveHosts,
						   std::map<std::string, uint8_t> *activeChannels,
						   std::map<std::string, StateRunnedHosts> *runned_hosts) -> bool
{
	if (!startDACStreaming(cl, masterHosts, slaveHosts, activeChannels, runned_hosts)) {
		return false;
	}

	auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	auto curTime = beginTime;
	g_roption.timeout = g_roption.timeout == 0 ? 1000 : g_roption.timeout;
	while ((curTime - beginTime <= g_roption.timeout) && (masterHosts.size() > 0 || slaveHosts.size() > 0)) {
		sleepMs(1);
		if (g_rexit_flag)
			break;
		curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	}

	if (!stopDACStreaming(cl, masterHosts, slaveHosts, runned_hosts)) {
		return false;
	}

	return true;
}
