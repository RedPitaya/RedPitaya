#include <vector>

// System includes
#ifndef _WIN32
#include <syslog.h>
#endif

// Project includes
#include "dac_streaming.h"
#include "dac_streaming_lib/dac_streaming_application.h"
#include "dac_streaming_lib/dac_streaming_manager.h"
#include "options.h"
#include "uio_lib/hardware.h"

using namespace dac_streaming_lib;
using namespace uio_lib;

// ============================================================================
// Global variables
// ============================================================================
CDACStreamingApplication::Ptr g_dac_app = nullptr;
CDACStreamingManager::Ptr g_dac_manger = nullptr;
CHardware::Ptr g_dac_hw = nullptr;
ServerNetConfigManager::Ptr g_serverDACNetConfig = nullptr;

bool g_dac_verbMode = false;
std::atomic_bool g_dac_serverRun(false);

// ============================================================================
// Helper functions
// ============================================================================

auto setDACServer(ServerNetConfigManager::Ptr serverNetConfig) -> void {
    g_serverDACNetConfig = serverNetConfig;
}

auto setDACHardware(CHardware::Ptr hw) -> void {
    g_dac_hw = hw;
}

auto stopDACServer(CDACStreamingManager::NotifyResult x) -> void {
    try {
        // Stop and cleanup application
        if (g_dac_app) {
            g_dac_app->stop();
            g_dac_app = nullptr;
        }

        // Cleanup manager and memory
        g_dac_manger = nullptr;
        auto memmanager = uio_lib::CMemoryManager::instance();
        memmanager->releaseMemory(MM_DAC);

        // Send status notifications
        if (g_serverDACNetConfig) {
            switch (x) {
                case CDACStreamingManager::NR_STOP:
                    g_serverDACNetConfig->sendDACServerStopped();
                    break;
                case CDACStreamingManager::NR_ENDED:
                    g_serverDACNetConfig->sendDACServerStoppedSDDone();
                    break;
                case CDACStreamingManager::NR_EMPTY:
                    g_serverDACNetConfig->sendDACServerStoppedSDEmpty();
                    break;
                case CDACStreamingManager::NR_BROKEN:
                    g_serverDACNetConfig->sendDACServerStoppedSDBroken();
                    break;
                case CDACStreamingManager::NR_MISSING_FILE:
                    g_serverDACNetConfig->sendDACServerStoppedSDMissingFile();
                    break;
                case CDACStreamingManager::NR_MEM_ERROR:
                    g_serverDACNetConfig->sendDACServerMemoryErrorStopped();
                    break;
                case CDACStreamingManager::NR_MEM_MODIFY:
                    g_serverDACNetConfig->sendDACServerMemoryModifyStopped();
                    break;
                case CDACStreamingManager::NR_SETTINGS_ERROR:
                    g_serverDACNetConfig->sendDACServerConfigErrorStopped();
                    break;
                default:
                    throw std::runtime_error("Unknown state");
                    break;
            }
        }

        // Log stop
        if (g_dac_verbMode) {
            printWithLog(LOG_NOTICE, stdout, "[Streaming] Stop dac server\n");
        }
    } catch (std::exception& e) {
        printWithLog(LOG_NOTICE, stdout, "Error: stopDACServer() %s\n", e.what());
    }
}

auto stopDACNonBlocking(CDACStreamingManager::NotifyResult x) -> void {
    try {
        std::thread th(stopDACServer, x);
        th.detach();
    } catch (std::exception& e) {
        printWithLog(LOG_ERR, stderr, "Error: StopNonBlocking() %s\n", e.what());
    }
}


// ============================================================================
// Streaming mode initialization
// ============================================================================

/**
 * Create network-based streaming manager
 */
auto createNetworkManager(const std::string& ip_addr_host, bool verbMode) -> CDACStreamingManager::Ptr {
    return CDACStreamingManager::Create(ip_addr_host, verbMode);
}

/**
 * Create file-based streaming manager; activeChannels gets the channels of the file
 */
auto createFileManager(const CStreamSettings& settings, bool verbMode, dac_channels_t& activeChannels) -> CDACStreamingManager::Ptr {
    auto format = settings.getDACFileType();
    auto filePath = settings.getDACFile();
    auto dacRepeatMode = settings.getDACRepeat();
    auto dacRepeatCount = settings.getDACRepeatCount();
    std::string fullPath = CStreamSettings::getDACDirPath() + "/" + filePath;

    CDACStreamingManager::Ptr manager = nullptr;

    if (format.value == CStreamSettings::DataFormat::WAV) {
        manager = CDACStreamingManager::Create(CDACStreamingManager::WAV_TYPE, fullPath, dacRepeatMode, dacRepeatCount, settings.getMemoryBlockSize(), verbMode);
    } else if (format.value == CStreamSettings::DataFormat::TDMS) {
        manager = CDACStreamingManager::Create(CDACStreamingManager::TDMS_TYPE, fullPath, dacRepeatMode, dacRepeatCount, settings.getMemoryBlockSize(), verbMode);
    }

    if (manager) {
        // Register stop callback
        manager->notifyStop.connect([](CDACStreamingManager::NotifyResult status) { stopDACNonBlocking(status); });

        // Validate channels
        if (!manager->getChannels(activeChannels)) {
            printWithLog(LOG_ERR, stdout, "There are no channels in the file.\n");
            return nullptr;
        }
    }

    return manager;
}

// ============================================================================
// Memory management
// ============================================================================

/**
 * Initialize and allocate memory for DAC streaming
 */
auto initializeMemory(const CStreamSettings& settings, dac_channels_t activeChannels) -> bool {
    auto memmanager = uio_lib::CMemoryManager::instance();

    // Configure memory
    memmanager->releaseMemory(MM_DAC);
    memmanager->setReserverdMemory(uio_lib::MM_DAC, settings.getDACSize());

    // Validate memory size
    auto ramSize = memmanager->getReserverdMemory(MM_DAC);
    if (ramSize < memmanager->getMinRAMSize(MM_DAC)) {
        printWithLog(LOG_ERR, stdout, "Not enough memory for MM_DAC mode\n");
        stopDACNonBlocking(CDACStreamingManager::NR_MEM_ERROR);
        return false;
    }

    // Allocate memory blocks
    auto mbSize = memmanager->getMemoryBlockSize();
    auto freeblocks = ramSize / mbSize;
    auto reservedBlocks = memmanager->reserveMemory(MM_DAC, freeblocks, activeChannels.count());

    if (reservedBlocks == 0) {
        printWithLog(LOG_ERR, stdout, "Can't reserve memory via memory manager\n");
        stopDACNonBlocking(CDACStreamingManager::NR_MEM_ERROR);
        return false;
    }

    // Generate buffer structures
    g_dac_manger->getBufferManager()->generateBuffersEmptyDAC(activeChannels, memmanager->getRegions(MM_DAC), DataLib::sizeHeader());

    return true;
}

// ============================================================================
// Main DAC server control
// ============================================================================

/**
 * Start DAC server with specified configuration
 */
auto startDACServer(bool verbMode, dac_channels_t activeChannels) -> void {
    if (!g_serverDACNetConfig || !g_dac_hw) {
        return;
    }

    // Reset global state
    g_dac_app = nullptr;
    g_dac_manger = nullptr;
    g_dac_verbMode = verbMode;

    try {
        // Get configuration
        auto memmanager = uio_lib::CMemoryManager::instance();
        CStreamSettings settings = g_serverDACNetConfig->getSettings();
        g_dac_serverRun = true;

        auto use_file = settings.getDACPassMode();
        auto dac_speed = settings.getDACSpeed();
        auto ip_addr_host = "127.0.0.1";

        // The rate, the calibration and the gain of the outputs
        DACConfig dac;
        dac.rate = dac_speed;
        dac.gain5x = {settings.getDACGain(1).value == CStreamSettings::DACGain::X5, settings.getDACGain(2).value == CStreamSettings::DACGain::X5};
        dac.bufferBytes = settings.getMemoryBlockSize();
        if (!g_dac_hw->dacConfigure(dac)) {
            printWithLog(LOG_ERR, stderr, "[Streaming] Can't configure the DAC (rate %u)\n", dac_speed);
            stopDACNonBlocking(CDACStreamingManager::NR_SETTINGS_ERROR);
            return;
        }

        // Create streaming manager based on mode
        if (use_file.value == CStreamSettings::DACPassMode::DAC_NET) {
            g_dac_manger = createNetworkManager(ip_addr_host, verbMode);
        } else if (use_file.value == CStreamSettings::DACPassMode::DAC_FILE) {
            g_dac_manger = createFileManager(settings, verbMode, activeChannels);
            if (!g_dac_manger) {
                g_serverDACNetConfig->sendDACServerStoppedSDBroken();
                return;
            }
        }

        // Initialize memory
        if (!initializeMemory(settings, activeChannels)) {
            return;
        }

        // Create and start application
        g_dac_app = std::make_shared<CDACStreamingApplication>(g_dac_manger, g_dac_hw);
        g_dac_app->setVerboseMode(g_dac_verbMode);

        // Started goes out before the playback: a short file can end at once, and the stop
        // (which also drops g_dac_manger) must not reach the client first
        if (g_dac_manger->isLocalMode()) {
            g_serverDACNetConfig->sendDACServerStartedSD();
        } else {
            g_serverDACNetConfig->sendDACServerStarted();
        }
        g_dac_app->runNonBlock();

        if (g_dac_verbMode) {
            printWithLog(LOG_ERR, stderr, "[Streaming] Start dac server\n");
        }
    } catch (std::exception& e) {
        printWithLog(LOG_ERR, stderr, "Error: startDACServer() %s\n", e.what());
    }
}
