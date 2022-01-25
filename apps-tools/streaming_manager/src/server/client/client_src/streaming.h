#pragma once

#include "options.h"
#include "config.h"
#include "ClientNetConfigManager.h"


auto startStreaming(std::shared_ptr<ClientNetConfigManager> cl,ClientOpt::Options &option) -> void;
auto streamingSIGHandler() -> void;
