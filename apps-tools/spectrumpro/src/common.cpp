#include <stdio.h>
#include <stdlib.h>
#include "common.h"

auto getADCChannels() -> uint8_t{
    uint8_t c = 0;
    if (rp_HPGetFastADCChannelsCount(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast ADC channels count\n");
    }
    if (c > MAX_ADC_CHANNELS){
        fprintf(stderr,"[Error] The number of channels is more than allowed\n");
        exit(-1);
    }
    return c;
}

auto getDACChannels() -> uint8_t{
    uint8_t c = 0;

    if (rp_HPGetFastDACChannelsCount(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast DAC channels count\n");
    }

    if (c > MAX_DAC_CHANNELS){
        fprintf(stderr,"[Error] The number of channels is more than allowed\n");
        exit(-1);
    }
    return c;
}

auto getDACRate() -> uint32_t{
    uint32_t c = 0;
    if (rp_HPGetBaseFastDACSpeedHz(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast DAC channels count\n");
    }
    return c;
}

auto getADCRate() -> uint32_t{
    uint32_t c = 0;
    if (rp_HPGetBaseFastADCSpeedHz(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast ADC channels count\n");
    }
    return c;
}

 auto getModel() -> rp_HPeModels_t{
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get board model\n");
    }
    return c;
}

auto getMaxFreqRate() -> float{
    uint32_t c = 0;
    if (rp_HPGetSpectrumFastADCSpeedHz(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast ADC spectrum resolution\n");
    }
    return c;
}

auto loadARBList() -> std::string{
    uint32_t c = 0;
    rp_ARBInit();
    std::string list;
    if (!rp_ARBGetCount(&c)){
        for(uint32_t i = 0; i < c; i++){
            std::string name;
            if (!rp_ARBGetName(i,&name)){
                list += "A" + name + "\n";
            }
        }
    }
    return list;
}

auto isZModePresent() -> bool{
    auto model = getModel();
    switch (model)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            return false;
        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_120:
            return true;
        default:{
            fprintf(stderr,"[Error:isZModePresent] Unknown model: %d.\n",model);
            return false;
        }
    }
}

auto outAmpDef() -> float{
    auto model = getModel();
    switch (model)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
            return 0.9;
        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return 0.4;
        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            return 0.9;
        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_120:
            return 0.9;
        default:{
            fprintf(stderr,"[Error:outAmpDef] Unknown model: %d.\n",model);
            return 0;
        }
    }
}

auto outAmpMax() -> float{
    auto model = getModel();
    switch (model)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
            return 1;
        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return 0.5;
        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            return 1;
        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_120:
            return 5.0;
        default:{
            fprintf(stderr,"[Error:outAmpMax] Unknown model: %d.\n",model);
            return 0;
        }
    }
}



auto getModelName() -> std::string{
    auto model = getModel();
    switch (model)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
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
            return "Z20_250_12";
        case STEM_250_12_120:
            return "Z20_250_120";
        default:{
            fprintf(stderr,"[Error:getModelName] Unknown model: %d.\n",model);
            return "";
        }
    }
    return "";
}
