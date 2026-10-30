#ifndef STREAMING_H
#define STREAMING_H

#include "config_net_lib/server_net_config_manager.h"
#include "uio_lib/hardware.h"

auto startServer(bool verbMode, bool testMode) -> void;
auto setHardware(uio_lib::CHardware::Ptr hw) -> void;
auto stopNonBlocking(ServerNetConfigManager::EStopReason x) -> void;
auto stopServer(ServerNetConfigManager::EStopReason reason) -> void;
auto setServer(std::shared_ptr<ServerNetConfigManager> serverNetConfig) -> void;
auto startADC() -> void;

#endif
