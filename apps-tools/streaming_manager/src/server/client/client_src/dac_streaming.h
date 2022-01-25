#pragma once

#include "options.h"
#include "config.h"
#include "ClientNetConfigManager.h"
#include <chrono>

auto startDACStreaming(std::shared_ptr<ClientNetConfigManager> cl,std::string &conf) -> void;
auto startDACStreaming(std::shared_ptr<ClientNetConfigManager> cl,ClientOpt::Options &option) -> void;
auto dac_streamingSIGHandler() -> void;
