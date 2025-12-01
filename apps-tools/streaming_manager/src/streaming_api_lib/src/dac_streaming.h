#ifndef DAC_STREAMING_H
#define DAC_STREAMING_H

#include <stdint.h>
#include <string>
#include <vector>

class DACCallback;

class DACStreamClient {
   public:
    DACStreamClient();
    ~DACStreamClient();

    auto connect() -> bool;
    auto connect(std::string host) -> bool;

    auto setRepeatCount(uint64_t count) -> void;
    auto setRepeatInf(bool enable) -> void;
    auto setMemory8Bit(uint8_t channel, std::vector<int8_t> buffer) -> bool;
    auto setMemory16Bit(uint8_t channel, std::vector<int16_t> buffer) -> bool;

    auto startStreamingTDMS(std::string fileName) -> bool;
    auto startStreamingWAV(std::string fileName) -> bool;
    auto startStreamingFromMemory() -> bool;
    auto stopStreaming() -> void;

    auto wait() -> void;
    auto notifyStop() -> void;

    auto sendConfig(std::string key, std::string value) -> bool;
    auto sendConfig(std::string host, std::string key, std::string value) -> bool;
    auto getConfig(std::string key) -> std::string;
    auto getConfig(std::string host, std::string key) -> std::string;

    auto sendFileConfig(std::string config) -> bool;
    auto sendFileConfig(std::string host, std::string config) -> bool;
    auto getFileConfig() -> std::string;
    auto getFileConfig(std::string host) -> std::string;

    auto setVerbose(bool enable) -> void;

    auto setCallbackFunction(DACCallback* callback) -> void;
    auto removeCallbackFunction() -> void;

   private:
    auto startStreaming() -> bool;

    struct Impl;
    // Pointer to the internal implementation
    Impl* m_pimpl;
};

#endif
