#ifndef DAC_STREAMING_H
#define DAC_STREAMING_H

#include <stdint.h>
#include <memory>
#include <string>
#include <vector>

class DACCallback;
class ConfigStreamClient;

enum DACStreamBytes { DAC_8BIT = 1, DAC_16BIT = 2 };

class DACStreamClient {
   public:
    DACStreamClient(std::shared_ptr<ConfigStreamClient> configClient);
    ~DACStreamClient();

    auto setRepeatCount(uint64_t count) -> void;
    auto setRepeatInf(bool enable) -> void;
    auto setMemory8Bit(uint8_t channel, std::vector<int8_t> buffer) -> bool;
    auto setMemory16Bit(uint8_t channel, std::vector<int16_t> buffer) -> bool;

    auto startStreamingTDMS(std::string host, std::string fileName) -> bool;
    auto startStreamingWAV(std::string host, std::string fileName) -> bool;
    auto startStreamingFromMemory(std::string host) -> bool;
    auto startStreamingFromMemorySink(std::string host, bool enableCh1, bool enableCh2, DACStreamBytes bytePerSample) -> bool;
    auto stopStreaming() -> void;

    auto wait() -> void;
    auto notifyStop() -> void;

    auto setVerbose(bool enable) -> void;

    auto setCallback(std::shared_ptr<DACCallback> callback) -> void;
    auto removeCallback() -> void;

   private:
    auto startStreaming() -> bool;

    struct Impl;
    // Pointer to the internal implementation
    Impl* m_pimpl;
    friend class ConfigStreamClient;
};

#endif
