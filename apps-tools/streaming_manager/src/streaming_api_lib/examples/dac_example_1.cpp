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

// WAV file header structure
#pragma pack(push, 1)
struct WAVHeader {
    uint8_t riff[4] = {'R', 'I', 'F', 'F'};
    uint32_t fileSize;
    uint8_t wave[4] = {'W', 'A', 'V', 'E'};
    uint8_t fmt[4] = {'f', 'm', 't', ' '};
    uint32_t fmtSize = 16;
    uint16_t audioFormat = 1;
    uint16_t numChannels = 1;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample = 16;
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

void writeWAV(const std::string& filename, int sampleRate, const std::vector<int16_t>& samples) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open WAV file");
    }

    WAVHeader header;
    header.sampleRate = sampleRate;
    header.numChannels = 1;
    header.bitsPerSample = 16;
    header.byteRate = sampleRate * header.numChannels * header.bitsPerSample / 8;
    header.blockAlign = header.numChannels * header.bitsPerSample / 8;
    header.dataSize = samples.size() * sizeof(int16_t);
    header.fileSize = sizeof(WAVHeader) - 8 + header.dataSize;

    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    file.write(reinterpret_cast<const char*>(samples.data()), samples.size() * sizeof(int16_t));
}

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
    DACStreamClient client;
    DACCallbackHandler* callback = new DACCallbackHandler();
    client.setCallbackFunction(callback);
    client.setVerbose(true);

    if (!client.connect()) {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }

    // Configure DAC
    client.sendConfig("dac_pass_mode", "DAC_NET");
    client.sendConfig("dac_rate", "125000000");
    client.sendConfig("block_size", "16384");
    client.sendConfig("adc_size", "1638400");

    // Generate and save WAV
    auto samples = generateSineWave(1.0, 4096, 1.0);
    writeWAV("sin.wav", 4096, samples);

    // Setup streaming
    client.setRepeatInf(false);
    client.setRepeatCount(2);

    if (client.startStreamingWAV("./sin.wav")) {
        std::cout << "Streaming started" << std::endl;
    } else {
        std::cerr << "Failed to start streaming" << std::endl;
        return 1;
    }

    client.wait();
    std::cout << "Sent packets: " << callback->counter << std::endl;

    return 0;
}