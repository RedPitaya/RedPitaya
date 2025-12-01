#ifndef COMMON_H
#define COMMON_H

#include <string>
#include "rp.h"
#include "rp_hw-profiles.h"

auto getADCChannels() -> uint8_t;
auto getDACChannels() -> uint8_t;
auto getDACRate() -> uint32_t;
auto getADCRate() -> uint32_t;
auto getModel() -> rp_HPeModels_t;
auto getDACDevider() -> double;
auto getModelName() -> std::string;
auto getADCSamplePeriod(double* value) -> int;

#endif
