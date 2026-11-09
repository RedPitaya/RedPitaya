#ifndef UIO_LIB_HARDWARE_H
#define UIO_LIB_HARDWARE_H

#include <array>
#include <cstdint>
#include <memory>
#include <string>

namespace uio_lib {

// ---------------------------------------------------------------------------------------------
// What the hardware facade needs from the board. uio_lib does not read the profile or the
// EEPROM itself: board_lib implements IBoard (librp-hw on a board, a dummy on the host).
// ---------------------------------------------------------------------------------------------

struct BoardInfo {
    uint8_t adcChannels = 2;
    uint8_t dacChannels = 2;
    uint32_t adcRate = 125000000;  // Hz, before the decimation
    uint32_t dacRate = 125000000;  // Hz
    uint8_t adcBits = 14;          // of the ADC itself; the stream carries 16 or 8 bit
    bool adcFilter = false;        // the equalization filter is built into the FPGA
    bool dacGain = false;          // the DAC has a x1/x5 gain switch (250-12)
    bool daisyChain = false;       // the board can be a daisy chain (x-streaming) slave
    uint8_t model = 0;             // the profile's model (rp_HPeModels_t), as the broadcast carries it
    std::string mac = "NONE";      // of the Ethernet port, for the broadcast
};

struct ADCCalibration {
    int32_t offset = 0;
    double gain = 1;
    // equalization filter, used when BoardInfo::adcFilter
    uint32_t aa = 0;
    uint32_t bb = 0;
    uint32_t kk = 0xFFFFFF;
    uint32_t pp = 0;
};

struct DACCalibration {
    int32_t offset = 0;
    double gain = 1;
};

// Where the FPGA applies the ADC offset and gain: the EEPROM calibration up to v5 was taken
// after the equalization filter, from v6 on it is taken before it (register 0x84 of the scope)
enum class ADCCalibrationPlace { BEFORE_FILTER, AFTER_FILTER };

class IBoard {
   public:
    virtual ~IBoard() = default;
    virtual auto info() const -> BoardInfo = 0;
    // reads the calibration again: it can change between runs
    virtual auto loadCalibration() -> bool = 0;
    virtual auto adcCalibrationPlace() -> ADCCalibrationPlace = 0;
    virtual auto adcCalibration(uint8_t ch, bool att1_20, bool dc) -> ADCCalibration = 0;
    virtual auto dacCalibration(uint8_t ch, bool gain5x) -> DACCalibration = 0;
    // front end relays, where the board has them
    virtual auto setADCFrontEnd(uint8_t ch, bool att1_20, bool dc) -> void = 0;
    virtual auto setDACGain(uint8_t ch, bool gain5x) -> void = 0;
};

// ---------------------------------------------------------------------------------------------
// Configuration of the blocks
// ---------------------------------------------------------------------------------------------

// The block whose start/stop/reset/trigger events a block follows; OWN - its own
enum class EventSource : int8_t { OWN = -1, GEN_A = 0, GEN_B = 1, OSC1 = 2, OSC2 = 3, LA = 4 };

struct ADCConfig {
    uint32_t decimation = 1;
    std::array<bool, 4> channels = {true, false, false, false};
    std::array<bool, 4> att1_20 = {false, false, false, false};
    std::array<bool, 4> dc = {true, true, true, true};
    bool eightBit = false;
    bool calibration = true;  // apply the board calibration, else raw codes
    uint32_t bufferBytes = 0;  // DMA buffer of one channel
};

// The sources, the level and the blocks apply in the trigger mode only; the event source always
struct ADCTrigger {
    bool enable = false;      // wait for a trigger before writing
    uint32_t sources = 0;     // TRIG_SRC_* mask
    int32_t low = 0;          // level trigger, ADC codes
    int32_t high = 0;
    bool falling = false;
    EventSource events = EventSource::OWN;
    uint16_t blocks = 0;      // buffers per trigger, 0 - continuous after the trigger
};

struct DACConfig {
    uint32_t rate = 125000000;  // Hz
    std::array<bool, 2> gain5x = {false, false};
    bool calibration = true;
    uint32_t bufferBytes = 0;
};

struct DACTrigger {
    std::array<bool, 2> enable = {false, false};  // per channel: play on a trigger
    uint32_t sources = 0;                         // with a channel in the trigger mode
    EventSource events = EventSource::OWN;
};

// GPIO streaming of the DIO lines: bits 7:0 the P lines, 15:8 the N lines. The input takes the
// input fields and the shared ones, the output the output fields and the shared ones.
struct GPIOConfig {
    uint8_t outputsP = 0;      // output: lines driven by the output stream
    uint8_t outputsN = 0;
    uint16_t polarity = 0;     // input: inverted lines
    uint32_t decimation = 1;   // input
    bool rle = false;          // shared: run length coded words
    uint32_t outStep = 1;      // output: the read pointer step of its DMA (register 0xC0)
    uint32_t bufferBytes = 0;  // shared: the DMA buffer
};

// The trigger of the input (input, blocks) or of the output (output) with the shared part
struct GPIOTrigger {
    bool input = false;        // the input waits for a trigger before writing
    bool output = false;       // the output plays from a trigger
    uint32_t sources = 0;      // shared: TRIG_SRC_* mask
    EventSource events = EventSource::OWN;  // shared
    uint16_t blocks = 0;       // input buffers per trigger, 0 - continuous after the trigger
    // shared: pattern trigger (the LA trigger source)
    uint16_t patternMask = 0;
    uint16_t patternValue = 0;
    uint16_t patternPosEdge = 0;
    uint16_t patternNegEdge = 0;
};

struct ExtTrigger {
    bool conditioner = false;
    bool falling = false;
    uint32_t debounceClocks = 0;  // 0 - off
    bool dio0 = true;             // DIO0_P feeds the external trigger
};

// What the bitstream has (FEATURES registers)
struct HardwareFeatures {
    bool adc = false;
    bool adcBlockMode = false;
    bool adcWaitCounter = false;
    bool dac = false;
    bool dacLoop = false;  // the DMA repeats a buffer itself
    bool gpio = false;
    bool gpioTriggerMode = false;  // output trigger mode
    bool gpioBlockMode = false;    // input trigger and block mode
    bool gpioWaitCounter = false;
    bool gpioOutRefill = false;    // the output tells its free buffers: it streams more than two
    bool gpioOutWait = false;      // the output counts the clocks it waited for a buffer
    bool extTrigger = false;
    bool slave = false;    // daisy chain slave board
};

// A full ADC buffer: what the FPGA latched for it
struct ADCBufferInfo {
    int64_t time = 0;                   // ns since the epoch, of its first sample
    std::array<uint64_t, 4> lost = {};  // overflow before it, per channel (the wait taken out)
    uint64_t wait = 0;                  // trigger wait before it
    uint32_t triggerCount = 0;
};

struct ADCStatus {
    bool running = false;
    bool armed = false;
    uint32_t triggerCount = 0;
};

// A full GPIO input buffer
struct GPIOBufferInfo {
    uint64_t lost = 0;  // overflow before it, in samples (an RLE word of count c is c + 1)
    uint64_t wait = 0;  // trigger wait before it
    uint32_t triggerCount = 0;
};

struct DACStatus {
    bool running = false;
    std::array<bool, 2> armed = {false, false};
};

struct GPIOStatus {
    bool running = false;  // either direction
    bool inputRunning = false;
    bool outputRunning = false;
    uint64_t outputWait = 0;  // clocks the output waited for a buffer since its start
    bool inputArmed = false;
    bool outputArmed = false;
    uint32_t triggerCount = 0;
};

// ---------------------------------------------------------------------------------------------
// The stream_app FPGA: one object per process, the only owner of the scope, the generator, the
// GPIO block and the external trigger. Everything between the blocks (event sources, trigger
// sources, the state after the bitstream load) is done here.
// Each block is used from its own thread; the calls of one block are serialized.
// ---------------------------------------------------------------------------------------------

class CHardware {
   public:
    using Ptr = std::shared_ptr<CHardware>;

    // Opens the uio devices of the loaded bitstream; every block stopped, on its own events
    static auto create(std::shared_ptr<IBoard> board) -> Ptr;
    ~CHardware();

    auto info() const -> BoardInfo;
    auto features() const -> HardwareFeatures;

    // External trigger conditioner (shared by all blocks)
    auto setExtTrigger(const ExtTrigger& cfg) -> bool;

    // ADC. Set up while stopped, refused while running. adcStart() prepares and starts.
    auto adcConfigure(const ADCConfig& cfg) -> bool;
    auto adcSetTrigger(const ADCTrigger& trig) -> bool;
    auto adcConfig() const -> ADCConfig;
    auto adcTrigger() const -> ADCTrigger;
    auto adcSetSkipBuffer(uint32_t address) -> void;  // where the data goes while no buffer is free
    auto adcSetBuffer(uint8_t index, const std::array<uint32_t, 4>& addresses) -> void;
    auto adcStart() -> void;
    auto adcWait() -> bool;                            // an interrupt; false after 1 s without
    auto adcBufferFull(uint8_t index) -> bool;         // written, not released yet
    auto adcBuffer(uint8_t index) -> ADCBufferInfo;    // read before adcReleaseBuffer
    auto adcReleaseBuffer(uint8_t index) -> void;
    auto adcStop() -> void;
    auto adcSoftwareTrigger() -> void;
    auto adcStatus() -> ADCStatus;

    // DAC: dacPrepare(), the first buffer, dacStart(), then the next buffers as they free up;
    // or dacPrepare() and dacStartLoop(). The sample width comes with the data.
    auto dacConfigure(const DACConfig& cfg) -> bool;
    auto dacSetTrigger(const DACTrigger& trig) -> bool;
    auto dacPrepare() -> void;
    auto dacSetBuffer(uint8_t index, uint32_t ch1, uint32_t ch2, uint32_t bytes) -> bool;  // false: still playing
    auto dacStart(const std::array<bool, 2>& channels, const std::array<bool, 2>& eightBit) -> void;
    // the DMA repeats buffer 1 count times, 0 - until dacStop(); started at once
    auto dacStartLoop(uint32_t ch1, uint32_t ch2, uint32_t bytes, uint32_t count, const std::array<bool, 2>& eightBit) -> void;
    auto dacLoopDone() -> bool;
    auto dacLoopPlayed() -> uint32_t;
    auto dacWaitPlayed(uint32_t timeoutMs) -> bool;
    auto dacStop() -> void;
    auto dacSoftwareTrigger() -> void;
    auto dacStatus() -> DACStatus;

    // GPIO: the input and the output run on their own, each prepare, the buffers, start. What
    // both use (RLE, the buffer size, the trigger sources, the event source, the pattern) is set
    // while neither runs; while one runs the other takes it only as it is.
    auto gpioConfigureInput(const GPIOConfig& cfg) -> bool;
    auto gpioConfigureOutput(const GPIOConfig& cfg) -> bool;
    auto gpioSetInputTrigger(const GPIOTrigger& trig) -> bool;
    auto gpioSetOutputTrigger(const GPIOTrigger& trig) -> bool;
    auto gpioPrepareInput() -> void;
    auto gpioPrepareOutput() -> void;
    auto gpioSetInBuffer(uint8_t index, uint32_t address) -> void;
    auto gpioSetOutBuffer(uint8_t index, uint32_t address) -> bool;  // false: still playing
    auto gpioStartInput() -> void;
    auto gpioStartOutput() -> void;
    auto gpioWaitIn(uint8_t index, int timeoutMs) -> bool;  // input buffer `index` is full
    auto gpioInBuffer(uint8_t index) -> GPIOBufferInfo;
    auto gpioReleaseInBuffer(uint8_t index) -> void;
    auto gpioStopInput() -> void;
    auto gpioStopOutput() -> void;
    auto gpioSoftwareTrigger() -> void;
    auto gpioStatus() -> GPIOStatus;

   private:
    explicit CHardware(std::shared_ptr<IBoard> board);
    CHardware(const CHardware&) = delete;
    CHardware& operator=(const CHardware&) = delete;

    struct Impl;
    Impl* m_pimpl;
};

}  // namespace uio_lib

#endif
