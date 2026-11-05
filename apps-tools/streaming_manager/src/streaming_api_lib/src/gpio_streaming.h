#ifndef GPIO_STREAMING_H
#define GPIO_STREAMING_H

#include <stdint.h>
#include <memory>
#include <string>
#include <vector>

class GPIOCallback;
class ConfigStreamClient;

// The GPIO streaming of one board (DIO0..7 P and N): the input lines come in as GPIOPack, the
// output lines go out from setOutputData() or from GPIOCallback::outputData(). The other GPIO
// settings (outputs, decimation, RLE, trigger) are the board's, set with ConfigStreamClient.
class GPIOStreamClient {
   public:
    GPIOStreamClient(std::shared_ptr<ConfigStreamClient> configClient);
    ~GPIOStreamClient();

    // Output words {P lines, N lines, 8 bit unused, RLE count}, played `repeat` times (0 - until
    // the stop); without them the output asks GPIOCallback::outputData()
    auto setOutputData(std::vector<uint32_t> words, uint64_t repeat = 1) -> void;
    auto clearOutputData() -> void;

    // Sets gpio_in_streaming / gpio_out_streaming on the board and starts its GPIO streaming
    auto startStreaming(std::string host, bool input, bool output) -> bool;
    // Stops the board's GPIO streaming and the client
    auto stopStreaming() -> void;

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
