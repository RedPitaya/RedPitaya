#ifndef AMS_MONITOR_CONFIG_H
#define AMS_MONITOR_CONFIG_H

// ============================================================================
// Configuration structure for AmsMonitor
// ============================================================================
struct AmsMonitorConfig {
    int updateIntervalMs = 200;   // Interval between updates in milliseconds
    int selectedChannels[16] = {  // Default channels to display (all 16)
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        8,
        9,
        10,
        11,
        12,
        13,
        14,
        15};
    int numSelectedChannels = 16;  // Number of channels in selection
};

#endif  // AMS_MONITOR_CONFIG_H