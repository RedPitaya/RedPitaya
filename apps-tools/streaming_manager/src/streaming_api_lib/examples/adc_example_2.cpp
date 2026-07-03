#include <algorithm>
#include <csignal>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "adc_streaming.h"
#include "callbacks.h"
#include "config_streaming.h"

std::shared_ptr<ADCStreamClient> obj = nullptr;

/**
 * Callback handler class for ADC streaming events
 * Inherits from ADCCallback to receive streaming events
 */
class Callback : public ADCCallback {
   public:
    // Counters for samples and lost frames per host
    std::unordered_map<std::string, uint64_t> counter;
    std::unordered_map<std::string, uint64_t> fpgaLost;

    /**
     * Called when a new data packet is received
     * @param client Pointer to the ADCStreamClient instance
     * @param pack Reference to the received data packet
     */
    void receivePack(ADCStreamClient* client, ADCPack& pack) override {
        // Update sample counter for this host (sum of channel1 and channel2 samples)
        counter[pack.host] += pack.channel1.samples + pack.channel2.samples;

        // Calculate maximum lost frames between channels
        uint64_t current_max = std::max(pack.channel1.fpgaLost, pack.channel2.fpgaLost);
        fpgaLost[pack.host] += current_max;

        // Stop streaming if we've received enough samples (50 million)
        if (counter[pack.host] > 50000000) {
            client->notifyStop(pack.host);
        }
    }

    // Connection established callback
    void connected(ADCStreamClient* client, std::string host) override { std::cout << "Client connected to " << host << std::endl; }

    // Connection lost callback
    void disconnected(ADCStreamClient* client, std::string host) override { std::cout << "Client disconnected from " << host << std::endl; }

    // Error event callback
    void error(ADCStreamClient* client, std::string host, int code) override { std::cout << "Client error on " << host << " code: " << code << std::endl; }

    // ... [other callback methods with similar documentation] ...
    // Note: Implement all other required virtual methods from ADCCallback here
    // with proper documentation similar to the examples above
};

class ConfigCallbackImpl : public ConfigCallback {
   public:
    void sigInt() override {
        if (obj)
            obj->notifyStop();
    }
};

/**
 * Main application entry point
 */
int main() {
    // Configuration for master/slave hosts
    std::vector<std::string> hosts = {"200.0.0.15", "200.0.0.17"};

    auto confClient = std::make_shared<ConfigStreamClient>();
    obj = std::make_shared<ADCStreamClient>(confClient);

    // Creating a callback handler.And also remove the owner, since the client itself will delete the handler.
    auto confCallback = std::make_shared<ConfigCallbackImpl>();
    confClient->addCallback(confCallback);

    auto callback = std::make_shared<Callback>();
    obj->setCallback(callback);

    // Enable client logs
    confClient->setVerbose(true);
    obj->setVerbose(true);

    // Attempt connection to all hosts
    std::cout << "Attempting connection to hosts:";
    for (const auto& host : hosts) {
        std::cout << " " << host;
    }
    std::cout << std::endl;

    if (!confClient->connect(hosts)) {
        std::cerr << "Connection failed to all specified hosts" << std::endl;
        return 1;
    }

    // Configure master host (first in the list)
    const std::string& master_host = hosts[0];

    // Apply configuration to master host
    confClient->sendConfig(master_host, "adc_pass_mode", "NET");
    confClient->sendConfig(master_host, "adc_decimation", "64");
    confClient->sendConfig(master_host, "block_size", "16384");
    confClient->sendConfig(master_host, "adc_size", "1638400");
    confClient->sendConfig(master_host, "channel_state_1", "ON");
    confClient->sendConfig(master_host, "channel_state_2", "ON");

    // Clone configuration from master to slave
    std::string full_config = confClient->getFileConfig(master_host);
    confClient->sendFileConfig(hosts[1], full_config);

    // Start streaming session
    if (obj->startStreaming()) {
        std::cout << "Streaming session started successfully" << std::endl;
    } else {
        std::cerr << "Failed to start streaming session" << std::endl;
        return 1;
    }

    // Wait for streaming to complete
    obj->wait();

    // Display collected statistics
    std::cout << "\nStreaming results:" << std::endl;
    std::cout << "Received samples per host:" << std::endl;
    for (const auto& [host, count] : callback->counter) {
        std::cout << "  " << host << ": " << count << " samples" << std::endl;
    }

    std::cout << "Lost samples per host:" << std::endl;
    for (const auto& [host, lost] : callback->fpgaLost) {
        std::cout << "  " << host << ": " << lost << " samples" << std::endl;
    }

    return 0;
}