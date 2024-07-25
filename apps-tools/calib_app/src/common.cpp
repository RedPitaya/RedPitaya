#include <stdio.h>
#include <stdlib.h>
#include "common.h"

auto getADCChannels() -> uint8_t{
    uint8_t c = 0;
    if (rp_HPGetFastADCChannelsCount(&c) != RP_HP_OK){
        ERROR("Can't get fast ADC channels count");
    }
    if (c > MAX_ADC_CHANNELS){
        FATAL("The number of channels is more than allowed");
        exit(-1);
    }
    return c;
}

auto getDACChannels() -> uint8_t{
    uint8_t c = 0;

    if (rp_HPGetFastDACChannelsCount(&c) != RP_HP_OK){
        ERROR("Can't get fast DAC channels count");
    }

    if (c > MAX_DAC_CHANNELS){
        FATAL("The number of channels is more than allowed");
        exit(-1);
    }
    return c;
}

auto getDACRate() -> uint32_t{
    uint32_t c = 0;
    if (rp_HPGetBaseFastDACSpeedHz(&c) != RP_HP_OK){
        ERROR("Can't get fast DAC channels count");
    }
    return c;
}

auto getADCRate() -> uint32_t{
    uint32_t c = 0;
    if (rp_HPGetBaseFastADCSpeedHz(&c) != RP_HP_OK){
        ERROR("Can't get fast ADC channels count");
    }
    return c;
}

 auto getModel() -> rp_HPeModels_t{
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK){
        ERROR("Can't get board model");
    }
    return c;
}

auto getDACDevider() -> double{
    if (getDACRate() > 125e6)
        return 4;
    return 2;
}

auto getModelName() -> std::string{
    auto model = getModel();
    switch (model)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
            return "Z10";
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
            return "Z20_125";
        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return "Z20";
        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            return "Z20_125_4CH";
        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
            return "Z20_250_12";
        case STEM_250_12_120:
            return "Z20_250_12_120";
        default:{
            ERROR("Unknown model: %d.",model);
            return "";
        }
    }
    return "";
}

auto getADCSamplePeriod(double *value) -> int{
    *value = 0;
    uint32_t speed = 0;
    int ret = rp_HPGetBaseFastADCSpeedHz(&speed);
    if (ret == RP_HP_OK){
        *value = (double)1e9/speed;
    }else{
        ERROR("Can't get FAST ADC Rate");
    }
    return ret;
}