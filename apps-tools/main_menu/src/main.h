#pragma once

#include "rp_hw-profiles.h"

#ifdef __cplusplus
extern "C" {
#endif

const char* rp_app_desc(void);
int rp_app_init(void);
int rp_app_exit(void);

auto getModel() -> rp_HPeModels_t;
auto getHomeDirectory() -> std::string;
auto isDirectory(const std::string& _path) -> bool;
auto createDirectory(const std::string& _path) -> bool;

#ifdef __cplusplus
}
#endif
