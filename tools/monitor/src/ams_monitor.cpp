#include "ams_monitor.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include "ams_readers.h"
#include "rp_session.h"

// ANSI color codes
const char* AmsMonitor::COLOR_RESET = "\033[0m";
const char* AmsMonitor::COLOR_GREEN = "\033[32m";
const char* AmsMonitor::COLOR_YELLOW = "\033[33m";
const char* AmsMonitor::COLOR_RED = "\033[31m";
const char* AmsMonitor::COLOR_CYAN = "\033[36m";
const char* AmsMonitor::COLOR_BOLD = "\033[1m";
const char* AmsMonitor::COLOR_DIM = "\033[2m";

// ============================================================================
// Constructor
// ============================================================================
AmsMonitor::AmsMonitor(const AmsMonitorConfig& config) : config_(config), descriptions_(buildDescriptions()), running_(false) {
    RpSession::init();
}

// ============================================================================
// Destructor
// ============================================================================
AmsMonitor::~AmsMonitor() {
    stop();
}

// ============================================================================
// Start monitoring
// ============================================================================
void AmsMonitor::start() {
    if (running_)
        return;

    running_ = true;
    monitorThread_ = std::thread(&AmsMonitor::monitorLoop, this);
}

// ============================================================================
// Stop monitoring
// ============================================================================
void AmsMonitor::stop() {
    running_ = false;
    if (monitorThread_.joinable()) {
        monitorThread_.join();
    }
}

// ============================================================================
// Take snapshot of all channels and update min/max
// ============================================================================
void AmsMonitor::snapshot() {
    for (size_t i = 0; i < static_cast<size_t>(AmsChannel::Count); ++i) {
        float value;
        uint32_t raw;

        if (readFunctions[i](value, raw)) {
            auto& chan = channels_[i];
            chan.current = value;

            if (!chan.initialized) {
                chan.min = value;
                chan.max = value;
                chan.initialized = true;
            } else {
                if (value < chan.min)
                    chan.min = value;
                if (value > chan.max)
                    chan.max = value;
            }
        }
    }
}

// ============================================================================
// Main display function
// ============================================================================
void AmsMonitor::display() {
    std::lock_guard<std::mutex> lock(displayMutex_);

    // Build entire output in string to prevent flickering
    std::ostringstream output;

    // Move cursor to home without clearing (prevents flicker)
    output << "\033[H";

    // Draw header
    drawHeader(output);

    // Draw each channel gauge
    for (int j = 0; j < config_.numSelectedChannels; ++j) {
        int i = config_.selectedChannels[j];
        if (i < 0 || i >= static_cast<int>(AmsChannel::Count))
            continue;

        size_t idx = static_cast<size_t>(i);
        const auto& chan = channels_[idx];

        if (!chan.initialized)
            continue;

        drawGauge(output, idx, chan, descriptions_[idx]);
    }

    // Draw footer
    drawFooter(output);

    // Clear the rest of the screen
    output << "\033[J";

    // Write everything at once
    printf("%s", output.str().c_str());
    fflush(stdout);
}

// ============================================================================
// Draw header with title and timestamp
// ============================================================================
void AmsMonitor::drawHeader(std::ostringstream& out) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    char timeStr[32];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", localtime(&time_t));

    out << COLOR_BOLD << COLOR_CYAN;
    out << "╔═══════════════════════════════════════════════════════════════════════════════════════╗\n";
    out << "║  RED PITAYA MONITOR  │  " << timeStr << "  │  Update: " << config_.updateIntervalMs << "ms";

    // Calculate padding to right border
    int headerLen = 22 + strlen(timeStr) + 14 + std::to_string(config_.updateIntervalMs).length();
    int padding = 83 - headerLen;
    for (int i = 0; i < padding; i++)
        out << " ";
    out << "║\n";

    out << "╠══════════════════════╤══════════╤══════════╤══════════╤═══════════════════════════════╣\n";
    out << "║  Channel             │  Min     │  Current │  Max     │  Gauge                        ║\n";
    out << "╠══════════════════════╪══════════╪══════════╪══════════╪═══════════════════════════════╣\n";
    out << COLOR_RESET;
}

// ============================================================================
// Draw a single gauge with progress bar
// ============================================================================
void AmsMonitor::drawGauge(std::ostringstream& out, size_t channelIndex, const ChannelData& data, const std::string& description) {

    float current = data.current;
    float min = data.min;
    float max = data.max;

    // Add padding to range for better visualization
    float range = max - min;
    if (range < 0.001f) {
        // For stable values, create artificial range around current
        min = current - 0.05f;
        max = current + 0.05f;
        range = 0.1f;
    }

    // Calculate normalized position (0.0 to 1.0)
    float normalized = (current - min) / range;
    normalized = std::max(0.0f, std::min(1.0f, normalized));

    // Choose color based on position
    const char* color = getValueColor(current, min, max);

    // Channel name (left column, fixed width)
    out << "║ " << COLOR_BOLD;
    out << std::left << std::setw(20) << description.substr(0, 20);
    out << COLOR_RESET;

    // Min value
    out << " │ " << COLOR_DIM;
    out << std::right << std::setw(8) << std::fixed << std::setprecision(3) << min;
    out << COLOR_RESET;

    // Current value (highlighted)
    out << " │ " << color << COLOR_BOLD;
    out << std::right << std::setw(8) << std::fixed << std::setprecision(3) << current;
    out << COLOR_RESET;

    // Max value
    out << " │ " << COLOR_DIM;
    out << std::right << std::setw(8) << std::fixed << std::setprecision(3) << max;
    out << COLOR_RESET;

    // Gauge bar
    out << " │ ";

    int barWidth = 27;
    int filledPos = static_cast<int>(normalized * barWidth);

    out << color;
    out << "[";

    for (int i = 0; i < barWidth; ++i) {
        if (i < filledPos) {
            out << "█";  // Filled part
        } else if (i == filledPos) {
            out << COLOR_RESET << COLOR_BOLD << "▌" << color;  // Current position marker
        } else {
            out << "·";  // Empty part (using middle dot for visibility)
        }
    }

    out << "]";
    out << COLOR_RESET << " ║\n";
}

// ============================================================================
// Draw footer with status
// ============================================================================
void AmsMonitor::drawFooter(std::ostringstream& out) {
    out << COLOR_BOLD << COLOR_CYAN;
    out << "╚══════════════════════╧══════════╧══════════╧══════════╧═══════════════════════════════╝\n";
    out << COLOR_RESET;

    out << COLOR_DIM;
    out << "  Ctrl+C to stop  │  Channels: " << config_.numSelectedChannels << "/" << static_cast<int>(AmsChannel::Count);
    out << COLOR_RESET << "\n";
}

// ============================================================================
// Get color based on value position
// ============================================================================
const char* AmsMonitor::getValueColor(float current, float min, float max) const {
    float range = max - min;
    if (range < 0.001f)
        return COLOR_GREEN;

    float normalized = (current - min) / range;

    if (normalized < 0.5f)
        return COLOR_GREEN;
    if (normalized < 0.8f)
        return COLOR_YELLOW;
    return COLOR_RED;
}

// ============================================================================
// Update configuration
// ============================================================================
void AmsMonitor::updateConfig(const AmsMonitorConfig& newConfig) {
    bool wasRunning = running_;
    stop();

    std::lock_guard<std::mutex> lock(displayMutex_);
    config_ = newConfig;

    if (wasRunning)
        start();
}

// ============================================================================
// Set display channels
// ============================================================================
void AmsMonitor::setDisplayChannels(const int* channels, int count) {
    std::lock_guard<std::mutex> lock(displayMutex_);
    count = std::min(count, 16);
    config_.numSelectedChannels = count;
    for (int i = 0; i < count; ++i) {
        config_.selectedChannels[i] = channels[i];
    }
}

// ============================================================================
// Reset min/max values
// ============================================================================
void AmsMonitor::resetMinMax() {
    std::lock_guard<std::mutex> lock(displayMutex_);
    for (auto& chan : channels_) {
        if (chan.initialized) {
            chan.min = chan.current;
            chan.max = chan.current;
        }
    }
}

// ============================================================================
// Get current value
// ============================================================================
float AmsMonitor::getCurrentValue(AmsChannel channel) const {
    size_t index = static_cast<size_t>(channel);
    return channels_[index].current;
}

// ============================================================================
// Get min value
// ============================================================================
float AmsMonitor::getMinValue(AmsChannel channel) const {
    size_t index = static_cast<size_t>(channel);
    return channels_[index].min;
}

// ============================================================================
// Get max value
// ============================================================================
float AmsMonitor::getMaxValue(AmsChannel channel) const {
    size_t index = static_cast<size_t>(channel);
    return channels_[index].max;
}

// ============================================================================
// Main monitoring loop
// ============================================================================
void AmsMonitor::monitorLoop() {
    clearScreen();

    while (running_) {
        snapshot();
        display();

        std::this_thread::sleep_for(std::chrono::milliseconds(config_.updateIntervalMs));
    }

    printf("\033[2J\033[H");
    printf("Monitor stopped.\n");
}

// ============================================================================
// Clear terminal screen
// ============================================================================
void AmsMonitor::clearScreen() {
    printf("\033[2J\033[H");
    fflush(stdout);
}