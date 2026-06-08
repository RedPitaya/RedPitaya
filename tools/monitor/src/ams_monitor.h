#ifndef AMS_MONITOR_H
#define AMS_MONITOR_H

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "ams_channels.h"
#include "ams_monitor_config.h"

// ============================================================================
// Class for real-time console-based monitoring of AMS parameters.
// Displays gauges with progress bars showing current/min/max values.
// No history kept - only min/max since monitoring started.
// ============================================================================
class AmsMonitor {
   public:
    // Constructor with optional custom configuration
    explicit AmsMonitor(const AmsMonitorConfig& config = AmsMonitorConfig());

    // Destructor ensures monitoring is stopped
    ~AmsMonitor();

    // Delete copy constructor and assignment (manages thread)
    AmsMonitor(const AmsMonitor&) = delete;
    AmsMonitor& operator=(const AmsMonitor&) = delete;

    // Start periodic monitoring in a background thread
    void start();

    // Stop monitoring and join the background thread
    void stop();

    // Check if monitor is currently running
    bool isRunning() const { return running_; }

    // Take a single snapshot of all channels and update min/max
    void snapshot();

    // Display current values (thread-safe)
    void display();

    // Get current configuration (read-only)
    const AmsMonitorConfig& getConfig() const { return config_; }

    // Update configuration while running
    void updateConfig(const AmsMonitorConfig& newConfig);

    // Get the current value for a specific channel
    float getCurrentValue(AmsChannel channel) const;

    // Get min value for a specific channel since monitoring started
    float getMinValue(AmsChannel channel) const;

    // Get max value for a specific channel since monitoring started
    float getMaxValue(AmsChannel channel) const;

    // Reset min/max values
    void resetMinMax();

    // Set which channels to display
    void setDisplayChannels(const int* channels, int count);

   private:
    struct ChannelData {
        float current = 0.0f;
        float min = 0.0f;
        float max = 0.0f;
        bool initialized = false;
    };

    AmsMonitorConfig config_;
    std::array<std::string, static_cast<size_t>(AmsChannel::Count)> descriptions_;
    std::array<ChannelData, static_cast<size_t>(AmsChannel::Count)> channels_;
    std::atomic<bool> running_;
    std::thread monitorThread_;
    std::mutex displayMutex_;

    // Color codes for terminal output
    static const char* COLOR_RESET;
    static const char* COLOR_GREEN;
    static const char* COLOR_YELLOW;
    static const char* COLOR_RED;
    static const char* COLOR_CYAN;
    static const char* COLOR_BOLD;
    static const char* COLOR_DIM;

    // Main monitoring loop (runs in background thread)
    void monitorLoop();

    // Draw functions with output stream parameter
    void drawHeader(std::ostringstream& out);
    void drawGauge(std::ostringstream& out, size_t channelIndex, const ChannelData& data, const std::string& description);
    void drawFooter(std::ostringstream& out);

    // Get color based on value position in range (green/yellow/red)
    const char* getValueColor(float current, float min, float max) const;

    // Clear terminal screen
    static void clearScreen();
};

#endif  // AMS_MONITOR_H