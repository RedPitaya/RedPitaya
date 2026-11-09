#include "gpio_streaming.h"

#include <algorithm>
#include <atomic>
#include <chrono>
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

    Ring(uint32_t blockSize, uio_lib::MemoryTAG tag) {
        memory.setMemoryBlockSize(blockSize);
        memory.reallocateBlocks();
        memory.reserveMemory(tag, memory.getFreeBlockCount(), 1);
        adc_channels_t one;
        one[ADCChannels::ADC_CH1] = true;
        buffers->generateBuffersEmptyADC(one, memory.getRegions(tag), DataLib::sizeHeader());
    }
};

}  // namespace

struct GPIOStreamClient::Impl {
    GPIOStreamClient* owner = nullptr;
    std::shared_ptr<ConfigStreamClient> m_configClient;
    std::shared_ptr<ConfigCallback> m_configCallback;
    std::shared_ptr<GPIOCallback> m_callback;
    std::mutex m_smutex;
    bool m_verbose = false;
    std::string m_inHost;
    std::string m_outHost;
    std::atomic<bool> m_inTerminate{true};
    std::atomic<bool> m_outTerminate{true};
    std::atomic<bool> m_inStopped{false};   // the board answered the stop of the input
    std::atomic<bool> m_outStopped{false};  // the board answered the stop of the output
    std::atomic<uint64_t> m_outWait{0};
    std::atomic<bool> m_inFile{false};   // the board writes the input to a file of its own
    std::atomic<bool> m_outFile{false};  // the board plays a file of its own

    std::vector<uint32_t> m_words;
    uint64_t m_repeat = 1;
    bool m_memory = false;
    std::thread m_input;
    std::thread m_output;

    auto runInput(uint32_t blockSize) -> void;
    auto runOutput(uint32_t blockSize) -> void;
    // the file mode: no data connection, the direction lasts until the board ends it
    auto runUntil(std::atomic<bool>& terminate) -> void;
    auto joinInput() -> void;
    auto joinOutput() -> void;

    class ConfigCb;
};

// The board's answers that end a direction
class GPIOStreamClient::Impl::ConfigCb : public ConfigCallback {
    void configError(ConfigStreamClient*, std::string, int) override {
        m_p->m_inTerminate = true;
        m_p->m_outTerminate = true;
    }
    void gpioInServerStopped(ConfigStreamClient*, std::string host) override { endInput(host); }
    void gpioInServerStoppedMemError(ConfigStreamClient*, std::string host) override { endInput(host); }
    void gpioInServerStoppedConfigError(ConfigStreamClient*, std::string host) override { endInput(host); }
    void gpioOutServerStopped(ConfigStreamClient*, std::string host, uint64_t waitClocks) override { endOutput(host, waitClocks); }
    void gpioOutServerStoppedMemError(ConfigStreamClient*, std::string host) override { endOutput(host, 0); }
    void gpioOutServerStoppedConfigError(ConfigStreamClient*, std::string host) override { endOutput(host, 0); }
    void gpioInServerStoppedSDFull(ConfigStreamClient*, std::string host) override { endInputFile(host, GPIO_FILE_SD_FULL); }
    void gpioInServerStoppedSDDone(ConfigStreamClient*, std::string host) override { endInputFile(host, GPIO_FILE_DONE); }
    void gpioInServerStoppedSDBroken(ConfigStreamClient*, std::string host) override { endInputFile(host, GPIO_FILE_BROKEN); }
    void gpioOutServerStoppedSDDone(ConfigStreamClient*, std::string host, uint64_t waitClocks) override { endOutputFile(host, GPIO_FILE_DONE, waitClocks); }
    void gpioOutServerStoppedSDEmpty(ConfigStreamClient*, std::string host) override { endOutputFile(host, GPIO_FILE_EMPTY, 0); }
    void gpioOutServerStoppedSDBroken(ConfigStreamClient*, std::string host) override { endOutputFile(host, GPIO_FILE_BROKEN, 0); }
    void gpioOutServerStoppedSDMissing(ConfigStreamClient*, std::string host) override { endOutputFile(host, GPIO_FILE_MISSING, 0); }

    auto endInput(const std::string& host) -> void {
        if (host != m_p->m_inHost)
            return;
        m_p->m_inTerminate = true;
        m_p->m_inStopped = true;
    }

    auto endInputFile(const std::string& host, GPIOFileStop reason) -> void {
        if (host != m_p->m_inHost)
            return;
        m_p->m_inTerminate = true;
        m_p->m_inStopped = true;
        const std::lock_guard lock(m_p->m_smutex);
        if (m_p->m_callback)
            m_p->m_callback->inputFileStopped(m_p->owner, host, reason);
    }

    auto endOutputFile(const std::string& host, GPIOFileStop reason, uint64_t waitClocks) -> void {
        if (host != m_p->m_outHost)
            return;
        m_p->m_outWait = waitClocks;
        m_p->m_outTerminate = true;
        m_p->m_outStopped = true;
        const std::lock_guard lock(m_p->m_smutex);
        if (m_p->m_callback)
            m_p->m_callback->outputFileStopped(m_p->owner, host, reason, waitClocks);
    }

    auto endOutput(const std::string& host, uint64_t waitClocks) -> void {
        if (host != m_p->m_outHost)
            return;
        m_p->m_outWait = waitClocks;
        m_p->m_outTerminate = true;
        m_p->m_outStopped = true;
        const std::lock_guard lock(m_p->m_smutex);
        if (m_p->m_callback)
            m_p->m_callback->outputStopped(m_p->owner, host, waitClocks);
    }

    GPIOStreamClient::Impl* m_p;

   public:
    explicit ConfigCb(GPIOStreamClient::Impl* p) : m_p(p) {}
};

auto GPIOStreamClient::Impl::runInput(uint32_t blockSize) -> void {
    Ring ring(blockSize, uio_lib::MM_GPIO_IN);
    auto net = net_lib::CAsioNet::create(net_lib::M_CLIENT, m_inHost, NET_GPIO_IN_STREAMING_PORT, ring.buffers);
    net->setPackID(DataLib::ID_PACK_GPIO);
    net->clientConnectNotify.connect([&](std::string host) {
        const std::lock_guard lock(m_smutex);
        if (m_verbose)
            aprintf(stdout, "%s GPIO input connected %s\n", getTS(": ").c_str(), host.c_str());
        if (m_callback)
            m_callback->connected(owner, host);
    });
    net->clientDisconnectNotify.connect([&](std::string host) {
        const std::lock_guard lock(m_smutex);
        if (m_callback)
            m_callback->disconnected(owner, host);
    });
    net->clientErrorNotify.connect([&](std::error_code err) {
        const std::lock_guard lock(m_smutex);
        if (m_inTerminate)
            return;  // the connection closed by the stop
        if (m_verbose)
            aprintf(stdout, "%s GPIO input error %s %s\n", getTS(": ").c_str(), m_inHost.c_str(), err.message().c_str());
        if (m_callback)
            m_callback->error(owner, m_inHost, err.value());
        m_inTerminate = true;
    });
    net->reciveNotify.connect([&](std::error_code error, DataLib::CDataBuffersPackDMA::Ptr pack) {
        const std::lock_guard lock(m_smutex);
        if (!error && m_callback) {
            auto buffer = pack->getBuffer(DataLib::EDataBuffersPackChannel::CH1);
            DataLib::GPIOHeader h;
            GPIOPack out;
            if (DataLib::getHeaderGPIO(buffer, h, &out.packId)) {
                out.host = m_inHost;
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
                m_callback->receivePack(owner, out);
            }
        }
        ring.buffers->unlockBufferRead();
    });
    net->start();
    while (!m_inTerminate)
        sleepMs(100);
    net->stop();
}

// The words go out in buffers of the board's block size; the last one says how much of it holds
// words. The connection stays until the stop: the board keeps the lines of the last word.
auto GPIOStreamClient::Impl::runOutput(uint32_t blockSize) -> void {
    Ring ring(blockSize, uio_lib::MM_GPIO_OUT);
    std::atomic<bool> connected{false};
    auto net = net_lib::CAsioNet::create(net_lib::M_CLIENT, m_outHost, NET_GPIO_OUT_STREAMING_PORT, nullptr);
    net->clientConnectNotify.connect([&](std::string host) {
        const std::lock_guard lock(m_smutex);
        if (m_callback)
            m_callback->connected(owner, host);
        connected = true;
    });
    net->clientDisconnectNotify.connect([&](std::string host) {
        const std::lock_guard lock(m_smutex);
        if (m_callback)
            m_callback->disconnected(owner, host);
        m_outTerminate = true;
    });
    net->clientErrorNotify.connect([&](std::error_code err) {
        const std::lock_guard lock(m_smutex);
        if (m_outTerminate)
            return;  // the connection closed by the stop
        if (m_callback)
            m_callback->error(owner, m_outHost, err.value());
        m_outTerminate = true;
    });
    net->start();
    for (int i = 0; i < 500 && !connected && !m_outTerminate; i++)
        sleepMs(10);

    auto pack = ring.buffers->writeBuffer(true);
    auto buffer = pack ? pack->getBuffer(DataLib::EDataBuffersPackChannel::CH1) : nullptr;
    const size_t maxWords = buffer ? buffer->getDataLenght() / 4 : 0;
    auto data = buffer ? static_cast<uint32_t*>(buffer->getMappedDataMemory()) : nullptr;
    uint64_t id = 0;
    size_t position = 0;  // memory mode: the next word
    uint64_t played = 0;  // memory mode: the repeats done
    while (connected && !m_outTerminate && buffer) {
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
                    words = m_callback->outputData(owner, maxWords);
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
            m_callback->sentPack(owner, count);
    }
    while (connected && !m_outTerminate)
        sleepMs(100);
    net->stop();
}

auto GPIOStreamClient::Impl::runUntil(std::atomic<bool>& terminate) -> void {
    while (!terminate)
        sleepMs(100);
}

auto GPIOStreamClient::Impl::joinInput() -> void {
    if (m_input.joinable())
        m_input.join();
}

auto GPIOStreamClient::Impl::joinOutput() -> void {
    if (m_output.joinable())
        m_output.join();
}

GPIOStreamClient::GPIOStreamClient(std::shared_ptr<ConfigStreamClient> configClient) {
    m_pimpl = new Impl();
    m_pimpl->owner = this;
    m_pimpl->m_configClient = configClient;
    m_pimpl->m_configCallback = std::make_shared<Impl::ConfigCb>(m_pimpl);
    m_pimpl->m_configClient->addCallback(m_pimpl->m_configCallback);
}

GPIOStreamClient::~GPIOStreamClient() {
    m_pimpl->m_inTerminate = true;
    m_pimpl->m_outTerminate = true;
    m_pimpl->joinInput();
    m_pimpl->joinOutput();
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

auto GPIOStreamClient::startInput(std::string host) -> bool {
    auto& p = *m_pimpl;
    p.m_inTerminate = true;
    p.joinInput();
    p.m_inHost = host;
    std::map<std::string, uint32_t> blockSizes;
    if (!requestMemoryBlockSizeCommon(p.m_configClient, {host}, &blockSizes, p.m_verbose)) {
        aprintf(stderr, "%s Can't get block sizes\n", getTS(": ").c_str());
        return false;
    }
    bool toFile = false;
    if (!requestStartGPIOStreamingCommon(p.m_configClient, host, false, p.m_verbose, &toFile)) {
        aprintf(stderr, "%s Can't start the GPIO input of %s\n", getTS(": ").c_str(), host.c_str());
        return false;
    }
    p.m_inFile = toFile;
    p.m_inStopped = false;
    p.m_inTerminate = false;  // a stop reply of an earlier session may have come with the start
    if (toFile)
        p.m_input = std::thread(&Impl::runUntil, m_pimpl, std::ref(p.m_inTerminate));
    else
        p.m_input = std::thread(&Impl::runInput, m_pimpl, blockSizes[host]);
    return true;
}

auto GPIOStreamClient::startOutput(std::string host) -> bool {
    auto& p = *m_pimpl;
    p.m_outTerminate = true;
    p.joinOutput();
    p.m_outHost = host;
    std::map<std::string, uint32_t> blockSizes;
    if (!requestMemoryBlockSizeCommon(p.m_configClient, {host}, &blockSizes, p.m_verbose)) {
        aprintf(stderr, "%s Can't get block sizes\n", getTS(": ").c_str());
        return false;
    }
    bool toFile = false;
    if (!requestStartGPIOStreamingCommon(p.m_configClient, host, true, p.m_verbose, &toFile)) {
        aprintf(stderr, "%s Can't start the GPIO output of %s\n", getTS(": ").c_str(), host.c_str());
        return false;
    }
    p.m_outFile = toFile;
    p.m_outTerminate = false;
    p.m_outStopped = false;
    p.m_outWait = 0;
    if (toFile)
        p.m_output = std::thread(&Impl::runUntil, m_pimpl, std::ref(p.m_outTerminate));
    else
        p.m_output = std::thread(&Impl::runOutput, m_pimpl, blockSizes[host]);
    return true;
}

// The client lets its connection go first: the board's stop closes it from its side. The board
// stops the input in the background (a file is closed first) and answers then: waited for here up
// to 3 s, so that a late answer is not taken for the answer to the next start.
auto GPIOStreamClient::stopInput() -> void {
    auto& p = *m_pimpl;
    p.m_inTerminate = true;
    p.joinInput();
    if (p.m_inHost.empty())
        return;
    p.m_inStopped = false;
    if (p.m_configClient->requestGPIOInServerStop(p.m_inHost)) {
        for (int i = 0; i < 300 && !p.m_inStopped; i++)
            sleepMs(10);
    }
}

// The board answers the stop with the clocks its output waited: waited for here up to 3 s
auto GPIOStreamClient::stopOutput() -> void {
    auto& p = *m_pimpl;
    p.m_outTerminate = true;
    p.joinOutput();
    if (p.m_outHost.empty())
        return;
    p.m_outStopped = false;
    if (p.m_configClient->requestGPIOOutServerStop(p.m_outHost)) {
        for (int i = 0; i < 300 && !p.m_outStopped; i++)
            sleepMs(10);
    }
}

auto GPIOStreamClient::stopStreaming() -> void {
    stopOutput();
    stopInput();
}

auto GPIOStreamClient::inputToFile() -> bool {
    return m_pimpl->m_inFile;
}

auto GPIOStreamClient::outputFromFile() -> bool {
    return m_pimpl->m_outFile;
}

auto GPIOStreamClient::outputWaitClocks() -> uint64_t {
    return m_pimpl->m_outWait;
}

auto GPIOStreamClient::wait() -> void {
    m_pimpl->joinInput();
    m_pimpl->joinOutput();
}

auto GPIOStreamClient::notifyStop() -> void {
    m_pimpl->m_inTerminate = true;
    m_pimpl->m_outTerminate = true;
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
