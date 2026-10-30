#include <algorithm>
#include <memory>

// System includes
#ifndef _WIN32
#include <syslog.h>
#endif

// Project includes
#include "data_lib/buffers_cached.h"
#include "streaming_lib/streaming_file.h"
#include "streaming_lib/streaming_fpga.h"
#include "streaming_lib/streaming_net.h"
#include "uio_lib/memory_manager.h"
#include "uio_lib/hardware.h"

#include "options.h"
#include "streaming.h"

using namespace streaming_lib;
using namespace DataLib;
using namespace uio_lib;

// ============================================================================
// Global variables
// ============================================================================

CHardware::Ptr g_hw = nullptr;
CStreamingFPGA::Ptr g_s_fpga = nullptr;
CBuffersCached::Ptr g_s_buffer = nullptr;
CStreamingNet::Ptr g_s_net = nullptr;
CStreamingFile::Ptr g_s_file = nullptr;

bool g_verbMode = false;
std::shared_ptr<ServerNetConfigManager> g_serverNetConfig = nullptr;

// ============================================================================
// Forward declarations
// ============================================================================

auto stopNonBlocking(ServerNetConfigManager::EStopReason x) -> void;
auto stopServer(ServerNetConfigManager::EStopReason reason) -> void;

// ============================================================================
// Utility functions
// ============================================================================

auto setServer(std::shared_ptr<ServerNetConfigManager> serverNetConfig) -> void {
    g_serverNetConfig = serverNetConfig;
}

auto setHardware(CHardware::Ptr hw) -> void {
    g_hw = hw;
}

// ============================================================================
// Stop/cleanup functions
// ============================================================================

auto stopServer(ServerNetConfigManager::EStopReason reason) -> void {
    try {
        if (g_s_buffer) {
            g_s_buffer->notifyToDestory();
        }

        g_s_fpga = nullptr;
        // The acquisition has stopped: what is queued for the SD card still goes into the file
        if (g_s_file)
            g_s_file->stopAndFlush();
        g_s_buffer = nullptr;
        g_s_net = nullptr;
        g_s_file = nullptr;

        if (g_serverNetConfig) {
            switch (reason) {
                case ServerNetConfigManager::EStopReason::NORMAL:
                    g_serverNetConfig->sendServerStopped();
                    break;
                case ServerNetConfigManager::EStopReason::SD_FULL:
                    g_serverNetConfig->sendServerStoppedSDFull();
                    break;
                case ServerNetConfigManager::EStopReason::DONE:
                    g_serverNetConfig->sendServerStoppedDone();
                    break;
                case ServerNetConfigManager::EStopReason::MEM_ERROR:
                    g_serverNetConfig->sendServerMemoryErrorStopped();
                    break;
                case ServerNetConfigManager::EStopReason::MEM_MODIFY:
                    g_serverNetConfig->sendServerMemoryModifyStopped();
                    break;
                case ServerNetConfigManager::EStopReason::NO_CHANNELS:
                    g_serverNetConfig->sendServerNoChannelsStopped();
                    break;
                case ServerNetConfigManager::EStopReason::CONFIG_ERROR:
                    g_serverNetConfig->sendServerConfigErrorStopped();
                    break;
                default:
                    throw std::runtime_error("Unknown state");
            }
        }

        if (g_verbMode) {
            aprintf(stdout, "[Streaming] Stop server\n");
#ifndef _WIN32
            syslog(LOG_NOTICE, "[Streaming] Stop server\n");
#endif
        }
    } catch (std::exception& e) {
        printWithLog(LOG_ERR, stderr, "Error: StopServer() %s\n", e.what());
    }
}

auto stopNonBlocking(ServerNetConfigManager::EStopReason x) -> void {
    try {
        std::thread th(stopServer, x);
        th.detach();
    } catch (std::exception& e) {
        printWithLog(LOG_ERR, stderr, "Error: StopNonBlocking() %s\n", e.what());
    }
}


// ============================================================================
// Memory and channel configuration
// ============================================================================

auto configureChannels(const CStreamSettings& settings, uint8_t max_channels, uint8_t resolution_bits) -> uint8_t {
    uint8_t channelsActive = 0;

    for (int i = 0; i < max_channels; i++) {
        if (settings.getADCChannels(i + 1).value == CStreamSettings::State::ON) {
            g_s_buffer->addChannel(
                (DataLib::EDataBuffersPackChannel)i,
                resolution_bits,
                settings.getADCAttenuator(i + 1).value == CStreamSettings::Attenuator::A_1_20 ? DataLib::CDataBufferDMA::ATT_1_20 : DataLib::CDataBufferDMA::ATT_1_1);
            channelsActive++;
        }
    }

    if (channelsActive == 0) {
        printWithLog(LOG_ERR, stdout, "No active channels\n");
        stopNonBlocking(ServerNetConfigManager::EStopReason::NO_CHANNELS);
    }

    return channelsActive;
}

auto initializeMemory(const CStreamSettings& settings, uint8_t channelsActive, bool buffersTestMode) -> bool {
    auto memmanager = uio_lib::CMemoryManager::instance();

    memmanager->releaseMemory(MM_ADC);
    memmanager->setReserverdMemory(uio_lib::MM_ADC, settings.getADCSize());

    auto ramSize = memmanager->getReserverdMemory(MM_ADC);
    if (ramSize < memmanager->getMinRAMSize(MM_ADC)) {
        printWithLog(LOG_ERR, stdout, "Not enough memory for ADC mode\n");
        stopNonBlocking(ServerNetConfigManager::EStopReason::MEM_ERROR);
        return false;
    }

    auto mbSize = memmanager->getMemoryBlockSize();
    auto freeblocks = ramSize / mbSize;

    if (buffersTestMode) {
        freeblocks = std::min(freeblocks, 100u);
    }

    auto reservedBlocks = memmanager->reserveMemory(MM_ADC, freeblocks, channelsActive);

    if (reservedBlocks == 0) {
        printWithLog(LOG_ERR, stdout, "Can't reserve memory via memory manager\n");
        stopNonBlocking(ServerNetConfigManager::EStopReason::MEM_ERROR);
        return false;
    }

    if (g_verbMode) {
        printWithLog(LOG_INFO, stdout, "Reserved memory blocks: %d size: %d\n", reservedBlocks, reservedBlocks * memmanager->getMemoryBlockSize());
    }

    return true;
}

// ============================================================================
// Main server control
// ============================================================================

auto startServer(bool verbMode, bool testMode) -> void {
    if (!g_serverNetConfig || !g_hw) {
        return;
    }

    g_s_fpga = nullptr;
    g_s_buffer = nullptr;
    g_s_file = nullptr;
    g_s_net = nullptr;
    g_verbMode = verbMode;

    try {
        CStreamSettings settings = g_serverNetConfig->getSettings();

        auto resolution = settings.getADCResolution();
        auto use_file = settings.getADCPassMode();
        auto rate = settings.getADCDecimation();
        auto ip_addr_host = std::string("127.0.0.1");
        auto save_capture_time = settings.getADCCaptureTime();
        auto board = g_hw->info();
        auto max_channels = board.adcChannels;
        auto buffersTestMode = testMode;
        uint8_t resolution_bits = (resolution.value == CStreamSettings::Resolution::BIT_8) ? 8 : 16;

#ifdef RP_PLATFORM
        bool is_platform = true;
#else
        buffersTestMode = true;
        bool is_platform = false;
#endif

        // where the scope writes while no buffer is free
        auto memmanager = uio_lib::CMemoryManager::instance();
        memmanager->releaseMemory(MM_ADC_RESERVE_SKIP);
        if (memmanager->reserveMemory(MM_ADC_RESERVE_SKIP, 1, 1) != 1) {
            printWithLog(LOG_ERR, stdout, "Can't reserve memory via memory manager\n");
            stopNonBlocking(ServerNetConfigManager::MEM_ERROR);
            return;
        }
        g_hw->adcSetSkipBuffer(memmanager->getRegions(MM_ADC_RESERVE_SKIP)[0].start);

        // Create buffer cache
        g_s_buffer = CBuffersCached::create();
        auto g_s_buffer_w = std::weak_ptr<CBuffersCached>(g_s_buffer);

        // Create streaming handlers
        if (use_file.value == CStreamSettings::PassMode::NET) {
            g_s_net = streaming_lib::CStreamingNet::create(ip_addr_host, NET_ADC_STREAMING_PORT);
            g_s_net->getBuffer = [g_s_buffer_w]() -> CDataBuffersPackDMA::Ptr {
                auto obj = g_s_buffer_w.lock();
                return obj ? obj->readBuffer() : nullptr;
            };
            g_s_net->unlockBufferF = [g_s_buffer_w]() {
                auto obj = g_s_buffer_w.lock();
                if (obj)
                    obj->unlockBufferRead();
            };
        }

        if (use_file.value == CStreamSettings::PassMode::FILE) {
            auto path = std::string(FILE_PATH);
            g_s_file = streaming_lib::CStreamingFile::create(
                settings.getADCFormat(), path, settings.getADCSamples(), settings.getADCType().value == CStreamSettings::DataType::VOLT, testMode, is_platform);
            g_s_file->setTDMSTimeColumn(settings.getTDMSTimeColumn().value == CStreamSettings::State::ON);

            g_s_file->stopNotify.connect([](CStreamingFile::EStopReason r) {
                switch (r) {
                    case CStreamingFile::EStopReason::NORMAL:
                        stopNonBlocking(ServerNetConfigManager::EStopReason::NORMAL);
                        break;
                    case CStreamingFile::EStopReason::OUT_SPACE:
                        stopNonBlocking(ServerNetConfigManager::EStopReason::SD_FULL);
                        break;
                    case CStreamingFile::EStopReason::REACH_LIMIT:
                        stopNonBlocking(ServerNetConfigManager::EStopReason::DONE);
                        break;
                }
            });
        }

        // Configure channels
        uint8_t channelsActive = configureChannels(settings, max_channels, resolution_bits);
        if (channelsActive == 0) {
            return;
        }

        // Initialize memory
        if (!initializeMemory(settings, channelsActive, buffersTestMode)) {
            return;
        }

        // Generate buffers
        g_s_buffer->generateBuffers(memmanager->getRegions(MM_ADC), (use_file.value == CStreamSettings::PassMode::NET ? DataLib::sizeHeader() : 0), buffersTestMode);
        g_s_buffer->setADCBits(board.adcBits);
        g_s_buffer->setOSCRate(board.adcRate / rate);

        if (use_file.value == CStreamSettings::PassMode::NET) {
            g_s_buffer->initHeadersADC();
        }

        // Create FPGA streaming
        ADCConfig adc;
        adc.decimation = rate;
        adc.eightBit = resolution_bits == 8;
        adc.calibration = settings.getADCCalibration();
        adc.bufferBytes = g_s_buffer->getDataSize();
        for (uint8_t ch = 0; ch < max_channels && ch < 4; ch++) {
            adc.channels[ch] = settings.getADCChannels(ch + 1).value == CStreamSettings::State::ON;
            adc.att1_20[ch] = settings.getADCAttenuator(ch + 1).value == CStreamSettings::Attenuator::A_1_20;
            adc.dc[ch] = settings.getADCAC_DC(ch + 1).value == CStreamSettings::AC_DC::DC;
        }
        if (!g_hw->adcConfigure(adc)) {
            printWithLog(LOG_ERR, stdout, "Can't configure the ADC\n");
            stopNonBlocking(ServerNetConfigManager::EStopReason::CONFIG_ERROR);
            return;
        }

        g_s_fpga = std::make_shared<streaming_lib::CStreamingFPGA>(g_hw, save_capture_time);
        g_s_fpga->setVerboseMode(g_verbMode);
        g_s_fpga->setTestMode(testMode);

        // Setup callbacks
        auto weak_obj = std::weak_ptr<CBuffersCached>(g_s_buffer);
        g_s_fpga->getBuffF = [weak_obj]() -> DataLib::CDataBuffersPackDMA::Ptr {
            auto obj = weak_obj.lock();
            return obj ? obj->writeBuffer() : nullptr;
        };
        g_s_fpga->unlockBuffF = [weak_obj]() {
            auto obj = weak_obj.lock();
            if (obj)
                obj->unlockBufferWrite();
        };

        auto g_s_file_w = std::weak_ptr<CStreamingFile>(g_s_file);
        g_s_fpga->oscNotify.connect([g_s_file_w, g_s_buffer_w](DataLib::CDataBuffersPackDMA::Ptr) {
            auto f_obj = g_s_file_w.lock();
            auto b_obj = g_s_buffer_w.lock();
            if (f_obj && b_obj) {
                auto p = b_obj->readBuffer();
                if (p) {
                    f_obj->passBuffers(p);
                    b_obj->unlockBufferRead();
                }
            }
        });

        // Start services
        if (g_s_net) {
            g_s_net->run();
            g_serverNetConfig->sendADCServerStartedTCP();
        }

        if (g_s_file) {
            char time_str[40];
            time_t now = time(nullptr);
            struct tm* timenow = gmtime(&now);
            strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S", timenow);
            g_s_file->run(std::string(time_str));
            g_serverNetConfig->sendADCServerStartedSD();
        }

        if (g_verbMode) {
            printWithLog(LOG_NOTICE, stdout, "[Streaming] Start server %s\n", testMode ? "[Benchmark mode]" : "");
        }
    } catch (std::exception& e) {
        printWithLog(LOG_ERR, stderr, "Error: StartServer() %s\n", e.what());
    }
}

auto startADC() -> void {
    try {
        if (g_s_fpga) {
            g_s_fpga->runNonBlock();
        }
    } catch (std::exception& e) {
        printWithLog(LOG_ERR, stderr, "Error: startADC() %s\n", e.what());
    }
}
