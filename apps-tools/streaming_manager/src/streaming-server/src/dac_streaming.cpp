#include <memory>

#include "dac_streaming_lib/dac_stream_server.h"
#include "uio_lib/memory_manager.h"

#include "dac_streaming.h"

using namespace dac_streaming_lib;

// The DAC streaming of the board; this file only connects it to the config protocol

namespace {

ServerNetConfigManager::Ptr g_serverDACNetConfig = nullptr;
DACStreamServer::Ptr g_dac = nullptr;

class ProtocolCallback : public DACServerCallback {
    void startedTCP(DACStreamServer*) override {
        if (g_serverDACNetConfig)
            g_serverDACNetConfig->sendDACServerStarted();
    }

    void startedSD(DACStreamServer*) override {
        if (g_serverDACNetConfig)
            g_serverDACNetConfig->sendDACServerStartedSD();
    }

    void stopped(DACStreamServer*, DACStopReason reason) override {
        if (!g_serverDACNetConfig)
            return;
        switch (reason) {
            case DACStopReason::NORMAL:
                g_serverDACNetConfig->sendDACServerStopped();
                break;
            case DACStopReason::DONE:
                g_serverDACNetConfig->sendDACServerStoppedSDDone();
                break;
            case DACStopReason::EMPTY:
                g_serverDACNetConfig->sendDACServerStoppedSDEmpty();
                break;
            case DACStopReason::BROKEN:
                g_serverDACNetConfig->sendDACServerStoppedSDBroken();
                break;
            case DACStopReason::MISSING_FILE:
                g_serverDACNetConfig->sendDACServerStoppedSDMissingFile();
                break;
            case DACStopReason::MEM_ERROR:
            case DACStopReason::NO_CHANNELS:  // the protocol has no reply of its own for it
                g_serverDACNetConfig->sendDACServerMemoryErrorStopped();
                break;
            case DACStopReason::MEM_MODIFY:
                g_serverDACNetConfig->sendDACServerMemoryModifyStopped();
                break;
            case DACStopReason::CONFIG_ERROR:
                g_serverDACNetConfig->sendDACServerConfigErrorStopped();
                break;
        }
    }
};

}  // namespace

auto setDACServer(ServerNetConfigManager::Ptr serverNetConfig) -> void {
    g_serverDACNetConfig = serverNetConfig;
}

auto setDACHardware(uio_lib::CHardware::Ptr hw) -> void {
    g_dac = std::make_shared<DACStreamServer>(hw);
    g_dac->setCallback(std::make_shared<ProtocolCallback>());
}

auto startDACServer(bool verbMode, dac_channels_t activeChannels) -> void {
    if (!g_serverDACNetConfig || !g_dac)
        return;
    CStreamSettings settings = g_serverDACNetConfig->getSettings();
    uio_lib::CMemoryManager::instance()->setReserverdMemory(uio_lib::MM_DAC, settings.getDACSize());
    g_dac->setVerbose(verbMode);
    g_dac->start(settings, activeChannels);
}

auto stopDACServer(DACStopReason reason) -> void {
    if (g_dac)
        g_dac->stop(reason);
}

auto stopDACNonBlocking(DACStopReason reason) -> void {
    if (g_dac)
        g_dac->stopNonBlocking(reason);
}
