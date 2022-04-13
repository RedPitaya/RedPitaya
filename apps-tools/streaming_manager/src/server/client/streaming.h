#pragma once

#include "options.h"
#include "config.h"
#include "ClientNetConfigManager.h"
#include <chrono>

auto time_point_to_string(std::chrono::system_clock::time_point &tp) -> std::string;
auto startStreaming(ClientOpt::Options &option) -> void;
auto streamingSIGHandler() -> void;
