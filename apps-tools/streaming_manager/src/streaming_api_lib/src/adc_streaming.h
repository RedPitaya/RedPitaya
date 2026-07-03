#ifndef ADC_STREAMING_H
#define ADC_STREAMING_H

#include <memory>
#include <string>
#include <vector>

class ADCCallback;
class ConfigStreamClient;

class ADCStreamClient {
   public:
    ADCStreamClient(std::shared_ptr<ConfigStreamClient> configClient);
    ~ADCStreamClient();

    auto startStreaming() -> bool;
    auto stopStreaming() -> void;
    auto wait() -> void;
    auto notifyStop() -> void;
    auto notifyStop(std::string host) -> void;

    auto setVerbose(bool enable) -> void;

    auto setCallback(std::shared_ptr<ADCCallback> callback) -> void;
    auto removeCallback() -> void;

   private:
    struct Impl;
    // Pointer to the internal implementation
    Impl* m_pimpl;
    friend class ConfigStreamClient;
};

#endif
