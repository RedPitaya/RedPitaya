#include "rp_hw-profiles.h"
#include "common.h"
#include "rp.h"

auto getMAXFreq() -> uint32_t{
    uint32_t value = 125e6;
    if (rp_HPGetBaseSpeedHz(&value) != RP_HP_OK){
       ERROR_LOG("Can't get base speed");
    }
    return value;
}
