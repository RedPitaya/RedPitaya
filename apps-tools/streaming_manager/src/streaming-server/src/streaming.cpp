#include <algorithm>
#include <memory>

// Platform-specific includes
#ifdef RP_PLATFORM
#include "api250-12/rp-gpio-power.h"
#include "api250-12/rp-i2c-max7311.h"
#include "api250-12/rp-spi.h"
#include "rp_hw-profiles.h"
#include "rp_hw_calib.h"
#endif

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
#include "uio_lib/oscilloscope.h"

#include "options.h"
#include "streaming.h"

using namespace streaming_lib;
using namespace DataLib;
using namespace uio_lib;

// ============================================================================
// Global variables
// ============================================================================

COscilloscope::Ptr g_osc = nullptr;
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

// ============================================================================
// Stop/cleanup functions
// ============================================================================

auto stopServer(ServerNetConfigManager::EStopReason reason) -> void {
    try {
        if (g_s_buffer) {
            g_s_buffer->notifyToDestory();
        }

        g_s_fpga = nullptr;
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
// Platform-specific calibration
// ============================================================================

#ifdef RP_PLATFORM

struct CalibrationData {
    int32_t ch_off[MAX_ADC_CHANNELS] = {0, 0, 0, 0};
    double ch_gain[MAX_ADC_CHANNELS] = {1, 1, 1, 1};
    uint32_t aa_ch[MAX_ADC_CHANNELS] = {0, 0, 0, 0};
    uint32_t bb_ch[MAX_ADC_CHANNELS] = {0, 0, 0, 0};
    uint32_t kk_ch[MAX_ADC_CHANNELS] = {0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF};
    uint32_t pp_ch[MAX_ADC_CHANNELS] = {0, 0, 0, 0};
    bool isNewCalib = false;
};

auto initPlatformCalibration(const CStreamSettings& settings, CalibrationData& calib, uint8_t max_channels, bool filterBypass) -> void {
    // Check calibration version
    uint8_t cver = 0;
    if (rp_GetCalibrationVersion(&cver) == RP_HW_CALIB_OK) {
        calib.isNewCalib = cver >= RP_HW_PACK_ID_V6;
    } else {
        ERROR_LOG("Error get calibration version");
    }

    for (uint8_t ch = 0; ch < max_channels; ++ch) {
        rp_acq_ac_dc_mode_calib_t mode = (settings.getADCAC_DC(ch + 1).value == CStreamSettings::AC_DC::DC) ? RP_DC_CALIB : RP_AC_CALIB;
        bool isAttenuator1_1 = (settings.getADCAttenuator(ch + 1).value == CStreamSettings::Attenuator::A_1_1);

        if (isAttenuator1_1) {
            if (rp_CalibGetFastADCCalibValue((rp_channel_calib_t)ch, mode, &calib.ch_gain[ch], &calib.ch_off[ch]) != RP_HW_CALIB_OK) {
                fprintf(stderr, "Error get calibration channel: %d\n", ch);
            }
            if (!filterBypass) {
                channel_filter_t f;
                if (rp_CalibGetFastADCFilter((rp_channel_calib_t)ch, &f) == RP_HW_CALIB_OK) {
                    calib.aa_ch[ch] = f.aa;
                    calib.bb_ch[ch] = f.bb;
                    calib.kk_ch[ch] = f.kk;
                    calib.pp_ch[ch] = f.pp;
                } else {
                    fprintf(stderr, "Error get filter value: %d\n", ch);
                }
            }
        } else {
            if (rp_CalibGetFastADCCalibValue_1_20((rp_channel_calib_t)ch, mode, &calib.ch_gain[ch], &calib.ch_off[ch]) != RP_HW_CALIB_OK) {
                fprintf(stderr, "Error get calibration channel: %d\n", ch);
            }
            if (!filterBypass) {
                channel_filter_t f;
                if (rp_CalibGetFastADCFilter_1_20((rp_channel_calib_t)ch, &f) == RP_HW_CALIB_OK) {
                    calib.aa_ch[ch] = f.aa;
                    calib.bb_ch[ch] = f.bb;
                    calib.kk_ch[ch] = f.kk;
                    calib.pp_ch[ch] = f.pp;
                } else {
                    fprintf(stderr, "Error get filter value: %d\n", ch);
                }
            }
        }
    }

    // Configure hardware attenuators
    if (rp_HPGetIsAttenuatorControllerPresentOrDefault()) {
        rp_max7311::rp_setAttenuator(RP_MAX7311_IN1, settings.getADCAttenuator(1).value == CStreamSettings::Attenuator::A_1_20 ? RP_ATTENUATOR_1_20 : RP_ATTENUATOR_1_1);
        rp_max7311::rp_setAttenuator(RP_MAX7311_IN2, settings.getADCAttenuator(2).value == CStreamSettings::Attenuator::A_1_20 ? RP_ATTENUATOR_1_20 : RP_ATTENUATOR_1_1);
    }

    // Configure AC/DC coupling
    if (rp_HPGetFastADCIsAC_DCOrDefault()) {
        rp_max7311::rp_setAC_DC(RP_MAX7311_IN1, settings.getADCAC_DC(1).value == CStreamSettings::AC_DC::DC ? RP_DC_MODE : RP_AC_MODE);
        rp_max7311::rp_setAC_DC(RP_MAX7311_IN2, settings.getADCAC_DC(2).value == CStreamSettings::AC_DC::DC ? RP_DC_MODE : RP_AC_MODE);
    }
}

auto createPlatformOscilloscope(const std::vector<UioT>& uioList, uint32_t rate, bool is_master, bool filterBypass, uint8_t max_channels, const CalibrationData& calib,
                                uint8_t resolution_bits) -> COscilloscope::Ptr {
    auto memmanager = uio_lib::CMemoryManager::instance();

    for (const auto& uio : uioList) {
        if (uio.nodeName == "rp_oscilloscope@40000000") {
            memmanager->releaseMemory(MM_ADC_RESERVE_SKIP);
            if (memmanager->reserveMemory(MM_ADC_RESERVE_SKIP, 1, 1) != 1) {
                printWithLog(LOG_ERR, stdout, "Can't reserve memory via memory manager\n");
                stopNonBlocking(ServerNetConfigManager::MEM_ERROR);
                return nullptr;
            }
            auto blocks = memmanager->getRegions(MM_ADC_RESERVE_SKIP);

            auto osc = COscilloscope::create(uio, rate, is_master, ClientOpt::getADCRate(), !filterBypass, ClientOpt::getADCBits(), max_channels);

            for (uint8_t ch = 0; ch < max_channels; ++ch) {
                osc->setCalibration(ch, calib.ch_off[ch], calib.ch_gain[ch], calib.isNewCalib);
                osc->setFilterCalibration(ch, calib.aa_ch[ch], calib.bb_ch[ch], calib.kk_ch[ch], calib.pp_ch[ch]);
            }

            osc->setFilterBypass(filterBypass);
            osc->set8BitMode(resolution_bits == 8);
            osc->setSkipDataAddress(blocks[0]);

            return osc;
        }
    }

    return nullptr;
}

#endif

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

auto startServer(bool verbMode, bool testMode, bool is_master) -> void {
    if (!g_serverNetConfig) {
        return;
    }

    g_s_fpga = nullptr;
    g_osc = nullptr;
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
        auto max_channels = ClientOpt::getADCChannels();
        auto buffersTestMode = testMode;
        auto uioList = uio_lib::GetUioList();
        uint8_t resolution_bits = (resolution.value == CStreamSettings::Resolution::BIT_8) ? 8 : 16;

#ifdef RP_PLATFORM
        if (rp_CalibInit() != RP_HW_CALIB_OK) {
            fprintf(stderr, "Error init calibration\n");
        }
        bool filterBypass = !rp_HPGetFastADCIsFilterPresentOrDefault();
        bool is_platform = true;
        CalibrationData calib;
        if (settings.getADCCalibration()) {
            initPlatformCalibration(settings, calib, max_channels, filterBypass);
        }

        g_osc = createPlatformOscilloscope(uioList, rate, is_master, filterBypass, max_channels, calib, resolution_bits);
        if (!g_osc) {
            return;
        }
#else
        buffersTestMode = true;
        bool is_platform = false;
        uio_lib::UioT uio_t;
        g_osc = COscilloscope::create(uio_t, rate, is_master, ClientOpt::getADCRate(), false, 16, 2);
#endif

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
        auto memmanager = uio_lib::CMemoryManager::instance();
        g_s_buffer->generateBuffers(memmanager->getRegions(MM_ADC), (use_file.value == CStreamSettings::PassMode::NET ? DataLib::sizeHeader() : 0), buffersTestMode);
        g_s_buffer->setADCBits(ClientOpt::getADCBits());
        g_s_buffer->setOSCRate(ClientOpt::getADCRate() / rate);

        if (use_file.value == CStreamSettings::PassMode::NET) {
            g_s_buffer->initHeadersADC();
        }

        // Create FPGA streaming
        g_s_fpga = std::make_shared<streaming_lib::CStreamingFPGA>(g_osc, save_capture_time);
        g_osc->setDataSize(g_s_buffer->getDataSize());
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
