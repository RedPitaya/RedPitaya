#ifndef GPIO_STREAMING_H
#define GPIO_STREAMING_H

#include <stdint.h>
#include <memory>
#include <string>
#include <vector>

class GPIOCallback;
class ConfigStreamClient;

// The GPIO streaming of one board (DIO0..7 P and N). The input and the output start and stop on
// their own: the input lines come in as GPIOPack, the output lines go out from setOutputData() or
// from GPIOCallback::outputData(). The other GPIO settings (outputs, decimation, RLE, trigger,
// gpio_in_size, gpio_out_size) are the board's, set with ConfigStreamClient.
//
// The output plays one word a clock, count + 1 clocks with RLE: without RLE that is 500 MB/s at
// 125 MHz, far more than the network gives. When the data comes late the lines keep the last word
// until it comes; the board counts these clocks and reports them with the stop
// (GPIOCallback::outputStopped, outputWaitClocks()). The board fills its ring before it plays.
//
// The file mode (gpio_in_pass_mode / gpio_out_pass_mode = FILE, set with ConfigStreamClient): the
// board writes the input to a file of its own (gpio_in_format_sd, gpio_in_samples_limit_sd) or
// plays the output from one (gpio_out_file_sd); startInput()/startOutput() then open no data
// connection, wait() lasts until the board ends the file or the stop, and the end comes with
// GPIOCallback::inputFileStopped / outputFileStopped.
class GPIOStreamClient {
   public:
    GPIOStreamClient(std::shared_ptr<ConfigStreamClient> configClient);
    ~GPIOStreamClient();

    // Output words {P lines, N lines, 8 bit unused, RLE count}, played `repeat` times (0 - until
    // the stop); without them the output asks GPIOCallback::outputData()
    auto setOutputData(std::vector<uint32_t> words, uint64_t repeat = 1) -> void;
    auto clearOutputData() -> void;

    auto startInput(std::string host) -> bool;
    auto startOutput(std::string host) -> bool;
    // stop the board's input or output and the client's side of it; each waits up to 3 s for the
    // board's answer
    auto stopInput() -> void;
    auto stopOutput() -> void;
    auto stopStreaming() -> void;  // both

    // the board started its file mode at the last start
    auto inputToFile() -> bool;
    auto outputFromFile() -> bool;

    // the clocks the board's output waited for this client's data, from its last stop
    auto outputWaitClocks() -> uint64_t;

    auto wait() -> void;
    auto notifyStop() -> void;

    auto setVerbose(bool enable) -> void;

    auto setCallback(std::shared_ptr<GPIOCallback> callback) -> void;
    auto removeCallback() -> void;

   private:
    struct Impl;
    // Pointer to the internal implementation
    Impl* m_pimpl;
};

#endif
