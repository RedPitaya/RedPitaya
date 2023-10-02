#pragma once

#include "rp.h"

auto initMathAfterLoad() -> void;
auto updateMathParametersToWEB(bool is_auto_scale = false) -> void;

auto resetMathParams() -> void;
auto setMathParams() -> void;
auto checkMathScale() -> void;

auto updateMathSignal() -> void;
auto updateMathParams(bool force) -> void;