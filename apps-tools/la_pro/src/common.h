#pragma once

#include <stdint.h>
#include <map>
#include <string>
#include "rp_hw-profiles.h"

enum LA_MODE { LA_APP_BASIC_ONLY, LA_APP_PRO_ONLY };

enum MEASURE_MODE { LA_APP_NONE, LA_APP_BASIC, LA_APP_PRO };

enum MEASURE_STATE { LA_APP_STOP = 1, LA_APP_RUNNED = 2, LA_APP_DONE = 3, LA_APP_TIMEOUT = 4 };

constexpr float DEF_MIN_SCALE = 1.f / 1000.f;
constexpr float DEF_MAX_SCALE = 5.f;

auto getModel() -> rp_HPeModels_t;
auto getMAXFreq() -> uint32_t;

auto getNameFromConfig(std::string& json) -> std::string;
auto getParamFromConfig(std::string& json) -> std::string;
auto getConfig(std::string& name, std::string& param_json) -> std::string;
auto annoToJSON(std::map<uint8_t, std::string> map) -> std::string;

#define DEBUG_SIGNAL_PERIOD 50
#define DEBUG_PARAM_PERIOD 50
#define BUFFER_MAX_SIZE 1024 * 1024 * 2
#define MAX_SAMPLES 750000.0
