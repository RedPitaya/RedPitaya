#include <algorithm>
#include <csignal>
#include <iostream>
#include <memory>
#include <string>
#include "adc_streaming.h"
#include "callbacks.h"
#include "config_streaming.h"

std::shared_ptr<ADCStreamClient> obj = nullptr;

class Callback : public ADCCallback {
   public:
    uint64_t counter = 0;
    uint64_t fpgaLost = 0;

    void receivePack(ADCStreamClient* client, ADCPack& pack) override {
        counter += pack.channel1.samples + pack.channel2.samples;
        fpgaLost += std::max({pack.channel1.fpgaLost, pack.channel2.fpgaLost, pack.channel3.fpgaLost, pack.channel4.fpgaLost});

        if (counter > 50000000) {
            client->notifyStop();
        }
    }

    void connected(ADCStreamClient* client, std::string host) override { std::cout << "Client connected " << host << std::endl; }

    void disconnected(ADCStreamClient* client, std::string host) override { std::cout << "Client disconnected " << host << std::endl; }

    void error(ADCStreamClient* client, std::string host, int code) override { std::cout << "Client error " << host << " code " << code << std::endl; }
};

class ConfigCallbackImpl : public ConfigCallback {
   public:
    void configConnected(ConfigStreamClient* client, std::string host) override { std::cout << "Config client connected to " << host << std::endl; }

    void configError(ConfigStreamClient* client, std::string host, int code) override { std::cout << "Config client error on " << host << " code " << code << std::endl; }

    void configErrorTimeout(ConfigStreamClient* client, std::string host) override { std::cout << "Config client timeout on " << host << std::endl; }

    void configErrorFileMissed(ConfigStreamClient* client, std::string host) override { std::cout << "Config client error on " << host << ": File missed" << std::endl; }

    void configMemoryBlockSize(ConfigStreamClient* client, std::string host, size_t blockSize) override {
        std::cout << "Memory block size configured on " << host << ": " << blockSize << " bytes" << std::endl;
    }

    void configActiveChannels(ConfigStreamClient* client, std::string host, size_t channels) override {
        std::cout << "Active channels configured on " << host << ": " << channels << std::endl;
    }

    void configSuccessSend(ConfigStreamClient* client, std::string host) override { std::cout << "Configuration sent successfully to " << host << std::endl; }

    void configFailSend(ConfigStreamClient* client, std::string host) override { std::cout << "Failed to send configuration to " << host << std::endl; }

    void configSuccessSave(ConfigStreamClient* client, std::string host) override { std::cout << "Configuration saved successfully on " << host << std::endl; }

    void configFailSave(ConfigStreamClient* client, std::string host) override { std::cout << "Failed to save configuration on " << host << std::endl; }

    void configGetNewSettings(ConfigStreamClient* client, std::string host) override { std::cout << "Getting new settings from " << host << std::endl; }

    void adcServerStopped(ConfigStreamClient* client, std::string host) override { std::cout << "ADC server stopped on " << host << std::endl; }

    void adcServerStoppedNoActiveChannels(ConfigStreamClient* client, std::string host) override {
        std::cout << "ADC server stopped on " << host << ": No active channels" << std::endl;
    }

    void adcServerStoppedMemError(ConfigStreamClient* client, std::string host) override { std::cout << "ADC server stopped on " << host << ": Memory error" << std::endl; }

    void adcServerStoppedMemModify(ConfigStreamClient* client, std::string host) override { std::cout << "ADC server stopped on " << host << ": Memory modified" << std::endl; }

    void adcServerStoppedSDFull(ConfigStreamClient* client, std::string host) override { std::cout << "ADC server stopped on " << host << ": SD card is full" << std::endl; }

    void adcServerStoppedSDDone(ConfigStreamClient* client, std::string host) override {
        std::cout << "ADC server stopped on " << host << ": Data written to SD card" << std::endl;
    }

    void adcServerStartedTCP(ConfigStreamClient* client, std::string host) override { std::cout << "ADC server started on " << host << " (TCP mode)" << std::endl; }

    void adcServerStartedSD(ConfigStreamClient* client, std::string host) override { std::cout << "ADC server started on " << host << " (SD card mode)" << std::endl; }

    void adcServerStartedFPGA(ConfigStreamClient* client, std::string host) override { std::cout << "ADC server started on " << host << " (FPGA)" << std::endl; }

    void sigInt() override {
        if (obj)
            obj->notifyStop();
    }
};

int main() {
    // Creating a streaming client
    auto confClient = std::make_shared<ConfigStreamClient>();
    auto confCallback = std::make_shared<ConfigCallbackImpl>();
    confClient->addCallback(confCallback);
    obj = std::make_shared<ADCStreamClient>(confClient);

    // Creating a callback handler.And also remove the owner, since the client itself will delete the handler.
    auto callback = std::make_shared<Callback>();
    obj->setCallback(callback);

    // Enable client logs
    confClient->setVerbose(true);
    obj->setVerbose(true);

    // Connect to the server
    if (!confClient->connect()) {
        std::cerr << "The client did not connect" << std::endl;
        return 1;
    }

    // Get the current decimation setting
    std::string current_decimation = confClient->getConfig("adc_decimation");
    std::cout << "Current decimation " << current_decimation << std::endl;

    // Setting up network mode
    confClient->sendConfig("adc_pass_mode", "NET");

    // Setting up a new decimation setting
    confClient->sendConfig("adc_decimation", "64");

    // Setting the memory block size
    confClient->sendConfig("block_size", "16384");

    // Setting the size of reserved memory for ADC streaming
    confClient->sendConfig("adc_size", "1638400");

    // Turn on the first and second channels
    confClient->sendConfig("channel_state_1", "ON");
    confClient->sendConfig("channel_state_2", "ON");

    if (obj->startStreaming()) {
        std::cout << "Streaming is launched" << std::endl;
    } else {
        std::cerr << "Error starting streaming" << std::endl;
        return 1;
    }

    // Waiting for the streaming client to complete its work
    obj->wait();

    std::cout << "Received samples " << callback->counter << std::endl;
    std::cout << "Number of lost samples " << callback->fpgaLost << std::endl;

    return 0;
}