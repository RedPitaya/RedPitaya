#pragma once

#include "rp.h"
#include "DACStreamingApplication.h"
#include "DACStreamingManager.h"
#include "ServerNetConfigManager.h"
#include "options.h"

#ifdef Z20_250_12
#include "rp-spi.h"
#include "rp-i2c-max7311.h"
#endif

auto startDACServer(std::shared_ptr<ServerNetConfigManager> serverNetConfig) -> void;
auto stopDACNonBlocking(int x) -> void;
auto stopDACServer(int x) -> void;
