#pragma once

#include "rp.h"

#define MAX_BUFFERS 150

auto initOscBeforeLoadConfig() -> void;
auto initExtTriggerLimits() -> void;
auto initOscAfterLoad() -> void;
auto updateOscParametersToWEB() -> void;
auto getOscRunState() -> bool;
auto isAutoScale() -> bool;
auto updateOscSignal() -> void;
auto updateOscParams(bool force) -> void;
auto getOSCTimeScale() -> float;
auto getNeedUpdateSigGen() -> bool;
auto updateTriggerLimit(bool force) -> void;