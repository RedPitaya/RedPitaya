#include <algorithm>
#include <memory>

#ifdef RP_PLATFORM
#include "api250-12/rp-gpio-power.h"
#include "api250-12/rp-i2c-max7311.h"
#include "api250-12/rp-spi.h"
#include "rp_hw-calib.h"
#include "rp_hw-profiles.h"
#endif

#ifndef _WIN32
#include <syslog.h>
#endif

#include "data_lib/buffers_cached.h"
#include "streaming_lib/streaming_file.h"
#include "streaming_lib/streaming_fpga.h"
#include "streaming_lib/streaming_net.h"
#include "uio_lib/memory_manager.h"
#include "uio_lib/oscilloscope.h"

#include "options.h"
#include "streaming.h"
#include "streaming_fpga.h"

using namespace streaming_lib;
using namespace DataLib;
using namespace uio_lib;

COscilloscope::Ptr g_osc = nullptr;
CStreamingFPGA::Ptr g_s_fpga = nullptr;
CBuffersCached::Ptr g_s_buffer = nullptr;
CStreamingNet::Ptr g_s_net = nullptr;
CStreamingFile::Ptr g_s_file = nullptr;

bool g_verbMode = false;
std::shared_ptr<ServerNetConfigManager> g_serverNetConfig = nullptr;

auto calibFullScaleToVoltage(uint32_t fullScaleGain) -> float {
    /* no scale */
    if (fullScaleGain == 0) {
        return 1;
    }
    return (float)((float)fullScaleGain * 100.0 / ((uint64_t)1 << 32));
}

auto setServer(std::shared_ptr<ServerNetConfigManager> serverNetConfig) -> void {
    g_serverNetConfig = serverNetConfig;
}

auto startServer(bool verbMode, bool testMode, bool is_master) -> void {
    // Search oscilloscope
    if (!g_serverNetConfig)
        return;

    g_s_fpga = {nullptr};
    g_osc = {nullptr};
    g_s_buffer = {nullptr};
    g_s_file = {nullptr};
    g_s_net = {nullptr};
    g_verbMode = verbMode;
    try {
        CStreamSettings settings = g_serverNetConfig->getSettings();

        auto resolution = settings.getADCResolution();
        auto format = settings.getADCFormat();
        auto sock_port = NET_ADC_STREAMING_PORT;
        auto use_file = settings.getADCPassMode();
        auto rate = settings.getADCDecimation();
        auto ip_addr_host = std::string("127.0.0.1");
        auto samples = settings.getADCSamples();
        auto data_type = settings.getADCType();

        auto use_calib __attribute__((unused)) = settings.getADCCalibration();
        auto max_channels = ClientOpt::getADCChannels();
        auto buffersTestMode = testMode;
#ifdef RP_PLATFORM
        use_calib = settings.getADCCalibration();
        if (rp_CalibInit() != RP_HW_CALIB_OK) {
            fprintf(stderr, "Error init calibration\n");
        }
#else
        buffersTestMode = true;
#endif

        auto uioList = uio_lib::GetUioList();
        int32_t ch_off[MAX_ADC_CHANNELS] __attribute__((unused)) = {0, 0, 0, 0};
        double ch_gain[MAX_ADC_CHANNELS] __attribute__((unused)) = {1, 1, 1, 1};
        bool filterBypass = true;
        uint32_t aa_ch[MAX_ADC_CHANNELS] __attribute__((unused)) = {0, 0, 0, 0};
        uint32_t bb_ch[MAX_ADC_CHANNELS] __attribute__((unused)) = {0, 0, 0, 0};
        uint32_t kk_ch[MAX_ADC_CHANNELS] __attribute__((unused)) = {0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF};
        uint32_t pp_ch[MAX_ADC_CHANNELS] __attribute__((unused)) = {0, 0, 0, 0};
        bool is_platform = false;

#ifdef RP_PLATFORM
        is_platform = true;
        filterBypass = !rp_HPGetFastADCIsFilterPresentOrDefault();
        if (use_calib) {
            for (uint8_t ch = 0; ch < max_channels; ++ch) {
                rp_acq_ac_dc_mode_calib_t mode = (settings.getADCAC_DC(ch + 1).value == CStreamSettings::AC_DC::DC ? RP_DC_CALIB : RP_AC_CALIB);
                if (settings.getADCAttenuator(ch + 1).value == CStreamSettings::Attenuator::A_1_1) {
                    if (rp_CalibGetFastADCCalibValue((rp_channel_calib_t)ch, mode, &ch_gain[ch], &ch_off[ch]) != RP_HW_CALIB_OK) {
                        fprintf(stderr, "Error get calibration channel: %d\n", ch);
                    }

                    if (!filterBypass) {
                        channel_filter_t f;
                        if (rp_CalibGetFastADCFilter((rp_channel_calib_t)ch, &f) != RP_HW_CALIB_OK) {
                            fprintf(stderr, "Error get filter value: %d\n", ch);
                        }
                        aa_ch[ch] = f.aa;
                        bb_ch[ch] = f.bb;
                        kk_ch[ch] = f.kk;
                        pp_ch[ch] = f.pp;
                    }
                } else {
                    if (rp_CalibGetFastADCCalibValue_1_20((rp_channel_calib_t)ch, mode, &ch_gain[ch], &ch_off[ch]) != RP_HW_CALIB_OK) {
                        fprintf(stderr, "Error get calibration channel: %d\n", ch);
                    }

                    if (!filterBypass) {
                        channel_filter_t f;
                        if (rp_CalibGetFastADCFilter_1_20((rp_channel_calib_t)ch, &f) != RP_HW_CALIB_OK) {
                            fprintf(stderr, "Error get filter value: %d\n", ch);
                        }
                        aa_ch[ch] = f.aa;
                        bb_ch[ch] = f.bb;
                        kk_ch[ch] = f.kk;
                        pp_ch[ch] = f.pp;
                    }
                }
            }
        }

        if (rp_HPGetIsAttenuatorControllerPresentOrDefault()) {
            rp_max7311::rp_setAttenuator(RP_MAX7311_IN1,
                                         settings.getADCAttenuator(1).value == CStreamSettings::Attenuator::A_1_20 ? RP_ATTENUATOR_1_20 : RP_ATTENUATOR_1_1);
            rp_max7311::rp_setAttenuator(RP_MAX7311_IN2,
                                         settings.getADCAttenuator(2).value == CStreamSettings::Attenuator::A_1_20 ? RP_ATTENUATOR_1_20 : RP_ATTENUATOR_1_1);
        }

        if (rp_HPGetFastADCIsAC_DCOrDefault()) {
            rp_max7311::rp_setAC_DC(RP_MAX7311_IN1, settings.getADCAC_DC(1).value == CStreamSettings::AC_DC::DC ? RP_DC_MODE : RP_AC_MODE);
            rp_max7311::rp_setAC_DC(RP_MAX7311_IN2, settings.getADCAC_DC(2).value == CStreamSettings::AC_DC::DC ? RP_DC_MODE : RP_AC_MODE);
        }

        auto memmanager = uio_lib::CMemoryManager::instance();

        for (auto& uio : uioList) {
            if (uio.nodeName == "rp_oscilloscope") {
                memmanager->releaseMemory(MM_ADC_RESERVE_SKIP);
                if (memmanager->reserveMemory(MM_ADC_RESERVE_SKIP, 1, 1) != 1) {
                    printWithLog(LOG_ERR, stdout, "Can't reserve memory via memory manager") stopNonBlocking(ServerNetConfigManager::MEM_ERROR);
                    return;
                }
                auto blocks = memmanager->getRegions(MM_ADC_RESERVE_SKIP);

                g_osc = COscilloscope::create(uio, rate, is_master, ClientOpt::getADCRate(), !filterBypass, ClientOpt::getADCBits(), max_channels);
                for (uint8_t ch = 0; ch < max_channels; ++ch) {
                    g_osc->setCalibration(ch, ch_off[ch], ch_gain[ch]);
                    g_osc->setFilterCalibration(ch, aa_ch[ch], bb_ch[ch], kk_ch[ch], pp_ch[ch]);
                }
                g_osc->setFilterBypass(filterBypass);
                g_osc->set8BitMode(resolution.value == CStreamSettings::Resolution::BIT_8);
                g_osc->setSkipDataAddress(blocks[0]);
                break;
            }
        }
#else
        auto memmanager = uio_lib::CMemoryManager::instance();
        uio_lib::UioT uio_t;
        g_osc = COscilloscope::create(uio_t, rate, is_master, ClientOpt::getADCRate(), !filterBypass, 16, 2);
#endif
        g_s_buffer = CBuffersCached::create();
        auto g_s_buffer_w = std::weak_ptr<CBuffersCached>(g_s_buffer);

        if (use_file.value == CStreamSettings::PassMode::NET) {
            g_s_net = streaming_lib::CStreamingNet::create(ip_addr_host, sock_port);

            g_s_net->getBuffer = [g_s_buffer_w]() -> CDataBuffersPackDMA::Ptr {
                auto obj = g_s_buffer_w.lock();
                if (obj) {
                    return obj->readBuffer();
                }
                return nullptr;
            };

            g_s_net->unlockBufferF = [g_s_buffer_w]() {
                auto obj = g_s_buffer_w.lock();
                if (obj) {
                    obj->unlockBufferRead();
                }
                return nullptr;
            };
        }

        if (use_file.value == CStreamSettings::PassMode::FILE) {
            auto f_path = std::string(FILE_PATH);
            g_s_file =
                streaming_lib::CStreamingFile::create(format, f_path, samples, data_type.value == CStreamSettings::DataType::VOLT, testMode, is_platform);
            g_s_file->stopNotify.connect([](CStreamingFile::EStopReason r) {
                switch (r) {
                    case CStreamingFile::EStopReason::NORMAL: {
                        stopNonBlocking(ServerNetConfigManager::EStopReason::NORMAL);
                        break;
                    }

                    case CStreamingFile::EStopReason::OUT_SPACE: {
                        stopNonBlocking(ServerNetConfigManager::EStopReason::SD_FULL);
                        break;
                    }

                    case CStreamingFile::EStopReason::REACH_LIMIT: {
                        stopNonBlocking(ServerNetConfigManager::EStopReason::DONE);
                        break;
                    }
                }
            });
        }

        g_s_fpga = std::make_shared<streaming_lib::CStreamingFPGA>(g_osc, 16);
        uint8_t bits = (resolution.value == CStreamSettings::Resolution::BIT_8 ? 8 : 16);

        uint8_t channelsActive = 0;
        for (int i = 0; i < max_channels; i++) {
            if (settings.getADCChannels(i + 1).value == CStreamSettings::State::ON) {
                g_s_buffer->addChannel((DataLib::EDataBuffersPackChannel)i, bits,
                                       settings.getADCAttenuator(i + 1).value == CStreamSettings::Attenuator::A_1_20 ? DataLib::CDataBufferDMA::ATT_1_20
                                                                                                                     : DataLib::CDataBufferDMA::ATT_1_1);
                channelsActive++;
            }
        }
        memmanager->releaseMemory(MM_ADC);
        memmanager->setReserverdMemory(uio_lib::MM_ADC, settings.getADCSize());
        auto mbSize = memmanager->getMemoryBlockSize();
        auto ramSize = memmanager->getReserverdMemory(MM_ADC);
        if (ramSize < memmanager->getMinRAMSize(MM_ADC)) {
            printWithLog(LOG_ERR, stdout, "Not enough memory for ADC mode\n");
            stopNonBlocking(ServerNetConfigManager::EStopReason::MEM_ERROR);
            return;
        }
        auto freeblocks = ramSize / mbSize;
        if (buffersTestMode) {
            // We reduce the number of allocated blocks for test mode. It is necessary that there is no waste of initialization time
            freeblocks = std::min(freeblocks, 100u);
        }
        auto reservedBlocks = memmanager->reserveMemory(MM_ADC, freeblocks, channelsActive);

        if (channelsActive == 0) {
            printWithLog(LOG_ERR, stdout, "No active channels\n") stopNonBlocking(ServerNetConfigManager::EStopReason::NO_CHANNELS);
            return;
        }

        if (reservedBlocks == 0) {
            printWithLog(LOG_ERR, stdout, "Can't reserve memory via memory manager\n") stopNonBlocking(ServerNetConfigManager::EStopReason::MEM_ERROR);
            return;
        }

        if (g_verbMode) {
            printWithLog(LOG_INFO, stdout, "Reserved memory blocks: %d size: %d\n", reservedBlocks, reservedBlocks * memmanager->getMemoryBlockSize());
        }

        g_s_buffer->generateBuffers(memmanager->getRegions(MM_ADC), (use_file.value == CStreamSettings::PassMode::NET ? DataLib::sizeHeader() : 0),
                                    buffersTestMode);
        g_s_buffer->setADCBits(ClientOpt::getADCBits());
        g_s_buffer->setOSCRate(ClientOpt::getADCRate() / rate);
        if (use_file.value == CStreamSettings::PassMode::NET)
            g_s_buffer->initHeadersADC();
        g_osc->setDataSize(g_s_buffer->getDataSize());
        g_s_fpga->setVerbousMode(g_verbMode);
        g_s_fpga->setTestMode(testMode);

        auto weak_obj = std::weak_ptr<CBuffersCached>(g_s_buffer);
        g_s_fpga->getBuffF = [weak_obj]() -> DataLib::CDataBuffersPackDMA::Ptr {
            auto obj = weak_obj.lock();
            if (obj) {
                return obj->writeBuffer();
            }
            return nullptr;
        };

        g_s_fpga->unlockBuffF = [weak_obj]() {
            auto obj = weak_obj.lock();
            if (obj) {
                obj->unlockBufferWrite();
            }
            return nullptr;
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

        char time_str[40];
        struct tm* timenow;
        time_t now = time(nullptr);
        timenow = gmtime(&now);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S", timenow);
        std::string filenameDate = time_str;

        if (g_s_net) {
            g_s_net->run();
            g_serverNetConfig->sendADCServerStartedTCP();
        }

        if (g_s_file) {
            g_s_file->run(filenameDate);
            g_serverNetConfig->sendADCServerStartedSD();
        }

        if (g_verbMode) {
            printWithLog(LOG_NOTICE, stdout, "[Streaming] Start server %s\n", testMode ? "[Benchmark mode]" : "");
        }
    } catch (std::exception& e) {
        printWithLog(LOG_ERR, stderr, "Error: StartServer() %s\n", e.what());
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

auto stopServer(ServerNetConfigManager::EStopReason reason) -> void {
    try {
        if (g_s_buffer)
            g_s_buffer->notifyToDestory();
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
                    throw runtime_error("Unknown state");
                    break;
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

auto startADC() -> void {
    try {
        if (g_s_fpga) {
            g_s_fpga->runNonBlock();
        }
    } catch (std::exception& e) {
        printWithLog(LOG_ERR, stderr, "Error: startADC() %s\n", e.what());
    }
}
