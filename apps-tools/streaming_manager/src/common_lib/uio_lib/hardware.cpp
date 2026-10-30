#include "hardware.h"

#include <algorithm>
#include <mutex>

#include "ext_trigger.h"
#include "generator.h"
#include "gpio.h"
#include "oscilloscope.h"
#include "trigger.h"
#include "uio_parser.h"

using namespace uio_lib;

namespace {

constexpr const char* kScopeNode = "rp_oscilloscope@40000000";
constexpr const char* kDacNode = "rp_dac@40100000";

// The uio device of a block; on the host the dummy modules take an empty one
auto findUio(const std::vector<UioT>& list, const char* node, UioT& uio) -> bool {
#ifdef RP_PLATFORM
    for (const auto& u : list) {
        if (u.nodeName == node) {
            uio = u;
            return true;
        }
    }
    return false;
#else
    (void)list;
    (void)node;
    uio = UioT();
    return true;
#endif
}

auto eventId(EventSource source, uint32_t own) -> uint32_t {
    return source == EventSource::OWN ? own : static_cast<uint32_t>(source);
}

}  // namespace

struct CHardware::Impl {
    std::shared_ptr<IBoard> board;
    BoardInfo info;
    HardwareFeatures features;

    COscilloscope::Ptr osc;
    CGenerator::Ptr gen;
    CGpio::Ptr gpio;
    CExtTrigger::Ptr ext;

    // configuration calls of a block; the stream calls (wait, buffers) come from one thread
    std::mutex adcMtx;
    std::mutex dacMtx;
    std::mutex gpioMtx;
    std::mutex extMtx;

    ADCConfig adcCfg;
    ADCTrigger adcTrig;
    DACConfig dacCfg;
    GPIOConfig gpioCfg;
    GPIOTrigger gpioTrig;
};

auto CHardware::create(std::shared_ptr<IBoard> board) -> Ptr {
    if (!board)
        return nullptr;
    return Ptr(new CHardware(board));
}

CHardware::CHardware(std::shared_ptr<IBoard> board) : m_pimpl(new Impl()) {
    auto& p = *m_pimpl;
    p.board = board;
    p.info = board->info();
    auto list = GetUioList();

    UioT uio;
    if (findUio(list, kScopeNode, uio)) {
        // the daisy chain role is read from the scope; it fixes the pre-trigger samples and the
        // start of the DMA, so the scope is created again with it
        bool slave = false;
        if (p.info.daisyChain) {
            auto probe = COscilloscope::create(uio, 1, true, p.info.adcRate, p.info.adcFilter, p.info.adcBits, p.info.adcChannels);
            slave = probe && probe->isMaster() == BoardMode::SLAVE;
        }
        p.osc = COscilloscope::create(uio, 1, !slave, p.info.adcRate, p.info.adcFilter, p.info.adcBits, p.info.adcChannels);
        p.features.slave = slave;
    }
    if (findUio(list, kDacNode, uio))
        p.gen = CGenerator::create(uio, true, true, p.info.dacRate, p.info.dacRate);
    p.gpio = CGpio::create(list);
    p.ext = CExtTrigger::create(list);

    if (p.osc) {
        p.features.adc = true;
        p.features.adcBlockMode = p.osc->hasBlockMode();
        p.features.adcWaitCounter = p.osc->hasWaitCounter();
    }
    if (p.gen) {
        p.features.dac = true;
        p.features.dacLoop = p.gen->hasLoopMode();
    }
    if (p.gpio) {
        p.features.gpio = true;
        p.features.gpioTriggerMode = p.gpio->hasTriggerMode();
        p.features.gpioBlockMode = p.gpio->hasInputBlockMode();
        p.features.gpioWaitCounter = p.gpio->hasWaitCounter();
    }
    p.features.extTrigger = p.ext != nullptr;

    // after the bitstream load every event selector is 0 (DAC A): each block on its own events
    if (p.osc)
        p.osc->stop();
    if (p.gen)
        p.gen->stop();
    if (p.gpio)
        p.gpio->stop();
}

CHardware::~CHardware() {
    delete m_pimpl;
}

auto CHardware::info() const -> BoardInfo {
    return m_pimpl->info;
}

auto CHardware::features() const -> HardwareFeatures {
    return m_pimpl->features;
}

auto CHardware::setExtTrigger(const ExtTrigger& cfg) -> bool {
    auto& p = *m_pimpl;
    std::lock_guard lock(p.extMtx);
    if (!p.ext)
        return false;
    p.ext->setConditioner(cfg.conditioner);
    p.ext->setEdge(cfg.falling);
    bool ok = p.ext->setDebounce(cfg.debounceClocks != 0, cfg.debounceClocks);
    ok = p.ext->setDio0Trigger(cfg.dio0) && ok;
    return ok;
}

// ---------------------------------------------------------------------------------------------
// ADC
// ---------------------------------------------------------------------------------------------

auto CHardware::adcConfigure(const ADCConfig& cfg) -> bool {
    auto& p = *m_pimpl;
    std::lock_guard lock(p.adcMtx);
    if (!p.osc || p.osc->isRunning())
        return false;
    if (!p.osc->setDecimation(cfg.decimation))
        return false;
    if (cfg.calibration && !p.board->loadCalibration())
        return false;
    p.osc->set8BitMode(cfg.eightBit);
    p.osc->setFilterBypass(!p.info.adcFilter);
    // offset and gain before the filter (calibration v6) or after it; without the calibration
    // the codes pass unchanged, so the place does not matter
    bool beforeFilter = cfg.calibration && p.board->adcCalibrationPlace() == ADCCalibrationPlace::BEFORE_FILTER;
    for (uint8_t ch = 0; ch < p.info.adcChannels && ch < 4; ch++) {
        p.board->setADCFrontEnd(ch, cfg.att1_20[ch], cfg.dc[ch]);
        ADCCalibration c;
        if (cfg.calibration)
            c = p.board->adcCalibration(ch, cfg.att1_20[ch], cfg.dc[ch]);
        if (!p.info.adcFilter)
            c.aa = 0, c.bb = 0, c.kk = 0xFFFFFF, c.pp = 0;
        p.osc->setCalibration(ch, c.offset, c.gain, beforeFilter);
        p.osc->setFilterCalibration(ch, c.aa, c.bb, c.kk, c.pp);
    }
    if (cfg.bufferBytes)
        p.osc->setDataSize(cfg.bufferBytes);
    p.adcCfg = cfg;
    return true;
}

auto CHardware::adcSetTrigger(const ADCTrigger& trig) -> bool {
    auto& p = *m_pimpl;
    std::lock_guard lock(p.adcMtx);
    if (!p.osc || p.osc->isRunning())
        return false;
    if (trig.blocks && !p.features.adcBlockMode)
        return false;
    bool ok = p.osc->setTriggerMode(trig.enable);
    ok = p.osc->setTriggerSources(trig.sources) && ok;
    ok = p.osc->setTriggerLevel(trig.low, trig.high) && ok;
    ok = p.osc->setTriggerEdge(trig.falling) && ok;
    ok = p.osc->setEventSource(eventId(trig.events, EVENT_SRC_OSC1)) && ok;
    ok = p.osc->setTriggerBlocks(trig.blocks) && ok;
    if (ok)
        p.adcTrig = trig;
    return ok;
}

auto CHardware::adcConfig() const -> ADCConfig {
    std::lock_guard lock(m_pimpl->adcMtx);
    return m_pimpl->adcCfg;
}

auto CHardware::adcTrigger() const -> ADCTrigger {
    std::lock_guard lock(m_pimpl->adcMtx);
    return m_pimpl->adcTrig;
}

auto CHardware::adcSetSkipBuffer(uint32_t address) -> void {
    auto& p = *m_pimpl;
    if (!p.osc)
        return;
    MemoryRegionT region;
    region.start = address;
    p.osc->setSkipDataAddress(region);
}

auto CHardware::adcSetBuffer(uint8_t index, const std::array<uint32_t, 4>& addresses) -> void {
    auto& p = *m_pimpl;
    if (p.osc)
        p.osc->setDataAddress(index, addresses[0], addresses[1], addresses[2], addresses[3]);
}

auto CHardware::adcStart() -> void {
    auto& p = *m_pimpl;
    std::lock_guard lock(p.adcMtx);
    if (p.osc)
        p.osc->prepare();
}

auto CHardware::adcWait() -> bool {
    auto& p = *m_pimpl;
    return p.osc && p.osc->wait();
}

auto CHardware::adcBuffer(uint8_t index) -> ADCBufferInfo {
    auto& p = *m_pimpl;
    ADCBufferInfo r;
    if (!p.osc)
        return r;
    r.time = p.osc->getBufferTime(index);
    // The 64 bit counter of each channel, latched when the buffer started: the samples missing
    // right before it. The part not written while waiting for a trigger (the series gap of the
    // block mode) is a pause, not a loss: it goes apart, the loss keeps the overflow only.
    r.wait = p.osc->getBufferWait(index);
    for (uint8_t ch = 0; ch < 4; ch++) {
        uint64_t all = p.osc->getBufferLost(index, ch);
        r.lost[ch] = all - std::min(all, r.wait);
    }
    r.triggerCount = p.osc->getTriggerCount();
    return r;
}

auto CHardware::adcReleaseBuffer(uint8_t index) -> void {
    auto& p = *m_pimpl;
    if (p.osc)
        p.osc->clearBuffer(index);
}

auto CHardware::adcStop() -> void {
    auto& p = *m_pimpl;
    std::lock_guard lock(p.adcMtx);
    if (p.osc)
        p.osc->stop();
}

auto CHardware::adcSoftwareTrigger() -> void {
    auto& p = *m_pimpl;
    if (p.osc)
        p.osc->softwareTrigger();
}

auto CHardware::adcStatus() -> ADCStatus {
    auto& p = *m_pimpl;
    ADCStatus s;
    if (!p.osc)
        return s;
    s.running = p.osc->isRunning();
    s.armed = p.osc->isArmed();
    s.triggerCount = p.osc->getTriggerCount();
    return s;
}

// ---------------------------------------------------------------------------------------------
// DAC
// ---------------------------------------------------------------------------------------------

auto CHardware::dacConfigure(const DACConfig& cfg) -> bool {
    auto& p = *m_pimpl;
    std::lock_guard lock(p.dacMtx);
    if (!p.gen || p.gen->isRunning())
        return false;
    if (!p.gen->setDacHz(cfg.rate))
        return false;
    if (cfg.calibration && !p.board->loadCalibration())
        return false;
    DACCalibration c[2];
    for (uint8_t ch = 0; ch < p.info.dacChannels && ch < 2; ch++) {
        if (p.info.dacGain)
            p.board->setDACGain(ch, cfg.gain5x[ch]);
        if (cfg.calibration)
            c[ch] = p.board->dacCalibration(ch, cfg.gain5x[ch]);
    }
    p.gen->setCalibration(c[0].offset, c[0].gain, c[1].offset, c[1].gain);
    if (cfg.bufferBytes)
        p.gen->setDataSize(cfg.bufferBytes);
    p.dacCfg = cfg;
    return true;
}

auto CHardware::dacSetTrigger(const DACTrigger& trig) -> bool {
    auto& p = *m_pimpl;
    std::lock_guard lock(p.dacMtx);
    if (!p.gen || p.gen->isRunning())
        return false;
    bool ok = p.gen->setTriggerMode(trig.enable[0], trig.enable[1]);
    ok = p.gen->setTriggerSources(trig.sources) && ok;
    ok = p.gen->setEventSource(eventId(trig.events, gen0_event_id)) && ok;
    return ok;
}

auto CHardware::dacPrepare() -> void {
    auto& p = *m_pimpl;
    std::lock_guard lock(p.dacMtx);
    if (!p.gen)
        return;
    p.gen->stop();
    p.gen->prepare();
}

auto CHardware::dacSetBuffer(uint8_t index, uint32_t ch1, uint32_t ch2, uint32_t bytes) -> bool {
    auto& p = *m_pimpl;
    return p.gen && p.gen->setDataAddress(index, ch1, ch2, bytes);
}

auto CHardware::dacStart(const std::array<bool, 2>& channels, const std::array<bool, 2>& eightBit) -> void {
    auto& p = *m_pimpl;
    std::lock_guard lock(p.dacMtx);
    if (!p.gen)
        return;
    p.gen->setDataBits(eightBit[0], eightBit[1]);
    p.gen->start(channels[0], channels[1]);
}

auto CHardware::dacStartLoop(uint32_t ch1, uint32_t ch2, uint32_t bytes, uint32_t count, const std::array<bool, 2>& eightBit) -> void {
    auto& p = *m_pimpl;
    std::lock_guard lock(p.dacMtx);
    if (!p.gen)
        return;
    p.gen->startLoop(ch1, ch2, bytes, count);
    p.gen->setDataBits(eightBit[0], eightBit[1]);
    p.gen->start(ch1 != 0, ch2 != 0);
}

auto CHardware::dacLoopDone() -> bool {
    auto& p = *m_pimpl;
    return p.gen && p.gen->isLoopDone();
}

auto CHardware::dacLoopPlayed() -> uint32_t {
    auto& p = *m_pimpl;
    return p.gen ? p.gen->getLoopPlayed() : 0;
}

auto CHardware::dacWaitPlayed(uint32_t timeoutMs) -> bool {
    auto& p = *m_pimpl;
    return p.gen && p.gen->waitPlayed(timeoutMs);
}

auto CHardware::dacStop() -> void {
    auto& p = *m_pimpl;
    std::lock_guard lock(p.dacMtx);
    if (p.gen)
        p.gen->stop();
}

auto CHardware::dacSoftwareTrigger() -> void {
    auto& p = *m_pimpl;
    if (p.gen)
        p.gen->softwareTrigger();
}

auto CHardware::dacStatus() -> DACStatus {
    auto& p = *m_pimpl;
    DACStatus s;
    if (!p.gen)
        return s;
    s.running = p.gen->isRunning();
    auto armed = p.gen->getArmed();
    s.armed = {(armed & 0x1) != 0, (armed & 0x2) != 0};
    return s;
}

// ---------------------------------------------------------------------------------------------
// GPIO
// ---------------------------------------------------------------------------------------------

auto CHardware::gpioConfigure(const GPIOConfig& cfg) -> bool {
    auto& p = *m_pimpl;
    std::lock_guard lock(p.gpioMtx);
    if (!p.gpio || p.gpio->isRunning())
        return false;
    bool ok = p.gpio->setOutputs(cfg.outputsP, cfg.outputsN);
    ok = p.gpio->setPolarity(cfg.polarity) && ok;
    ok = p.gpio->setDecimation(cfg.decimation) && ok;
    ok = p.gpio->setRLE(cfg.rle) && ok;
    if (cfg.bufferBytes)
        ok = p.gpio->setDataSize(cfg.bufferBytes) && ok;
    if (ok)
        p.gpioCfg = cfg;
    return ok;
}

auto CHardware::gpioSetTrigger(const GPIOTrigger& trig) -> bool {
    auto& p = *m_pimpl;
    std::lock_guard lock(p.gpioMtx);
    if (!p.gpio || p.gpio->isRunning())
        return false;
    if ((trig.output && !p.features.gpioTriggerMode) || ((trig.input || trig.blocks) && !p.features.gpioBlockMode))
        return false;
    bool ok = p.gpio->setTriggerSources(trig.sources);
    ok = p.gpio->setEventSource(eventId(trig.events, gpio_event_id)) && ok;
    ok = p.gpio->setPattern(trig.patternMask, trig.patternValue, trig.patternPosEdge, trig.patternNegEdge) && ok;
    if (p.features.gpioTriggerMode)
        ok = p.gpio->setOutputTriggerMode(trig.output) && ok;
    if (p.features.gpioBlockMode) {
        ok = p.gpio->setInputTriggerMode(trig.input) && ok;
        ok = p.gpio->setTriggerBlocks(trig.blocks) && ok;
    }
    if (ok)
        p.gpioTrig = trig;
    return ok;
}

auto CHardware::gpioPrepare() -> void {
    auto& p = *m_pimpl;
    std::lock_guard lock(p.gpioMtx);
    if (p.gpio)
        p.gpio->prepare(p.gpioCfg.input, p.gpioCfg.output);
}

auto CHardware::gpioSetInBuffer(uint8_t index, uint32_t address) -> void {
    auto& p = *m_pimpl;
    if (p.gpio)
        p.gpio->setInAddress(index, address);
}

auto CHardware::gpioSetOutBuffer(uint8_t index, uint32_t address) -> bool {
    auto& p = *m_pimpl;
    if (!p.gpio)
        return false;
    if (p.gpio->isRunning() && !p.gpio->isOutBufferDone(index))
        return false;
    return p.gpio->setOutAddress(index, address);
}

auto CHardware::gpioStart() -> void {
    auto& p = *m_pimpl;
    std::lock_guard lock(p.gpioMtx);
    if (p.gpio)
        p.gpio->start(p.gpioCfg.input, p.gpioCfg.output);
}

auto CHardware::gpioWaitIn(uint8_t index, int timeoutMs) -> bool {
    auto& p = *m_pimpl;
    return p.gpio && p.gpio->waitIn(index, timeoutMs);
}

auto CHardware::gpioInBuffer(uint8_t index) -> GPIOBufferInfo {
    auto& p = *m_pimpl;
    GPIOBufferInfo r;
    if (!p.gpio)
        return r;
    r.wait = p.gpio->getInWait64(index);
    uint64_t all = p.gpio->getInLost64(index);
    r.lost = all - std::min(all, r.wait);
    r.triggerCount = p.gpio->getInputTriggerCount();
    return r;
}

auto CHardware::gpioReleaseInBuffer(uint8_t index) -> void {
    auto& p = *m_pimpl;
    if (p.gpio)
        p.gpio->clearInBuffer(index);
}

auto CHardware::gpioStop() -> void {
    auto& p = *m_pimpl;
    std::lock_guard lock(p.gpioMtx);
    if (p.gpio)
        p.gpio->stop();
}

auto CHardware::gpioSoftwareTrigger() -> void {
    auto& p = *m_pimpl;
    if (p.gpio)
        p.gpio->softwareTrigger();
}

auto CHardware::gpioStatus() -> GPIOStatus {
    auto& p = *m_pimpl;
    GPIOStatus s;
    if (!p.gpio)
        return s;
    s.running = p.gpio->isRunning();
    s.inputArmed = p.features.gpioBlockMode && p.gpio->isInputArmed();
    s.outputArmed = p.features.gpioTriggerMode && p.gpio->isOutputArmed();
    s.triggerCount = p.gpio->getInputTriggerCount();
    return s;
}
