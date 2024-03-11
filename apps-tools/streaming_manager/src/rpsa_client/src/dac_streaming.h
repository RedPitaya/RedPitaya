#ifndef DAC_STREAMING_H
#define DAC_STREAMING_H

#include "options.h"
#include "config.h"
#include "config_net_lib/client_net_config_manager.h"
#include <chrono>

auto startDACStreaming(ClientNetConfigManager::Ptr cl,std::string &conf) -> void;
auto startDACStreaming(ClientNetConfigManager::Ptr cl,ClientOpt::Options &option) -> void;
auto dac_streamingSIGHandler() -> void;

#endif
