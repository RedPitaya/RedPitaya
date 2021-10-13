#pragma once

#include "options.h"
#include "ClientNetConfigManager.h"

auto sleepMs(int ms) -> void;
auto startConfig(ClientOpt::Options &option) -> void;
auto configSIGHandler() -> void;