#ifndef CONFIG_NET_LIB_CONFIG_APPLY_H
#define CONFIG_NET_LIB_CONFIG_APPLY_H

#include <string>
#include "settings_lib/stream_settings.h"

// Applies a settings document received over the network on top of _settings. parseJson skips a
// value its setter refuses, so every key the document carries is checked to have taken the
// given value; if one did not, nothing is applied. Unknown sections and keys are ignored.
auto applyConfigJson(const std::string& _json, CStreamSettings& _settings) -> bool;

#endif
