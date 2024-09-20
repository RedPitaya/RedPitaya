#include <memory>
#include <map>
#include <string>
#include "adc_streaming.h"
#include "common.h"
#include "adc_callback.h"
#include "config_net_lib/client_net_config_manager.h"
#include "data_lib/buffers_cached.h"
#include "uio_lib/memory_manager.h"
#include "logger_lib/file_logger.h"
#include "net_lib/asio_net.h"

struct ADCStreamClient::Impl {
    ClientNetConfigManager::Ptr m_configClient = nullptr;
    bool m_verbose = false;
    ADCCallback* m_callback = nullptr;
    std::map<std::string, bool> m_terminate;
    std::mutex m_smutex;
    std::atomic<int> m_runClientCounter;
    auto runClient(std::string host, uint32_t size, uint32_t activeChannels) -> void;
    std::vector<std::thread*> clients;
};


auto ADCStreamClient::Impl::runClient(std::string host, uint32_t size, uint32_t activeChannels) -> void
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
        m_runClientCounter--;
	});

	g_asionet->clientErrorNotify.connect([&](std::error_code err) {
		const std::lock_guard lock(m_smutex);
		if (m_verbose)
			aprintf(stdout, "%s Error %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
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
                    pack_py.fpgaLost = pack->getLostAll();
                    pack_py.ch1_raw.push_back(1);
                    pack_py.ch1_raw.push_back(2);
                    pack_py.ch1_raw.push_back(3);
                    pack_py.ch2_raw.push_back(666);
                    m_callback->recievePack(pack_py);
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

	m_pimpl->m_configClient->serverConnectedNofiy.connect([&](std::string host) {
		const std::lock_guard lock(m_pimpl->m_smutex);
		if (m_pimpl->m_verbose)
			aprintf(stderr, "%s Connected: %s\n", getTS(": ").c_str(), host.c_str());
		connect_counter--;
	});

	m_pimpl->m_configClient->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, error_code err) {
		const std::lock_guard lock(m_pimpl->m_smutex);
		if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
			aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
			connect_counter--;
		}

		if (errors == ClientNetConfigManager::Errors::CONNECT_TIMEOUT) {
			if (m_pimpl->m_verbose)
				aprintf(stderr, "%s Connect timeout: %s\n", getTS(": ").c_str(), host.c_str());
			connect_counter--;
		}
	});

	connect_counter = hosts.size();
	m_pimpl->m_configClient->connectToServers(hosts, NET_CONFIG_PORT);

	auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	auto timeout = false;
	while (!timeout && connect_counter > 0) {
		sleepMs(100);
		timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime  > 5000);
	}
	m_pimpl->m_configClient->removeHadlers();
	return !timeout;
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
    if (requestStartStreaming(m_pimpl->m_configClient, masterHosts,slaveHosts,&runned_hosts,m_pimpl->m_verbose)) {
		for (auto kv : runned_hosts) {
			if (kv.second == StateRunnedHosts::TCP && activeChannels[kv.first] > 0)
				m_pimpl->clients.push_back(new std::thread(&ADCStreamClient::Impl::runClient,m_pimpl, kv.first, blockSizes[kv.first], activeChannels[kv.first]));
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
			m_pimpl->m_terminate[host] = true;
		});

		m_pimpl->m_configClient->serverStoppedNofiy.connect([&](std::string host) {
			if (m_pimpl->m_verbose)
				aprintf(stderr, "%s Streaming stopped: %s [OK]\n", getTS(": ").c_str(), host.c_str());
			m_pimpl->m_terminate[host] = true;
		});

		m_pimpl->m_configClient->serverStoppedMemErrorNofiy.connect([&](std::string host) {
			if (m_pimpl->m_verbose)
				aprintf(stderr, "%s Streaming stopped: %s [OK]. Not enough DMA memory.\n", getTS(": ").c_str(), host.c_str());
			m_pimpl->m_terminate[host] = true;
		});

		m_pimpl->m_configClient->serverStoppedMemModifyNofiy.connect([&](std::string host) {
			if (m_pimpl->m_verbose)
				aprintf(stderr, "%s Streaming stopped: %s [OK]. The memory manager has changed.\n", getTS(": ").c_str(), host.c_str());
			m_pimpl->m_terminate[host] = true;
		});

		m_pimpl->m_configClient->serverStoppedSDFullNofiy.connect([&](std::string host) {
			if (m_pimpl->m_verbose)
				aprintf(stderr, "%s Streaming stopped: %s SD is full [OK]\n", getTS(": ").c_str(), host.c_str());
			m_pimpl->m_terminate[host] = true;
		});

		m_pimpl->m_configClient->serverStoppedSDDoneNofiy.connect([&](std::string host) {
			if (m_pimpl->m_verbose)
				aprintf(stderr, "%s Streaming stopped: %s Local mode [OK]\n", getTS(": ").c_str(), host.c_str());
			m_pimpl->m_terminate[host] = true;
		});
	}
    return true;
}

auto ADCStreamClient::stopStreaming() -> bool{
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
    return 0;
}

auto ADCStreamClient::setVerbose(bool enable) -> void{
    m_pimpl->m_verbose = enable;
}

auto ADCStreamClient::sendConfig(std::string key, std::string value) -> bool{
    printf("Send %s %s\n",key.c_str(),value.c_str());
    return true;
}

auto ADCStreamClient::sendConfig(std::string host, std::string key, std::string value) -> bool{
     printf("Send %s %s\n",key.c_str(),value.c_str());
    return true;
}


auto ADCStreamClient::getConfig(std::string key) -> std::string{
    return "test";
}

auto ADCStreamClient::getConfig(std::string host, std::string key) -> std::string{

    return "test";
}



