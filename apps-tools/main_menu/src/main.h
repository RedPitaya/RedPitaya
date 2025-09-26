#pragma once

#include "rp_hw-profiles.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Parameters description structure - must be the same for all RP controllers */
typedef struct rp_app_params_s {
    char* name;
    float value;
    int fpga_update;
    int read_only;
    float min_val;
    float max_val;
} rp_app_params_t;

const char* rp_app_desc(void);
int rp_app_init(void);
int rp_app_exit(void);
int rp_set_params(rp_app_params_t* p, int len);
int rp_get_params(rp_app_params_t** p);
int rp_get_signals(float*** s, int* sig_num, int* sig_len);

auto getModel() -> rp_HPeModels_t;
auto getHomeDirectory() -> std::string;
auto isDirectory(const std::string& _path) -> bool;
auto createDirectory(const std::string& _path) -> bool;

#ifdef __cplusplus
}
#endif
