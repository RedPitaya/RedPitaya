#include <chrono>

#include "dac_streaming.h"
#include "dac_streaming_lib/dac_streaming_manager.h"
#include "logger_lib/file_logger.h"
#include "remote.h"
#include "settings_lib/dac_settings_client.h"

using namespace dac_streaming_lib;

std::map<std::string, CDACStreamingManager::Ptr> g_dac_manger;
std::map<std::string, net_lib::CAsioNet::Ptr> g_dac_asionet;

std::mutex g_dac_smutex;
ClientOpt::Options g_dac_soption;
std::atomic<bool> dac_sig_exit_flag(false);

std::map<std::string, long long int> g_dac_timeBegin;
std::map<std::string, bool> g_dac_terminate;
std::map<std::string, bool> g_dac_connected;
std::map<std::string, uint64_t> g_dac_BytesCount;
std::map<std::string, uint64_t> g_dac_packCounter_ch1;
std::map<std::string, uint64_t> g_dac_packCounter_ch2;

std::vector<std::thread> dac_clients;

auto stopDACStreaming() -> void;
auto stopDACStreaming(std::string host) -> void;

auto getActiveChannels(DACSettingsClient conf) -> int
{
	auto file_type = CDACStreamingManager::WAV_TYPE;

	switch (conf.file_type) {
		case CStreamSettings::DataFormat::TDMS:
			file_type = CDACStreamingManager::TDMS_TYPE;
			break;
		case CStreamSettings::DataFormat::WAV:
			file_type = CDACStreamingManager::WAV_TYPE;
			break;
		default:
			return 0;
	}

	auto x = CDACStreamingManager::Create(file_type, conf.dac_file, CStreamSettings::DACRepeat::DAC_REP_OFF, 0, 0, false);
	bool chActive[2] = {false, false};
	if (!x->getChannels(&chActive[0], &chActive[1])) {
		return 0;
	}
	return (int) chActive[0] + (int) chActive[1];
}

auto dac_runClient(DACSettingsClient conf, uint32_t blockSize) -> void
{
	auto memoryManager = new uio_lib::CMemoryManager();
	memoryManager->setMemoryBlockSize(blockSize);
	memoryManager->reallocateBlocks();
	auto blocks = memoryManager->getFreeBlockCount();

	std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
	g_dac_timeBegin[conf.host] = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNow).time_since_epoch().count();

	g_dac_packCounter_ch1[conf.host] = 0;
	g_dac_packCounter_ch2[conf.host] = 0;
	g_dac_BytesCount[conf.host] = 0;
	g_dac_terminate[conf.host] = false;

	auto file_type = CDACStreamingManager::WAV_TYPE;

	switch (conf.file_type) {
		case CStreamSettings::DataFormat::TDMS:
			file_type = CDACStreamingManager::TDMS_TYPE;
			break;
		case CStreamSettings::DataFormat::WAV:
			file_type = CDACStreamingManager::WAV_TYPE;
			break;
		default:
			stopDACStreaming(conf.host);
			return;
	}

	auto rep_mode = CStreamSettings::DACRepeat::DAC_REP_OFF;

	if (conf.dac_repeat == (int) ClientOpt::RepeatDAC::INF) {
		rep_mode = CStreamSettings::DACRepeat::DAC_REP_INF;
	}
	if (conf.dac_repeat >= 0) {
		rep_mode = CStreamSettings::DACRepeat::DAC_REP_ON;
	}

	int dac_rep = conf.dac_repeat >= 0 ? conf.dac_repeat : 0;

	g_dac_manger[conf.host] = CDACStreamingManager::Create(file_type, conf.dac_file, rep_mode, dac_rep, blockSize, conf.verbous);

	bool chActive[2] = {false, false};
	if (!g_dac_manger[conf.host]->getChannels(&chActive[0], &chActive[1])) {
		FATAL("Incorrect mode")
	}
	auto activeChannels = (int) chActive[0] + (int) chActive[1];

	auto reserved __attribute__((unused)) = memoryManager->reserveMemory(uio_lib::MM_DAC, blocks, activeChannels);
	g_dac_manger[conf.host]->getBufferManager()->generateBuffersEmpty(activeChannels,
																	  memoryManager->getRegions(uio_lib::MM_DAC),
																	  DataLib::sizeHeader());
	g_dac_manger[conf.host]->getBufferManager()->initHeadersDAC(activeChannels);
	TRACE_SHORT("Reserved blocks %d", reserved)

	g_dac_manger[conf.host]->notifyStop.connect([=](CDACStreamingManager::NotifyResult res) {
		const std::lock_guard lock(g_dac_smutex);
		switch (res) {
			case CDACStreamingManager::NotifyResult::NR_BROKEN: {
				if (conf.verbous)
					aprintf(stdout, "%s File %s is broken\n", getTS(": ").c_str(), conf.dac_file.c_str());
				break;
			}
			case CDACStreamingManager::NotifyResult::NR_EMPTY: {
				if (conf.verbous)
					aprintf(stdout, "%s File %s is empty\n", getTS(": ").c_str(), conf.dac_file.c_str());
				break;
			}
			case CDACStreamingManager::NotifyResult::NR_MISSING_FILE: {
				if (conf.verbous)
					aprintf(stdout, "%s File %s is missing\n", getTS(": ").c_str(), conf.dac_file.c_str());
				break;
			}
			case CDACStreamingManager::NotifyResult::NR_STOP: {
				if (conf.verbous)
					aprintf(stdout, "%s Got stop command from data controller\n", getTS(": ").c_str());
				break;
			}
			case CDACStreamingManager::NotifyResult::NR_ENDED: {
				if (conf.verbous)
					aprintf(stdout, "%s All data from the file has been read: %s\n", getTS(": ").c_str(), conf.dac_file.c_str());
				break;
			}
			default:
				break;
		}
		g_dac_terminate[conf.host] = true;
	});

	g_dac_connected[conf.host] = false;
	g_dac_asionet[conf.host] = net_lib::CAsioNet::create(net_lib::EMode::M_CLIENT, conf.host, NET_DAC_STREAMING_PORT, nullptr);
	g_dac_asionet[conf.host]->clientConnectNotify.connect([=](std::string host) {
		const std::lock_guard lock(g_dac_smutex);
		aprintf(stdout, "%s CLIENT CONNECTED  %s\n", getTS(": ").c_str(), host.c_str());
		g_dac_connected[host] = true;
	});

	g_dac_asionet[conf.host]->clientDisconnectNotify.connect([=](std::string host) {
		const std::lock_guard lock(g_dac_smutex);
		if (g_dac_connected[host])
			aprintf(stdout, "%s CLIENT DISCONNECTED  %s\n", getTS(": ").c_str(), host.c_str());
		g_dac_connected[host] = false;
	});

	g_dac_asionet[conf.host]->sendNotify.connect([=](error_code err, size_t) {
		if (err.value() != 0 && g_dac_terminate[conf.host] == false) {
			aprintf(stdout, "%s SEND ERROR  %s\n", getTS(": ").c_str(), conf.host.c_str());
			g_dac_terminate[conf.host] = true;
		}
	});

	g_dac_asionet[conf.host]->start();

	auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	auto curTime = beginTime;
	while (!g_dac_connected[conf.host]) {
		if (curTime - beginTime >= 5000)
			break;
		curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	}

	if (g_dac_connected[conf.host]) {
		g_dac_manger[conf.host]->run();
		while (1) {
			auto pack = g_dac_manger[conf.host]->getBuffer();
			if (pack) {
				g_dac_asionet[conf.host]->sendSyncData(pack);
				auto ch1 = pack->getBuffer(DataLib::EDataBuffersPackChannel::CH1);
				auto ch2 = pack->getBuffer(DataLib::EDataBuffersPackChannel::CH2);

				if (ch1) {
					g_dac_packCounter_ch1[conf.host]++;
					g_dac_BytesCount[conf.host] += ch1->getDataLenght();
				}

				if (ch2) {
					g_dac_packCounter_ch2[conf.host]++;
					g_dac_BytesCount[conf.host] += ch2->getDataLenght();
				}
				g_dac_manger[conf.host]->unlockBuffer();
			}

			if (g_dac_terminate[conf.host]) {
				break;
			}

			std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
			auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNow);
			auto value = curTime.time_since_epoch();
			if ((value.count() - g_dac_timeBegin[conf.host]) >= 5000) {
				const std::lock_guard lock(g_dac_smutex);
				uint64_t bw = g_dac_BytesCount[conf.host];
				std::string pref = " ";
				if (g_dac_BytesCount[conf.host] > (1024 * 5)) {
					bw = g_dac_BytesCount[conf.host] / (1024 * 5);
					pref = " ki";
				}

				if (g_dac_BytesCount[conf.host] > (1024 * 1024 * 5)) {
					bw = g_dac_BytesCount[conf.host] / (1024 * 1024 * 5);
					pref = " Mi";
				}
				aprintf(stdout,
						"%s\tHOST IP: %s: Bandwidth:\t%d %sB/s \tData count ch1:\t%d\tch2:\t%d\n",
						getTS(": ").c_str(),
						conf.host.c_str(),
						bw,
						pref.c_str(),
						g_dac_packCounter_ch1[conf.host],
						g_dac_packCounter_ch2[conf.host]);
				g_dac_BytesCount[conf.host] = 0;
				g_dac_timeBegin[conf.host] = value.count();
			}
		}
	}
	stopDACStreaming(conf.host);
	delete memoryManager;
}

auto startDACStreaming(std::shared_ptr<ClientNetConfigManager> cl, ClientOpt::Options &option) -> void
{
	g_dac_soption = option;

	std::map<std::string, uint32_t> blockSizes;
	auto hosts = cl->getHosts();
	if (!requestMemoryBlockSize(cl, hosts, &blockSizes, option.verbous)) {
		aprintf(stdout, "%s Can't get block sizes\n", getTS(": ").c_str());
		return;
	}

	std::map<std::string, uint8_t> acMap;
	for (auto &item : cl->getHosts()) {
		DACSettingsClient conf;
		conf.host = item;
		switch (g_dac_soption.streamign_type) {
			case ClientOpt::StreamingType::TDMS:
				conf.file_type = CStreamSettings::DataFormat::TDMS;
				break;
			case ClientOpt::StreamingType::WAV:
				conf.file_type = CStreamSettings::DataFormat::WAV;
				break;
			default:
				conf.file_type = CStreamSettings::DataFormat::BIN;
				return;
		}
		conf.dac_file = g_dac_soption.dac_file;
		conf.dac_repeat = 0;
		conf.verbous = false;
		auto ac = getActiveChannels(conf);
		acMap[item] = ac;
	}

	ClientOpt::Options remote_opt = g_dac_soption;
	remote_opt.mode = ClientOpt::Mode::REMOTE;
	remote_opt.remote_mode = ClientOpt::RemoteMode::START_DAC;
	remote_opt.verbous = g_dac_soption.verbous;
	std::map<string, StateRunnedHosts> runned_hosts;
	if (startRemote(cl, remote_opt, &acMap, &runned_hosts)) {
		for (auto &kv : runned_hosts) {
			if (kv.second == StateRunnedHosts::TCP) {
				DACSettingsClient conf;
				conf.host = kv.first;
				switch (g_dac_soption.streamign_type) {
					case ClientOpt::StreamingType::TDMS:
						conf.file_type = CStreamSettings::DataFormat::TDMS;
						break;
					case ClientOpt::StreamingType::WAV:
						conf.file_type = CStreamSettings::DataFormat::WAV;
						break;
					default:
						conf.file_type = CStreamSettings::DataFormat::BIN;
						return;
				}
				conf.dac_file = g_dac_soption.dac_file;
				conf.dac_repeat = g_dac_soption.dac_repeat;
				conf.verbous = g_dac_soption.verbous;
				dac_clients.push_back(std::thread(dac_runClient, conf, blockSizes[kv.first]));
			}
		}

		for (std::thread &t : dac_clients) {
			if (t.joinable()) {
				t.join();
			}
		}
	}
}

auto dac_streamingSIGHandler() -> void
{
	stopDACStreaming();
	dac_sig_exit_flag = true;
}

auto stopDACStreaming(std::string host) -> void
{
	if (g_dac_asionet.count(host)) {
		if (g_dac_asionet[host]) {
			g_dac_asionet[host]->stop();
			g_dac_asionet[host] = nullptr;
		}
	}
	if (g_dac_manger.count(host)) {
		if (g_dac_manger[host]) {
			g_dac_manger[host]->stop();
			g_dac_manger[host] = nullptr;
		}
	}
	g_dac_terminate[host] = true;
}

auto stopDACStreaming() -> void
{
	for (auto &kv : g_dac_terminate) {
		kv.second = true;
	}
}
