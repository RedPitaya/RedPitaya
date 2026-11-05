#include "gpio_stream_server.h"

#include <atomic>
#include <cstring>
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

struct GPIOStreamServer::Impl {
    GPIOStreamServer* owner = nullptr;
    std::weak_ptr<Impl> self;
    uio_lib::CHardware::Ptr hw;
    std::mutex mtx;  // start and stop
    std::mutex cbMtx;
    std::shared_ptr<GPIOServerCallback> callback;
    bool verbose = false;

    // Every start() is a new session. A stop from a thread of its own belongs to the session it
    // was asked in; atomic: stopNonBlocking() is also called from start() under mtx.
    std::atomic<uint64_t> session{0};
    std::atomic<bool> active{false};  // the session is not torn down yet

    // the run
    std::atomic<bool> run{false};
    bool in = false;
    bool out = false;
    CBuffersCached::Ptr inRing;
    CBuffersCached::Ptr outRing;
    net_lib::CAsioNet::Ptr inNet;
    net_lib::CAsioNet::Ptr outNet;
    std::thread inThread;    // FPGA input buffers to the ring
    std::thread sendThread;  // the ring to the client
    std::thread outThread;   // the client's buffers to the FPGA
    CDataBuffersPackDMA::Ptr inPacks[2];
    DataLib::GPIOHeader inHeader;  // what every input buffer of the run carries
    bool rle = false;
    std::mutex startMtx;
    bool blockStarted = false;

    auto teardown() -> void;
    // of: the session the stop was asked in, none for a stop of whatever runs
    auto stop(GPIOStopReason reason, std::optional<uint64_t> of = std::nullopt) -> void;
    auto stopNonBlocking(GPIOStopReason reason) -> void;
    auto startBlock() -> void;
    auto inWork() -> void;
    auto sendWork() -> void;
    auto outWork() -> void;
    template <typename F>
    auto notify(F f) -> void {
        std::shared_ptr<GPIOServerCallback> cb;
        {
            std::lock_guard lock(cbMtx);
            cb = callback;
        }
        if (cb)
            f(cb.get());
    }
};

// The block stops first, then the network: a sender blocked on a client that does not read and a
// receiver waiting for a free buffer are released, then the threads end and the memory goes
auto GPIOStreamServer::Impl::teardown() -> void {
    active = false;
    run = false;
    if (hw)
        hw->gpioStop();
    if (inRing)
        inRing->notifyToDestory();
    if (outRing)
        outRing->notifyToDestory();
    if (inNet)
        inNet->disconnect();
    if (outNet) {
        outNet->stop();
        outNet->disconnect();
    }
    for (auto t : {&inThread, &sendThread, &outThread})
        if (t->joinable())
            t->join();
    inNet = nullptr;
    outNet = nullptr;
    inPacks[0] = inPacks[1] = nullptr;
    inRing = nullptr;
    outRing = nullptr;
    blockStarted = false;
    uio_lib::CMemoryManager::instance()->releaseMemory(uio_lib::MM_GPIO);
}

// A stop asked in an earlier session neither tears down nor reports: the session it was for is
// gone with the start that followed, and its reason (a failed start) is not the new one's.
auto GPIOStreamServer::Impl::stop(GPIOStopReason reason, std::optional<uint64_t> of) -> void {
    try {
        {
            std::lock_guard lock(mtx);
            if (of && session != *of) {
                if (verbose)
                    aprintf(stdout, "[Streaming] A stop of an earlier session is ignored\n");
                return;
            }
            teardown();
        }
        notify([&](GPIOServerCallback* cb) { cb->stopped(owner, reason); });
        if (verbose) {
            aprintf(stdout, "[Streaming] Stop gpio server\n");
#ifndef _WIN32
            syslog(LOG_NOTICE, "[Streaming] Stop gpio server\n");
#endif
        }
    } catch (std::exception& e) {
        ERROR_LOG("GPIOStreamServer::stop() %s", e.what())
    }
}

auto GPIOStreamServer::Impl::stopNonBlocking(GPIOStopReason reason) -> void {
    try {
        auto impl = self.lock();
        uint64_t of = session;
        if (impl)
            std::thread([impl, reason, of]() { impl->stop(reason, of); }).detach();
    } catch (std::exception& e) {
        ERROR_LOG("GPIOStreamServer::stopNonBlocking() %s", e.what())
    }
}

auto GPIOStreamServer::Impl::startBlock() -> void {
    std::lock_guard lock(startMtx);
    if (blockStarted || !run)
        return;
    hw->gpioStart();
    blockStarted = true;
    if (verbose)
        aprintf(stdout, "[GPIO] Started the block: input %d output %d\n", in, out);
}

// The input buffers of the FPGA: a full one goes to the ring with its header, a free pack takes
// its place. Without a free pack within a second the buffer is dropped and written again; its
// samples count as lost before the next buffer.
auto GPIOStreamServer::Impl::inWork() -> void {
    uint8_t index = 0;
    uint64_t droppedLost = 0;
    uint64_t droppedWait = 0;
    try {
        while (run) {
            if (!hw->gpioWaitIn(index, 100))
                continue;
            auto info = hw->gpioInBuffer(index);
            auto full = inPacks[index];
            auto next = inRing->writeBuffer(true);
            auto buffer = bufferOf(full);
            if (next) {
                auto header = inHeader;
                header.lostFPGA = info.lost + droppedLost;
                header.waitFPGA = info.wait + droppedWait;
                header.triggerCount = info.triggerCount;
                header.dataSize = buffer->getDataLenght();
                DataLib::setHeaderGPIO(buffer, 0, header);
                inRing->unlockBufferWrite();
                droppedLost = droppedWait = 0;
                inPacks[index] = next;
                hw->gpioSetInBuffer(index, bufferOf(next)->getDataAddress());
            } else {
                droppedLost += info.lost + samplesOf(buffer, rle);
                droppedWait += info.wait;
            }
            hw->gpioReleaseInBuffer(index);
            index ^= 1;
        }
    } catch (std::exception& e) {
        ERROR_LOG("GPIOStreamServer input %s", e.what())
    }
}

// Without a client the buffers are dropped here, as the ADC does
auto GPIOStreamServer::Impl::sendWork() -> void {
    uint64_t id = 0;
    try {
        while (run) {
            auto pack = inRing->readBuffer();
            if (!pack)
                continue;
            if (inNet->isConnected()) {
                auto buffer = bufferOf(pack);
                DataLib::GPIOHeader header;
                if (DataLib::getHeaderGPIO(buffer, header))
                    DataLib::setHeaderGPIO(buffer, id, header);
                inNet->sendSyncData(pack);
                id++;
            }
            inRing->unlockBufferRead();
        }
    } catch (std::exception& e) {
        ERROR_LOG("GPIOStreamServer sender %s", e.what())
    }
}

// The output buffers from the client go to the two slots of the FPGA in turn. The FPGA reads a
// buffer until its slot takes the next but one: a pack goes back to the ring only then. A buffer
// filled in part (the end of the client's data) is filled up with its last lines.
// The ready flags of the slots are cleared while the output DMA is stopped: the first buffer
// plays without one, so the block starts right after it and the second is handed over running.
auto GPIOStreamServer::Impl::outWork() -> void {
    uint8_t index = 0;
    int handedOver = 0;
    try {
        while (run) {
            auto pack = outRing->readBuffer();
            if (!pack)
                continue;
            auto buffer = bufferOf(pack);
            DataLib::GPIOHeader header;
            size_t size = buffer->getDataLenght();
            if (DataLib::getHeaderGPIO(buffer, header) && header.dataSize < size)
                size = header.dataSize & ~3u;
            if (size == 0) {
                outRing->unlockBufferRead();
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
            if (handedOver == 1)
                startBlock();
            if (handedOver > 2) {
                outRing->unlockBufferRead();
                handedOver--;
            }
        }
    } catch (std::exception& e) {
        ERROR_LOG("GPIOStreamServer output %s", e.what())
    }
    // the block is stopped by now (teardown): the packs still held go back
    for (; handedOver > 0; handedOver--)
        outRing->unlockBufferRead();
}

GPIOStreamServer::GPIOStreamServer(uio_lib::CHardware::Ptr hw) : m_pimpl(std::make_shared<Impl>()) {
    m_pimpl->owner = this;
    m_pimpl->self = m_pimpl;
    m_pimpl->hw = hw;
}

GPIOStreamServer::~GPIOStreamServer() {
    removeCallback();
    std::lock_guard lock(m_pimpl->mtx);
    m_pimpl->teardown();
}

auto GPIOStreamServer::start(const CStreamSettings& settings, const GPIOStreamOptions& options) -> bool {
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
        p.in = settings.getGPIOInStreaming().value == CStreamSettings::State::ON;
        p.out = settings.getGPIOOutStreaming().value == CStreamSettings::State::ON;
        if (!p.in && !p.out) {
            ERROR_LOG("Neither the GPIO input nor the output is on")
            p.stopNonBlocking(GPIOStopReason::CONFIG_ERROR);
            return false;
        }
        if (p.out && !p.hw->features().gpioOutRefill) {
            ERROR_LOG("The GPIO output of this FPGA plays two buffers only, it can't stream")
            p.stopNonBlocking(GPIOStopReason::CONFIG_ERROR);
            return false;
        }

        // the block
        uio_lib::GPIOConfig cfg;
        cfg.input = p.in;
        cfg.output = p.out;
        cfg.outputsP = settings.getGPIOOutputsP();
        cfg.outputsN = settings.getGPIOOutputsN();
        cfg.polarity = settings.getGPIOPolarity();
        cfg.decimation = settings.getGPIODecimation();
        cfg.rle = settings.getGPIORLE().value == CStreamSettings::State::ON;
        cfg.outStep = settings.getGPIOOutStep();
        cfg.bufferBytes = settings.getMemoryBlockSize();
        uio_lib::GPIOTrigger trig;
        trig.input = p.in && settings.getGPIOInTriggerMode().value == CStreamSettings::State::ON;
        trig.output = p.out && settings.getGPIOOutTriggerMode().value == CStreamSettings::State::ON;
        trig.sources = settings.getGPIOTriggerSources();
        trig.events = static_cast<uio_lib::EventSource>(static_cast<int>(settings.getGPIOEventSource().value));
        auto blocks = settings.getGPIOTriggerBlocks();
        trig.blocks = static_cast<uint16_t>(std::min<uint32_t>(blocks, UINT16_MAX));
        trig.patternMask = settings.getGPIOPatternMask();
        trig.patternValue = settings.getGPIOPatternValue();
        trig.patternPosEdge = settings.getGPIOPatternPosEdge();
        trig.patternNegEdge = settings.getGPIOPatternNegEdge();
        if (!p.hw->gpioConfigure(cfg) || blocks > UINT16_MAX || !p.hw->gpioSetTrigger(trig)) {
            ERROR_LOG("The GPIO settings are not taken (decimation %u, block %u bytes, trigger blocks %u)", cfg.decimation, cfg.bufferBytes, blocks)
            p.stopNonBlocking(GPIOStopReason::CONFIG_ERROR);
            return false;
        }
        p.rle = cfg.rle;
        p.inHeader = DataLib::GPIOHeader();
        p.inHeader.decimation = cfg.decimation;
        // the block runs on the DAC clock; a board without a DAC (4 inputs) has the ADC one
        p.inHeader.gpioClock = p.hw->info().dacRate ? p.hw->info().dacRate : p.hw->info().adcRate;
        p.inHeader.triggerBlocks = trig.blocks;
        p.inHeader.flags = (cfg.rle ? DataLib::GPIO_FLAG_RLE : 0) | (trig.input ? DataLib::GPIO_FLAG_TRIGGER_MODE : 0) |
                           (p.hw->features().gpioWaitCounter ? DataLib::GPIO_FLAG_WAIT_COUNTER : 0);

        // memory: half for each direction
        auto memory = uio_lib::CMemoryManager::instance();
        memory->releaseMemory(uio_lib::MM_GPIO);
        auto ramSize = memory->getReserverdMemory(uio_lib::MM_GPIO);
        if (ramSize < memory->getMinRAMSize(uio_lib::MM_GPIO)) {
            ERROR_LOG("Not enough memory for GPIO mode")
            p.stopNonBlocking(GPIOStopReason::MEM_ERROR);
            return false;
        }
        uint32_t directions = (p.in ? 1 : 0) + (p.out ? 1 : 0);
        auto reserved = memory->reserveMemory(uio_lib::MM_GPIO, ramSize / memory->getMemoryBlockSize(), directions);
        if (reserved == 0) {
            ERROR_LOG("Can't reserve memory via memory manager")
            p.stopNonBlocking(GPIOStopReason::MEM_ERROR);
            return false;
        }
        if (p.verbose)
            aprintf(stdout, "Reserved memory blocks: %d size: %d\n", reserved, reserved * memory->getMemoryBlockSize());
        auto regions = memory->getRegions(uio_lib::MM_GPIO);
        auto half = regions.begin() + regions.size() / directions;
        adc_channels_t one;
        one[ADCChannels::ADC_CH1] = true;
        if (p.in) {
            p.inRing = CBuffersCached::create();
            p.inRing->generateBuffersEmptyADC(one, std::vector<uio_lib::MemoryRegionT>(regions.begin(), half), DataLib::sizeHeader());
        }
        if (p.out) {
            p.outRing = CBuffersCached::create();
            p.outRing->generateBuffersEmptyADC(one, std::vector<uio_lib::MemoryRegionT>(p.in ? half : regions.begin(), regions.end()), DataLib::sizeHeader());
        }

        p.hw->gpioPrepare();
        p.run = true;
        if (p.in) {
            for (uint8_t i = 0; i < 2; i++) {
                p.inPacks[i] = p.inRing->writeBuffer(true);
                p.hw->gpioSetInBuffer(i, bufferOf(p.inPacks[i])->getDataAddress());
            }
            p.inNet = net_lib::CAsioNet::create(net_lib::M_SERVER, options.host, NET_GPIO_IN_STREAMING_PORT, nullptr);
            p.inNet->start();
            p.inThread = std::thread(&Impl::inWork, &p);
            p.sendThread = std::thread(&Impl::sendWork, &p);
        }
        if (p.out) {
            p.outNet = net_lib::CAsioNet::create(net_lib::M_SERVER, options.host, NET_GPIO_OUT_STREAMING_PORT, p.outRing);
            p.outNet->setPackID(DataLib::ID_PACK_GPIO);
            p.outNet->start();
            p.outThread = std::thread(&Impl::outWork, &p);
        } else {
            p.startBlock();
        }
        p.notify([&](GPIOServerCallback* cb) { cb->started(this); });
        if (p.verbose)
            aprintf(stdout, "[Streaming] Start gpio server: input %d output %d\n", p.in, p.out);
        return true;
    } catch (std::exception& e) {
        ERROR_LOG("GPIOStreamServer::start() %s", e.what())
        p.stopNonBlocking(GPIOStopReason::CONFIG_ERROR);
    }
    return false;
}

auto GPIOStreamServer::stop(GPIOStopReason reason) -> void {
    m_pimpl->stop(reason);
}

auto GPIOStreamServer::stopNonBlocking(GPIOStopReason reason) -> void {
    m_pimpl->stopNonBlocking(reason);
}

auto GPIOStreamServer::isRunning() -> bool {
    return m_pimpl->active && m_pimpl->run;
}

auto GPIOStreamServer::setVerbose(bool enable) -> void {
    m_pimpl->verbose = enable;
}

auto GPIOStreamServer::setCallback(std::shared_ptr<GPIOServerCallback> callback) -> void {
    std::lock_guard lock(m_pimpl->cbMtx);
    m_pimpl->callback = callback;
}

auto GPIOStreamServer::removeCallback() -> void {
    std::lock_guard lock(m_pimpl->cbMtx);
    m_pimpl->callback = nullptr;
}
