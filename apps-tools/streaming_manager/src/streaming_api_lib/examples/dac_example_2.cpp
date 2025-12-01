#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include "callbacks.h"
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

class DACCallbackHandler : public DACCallback {
   public:
    int counter = 0;

    void sendedPack(DACStreamClient* client, uint32_t ch1_size, uint32_t ch2_size) override { counter++; }

    void connected(DACStreamClient* client, std::string host) override { std::cout << "DAC client connected to " << host << std::endl; }

    void error(DACStreamClient* client, std::string host, int code) override {
        std::cerr << "Error (" << code << ") on host: " << host << std::endl;
        client->notifyStop();
    }

    // Implement all other required callback methods...
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
    DACStreamClient client;
    DACCallbackHandler* callback = new DACCallbackHandler();
    client.setCallbackFunction(callback);
    client.setVerbose(true);

    if (!client.connect()) {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }

    // Configure with larger buffers
    client.sendConfig("dac_pass_mode", "DAC_NET");
    client.sendConfig("dac_rate", "125000000");
    client.sendConfig("block_size", "262144");
    client.sendConfig("adc_size", "2621440");

    // Generate stereo WAV
    auto left = generate8bitSine(1.0, 262144, 1.0);
    auto right = generate8bitSine(1.0, 262144, 1.0);
    writeStereoWAV("sin.wav", 262144, left, right);

    // Infinite repeat
    client.setRepeatInf(true);

    if (client.startStreamingWAV("./sin.wav")) {
        std::cout << "Stereo streaming started" << std::endl;
    } else {
        std::cerr << "Failed to start streaming" << std::endl;
        return 1;
    }

    client.wait();
    std::cout << "Sent packets: " << callback->counter << std::endl;

    return 0;
}
