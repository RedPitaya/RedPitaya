#include <algorithm>
#include <cmath>
#include <csignal>
#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include "adc_streaming.h"
#include "callbacks.h"
#include "config_streaming.h"
#include "dac_streaming.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Signal detection threshold
const int16_t SIGNAL_THRESHOLD = 1000;  // Adjust for your needs

std::shared_ptr<ADCStreamClient> adcClient = nullptr;
std::shared_ptr<DACStreamClient> dacClient = nullptr;

// Shared data between callbacks

std::queue<std::vector<int16_t>> g_list;
// Global shared data instance

// ==================== ADC Callback Class ====================
class ADCCallbackHandler : public ADCCallback {
   public:
    uint64_t adcCounter = 0;
    uint64_t fpgaLost = 0;

    void receivePack(ADCStreamClient* client, ADCPack& pack) override {
        // Update counters
        adcCounter += pack.channel1.samples + pack.channel2.samples + pack.channel3.samples + pack.channel4.samples;

        fpgaLost += std::max({pack.channel1.fpgaLost, pack.channel2.fpgaLost, pack.channel3.fpgaLost, pack.channel4.fpgaLost});

        // Analyze received data for signal presence (using channel1 as example)
        analyzeSignal(pack.channel1);
    }

    void connected(ADCStreamClient* client, std::string host) override { std::cout << "ADC connected to " << host << std::endl; }

    void disconnected(ADCStreamClient* client, std::string host) override { std::cout << "ADC disconnected from " << host << std::endl; }

    void error(ADCStreamClient* client, std::string host, int code) override { std::cout << "ADC error " << host << " code " << code << std::endl; }

    void analyzeSignal(ADCChannel& channel) {
        if (channel.samples == 0 || channel.raw.empty()) {
            return;
        }

        bool found = false;

        // Analyze raw data for signal presence
        for (const auto& sample : channel.raw) {
            if (std::abs(sample) > SIGNAL_THRESHOLD) {
                found = true;
                break;
            }
        }

        // If signal detected, save data to shared buffer
        if (found) {
            g_list.push(channel.raw);
            std::cout << "Signal detected! Saved " << channel.raw.size() << " samples to DAC buffer" << std::endl;
        }
    }
};

// ==================== DAC Callback Class ====================
class DACCallbackHandler : public DACCallback {
   public:
    int dacCounter = 0;

    bool streamData8Bit(DACStreamClient* client, int8_t* ch1_8Bit, int8_t* ch2_8Bit, size_t size) override {
        // If no signal detected or buffer empty, send silence (zeros)
        if (g_list.empty()) {
            for (size_t idx = 0; idx < size; idx++) {
                if (ch1_8Bit)
                    ch1_8Bit[idx] = 0;
                if (ch2_8Bit)
                    ch2_8Bit[idx] = 0;
            }
            return false;  // Don't stop
        }
        auto data = g_list.front();
        g_list.pop();
        // Transmit data from buffer (convert 16-bit to 8-bit)
        size_t samplesToSend = std::min(size, data.size());
        for (size_t idx = 0; idx < samplesToSend; idx++) {
            // Convert 16-bit to 8-bit by taking the most significant byte
            int8_t sample = static_cast<int8_t>(data[idx] >> 8);

            if (ch1_8Bit)
                ch1_8Bit[idx] = sample;
            if (ch2_8Bit)
                ch2_8Bit[idx] = sample;
        }

        // Fill remaining with zeros if needed
        for (size_t idx = samplesToSend; idx < size; idx++) {
            if (ch1_8Bit)
                ch1_8Bit[idx] = 0;
            if (ch2_8Bit)
                ch2_8Bit[idx] = 0;
        }

        return false;  // Don't stop
    }

    bool streamData16Bit(DACStreamClient* client, int16_t* ch1_16Bit, int16_t* ch2_16Bit, size_t size) override {
        // If no signal detected or buffer empty, send silence (zeros)
        if (g_list.empty()) {
            for (size_t idx = 0; idx < size; idx++) {
                if (ch1_16Bit)
                    ch1_16Bit[idx] = 0;
                if (ch2_16Bit)
                    ch2_16Bit[idx] = 0;
            }
            return false;  // Don't stop
        }
        auto data = g_list.front();
        g_list.pop();
        // Transmit data from buffer (convert 16-bit to 8-bit)
        size_t samplesToSend = std::min(size, data.size());
        for (size_t idx = 0; idx < samplesToSend; idx++) {
            if (ch1_16Bit)
                ch1_16Bit[idx] = data[idx];
            if (ch2_16Bit)
                ch2_16Bit[idx] = data[idx];
        }

        // Fill remaining with zeros if needed
        for (size_t idx = samplesToSend; idx < size; idx++) {
            if (ch1_16Bit)
                ch1_16Bit[idx] = 0;
            if (ch2_16Bit)
                ch2_16Bit[idx] = 0;
        }

        return false;  // Don't stop
    }

    void connected(DACStreamClient* client, std::string host) override { std::cout << "DAC connected to " << host << std::endl; }

    void disconnected(DACStreamClient* client, std::string host) override { std::cout << "DAC disconnected from " << host << std::endl; }

    void error(DACStreamClient* client, std::string host, int code) override { std::cerr << "DAC error (" << code << ") on host: " << host << std::endl; }

    void stopped(DACStreamClient* client, std::string host) override { std::cout << "DAC stopped on host: " << host << std::endl; }

    void stoppedFileEnd(DACStreamClient* client, std::string host) override { std::cout << "DAC stopped - file end on host: " << host << std::endl; }

    void stoppedFileBroken(DACStreamClient* client, std::string host) override { std::cout << "DAC stopped - file broken on host: " << host << std::endl; }

    void stoppedEmpty(DACStreamClient* client, std::string host) override { std::cout << "DAC stopped - empty on host: " << host << std::endl; }

    void stoppedMissingFile(DACStreamClient* client, std::string host) override { std::cout << "DAC stopped - missing file on host: " << host << std::endl; }

    void stoppedMemError(DACStreamClient* client, std::string host) override { std::cout << "DAC stopped - memory error on host: " << host << std::endl; }

    void stoppedMemModify(DACStreamClient* client, std::string host) override { std::cout << "DAC stopped - memory modified on host: " << host << std::endl; }
};

class ConfigCallbackHandler : public ConfigCallback {
   public:
    void sigInt() override {
        if (adcClient)
            adcClient->notifyStop();
        if (dacClient)
            dacClient->notifyStop();
    }
};

// ==================== Main Function ====================
int main() {
    // Create ADC and DAC clients
    auto confClient = std::make_shared<ConfigStreamClient>();
    adcClient = std::make_shared<ADCStreamClient>(confClient);
    dacClient = std::make_shared<DACStreamClient>(confClient);

    // Create separate callback handlers (stack allocated)
    auto adcCallback = std::make_shared<ADCCallbackHandler>();
    auto dacCallback = std::make_shared<DACCallbackHandler>();
    auto confCallback = std::make_shared<ConfigCallbackHandler>();

    // Set callbacks for clients
    adcClient->setCallback(adcCallback);
    dacClient->setCallback(dacCallback);
    confClient->addCallback(confCallback);

    // Disable logs for ADC (optional)
    adcClient->setVerbose(true);
    dacClient->setVerbose(true);
    confClient->setVerbose(true);

    // ==================== ADC Connection ====================
    std::cout << "Connecting ADC..." << std::endl;
    if (!confClient->connect()) {
        std::cerr << "ADC client did not connect" << std::endl;
        return 1;
    }

    // ADC configuration
    confClient->sendConfig("adc_pass_mode", "NET");
    confClient->sendConfig("adc_decimation", "64");
    confClient->sendConfig("block_size", "65536");
    confClient->sendConfig("adc_size", "1638400");
    confClient->sendConfig("channel_state_1", "ON");
    confClient->sendConfig("channel_state_2", "ON");

    // DAC configuration
    confClient->sendConfig("dac_pass_mode", "DAC_NET");
    confClient->sendConfig("dac_rate", "1953125");
    confClient->sendConfig("dac_size", "1638400");

    // ==================== Start Streams ====================
    // Start ADC
    if (adcClient->startStreaming()) {
        std::cout << "ADC streaming started" << std::endl;
    } else {
        std::cerr << "Error starting ADC streaming" << std::endl;
        return 1;
    }

    // Start DAC with 16-bit mode (better quality)
    auto host = confClient->getHosts().front();
    if (dacClient->startStreamingFromMemorySink(host, true, true, DACStreamBytes::DAC_16BIT)) {
        std::cout << "DAC streaming started (16-bit mode)" << std::endl;
    } else {
        std::cerr << "Failed to start DAC streaming" << std::endl;
        adcClient->notifyStop();
        return 1;
    }

    std::cout << "Both ADC and DAC are running. Waiting for data..." << std::endl;
    std::cout << "Signal threshold: " << SIGNAL_THRESHOLD << std::endl;

    // ==================== Wait for Completion ====================
    // Wait for ADC to finish (it stops after receiving 50M samples)
    adcClient->wait();

    // Stop DAC
    dacClient->notifyStop();
    dacClient->wait();

    // ==================== Print Statistics ====================
    std::cout << "\n=== Statistics ===" << std::endl;
    std::cout << "ADC received samples: " << adcCallback->adcCounter << std::endl;
    std::cout << "ADC lost samples: " << adcCallback->fpgaLost << std::endl;
    std::cout << "DAC sent packets: " << dacCallback->dacCounter << std::endl;

    return 0;
}