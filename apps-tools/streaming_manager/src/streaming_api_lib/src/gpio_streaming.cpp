#include "gpio_streaming.h"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <mutex>
#include <thread>

#include "callbacks.h"
#include "common.h"
#include "config_streaming.h"
#include "data_lib/buffers_cached.h"
#include "data_lib/network_header.h"
#include "logger_lib/file_logger.h"
#include "net_lib/asio_net.h"
#include "uio_lib/memory_manager.h"

namespace {

// A ring of one channel of the board's block size in memory of its own
struct Ring {
    uio_lib::CMemoryManager memory;
    DataLib::CBuffersCached::Ptr buffers = DataLib::CBuffersCached::create();

    explicit Ring(uint32_t blockSize) {
        memory.setMemoryBlockSize(blockSize);
        memory.reallocateBlocks();
        memory.reserveMemory(uio_lib::MM_GPIO, memory.getFreeBlockCount(), 1);
        adc_channels_t one;
        one[ADCChannels::ADC_CH1] = true;
        buffers->generateBuffersEmptyADC(one, memory.getRegions(uio_lib::MM_GPIO), DataLib::sizeHeader());
    }
};

}  // namespace

class GPIOCb : public ConfigCallback {
    void configError(ConfigStreamClient*, std::string, int) override { *m_terminate = true; }
    void gpioServerStopped(ConfigStreamClient*, std::string) override { *m_terminate = true; }
    void gpioServerStoppedMemError(ConfigStreamClient*, std::string) override { *m_terminate = true; }
    void gpioServerStoppedConfigError(ConfigStreamClient*, std::string) override { *m_terminate = true; }

    std::atomic<bool>* m_terminate;

   public:
    explicit GPIOCb(std::atomic<bool>* terminate) : m_terminate(terminate) {}
};

struct GPIOStreamClient::Impl {
    std::shared_ptr<ConfigStreamClient> m_configClient;
    std::shared_ptr<GPIOCb> m_configCallback;
    std::shared_ptr<GPIOCallback> m_callback;
    std::mutex m_smutex;
    bool m_verbose = false;
    std::string m_host;
    std::atomic<bool> m_terminate{false};
    std::vector<uint32_t> m_words;
    uint64_t m_repeat = 1;
    bool m_memory = false;
    std::thread m_input;
    std::thread m_output;

    auto runInput(GPIOStreamClient* client, uint32_t blockSize) -> void;
    auto runOutput(GPIOStreamClient* client, uint32_t blockSize) -> void;
    auto join() -> void;
};

auto GPIOStreamClient::Impl::runInput(GPIOStreamClient* client, uint32_t blockSize) -> void {
    Ring ring(blockSize);
    auto net = net_lib::CAsioNet::create(net_lib::M_CLIENT, m_host, NET_GPIO_IN_STREAMING_PORT, ring.buffers);
    net->setPackID(DataLib::ID_PACK_GPIO);
    net->clientConnectNotify.connect([&](std::string host) {
        const std::lock_guard lock(m_smutex);
        if (m_verbose)
            aprintf(stdout, "%s GPIO input connected %s\n", getTS(": ").c_str(), host.c_str());
        if (m_callback)
            m_callback->connected(client, host);
    });
    net->clientDisconnectNotify.connect([&](std::string host) {
        const std::lock_guard lock(m_smutex);
        if (m_callback)
            m_callback->disconnected(client, host);
    });
    net->clientErrorNotify.connect([&](std::error_code err) {
        const std::lock_guard lock(m_smutex);
        if (m_terminate)
            return;  // the connection closed by the stop
        if (m_verbose)
            aprintf(stdout, "%s GPIO input error %s %s\n", getTS(": ").c_str(), m_host.c_str(), err.message().c_str());
        if (m_callback)
            m_callback->error(client, m_host, err.value());
        m_terminate = true;
    });
    net->reciveNotify.connect([&](std::error_code error, DataLib::CDataBuffersPackDMA::Ptr pack) {
        const std::lock_guard lock(m_smutex);
        if (!error && m_callback) {
            auto buffer = pack->getBuffer(DataLib::EDataBuffersPackChannel::CH1);
            DataLib::GPIOHeader h;
            GPIOPack out;
            if (DataLib::getHeaderGPIO(buffer, h, &out.packId)) {
                out.host = m_host;
                out.fpgaLost = h.lostFPGA;
                out.fpgaWait = h.waitFPGA;
                out.decimation = h.decimation;
                out.baseRate = h.gpioClock;
                out.triggerCount = h.triggerCount;
                out.triggerBlocks = h.triggerBlocks;
                out.rle = h.flags & DataLib::GPIO_FLAG_RLE;
                out.triggerMode = h.flags & DataLib::GPIO_FLAG_TRIGGER_MODE;
                auto words = std::min<size_t>(h.dataSize, buffer->getDataLenght()) / 4;
                auto data = static_cast<const uint32_t*>(buffer->getMappedDataMemory());
                out.words.assign(data, data + words);
                m_callback->receivePack(client, out);
            }
        }
        ring.buffers->unlockBufferRead();
    });
    net->start();
    while (!m_terminate)
        sleepMs(100);
    net->stop();
}

// The words go out in buffers of the board's block size; the last one says how much of it holds
// words. The connection stays until the stop: the board keeps the lines of the last word.
auto GPIOStreamClient::Impl::runOutput(GPIOStreamClient* client, uint32_t blockSize) -> void {
    Ring ring(blockSize);
    std::atomic<bool> connected{false};
    auto net = net_lib::CAsioNet::create(net_lib::M_CLIENT, m_host, NET_GPIO_OUT_STREAMING_PORT, nullptr);
    net->clientConnectNotify.connect([&](std::string host) {
        const std::lock_guard lock(m_smutex);
        if (m_callback)
            m_callback->connected(client, host);
        connected = true;
    });
    net->clientDisconnectNotify.connect([&](std::string host) {
        const std::lock_guard lock(m_smutex);
        if (m_callback)
            m_callback->disconnected(client, host);
        m_terminate = true;
    });
    net->clientErrorNotify.connect([&](std::error_code err) {
        const std::lock_guard lock(m_smutex);
        if (m_terminate)
            return;  // the connection closed by the stop
        if (m_callback)
            m_callback->error(client, m_host, err.value());
        m_terminate = true;
    });
    net->start();
    for (int i = 0; i < 500 && !connected && !m_terminate; i++)
        sleepMs(10);

    auto pack = ring.buffers->writeBuffer(true);
    auto buffer = pack ? pack->getBuffer(DataLib::EDataBuffersPackChannel::CH1) : nullptr;
    const size_t maxWords = buffer ? buffer->getDataLenght() / 4 : 0;
    auto data = buffer ? static_cast<uint32_t*>(buffer->getMappedDataMemory()) : nullptr;
    uint64_t id = 0;
    size_t position = 0;  // memory mode: the next word
    uint64_t played = 0;  // memory mode: the repeats done
    while (connected && !m_terminate && buffer) {
        size_t count = 0;
        if (m_memory) {
            while (count < maxWords && !m_words.empty() && (m_repeat == 0 || played < m_repeat)) {
                auto n = std::min(maxWords - count, m_words.size() - position);
                std::memcpy(data + count, m_words.data() + position, n * 4);
                count += n;
                position += n;
                if (position == m_words.size()) {
                    position = 0;
                    played++;
                }
            }
        } else {
            std::vector<uint32_t> words;
            {
                const std::lock_guard lock(m_smutex);
                if (m_callback)
                    words = m_callback->outputData(client, maxWords);
            }
            count = std::min(words.size(), maxWords);
            std::memcpy(data, words.data(), count * 4);
        }
        if (count == 0)
            break;
        DataLib::GPIOHeader h;
        h.dataSize = count * 4;
        DataLib::setHeaderGPIO(buffer, id++, h);
        if (!net->sendSyncData(pack))
            break;
        const std::lock_guard lock(m_smutex);
        if (m_callback)
            m_callback->sentPack(client, count);
    }
    while (connected && !m_terminate)
        sleepMs(100);
    net->stop();
}

auto GPIOStreamClient::Impl::join() -> void {
    for (auto t : {&m_input, &m_output})
        if (t->joinable())
            t->join();
}

GPIOStreamClient::GPIOStreamClient(std::shared_ptr<ConfigStreamClient> configClient) {
    m_pimpl = new Impl();
    m_pimpl->m_configClient = configClient;
    m_pimpl->m_configCallback = std::make_shared<GPIOCb>(&m_pimpl->m_terminate);
    m_pimpl->m_configClient->addCallback(m_pimpl->m_configCallback);
}

GPIOStreamClient::~GPIOStreamClient() {
    m_pimpl->m_terminate = true;
    m_pimpl->join();
    removeCallback();
    m_pimpl->m_configClient->removeCallback(m_pimpl->m_configCallback);
    delete m_pimpl;
}

auto GPIOStreamClient::setOutputData(std::vector<uint32_t> words, uint64_t repeat) -> void {
    m_pimpl->m_words = std::move(words);
    m_pimpl->m_repeat = repeat;
    m_pimpl->m_memory = true;
}

auto GPIOStreamClient::clearOutputData() -> void {
    m_pimpl->m_words.clear();
    m_pimpl->m_memory = false;
}

auto GPIOStreamClient::startStreaming(std::string host, bool input, bool output) -> bool {
    auto& p = *m_pimpl;
    p.m_terminate = true;
    p.join();
    p.m_terminate = false;
    p.m_host = host;
    if (!input && !output)
        return false;
    if (!p.m_configClient->sendConfig(host, "gpio_in_streaming", input ? "ON" : "OFF") ||
        !p.m_configClient->sendConfig(host, "gpio_out_streaming", output ? "ON" : "OFF")) {
        aprintf(stderr, "%s Can't set the GPIO directions of %s\n", getTS(": ").c_str(), host.c_str());
        return false;
    }
    std::map<std::string, uint32_t> blockSizes;
    if (!requestMemoryBlockSizeCommon(p.m_configClient, {host}, &blockSizes, p.m_verbose)) {
        aprintf(stderr, "%s Can't get block sizes\n", getTS(": ").c_str());
        return false;
    }
    if (!requestStartGPIOStreamingCommon(p.m_configClient, host, p.m_verbose)) {
        aprintf(stderr, "%s Can't start the GPIO streaming of %s\n", getTS(": ").c_str(), host.c_str());
        return false;
    }
    p.m_terminate = false;  // a stop reply of an earlier session may have come with the start
    if (input)
        p.m_input = std::thread(&Impl::runInput, m_pimpl, this, blockSizes[host]);
    if (output)
        p.m_output = std::thread(&Impl::runOutput, m_pimpl, this, blockSizes[host]);
    return true;
}

// The client lets its connections go first: the board's stop closes them from its side
auto GPIOStreamClient::stopStreaming() -> void {
    m_pimpl->m_terminate = true;
    m_pimpl->join();
    if (!m_pimpl->m_host.empty())
        m_pimpl->m_configClient->requestGPIOServerStop(m_pimpl->m_host);
}

auto GPIOStreamClient::wait() -> void {
    m_pimpl->join();
}

auto GPIOStreamClient::notifyStop() -> void {
    m_pimpl->m_terminate = true;
}

auto GPIOStreamClient::setVerbose(bool enable) -> void {
    m_pimpl->m_verbose = enable;
}

auto GPIOStreamClient::setCallback(std::shared_ptr<GPIOCallback> callback) -> void {
    const std::lock_guard lock(m_pimpl->m_smutex);
    m_pimpl->m_callback = callback;
}

auto GPIOStreamClient::removeCallback() -> void {
    const std::lock_guard lock(m_pimpl->m_smutex);
    m_pimpl->m_callback = nullptr;
}
