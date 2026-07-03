#include "ams_readers.h"
#include "hardware_info.h"
#include "rp.h"

namespace {

bool readTemp(float& val, uint32_t& raw) {
    val = rp_GetCPUTemperature(&raw);
    return true;  // Always succeeds
}

bool readAI0(float& val, uint32_t& raw) {
    return rp_ApinGetValue(RP_AIN0, &val, &raw) == RP_OK;
}

bool readAI1(float& val, uint32_t& raw) {
    return rp_ApinGetValue(RP_AIN1, &val, &raw) == RP_OK;
}

bool readAI2(float& val, uint32_t& raw) {
    return rp_ApinGetValue(RP_AIN2, &val, &raw) == RP_OK;
}

bool readAI3(float& val, uint32_t& raw) {
    return rp_ApinGetValue(RP_AIN3, &val, &raw) == RP_OK;
}

bool readAI4(float& val, uint32_t& raw) {
    return rp_GetPowerI4(&raw, &val) == RP_OK;
}

bool readVCCPINT(float& val, uint32_t& raw) {
    return rp_GetPowerVCCPINT(&raw, &val) == RP_OK;
}

bool readVCCPAUX(float& val, uint32_t& raw) {
    return rp_GetPowerVCCPAUX(&raw, &val) == RP_OK;
}

bool readVCCBRAM(float& val, uint32_t& raw) {
    return rp_GetPowerVCCBRAM(&raw, &val) == RP_OK;
}

bool readVCCINT(float& val, uint32_t& raw) {
    return rp_GetPowerVCCINT(&raw, &val) == RP_OK;
}

bool readVCCAUX(float& val, uint32_t& raw) {
    return rp_GetPowerVCCAUX(&raw, &val) == RP_OK;
}

bool readVCCDDR(float& val, uint32_t& raw) {
    return rp_GetPowerVCCDDR(&raw, &val) == RP_OK;
}

bool readAO0(float& val, uint32_t& raw) {
    return rp_ApinGetValue(RP_AOUT0, &val, &raw) == RP_OK;
}

bool readAO1(float& val, uint32_t& raw) {
    return rp_ApinGetValue(RP_AOUT1, &val, &raw) == RP_OK;
}

bool readAO2(float& val, uint32_t& raw) {
    return rp_ApinGetValue(RP_AOUT2, &val, &raw) == RP_OK;
}

bool readAO3(float& val, uint32_t& raw) {
    return rp_ApinGetValue(RP_AOUT3, &val, &raw) == RP_OK;
}
}  // namespace

const std::array<ReadFunc, static_cast<size_t>(AmsChannel::Count)> readFunctions =
    {readTemp, readAI0, readAI1, readAI2, readAI3, readAI4, readVCCPINT, readVCCPAUX, readVCCBRAM, readVCCINT, readVCCAUX, readVCCDDR, readAO0, readAO1, readAO2, readAO3};

std::array<std::string, static_cast<size_t>(AmsChannel::Count)> buildDescriptions() {
    const std::string& ddr = HardwareInfo::getDDRVoltage();

    return {"Temp(0C-85C)",
            "AI0(0-3.5V)",
            "AI1(0-3.5V)",
            "AI2(0-3.5V)",
            "AI3(0-3.5V)",
            "AI4(5V0)",
            "VCCPINT(1V0)",
            "VCCPAUX(1V8)",
            "VCCBRAM(1V0)",
            "VCCINT(1V0)",
            "VCCAUX(1V8)",
            "VCCDDR(1V" + ddr + ")",
            "AO0(0-1.8V)",
            "AO1(0-1.8V)",
            "AO2(0-1.8V)",
            "AO3(0-1.8V)"};
}