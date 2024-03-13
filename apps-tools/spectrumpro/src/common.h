#ifndef COMMON_H
#define COMMON_H

#include <string>
#include "rp.h"
#include "rp_hw-profiles.h"
#include "common/rp_arb.h"

#define MAX_ADC_CHANNELS 4
#define MAX_DAC_CHANNELS 2

auto getADCChannels() -> uint8_t;
auto getDACChannels() -> uint8_t;
auto getDACRate() -> uint32_t;
auto getADCRate() -> uint32_t;
auto getMaxFreqRate() -> float;

auto getModel() -> rp_HPeModels_t;
auto isZModePresent() -> bool;
auto getModelName() -> std::string;

auto outAmpDef() -> float;
auto outAmpMax() -> float;
auto loadARBList() -> std::string;

#endif
