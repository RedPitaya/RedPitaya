#include <memory>

#include "streaming_lib/adc_stream_server.h"
#include "uio_lib/memory_manager.h"

#include "options.h"
#include "streaming.h"

using namespace streaming_lib;

// The ADC streaming of the board; this file only connects it to the config protocol

namespace {

std::shared_ptr<ServerNetConfigManager> g_serverNetConfig = nullptr;
ADCStreamServer::Ptr g_adc = nullptr;

auto toADC(ServerNetConfigManager::EStopReason reason) -> ADCStopReason {
    switch (reason) {
        case ServerNetConfigManager::EStopReason::SD_FULL:
            return ADCStopReason::SD_FULL;
        case ServerNetConfigManager::EStopReason::DONE:
            return ADCStopReason::DONE;
        case ServerNetConfigManager::EStopReason::MEM_ERROR:
            return ADCStopReason::MEM_ERROR;
        case ServerNetConfigManager::EStopReason::MEM_MODIFY:
            return ADCStopReason::MEM_MODIFY;
        case ServerNetConfigManager::EStopReason::NO_CHANNELS:
            return ADCStopReason::NO_CHANNELS;
        case ServerNetConfigManager::EStopReason::CONFIG_ERROR:
            return ADCStopReason::CONFIG_ERROR;
        default:
            return ADCStopReason::NORMAL;
    }
}

class ProtocolCallback : public ADCServerCallback {
    void startedTCP(ADCStreamServer*) override {
        if (g_serverNetConfig)
            g_serverNetConfig->sendADCServerStartedTCP();
    }

    void startedSD(ADCStreamServer*) override {
        if (g_serverNetConfig)
            g_serverNetConfig->sendADCServerStartedSD();
    }

    void stopped(ADCStreamServer*, ADCStopReason reason) override {
        if (!g_serverNetConfig)
            return;
        switch (reason) {
            case ADCStopReason::NORMAL:
                g_serverNetConfig->sendServerStopped();
                break;
            case ADCStopReason::SD_FULL:
                g_serverNetConfig->sendServerStoppedSDFull();
                break;
            case ADCStopReason::DONE:
                g_serverNetConfig->sendServerStoppedDone();
                break;
            case ADCStopReason::MEM_ERROR:
                g_serverNetConfig->sendServerMemoryErrorStopped();
                break;
            case ADCStopReason::MEM_MODIFY:
                g_serverNetConfig->sendServerMemoryModifyStopped();
                break;
            case ADCStopReason::NO_CHANNELS:
                g_serverNetConfig->sendServerNoChannelsStopped();
                break;
            case ADCStopReason::CONFIG_ERROR:
                g_serverNetConfig->sendServerConfigErrorStopped();
                break;
        }
    }
};

}  // namespace

auto setServer(std::shared_ptr<ServerNetConfigManager> serverNetConfig) -> void {
    g_serverNetConfig = serverNetConfig;
}

auto setHardware(uio_lib::CHardware::Ptr hw) -> void {
    g_adc = std::make_shared<ADCStreamServer>(hw);
    g_adc->setCallback(std::make_shared<ProtocolCallback>());
}

auto startServer(bool verbMode, bool testMode) -> void {
    if (!g_serverNetConfig || !g_adc)
        return;
    CStreamSettings settings = g_serverNetConfig->getSettings();
    uio_lib::CMemoryManager::instance()->setReserverdMemory(uio_lib::MM_ADC, settings.getADCSize());
    ADCStreamOptions options;
    options.testMode = testMode;
    g_adc->setVerbose(verbMode);
    g_adc->start(settings, options);
}

auto startADC() -> void {
    if (g_adc)
        g_adc->startADC();
}

auto stopServer(ServerNetConfigManager::EStopReason reason) -> void {
    if (g_adc)
        g_adc->stop(toADC(reason));
}

auto stopNonBlocking(ServerNetConfigManager::EStopReason reason) -> void {
    if (g_adc)
        g_adc->stopNonBlocking(toADC(reason));
}
