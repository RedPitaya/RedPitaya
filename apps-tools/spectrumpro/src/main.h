#pragma once

#include <CustomParameters.h>
#include <DataManager.h>
#include <math.h>
#include <fstream>
#include <limits>
#include <mutex>

#include "common/version.h"
#include "rpApp.h"

/* Parameters description structure - must be the same for all RP controllers */
typedef struct rp_app_params_s {
    char* name;
    float value;
    int fpga_update;
    int read_only;
    float min_val;
    float max_val;
} rp_app_params_t;
