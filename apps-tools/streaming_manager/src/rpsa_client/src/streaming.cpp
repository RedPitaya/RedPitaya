#include "streaming.h"
#include "config.h"
#include "converter_lib/converter.h"
#include "logger_lib/file_logger.h"
#include "net_lib/asio_net.h"
#include "remote.h"
#include "settings_lib/stream_settings.h"
#include "streaming_lib/streaming_file.h"
#include "test_helper.h"
#include "writer_lib/file_helper.h"

std::map<std::string, converter_lib::CConverter::Ptr> g_converter;

std::mutex g_smutex;
std::mutex g_s_csv_mutex;
ClientOpt::Options g_soption;
std::string g_filenameDate;
std::atomic<bool> sig_exit_flag(false);
std::atomic<int> g_runClientCounter;

std::map<std::string, bool> g_terminate;

std::vector<std::thread> clients;

auto stopCSV() -> void;
auto stopStreaming() -> void;
auto stopStreaming(std::string host) -> void;

auto runClient(std::string host, StateRunnedHosts, uint32_t size, uint32_t activeChannels) -> void
{
	auto memoryManager = new uio_lib::CMemoryManager();
	auto buffers = DataLib::CBuffersCached::create();
	memoryManager->setMemoryBlockSize(size);
	memoryManager->reallocateBlocks();
	auto blocks = memoryManager->getFreeBlockCount();
	auto reserved __attribute__((unused)) = memoryManager->reserveMemory(uio_lib::MM_ADC, blocks, activeChannels);
	buffers->generateBuffersEmpty(activeChannels, memoryManager->getRegions(uio_lib::MM_ADC), DataLib::sizeHeader());
	TRACE_SHORT("Reserved blocks %d", reserved)

	g_terminate[host] = false;

	if (g_soption.save_dir == "")
		g_soption.save_dir = ".";

	CStreamSettings::DataFormat file_type = CStreamSettings::DataFormat::BIN;
	switch (g_soption.streamign_type) {
		case ClientOpt::StreamingType::TDMS:
			file_type = CStreamSettings::DataFormat::TDMS;
			break;
		case ClientOpt::StreamingType::WAV:
			file_type = CStreamSettings::DataFormat::WAV;
			break;
		case ClientOpt::StreamingType::CSV:
			file_type = CStreamSettings::DataFormat::BIN;
			break;
		case ClientOpt::StreamingType::BIN:
			file_type = CStreamSettings::DataFormat::BIN;
			break;
		default:
			stopStreaming(host);
			return;
	}

	bool convert_v = false;
	switch (g_soption.save_type) {
		case ClientOpt::SaveType::RAW:
			convert_v = false;
			break;
		case ClientOpt::SaveType::VOL:
			convert_v = true;
			break;
		default:
			convert_v = false;
			break;
	}

	auto g_file_manager = streaming_lib::CStreamingFile::create(file_type, g_soption.save_dir, g_soption.samples, convert_v, false);
	g_file_manager->run(host + "_" + g_filenameDate);

	auto g_s_file_w = std::weak_ptr<streaming_lib::CStreamingFile>(g_file_manager);
	auto g_s_buffers_w = std::weak_ptr<DataLib::CBuffersCached>(buffers);

	auto g_asionet = net_lib::CAsioNet::create(net_lib::M_CLIENT, host, NET_ADC_STREAMING_PORT, buffers);

	g_asionet->clientConnectNotify.connect([](std::string host) {
		const std::lock_guard lock(g_smutex);
		if (g_soption.verbous)
			aprintf(stdout, "%s Connect %s\n", getTS(": ").c_str(), host.c_str());
		g_runClientCounter--;
	});

	g_asionet->clientErrorNotify.connect([host](std::error_code err) {
		const std::lock_guard lock(g_smutex);
		if (g_soption.verbous)
			aprintf(stdout, "%s Error %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
		stopStreaming(host);
		g_runClientCounter--;
	});

	// auto g_net_buffer_w = std::weak_ptr<streaming_lib::CStreamingNetBuffer>(g_net_buffer);
	g_asionet->reciveNotify.connect([g_s_file_w, g_s_buffers_w, host](std::error_code error, DataLib::CDataBuffersPackDMA::Ptr pack) {
		if (!error) {
			auto obj = g_s_file_w.lock();
			auto obj2 = g_s_buffers_w.lock();
			if (obj && obj2) {
				pack->getInfoFromHeaderADC();
				if (g_soption.verbous) {
					uint64_t sempCh1 = 0;
					uint64_t sempCh2 = 0;
					uint64_t sempCh3 = 0;
					uint64_t sempCh4 = 0;
					uint64_t sizeCh1 = 0;
					uint64_t sizeCh2 = 0;
					uint64_t sizeCh3 = 0;
					uint64_t sizeCh4 = 0;

					uint64_t lostRate = 0;
					auto ch1 = pack->getBuffer(DataLib::CH1);
					auto ch2 = pack->getBuffer(DataLib::CH2);
					auto ch3 = pack->getBuffer(DataLib::CH3);
					auto ch4 = pack->getBuffer(DataLib::CH4);

					if (ch1) {
						sempCh1 = ch1->getSamplesCount();
						sizeCh1 = ch1->getDataLenght();
						lostRate += ch1->getLostSamplesAll();
					}

					if (ch2) {
						sempCh2 = ch2->getSamplesCount();
						sizeCh2 = ch2->getDataLenght();
						lostRate += ch2->getLostSamplesAll();
					}

					if (ch3) {
						sempCh3 = ch3->getSamplesCount();
						sizeCh3 = ch3->getDataLenght();
						lostRate += ch3->getLostSamplesAll();
					}

					if (ch4) {
						sempCh4 = ch4->getSamplesCount();
						sizeCh4 = ch4->getDataLenght();
						lostRate += ch4->getLostSamplesAll();
					}

					auto flost = obj->getFileLost();
					// int  brokenBuffer = -1;
					// if (g_soption.testStreamingMode == ClientOpt::TestSteamingMode::WITH_TEST_DATA){
					//     brokenBuffer = testBuffer(ch1 ? static_cast<uint8_t*>(ch1->getMappedDataMemory()) : nullptr,
					//                               ch2 ? static_cast<uint8_t*>(ch2->getMappedDataMemory()) : nullptr,
					//                               ch3 ? static_cast<uint8_t*>(ch3->getMappedDataMemory()) : nullptr,
					//                               ch4 ? static_cast<uint8_t*>(ch4->getMappedDataMemory()) : nullptr,
					//                               sizeCh1,sizeCh2,sizeCh3,sizeCh4) ? 0 : 1;
					// }
					auto h = host;
					addStatisticSteaming(h, sizeCh1 + sizeCh2 + sizeCh3 + sizeCh4, sempCh1, sempCh2, sempCh3, sempCh4, lostRate, flost, -1);
				}
				obj->passBuffers(pack);
				obj2->unlockBufferRead();
			}
		}
	});
	g_asionet->start();
	auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	auto curTime = beginTime;
	while (g_file_manager->isFileThreadWork() && !g_terminate[host]) {
		sleepMs(1);
		if (g_soption.timeout >= 0) {
			if (curTime - beginTime >= g_soption.timeout)
				break;
			curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
		}
		if (g_soption.verbous) {
			printStatisitc(false);
		}
	}

	g_file_manager->stopAndFlush();
	g_asionet->stop();
	g_asionet = nullptr;
	if (g_soption.streamign_type == ClientOpt::StreamingType::CSV) {
		const std::lock_guard lock(g_s_csv_mutex);
		auto fileName = g_file_manager->getCSVFileName();
		g_converter[host] = converter_lib::CConverter::create();
		g_converter[host]->convertToCSV(fileName, host);
	}
	delete memoryManager;
}

auto startStreaming(std::shared_ptr<ClientNetConfigManager> cl, ClientOpt::Options &option) -> void
{
	g_soption = option;

	char time_str[40];
	struct tm *timenow;
	time_t now = time(nullptr);
	timenow = gmtime(&now);
	strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S", timenow);
	g_filenameDate = time_str;

#ifndef _WIN32
	auto size = getFreeSpaceDisk(option.save_dir != "" ? option.save_dir : ".");
	if (g_soption.verbous)
		aprintf(stdout, "%s Free disk space:  %d Mb\n", getTS(": ").c_str(), size / (1024 * 1024));
#endif

	resetStreamingCounter();

	ClientOpt::Options remote_opt = g_soption;
	remote_opt.mode = ClientOpt::Mode::REMOTE;
	remote_opt.remote_mode = ClientOpt::RemoteMode::STOP;
	remote_opt.verbous = g_soption.verbous;
	std::map<string, StateRunnedHosts> runned_hosts;
	if (!startRemote(cl, remote_opt, nullptr, &runned_hosts)) {
		aprintf(stdout, "%s Can't stop streaming on remote machines\n", getTS(": ").c_str());
		return;
	}

	std::map<std::string, uint32_t> blockSizes;
	auto hosts = cl->getHosts();
	if (!requestMemoryBlockSize(cl, hosts, &blockSizes, remote_opt.verbous)) {
		aprintf(stdout, "%s Can't get block sizes\n", getTS(": ").c_str());
		return;
	}

	std::map<std::string, uint32_t> activeChannels;
	if (!requestActiveChannels(cl, hosts, &activeChannels, remote_opt.verbous)) {
		aprintf(stdout, "%s Can't get active channels\n", getTS(": ").c_str());
		return;
	}

	runned_hosts.clear();
	remote_opt.remote_mode = ClientOpt::RemoteMode::START;
	if (startRemote(cl, remote_opt, nullptr, &runned_hosts)) {
		g_runClientCounter = runned_hosts.size();
		for (auto kv : runned_hosts) {
			if (kv.second == StateRunnedHosts::TCP && activeChannels[kv.first] > 0)
				clients.push_back(std::thread(runClient, kv.first, kv.second, blockSizes[kv.first], activeChannels[kv.first]));
		}
		while (g_runClientCounter > 0) {
			sleepMs(100);
			if (sig_exit_flag) {
				break;
			}
		}

		remote_opt.remote_mode = ClientOpt::RemoteMode::START_FPGA_ADC;
		std::vector<string> runnedThreads;
		for (const auto &[key, _] : runned_hosts) {
			runnedThreads.push_back(key);
		}
		if (!startRemote(cl, remote_opt, nullptr, &runned_hosts)) {
			aprintf(stdout, "%s Can't start ADC on remote machines\n", getTS(": ").c_str());
			for (auto &h : runnedThreads) {
				if (runned_hosts.count(h) == 0)
					stopStreaming(h);
			}
		}

		cl->errorNofiy.connect([](ClientNetConfigManager::Errors errors, std::string host, error_code err) {
			if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
				aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
			}
			stopStreaming(host);
		});

		cl->serverStoppedNofiy.connect([](std::string host) {
			if (g_soption.verbous)
				aprintf(stderr, "%s Streaming stopped: %s [OK]\n", getTS(": ").c_str(), host.c_str());
			stopStreaming(host);
		});

		cl->serverStoppedMemErrorNofiy.connect([](std::string host) {
			if (g_soption.verbous)
				aprintf(stderr, "%s Streaming stopped: %s [OK]. Not enough DMA memory.\n", getTS(": ").c_str(), host.c_str());
			stopStreaming(host);
		});

		cl->serverStoppedMemModifyNofiy.connect([](std::string host) {
			if (g_soption.verbous)
				aprintf(stderr, "%s Streaming stopped: %s [OK]. The memory manager has changed.\n", getTS(": ").c_str(), host.c_str());
			stopStreaming(host);
		});

		cl->serverStoppedSDFullNofiy.connect([](std::string host) {
			if (g_soption.verbous)
				aprintf(stderr, "%s Streaming stopped: %s SD is full [OK]\n", getTS(": ").c_str(), host.c_str());
			stopStreaming(host);
		});

		cl->serverStoppedSDDoneNofiy.connect([](std::string host) {
			if (g_soption.verbous)
				aprintf(stderr, "%s Streaming stopped: %s Local mode [OK]\n", getTS(": ").c_str(), host.c_str());
			stopStreaming(host);
		});

		for (std::thread &t : clients) {
			if (t.joinable()) {
				t.join();
			}
		}

		remote_opt.remote_mode = ClientOpt::RemoteMode::STOP;
		if (!startRemote(cl, remote_opt, nullptr, &runned_hosts)) {
			aprintf(stdout, "%s Can't stop streaming on remote machines\n", getTS(": ").c_str());
			g_converter.clear();
			return;
		}

		if (g_soption.verbous) {
			const std::lock_guard<std::mutex> lock(g_smutex);
			printFinalStatisitc();
		}
	}
	g_converter.clear();
}

auto streamingSIGHandler() -> void
{
	stopCSV();
	stopStreaming();
	sig_exit_flag = true;
}

auto stopCSV() -> void
{
	for (const auto &kv : g_converter) {
		kv.second->stopWriteToCSV();
	}
}

auto stopStreaming(std::string host) -> void
{
	g_terminate[host] = true;
}

auto stopStreaming() -> void
{
	for (auto &kv : g_terminate) {
		kv.second = true;
	}
}
