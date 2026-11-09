#ifndef DAC_STREAMING_H
#define DAC_STREAMING_H

#include "config_net_lib/client_net_config_manager.h"
#include "options.h"

// false: the boards refused, the file could not be played or sent
auto startDACStreaming(ClientNetConfigManager::Ptr cl, ClientOpt::Options& option) -> bool;
auto dac_streamingSIGHandler() -> void;

#endif
