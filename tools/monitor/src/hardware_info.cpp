#include "hardware_info.h"

const std::string& HardwareInfo::getDDRVoltage() {
    static const std::string ddr = queryDDRVoltage();
    return ddr;
}

bool HardwareInfo::is250_12_series() {
    static const bool result = queryIs250_12();
    return result;
}

std::string HardwareInfo::queryDDRVoltage() {
    return is250_12_series() ? "35" : "5";
}

bool HardwareInfo::queryIs250_12() {
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
            return true;
        default:
            return false;
    }
}