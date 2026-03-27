#include "adc_streaming.h"
#include <csignal>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include "callbacks.h"
#include "common.h"
#include "config.h"
#include "config_net_lib/client_net_config_manager.h"
#include "config_streaming.h"
#include "data_lib/buffers_cached.h"
#include "logger_lib/file_logger.h"
#include "net_lib/asio_net.h"
#include "uio_lib/memory_manager.h"

class ADCCb : public ConfigCallback {

    void configError(ConfigStreamClient* cl, std::string host, int error) override {
        if (m_terminate)
            m_terminate->at(host) = true;
    }

    void adcServerStopped(ConfigStreamClient*, std::string host) override {
        if (m_terminate)
            m_terminate->at(host) = true;
    }
    void adcServerStoppedNoActiveChannels(ConfigStreamClient*, std::string host) override {
        if (m_terminate)
            m_terminate->at(host) = true;
    }
    void adcServerStoppedMemError(ConfigStreamClient*, std::string host) override {
        if (m_terminate)
            m_terminate->at(host) = true;
    }
    void adcServerStoppedMemModify(ConfigStreamClient*, std::string host) override {
        if (m_terminate)
            m_terminate->at(host) = true;
    }
    void adcServerStoppedSDFull(ConfigStreamClient*, std::string host) override {
        if (m_terminate)
            m_terminate->at(host) = true;
    }
    void adcServerStoppedSDDone(ConfigStreamClient*, std::string host) override {
        if (m_terminate)
            m_terminate->at(host) = true;
    }

    std::map<std::string, bool>* m_terminate = nullptr;

   public:
    explicit ADCCb(std::map<std::string, bool>* terminate) : m_terminate(terminate){};
};

struct ADCStreamClient::Impl {
    std::shared_ptr<ConfigStreamClient> m_configClient = nullptr;
    bool m_verbose = false;
    std::shared_ptr<ADCCallback> m_callback;
    std::map<std::string, bool> m_terminate;
    std::mutex m_smutex;
    std::atomic<int> m_runClientCounter;
    auto runClient(ADCStreamClient* client, std::string host, uint32_t size, uint32_t activeChannels) -> void;
    std::vector<std::thread*> clients;
    std::shared_ptr<ADCCb> m_configCallback;
};

auto ADCStreamClient::Impl::runClient(ADCStreamClient* client, std::string host, uint32_t size, uint32_t activeChannels) -> void {
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
        if (m_callback)
            m_callback->connected(client, host);
        m_runClientCounter--;
    });

    g_asionet->clientDisconnectNotify.connect([&](std::string host) {
        const std::lock_guard lock(m_smutex);
        if (m_verbose)
            aprintf(stdout, "%s Disconnected %s\n", getTS(": ").c_str(), host.c_str());
        if (m_callback)
            m_callback->disconnected(client, host);
    });

    g_asionet->clientErrorNotify.connect([&](std::error_code err) {
        const std::lock_guard lock(m_smutex);
        if (m_verbose)
            aprintf(stdout, "%s Error %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
        if (m_callback)
            m_callback->error(client, host, err.value());
        m_terminate[host] = true;
        m_runClientCounter--;
    });

    g_asionet->reciveNotify.connect([&](std::error_code error, DataLib::CDataBuffersPackDMA::Ptr pack) {
        const std::lock_guard lock(m_smutex);
        if (!error) {
            auto obj = g_s_buffers_w.lock();
            if (obj) {
                pack->getInfoFromHeaderADC();
                if (m_callback) {
                    ADCPack pack_py;
                    pack_py.host = host;

                    auto setChannelData = [](ADCChannel& channel, DataLib::CDataBufferDMA::Ptr buff) {
                        if (buff) {
                            channel.bitsPerSample = buff->getBitBySample();
                            channel.fpgaLost = buff->getLostSamples(DataLib::FPGA);
                            channel.samples = buff->getSamplesCount();
                            channel.adcBaseBits = buff->getADCBaseBits();
                            channel.baseRate = buff->getADCBaseRate();
                            channel.attenuator_1_20 = buff->getADCMode();
                            channel.packId = buff->getADCPackId();
                            channel.timeCapture = buff->getTimeCapture();
                            channel.raw.reserve(channel.samples);
                            if (channel.bitsPerSample == 8) {
                                auto data = reinterpret_cast<int8_t*>(buff->getMappedDataMemory());
                                for (size_t k = 0; k < channel.samples; k++) {
                                    channel.raw.push_back(data[k]);
                                }
                            }

                            if (channel.bitsPerSample == 16) {
                                auto data = reinterpret_cast<int16_t*>(buff->getMappedDataMemory());
                                for (size_t k = 0; k < channel.samples; k++) {
                                    channel.raw.push_back(data[k]);
                                }
                            }
                        }
                    };

                    setChannelData(pack_py.channel1, pack->getBuffer(DataLib::EDataBuffersPackChannel::CH1));
                    setChannelData(pack_py.channel2, pack->getBuffer(DataLib::EDataBuffersPackChannel::CH2));
                    setChannelData(pack_py.channel3, pack->getBuffer(DataLib::EDataBuffersPackChannel::CH3));
                    setChannelData(pack_py.channel4, pack->getBuffer(DataLib::EDataBuffersPackChannel::CH4));

                    m_callback->receivePack(client, pack_py);
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

ADCStreamClient::ADCStreamClient(std::shared_ptr<ConfigStreamClient> configClient) {
    m_pimpl = new Impl();
    m_pimpl->m_configClient = configClient;
    m_pimpl->m_configCallback = std::make_shared<ADCCb>(&m_pimpl->m_terminate);
    m_pimpl->m_configClient->addCallback(m_pimpl->m_configCallback);
}

ADCStreamClient::~ADCStreamClient() {
    stopStreaming();
    removeCallback();
    delete m_pimpl;
}

auto ADCStreamClient::setCallback(std::shared_ptr<ADCCallback> callback) -> void {
    removeCallback();
    const std::lock_guard lock(m_pimpl->m_smutex);
    m_pimpl->m_callback = callback;
}

auto ADCStreamClient::removeCallback() -> void {
    const std::lock_guard lock(m_pimpl->m_smutex);
    m_pimpl->m_callback = nullptr;
    m_pimpl->m_configClient->removeCallback(m_pimpl->m_configCallback);
}

auto ADCStreamClient::startStreaming() -> bool {

    std::list<std::string> connected_hosts;
    std::list<std::string> slaveHosts;
    std::list<std::string> masterHosts;

    connected_hosts = m_pimpl->m_configClient->getHosts();
    for (auto& host : connected_hosts) {
        switch (m_pimpl->m_configClient->getModeByHost(host)) {
            case ConfigStreamClient::AB_SERVER_MASTER:
                masterHosts.push_back(host);
                break;
            case ConfigStreamClient::AB_SERVER_SLAVE:
                slaveHosts.push_back(host);
                break;
            default:
                break;
        }
    }
    if (!requestStopStreamingCommon(m_pimpl->m_configClient, masterHosts, slaveHosts, m_pimpl->m_verbose)) {
        aprintf(stderr, "%s Can't stop streaming on remote machines\n", getTS(": ").c_str());
        return false;
    }

    std::map<std::string, uint32_t> blockSizes;
    auto hosts = m_pimpl->m_configClient->getHosts();
    if (!requestMemoryBlockSizeCommon(m_pimpl->m_configClient, hosts, &blockSizes, m_pimpl->m_verbose)) {
        aprintf(stderr, "%s Can't get block sizes\n", getTS(": ").c_str());
        return false;
    }

    std::map<std::string, uint32_t> activeChannels;
    if (!requestActiveChannelsCommon(m_pimpl->m_configClient, hosts, &activeChannels, m_pimpl->m_verbose)) {
        aprintf(stderr, "%s Can't get active channels\n", getTS(": ").c_str());
        return false;
    }

    std::map<string, StateRunningHosts> runned_hosts;
    stopStreaming();

    if (requestStartStreamingCommon(m_pimpl->m_configClient, masterHosts, slaveHosts, &runned_hosts, m_pimpl->m_verbose)) {
        m_pimpl->m_runClientCounter = runned_hosts.size();
        for (auto kv : runned_hosts) {
            if (kv.second == StateRunningHosts::TCP && activeChannels[kv.first] > 0)
                m_pimpl->clients.push_back(new std::thread(&ADCStreamClient::Impl::runClient, m_pimpl, this, kv.first, blockSizes[kv.first], activeChannels[kv.first]));
            else {
                m_pimpl->m_runClientCounter--;
            }
        }

        auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
        auto timeout = false;
        while (!timeout && m_pimpl->m_runClientCounter > 0) {
            sleepMs(100);
            timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime > 5000);
        }

        std::vector<string> runnedThreads;
        for (const auto& [key, _] : runned_hosts) {
            runnedThreads.push_back(key);
        }

        if (!requestStartADCCommon(m_pimpl->m_configClient, masterHosts, slaveHosts, &runned_hosts, m_pimpl->m_verbose)) {
            aprintf(stdout, "%s Can't start ADC on remote machines\n", getTS(": ").c_str());
            for (auto& h : runnedThreads) {
                if (runned_hosts.count(h) == 0)
                    m_pimpl->m_terminate[h] = true;
            }
        }
    }
    return true;
}

auto ADCStreamClient::stopStreaming() -> void {
    for (auto& kv : m_pimpl->m_terminate) {
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

auto ADCStreamClient::wait() -> void {
    for (auto t : m_pimpl->clients) {
        if (t->joinable()) {
            t->join();
        }
    }
}

auto ADCStreamClient::notifyStop() -> void {
    for (auto& kv : m_pimpl->m_terminate) {
        kv.second = true;
    }
}

auto ADCStreamClient::notifyStop(std::string host) -> void {
    for (auto& kv : m_pimpl->m_terminate) {
        if (host == kv.first)
            kv.second = true;
    }
}

auto ADCStreamClient::setVerbose(bool enable) -> void {
    m_pimpl->m_verbose = enable;
}
