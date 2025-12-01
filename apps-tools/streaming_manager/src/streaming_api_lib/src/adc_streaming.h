#ifndef ADC_STREAMING_H
#define ADC_STREAMING_H

#include <string>
#include <vector>

class ADCCallback;

class ADCStreamClient {
   public:
    ADCStreamClient();
    ~ADCStreamClient();

    auto connect() -> bool;
    auto connect(std::vector<std::string> hosts) -> bool;

    auto startStreaming() -> bool;
    auto stopStreaming() -> void;
    auto wait() -> void;
    auto notifyStop() -> void;
    auto notifyStop(std::string host) -> void;

    auto sendConfig(std::string key, std::string value) -> bool;
    auto sendConfig(std::string host, std::string key, std::string value) -> bool;
    auto getConfig(std::string key) -> std::string;
    auto getConfig(std::string host, std::string key) -> std::string;

    auto sendFileConfig(std::string config) -> bool;
    auto sendFileConfig(std::string host, std::string config) -> bool;
    auto getFileConfig() -> std::string;
    auto getFileConfig(std::string host) -> std::string;

    auto setVerbose(bool enable) -> void;

    auto setReciveDataFunction(ADCCallback* callback) -> void;
    auto removeReciveDataFunction() -> void;

   private:
    struct Impl;
    // Pointer to the internal implementation
    Impl* m_pimpl;
};

#endif
