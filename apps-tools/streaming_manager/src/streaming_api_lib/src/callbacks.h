#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <stdint.h>
#include <array>
#include <string>
#include <vector>

struct ADCChannel {
    uint32_t samples = 0;
    uint8_t bitsPerSample = 0;
    uint64_t fpgaLost = 0;
    bool attenuator_1_20 = false;
    uint32_t baseRate = 0;
    uint8_t adcBaseBits = 0;
    uint64_t packId = 0;
    int64_t timeCapture = 0;
    std::vector<int16_t> raw;
};

struct ADCPack {
    std::string host;
    ADCChannel channel1;
    ADCChannel channel2;
    ADCChannel channel3;
    ADCChannel channel4;
};

// A buffer of the GPIO input: 32 bit words {N count, N lines, P count, P lines}; with RLE a word
// stands for count + 1 samples, else for one
struct GPIOPack {
    std::string host;
    uint64_t packId = 0;
    uint64_t fpgaLost = 0;  // samples lost right before this buffer (overflow)
    uint64_t fpgaWait = 0;  // samples skipped right before this buffer while waiting for a trigger
    uint32_t decimation = 0;
    uint32_t baseRate = 0;  // Hz before the decimation
    uint32_t triggerCount = 0;
    uint16_t triggerBlocks = 0;
    bool rle = false;
    bool triggerMode = false;
    std::vector<uint32_t> words;
};

// Why the board ended a GPIO file (the file mode: gpio_in_pass_mode / gpio_out_pass_mode = FILE)
enum GPIOFileStop {
    GPIO_FILE_DONE = 0,     // input: the sample limit is written; output: the file is played to its end
    GPIO_FILE_SD_FULL = 1,  // input: no room left on the SD card
    GPIO_FILE_BROKEN = 2,   // the file can not be written (input) or read (output)
    GPIO_FILE_EMPTY = 3,    // output: the file holds no words
    GPIO_FILE_MISSING = 4   // output: there is no such file (gpio_out_file_sd)
};

class ADCStreamClient;
class DACStreamClient;
class GPIOStreamClient;
class ConfigStreamClient;

class ConfigCallback {
   public:
    virtual ~ConfigCallback() {}
    virtual void configConnected(ConfigStreamClient*, std::string) {}
    virtual void configError(ConfigStreamClient*, std::string, int) {}
    virtual void configErrorTimeout(ConfigStreamClient*, std::string) {}
    virtual void configErrorFileMissed(ConfigStreamClient*, std::string) {}
    virtual void configMemoryBlockSize(ConfigStreamClient*, std::string, size_t) {}
    virtual void configActiveChannels(ConfigStreamClient*, std::string, std::array<bool, 4>) {}

    virtual void configSuccessSend(ConfigStreamClient*, std::string) {}
    virtual void configFailSend(ConfigStreamClient*, std::string) {}
    virtual void configSuccessSave(ConfigStreamClient*, std::string) {}
    virtual void configFailSave(ConfigStreamClient*, std::string) {}

    virtual void configGetNewSettings(ConfigStreamClient*, std::string) {}
    virtual void configGetNewSettingsItem(ConfigStreamClient*, std::string) {}

    virtual void adcServerStopped(ConfigStreamClient*, std::string) {}
    virtual void adcServerStoppedNoActiveChannels(ConfigStreamClient*, std::string) {}
    virtual void adcServerStoppedMemError(ConfigStreamClient*, std::string) {}
    virtual void adcServerStoppedMemModify(ConfigStreamClient*, std::string) {}
    virtual void adcServerStoppedSDFull(ConfigStreamClient*, std::string) {}
    virtual void adcServerStoppedSDDone(ConfigStreamClient*, std::string) {}
    virtual void adcServerStartedTCP(ConfigStreamClient*, std::string) {}
    virtual void adcServerStartedSD(ConfigStreamClient*, std::string) {}
    virtual void adcServerStartedFPGA(ConfigStreamClient*, std::string) {}

    virtual void dacServerStartedTCP(ConfigStreamClient*, std::string) {}
    virtual void dacServerStartedSD(ConfigStreamClient*, std::string) {}
    virtual void dacServerStoppedMemError(ConfigStreamClient*, std::string) {}
    virtual void dacServerStoppedMemModify(ConfigStreamClient*, std::string) {}
    virtual void dacServerStoppedConfigError(ConfigStreamClient*, std::string) {}
    virtual void dacServerStoppedFileMissed(ConfigStreamClient*, std::string) {}
    virtual void dacServerStoppedSDDone(ConfigStreamClient*, std::string) {}
    virtual void dacServerStoppedSDEmpty(ConfigStreamClient*, std::string) {}
    virtual void dacServerStoppedSDBroken(ConfigStreamClient*, std::string) {}
    virtual void dacServerStoppedSDMissing(ConfigStreamClient*, std::string) {}

    virtual void gpioInServerStarted(ConfigStreamClient*, std::string) {}
    virtual void gpioInServerStopped(ConfigStreamClient*, std::string) {}
    virtual void gpioInServerStoppedMemError(ConfigStreamClient*, std::string) {}
    virtual void gpioInServerStoppedConfigError(ConfigStreamClient*, std::string) {}
    virtual void gpioOutServerStarted(ConfigStreamClient*, std::string) {}
    // waitClocks: the clocks the output waited for the client's data since it began to play
    // (the network was too slow), the time after the last buffer up to the stop included
    virtual void gpioOutServerStopped(ConfigStreamClient*, std::string, [[maybe_unused]] uint64_t waitClocks) {}
    virtual void gpioOutServerStoppedMemError(ConfigStreamClient*, std::string) {}
    virtual void gpioOutServerStoppedConfigError(ConfigStreamClient*, std::string) {}

    // The GPIO file mode: the input writes a file on the board, the output plays one from there
    virtual void gpioInServerStartedSD(ConfigStreamClient*, std::string) {}
    virtual void gpioInServerStoppedSDFull(ConfigStreamClient*, std::string) {}
    // the sample limit is written (taken at a buffer: the file may run past it by up to a buffer)
    virtual void gpioInServerStoppedSDDone(ConfigStreamClient*, std::string) {}
    virtual void gpioInServerStoppedSDBroken(ConfigStreamClient*, std::string) {}
    virtual void gpioOutServerStartedSD(ConfigStreamClient*, std::string) {}
    // the file is played to its end; waitClocks as in gpioOutServerStopped
    virtual void gpioOutServerStoppedSDDone(ConfigStreamClient*, std::string, [[maybe_unused]] uint64_t waitClocks) {}
    virtual void gpioOutServerStoppedSDEmpty(ConfigStreamClient*, std::string) {}
    virtual void gpioOutServerStoppedSDBroken(ConfigStreamClient*, std::string) {}
    virtual void gpioOutServerStoppedSDMissing(ConfigStreamClient*, std::string) {}

    virtual void sigInt() {};
};

class ADCCallback {
   public:
    virtual ~ADCCallback() {}
    virtual void receivePack(ADCStreamClient*, ADCPack&) {}
    virtual void connected(ADCStreamClient*, std::string) {}
    virtual void disconnected(ADCStreamClient*, std::string) {}
    virtual void error(ADCStreamClient*, std::string, int) {}
};

class DACCallback {
   public:
    virtual ~DACCallback() {}
    virtual void sentPack(DACStreamClient*, [[maybe_unused]] uint32_t ch1_size, [[maybe_unused]] uint32_t ch2_size) {}
    virtual void connected(DACStreamClient*, std::string) {}
    virtual void disconnected(DACStreamClient*, std::string) {}
    virtual void error(DACStreamClient*, std::string, int) {}
    virtual void stopped(DACStreamClient*, std::string) {}
    virtual void stoppedFileEnd(DACStreamClient*, std::string) {}
    virtual void stoppedFileBroken(DACStreamClient*, std::string) {}
    virtual void stoppedEmpty(DACStreamClient*, std::string) {}
    virtual void stoppedMissingFile(DACStreamClient*, std::string) {}
    virtual void stoppedMemError(DACStreamClient*, std::string) {}
    virtual void stoppedMemModify(DACStreamClient*, std::string) {}

    virtual bool streamData8Bit(DACStreamClient*, [[maybe_unused]] int8_t* ch1_8Bit, [[maybe_unused]] int8_t* ch2_8Bit, [[maybe_unused]] size_t size) { return true; }
    virtual bool streamData16Bit(DACStreamClient*, [[maybe_unused]] int16_t* ch1_16Bit, [[maybe_unused]] int16_t* ch2_16Bit, [[maybe_unused]] size_t size) { return true; }
};
class GPIOCallback {
   public:
    virtual ~GPIOCallback() {}
    virtual void receivePack(GPIOStreamClient*, GPIOPack&) {}
    virtual void sentPack(GPIOStreamClient*, [[maybe_unused]] uint32_t words) {}
    virtual void connected(GPIOStreamClient*, std::string) {}
    virtual void disconnected(GPIOStreamClient*, std::string) {}
    virtual void error(GPIOStreamClient*, std::string, int) {}
    // the board stopped the output: the clocks it waited for this client's data (see ConfigCallback)
    virtual void outputStopped(GPIOStreamClient*, std::string, [[maybe_unused]] uint64_t waitClocks) {}
    // The file mode: the board ended its input file or the file its output played by itself
    // (not on a stop of the client)
    virtual void inputFileStopped(GPIOStreamClient*, std::string, [[maybe_unused]] GPIOFileStop reason) {}
    virtual void outputFileStopped(GPIOStreamClient*, std::string, [[maybe_unused]] GPIOFileStop reason, [[maybe_unused]] uint64_t waitClocks) {}
    // The output words {P lines, N lines, 8 bit unused, RLE count} when the client streams them
    // itself (startStreaming without setOutputData): at most maxWords; none ends the output
    virtual std::vector<uint32_t> outputData(GPIOStreamClient*, [[maybe_unused]] size_t maxWords) { return {}; }
};

#endif
