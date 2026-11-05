#include <memory>

#include "uio_lib/memory_manager.h"

#include "gpio_streaming.h"

using namespace gpio_streaming_lib;

// The GPIO input and output of the board; this file only connects them to the config protocol

namespace {

ServerNetConfigManager::Ptr g_serverGPIONetConfig = nullptr;
GPIOInStreamServer::Ptr g_gpioIn = nullptr;
GPIOOutStreamServer::Ptr g_gpioOut = nullptr;

class InCallback : public GPIOInServerCallback {
    void started(GPIOInStreamServer*) override {
        if (g_serverGPIONetConfig)
            g_serverGPIONetConfig->sendGPIOInServerStarted();
    }

    void stopped(GPIOInStreamServer*, GPIOStopReason reason) override {
        if (!g_serverGPIONetConfig)
            return;
        switch (reason) {
            case GPIOStopReason::NORMAL:
            case GPIOStopReason::MEM_MODIFY:  // the protocol has no reply of its own for it
                g_serverGPIONetConfig->sendGPIOInServerStopped();
                break;
            case GPIOStopReason::MEM_ERROR:
                g_serverGPIONetConfig->sendGPIOInServerMemoryErrorStopped();
                break;
            case GPIOStopReason::CONFIG_ERROR:
                g_serverGPIONetConfig->sendGPIOInServerConfigErrorStopped();
                break;
        }
    }
};

class OutCallback : public GPIOOutServerCallback {
    void started(GPIOOutStreamServer*) override {
        if (g_serverGPIONetConfig)
            g_serverGPIONetConfig->sendGPIOOutServerStarted();
    }

    void stopped(GPIOOutStreamServer*, GPIOStopReason reason, uint64_t waitClocks) override {
        if (!g_serverGPIONetConfig)
            return;
        switch (reason) {
            case GPIOStopReason::NORMAL:
            case GPIOStopReason::MEM_MODIFY:
                g_serverGPIONetConfig->sendGPIOOutServerStopped(waitClocks);
                break;
            case GPIOStopReason::MEM_ERROR:
                g_serverGPIONetConfig->sendGPIOOutServerMemoryErrorStopped();
                break;
            case GPIOStopReason::CONFIG_ERROR:
                g_serverGPIONetConfig->sendGPIOOutServerConfigErrorStopped();
                break;
        }
    }
};

}  // namespace

auto setGPIOServer(ServerNetConfigManager::Ptr serverNetConfig) -> void {
    g_serverGPIONetConfig = serverNetConfig;
}

auto setGPIOHardware(uio_lib::CHardware::Ptr hw) -> void {
    g_gpioIn = std::make_shared<GPIOInStreamServer>(hw);
    g_gpioIn->setCallback(std::make_shared<InCallback>());
    g_gpioOut = std::make_shared<GPIOOutStreamServer>(hw);
    g_gpioOut->setCallback(std::make_shared<OutCallback>());
}

auto startGPIOInServer(bool verbMode) -> void {
    if (!g_serverGPIONetConfig || !g_gpioIn)
        return;
    CStreamSettings settings = g_serverGPIONetConfig->getSettings();
    uio_lib::CMemoryManager::instance()->setReserverdMemory(uio_lib::MM_GPIO_IN, settings.getGPIOInSize());
    g_gpioIn->setVerbose(verbMode);
    g_gpioIn->start(settings);
}

auto stopGPIOInServer(GPIOStopReason reason) -> void {
    if (g_gpioIn)
        g_gpioIn->stop(reason);
}

auto stopGPIOInNonBlocking(GPIOStopReason reason) -> void {
    if (g_gpioIn)
        g_gpioIn->stopNonBlocking(reason);
}

auto startGPIOOutServer(bool verbMode) -> void {
    if (!g_serverGPIONetConfig || !g_gpioOut)
        return;
    CStreamSettings settings = g_serverGPIONetConfig->getSettings();
    uio_lib::CMemoryManager::instance()->setReserverdMemory(uio_lib::MM_GPIO_OUT, settings.getGPIOOutSize());
    g_gpioOut->setVerbose(verbMode);
    g_gpioOut->start(settings);
}

auto stopGPIOOutServer(GPIOStopReason reason) -> void {
    if (g_gpioOut)
        g_gpioOut->stop(reason);
}

auto stopGPIOOutNonBlocking(GPIOStopReason reason) -> void {
    if (g_gpioOut)
        g_gpioOut->stopNonBlocking(reason);
}
