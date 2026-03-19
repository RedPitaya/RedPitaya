#include <cmath>
#include <csignal>
#include <iostream>
#include <memory>
#include <vector>
#include "callbacks.h"
#include "config_streaming.h"
#include "dac_streaming.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

std::shared_ptr<DACStreamClient> obj = nullptr;

class DACCallbackHandler : public DACCallback {
   public:
    int counter = 0;

    void sentPack(DACStreamClient* client, uint32_t ch1_size, uint32_t ch2_size) override { counter++; }

    void connected(DACStreamClient* client, std::string host) override { std::cout << "DAC client connected to " << host << std::endl; }

    void error(DACStreamClient* client, std::string host, int code) override {
        std::cerr << "Error (" << code << ") on host: " << host << std::endl;
        client->notifyStop();
    }

    // Implement all other required callback methods...
};

class ConfigCallbackImpl : public ConfigCallback {
   public:
    void sigInt() override {
        if (obj)
            obj->notifyStop();
    }
};

std::vector<int16_t> generateSineWave(double freq, int sampleRate, double duration) {
    std::vector<int16_t> samples;
    const int numSamples = sampleRate * duration;
    const double amplitude = 32767.0;

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        samples.push_back(static_cast<int16_t>(amplitude * std::sin(2.0 * M_PI * freq * t)));
    }

    return samples;
}

int main() {
    auto confClient = std::make_shared<ConfigStreamClient>();
    obj = std::make_shared<DACStreamClient>(confClient);

    auto confCallback = std::make_shared<ConfigCallbackImpl>();
    confClient->addCallback(confCallback);

    auto callback = std::make_shared<DACCallbackHandler>();
    obj->setCallback(callback);
    obj->setVerbose(true);
    confClient->setVerbose(true);

    if (!confClient->connect()) {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }

    // Standard configuration
    confClient->sendConfig("dac_pass_mode", "DAC_NET");
    confClient->sendConfig("dac_rate", "125000000");
    confClient->sendConfig("block_size", "16384");
    confClient->sendConfig("dac_size", "1638400");

    // Generate and load to memory
    auto samples = generateSineWave(1.0, 32768, 1.0);
    obj->setMemory16Bit(1, samples);  // Channel 1
    obj->setMemory16Bit(2, samples);  // Channel 2
    auto host = confClient->getHosts().front();
    if (obj->startStreamingFromMemory(host)) {
        std::cout << "Memory streaming started" << std::endl;
    } else {
        std::cerr << "Failed to start streaming" << std::endl;
        return 1;
    }

    obj->wait();
    std::cout << "Sent packets: " << callback->counter << std::endl;

    return 0;
}