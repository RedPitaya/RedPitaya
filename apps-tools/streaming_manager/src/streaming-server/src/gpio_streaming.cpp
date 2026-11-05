#include <memory>

#include "gpio_streaming_lib/gpio_stream_server.h"
#include "uio_lib/memory_manager.h"

#include "gpio_streaming.h"

using namespace gpio_streaming_lib;

// The GPIO streaming of the board; this file only connects it to the config protocol

namespace {

ServerNetConfigManager::Ptr g_serverGPIONetConfig = nullptr;
GPIOStreamServer::Ptr g_gpio = nullptr;

class ProtocolCallback : public GPIOServerCallback {
    void started(GPIOStreamServer*) override {
        if (g_serverGPIONetConfig)
            g_serverGPIONetConfig->sendGPIOServerStarted();
    }

    void stopped(GPIOStreamServer*, GPIOStopReason reason) override {
        if (!g_serverGPIONetConfig)
            return;
        switch (reason) {
            case GPIOStopReason::NORMAL:
            case GPIOStopReason::MEM_MODIFY:  // the protocol has no reply of its own for it
                g_serverGPIONetConfig->sendGPIOServerStopped();
                break;
            case GPIOStopReason::MEM_ERROR:
                g_serverGPIONetConfig->sendGPIOServerMemoryErrorStopped();
                break;
            case GPIOStopReason::CONFIG_ERROR:
                g_serverGPIONetConfig->sendGPIOServerConfigErrorStopped();
                break;
        }
    }
};

}  // namespace

auto setGPIOServer(ServerNetConfigManager::Ptr serverNetConfig) -> void {
    g_serverGPIONetConfig = serverNetConfig;
}

auto setGPIOHardware(uio_lib::CHardware::Ptr hw) -> void {
    g_gpio = std::make_shared<GPIOStreamServer>(hw);
    g_gpio->setCallback(std::make_shared<ProtocolCallback>());
}

auto startGPIOServer(bool verbMode) -> void {
    if (!g_serverGPIONetConfig || !g_gpio)
        return;
    CStreamSettings settings = g_serverGPIONetConfig->getSettings();
    uio_lib::CMemoryManager::instance()->setReserverdMemory(uio_lib::MM_GPIO, settings.getGPIOSize());
    g_gpio->setVerbose(verbMode);
    g_gpio->start(settings);
}

auto stopGPIOServer(GPIOStopReason reason) -> void {
    if (g_gpio)
        g_gpio->stop(reason);
}

auto stopGPIONonBlocking(GPIOStopReason reason) -> void {
    if (g_gpio)
        g_gpio->stopNonBlocking(reason);
}
