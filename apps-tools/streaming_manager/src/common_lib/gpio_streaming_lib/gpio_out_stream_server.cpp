#include "gpio_out_stream_server.h"

#include <atomic>
#include <chrono>
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

using namespace gpio_streaming_lib;
using DataLib::CBuffersCached;
using DataLib::CDataBufferDMA;
using DataLib::CDataBuffersPackDMA;

namespace {

auto bufferOf(const CDataBuffersPackDMA::Ptr& pack) -> CDataBufferDMA::Ptr {
    return pack ? pack->getBuffer(DataLib::EDataBuffersPackChannel::CH1) : nullptr;
}

constexpr auto kPrefillQuiet = std::chrono::milliseconds(100);  // no buffer for this long: play what came

}  // namespace

struct GPIOOutStreamServer::Impl {
    GPIOOutStreamServer* owner = nullptr;
    std::weak_ptr<Impl> self;
    uio_lib::CHardware::Ptr hw;
    std::mutex mtx;  // start and stop
    std::mutex cbMtx;
    std::shared_ptr<GPIOOutServerCallback> callback;
    bool verbose = false;

    // Every start() is a new session. A stop from a thread of its own belongs to the session it
    // was asked in; atomic: stopNonBlocking() is also called from start() under mtx.
    std::atomic<uint64_t> session{0};
    std::atomic<bool> active{false};  // the session is not torn down yet

    // the run
    std::atomic<bool> run{false};
    std::atomic<bool> playing{false};  // the output started
    CBuffersCached::Ptr ring;
    net_lib::CAsioNet::Ptr net;
    std::thread fpgaThread;  // the client's buffers to the FPGA

    auto teardown() -> uint64_t;
    // of: the session the stop was asked in, none for a stop of whatever runs
    auto stop(GPIOStopReason reason, std::optional<uint64_t> of = std::nullopt) -> void;
    auto stopNonBlocking(GPIOStopReason reason) -> void;
    auto prefill() -> void;
    auto fpgaWork() -> void;
    template <typename F>
    auto notify(F f) -> void {
        std::shared_ptr<GPIOOutServerCallback> cb;
        {
            std::lock_guard lock(cbMtx);
            cb = callback;
        }
        if (cb)
            f(cb.get());
    }
};

// The output stops first (it reads no memory after it), then the network: a receiver waiting
// for a free buffer is released, the thread ends and the memory goes. Gives the clocks the
// output waited for a buffer.
auto GPIOOutStreamServer::Impl::teardown() -> uint64_t {
    active = false;
    bool was = run.exchange(false);
    uint64_t wait = 0;
    if (hw && was) {
        hw->gpioStopOutput();
        if (playing)
            wait = hw->gpioStatus().outputWait;
    }
    playing = false;
    if (ring)
        ring->notifyToDestory();
    if (net) {
        net->stop();
        net->disconnect();
    }
    if (fpgaThread.joinable())
        fpgaThread.join();
    net = nullptr;
    ring = nullptr;
    uio_lib::CMemoryManager::instance()->releaseMemory(uio_lib::MM_GPIO_OUT);
    return wait;
}

// A stop asked in an earlier session neither tears down nor reports: the session it was for is
// gone with the start that followed, and its reason (a failed start) is not the new one's.
auto GPIOOutStreamServer::Impl::stop(GPIOStopReason reason, std::optional<uint64_t> of) -> void {
    try {
        uint64_t wait = 0;
        {
            std::lock_guard lock(mtx);
            if (of && session != *of) {
                if (verbose)
                    aprintf(stdout, "[Streaming] A stop of an earlier session is ignored\n");
                return;
            }
            wait = teardown();
        }
        notify([&](GPIOOutServerCallback* cb) { cb->stopped(owner, reason, wait); });
        if (verbose) {
            aprintf(stdout, "[Streaming] Stop gpio output server, waited %llu clocks for buffers\n", (unsigned long long)wait);
#ifndef _WIN32
            syslog(LOG_NOTICE, "[Streaming] Stop gpio output server\n");
#endif
        }
    } catch (std::exception& e) {
        ERROR_LOG("GPIOOutStreamServer::stop() %s", e.what())
    }
}

auto GPIOOutStreamServer::Impl::stopNonBlocking(GPIOStopReason reason) -> void {
    try {
        auto impl = self.lock();
        uint64_t of = session;
        if (impl)
            std::thread([impl, reason, of]() { impl->stop(reason, of); }).detach();
    } catch (std::exception& e) {
        ERROR_LOG("GPIOOutStreamServer::stopNonBlocking() %s", e.what())
    }
}

// The client's first buffers fill the ring before the output plays: a reserve for a client that
// sends unevenly. The receiver holds one pack while it reads into it: the ring is full with one
// less. A client that sends less plays 100 ms after its last buffer.
auto GPIOOutStreamServer::Impl::prefill() -> void {
    uint32_t seen = 0;
    auto last = std::chrono::steady_clock::now();
    const uint32_t full = ring->ringSize() > 1 ? ring->ringSize() - 1 : 1;
    while (run) {
        auto n = ring->filled();
        auto now = std::chrono::steady_clock::now();
        if (n != seen) {
            seen = n;
            last = now;
        }
        if (n >= full || (n > 0 && now - last >= kPrefillQuiet))
            return;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// The buffers go to the two slots of the FPGA in turn. The FPGA reads a buffer until its slot
// takes the next but one: a pack goes back to the ring only then. A buffer filled in part (the
// end of the client's data) is filled up with its last lines. The ready flags of the slots are
// cleared while the output DMA is stopped: the first buffer plays without one, so the output
// starts right after it and the second is handed over running.
auto GPIOOutStreamServer::Impl::fpgaWork() -> void {
    uint8_t index = 0;
    int handedOver = 0;
    try {
        prefill();
        while (run) {
            auto pack = ring->readBuffer();
            if (!pack)
                continue;
            auto buffer = bufferOf(pack);
            DataLib::GPIOHeader header;
            size_t size = buffer->getDataLenght();
            if (DataLib::getHeaderGPIO(buffer, header) && header.dataSize < size)
                size = header.dataSize & ~3u;
            if (size == 0) {
                ring->unlockBufferRead();
                continue;
            }
            auto words = static_cast<uint32_t*>(buffer->getMappedDataMemory());
            auto count = buffer->getDataLenght() / 4;
            for (size_t i = size / 4; i < count; i++)
                words[i] = words[size / 4 - 1] & 0xFFFF0000u;  // the same lines, one sample each

            bool taken = false;
            while (run && !(taken = hw->gpioSetOutBuffer(index, buffer->getDataAddress())))
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            handedOver++;  // a pack not taken (the stop) goes back with the rest
            if (!taken)
                break;
            index ^= 1;
            if (handedOver == 1) {
                hw->gpioStartOutput();
                playing = true;
                if (verbose)
                    aprintf(stdout, "[GPIO] The output plays, %u buffers in the ring\n", ring->filled() + 1);
            }
            if (handedOver > 2) {
                ring->unlockBufferRead();
                handedOver--;
            }
        }
    } catch (std::exception& e) {
        ERROR_LOG("GPIOOutStreamServer output %s", e.what())
    }
    // the output is stopped by now (teardown): the packs still held go back
    for (; handedOver > 0; handedOver--)
        ring->unlockBufferRead();
}

GPIOOutStreamServer::GPIOOutStreamServer(uio_lib::CHardware::Ptr hw) : m_pimpl(std::make_shared<Impl>()) {
    m_pimpl->owner = this;
    m_pimpl->self = m_pimpl;
    m_pimpl->hw = hw;
}

GPIOOutStreamServer::~GPIOOutStreamServer() {
    removeCallback();
    std::lock_guard lock(m_pimpl->mtx);
    m_pimpl->teardown();
}

auto GPIOOutStreamServer::start(const CStreamSettings& settings, const GPIOStreamOptions& options) -> bool {
    auto& p = *m_pimpl;
    try {
        std::lock_guard lock(p.mtx);
        p.teardown();
        p.session++;
        p.active = true;
        if (!p.hw)
            return false;
        if (!p.hw->features().gpio) {
            ERROR_LOG("There is no GPIO block in the FPGA")
            p.stopNonBlocking(GPIOStopReason::CONFIG_ERROR);
            return false;
        }
        if (!p.hw->features().gpioOutRefill) {
            ERROR_LOG("The GPIO output of this FPGA plays two buffers only, it can't stream")
            p.stopNonBlocking(GPIOStopReason::CONFIG_ERROR);
            return false;
        }

        uio_lib::GPIOConfig cfg;
        cfg.outputsP = settings.getGPIOOutputsP();
        cfg.outputsN = settings.getGPIOOutputsN();
        cfg.rle = settings.getGPIORLE().value == CStreamSettings::State::ON;
        cfg.outStep = settings.getGPIOOutStep();
        cfg.bufferBytes = settings.getMemoryBlockSize();
        uio_lib::GPIOTrigger trig;
        trig.output = settings.getGPIOOutTriggerMode().value == CStreamSettings::State::ON;
        trig.sources = settings.getGPIOTriggerSources();
        trig.events = static_cast<uio_lib::EventSource>(static_cast<int>(settings.getGPIOEventSource().value));
        trig.patternMask = settings.getGPIOPatternMask();
        trig.patternValue = settings.getGPIOPatternValue();
        trig.patternPosEdge = settings.getGPIOPatternPosEdge();
        trig.patternNegEdge = settings.getGPIOPatternNegEdge();
        if (!p.hw->gpioConfigureOutput(cfg) || !p.hw->gpioSetOutputTrigger(trig)) {
            ERROR_LOG("The GPIO output settings are not taken (block %u bytes; the input may run with others)", cfg.bufferBytes)
            p.stopNonBlocking(GPIOStopReason::CONFIG_ERROR);
            return false;
        }

        auto memory = uio_lib::CMemoryManager::instance();
        memory->releaseMemory(uio_lib::MM_GPIO_OUT);
        auto ramSize = memory->getReserverdMemory(uio_lib::MM_GPIO_OUT);
        if (ramSize < memory->getMinRAMSize(uio_lib::MM_GPIO_OUT)) {
            ERROR_LOG("Not enough memory for the GPIO output")
            p.stopNonBlocking(GPIOStopReason::MEM_ERROR);
            return false;
        }
        auto reserved = memory->reserveMemory(uio_lib::MM_GPIO_OUT, ramSize / memory->getMemoryBlockSize(), 1);
        if (reserved < 4) {  // two in the FPGA, one in the receiver, one to fill
            ERROR_LOG("Can't reserve memory via memory manager")
            p.stopNonBlocking(GPIOStopReason::MEM_ERROR);
            return false;
        }
        if (p.verbose)
            aprintf(stdout, "Reserved memory blocks: %d size: %d\n", reserved, reserved * memory->getMemoryBlockSize());
        adc_channels_t one;
        one[ADCChannels::ADC_CH1] = true;
        p.ring = CBuffersCached::create();
        p.ring->generateBuffersEmptyADC(one, memory->getRegions(uio_lib::MM_GPIO_OUT), DataLib::sizeHeader());

        p.hw->gpioPrepareOutput();
        p.net = net_lib::CAsioNet::create(net_lib::M_SERVER, options.host, NET_GPIO_OUT_STREAMING_PORT, p.ring);
        p.net->setPackID(DataLib::ID_PACK_GPIO);
        p.net->start();
        p.run = true;
        p.fpgaThread = std::thread(&Impl::fpgaWork, &p);
        p.notify([&](GPIOOutServerCallback* cb) { cb->started(this); });
        if (p.verbose)
            aprintf(stdout, "[Streaming] Start gpio output server\n");
        return true;
    } catch (std::exception& e) {
        ERROR_LOG("GPIOOutStreamServer::start() %s", e.what())
        p.stopNonBlocking(GPIOStopReason::CONFIG_ERROR);
    }
    return false;
}

auto GPIOOutStreamServer::stop(GPIOStopReason reason) -> void {
    m_pimpl->stop(reason);
}

auto GPIOOutStreamServer::stopNonBlocking(GPIOStopReason reason) -> void {
    m_pimpl->stopNonBlocking(reason);
}

auto GPIOOutStreamServer::isRunning() -> bool {
    return m_pimpl->active && m_pimpl->run;
}

auto GPIOOutStreamServer::waitClocks() -> uint64_t {
    auto& p = *m_pimpl;
    return p.playing && p.hw ? p.hw->gpioStatus().outputWait : 0;
}

auto GPIOOutStreamServer::setVerbose(bool enable) -> void {
    m_pimpl->verbose = enable;
}

auto GPIOOutStreamServer::setCallback(std::shared_ptr<GPIOOutServerCallback> callback) -> void {
    std::lock_guard lock(m_pimpl->cbMtx);
    m_pimpl->callback = callback;
}

auto GPIOOutStreamServer::removeCallback() -> void {
    std::lock_guard lock(m_pimpl->cbMtx);
    m_pimpl->callback = nullptr;
}
