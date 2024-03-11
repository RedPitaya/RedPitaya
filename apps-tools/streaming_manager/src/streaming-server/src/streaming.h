#ifndef STREAMING_H
#define STREAMING_H

#include "options.h"
#include "config_net_lib/server_net_config_manager.h"

auto startServer(bool verbMode,bool testMode,bool is_master) -> void;
auto stopNonBlocking(ServerNetConfigManager::EStopReason x) -> void;
auto stopServer(ServerNetConfigManager::EStopReason reason) -> void;
auto setServer(std::shared_ptr<ServerNetConfigManager> serverNetConfig) -> void;
auto startADC() -> void;

#endif
