#pragma once

#include "rp.h"

auto generateOutSignalForWeb(float tscale) -> void;
auto generate(rp_channel_t channel,float tscale) -> void;

auto checkBurstDelayChanged(rp_channel_t ch) -> void;

auto deleteSweepController() -> void;
auto resumeSweepController(bool pause) -> void;

auto updateGenTempProtection() -> void;
auto sendFreqInSweepMode() -> void;
auto initGenAfterLoad() -> void;
auto updateGeneratorParameters(bool force) -> void;
auto setNeedUpdateGenSignal() -> void;
auto loadARBList() -> std::string;