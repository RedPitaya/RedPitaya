#ifndef GPIO_STREAMING_H
#define GPIO_STREAMING_H

#include "config_net_lib/server_net_config_manager.h"
#include "gpio_streaming_lib/gpio_stream_server.h"
#include "uio_lib/hardware.h"

auto startGPIOServer(bool verbMode) -> void;
auto stopGPIONonBlocking(gpio_streaming_lib::GPIOStopReason reason) -> void;
auto stopGPIOServer(gpio_streaming_lib::GPIOStopReason reason) -> void;
auto setGPIOServer(ServerNetConfigManager::Ptr serverNetConfig) -> void;
auto setGPIOHardware(uio_lib::CHardware::Ptr hw) -> void;

#endif
