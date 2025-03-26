#include <vector>

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

#include "dac_streaming.h"
#include "dac_streaming_lib/dac_streaming_application.h"
#include "dac_streaming_lib/dac_streaming_manager.h"
#include "options.h"
#include "uio_lib/generator.h"

using namespace dac_streaming_lib;
using namespace uio_lib;

CDACStreamingApplication::Ptr g_dac_app = nullptr;
CDACStreamingManager::Ptr g_dac_manger = nullptr;
CGenerator::Ptr g_gen = nullptr;
ServerNetConfigManager::Ptr g_serverDACNetConfig = nullptr;

bool g_dac_verbMode = false;
std::atomic_bool g_dac_serverRun(false);

auto setDACServer(ServerNetConfigManager::Ptr serverNetConfig) -> void {
    g_serverDACNetConfig = serverNetConfig;
}

auto startDACServer(bool verbMode, uint8_t activeChannels) -> void {
    if (!g_serverDACNetConfig)
        return;
    g_gen = nullptr;
    g_dac_app = nullptr;
    g_dac_manger = nullptr;
    g_dac_verbMode = verbMode;
    try {
        auto memmanager = uio_lib::CMemoryManager::instance();
        CStreamSettings settings = g_serverDACNetConfig->getSettings();

        g_dac_serverRun = true;
        auto use_file = settings.getDACPassMode();
        auto dac_speed = settings.getDACSpeed();
        auto ip_addr_host = "127.0.0.1";

#ifdef RP_PLATFORM
        auto use_calib = true;
        if (rp_CalibInit() != RP_HW_CALIB_OK) {
            aprintf(stderr, "Error init calibration\n");
        }
        CStreamSettings::DACGain dac_gain[2] = {settings.getDACGain(1), settings.getDACGain(2)};
        auto channels_max = ClientOpt::getDACChannels();
#endif

        std::vector<UioT> uioList = GetUioList();
        int32_t ch_off[MAX_DAC_CHANNELS] = {0, 0};
        double ch_gain[MAX_DAC_CHANNELS] = {1, 1};
#ifdef RP_PLATFORM

        if (use_calib) {
            for (uint8_t ch = 0; ch < channels_max; ++ch) {
                rp_gen_gain_calib_t mode = dac_gain[ch].value == CStreamSettings::DACGain::X1 ? RP_GAIN_CALIB_1X : RP_GAIN_CALIB_5X;
                if (rp_CalibGetFastDACCalibValue((rp_channel_calib_t)ch, mode, RP_CALIB_HIZ, &ch_gain[ch], &ch_off[ch]) != RP_HW_CALIB_OK) {
                    aprintf(stderr, "Error get calibration channel: %d\n", ch);
                }
            }
        }

        if (rp_HPGetIsGainDACx5OrDefault()) {
            rp_max7311::rp_setGainOut(RP_MAX7311_OUT1, dac_gain[0].value == CStreamSettings::DACGain::X5 ? RP_GAIN_10V : RP_GAIN_2V);
            rp_max7311::rp_setGainOut(RP_MAX7311_OUT2, dac_gain[1].value == CStreamSettings::DACGain::X5 ? RP_GAIN_10V : RP_GAIN_2V);
        }

        for (const UioT& uio : uioList) {
            if (uio.nodeName == "rp_dac") {
                g_gen = CGenerator::create(uio, true, true, dac_speed, ClientOpt::getDACRate());
                g_gen->setCalibration(ch_off[0], ch_gain[0], ch_off[1], ch_gain[1]);
                if (!g_gen->setDacHz(dac_speed)) {
                    printWithLog(LOG_ERR, stdout, "DAC rate cannot be set.\n");
                    stopDACNonBlocking(CDACStreamingManager::NR_SETTINGS_ERROR);
                    return;
                }
            }
        }
#else
        uio_lib::UioT uio_t;
        g_gen = CGenerator::create(uio_t, true, true, dac_speed, ClientOpt::getDACRate());
        g_gen->setCalibration(ch_off[0], ch_gain[0], ch_off[1], ch_gain[1]);
        g_gen->setDacHz(dac_speed);
#endif

        if (use_file.value == CStreamSettings::DACPassMode::DAC_NET) {
            g_dac_manger = CDACStreamingManager::Create(ip_addr_host, verbMode);
        }

        if (use_file.value == CStreamSettings::DACPassMode::DAC_FILE) {
            auto format = settings.getDACFileType();
            auto filePath = settings.getDACFile();
            auto dacRepeatMode = settings.getDACRepeat();
            auto dacRepeatCount = settings.getDACRepeatCount();
            if (format.value == CStreamSettings::DataFormat::WAV) {
                g_dac_manger = CDACStreamingManager::Create(CDACStreamingManager::WAV_TYPE, CStreamSettings::getDACDirPath() + "/" + filePath, dacRepeatMode,
                                                            dacRepeatCount, settings.getMemoryBlockSize(), verbMode);
            } else if (format.value == CStreamSettings::DataFormat::TDMS) {
                g_dac_manger = CDACStreamingManager::Create(CDACStreamingManager::TDMS_TYPE, CStreamSettings::getDACDirPath() + "/" + filePath, dacRepeatMode,
                                                            dacRepeatCount, settings.getMemoryBlockSize(), verbMode);
            } else {
                g_serverDACNetConfig->sendDACServerStoppedSDBroken();
                return;
            }
            g_dac_manger->notifyStop.connect([](CDACStreamingManager::NotifyResult status) { stopDACNonBlocking(status); });
            bool chActive[2] = {false, false};
            if (!g_dac_manger->getChannels(&chActive[0], &chActive[1])) {
                printWithLog(LOG_ERR, stdout, "There are no channels in the file.\n");
                stopDACNonBlocking(CDACStreamingManager::NR_EMPTY);
                return;
            }
            activeChannels = (int)chActive[0] + (int)chActive[1];
        }

        memmanager->releaseMemory(MM_DAC);
        memmanager->setReserverdMemory(uio_lib::MM_DAC, settings.getDACSize());
        auto mbSize = memmanager->getMemoryBlockSize();
        auto ramSize = memmanager->getReserverdMemory(MM_DAC);
        if (ramSize < memmanager->getMinRAMSize(MM_DAC)) {
            printWithLog(LOG_ERR, stdout, "Not enough memory for MM_DAC mode\n") stopDACNonBlocking(CDACStreamingManager::NR_MEM_ERROR);
            return;
        }
        auto freeblocks = ramSize / mbSize;
        auto reservedBlocks = memmanager->reserveMemory(MM_DAC, freeblocks, activeChannels);

        if (!g_gen) {
            printWithLog(LOG_ERR, stderr, "[Streaming] Error init generator module\n");
            return;
        }

        if (reservedBlocks == 0) {
            printWithLog(LOG_ERR, stdout, "Can't reserve memory via memory manager\n");
            stopDACNonBlocking(CDACStreamingManager::NR_MEM_ERROR);
            return;
        }

        g_dac_manger->getBufferManager()->generateBuffersEmpty(activeChannels, memmanager->getRegions(MM_DAC), DataLib::sizeHeader());

        g_dac_app = std::make_shared<CDACStreamingApplication>(g_dac_manger, g_gen);
        g_gen->setDataSize(settings.getMemoryBlockSize());

        g_dac_app->runNonBlock();
        if (g_dac_manger->isLocalMode()) {
            g_serverDACNetConfig->sendDACServerStartedSD();
        } else {
            g_serverDACNetConfig->sendDACServerStarted();
        }
        if (g_dac_verbMode) {
            printWithLog(LOG_ERR, stderr, "[Streaming] Start dac server\n");
        }
    } catch (std::exception& e) {
        printWithLog(LOG_ERR, stderr, "Error: startDACServer() %s\n", e.what());
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

auto stopDACServer(CDACStreamingManager::NotifyResult x) -> void {
    try {
        if (g_dac_app) {
            g_dac_app->stop();
            g_dac_app = nullptr;
        }

        g_dac_manger = nullptr;
        auto memmanager = uio_lib::CMemoryManager::instance();
        memmanager->releaseMemory(MM_DAC);

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
                    g_serverDACNetConfig->sendDACServerConfigErrorStopped();
                    break;
                case CDACStreamingManager::NR_SETTINGS_ERROR:
                    g_serverDACNetConfig->sendDACServerConfigErrorStopped();
                    break;
                default:
                    throw runtime_error("Unknown state");
                    break;
            }
        }
        if (g_dac_verbMode) {
            printWithLog(LOG_NOTICE, stdout, "[Streaming] Stop dac server\n");
        }
    } catch (std::exception& e) {
        printWithLog(LOG_NOTICE, stdout, "Error: stopDACServer() %s\n", e.what());
    }
}
