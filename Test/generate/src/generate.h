#ifndef __GENERATOR_H
#define __GENERATOR_H

#include <stdint.h>
#include <string>
#include <vector>
#include "common/version.h"

#include "rp.h"
#include "rp_hw-profiles.h"
#include "rp_hw_calib.h"

typedef enum { RP_125_14, RP_250_12, RP_125_14_4CH } models_t;

typedef struct {
    uint32_t ch = 0;
    double amp = 0;
    double freq = 0;
    double end_freq = 0;
    rp_waveform_t type = RP_WAVEFORM_SINE;
    rp_gen_gain_t gain = RP_GAIN_1X;
    rp_gen_load_mode_t load = RP_GEN_HI_Z;
    bool calib = true;
    std::string arb = "";
    bool regDebug = false;
} config_t;

uint8_t getChannels();
uint32_t getMaxSpeed();

bool getIsLoad50Ohm();
bool getIsGainX5();

float fullScale();
auto loadARBList() -> void;
auto getARBList() -> std::vector<std::string>;
int gen(config_t& conf);

#endif