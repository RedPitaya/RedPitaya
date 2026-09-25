#pragma once

#include "rp.h"

auto initXYAfterLoad() -> void;

auto updateXYParametersToWEB() -> void;

auto resetXYParams() -> void;
auto setXYParams() -> void;

auto updateXYSignal() -> void;
auto updateXYParams(bool force) -> void;

auto isXYShow() -> bool;