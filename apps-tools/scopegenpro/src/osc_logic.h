#pragma once

#include "rp.h"

auto initOscAfterLoad() -> void;
auto updateOscParametersToWEB() -> void;
auto getOscRunState() -> bool;
auto isAutoScale() -> bool;
auto updateOscSignal() -> void;
auto updateOscParams(bool force) -> void;
auto getOSCTimeScale() -> float;
auto getNeedUpdateSigGen() -> bool;
auto updateTriggerLimit(bool force) -> void;