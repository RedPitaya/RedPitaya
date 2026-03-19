#include <cmath>
#include <csignal>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include "callbacks.h"
#include "config_streaming.h"
#include "dac_streaming.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#pragma pack(push, 1)
struct WAVHeader {
    uint8_t riff[4] = {'R', 'I', 'F', 'F'};
    uint32_t fileSize;
    uint8_t wave[4] = {'W', 'A', 'V', 'E'};
    uint8_t fmt[4] = {'f', 'm', 't', ' '};
    uint32_t fmtSize = 16;
    uint16_t audioFormat = 1;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    uint8_t data[4] = {'d', 'a', 't', 'a'};
    uint32_t dataSize;
};
#pragma pack(pop)

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

void writeStereoWAV(const std::string& filename, int sampleRate, const std::vector<int8_t>& left, const std::vector<int8_t>& right) {
    if (left.size() != right.size()) {
        throw std::runtime_error("Channel size mismatch");
    }

    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open WAV file");
    }

    WAVHeader header;
    header.sampleRate = sampleRate;
    header.numChannels = 2;
    header.bitsPerSample = 8;
    header.byteRate = sampleRate * header.numChannels * header.bitsPerSample / 8;
    header.blockAlign = header.numChannels * header.bitsPerSample / 8;
    header.dataSize = left.size() * header.numChannels;
    header.fileSize = sizeof(WAVHeader) - 8 + header.dataSize;

    file.write(reinterpret_cast<const char*>(&header), sizeof(header));

    // Interleave samples (LRLRLR...)
    for (size_t i = 0; i < left.size(); ++i) {
        file.write(reinterpret_cast<const char*>(&left[i]), 1);
        file.write(reinterpret_cast<const char*>(&right[i]), 1);
    }
}

std::vector<int8_t> generate8bitSine(double freq, int sampleRate, double duration) {
    std::vector<int8_t> samples;
    const int numSamples = sampleRate * duration;
    const double amplitude = 127.0;

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        samples.push_back(static_cast<int8_t>(amplitude * std::sin(2.0 * M_PI * freq * t)));
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

    // Configure with larger buffers
    confClient->sendConfig("dac_pass_mode", "DAC_NET");
    confClient->sendConfig("dac_rate", "125000000");
    confClient->sendConfig("block_size", "262144");
    confClient->sendConfig("dac_size", "5242880");

    // Generate stereo WAV
    auto left = generate8bitSine(1.0, 262144, 1.0);
    auto right = generate8bitSine(1.0, 262144, 1.0);
    writeStereoWAV("sin.wav", 262144, left, right);

    // Infinite repeat
    obj->setRepeatInf(true);
    auto host = confClient->getHosts().front();
    if (obj->startStreamingWAV(host, "./sin.wav")) {
        std::cout << "Stereo streaming started" << std::endl;
    } else {
        std::cerr << "Failed to start streaming" << std::endl;
        return 1;
    }

    obj->wait();
    std::cout << "Sent packets: " << callback->counter << std::endl;

    return 0;
}
