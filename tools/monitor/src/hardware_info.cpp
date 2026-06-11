#include "hardware_info.h"

const std::string& HardwareInfo::getDDRVoltage() {
    static const std::string ddr = queryDDRVoltage();
    return ddr;
}

bool HardwareInfo::isLowVRAM_series() {
    static const bool result = queryIsLowVRAM();
    return result;
}

std::string HardwareInfo::queryDDRVoltage() {
    return isLowVRAM_series() ? "35" : "5";
}

bool HardwareInfo::queryIsLowVRAM() {
    rp_HPeModels_t model;
    if (rp_HPGetModel(&model) != RP_HP_OK)
        return false;

    switch (model) {
        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_120:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
        case STEM_125_14_BO_v2_0:
        case STEM_125_14_Pro_BO_v2_0:
        case STEM_125_14_Z7020_Pro_BO_v2_0:
        case STEM_125_14_Z7020_Pro_v1_0:
        case STEM_125_14_Z7020_Ind_v2_0:
        case STEM_125_14_Z7020_Pro_v2_0:
        case STEM_125_14_Z7020_LL_v1_1:
        case STEM_65_16_Z7020_LL_v1_1:
        case STEM_125_14_Z7020_LL_v1_2:
        case STEM_125_14_Z7020_TI_v1_3:
        case STEM_65_16_Z7020_TI_v1_3:
        case STEM_125_14_Pro_v2_0:
        case STEM_125_14_v2_0:
            return true;
        default:
            return false;
    }
}