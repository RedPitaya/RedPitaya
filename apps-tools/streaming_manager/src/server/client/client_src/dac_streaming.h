#pragma once

#include "options.h"
#include "config.h"
#include "ClientNetConfigManager.h"
#include <chrono>

auto startDACStreaming(std::string &conf) -> void;
auto startDACStreaming(ClientOpt::Options &option) -> void;
auto dac_streamingSIGHandler() -> void;
