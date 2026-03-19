#include "dac_streaming.h"
#include <csignal>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include "callbacks.h"
#include "common.h"
#include "config.h"
#include "config_net_lib/client_net_config_manager.h"
#include "dac_streaming_lib/dac_streaming_manager.h"
#include "data_lib/buffers_cached.h"
#include "data_lib/neon_asm.h"
#include "logger_lib/file_logger.h"
#include "net_lib/asio_net.h"
#include "uio_lib/memory_manager.h"

using namespace dac_streaming_lib;

enum dataMode { WAV_MODE = 0, TDMS_MODE = 1, MEM_MODE = 2, MEM_STREAM_MODE = 3 };

class DACCb : public ConfigCallback {

    void configError(ConfigStreamClient* cl, std::string host, int error) override {
        if (m_terminate)
            *m_terminate = true;
    }

    void dacServerStoppedMemError(ConfigStreamClient*, std::string host) override {
        if (m_terminate)
            *m_terminate = true;
    }
    void dacServerStoppedMemModify(ConfigStreamClient*, std::string host) override {
        if (m_terminate)
            *m_terminate = true;
    }
    void dacServerStoppedConfigError(ConfigStreamClient*, std::string host) override {
        if (m_terminate)
            *m_terminate = true;
    }
    void configErrorFileMissed(ConfigStreamClient*, std::string host) override {
        if (m_terminate)
            *m_terminate = true;
    }

    bool* m_terminate = nullptr;

   public:
    explicit DACCb(bool* terminate) : m_terminate(terminate){};
};

struct DACStreamClient::Impl {
    std::string m_host = "";
    CDACStreamingManager::Ptr m_dac_manger;
    net_lib::CAsioNet::Ptr m_dac_asionet;
    std::shared_ptr<ConfigStreamClient> m_configClient = nullptr;
    bool m_verbose = false;
    std::shared_ptr<DACCallback> m_callback = nullptr;
    bool m_terminate;
    bool m_fileEnded;
    std::mutex m_smutex;
    std::atomic<int> m_runClientCounter;
    uint64_t m_repeatCount = 1;
    bool m_dac_connected;
    bool m_repeatInf = false;
    dataMode m_dataMode;
    std::string m_fileName = "";
    uint8_t* m_memChannels[2] = {nullptr, nullptr};
    uint64_t m_memSize[2] = {0, 0};
    uint8_t m_memBytes[2] = {0, 0};
    CReaderController::dac_channels_t m_memSinkChannels;
    auto getActiveChannels() -> CReaderController::dac_channels_t;
    std::thread* m_client;
    auto runClient(DACStreamClient* client, std::string host, uint32_t size, CReaderController::dac_channels_t activeChannels) -> void;
    std::shared_ptr<DACCb> m_configCallback;
};

auto DACStreamClient::Impl::getActiveChannels() -> CReaderController::dac_channels_t {
    CReaderController::dac_channels_t channels;
    auto file_type = CDACStreamingManager::WAV_TYPE;
    switch (m_dataMode) {
        case dataMode::TDMS_MODE:
            file_type = CDACStreamingManager::TDMS_TYPE;
            break;
        case dataMode::WAV_MODE:
            file_type = CDACStreamingManager::WAV_TYPE;
            break;
        case dataMode::MEM_MODE: {
            for (int i = 0; i < 2; i++) {
                if (m_memChannels[i] != nullptr)
                    channels.enable((DACChannels)i);
            }
            return channels;
        }
        case dataMode::MEM_STREAM_MODE: {
            return m_memSinkChannels;
        }
        default:
            return channels;
    }
    auto x = CDACStreamingManager::Create(file_type, m_fileName, CStreamSettings::DACRepeat::DAC_REP_OFF, 0, 0, false);
    bool chActive[2] = {false, false};
    if (!x->getChannels(&chActive[0], &chActive[1])) {
        return channels;
    }
    for (int i = 0; i < 2; i++) {
        if (chActive[i])
            channels.enable((DACChannels)i);
    }
    return channels;
}

auto DACStreamClient::Impl::runClient(DACStreamClient* client, std::string host, uint32_t size, CReaderController::dac_channels_t activeChannels) -> void {
    auto memoryManager = new uio_lib::CMemoryManager();
    memoryManager->setMemoryBlockSize(size);
    memoryManager->reallocateBlocks();
    auto blocks = memoryManager->getFreeBlockCount();

    m_terminate = false;
    m_fileEnded = false;

    auto file_type = CDACStreamingManager::WAV_TYPE;
    if (m_dataMode == WAV_MODE) {
        file_type = CDACStreamingManager::WAV_TYPE;
    }

    if (m_dataMode == TDMS_MODE) {
        file_type = CDACStreamingManager::TDMS_TYPE;
    }

    m_repeatCount = m_repeatCount == 0 ? 1 : m_repeatCount;
    auto rep_mode = m_repeatInf ? CStreamSettings::DACRepeat::DAC_REP_INF : CStreamSettings::DACRepeat::DAC_REP_ON;
    switch (m_dataMode) {
        case MEM_MODE: {
            m_dac_manger = CDACStreamingManager::Create(m_memChannels, m_memSize, std::max(m_memBytes[0], m_memBytes[1]), rep_mode, m_repeatCount, size, m_verbose);
            break;
        }
        case MEM_STREAM_MODE: {
            m_dac_manger =
                CDACStreamingManager::Create(activeChannels,
                                             std::max(m_memBytes[0], m_memBytes[1]),
                                             size,
                                             m_verbose,
                                             [&](uint8_t bitsBySample, std::array<uint8_t*, CReaderController::MAX_CHANNES>& buffers, const uint32_t bufferSize) -> bool {
                                                 const std::lock_guard lock(m_smutex);
                                                 if (m_callback) {
                                                     if (bitsBySample == 8) {
                                                         return m_callback->streamData8Bit(client, (int8_t*)buffers[0], (int8_t*)buffers[1], bufferSize);
                                                     } else if (bitsBySample == 16) {
                                                         return m_callback->streamData16Bit(client, (int16_t*)buffers[0], (int16_t*)buffers[1], bufferSize / 2);
                                                     } else {
                                                         ERROR_LOG("Incorrect data type size")
                                                     }
                                                 }
                                                 return true;  // Return end of stream
                                             });
            break;
        }
        default:
            m_dac_manger = CDACStreamingManager::Create(file_type, m_fileName, rep_mode, m_repeatCount, size, m_verbose);
    }

    auto reserved __attribute__((unused)) = memoryManager->reserveMemory(uio_lib::MM_DAC, blocks, activeChannels.count());
    m_dac_manger->getBufferManager()->generateBuffersEmpty(activeChannels.count(), memoryManager->getRegions(uio_lib::MM_DAC), DataLib::sizeHeader());
    m_dac_manger->getBufferManager()->initHeadersDAC(activeChannels.count());
    TRACE_SHORT("Reserved blocks %d", reserved)

    m_dac_manger->notifyStop.connect([&](CDACStreamingManager::NotifyResult res) {
        const std::lock_guard lock(m_smutex);
        switch (res) {
            case CDACStreamingManager::NotifyResult::NR_BROKEN: {
                if (m_verbose)
                    aprintf(stdout, "%s File %s is broken\n", getTS(": ").c_str(), m_fileName.c_str());
                if (m_callback)
                    m_callback->stoppedFileBroken(client, m_host);
                break;
            }
            case CDACStreamingManager::NotifyResult::NR_EMPTY: {
                if (m_verbose)
                    aprintf(stdout, "%s File %s is empty\n", getTS(": ").c_str(), m_fileName.c_str());
                if (m_callback)
                    m_callback->stoppedEmpty(client, m_host);
                break;
            }
            case CDACStreamingManager::NotifyResult::NR_MISSING_FILE: {
                if (m_verbose)
                    aprintf(stdout, "%s File %s is missing\n", getTS(": ").c_str(), m_fileName.c_str());
                if (m_callback)
                    m_callback->stoppedMissingFile(client, m_host);
                break;
            }
            case CDACStreamingManager::NotifyResult::NR_STOP: {
                if (m_verbose)
                    aprintf(stdout, "%s Got stop command from data controller\n", getTS(": ").c_str());
                if (m_callback)
                    m_callback->stopped(client, m_host);
                break;
            }
            case CDACStreamingManager::NotifyResult::NR_ENDED: {
                if (m_verbose)
                    aprintf(stdout, "%s All data from the file has been read: %s\n", getTS(": ").c_str(), m_fileName.c_str());
                if (m_callback)
                    m_callback->stoppedFileEnd(client, m_host);
                m_fileEnded = true;
                break;
            }
            default:
                break;
        }
        m_terminate = true;
    });

    m_dac_connected = false;
    m_dac_asionet = net_lib::CAsioNet::create(net_lib::EMode::M_CLIENT, m_host, NET_DAC_STREAMING_PORT, nullptr);
    m_dac_asionet->clientConnectNotify.connect([&](std::string host) {
        const std::lock_guard lock(m_smutex);
        if (m_verbose)
            aprintf(stdout, "%s CLIENT CONNECTED  %s\n", getTS(": ").c_str(), host.c_str());
        if (m_callback)
            m_callback->connected(client, host);
        m_dac_connected = true;
    });

    m_dac_asionet->clientDisconnectNotify.connect([&](std::string host) {
        const std::lock_guard lock(m_smutex);
        if (m_dac_connected && m_verbose)
            aprintf(stdout, "%s CLIENT DISCONNECTED  %s\n", getTS(": ").c_str(), host.c_str());
        if (m_callback)
            m_callback->disconnected(client, host);
        m_terminate = false;
    });

    m_dac_asionet->sendNotify.connect([&](error_code err, size_t) {
        if (err.value() != 0 && m_terminate == false) {
            aprintf(stdout, "%s SEND ERROR  %s\n", getTS(": ").c_str(), m_fileName.c_str());
            if (m_callback)
                m_callback->error(client, host, err.value());
            m_terminate = true;
        }
    });

    m_dac_asionet->start();

    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    auto curTime = beginTime;
    while (!m_dac_connected) {
        if (curTime - beginTime >= 5000)
            break;
        curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    }
    if (m_dac_connected) {
        m_dac_manger->run();
        while (1) {
            auto pack = m_dac_manger->getBuffer();
            if (pack) {
                m_dac_asionet->sendSyncData(pack);
                auto ch1 = pack->getBuffer(DataLib::EDataBuffersPackChannel::CH1);
                auto ch2 = pack->getBuffer(DataLib::EDataBuffersPackChannel::CH2);
                if (m_callback)
                    m_callback->sentPack(client, ch1 ? ch1->getDataLenght() : 0, ch2 ? ch2->getDataLenght() : 0);
                m_dac_manger->unlockBuffer();
            }

            if (m_terminate) {
                if (m_dac_connected) {
                    if (m_fileEnded) {
                        if (m_dac_manger->isEmptyBuffer())
                            break;
                    } else
                        break;

                } else {
                    break;
                }
            }
        }
    }
    m_dac_manger->stop();
    m_dac_manger = nullptr;
    m_dac_asionet->stop();
    m_dac_asionet = nullptr;
    delete memoryManager;
}

DACStreamClient::DACStreamClient(std::shared_ptr<ConfigStreamClient> configClient) {
    m_pimpl = new Impl();
    m_pimpl->m_configClient = configClient;
    m_pimpl->m_configCallback = std::make_shared<DACCb>(&m_pimpl->m_terminate);
    m_pimpl->m_configClient->addCallback(m_pimpl->m_configCallback);
}

DACStreamClient::~DACStreamClient() {
    stopStreaming();
    removeCallback();
    for (int i = 0; i < 2; i++) {
        delete[] m_pimpl->m_memChannels[i];
    }
    delete m_pimpl;
}

auto DACStreamClient::setCallback(std::shared_ptr<DACCallback> callback) -> void {
    removeCallback();
    const std::lock_guard lock(m_pimpl->m_smutex);
    m_pimpl->m_callback = callback;
}

auto DACStreamClient::removeCallback() -> void {
    const std::lock_guard lock(m_pimpl->m_smutex);
    m_pimpl->m_callback = nullptr;
    m_pimpl->m_configClient->removeCallback(m_pimpl->m_configCallback);
}

auto DACStreamClient::startStreamingTDMS(std::string host, std::string fileName) -> bool {
    m_pimpl->m_dataMode = dataMode::TDMS_MODE;
    m_pimpl->m_fileName = fileName;
    m_pimpl->m_host = host;
    return startStreaming();
}

auto DACStreamClient::startStreamingWAV(std::string host, std::string fileName) -> bool {
    m_pimpl->m_dataMode = dataMode::WAV_MODE;
    m_pimpl->m_fileName = fileName;
    m_pimpl->m_host = host;
    return startStreaming();
}

auto DACStreamClient::startStreamingFromMemory(std::string host) -> bool {
    m_pimpl->m_dataMode = dataMode::MEM_MODE;
    m_pimpl->m_host = host;
    if (m_pimpl->m_memChannels[0] && m_pimpl->m_memChannels[1]) {
        if (m_pimpl->m_memBytes[0] != m_pimpl->m_memBytes[1]) {
            WARNING("The data type of the first and second channels do not match")
            return false;
        }
        if (m_pimpl->m_memSize[0] != m_pimpl->m_memSize[1]) {
            WARNING("The memory size of the first and second channels do not match")
            return false;
        }
    }
    return startStreaming();
}

auto DACStreamClient::startStreamingFromMemorySink(std::string host, bool enableCh1, bool enableCh2, DACStreamBytes bytePerSample) -> bool {
    m_pimpl->m_dataMode = dataMode::MEM_STREAM_MODE;
    CReaderController::dac_channels_t channels;
    channels[DACChannels::DAC_CH1] = enableCh1;
    channels[DACChannels::DAC_CH2] = enableCh2;
    m_pimpl->m_memBytes[0] = (int)bytePerSample;
    m_pimpl->m_memBytes[1] = (int)bytePerSample;
    m_pimpl->m_memSinkChannels = channels;
    m_pimpl->m_host = host;
    return startStreaming();
}

auto DACStreamClient::startStreaming() -> bool {
    std::map<std::string, uint32_t> blockSizes;
    if (!requestMemoryBlockSizeCommon(m_pimpl->m_configClient, {m_pimpl->m_host}, &blockSizes, m_pimpl->m_verbose)) {
        aprintf(stdout, "%s Can't get block sizes\n", getTS(": ").c_str());
        return false;
    }

    auto ac_channels = m_pimpl->getActiveChannels();
    uint32_t blockSize = blockSizes[m_pimpl->m_host];
    StateRunningHosts runned_host;
    if (requestStartDACStreamingCommon(m_pimpl->m_configClient, m_pimpl->m_host, ac_channels.count(), &runned_host, m_pimpl->m_verbose)) {
        if (runned_host == StateRunningHosts::TCP && ac_channels.count() > 0)
            m_pimpl->m_client = new std::thread(&DACStreamClient::Impl::runClient, m_pimpl, this, m_pimpl->m_host, blockSize, ac_channels);
    }
    return true;
}

auto DACStreamClient::stopStreaming() -> void {
    m_pimpl->m_terminate = true;
    if (m_pimpl->m_client) {
        if (m_pimpl->m_client->joinable()) {
            m_pimpl->m_client->join();
        }
    }
}

auto DACStreamClient::wait() -> void {
    if (m_pimpl->m_client) {
        if (m_pimpl->m_client->joinable()) {
            m_pimpl->m_client->join();
        }
    }
}

auto DACStreamClient::notifyStop() -> void {
    m_pimpl->m_terminate = true;
}

auto DACStreamClient::setVerbose(bool enable) -> void {
    m_pimpl->m_verbose = enable;
}

auto DACStreamClient::setRepeatCount(uint64_t count) -> void {
    m_pimpl->m_repeatCount = count;
}

auto DACStreamClient::setRepeatInf(bool enable) -> void {
    m_pimpl->m_repeatInf = enable;
}

auto DACStreamClient::setMemory8Bit(uint8_t channel, std::vector<int8_t> buffer) -> bool {
    if (channel > 2 || channel == 0)
        return false;
    if (buffer.size() % 128) {
        WARNING("Data must be a multiple of 128 bytes")
        return false;
    }
    delete[] m_pimpl->m_memChannels[channel - 1];
    m_pimpl->m_memChannels[channel - 1] = (uint8_t*)new int8_t[buffer.size()];
    memcpy(m_pimpl->m_memChannels[channel - 1], buffer.data(), buffer.size());
    m_pimpl->m_memBytes[channel - 1] = 1;
    m_pimpl->m_memSize[channel - 1] = buffer.size();
    return true;
}

auto DACStreamClient::setMemory16Bit(uint8_t channel, std::vector<int16_t> buffer) -> bool {
    if (channel > 2 || channel == 0)
        return false;
    if (buffer.size() % 64) {
        WARNING("Data must be a multiple of 64 bytes")
        return false;
    }
    delete[] m_pimpl->m_memChannels[channel - 1];
    m_pimpl->m_memChannels[channel - 1] = (uint8_t*)new int16_t[buffer.size()];
    memcpy(m_pimpl->m_memChannels[channel - 1], buffer.data(), buffer.size() * 2);
    m_pimpl->m_memBytes[channel - 1] = 2;
    m_pimpl->m_memSize[channel - 1] = buffer.size() * 2;
    return true;
}