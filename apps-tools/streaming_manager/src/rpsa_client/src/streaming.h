#ifndef STREAMING_H
#define STREAMING_H

#include "config_net_lib/client_net_config_manager.h"
#include "options.h"

// false: the boards refused or a stream ended with an error
auto startStreaming(std::shared_ptr<ClientNetConfigManager> cl, ClientOpt::Options& option) -> bool;
auto streamingSIGHandler() -> void;

#endif
