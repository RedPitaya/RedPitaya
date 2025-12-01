#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "adc_streaming.h"
#include "callbacks.h"

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
    void recievePack(ADCStreamClient* client, ADCPack& pack) override {
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

/**
 * Main application entry point
 */
int main() {
    // Configuration for master/slave hosts
    std::vector<std::string> hosts = {"200.0.0.7", "200.0.0.8"};

    // Create streaming client instance
    ADCStreamClient obj;
    Callback* callback = new Callback();

    // Setup callback handler
    obj.setReciveDataFunction(callback);
    obj.setVerbose(false);

    // Attempt connection to all hosts
    std::cout << "Attempting connection to hosts:";
    for (const auto& host : hosts) {
        std::cout << " " << host;
    }
    std::cout << std::endl;

    if (!obj.connect(hosts)) {
        std::cerr << "Connection failed to all specified hosts" << std::endl;
        return 1;
    }

    // Configure master host (first in the list)
    const std::string& master_host = hosts[0];

    // Apply configuration to master host
    obj.sendConfig(master_host, "adc_pass_mode", "NET");
    obj.sendConfig(master_host, "adc_decimation", "64");
    obj.sendConfig(master_host, "block_size", "16384");
    obj.sendConfig(master_host, "adc_size", "1638400");
    obj.sendConfig(master_host, "channel_state_1", "ON");
    obj.sendConfig(master_host, "channel_state_2", "ON");

    // Clone configuration from master to slave
    std::string full_config = obj.getFileConfig(master_host);
    obj.sendFileConfig(hosts[1], full_config);

    // Start streaming session
    if (obj.startStreaming()) {
        std::cout << "Streaming session started successfully" << std::endl;
    } else {
        std::cerr << "Failed to start streaming session" << std::endl;
        return 1;
    }

    // Wait for streaming to complete
    obj.wait();

    // Display collected statistics
    std::cout << "\nStreaming results:" << std::endl;
    std::cout << "Received samples per host:" << std::endl;
    for (const auto& [host, count] : callback->counter) {
        std::cout << "  " << host << ": " << count << " samples" << std::endl;
    }

    std::cout << "Lost frames per host:" << std::endl;
    for (const auto& [host, lost] : callback->fpgaLost) {
        std::cout << "  " << host << ": " << lost << " frames" << std::endl;
    }

    return 0;
}