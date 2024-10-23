#include "adc_streaming.h"
#include "callbacks.h"
#include "common.h"
#include "config.h"
#include "config_net_lib/client_net_config_manager.h"
#include "data_lib/buffers_cached.h"
#include "logger_lib/file_logger.h"
#include "net_lib/asio_net.h"
#include "uio_lib/memory_manager.h"
#include <csignal>
#include <functional>
#include <map>
#include <memory>
#include <string>

std::function<void(int)> shutdown_handler;

void signal_handler(int signal) {
	shutdown_handler(signal);
}

struct ADCStreamClient::Impl
{
	ClientNetConfigManager::Ptr m_configClient = nullptr;
	bool m_verbose = false;
	ADCCallback* m_callback = nullptr;
    std::map<std::string, bool> m_terminate;
    std::mutex m_smutex;
    std::atomic<int> m_runClientCounter;
    auto runClient(ADCStreamClient *client, std::string host, uint32_t size, uint32_t activeChannels) -> void;
    std::vector<std::thread*> clients;
};

auto ADCStreamClient::Impl::runClient(ADCStreamClient *client, std::string host, uint32_t size, uint32_t activeChannels) -> void
{
	auto memoryManager = new uio_lib::CMemoryManager();
	auto buffers = DataLib::CBuffersCached::create();
	memoryManager->setMemoryBlockSize(size);
	memoryManager->reallocateBlocks();
	auto blocks = memoryManager->getFreeBlockCount();
	auto reserved __attribute__((unused)) = memoryManager->reserveMemory(uio_lib::MM_ADC, blocks, activeChannels);
	buffers->generateBuffersEmpty(activeChannels, memoryManager->getRegions(uio_lib::MM_ADC), DataLib::sizeHeader());
	TRACE_SHORT("Reserved blocks %d", reserved)

	m_terminate[host] = false;

	auto g_s_buffers_w = std::weak_ptr<DataLib::CBuffersCached>(buffers);

	auto g_asionet = net_lib::CAsioNet::create(net_lib::M_CLIENT, host, NET_ADC_STREAMING_PORT, buffers);

	g_asionet->clientConnectNotify.connect([&](std::string host) {
		const std::lock_guard lock(m_smutex);
		if (m_verbose)
			aprintf(stdout, "%s Connect %s\n", getTS(": ").c_str(), host.c_str());
		if (m_callback) m_callback->connected(client,host);
        m_runClientCounter--;
	});

	g_asionet->clientDisconnectNotify.connect([&](std::string host) {
		const std::lock_guard lock(m_smutex);
		if (m_verbose)
			aprintf(stdout, "%s Disconnected %s\n", getTS(": ").c_str(), host.c_str());
		if (m_callback) m_callback->disconnected(client,host);
	});

	g_asionet->clientErrorNotify.connect([&](std::error_code err) {
		const std::lock_guard lock(m_smutex);
		if (m_verbose)
			aprintf(stdout, "%s Error %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
		if (m_callback) m_callback->error(client,host,err.value());
        m_runClientCounter--;
		m_terminate[host] = true;
	});

    g_asionet->reciveNotify.connect([&](std::error_code error, DataLib::CDataBuffersPackDMA::Ptr pack) {
        const std::lock_guard lock(m_smutex);
		if (!error) {
			auto obj = g_s_buffers_w.lock();
			if (obj) {
				pack->getInfoFromHeaderADC();
                if(m_callback){
                    ADCPack pack_py;
                    pack_py.host = host;

                    auto setChannelData = [](ADCChannel &channel, DataLib::CDataBufferDMA::Ptr buff){
                        if (buff){
                            channel.bitsBySample = buff->getBitBySample();
                            channel.fpgaLost = buff->getLostSamples(DataLib::FPGA);
                            channel.samples = buff->getSamplesCount();
                            channel.adcBaseBits = buff->getADCBaseBits();
                            channel.baseRate = buff->getADCBaseRate();
                            channel.attenuator_1_20 = buff->getADCMode();
                            channel.packId = buff->getADCPackId();
                            channel.raw.reserve(channel.samples);
                            if (channel.bitsBySample == 8){
                                auto data = reinterpret_cast<int8_t*>(buff->getMappedDataMemory());
                                for(size_t k = 0 ; k < channel.samples;k++){
                                    channel.raw.push_back(data[k]);
                                }
                            }

                            if (channel.bitsBySample == 16){
                                auto data = reinterpret_cast<int16_t*>(buff->getMappedDataMemory());
                                for(size_t k = 0 ; k < channel.samples;k++){
                                    channel.raw.push_back(data[k]);
                                }
                            }
                        }
                    };

                    setChannelData(pack_py.channel1,pack->getBuffer(DataLib::EDataBuffersPackChannel::CH1));
                    setChannelData(pack_py.channel2,pack->getBuffer(DataLib::EDataBuffersPackChannel::CH2));
                    setChannelData(pack_py.channel3,pack->getBuffer(DataLib::EDataBuffersPackChannel::CH3));
                    setChannelData(pack_py.channel4,pack->getBuffer(DataLib::EDataBuffersPackChannel::CH4));

                    m_callback->recievePack(client,pack_py);
                }
				obj->unlockBufferRead();
			}
		}
	});
	g_asionet->start();
	while (!m_terminate[host]) {
		sleepMs(100);
	}
	g_asionet->stop();
	g_asionet = nullptr;
	delete memoryManager;
}

ADCStreamClient::ADCStreamClient(){
    m_pimpl = new Impl();
    m_pimpl->m_configClient = std::make_shared<ClientNetConfigManager>("", false);
	auto sigInt = [&](int){
		notifyStop();
	};
	shutdown_handler = sigInt;
	std::signal(SIGINT, signal_handler);
}

ADCStreamClient::~ADCStreamClient(){
    stopStreaming();
    removeReciveDataFunction();
    delete m_pimpl;
}

auto ADCStreamClient::connect() -> bool{
    auto host = search();
    if (host == ""){
        aprintf(stdout, "%s Host not found\n", getTS(": ").c_str());
    }
    return connect({host});
}

auto ADCStreamClient::connect(std::vector<std::string> hosts) -> bool{
    std::atomic<int> connect_counter;
    std::atomic<uint16_t> connected;

	m_pimpl->m_configClient->serverConnectedNofiy.connect([&](std::string host) {
		const std::lock_guard lock(m_pimpl->m_smutex);
		if (m_pimpl->m_verbose)
			aprintf(stderr, "%s Connected: %s\n", getTS(": ").c_str(), host.c_str());
		if (m_pimpl->m_callback) m_pimpl->m_callback->configConnected(this, host);
		connect_counter--;
		connected++;
	});

	m_pimpl->m_configClient->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, error_code err) {
		const std::lock_guard lock(m_pimpl->m_smutex);
		if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
			aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
			if (m_pimpl->m_callback) m_pimpl->m_callback->configError(this, host,err.value());
			connect_counter--;
		}

		if (errors == ClientNetConfigManager::Errors::CONNECT_TIMEOUT) {
			if (m_pimpl->m_verbose)
				aprintf(stderr, "%s Connect timeout: %s\n", getTS(": ").c_str(), host.c_str());
			if (m_pimpl->m_callback) m_pimpl->m_callback->configErrorTimeout(this, host);
			connect_counter--;
		}
	});

	connect_counter = hosts.size();
	connected = 0;
	m_pimpl->m_configClient->connectToServers(hosts, NET_CONFIG_PORT);

	auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	auto timeout = false;
	while (!timeout && connect_counter > 0) {
		sleepMs(100);
		timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime  > 5000);
	}
	m_pimpl->m_configClient->removeHadlers();
	return !timeout && connected == hosts.size();
}

auto ADCStreamClient::setReciveDataFunction(ADCCallback *callback) -> void{
    removeReciveDataFunction();
    const std::lock_guard lock(m_pimpl->m_smutex);
    m_pimpl->m_callback = callback;
}

auto ADCStreamClient::removeReciveDataFunction() -> void{
    const std::lock_guard lock(m_pimpl->m_smutex);
    delete m_pimpl->m_callback;
    m_pimpl->m_callback = nullptr;
}


auto ADCStreamClient::startStreaming() -> bool{

    std::list<std::string> connected_hosts;
	std::list<std::string> slaveHosts;
	std::list<std::string> masterHosts;

	connected_hosts = m_pimpl->m_configClient->getHosts();
	for (auto &host : connected_hosts) {
		switch (m_pimpl->m_configClient->getModeByHost(host)) {
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
	if (!requestStopStreaming(m_pimpl->m_configClient, masterHosts, slaveHosts, m_pimpl->m_verbose)) {
		aprintf(stderr, "%s Can't stop streaming on remote machines\n", getTS(": ").c_str());
		return false;
	}

	std::map<std::string, uint32_t> blockSizes;
	auto hosts = m_pimpl->m_configClient->getHosts();
	if (!requestMemoryBlockSize(m_pimpl->m_configClient, hosts, &blockSizes,m_pimpl->m_verbose)) {
		aprintf(stderr, "%s Can't get block sizes\n", getTS(": ").c_str());
		return false;
	}

	std::map<std::string, uint32_t> activeChannels;
	if (!requestActiveChannels(m_pimpl->m_configClient, hosts, &activeChannels, m_pimpl->m_verbose)) {
		aprintf(stderr, "%s Can't get active channels\n", getTS(": ").c_str());
		return false;
	}

	std::map<string, StateRunnedHosts> runned_hosts;
	stopStreaming();

	if (requestStartStreaming(m_pimpl->m_configClient, masterHosts, slaveHosts, &runned_hosts, m_pimpl->m_verbose)) {
		m_pimpl->m_runClientCounter = runned_hosts.size();
		for (auto kv : runned_hosts) {
			if (kv.second == StateRunnedHosts::TCP && activeChannels[kv.first] > 0)
				m_pimpl->clients.push_back(new std::thread(&ADCStreamClient::Impl::runClient,m_pimpl, this, kv.first, blockSizes[kv.first], activeChannels[kv.first]));
			else{
				m_pimpl->m_runClientCounter--;
			}
		}

        auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
        auto timeout = false;
        while (!timeout && m_pimpl->m_runClientCounter  > 0) {
            sleepMs(100);
            timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime  > 5000);
        }

		std::vector<string> runnedThreads;
		for (const auto &[key, _] : runned_hosts) {
			runnedThreads.push_back(key);
		}

		if (!requestStartADC(m_pimpl->m_configClient, masterHosts,slaveHosts,&runned_hosts,m_pimpl->m_verbose)) {
			aprintf(stdout, "%s Can't start ADC on remote machines\n", getTS(": ").c_str());
			for (auto &h : runnedThreads) {
				if (runned_hosts.count(h) == 0)
					m_pimpl->m_terminate[h] = true;
			}
		}

		m_pimpl->m_configClient->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, error_code err) {
			if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
				aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
			}
			if (m_pimpl->m_callback) m_pimpl->m_callback->configError(this, host,err.value());
			m_pimpl->m_terminate[host] = true;
		});

		m_pimpl->m_configClient->serverStoppedNofiy.connect([&](std::string host) {
			if (m_pimpl->m_verbose)
				aprintf(stderr, "%s Streaming stopped: %s [OK]\n", getTS(": ").c_str(), host.c_str());
			if (m_pimpl->m_callback)
				m_pimpl->m_callback->stopped(this, host);
			m_pimpl->m_terminate[host] = true;
		});

		m_pimpl->m_configClient->serverStoppedNoActiveChannelsNofiy.connect([&](std::string host) {
			if (m_pimpl->m_verbose)
				aprintf(stderr, "%s Streaming stopped: %s [OK]. No active channels.\n", getTS(": ").c_str(), host.c_str());
			if (m_pimpl->m_callback)
				m_pimpl->m_callback->stoppedNoActiveChannels(this, host);
			m_pimpl->m_terminate[host] = true;
		});

		m_pimpl->m_configClient->serverStoppedMemErrorNofiy.connect([&](std::string host) {
			if (m_pimpl->m_verbose)
				aprintf(stderr, "%s Streaming stopped: %s [OK]. Not enough DMA memory.\n", getTS(": ").c_str(), host.c_str());
			if (m_pimpl->m_callback)
				m_pimpl->m_callback->stoppedMemError(this, host);
			m_pimpl->m_terminate[host] = true;
		});

		m_pimpl->m_configClient->serverStoppedMemModifyNofiy.connect([&](std::string host) {
			if (m_pimpl->m_verbose)
				aprintf(stderr, "%s Streaming stopped: %s [OK]. The memory manager has changed.\n", getTS(": ").c_str(), host.c_str());
			if (m_pimpl->m_callback)
				m_pimpl->m_callback->stoppedMemModify(this, host);
			m_pimpl->m_terminate[host] = true;
		});

		m_pimpl->m_configClient->serverStoppedSDFullNofiy.connect([&](std::string host) {
			if (m_pimpl->m_verbose)
				aprintf(stderr, "%s Streaming stopped: %s SD is full [OK]\n", getTS(": ").c_str(), host.c_str());
			if (m_pimpl->m_callback)
				m_pimpl->m_callback->stoppedSDFull(this, host);
			m_pimpl->m_terminate[host] = true;
		});

		m_pimpl->m_configClient->serverStoppedSDDoneNofiy.connect([&](std::string host) {
			if (m_pimpl->m_verbose)
				aprintf(stderr, "%s Streaming stopped: %s Local mode [OK]\n", getTS(": ").c_str(), host.c_str());
			if (m_pimpl->m_callback)
				m_pimpl->m_callback->stoppedSDDone(this, host);
			m_pimpl->m_terminate[host] = true;
		});
	}
	return true;
}

auto ADCStreamClient::stopStreaming() -> void{
    for(auto &kv : m_pimpl->m_terminate){
        kv.second = true;
    }
    for (auto t : m_pimpl->clients) {
		if (t->joinable()) {
			t->join();
		}
		delete t;
	}
    m_pimpl->clients.clear();
}

auto ADCStreamClient::wait() -> void{
	for (auto t : m_pimpl->clients) {
		if (t->joinable()) {
			t->join();
		}
	}
}

auto ADCStreamClient::notifyStop() -> void{
    for(auto &kv : m_pimpl->m_terminate){
        kv.second = true;
    }
}

auto ADCStreamClient::notifyStop(std::string host) -> void{
	for(auto &kv : m_pimpl->m_terminate){
		if (host == kv.first)
		    kv.second = true;
    }
}

auto ADCStreamClient::setVerbose(bool enable) -> void{
    m_pimpl->m_verbose = enable;
}

auto ADCStreamClient::sendConfig(std::string key, std::string value) -> bool{
	return sendConfigCommon(m_pimpl->m_configClient, key, value, m_pimpl->m_verbose);
}

auto ADCStreamClient::sendConfig(std::string host, std::string key, std::string value) -> bool{
	return sendConfigCommon(m_pimpl->m_configClient, host, key, value, m_pimpl->m_verbose);
}


auto ADCStreamClient::getConfig(std::string key) -> std::string{
	return getConfigCommon(m_pimpl->m_configClient, key, m_pimpl->m_verbose);
}

auto ADCStreamClient::getConfig(std::string host, std::string key) -> std::string{
	return getConfigCommon(m_pimpl->m_configClient, host, key, m_pimpl->m_verbose);
}

auto ADCStreamClient::sendFileConfig(std::string config) -> bool{
	return sendFileConfigCommon(m_pimpl->m_configClient, config, m_pimpl->m_verbose);
}

auto ADCStreamClient::sendFileConfig(std::string host, std::string config) -> bool{
	return sendFileConfigCommon(m_pimpl->m_configClient, host, config, m_pimpl->m_verbose);
}

auto ADCStreamClient::getFileConfig() -> std::string{
	return getFileConfigCommon(m_pimpl->m_configClient, m_pimpl->m_verbose);
}

auto ADCStreamClient::getFileConfig(std::string host) -> std::string{
	return getFileConfigCommon(m_pimpl->m_configClient, m_pimpl->m_verbose);
}
