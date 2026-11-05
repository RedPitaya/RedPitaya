#ifndef GPIO_STREAMING_H
#define GPIO_STREAMING_H

#include "config_net_lib/server_net_config_manager.h"
#include "gpio_streaming_lib/gpio_in_stream_server.h"
#include "gpio_streaming_lib/gpio_out_stream_server.h"
#include "uio_lib/hardware.h"

auto startGPIOInServer(bool verbMode) -> void;
auto stopGPIOInNonBlocking(gpio_streaming_lib::GPIOStopReason reason) -> void;
auto stopGPIOInServer(gpio_streaming_lib::GPIOStopReason reason) -> void;
auto startGPIOOutServer(bool verbMode) -> void;
auto stopGPIOOutNonBlocking(gpio_streaming_lib::GPIOStopReason reason) -> void;
auto stopGPIOOutServer(gpio_streaming_lib::GPIOStopReason reason) -> void;
auto setGPIOServer(ServerNetConfigManager::Ptr serverNetConfig) -> void;
auto setGPIOHardware(uio_lib::CHardware::Ptr hw) -> void;

#endif
