#include <algorithm>
#include <iostream>
#include <string>
#include "adc_streaming.h"
#include "callbacks.h"

class Callback : public ADCCallback {
   public:
    uint64_t counter = 0;
    uint64_t fpgaLost = 0;

    void recievePack(ADCStreamClient* client, ADCPack& pack) override {
        counter += pack.channel1.samples + pack.channel2.samples;
        fpgaLost += std::max({pack.channel1.fpgaLost, pack.channel2.fpgaLost, pack.channel3.fpgaLost, pack.channel4.fpgaLost});

        if (counter > 50000000) {
            client->notifyStop();
        }
    }

    void connected(ADCStreamClient* client, std::string host) override { std::cout << "Client connected " << host << std::endl; }

    void disconnected(ADCStreamClient* client, std::string host) override { std::cout << "Client disconnected " << host << std::endl; }

    void error(ADCStreamClient* client, std::string host, int code) override { std::cout << "Client error " << host << " code " << code << std::endl; }

    void stopped(ADCStreamClient* client, std::string host) override { std::cout << "Server stopped " << host << std::endl; }

    void stoppedNoActiveChannels(ADCStreamClient* client, std::string host) override { std::cout << "Server stopped " << host << ". No active channels." << std::endl; }

    void stoppedMemError(ADCStreamClient* client, std::string host) override { std::cout << "Server stopped " << host << ". Memory error." << std::endl; }

    void stoppedMemModify(ADCStreamClient* client, std::string host) override { std::cout << "Server stopped " << host << ". Memory changed" << std::endl; }

    void stoppedSDFull(ADCStreamClient* client, std::string host) override { std::cout << "Server stopped " << host << ". SD is full" << std::endl; }

    void stoppedSDDone(ADCStreamClient* client, std::string host) override { std::cout << "Server stopped " << host << ". The data is written to the memory card." << std::endl; }

    void configConnected(ADCStreamClient* client, std::string host) override { std::cout << "Control client connected " << host << std::endl; }

    void configError(ADCStreamClient* client, std::string host, int code) override { std::cout << "Control client error " << host << " code " << code << std::endl; }

    void configErrorTimeout(ADCStreamClient* client, std::string host) override { std::cout << "Control client error " << host << ". Connection timeout" << std::endl; }
};

int main() {
    // Creating a streaming client
    ADCStreamClient obj;

    // Creating a callback handler.And also remove the owner, since the client itself will delete the handler.
    Callback* callback = new Callback();
    obj.setReciveDataFunction(callback);

    // Disable client logs
    obj.setVerbose(false);

    // Connect to the server
    if (!obj.connect()) {
        std::cerr << "The client did not connect" << std::endl;
        return 1;
    }

    // Get the current decimation setting
    std::string current_decimation = obj.getConfig("adc_decimation");
    std::cout << "Current decimation " << current_decimation << std::endl;

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

    if (obj.startStreaming()) {
        std::cout << "Streaming is launched" << std::endl;
    } else {
        std::cerr << "Error starting streaming" << std::endl;
        return 1;
    }

    // Waiting for the streaming client to complete its work
    obj.wait();

    std::cout << "Received samples " << callback->counter << std::endl;
    std::cout << "Number of lost samples " << callback->fpgaLost << std::endl;

    return 0;
}