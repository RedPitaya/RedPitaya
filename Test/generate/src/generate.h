#ifndef __GENERATOR_H
#define __GENERATOR_H


#include <stdint.h>
#include <string>
#include <vector>
#include "common/version.h"

#include "rp.h"
#include "rp_hw-calib.h"
#include "rp_hw-profiles.h"


typedef enum {
    RP_125_14,
    RP_250_12,
    RP_125_14_4CH
} models_t;

typedef struct {
    uint32_t   ch;
    double     amp;
    double     freq;
    double     end_freq;
    rp_waveform_t type;
    rp_gen_gain_t gain;
    rp_gen_load_mode_t load = RP_GEN_HI_Z;
    bool       calib;
    std::string arb = "";
    bool regDebug = false;
} config_t;

uint8_t getChannels();
models_t getModel();
uint32_t getMaxSpeed();
float fullScale();
auto loadARBList() -> void;
auto getARBList() -> std::vector<std::string>;
int gen(config_t &conf);

#endif