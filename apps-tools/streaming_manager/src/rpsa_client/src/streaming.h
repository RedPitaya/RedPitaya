#ifndef STREAMING_H
#define STREAMING_H

#include "config_net_lib/client_net_config_manager.h"
#include "options.h"

auto startStreaming(std::shared_ptr<ClientNetConfigManager> cl, ClientOpt::Options &option) -> void;
auto streamingSIGHandler() -> void;

#endif
