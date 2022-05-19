#ifndef DAC_STREAMING_H
#define DAC_STREAMING_H

#include "config_net_lib/server_net_config_manager.h"
#include "dac_streaming_lib/dac_streaming_manager.h"
#include "options.h"

auto startDACServer(bool verbMode ,bool testMode) -> void;
auto stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NotifyResult res) -> void;
auto stopDACServer(dac_streaming_lib::CDACStreamingManager::NotifyResult x) -> void;
auto setDACServer(ServerNetConfigManager::Ptr serverNetConfig) -> void;

#endif
