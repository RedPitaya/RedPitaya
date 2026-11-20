#include "stream_session.h"
#include "streaming_lib/streaming_file.h"

#include <filesystem>

#include <atomic>
#include <mutex>

#include "data_lib/network_header.h"
#include "logger_lib/file_logger.h"
#include "trigger_commands.h"
#include "uio_lib/memory_manager.h"

using namespace session_lib;
using streaming_lib::ADCStopReason;
using dac_streaming_lib::DACStopReason;
using gpio_streaming_lib::GPIOStopReason;

struct StreamSession::Impl : public std::enable_shared_from_this<StreamSession::Impl> {
    uio_lib::CHardware::Ptr hw;
    SessionOptions options;
    ServerNetConfigManager::Ptr config;
    streaming_lib::ADCStreamServer::Ptr adc;
    dac_streaming_lib::DACStreamServer::Ptr dac;
    gpio_streaming_lib::GPIOInStreamServer::Ptr gpioIn;
    gpio_streaming_lib::GPIOOutStreamServer::Ptr gpioOut;

    std::mutex cbMtx;
    std::shared_ptr<SessionCallback> callback;

    // the starts and stops of the application and of the config channel, one at a time per stream
    std::mutex adcMtx;
    std::mutex dacMtx;
    std::mutex gpioInMtx;
    std::mutex gpioOutMtx;
    // the memory is split again with every stream stopped; no start in between. Taken before
    // the stream's mutex.
    std::mutex memoryMtx;

    std::atomic_bool stopped{false};

    auto app() -> std::shared_ptr<SessionCallback> {
        std::lock_guard lock(cbMtx);
        return callback;
    }

    // Whether the board's memory takes the settings: 12 blocks of the block size for a stream,
    // a stream's memory no more than the RAM
    static auto memoryTakes(const CStreamSettings& s) -> bool {
        auto mm = uio_lib::CMemoryManager::instance();
        const uint64_t block = uint64_t(s.getMemoryBlockSize()) + DataLib::sizeHeader();
        if (block > mm->getMaxBlockSize()) {
            WARNING("The block size %u does not fit the memory: at most %u with the header", s.getMemoryBlockSize(), mm->getMaxBlockSize())
            return false;
        }
        for (uint32_t size : {s.getADCSize(), s.getDACSize(), s.getGPIOInSize(), s.getGPIOOutSize()}) {
            if (size > mm->getRAMSize()) {
                WARNING("The stream memory %u is more than the RAM %u", size, mm->getRAMSize())
                return false;
            }
        }
        return true;
    }

    // A new block size splits the memory again with every stream stopped; the sizes of the
    // streams take effect at their next starts.
    auto applyMemory(const CStreamSettings& before, const CStreamSettings& after, bool initial) -> void {
        auto mm = uio_lib::CMemoryManager::instance();
        // the order of the locks: memoryMtx, then the stream's
        std::lock_guard lock(memoryMtx);
        if (initial || before.getMemoryBlockSize() != after.getMemoryBlockSize()) {
            if (!initial) {
                if (options.verbose)
                    aprintf(stdout, "The block size changes from %u to %u: the streams stop\n", before.getMemoryBlockSize(), after.getMemoryBlockSize());
                stopADC(ADCStopReason::MEM_MODIFY, true);
                stopDAC(DACStopReason::MEM_MODIFY, true);
                stopGPIOIn(GPIOStopReason::MEM_MODIFY, true);
                stopGPIOOut(GPIOStopReason::MEM_MODIFY, true);
            }
            if (!mm->setMemoryBlockSize(after.getMemoryBlockSize()))
                ERROR_LOG("The memory manager refuses the block size %u", after.getMemoryBlockSize())
            mm->reallocateBlocks();
        }
        mm->setReserverdMemory(uio_lib::MM_ADC, after.getADCSize());
        mm->setReserverdMemory(uio_lib::MM_DAC, after.getDACSize());
        mm->setReserverdMemory(uio_lib::MM_GPIO_IN, after.getGPIOInSize());
        mm->setReserverdMemory(uio_lib::MM_GPIO_OUT, after.getGPIOOutSize());
    }

    auto startADCStreaming() -> bool {
        std::lock_guard mem(memoryMtx);
        std::lock_guard lock(adcMtx);
        // a start the config channel asked for while the session stopped: stop() sets stopped
        // before it takes this mutex to stop the stream, so either it stops this one or none starts
        if (stopped)
            return false;
        auto s = config->getSettings();
        uio_lib::CMemoryManager::instance()->setReserverdMemory(uio_lib::MM_ADC, s.getADCSize());
        streaming_lib::ADCStreamOptions o;
        o.host = options.host;
        o.filePath = options.adcDir;
        o.testMode = options.adcTestMode;
        adc->setVerbose(options.verbose);
        return adc->start(s, o);
    }

    auto startADC() -> void {
        adc->startADC();
        config->sendADCStarted();
        if (auto a = app())
            a->adcRunning();
    }

    auto stopADC(ADCStopReason reason, bool wait) -> void {
        if (!wait) {
            adc->stopNonBlocking(reason);
            return;
        }
        std::lock_guard lock(adcMtx);
        adc->stop(reason);
    }

    auto startDAC(dac_channels_t channels) -> bool {
        std::lock_guard mem(memoryMtx);
        std::lock_guard lock(dacMtx);
        // a start the config channel asked for while the session stopped: stop() sets stopped
        // before it takes this mutex to stop the stream, so either it stops this one or none starts
        if (stopped)
            return false;
        auto s = config->getSettings();
        uio_lib::CMemoryManager::instance()->setReserverdMemory(uio_lib::MM_DAC, s.getDACSize());
        dac->setVerbose(options.verbose);
        dac_streaming_lib::DACStreamOptions o;
        o.host = options.host;
        return dac->start(s, channels, o);
    }

    auto stopDAC(DACStopReason reason, bool wait) -> void {
        if (!wait) {
            dac->stopNonBlocking(reason);
            return;
        }
        std::lock_guard lock(dacMtx);
        dac->stop(reason);
    }

    auto startGPIOIn() -> bool {
        std::lock_guard mem(memoryMtx);
        std::lock_guard lock(gpioInMtx);
        // a start the config channel asked for while the session stopped: stop() sets stopped
        // before it takes this mutex to stop the stream, so either it stops this one or none starts
        if (stopped)
            return false;
        auto s = config->getSettings();
        uio_lib::CMemoryManager::instance()->setReserverdMemory(uio_lib::MM_GPIO_IN, s.getGPIOInSize());
        gpioIn->setVerbose(options.verbose);
        gpio_streaming_lib::GPIOStreamOptions o;
        o.host = options.host;
        return gpioIn->start(s, o);
    }

    auto stopGPIOIn(GPIOStopReason reason, bool wait) -> void {
        if (!wait) {
            gpioIn->stopNonBlocking(reason);
            return;
        }
        std::lock_guard lock(gpioInMtx);
        gpioIn->stop(reason);
    }

    auto startGPIOOut() -> bool {
        std::lock_guard mem(memoryMtx);
        std::lock_guard lock(gpioOutMtx);
        // a start the config channel asked for while the session stopped: stop() sets stopped
        // before it takes this mutex to stop the stream, so either it stops this one or none starts
        if (stopped)
            return false;
        auto s = config->getSettings();
        uio_lib::CMemoryManager::instance()->setReserverdMemory(uio_lib::MM_GPIO_OUT, s.getGPIOOutSize());
        gpioOut->setVerbose(options.verbose);
        gpio_streaming_lib::GPIOStreamOptions o;
        o.host = options.host;
        return gpioOut->start(s, o);
    }

    auto stopGPIOOut(GPIOStopReason reason, bool wait) -> void {
        if (!wait) {
            gpioOut->stopNonBlocking(reason);
            return;
        }
        std::lock_guard lock(gpioOutMtx);
        gpioOut->stop(reason);
    }
};

namespace {

// the record status of the session (StreamSession::recordStatus, CS_REQUEST_RECORD_STATUS)
auto recordStatusOf(StreamSession::Impl& p) -> RecordStatus {
    RecordStatus st;
    auto fill = [](StreamRecordStatus& s, int mode, bool running, uint64_t samples, uint64_t limit, uint64_t written, double rate, double queue,
                   const std::string& file) {
        s.mode = static_cast<uint8_t>(mode);
        s.running = running;
        s.samples = samples;
        s.limit = limit;
        s.written = written;
        s.writeRate = rate;
        s.queue = queue;
        s.file = file;
    };
    const auto a = p.adc->getRecord();
    // the network: the samples the FPGA gave, the file has none
    fill(st.adc, a.mode, a.running, a.mode == 2 ? a.samples : p.adc->getBoardSamples(), a.limit, a.written, a.writeRate, a.queue, a.file);
    auto limits = [](StreamRecordStatus& s, const CStreamSettings::FileLimits& l, int part, uint64_t partWritten) {
        s.limitTime = l.time;
        s.limitSize = l.size;
        s.splitSize = l.splitSize;
        s.splitTime = l.splitTime;
        s.part = static_cast<uint32_t>(std::max(0, part));
        s.partWritten = partWritten;
    };
    limits(st.adc, a.limits, a.part, a.partWritten);
    st.adc.lostBoard = p.adc->getBoardLost();
    st.adc.lostQueue = p.adc->getFileDropped();
    if (p.gpioIn) {
        const auto g = p.gpioIn->getRecord();
        fill(st.gpioIn, g.mode, g.running, g.mode == 2 ? g.samples : p.gpioIn->getBoardSamples(), g.limit, g.written, g.writeRate, g.queue, g.file);
        limits(st.gpioIn, g.limits, g.part, g.partWritten);
        st.gpioIn.lostBoard = p.gpioIn->getBoardLost();
    }
    std::error_code ec;
    const std::string dir = p.options.adcDir.empty() ? std::string(FILE_PATH) : p.options.adcDir;
    const auto space = std::filesystem::space(dir, ec);
    if (!ec) {
        st.sdFree = space.available;
        st.sdTotal = space.capacity;
    }
    return st;
}

using Weak = std::weak_ptr<StreamSession::Impl>;

// The events of the streams: the reply of the config protocol, then the application's
class ADCEvents : public streaming_lib::ADCServerCallback {
   public:
    explicit ADCEvents(Weak s) : m_s(s) {}

    void startedTCP(streaming_lib::ADCStreamServer*) override {
        if (auto s = m_s.lock()) {
            s->config->sendADCServerStartedTCP();
            if (auto a = s->app())
                a->adcStarted(false);
        }
    }

    void startedSD(streaming_lib::ADCStreamServer*) override {
        if (auto s = m_s.lock()) {
            s->config->sendADCServerStartedSD();
            if (auto a = s->app())
                a->adcStarted(true);
        }
    }

    void stopped(streaming_lib::ADCStreamServer*, ADCStopReason reason) override {
        auto s = m_s.lock();
        if (!s)
            return;
        auto& c = *s->config;
        switch (reason) {
            case ADCStopReason::NORMAL:
            case ADCStopReason::DISCONNECTED:  // the client of the data is gone: an ordinary stop
                c.sendServerStopped();
                break;
            case ADCStopReason::SD_FULL:
                c.sendServerStoppedSDFull();
                break;
            case ADCStopReason::DONE:
                c.sendServerStoppedDone();
                break;
            case ADCStopReason::MEM_ERROR:
                c.sendServerMemoryErrorStopped();
                break;
            case ADCStopReason::MEM_MODIFY:
                c.sendServerMemoryModifyStopped();
                break;
            case ADCStopReason::NO_CHANNELS:
                c.sendServerNoChannelsStopped();
                break;
            case ADCStopReason::CONFIG_ERROR:
                c.sendServerConfigErrorStopped();
                break;
        }
        if (auto a = s->app())
            a->adcStopped(reason);
    }

   private:
    Weak m_s;
};

class DACEvents : public dac_streaming_lib::DACServerCallback {
   public:
    explicit DACEvents(Weak s) : m_s(s) {}

    void startedTCP(dac_streaming_lib::DACStreamServer*) override {
        if (auto s = m_s.lock()) {
            s->config->sendDACServerStarted();
            if (auto a = s->app())
                a->dacStarted(false);
        }
    }

    void startedSD(dac_streaming_lib::DACStreamServer*) override {
        if (auto s = m_s.lock()) {
            s->config->sendDACServerStartedSD();
            if (auto a = s->app())
                a->dacStarted(true);
        }
    }

    void stopped(dac_streaming_lib::DACStreamServer*, DACStopReason reason) override {
        auto s = m_s.lock();
        if (!s)
            return;
        auto& c = *s->config;
        switch (reason) {
            case DACStopReason::NORMAL:
            case DACStopReason::DISCONNECTED:
                c.sendDACServerStopped();
                break;
            case DACStopReason::DONE:
                c.sendDACServerStoppedSDDone();
                break;
            case DACStopReason::EMPTY:
                c.sendDACServerStoppedSDEmpty();
                break;
            case DACStopReason::BROKEN:
                c.sendDACServerStoppedSDBroken();
                break;
            case DACStopReason::MISSING_FILE:
                c.sendDACServerStoppedSDMissingFile();
                break;
            case DACStopReason::MEM_ERROR:
            case DACStopReason::NO_CHANNELS:  // the protocol has no reply of its own for it
                c.sendDACServerMemoryErrorStopped();
                break;
            case DACStopReason::MEM_MODIFY:
                c.sendDACServerMemoryModifyStopped();
                break;
            case DACStopReason::CONFIG_ERROR:
                c.sendDACServerConfigErrorStopped();
                break;
        }
        if (auto a = s->app())
            a->dacStopped(reason);
    }

   private:
    Weak m_s;
};

class GPIOInEvents : public gpio_streaming_lib::GPIOInServerCallback {
   public:
    explicit GPIOInEvents(Weak s) : m_s(s) {}

    void started(gpio_streaming_lib::GPIOInStreamServer*) override {
        if (auto s = m_s.lock()) {
            s->config->sendGPIOInServerStarted();
            if (auto a = s->app())
                a->gpioInStarted();
        }
    }

    void startedSD(gpio_streaming_lib::GPIOInStreamServer*) override {
        if (auto s = m_s.lock()) {
            s->config->sendGPIOInServerStartedSD();
            if (auto a = s->app())
                a->gpioInStarted();
        }
    }

    void stopped(gpio_streaming_lib::GPIOInStreamServer*, GPIOStopReason reason) override {
        auto s = m_s.lock();
        if (!s)
            return;
        auto& c = *s->config;
        switch (reason) {
            case GPIOStopReason::NORMAL:
            case GPIOStopReason::MEM_MODIFY:  // the protocol has no reply of its own for it
            case GPIOStopReason::DISCONNECTED:
                c.sendGPIOInServerStopped();
                break;
            case GPIOStopReason::MEM_ERROR:
                c.sendGPIOInServerMemoryErrorStopped();
                break;
            case GPIOStopReason::CONFIG_ERROR:
            case GPIOStopReason::MISSING_FILE:  // the output's only
            case GPIOStopReason::EMPTY:
                c.sendGPIOInServerConfigErrorStopped();
                break;
            case GPIOStopReason::DONE:
                c.sendGPIOInServerStoppedSDDone();
                break;
            case GPIOStopReason::SD_FULL:
                c.sendGPIOInServerStoppedSDFull();
                break;
            case GPIOStopReason::BROKEN:
                c.sendGPIOInServerStoppedSDBroken();
                break;
        }
        if (auto a = s->app())
            a->gpioInStopped(reason);
    }

   private:
    Weak m_s;
};

class GPIOOutEvents : public gpio_streaming_lib::GPIOOutServerCallback {
   public:
    explicit GPIOOutEvents(Weak s) : m_s(s) {}

    void started(gpio_streaming_lib::GPIOOutStreamServer*) override {
        if (auto s = m_s.lock()) {
            s->config->sendGPIOOutServerStarted();
            if (auto a = s->app())
                a->gpioOutStarted();
        }
    }

    void startedSD(gpio_streaming_lib::GPIOOutStreamServer*) override {
        if (auto s = m_s.lock()) {
            s->config->sendGPIOOutServerStartedSD();
            if (auto a = s->app())
                a->gpioOutStarted();
        }
    }

    void stopped(gpio_streaming_lib::GPIOOutStreamServer*, GPIOStopReason reason, uint64_t waitClocks) override {
        auto s = m_s.lock();
        if (!s)
            return;
        auto& c = *s->config;
        switch (reason) {
            case GPIOStopReason::NORMAL:
            case GPIOStopReason::MEM_MODIFY:
            case GPIOStopReason::DISCONNECTED:
                c.sendGPIOOutServerStopped(waitClocks);
                break;
            case GPIOStopReason::MEM_ERROR:
                c.sendGPIOOutServerMemoryErrorStopped();
                break;
            case GPIOStopReason::CONFIG_ERROR:
            case GPIOStopReason::SD_FULL:  // the input's only
                c.sendGPIOOutServerConfigErrorStopped();
                break;
            case GPIOStopReason::DONE:
                c.sendGPIOOutServerStoppedSDDone(waitClocks);
                break;
            case GPIOStopReason::EMPTY:
                c.sendGPIOOutServerStoppedSDEmpty();
                break;
            case GPIOStopReason::BROKEN:
                c.sendGPIOOutServerStoppedSDBroken();
                break;
            case GPIOStopReason::MISSING_FILE:
                c.sendGPIOOutServerStoppedSDMissingFile();
                break;
        }
        if (auto a = s->app())
            a->gpioOutStopped(reason, waitClocks);
    }

   private:
    Weak m_s;
};

}  // namespace

auto StreamSession::create(uio_lib::CHardware::Ptr hw, const SessionOptions& options) -> Ptr {
    if (!hw)
        throw std::runtime_error("StreamSession: no hardware");
    auto p = std::make_shared<Impl>();
    p->hw = hw;
    p->options = options;
    const auto mode = hw->features().slave ? broadcast_lib::EMode::AB_SERVER_SLAVE : broadcast_lib::EMode::AB_SERVER_MASTER;
    p->config = std::make_shared<ServerNetConfigManager>(options.configFile, mode, options.host, options.configPort, hw->info().adcChannels);
    {
        const auto info = hw->info();
        const auto features = hw->features();
        BoardInfoMessage msg;
        msg.model = info.model;
        msg.mac = info.mac;
        msg.master = !features.slave;
        msg.adcChannels = info.adcChannels;
        msg.dacChannels = info.dacChannels;
        msg.adcRate = info.adcRate;
        msg.dacRate = info.dacRate;
        msg.adcBits = info.adcBits;
        msg.adcRanges = info.adcRanges;
        msg.adcACDC = info.adcACDC;
        msg.dacGain = info.dacGain;
        msg.adc = features.adc;
        msg.adcBlockMode = features.adcBlockMode;
        msg.dac = features.dac;
        msg.dacTriggerOutputs = features.dacTriggerOutputs;
        msg.gpio = features.gpio;
        msg.extTrigger = features.extTrigger;
        msg.memorySize = uio_lib::CMemoryManager::instance()->getRAMSize();
        msg.memoryMaxBlock = uio_lib::CMemoryManager::instance()->getMaxBlockSize();
        p->config->setBoardInfo(msg);
    }

    // the directory of the ADC files (those of the DAC and the GPIO are made by settings_lib)
    const std::string adcDir = options.adcDir.empty() ? std::string(FILE_PATH) : options.adcDir;
    streaming_lib::CStreamingFile::makeEmptyDir(adcDir);
    if (!std::filesystem::is_directory(adcDir))
        ERROR_LOG("Can't make the directory of the ADC files %s", adcDir.c_str())

    p->config->setSettingsValidator(&Impl::memoryTakes);
    const auto initial = p->config->getSettings();
    if (!Impl::memoryTakes(initial))
        ERROR_LOG("The memory settings of %s do not fit the board", options.configFile.c_str())
    p->applyMemory(initial, initial, true);

    Weak w = p;
    p->adc = std::make_shared<streaming_lib::ADCStreamServer>(hw);
    p->adc->setCallback(std::make_shared<ADCEvents>(w));
    p->dac = std::make_shared<dac_streaming_lib::DACStreamServer>(hw);
    p->dac->setCallback(std::make_shared<DACEvents>(w));
    p->gpioIn = std::make_shared<gpio_streaming_lib::GPIOInStreamServer>(hw);
    p->gpioIn->setCallback(std::make_shared<GPIOInEvents>(w));
    p->gpioOut = std::make_shared<gpio_streaming_lib::GPIOOutStreamServer>(hw);
    p->gpioOut->setCallback(std::make_shared<GPIOOutEvents>(w));

    auto& c = *p->config;
    c.settingsChangedNofiy.connect([w](const CStreamSettings& before, const CStreamSettings& after) {
        if (auto s = w.lock()) {
            if (s->options.verbose) {
                CStreamSettings shown(after);
                aprintf(stdout, "Get new settings\n%s", shown.toString().c_str());
            }
            s->applyMemory(before, after, false);
            if (auto a = s->app())
                a->settingsChanged(before, after);
        }
    });
    c.startStreamingNofiy.connect([w]() {
        if (auto s = w.lock())
            s->startADCStreaming();
    });
    c.stopStreamingNofiy.connect([w]() {
        if (auto s = w.lock())
            s->stopADC(ADCStopReason::NORMAL, false);
    });
    c.startADCNofiy.connect([w]() {
        if (auto s = w.lock())
            s->startADC();
    });
    c.startDacStreamingNofiy.connect([w](dac_channels_t channels) {
        if (auto s = w.lock())
            s->startDAC(channels);
    });
    c.stopDacStreamingNofiy.connect([w]() {
        if (auto s = w.lock())
            s->stopDAC(DACStopReason::NORMAL, false);
    });
    c.startGpioInStreamingNofiy.connect([w]() {
        if (auto s = w.lock())
            s->startGPIOIn();
    });
    c.stopGpioInStreamingNofiy.connect([w]() {
        if (auto s = w.lock())
            s->stopGPIOIn(GPIOStopReason::NORMAL, false);
    });
    c.startGpioOutStreamingNofiy.connect([w]() {
        if (auto s = w.lock())
            s->startGPIOOut();
    });
    c.stopGpioOutStreamingNofiy.connect([w]() {
        if (auto s = w.lock())
            s->stopGPIOOut(GPIOStopReason::NORMAL, false);
    });
    connectTriggerCommands(p->config, hw);
    c.recordStatusRequestNofiy.connect([w]() {
        auto s = w.lock();
        if (!s || !s->config)
            return;
        s->config->sendRecordStatus(recordStatusOf(*s));
    });

    // The announcement: on every interface with its own address; listening on one address, that one
    // only, on its interface (or as a broadcast to all when no interface has it); a loopback
    // address is not announced, nobody else can connect to it
    const bool everyInterface = options.host.empty() || options.host == "0.0.0.0";
    const bool loopback = !everyInterface && options.host.rfind("127.", 0) == 0;
    if (options.broadcast && !loopback) {
        broadcast_lib::CAsioBroadcastSocket::TargetProvider targets = nullptr;
        if (!everyInterface) {
            const std::string host = options.host;
            targets = [host]() {
                std::vector<broadcast_lib::BroadcastTarget> list;
                for (const auto& t : broadcast_lib::CAsioBroadcastSocket::systemTargets())
                    if (t.address == host)
                        list.push_back(t);
                if (list.empty())
                    list.push_back({host, "255.255.255.255"});
                return list;
            };
        }
        const std::string announced = !everyInterface ? options.host : !options.broadcastHost.empty() ? options.broadcastHost : std::string("127.0.0.1");
        c.startBroadcast(hw->info().model, hw->info().mac, announced, options.broadcastPort, hw->features().gpio ? "gpio" : "", targets);
    } else if (options.broadcast && options.verbose) {
        aprintf(stdout, "[Streaming] Listening on %s only: the board is not announced\n", options.host.c_str());
    }

    return Ptr(new StreamSession(p));
}

StreamSession::StreamSession(std::shared_ptr<Impl> impl) : m_pimpl(std::move(impl)) {}

StreamSession::~StreamSession() {
    stop();
}

auto StreamSession::stop() -> void {
    auto& p = *m_pimpl;
    if (p.stopped.exchange(true))
        return;
    // the clients hear the stops, then the channel closes
    p.stopADC(ADCStopReason::NORMAL, true);
    p.stopDAC(DACStopReason::NORMAL, true);
    p.stopGPIOIn(GPIOStopReason::NORMAL, true);
    p.stopGPIOOut(GPIOStopReason::NORMAL, true);
    p.config->stop();
    p.config->settingsChangedNofiy.disconnect_all();
}

auto StreamSession::isMaster() const -> bool {
    return !m_pimpl->hw->features().slave;
}

auto StreamSession::hardware() const -> uio_lib::CHardware::Ptr {
    return m_pimpl->hw;
}

auto StreamSession::config() const -> ServerNetConfigManager::Ptr {
    return m_pimpl->config;
}

auto StreamSession::settings() -> CStreamSettings {
    return m_pimpl->config->getSettings();
}

auto StreamSession::updateSettings(const std::function<bool(CStreamSettings&)>& change) -> bool {
    return m_pimpl->config->changeSettings(change);
}

auto StreamSession::saveSettings() -> bool {
    auto s = m_pimpl->config->getSettings();
    return s.writeToFile(m_pimpl->options.configFile);
}

auto StreamSession::setBroadcastAddress(const std::string& host) -> void {
    m_pimpl->config->setBroadcastAddress(host);
}

auto StreamSession::startADCStreaming() -> bool {
    return m_pimpl->startADCStreaming();
}

auto StreamSession::startADC() -> void {
    m_pimpl->startADC();
}

auto StreamSession::stopADCStreaming(ADCStopReason reason, bool wait) -> void {
    m_pimpl->stopADC(reason, wait);
}

auto StreamSession::startDAC(dac_channels_t channels) -> bool {
    return m_pimpl->startDAC(channels);
}

auto StreamSession::stopDAC(DACStopReason reason, bool wait) -> void {
    m_pimpl->stopDAC(reason, wait);
}

auto StreamSession::startGPIOIn() -> bool {
    return m_pimpl->startGPIOIn();
}

auto StreamSession::stopGPIOIn(GPIOStopReason reason, bool wait) -> void {
    m_pimpl->stopGPIOIn(reason, wait);
}

auto StreamSession::startGPIOOut() -> bool {
    return m_pimpl->startGPIOOut();
}

auto StreamSession::stopGPIOOut(GPIOStopReason reason, bool wait) -> void {
    m_pimpl->stopGPIOOut(reason, wait);
}

auto StreamSession::adcPassSamples() -> uint64_t {
    return m_pimpl->adc->getPassSamples();
}

auto StreamSession::adcWrittenSize() -> uint64_t {
    return m_pimpl->adc->getWritedSize();
}

auto StreamSession::recordStatus() -> RecordStatus {
    return recordStatusOf(*m_pimpl);
}

auto StreamSession::adcBoardLost() -> uint64_t {
    return m_pimpl->adc->getBoardLost();
}

auto StreamSession::adcBoardSamples() -> uint64_t {
    return m_pimpl->adc->getBoardSamples();
}

auto StreamSession::adcFileDropped() -> uint64_t {
    return m_pimpl->adc->getFileDropped();
}

auto StreamSession::gpioInBoardLost() -> uint64_t {
    return m_pimpl->gpioIn ? m_pimpl->gpioIn->getBoardLost() : 0;
}

auto StreamSession::gpioInBoardSamples() -> uint64_t {
    return m_pimpl->gpioIn ? m_pimpl->gpioIn->getBoardSamples() : 0;
}

auto StreamSession::adcWriteRate() -> double {
    return m_pimpl->adc->getWriteRate();
}

auto StreamSession::adcFileQueueFill() -> double {
    return m_pimpl->adc->getQueueFill();
}

auto StreamSession::setCallback(std::shared_ptr<SessionCallback> callback) -> void {
    std::lock_guard lock(m_pimpl->cbMtx);
    m_pimpl->callback = std::move(callback);
}

auto StreamSession::removeCallback() -> void {
    setCallback(nullptr);
}
