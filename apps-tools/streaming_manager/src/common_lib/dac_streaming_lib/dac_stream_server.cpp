#include "dac_stream_server.h"

#include <atomic>
#include <filesystem>
#include <mutex>
#include <optional>
#include <thread>

#ifndef _WIN32
#include <syslog.h>
#endif

#include "dac_streaming_application.h"
#include "dac_streaming_manager.h"
#include "data_lib/network_header.h"
#include "logger_lib/file_logger.h"
#include "uio_lib/memory_manager.h"

using namespace dac_streaming_lib;

namespace {

auto toReason(CDACStreamingManager::NotifyResult r) -> DACStopReason {
    switch (r) {
        case CDACStreamingManager::NR_ENDED:
            return DACStopReason::DONE;
        case CDACStreamingManager::NR_BROKEN:
            return DACStopReason::BROKEN;
        case CDACStreamingManager::NR_EMPTY:
            return DACStopReason::EMPTY;
        case CDACStreamingManager::NR_MISSING_FILE:
            return DACStopReason::MISSING_FILE;
        case CDACStreamingManager::NR_MEM_ERROR:
            return DACStopReason::MEM_ERROR;
        case CDACStreamingManager::NR_MEM_MODIFY:
            return DACStopReason::MEM_MODIFY;
        case CDACStreamingManager::NR_SETTINGS_ERROR:
            return DACStopReason::CONFIG_ERROR;
        case CDACStreamingManager::NR_NO_ACTIVE_CNAHHELS:
            return DACStopReason::NO_CHANNELS;
        case CDACStreamingManager::NR_DISCONNECTED:
            return DACStopReason::DISCONNECTED;
        default:
            return DACStopReason::NORMAL;
    }
}

}  // namespace

struct DACStreamServer::Impl {
    DACStreamServer* owner = nullptr;
    std::weak_ptr<Impl> self;
    uio_lib::CHardware::Ptr hw;
    std::mutex mtx;  // start and stop
    std::mutex cbMtx;
    std::shared_ptr<DACServerCallback> callback;
    bool verbose = false;

    CDACStreamingManager::Ptr manager;
    CDACStreamingApplication::Ptr app;
    // Every start() is a new session. A stop from a thread of its own belongs to the session it
    // was asked in; atomic: stopNonBlocking() is also called from start() under mtx.
    std::atomic<uint64_t> session{0};
    std::atomic<bool> active{false};  // the session is not torn down yet

    auto teardown() -> void;
    // of: the session the stop was asked in, none for a stop of whatever runs;
    // own: asked by the pipeline itself (the end of the file), not by a client
    auto stop(DACStopReason reason, std::optional<uint64_t> of = std::nullopt, bool own = false) -> void;
    auto stopNonBlocking(DACStopReason reason, bool own) -> void;
    template <typename F>
    auto notify(F f) -> void {
        std::shared_ptr<DACServerCallback> cb;
        {
            std::lock_guard lock(cbMtx);
            cb = callback;
        }
        if (cb)
            f(cb.get());
    }
};

// The playback stops first (the generator, then the reader of the file), then the memory goes
auto DACStreamServer::Impl::teardown() -> void {
    active = false;
    if (app) {
        app->stop();
        app = nullptr;
    }
    if (manager) {
        manager->stop();
        manager = nullptr;
    }
    uio_lib::CMemoryManager::instance()->releaseMemory(uio_lib::MM_DAC);
}

// A stop asked in an earlier session neither tears down nor reports: the session it was for is
// gone with the start that followed. A stop of the pipeline itself comes after the end of the
// file and may find its session already stopped by the client: it is not reported twice.
auto DACStreamServer::Impl::stop(DACStopReason reason, std::optional<uint64_t> of, bool own) -> void {
    try {
        {
            std::lock_guard lock(mtx);
            if ((of && session != *of) || (own && !active)) {
                if (verbose)
                    aprintf(stdout, "[Streaming] A stop of a session that has ended is ignored\n");
                return;
            }
            teardown();
        }
        notify([&](DACServerCallback* cb) { cb->stopped(owner, reason); });
        if (verbose) {
            aprintf(stdout, "[Streaming] Stop dac server\n");
#ifndef _WIN32
            syslog(LOG_NOTICE, "[Streaming] Stop dac server\n");
#endif
        }
    } catch (std::exception& e) {
        ERROR_LOG("DACStreamServer::stop() %s", e.what())
    }
}

auto DACStreamServer::Impl::stopNonBlocking(DACStopReason reason, bool own) -> void {
    // from the threads of the pipeline itself (the end of a file): it cannot join them
    try {
        auto impl = self.lock();
        uint64_t of = session;
        if (impl)
            std::thread([impl, reason, of, own]() { impl->stop(reason, of, own); }).detach();
    } catch (std::exception& e) {
        ERROR_LOG("DACStreamServer::stopNonBlocking() %s", e.what())
    }
}

DACStreamServer::DACStreamServer(uio_lib::CHardware::Ptr hw) : m_pimpl(std::make_shared<Impl>()) {
    m_pimpl->owner = this;
    m_pimpl->self = m_pimpl;
    m_pimpl->hw = hw;
}

DACStreamServer::~DACStreamServer() {
    removeCallback();
    std::lock_guard lock(m_pimpl->mtx);
    m_pimpl->teardown();
}

auto DACStreamServer::start(const CStreamSettings& settings, dac_channels_t channels, const DACStreamOptions& options) -> bool {
    auto& p = *m_pimpl;
    try {
        std::lock_guard lock(p.mtx);
        p.teardown();
        p.session++;
        p.active = true;
        if (!p.hw) {
            p.stopNonBlocking(DACStopReason::CONFIG_ERROR, true);
            return false;
        }

        // the rate, the calibration and the gain of the outputs
        uio_lib::DACConfig dac;
        dac.rate = settings.getDACSpeed();
        dac.gain5x = {settings.getDACGain(1).value == CStreamSettings::DACGain::X5, settings.getDACGain(2).value == CStreamSettings::DACGain::X5};
        dac.bufferBytes = settings.getMemoryBlockSize();
        if (!p.hw->dacConfigure(dac)) {
            ERROR_LOG("Can't configure the DAC (rate %u)", dac.rate)
            p.stopNonBlocking(DACStopReason::CONFIG_ERROR, true);
            return false;
        }

        // the trigger: a channel in the trigger mode plays on the trigger of the sources
        uio_lib::DACTrigger trig;
        trig.enable = {settings.getDACTriggerMode(1).value == CStreamSettings::State::ON, settings.getDACTriggerMode(2).value == CStreamSettings::State::ON};
        trig.sources = settings.getDACTriggerSources();
        trig.events = static_cast<uio_lib::EventSource>(static_cast<int>(settings.getDACEventSource().value));
        if (!p.hw->dacSetTrigger(trig)) {
            ERROR_LOG("The DAC trigger settings are not taken (sources 0x%X)", trig.sources)
            p.stopNonBlocking(DACStopReason::CONFIG_ERROR, true);
            return false;
        }

        auto self = std::weak_ptr<Impl>(m_pimpl);
        bool file = settings.getDACPassMode().value == CStreamSettings::DACPassMode::DAC_FILE;
        if (file) {
            auto format = settings.getDACFileType();
            auto dir = options.dirPath.empty() ? CStreamSettings::getDACDirPath() : options.dirPath;
            auto path = dir + "/" + settings.getDACFile();
            CDACStreamingManager::DACStream_FileType type;
            if (format.value == CStreamSettings::DataFormat::WAV) {
                type = CDACStreamingManager::WAV_TYPE;
            } else if (format.value == CStreamSettings::DataFormat::TDMS) {
                type = CDACStreamingManager::TDMS_TYPE;
            } else if (format.value == CStreamSettings::DataFormat::BIN) {
                type = CDACStreamingManager::BIN_TYPE;
            } else {
                ERROR_LOG("The DAC plays WAV, TDMS and BIN files only")
                p.stopNonBlocking(DACStopReason::BROKEN, true);
                return false;
            }
            const bool gaps = settings.getDACPlayGaps().value == CStreamSettings::State::ON;
            p.manager = CDACStreamingManager::Create(type, path, settings.getDACRepeat(), settings.getDACRepeatCount(), settings.getMemoryBlockSize(), p.verbose, gaps);
            if (p.manager->getOpenResult() != CReaderController::OR_OK) {
                std::error_code ec;
                bool missing = !std::filesystem::exists(path, ec);
                ERROR_LOG("%s %s", missing ? "There is no file" : "Can't play the file", path.c_str())
                p.stopNonBlocking(missing ? DACStopReason::MISSING_FILE : DACStopReason::BROKEN, true);
                return false;
            }
            p.manager->getChannels(channels);
            if (channels.count() == 0) {
                ERROR_LOG("There are no channels in the file")
                p.stopNonBlocking(DACStopReason::EMPTY, true);
                return false;
            }
            p.manager->notifyStop.connect([self](CDACStreamingManager::NotifyResult r) {
                auto impl = self.lock();
                if (impl)
                    impl->stopNonBlocking(toReason(r), true);
            });
        } else {
            p.manager = CDACStreamingManager::Create(options.host, p.verbose);
            if (channels.count() == 0) {
                ERROR_LOG("No active channels")
                p.stopNonBlocking(DACStopReason::NO_CHANNELS, true);
                return false;
            }
            // the client that sends the data disconnects
            p.manager->notifyStop.connect([self](CDACStreamingManager::NotifyResult r) {
                auto impl = self.lock();
                if (impl)
                    impl->stopNonBlocking(toReason(r), true);
            });
        }

        // memory
        auto memory = uio_lib::CMemoryManager::instance();
        memory->releaseMemory(uio_lib::MM_DAC);
        auto ramSize = memory->getReserverdMemory(uio_lib::MM_DAC);
        if (ramSize < memory->getMinRAMSize(uio_lib::MM_DAC)) {
            ERROR_LOG("Not enough memory for DAC mode")
            p.stopNonBlocking(DACStopReason::MEM_ERROR, true);
            return false;
        }
        auto freeBlocks = ramSize / memory->getMemoryBlockSize();
        auto reserved = memory->reserveMemory(uio_lib::MM_DAC, freeBlocks, channels.count());
        if (reserved == 0) {
            ERROR_LOG("Can't reserve memory via memory manager")
            p.stopNonBlocking(DACStopReason::MEM_ERROR, true);
            return false;
        }
        if (p.verbose)
            aprintf(stdout, "Reserved memory blocks: %d size: %d\n", reserved, reserved * memory->getMemoryBlockSize());
        p.manager->getBufferManager()->generateBuffersEmptyDAC(channels, memory->getRegions(uio_lib::MM_DAC), DataLib::sizeHeader());

        // the data port opens before Started: a port taken is a failed start, not a start
        // that never gets data
        if (!file)
            p.manager->run();
        p.app = std::make_shared<CDACStreamingApplication>(p.manager, p.hw);
        p.app->setVerboseMode(p.verbose);

        // Started goes out before the playback: a short file can end at once, and its stop must
        // not reach the client first
        p.notify([&](DACServerCallback* cb) {
            if (file)
                cb->startedSD(this);
            else
                cb->startedTCP(this);
        });
        p.app->runNonBlock(!file);
        if (p.verbose)
            aprintf(stdout, "[Streaming] Start dac server\n");
        return true;
    } catch (std::exception& e) {
        // a start that throws (the data port taken, ...) says so as the other failed starts do
        ERROR_LOG("DACStreamServer::start() %s", e.what())
        p.stopNonBlocking(DACStopReason::CONFIG_ERROR, true);
    }
    return false;
}

auto DACStreamServer::stop(DACStopReason reason) -> void {
    m_pimpl->stop(reason);
}

auto DACStreamServer::stopNonBlocking(DACStopReason reason) -> void {
    m_pimpl->stopNonBlocking(reason, false);
}

auto DACStreamServer::isRunning() -> bool {
    std::lock_guard lock(m_pimpl->mtx);
    return m_pimpl->app && m_pimpl->app->isRun();
}

auto DACStreamServer::setVerbose(bool enable) -> void {
    m_pimpl->verbose = enable;
}

auto DACStreamServer::setCallback(std::shared_ptr<DACServerCallback> callback) -> void {
    std::lock_guard lock(m_pimpl->cbMtx);
    m_pimpl->callback = callback;
}

auto DACStreamServer::removeCallback() -> void {
    std::lock_guard lock(m_pimpl->cbMtx);
    m_pimpl->callback = nullptr;
}
