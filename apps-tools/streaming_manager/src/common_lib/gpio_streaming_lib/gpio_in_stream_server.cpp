#include "gpio_in_stream_server.h"

#include <atomic>
#include <chrono>
#include <ctime>
#include <mutex>
#include <optional>
#include <thread>

#ifndef _WIN32
#include <syslog.h>
#endif

#include "data_lib/buffers_cached.h"
#include "data_lib/network_header.h"
#include "logger_lib/file_logger.h"
#include "net_lib/asio_net.h"
#include "uio_lib/memory_manager.h"
#include "writer_lib/gpio_file_writer.h"

using namespace gpio_streaming_lib;
using DataLib::CBuffersCached;
using DataLib::CDataBufferDMA;
using DataLib::CDataBuffersPackDMA;

namespace {

auto bufferOf(const CDataBuffersPackDMA::Ptr& pack) -> CDataBufferDMA::Ptr {
    return pack ? pack->getBuffer(DataLib::EDataBuffersPackChannel::CH1) : nullptr;
}

// the samples a buffer of input words stands for: an RLE word of count c is c + 1
auto samplesOf(const CDataBufferDMA::Ptr& buffer, bool rle) -> uint64_t {
    auto words = buffer->getDataLenght() / 4;
    if (!rle)
        return words;
    auto data = static_cast<const uint32_t*>(buffer->getMappedDataMemory());
    uint64_t samples = 0;
    for (size_t i = 0; i < words; i++)
        samples += ((data[i] >> 8) & 0xFF) + 1;
    return samples;
}

}  // namespace

struct GPIOInStreamServer::Impl {
    GPIOInStreamServer* owner = nullptr;
    std::weak_ptr<Impl> self;
    uio_lib::CHardware::Ptr hw;
    std::mutex mtx;  // start and stop
    std::mutex cbMtx;
    std::shared_ptr<GPIOInServerCallback> callback;
    bool verbose = false;

    // Every start() is a new session. A stop from a thread of its own belongs to the session it
    // was asked in; atomic: stopNonBlocking() is also called from start() under mtx.
    std::atomic<uint64_t> session{0};
    std::atomic<bool> active{false};  // the session is not torn down yet

    // the run
    std::atomic<bool> run{false};
    CBuffersCached::Ptr ring;
    net_lib::CAsioNet::Ptr net;
    std::thread fpgaThread;  // FPGA buffers to the ring
    std::thread sendThread;  // the ring to the client, or to the file
    CGpioFileWriter::Ptr file;  // the file mode
    CDataBuffersPackDMA::Ptr packs[2];
    DataLib::GPIOHeader header;  // what every buffer of the run carries
    bool rle = false;

    auto teardown() -> void;
    // of: the session the stop was asked in, none for a stop of whatever runs
    auto stop(GPIOStopReason reason, std::optional<uint64_t> of = std::nullopt, bool own = false) -> void;
    // own: asked by the server's own threads (a client gone), void once the session ended
    auto stopNonBlocking(GPIOStopReason reason, bool own) -> void;
    auto fpgaWork() -> void;
    auto sendWork() -> void;
    auto fileWork() -> void;
    template <typename F>
    auto notify(F f) -> void {
        std::shared_ptr<GPIOInServerCallback> cb;
        {
            std::lock_guard lock(cbMtx);
            cb = callback;
        }
        if (cb)
            f(cb.get());
    }
};

// The input stops first, then the network: a sender blocked on a client that does not read is
// released, then the threads end and the memory goes
auto GPIOInStreamServer::Impl::teardown() -> void {
    active = false;
    bool was = run.exchange(false);
    if (hw && was)
        hw->gpioStopInput();
    if (ring)
        ring->notifyToDestory();
    if (net)
        net->disconnect();
    for (auto t : {&fpgaThread, &sendThread})
        if (t->joinable())
            t->join();
    if (file)
        file->close();  // what is queued still goes into the file
    net = nullptr;
    packs[0] = packs[1] = nullptr;
    ring = nullptr;
    uio_lib::CMemoryManager::instance()->releaseMemory(uio_lib::MM_GPIO_IN);
}

// A stop asked in an earlier session neither tears down nor reports: the session it was for is
// gone with the start that followed, and its reason (a failed start) is not the new one's.
auto GPIOInStreamServer::Impl::stop(GPIOStopReason reason, std::optional<uint64_t> of, bool own) -> void {
    try {
        {
            std::lock_guard lock(mtx);
            if ((of && session != *of) || (own && !active)) {
                if (verbose)
                    aprintf(stdout, "[Streaming] A stop of an earlier session is ignored\n");
                return;
            }
            teardown();
        }
        notify([&](GPIOInServerCallback* cb) { cb->stopped(owner, reason); });
        if (verbose) {
            aprintf(stdout, "[Streaming] Stop gpio input server\n");
#ifndef _WIN32
            syslog(LOG_NOTICE, "[Streaming] Stop gpio input server\n");
#endif
        }
    } catch (std::exception& e) {
        ERROR_LOG("GPIOInStreamServer::stop() %s", e.what())
    }
}

auto GPIOInStreamServer::Impl::stopNonBlocking(GPIOStopReason reason, bool own) -> void {
    try {
        auto impl = self.lock();
        uint64_t of = session;
        if (impl)
            std::thread([impl, reason, of, own]() { impl->stop(reason, of, own); }).detach();
    } catch (std::exception& e) {
        ERROR_LOG("GPIOInStreamServer::stopNonBlocking() %s", e.what())
    }
}

// A full FPGA buffer goes to the ring with its header, a free pack takes its place. Without a
// free pack within a second the buffer is dropped and written again; its samples count as lost
// before the next buffer.
auto GPIOInStreamServer::Impl::fpgaWork() -> void {
    uint8_t index = 0;
    uint64_t droppedLost = 0;
    uint64_t droppedWait = 0;
    try {
        while (run) {
            if (!hw->gpioWaitIn(index, 100))
                continue;
            auto info = hw->gpioInBuffer(index);
            auto full = packs[index];
            auto next = ring->writeBuffer(true);
            auto buffer = bufferOf(full);
            if (next) {
                auto h = header;
                h.lostFPGA = info.lost + droppedLost;
                h.waitFPGA = info.wait + droppedWait;
                h.triggerCount = info.triggerCount;
                h.dataSize = buffer->getDataLenght();
                DataLib::setHeaderGPIO(buffer, 0, h);
                ring->unlockBufferWrite();
                droppedLost = droppedWait = 0;
                packs[index] = next;
                hw->gpioSetInBuffer(index, bufferOf(next)->getDataAddress());
            } else {
                droppedLost += info.lost + samplesOf(buffer, rle);
                droppedWait += info.wait;
            }
            hw->gpioReleaseInBuffer(index);
            index ^= 1;
        }
    } catch (std::exception& e) {
        ERROR_LOG("GPIOInStreamServer input %s", e.what())
    }
}

// Without a client the buffers are dropped here, as the ADC does
auto GPIOInStreamServer::Impl::sendWork() -> void {
    uint64_t id = 0;
    try {
        while (run) {
            auto pack = ring->readBuffer();
            if (!pack)
                continue;
            if (net->isConnected()) {
                auto buffer = bufferOf(pack);
                DataLib::GPIOHeader h;
                if (DataLib::getHeaderGPIO(buffer, h))
                    DataLib::setHeaderGPIO(buffer, id, h);
                net->sendSyncData(pack);
                id++;
            }
            ring->unlockBufferRead();
        }
    } catch (std::exception& e) {
        ERROR_LOG("GPIOInStreamServer sender %s", e.what())
    }
}

// The file mode: the ring into the file. The sample limit, a full disk or an error end the run.
auto GPIOInStreamServer::Impl::fileWork() -> void {
    try {
        while (run) {
            auto pack = ring->readBuffer();
            if (!pack)
                continue;
            auto buffer = bufferOf(pack);
            DataLib::GPIOHeader h;
            DataLib::getHeaderGPIO(buffer, h);
            const auto* words = static_cast<const uint32_t*>(buffer->getMappedDataMemory());
            const auto result = file->write(words, buffer->getDataLenght() / 4, h.lostFPGA, h.waitFPGA, h.triggerCount);
            ring->unlockBufferRead();
            if (result == CGpioFileWriter::Result::OK)
                continue;
            stopNonBlocking(result == CGpioFileWriter::Result::LIMIT  ? GPIOStopReason::DONE
                            : result == CGpioFileWriter::Result::FULL ? GPIOStopReason::SD_FULL
                                                                      : GPIOStopReason::BROKEN,
                            true);
            break;
        }
    } catch (std::exception& e) {
        ERROR_LOG("GPIOInStreamServer file %s", e.what())
    }
}

GPIOInStreamServer::GPIOInStreamServer(uio_lib::CHardware::Ptr hw) : m_pimpl(std::make_shared<Impl>()) {
    m_pimpl->owner = this;
    m_pimpl->self = m_pimpl;
    m_pimpl->hw = hw;
}

GPIOInStreamServer::~GPIOInStreamServer() {
    removeCallback();
    std::lock_guard lock(m_pimpl->mtx);
    m_pimpl->teardown();
}

auto GPIOInStreamServer::start(const CStreamSettings& settings, const GPIOStreamOptions& options) -> bool {
    auto& p = *m_pimpl;
    try {
        std::lock_guard lock(p.mtx);
        p.teardown();
        p.file = nullptr;
        p.session++;
        p.active = true;
        if (!p.hw)
            return false;
        if (!p.hw->features().gpio) {
            ERROR_LOG("There is no GPIO block in the FPGA")
            p.stopNonBlocking(GPIOStopReason::CONFIG_ERROR, true);
            return false;
        }

        uio_lib::GPIOConfig cfg;
        cfg.polarity = settings.getGPIOPolarity();
        cfg.decimation = settings.getGPIODecimation();
        cfg.rle = settings.getGPIORLE().value == CStreamSettings::State::ON;
        cfg.bufferBytes = settings.getMemoryBlockSize();
        uio_lib::GPIOTrigger trig;
        trig.input = settings.getGPIOInTriggerMode().value == CStreamSettings::State::ON;
        trig.sources = settings.getGPIOTriggerSources();
        trig.events = static_cast<uio_lib::EventSource>(static_cast<int>(settings.getGPIOEventSource().value));
        auto blocks = settings.getGPIOTriggerBlocks();
        trig.blocks = static_cast<uint16_t>(std::min<uint32_t>(blocks, UINT16_MAX));
        trig.patternMask = settings.getGPIOPatternMask();
        trig.patternValue = settings.getGPIOPatternValue();
        trig.patternPosEdge = settings.getGPIOPatternPosEdge();
        trig.patternNegEdge = settings.getGPIOPatternNegEdge();
        if (!p.hw->gpioConfigureInput(cfg) || blocks > UINT16_MAX || !p.hw->gpioSetInputTrigger(trig)) {
            ERROR_LOG("The GPIO input settings are not taken (decimation %u, block %u bytes, trigger blocks %u; the output may run with others)", cfg.decimation,
                      cfg.bufferBytes, blocks)
            p.stopNonBlocking(GPIOStopReason::CONFIG_ERROR, true);
            return false;
        }
        p.rle = cfg.rle;
        p.header = DataLib::GPIOHeader();
        p.header.decimation = cfg.decimation;
        // the block runs on the DAC clock; a board without a DAC (4 inputs) has the ADC one
        p.header.gpioClock = p.hw->info().dacRate ? p.hw->info().dacRate : p.hw->info().adcRate;
        p.header.triggerBlocks = trig.blocks;
        p.header.flags = (cfg.rle ? DataLib::GPIO_FLAG_RLE : 0) | (trig.input ? DataLib::GPIO_FLAG_TRIGGER_MODE : 0) |
                         (p.hw->features().gpioWaitCounter ? DataLib::GPIO_FLAG_WAIT_COUNTER : 0);

        auto memory = uio_lib::CMemoryManager::instance();
        memory->releaseMemory(uio_lib::MM_GPIO_IN);
        auto ramSize = memory->getReserverdMemory(uio_lib::MM_GPIO_IN);
        if (ramSize < memory->getMinRAMSize(uio_lib::MM_GPIO_IN)) {
            ERROR_LOG("Not enough memory for the GPIO input")
            p.stopNonBlocking(GPIOStopReason::MEM_ERROR, true);
            return false;
        }
        auto reserved = memory->reserveMemory(uio_lib::MM_GPIO_IN, ramSize / memory->getMemoryBlockSize(), 1);
        if (reserved < 3) {  // two in the FPGA, one free
            ERROR_LOG("Can't reserve memory via memory manager")
            p.stopNonBlocking(GPIOStopReason::MEM_ERROR, true);
            return false;
        }
        if (p.verbose)
            aprintf(stdout, "Reserved memory blocks: %d size: %d\n", reserved, reserved * memory->getMemoryBlockSize());
        adc_channels_t one;
        one[ADCChannels::ADC_CH1] = true;
        p.ring = CBuffersCached::create();
        p.ring->generateBuffersEmptyADC(one, memory->getRegions(uio_lib::MM_GPIO_IN), DataLib::sizeHeader());

        p.hw->gpioPrepareInput();
        for (uint8_t i = 0; i < 2; i++) {
            p.packs[i] = p.ring->writeBuffer(true);
            p.hw->gpioSetInBuffer(i, bufferOf(p.packs[i])->getDataAddress());
        }
        auto selfW = std::weak_ptr<Impl>(m_pimpl);
        const bool toFile = settings.getGPIOInPassMode().value == CStreamSettings::PassMode::FILE;
        if (toFile) {
            GpioRecording rec;
            rec.format = settings.getGPIOInFormat();
            rec.rle = p.rle;
            rec.clock = p.header.gpioClock;
            rec.decimation = cfg.decimation;
            rec.triggerMode = trig.input;
            rec.waitCounter = p.hw->features().gpioWaitCounter;
            rec.startTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            rec.samplesLimit = settings.getGPIOInSamples();
            char name[48];
            const time_t now = time(nullptr);
            strftime(name, sizeof(name), "gpio_%Y-%m-%d_%H-%M-%S.", gmtime(&now));
            const std::string ext = rec.format.value == CStreamSettings::GPIOFormat::CSV ? "csv" : rec.format.value == CStreamSettings::GPIOFormat::VCD ? "vcd" : "bin";
            const auto dir = options.dirPath.empty() ? CStreamSettings::getGPIODirPath() : options.dirPath;
            p.file = std::make_shared<CGpioFileWriter>();
            if (!p.file->open(dir + "/" + name + ext, rec)) {
                ERROR_LOG("Can't make the GPIO file in %s", dir.c_str())
                p.file = nullptr;
                p.stopNonBlocking(GPIOStopReason::BROKEN, true);
                return false;
            }
            p.file->outOfSpaceNotify.connect([selfW]() {
                if (auto impl = selfW.lock())
                    impl->stopNonBlocking(GPIOStopReason::SD_FULL, true);
            });
        } else {
            p.net = net_lib::CAsioNet::create(net_lib::M_SERVER, options.host, NET_GPIO_IN_STREAMING_PORT, nullptr);
            // the client is gone: nothing reads the input any more
            p.net->serverDisconnectNotify.connect([selfW](std::string&) {
                if (auto impl = selfW.lock())
                    impl->stopNonBlocking(GPIOStopReason::DISCONNECTED, true);
            });
            p.net->start();
        }
        p.run = true;
        p.fpgaThread = std::thread(&Impl::fpgaWork, &p);
        p.sendThread = toFile ? std::thread(&Impl::fileWork, &p) : std::thread(&Impl::sendWork, &p);
        p.hw->gpioStartInput();
        if (toFile)
            p.notify([&](GPIOInServerCallback* cb) { cb->startedSD(this); });
        else
            p.notify([&](GPIOInServerCallback* cb) { cb->started(this); });
        if (p.verbose)
            aprintf(stdout, "[Streaming] Start gpio input server\n");
        return true;
    } catch (std::exception& e) {
        ERROR_LOG("GPIOInStreamServer::start() %s", e.what())
        p.stopNonBlocking(GPIOStopReason::CONFIG_ERROR, true);
    }
    return false;
}

auto GPIOInStreamServer::stop(GPIOStopReason reason) -> void {
    m_pimpl->stop(reason);
}

auto GPIOInStreamServer::stopNonBlocking(GPIOStopReason reason) -> void {
    m_pimpl->stopNonBlocking(reason, false);
}

auto GPIOInStreamServer::filePath() -> std::string {
    std::lock_guard lock(m_pimpl->mtx);
    return m_pimpl->file ? m_pimpl->file->path() : std::string();
}

auto GPIOInStreamServer::isRunning() -> bool {
    return m_pimpl->active && m_pimpl->run;
}

auto GPIOInStreamServer::setVerbose(bool enable) -> void {
    m_pimpl->verbose = enable;
}

auto GPIOInStreamServer::setCallback(std::shared_ptr<GPIOInServerCallback> callback) -> void {
    std::lock_guard lock(m_pimpl->cbMtx);
    m_pimpl->callback = callback;
}

auto GPIOInStreamServer::removeCallback() -> void {
    std::lock_guard lock(m_pimpl->cbMtx);
    m_pimpl->callback = nullptr;
}
