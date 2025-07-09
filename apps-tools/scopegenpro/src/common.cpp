#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

auto getADCChannels() -> uint8_t {
    uint8_t c = 0;
    if (rp_HPGetFastADCChannelsCount(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get fast ADC channels count");
    }
    if (c > MAX_ADC_CHANNELS) {
        ERROR_LOG("The number of channels is more than allowed");
        exit(-1);
    }
    return c;
}

auto getDACChannels() -> uint8_t {
    uint8_t c = 0;

    if (rp_HPGetFastDACChannelsCount(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC channels count");
    }

    if (c > MAX_DAC_CHANNELS) {
        ERROR_LOG("The number of channels is more than allowed");
        exit(-1);
    }
    return c;
}

auto getDACRate() -> uint32_t {
    uint32_t c = 0;
    if (rp_HPGetBaseFastDACSpeedHz(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC channels count");
    }
    return c;
}

auto getADCRate() -> uint32_t {
    uint32_t c = 0;
    if (rp_HPGetBaseFastADCSpeedHz(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get fast ADC channels count");
    }
    return c;
}

auto getModel() -> rp_HPeModels_t {
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get board model");
    }
    return c;
}

auto isZModePresent() -> bool {
    return rp_HPGetIsDAC50OhmModeOrDefault();
}

auto outAmpDef() -> float {
    auto model = getModel();
    switch (model) {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
        case STEM_125_14_v2_0:
        case STEM_125_14_Pro_v2_0:
        case STEM_125_14_Z7020_Pro_v1_0:
        case STEM_125_14_Z7020_Pro_v2_0:
        case STEM_125_14_Z7020_Ind_v2_0:
        case STEM_125_14_Z7020_LL_v1_1:
        case STEM_125_14_Z7020_LL_v1_2:
        case STEM_65_16_Z7020_LL_v1_1:
        case STEM_65_16_Z7020_TI_v1_3:
        case STEM_125_14_Z7020_TI_v1_3:
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
        case STEM_250_12_v1_2b:
        case STEM_250_12_120:
            return 0.9;
        default: {
            ERROR_LOG("Unknown model: %d.", model);
            return 0;
        }
    }
}

auto outAmpMax() -> float {
    auto model = getModel();
    switch (model) {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
            return 1;
        case STEM_125_14_v2_0:
        case STEM_125_14_Pro_v2_0:
        case STEM_125_14_Z7020_Pro_v1_0:
        case STEM_125_14_Z7020_Pro_v2_0:
        case STEM_125_14_Z7020_Ind_v2_0:
            return 2;
        case STEM_125_14_Z7020_LL_v1_1:
        case STEM_125_14_Z7020_LL_v1_2:
        case STEM_65_16_Z7020_LL_v1_1:
        case STEM_65_16_Z7020_TI_v1_3:
        case STEM_125_14_Z7020_TI_v1_3:
            return 2;
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
        case STEM_250_12_v1_2b:
        case STEM_250_12_120:
            return 10.0;
        default: {
            ERROR_LOG("Unknown model: %d.", model);
            return 0;
        }
    }
}

auto getModelName() -> std::string {
    auto model = getModel();
    switch (model) {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
        case STEM_125_14_v2_0:
        case STEM_125_14_Pro_v2_0:
            return "Z10";
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
        case STEM_125_14_Z7020_Pro_v1_0:
        case STEM_125_14_Z7020_Pro_v2_0:
        case STEM_125_14_Z7020_Ind_v2_0:
        case STEM_125_14_Z7020_LL_v1_1:
        case STEM_125_14_Z7020_LL_v1_2:
        case STEM_125_14_Z7020_TI_v1_3:
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
        case STEM_65_16_Z7020_LL_v1_1:
        case STEM_65_16_Z7020_TI_v1_3:
            return "Z20_65";
        default: {
            ERROR_LOG("Unknown model: %d.", model);
            return "";
        }
    }
    return "";
}

auto outFreqMin() -> int {
    auto model = getModel();
    switch (model) {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
        case STEM_125_14_v2_0:
        case STEM_125_14_Pro_v2_0:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
        case STEM_125_14_Z7020_Pro_v1_0:
        case STEM_125_14_Z7020_Pro_v2_0:
        case STEM_125_14_Z7020_Ind_v2_0:
        case STEM_125_14_Z7020_LL_v1_1:
        case STEM_125_14_Z7020_LL_v1_2:
        case STEM_125_14_Z7020_TI_v1_3:
            return 1;
        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return 300e3;
        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            return 1;
        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
        case STEM_250_12_120:
            return 1;
        case STEM_65_16_Z7020_LL_v1_1:
        case STEM_65_16_Z7020_TI_v1_3:
            return 1;
        default: {
            ERROR_LOG("Unknown model: %d.", model);
            return 1;
        }
    }
    return 1;
}

auto outFreqMax() -> int {
    auto model = getModel();
    switch (model) {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
        case STEM_125_14_v2_0:
        case STEM_125_14_Pro_v2_0:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
        case STEM_125_14_Z7020_Pro_v1_0:
        case STEM_125_14_Z7020_Pro_v2_0:
        case STEM_125_14_Z7020_Ind_v2_0:
        case STEM_125_14_Z7020_LL_v1_1:
        case STEM_125_14_Z7020_LL_v1_2:
        case STEM_125_14_Z7020_TI_v1_3:
            return 50e6;
        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return 122.880e6 / 2;
        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            return 1;
        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
            return 250e6 / 2;
        case STEM_250_12_120:
            return 120e6 / 2;
        case STEM_65_16_Z7020_LL_v1_1:
        case STEM_65_16_Z7020_TI_v1_3:
            return 65e6 / 2;
        default: {
            ERROR_LOG("Unknown model: %d.", model);
            return 1;
        }
    }
    return 1;
}

auto getMeasureValue(int measure) -> float {
    int mode = measure / 10;
    int channel = measure % 10;
    float value;
    switch (mode) {
        case 0:
            rpApp_OscMeasureVpp((rpApp_osc_source)channel, &value);
            value = fabs(value);
            break;
        case 1:
            rpApp_OscMeasureMeanVoltage((rpApp_osc_source)channel, &value);
            break;
        case 2:
            rpApp_OscMeasureAmplitudeMax((rpApp_osc_source)channel, &value);
            break;
        case 3:
            rpApp_OscMeasureAmplitudeMin((rpApp_osc_source)channel, &value);
            break;
        case 4:
            rpApp_OscMeasureRootMeanSquare((rpApp_osc_source)channel, &value);
            break;
        case 5:
            rpApp_OscMeasureDutyCycle((rpApp_osc_source)channel, &value);
            value *= 100;
            break;
        case 6:
            rpApp_OscMeasurePeriod((rpApp_osc_source)channel, &value);
            break;
        case 7:
            rpApp_OscMeasureFrequency((rpApp_osc_source)channel, &value);
            break;
        default:
            value = 0;
    }
    return value;
}
