#include <stdio.h>
#include <string>
#include "rp.h"
#include "rp_hw-profiles.h"
#include "rp_hw.h"

void set_DAC(float* values, int count) {
    rp_InitReset(false);
    for (int i = 0; i < count; ++i) {
        rp_AOpinSetValue(i, values[i]);
    }
    rp_Release();
}

std::string getDDRValue() {
    rp_HPeModels_t model;
    if (rp_HPGetModel(&model) == RP_HP_OK) {
        switch (model) {
            case STEM_250_12_v1_0:
            case STEM_250_12_v1_1:
            case STEM_250_12_v1_2:
            case STEM_250_12_120:
            case STEM_250_12_v1_2a:
            case STEM_250_12_v1_2b:
            case STEM_125_14_Z7020_Pro_v1_0:
            case STEM_125_14_Z7020_Ind_v2_0:
            case STEM_125_14_Z7020_Pro_v2_0:
                return "35";
            default:
                return "5";
        }
    }
    return "";
}

typedef enum {
    eAmsTemp = 0,
    eAmsAI0,
    eAmsAI1,
    eAmsAI2,
    eAmsAI3,
    eAmsAI4,
    eAmsVCCPINT,
    eAmsVCCPAUX,
    eAmsVCCBRAM,
    eAmsVCCINT,
    eAmsVCCAUX,
    eAmsVCCDDR,
    eAmsAO0,
    eAmsAO1,
    eAmsAO2,
    eAmsAO3,
    eSendNum
} ams_t;

constexpr char amsDesc[eSendNum][20] = {
    "Temp(0C-85C)", "AI0(0-3.5V)", "AI1(0-3.5V)", "AI2(0-3.5V)",  "AI3(0-3.5V)", "AI4(5V0)",    "VCCPINT(1V0)", "VCCPAUX(1V8)",
    "VCCBRAM(1V0)", "VCCINT(1V0)", "VCCAUX(1V8)", "VCCDDR(1V%s)", "AO0(0-1.8V)", "AO1(0-1.8V)", "AO2(0-1.8V)",  "AO3(0-1.8V)",
};

static void AmsList() {
    char desc[255];
    uint32_t i, raw;
    float val;
    printf("#ID\tDesc\t\tRaw\tVal\n");
    for (i = 0; i < eSendNum; i++) {
        switch (i) {
            case eAmsTemp:
                val = rp_GetCPUTemperature(&raw);
                strcpy(desc, amsDesc[i]);
                break;
            case eAmsAI0:
                rp_ApinGetValue(RP_AIN0, &val, &raw);
                strcpy(desc, amsDesc[i]);
                break;
            case eAmsAI1:
                rp_ApinGetValue(RP_AIN1, &val, &raw);
                strcpy(desc, amsDesc[i]);
                break;
            case eAmsAI2:
                rp_ApinGetValue(RP_AIN2, &val, &raw);
                strcpy(desc, amsDesc[i]);
                break;
            case eAmsAI3:
                rp_ApinGetValue(RP_AIN3, &val, &raw);
                strcpy(desc, amsDesc[i]);
                break;
            case eAmsAI4:
                rp_GetPowerI4(&raw, &val);
                strcpy(desc, amsDesc[i]);
                break;
            case eAmsVCCPINT:
                rp_GetPowerVCCPINT(&raw, &val);
                strcpy(desc, amsDesc[i]);
                break;
            case eAmsVCCPAUX:
                rp_GetPowerVCCPAUX(&raw, &val);
                strcpy(desc, amsDesc[i]);
                break;
            case eAmsVCCBRAM:
                rp_GetPowerVCCBRAM(&raw, &val);
                strcpy(desc, amsDesc[i]);
                break;
            case eAmsVCCINT:
                rp_GetPowerVCCINT(&raw, &val);
                strcpy(desc, amsDesc[i]);
                break;
            case eAmsVCCAUX:
                rp_GetPowerVCCAUX(&raw, &val);
                strcpy(desc, amsDesc[i]);
                break;
            case eAmsVCCDDR:
                rp_GetPowerVCCDDR(&raw, &val);
                sprintf(desc, amsDesc[i], getDDRValue().c_str());
                break;
            case eAmsAO0:
                rp_ApinGetValue(RP_AOUT0, &val, &raw);
                strcpy(desc, amsDesc[i]);
                break;
            case eAmsAO1:
                rp_ApinGetValue(RP_AOUT1, &val, &raw);
                strcpy(desc, amsDesc[i]);
                break;
            case eAmsAO2:
                rp_ApinGetValue(RP_AOUT2, &val, &raw);
                strcpy(desc, amsDesc[i]);
                break;
            case eAmsAO3:
                rp_ApinGetValue(RP_AOUT3, &val, &raw);
                strcpy(desc, amsDesc[i]);
                break;
            case eSendNum:
                break;
        }
        printf("%d\t%s\t0x%08x\t%.3f\n", i, desc, raw, val);
    }
}

void showAMS() {
    rp_InitReset(false);
    AmsList();
    rp_Release();
}