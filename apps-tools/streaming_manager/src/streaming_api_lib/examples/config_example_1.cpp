#include <algorithm>
#include <iostream>
#include <string>
#include "callbacks.h"
#include "config_streaming.h"

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

    void dacServerStartedTCP(ConfigStreamClient* client, std::string host) override { std::cout << "DAC server started on " << host << " (TCP mode)" << std::endl; }

    void dacServerStartedSD(ConfigStreamClient* client, std::string host) override { std::cout << "DAC server started on " << host << " (SD card mode)" << std::endl; }

    void dacServerStoppedMemError(ConfigStreamClient* client, std::string host) override { std::cout << "DAC server stopped on " << host << ": Memory error" << std::endl; }

    void dacServerStoppedMemModify(ConfigStreamClient* client, std::string host) override { std::cout << "DAC server stopped on " << host << ": Memory modified" << std::endl; }

    void dacServerStoppedConfigError(ConfigStreamClient* client, std::string host) override {
        std::cout << "DAC server stopped on " << host << ": Configuration error" << std::endl;
    }

    void dacServerStoppedFileMissed(ConfigStreamClient* client, std::string host) override { std::cout << "DAC server stopped on " << host << ": File missed" << std::endl; }

    void dacServerStoppedSDDone(ConfigStreamClient* client, std::string host) override {
        std::cout << "DAC server stopped on " << host << ": SD card operation completed" << std::endl;
    }

    void dacServerStoppedSDEmpty(ConfigStreamClient* client, std::string host) override { std::cout << "DAC server stopped on " << host << ": SD card is empty" << std::endl; }

    void dacServerStoppedSDBroken(ConfigStreamClient* client, std::string host) override { std::cout << "DAC server stopped on " << host << ": SD card is broken" << std::endl; }

    void dacServerStoppedSDMissing(ConfigStreamClient* client, std::string host) override { std::cout << "DAC server stopped on " << host << ": SD card is missing" << std::endl; }
};

int main() {
    // Creating a streaming client
    ConfigStreamClient obj;

    // Creating a callback handler.And also remove the owner, since the client itself will delete the handler.
    auto callback = std::make_shared<ConfigCallbackImpl>();
    obj.addCallback(callback);

    // Enable client logs
    obj.setVerbose(true);

    // Connect to the server
    if (!obj.connect()) {
        std::cerr << "The client did not connect" << std::endl;
        return 1;
    }

    // Get the current decimation setting
    std::string current_decimation = obj.getConfig("adc_decimation");
    std::cout << "Current decimation " << current_decimation << std::endl;

    obj.requestMemoryBlockSize(obj.getHosts().front());
    obj.requestActiveChannels(obj.getHosts().front());
    // Setting up network mode
    obj.sendConfig("adc_pass_mode", "NET");

    // Setting up a new decimation setting
    obj.sendConfig("adc_decimation", "64");

    // Setting the memory block size
    obj.sendConfig("block_size", "16384");

    // Setting the size of reserved memory for ADC streaming
    obj.sendConfig("adc_size", "1638400");

    // Turn on the first and second channels
    obj.sendConfig("channel_state_1", "ON");
    obj.sendConfig("channel_state_2", "ON");

    return 0;
}