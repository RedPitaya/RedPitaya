#pragma once

#include <math.h>
#include <limits>
#include <DataManager.h>
#include <CustomParameters.h>
#include <fstream>
#include <mutex>

extern "C" {
    #include "rpApp.h"
    #include "version.h"
}

/* Parameters description structure - must be the same for all RP controllers */
typedef struct rp_app_params_s {
    char  *name;
    float  value;
    int    fpga_update;
    int    read_only;
    float  min_val;
    float  max_val;
} rp_app_params_t;
