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

    bool streamData8Bit(DACStreamClient*, int8_t* ch1, int8_t* ch2, size_t size) override {
        for (size_t idx = 0; idx < size; idx++) {
            float sineValue = std::sin(2.0f * M_PI * idx / size);

            int8_t sample = static_cast<int8_t>((sineValue * 128));

            if (ch1)
                ch1[idx] = sample;
            if (ch2)
                ch2[idx] = sample;
        }
        std::cout << "streamData " << (ch1 ? " enCh1 " : " disCh1 ") << (ch2 ? " enCh2 " : " disCh2 ") << " size " << size << std::endl;
        return false;
    }

    void sentPack(DACStreamClient* client, uint32_t ch1_size, uint32_t ch2_size) override {
        std::cout << "sentPack " << counter << std::endl;
        counter++;
    }

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
    auto host = confClient->getHosts().front();
    if (obj->startStreamingFromMemorySink(host, true, true, DACStreamBytes::DAC_8BIT)) {
        std::cout << "Memory streaming started" << std::endl;
    } else {
        std::cerr << "Failed to start streaming" << std::endl;
        return 1;
    }

    obj->wait();
    std::cout << "Sent packets: " << callback->counter << std::endl;

    return 0;
}