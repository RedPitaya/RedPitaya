#pragma once

#include "options.h"
#include "config.h"
#include "ClientNetConfigManager.h"


auto startStreaming(ClientOpt::Options &option) -> void;
auto streamingSIGHandler() -> void;
