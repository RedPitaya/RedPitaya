#include "adc_stream_server.h"

#include <algorithm>
#include <array>
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
#include "net_lib/asio_common.h"
#include "streaming_file.h"
#include "streaming_fpga.h"
#include "streaming_net.h"
#include "uio_lib/memory_manager.h"

using namespace streaming_lib;
using DataLib::CBuffersCached;
using DataLib::CDataBuffersPackDMA;

struct ADCStreamServer::Impl {
    ADCStreamServer* owner = nullptr;
    std::weak_ptr<Impl> self;
    uio_lib::CHardware::Ptr hw;
    std::mutex mtx;  // start and stop
    std::mutex cbMtx;
    std::shared_ptr<ADCServerCallback> callback;
    bool verbose = false;

    CStreamingFPGA::Ptr fpga;
    CBuffersCached::Ptr buffer;
    CStreamingNet::Ptr net;
    CStreamingFile::Ptr file;
    bool adcStarted = false;  // startADC() taken: the FPGA thread of this run exists
    // Every start() is a new session. A stop from a thread of its own belongs to the session it
    // was asked in; atomic: stopNonBlocking() is also called from start() under mtx.
    std::atomic<uint64_t> session{0};
    std::atomic<bool> active{false};  // the session is not torn down yet
    // The losses of the run, per channel: samples the FPGA lost on the board (no free buffer) and
    // the samples it gave; the samples the SD file did not take (kept from the file at its end).
    // Reset by start(), kept after the stop.
    std::array<std::atomic<uint64_t>, 4> boardLost{};
    std::array<std::atomic<uint64_t>, 4> boardSamples{};
    std::atomic<uint64_t> fileDropped{0};
    // the losses count while the stream takes the data: not after its file ended (the sample
    // limit, a full card) or its client left, when the scope runs on until the stop with nobody
    // reading
    std::atomic<bool> counting{false};
    // the run: its destination (0 none yet, 1 network, 2 SD) and the file seen last
    std::atomic<int> runMode{0};
    std::mutex recMtx;
    ADCStreamServer::Record lastRecord;  // under recMtx
    auto refreshRecord() -> void;  // from the file, under mtx

    auto teardown() -> void;
    // of: the session the stop was asked in, none for a stop of whatever runs; own: asked by
    // the server's own threads (the end of a file, a client gone), void once the session ended
    auto stop(ADCStopReason reason, std::optional<uint64_t> of = std::nullopt, bool own = false) -> void;
    auto stopNonBlocking(ADCStopReason reason, bool own) -> void;
    auto notifyStopped(ADCStopReason reason) -> void;
    template <typename F>
    auto notify(F f) -> void {
        std::shared_ptr<ADCServerCallback> cb;
        {
            std::lock_guard lock(cbMtx);
            cb = callback;
        }
        if (cb)
            f(cb.get());
    }
};

// The acquisition stops first, then what is queued for the SD card still goes into the file
auto ADCStreamServer::Impl::teardown() -> void {
    active = false;
    counting = false;
    if (file)
        file->disableNotify();
    if (buffer)
        buffer->notifyToDestory();
    if (fpga) {
        fpga->stop();
        fpga = nullptr;
    }
    adcStarted = false;
    if (file) {
        file->stopAndFlush();
        fileDropped = file->getDroppedSamples();
        refreshRecord();
    }
    buffer = nullptr;
    net = nullptr;
    file = nullptr;
    auto memory = uio_lib::CMemoryManager::instance();
    memory->releaseMemory(uio_lib::MM_ADC);
    memory->releaseMemory(uio_lib::MM_ADC_RESERVE_SKIP);
}

// A stop asked in an earlier session neither tears down nor reports: the session it was for is
// gone with the start that followed, and its reason (a failed start) is not the new one's.
auto ADCStreamServer::Impl::stop(ADCStopReason reason, std::optional<uint64_t> of, bool own) -> void {
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
        notify([&](ADCServerCallback* cb) { cb->stopped(owner, reason); });
        if (verbose) {
            aprintf(stdout, "[Streaming] Stop server\n");
#ifndef _WIN32
            syslog(LOG_NOTICE, "[Streaming] Stop server\n");
#endif
        }
    } catch (std::exception& e) {
        ERROR_LOG("ADCStreamServer::stop() %s", e.what())
    }
}

auto ADCStreamServer::Impl::stopNonBlocking(ADCStopReason reason, bool own) -> void {
    // from the threads of the pipeline itself (the end of a file): it cannot join them
    try {
        auto impl = self.lock();
        uint64_t of = session;
        if (impl)
            std::thread([impl, reason, of, own]() { impl->stop(reason, of, own); }).detach();
    } catch (std::exception& e) {
        ERROR_LOG("ADCStreamServer::stopNonBlocking() %s", e.what())
    }
}

ADCStreamServer::ADCStreamServer(uio_lib::CHardware::Ptr hw) : m_pimpl(std::make_shared<Impl>()) {
    m_pimpl->owner = this;
    m_pimpl->self = m_pimpl;
    m_pimpl->hw = hw;
}

ADCStreamServer::~ADCStreamServer() {
    removeCallback();
    std::lock_guard lock(m_pimpl->mtx);
    m_pimpl->teardown();
}

auto ADCStreamServer::start(const CStreamSettings& settings, const ADCStreamOptions& options) -> bool {
    auto& p = *m_pimpl;
    try {
        std::lock_guard lock(p.mtx);
        p.teardown();
        p.session++;
        p.active = true;
        for (uint8_t ch = 0; ch < 4; ch++)
            p.boardLost[ch] = p.boardSamples[ch] = 0;
        p.fileDropped = 0;
        p.counting = true;
        if (!p.hw) {
            p.stopNonBlocking(ADCStopReason::CONFIG_ERROR, true);
            return false;
        }

        auto board = p.hw->info();
        auto passMode = settings.getADCPassMode();
        bool net = passMode.value == CStreamSettings::PassMode::NET;
        p.runMode = net ? 1 : 2;
        {
            std::lock_guard rec(p.recMtx);
            p.lastRecord = Record();
            p.lastRecord.mode = p.runMode;
            p.lastRecord.limit = net ? 0 : settings.getADCSamples();
        }
        auto decimation = settings.getADCDecimation();
        uint8_t bits = settings.getADCResolution().value == CStreamSettings::Resolution::BIT_8 ? 8 : 16;
#ifdef RP_PLATFORM
        bool buffersTestMode = options.testMode;
        bool isPlatform = true;
#else
        bool buffersTestMode = true;
        bool isPlatform = false;
#endif
        auto memory = uio_lib::CMemoryManager::instance();

        // where the scope writes while no buffer is free
        memory->releaseMemory(uio_lib::MM_ADC_RESERVE_SKIP);
        if (memory->reserveMemory(uio_lib::MM_ADC_RESERVE_SKIP, 1, 1) != 1) {
            ERROR_LOG("Can't reserve memory via memory manager")
            p.stopNonBlocking(ADCStopReason::MEM_ERROR, true);
            return false;
        }
        p.hw->adcSetSkipBuffer(memory->getRegions(uio_lib::MM_ADC_RESERVE_SKIP)[0].start);

        p.buffer = CBuffersCached::create();
        auto bufferW = std::weak_ptr<CBuffersCached>(p.buffer);

        if (net) {
            auto host = options.host;
            p.net = CStreamingNet::create(host, NET_ADC_STREAMING_PORT);
            p.net->getBuffer = [bufferW]() -> CDataBuffersPackDMA::Ptr {
                auto obj = bufferW.lock();
                return obj ? obj->readBuffer() : nullptr;
            };
            p.net->unlockBufferF = [bufferW]() {
                auto obj = bufferW.lock();
                if (obj)
                    obj->unlockBufferRead();
            };
            // the client of the data is gone: nothing reads the stream any more
            auto selfW = std::weak_ptr<Impl>(m_pimpl);
            p.net->clientGone = [selfW]() {
                if (auto impl = selfW.lock()) {
                    impl->counting = false;
                    impl->stopNonBlocking(ADCStopReason::DISCONNECTED, true);
                }
            };
        } else {
            auto path = options.filePath.empty() ? std::string(FILE_PATH) : options.filePath;
            p.file = CStreamingFile::create(
                settings.getADCFormat(), path, settings.getADCSamples(), settings.getADCType().value == CStreamSettings::DataType::VOLT, options.testMode, isPlatform);
            p.file->setTDMSTimeColumn(settings.getTDMSTimeColumn().value == CStreamSettings::State::ON);
            auto self = std::weak_ptr<Impl>(m_pimpl);
            p.file->stopNotify.connect([self](CStreamingFile::EStopReason r) {
                auto impl = self.lock();
                if (!impl)
                    return;
                impl->counting = false;  // the file takes no more
                switch (r) {
                    case CStreamingFile::EStopReason::NORMAL:
                        impl->stopNonBlocking(ADCStopReason::NORMAL, true);
                        break;
                    case CStreamingFile::EStopReason::OUT_SPACE:
                        impl->stopNonBlocking(ADCStopReason::SD_FULL, true);
                        break;
                    case CStreamingFile::EStopReason::REACH_LIMIT:
                        impl->stopNonBlocking(ADCStopReason::DONE, true);
                        break;
                }
            });
        }

        // channels
        uint8_t active = 0;
        for (uint8_t i = 0; i < board.adcChannels && i < 4; i++) {
            if (settings.getADCChannels(i + 1).value == CStreamSettings::State::ON) {
                bool att = settings.getADCAttenuator(i + 1).value == CStreamSettings::Attenuator::A_1_20;
                p.buffer->addChannel((DataLib::EDataBuffersPackChannel)i, bits, att ? DataLib::CDataBufferDMA::ATT_1_20 : DataLib::CDataBufferDMA::ATT_1_1);
                active++;
            }
        }
        if (active == 0) {
            ERROR_LOG("No active channels")
            p.stopNonBlocking(ADCStopReason::NO_CHANNELS, true);
            return false;
        }

        // memory
        memory->releaseMemory(uio_lib::MM_ADC);
        auto ramSize = memory->getReserverdMemory(uio_lib::MM_ADC);
        if (ramSize < memory->getMinRAMSize(uio_lib::MM_ADC)) {
            ERROR_LOG("Not enough memory for ADC mode")
            p.stopNonBlocking(ADCStopReason::MEM_ERROR, true);
            return false;
        }
        auto freeBlocks = ramSize / memory->getMemoryBlockSize();
        if (buffersTestMode)
            freeBlocks = std::min(freeBlocks, 100u);
        auto reserved = memory->reserveMemory(uio_lib::MM_ADC, freeBlocks, active);
        if (reserved == 0) {
            ERROR_LOG("Can't reserve memory via memory manager")
            p.stopNonBlocking(ADCStopReason::MEM_ERROR, true);
            return false;
        }
        if (p.verbose)
            aprintf(stdout, "Reserved memory blocks: %d size: %d\n", reserved, reserved * memory->getMemoryBlockSize());

        // the header goes with the packs over the network only
        p.buffer->generateBuffers(memory->getRegions(uio_lib::MM_ADC), net ? DataLib::sizeHeader() : 0, buffersTestMode);
        p.buffer->setADCBits(board.adcBits);
        p.buffer->setOSCRate(board.adcRate / decimation);
        if (net)
            p.buffer->initHeadersADC();

        // the scope
        uio_lib::ADCConfig adc;
        adc.decimation = decimation;
        adc.eightBit = bits == 8;
        adc.calibration = settings.getADCCalibration();
        adc.bufferBytes = p.buffer->getDataSize();
        for (uint8_t ch = 0; ch < board.adcChannels && ch < 4; ch++) {
            adc.channels[ch] = settings.getADCChannels(ch + 1).value == CStreamSettings::State::ON;
            adc.att1_20[ch] = settings.getADCAttenuator(ch + 1).value == CStreamSettings::Attenuator::A_1_20;
            adc.dc[ch] = settings.getADCAC_DC(ch + 1).value == CStreamSettings::AC_DC::DC;
        }
        if (!p.hw->adcConfigure(adc)) {
            ERROR_LOG("Can't configure the ADC")
            p.stopNonBlocking(ADCStopReason::CONFIG_ERROR, true);
            return false;
        }

        // the trigger (mode, sources, level, blocks) and the external trigger conditioner
        uio_lib::ADCTrigger trig;
        trig.enable = settings.getADCTriggerMode().value == CStreamSettings::State::ON;
        trig.sources = settings.getADCTriggerSources();
        trig.low = settings.getADCTriggerLevelLow();
        trig.high = settings.getADCTriggerLevelHigh();
        trig.falling = settings.getADCTriggerEdge().value == CStreamSettings::TriggerEdge::FALLING;
        trig.events = static_cast<uio_lib::EventSource>(static_cast<int>(settings.getADCEventSource().value));
        auto blocks = settings.getADCTriggerBlocks();
        trig.blocks = static_cast<uint16_t>(std::min<uint32_t>(blocks, UINT16_MAX));
        if (blocks > UINT16_MAX || !p.hw->adcSetTrigger(trig)) {
            ERROR_LOG("The ADC trigger settings are not taken (low %d, high %d, blocks %u)", trig.low, trig.high, blocks)
            p.stopNonBlocking(ADCStopReason::CONFIG_ERROR, true);
            return false;
        }
        if (p.hw->features().extTrigger) {
            uio_lib::ExtTrigger ext;
            ext.conditioner = settings.getExtTrigConditioner().value == CStreamSettings::State::ON;
            ext.falling = settings.getExtTrigEdge().value == CStreamSettings::TriggerEdge::FALLING;
            ext.debounceClocks = settings.getExtTrigDebounce();
            ext.dio0 = settings.getExtTrigDio0().value == CStreamSettings::State::ON;
            if (!p.hw->setExtTrigger(ext)) {
                ERROR_LOG("The external trigger settings are not taken (debounce %u)", ext.debounceClocks)
                p.stopNonBlocking(ADCStopReason::CONFIG_ERROR, true);
                return false;
            }
        }

        p.fpga = std::make_shared<CStreamingFPGA>(p.hw, settings.getADCCaptureTime());
        p.fpga->setVerboseMode(p.verbose);
        p.fpga->setTestMode(options.testMode);
        p.fpga->getBuffF = [bufferW]() -> CDataBuffersPackDMA::Ptr {
            auto obj = bufferW.lock();
            return obj ? obj->writeBuffer() : nullptr;
        };
        p.fpga->unlockBuffF = [bufferW]() {
            auto obj = bufferW.lock();
            if (obj)
                obj->unlockBufferWrite();
        };
        // every buffer of the FPGA, in both modes: what the board lost before it (a buffer it had to
        // drop is counted with the next one)
        auto selfW = std::weak_ptr<Impl>(m_pimpl);
        p.fpga->oscNotify.connect([selfW](CDataBuffersPackDMA::Ptr pack) {
            auto impl = selfW.lock();
            if (!impl || !pack || !impl->counting)
                return;
            for (uint8_t ch = 0; ch < 4; ch++) {
                auto buff = pack->getBuffer((DataLib::EDataBuffersPackChannel)ch);
                if (!buff)
                    continue;
                impl->boardLost[ch] += buff->getLostSamples(DataLib::FPGA);
                impl->boardSamples[ch] += buff->getSamplesCount();
            }
        });
        auto fileW = std::weak_ptr<CStreamingFile>(p.file);
        p.fpga->oscNotify.connect([fileW, bufferW](CDataBuffersPackDMA::Ptr) {
            auto f = fileW.lock();
            auto b = bufferW.lock();
            if (f && b) {
                auto pack = b->readBuffer();
                if (pack) {
                    f->passBuffers(pack);
                    b->unlockBufferRead();
                }
            }
        });

        if (p.net) {
            // zero copy needs the memory of the ring mapped by rp_resmem; the FPGA thread holds two
            // packs (4 channels in 12 blocks are a ring of 3: no pack may wait, it stalled)
            const bool zeroCopy = memory->isZeroCopy();
            p.net->setZeroCopy(zeroCopy, net_lib::zeroCopyMaxPending(p.buffer->ringSize(), 2));
            if (p.verbose)
                aprintf(stdout, "[Streaming] Zero copy send: %s\n", zeroCopy ? "on" : "off");
            p.net->run();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            p.notify([&](ADCServerCallback* cb) { cb->startedTCP(this); });
        }
        if (p.file) {
            char name[40];
            time_t now = time(nullptr);
            strftime(name, sizeof(name), "%Y-%m-%d_%H-%M-%S", gmtime(&now));
            p.file->run(std::string(name));
            p.notify([&](ADCServerCallback* cb) { cb->startedSD(this); });
        }
        if (p.verbose)
            aprintf(stdout, "[Streaming] Start server %s\n", options.testMode ? "[Benchmark mode]" : "");
        return true;
    } catch (std::exception& e) {
        // a start that throws (the data port taken, ...) says so as the other failed starts do
        ERROR_LOG("ADCStreamServer::start() %s", e.what())
        p.stopNonBlocking(ADCStopReason::CONFIG_ERROR, true);
    }
    return false;
}

auto ADCStreamServer::startADC() -> void {
    auto& p = *m_pimpl;
    try {
        std::lock_guard lock(p.mtx);
        // once per run: a second thread over the joinable one would end the process
        if (p.fpga && !p.adcStarted) {
            p.fpga->runNonBlock();
            p.adcStarted = true;
        } else if (p.adcStarted) {
            ERROR_LOG("The ADC already runs, the start is ignored")
        }
    } catch (std::exception& e) {
        ERROR_LOG("ADCStreamServer::startADC() %s", e.what())
    }
}

auto ADCStreamServer::stop(ADCStopReason reason) -> void {
    m_pimpl->stop(reason);
}

auto ADCStreamServer::stopNonBlocking(ADCStopReason reason) -> void {
    m_pimpl->stopNonBlocking(reason, false);
}

auto ADCStreamServer::isRunning() -> bool {
    std::lock_guard lock(m_pimpl->mtx);
    return m_pimpl->fpga && m_pimpl->fpga->isRun();
}

auto ADCStreamServer::setVerbose(bool enable) -> void {
    m_pimpl->verbose = enable;
}

auto ADCStreamServer::setCallback(std::shared_ptr<ADCServerCallback> callback) -> void {
    std::lock_guard lock(m_pimpl->cbMtx);
    m_pimpl->callback = callback;
}

auto ADCStreamServer::removeCallback() -> void {
    std::lock_guard lock(m_pimpl->cbMtx);
    m_pimpl->callback = nullptr;
}

auto ADCStreamServer::getPassSamples() -> uint64_t {
    std::lock_guard lock(m_pimpl->mtx);
    return m_pimpl->file ? m_pimpl->file->getPassSamples() : 0;
}

auto ADCStreamServer::Impl::refreshRecord() -> void {
    if (!file)
        return;
    std::lock_guard rec(recMtx);
    lastRecord.samples = file->getPassSamples();
    lastRecord.written = file->getWritedSize();
    lastRecord.writeRate = file->getWriteRate();
    lastRecord.queue = file->getQueueFill();
    lastRecord.file = file->getCSVFileName();
}

auto ADCStreamServer::getRunMode() -> int {
    return m_pimpl->runMode;
}

auto ADCStreamServer::getRecord() -> Record {
    auto& p = *m_pimpl;
    {
        std::unique_lock lock(p.mtx, std::try_to_lock);
        if (lock)
            p.refreshRecord();
    }
    Record r;
    {
        std::lock_guard rec(p.recMtx);
        r = p.lastRecord;
    }
    r.mode = p.runMode;
    r.running = p.active;
    return r;
}

auto ADCStreamServer::getBoardLost() -> uint64_t {
    uint64_t m = 0;
    for (auto& v : m_pimpl->boardLost)
        m = std::max<uint64_t>(m, v);
    return m;
}

auto ADCStreamServer::getBoardSamples() -> uint64_t {
    uint64_t m = 0;
    for (auto& v : m_pimpl->boardSamples)
        m = std::max<uint64_t>(m, v);
    return m;
}

auto ADCStreamServer::getFileDropped() -> uint64_t {
    // the page asks every 100 ms: a stop that writes the queue out holds mtx for seconds, then the
    // last value seen serves
    std::unique_lock lock(m_pimpl->mtx, std::try_to_lock);
    if (lock && m_pimpl->file)
        m_pimpl->fileDropped = m_pimpl->file->getDroppedSamples();
    return m_pimpl->fileDropped;
}

auto ADCStreamServer::getWriteRate() -> double {
    std::lock_guard lock(m_pimpl->mtx);
    return m_pimpl->file ? m_pimpl->file->getWriteRate() : 0;
}

auto ADCStreamServer::getQueueFill() -> double {
    std::lock_guard lock(m_pimpl->mtx);
    return m_pimpl->file ? m_pimpl->file->getQueueFill() : 0;
}

auto ADCStreamServer::getWritedSize() -> uint64_t {
    std::lock_guard lock(m_pimpl->mtx);
    return m_pimpl->file ? m_pimpl->file->getWritedSize() : 0;
}
