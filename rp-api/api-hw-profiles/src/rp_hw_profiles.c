#include "stdio.h"
#include "common.h"
#include "stem_125_10_v1.0.h"
#include "stem_125_14_v1.0.h"
#include "stem_125_14_v1.1.h"
#include "stem_122_16SDR_v1.0.h"
#include "stem_122_16SDR_v1.1.h"
#include "stem_125_14_LN_v1.1.h"
#include "stem_125_14_Z7020_v1.0.h"
#include "stem_125_14_Z7020_LN_v1.1.h"
#include "stem_125_14_Z7020_4IN_v1.0.h"
#include "stem_125_14_Z7020_4IN_v1.2.h"
#include "stem_125_14_Z7020_4IN_v1.3.h"
#include "stem_250_12_v1.0.h"
#include "stem_250_12_v1.1.h"
#include "stem_250_12_v1.2.h"
#include "stem_250_12_v1.2a.h"
#include "stem_250_12_v1.2b.h"
#include "stem_250_12_120.h"

profiles_t* getProfile(int *state){
    profiles_t *p = hp_cmn_GetLoadedProfile();
    if (!p){
        *state = hp_cmn_Init();
        if (*state != RP_HP_OK){
            return NULL;
        }
        p = hp_cmn_GetLoadedProfile();
    }
    *state = RP_HP_OK;
    return p;
}

profiles_t* getProfileDefualt(){
    profiles_t *p = hp_cmn_GetLoadedProfile();
    if (!p){
        hp_cmn_Init();
        p = hp_cmn_GetLoadedProfile();
    }
    if (!p){
        p = getProfile_STEM_125_10_v1_0();
    }
    return p;
}

int rp_HPPrint(){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        hp_cmn_Print(p);
        return RP_HP_OK;
    }
    return state;
}

int rp_HPPrintAll(){
    hp_cmn_Print(getProfile_STEM_125_10_v1_0());
    hp_cmn_Print(getProfile_STEM_125_14_v1_0());
    hp_cmn_Print(getProfile_STEM_125_14_v1_1());
    hp_cmn_Print(getProfile_STEM_122_16SDR_v1_0());
    hp_cmn_Print(getProfile_STEM_122_16SDR_v1_1());
    hp_cmn_Print(getProfile_STEM_125_14_LN_v1_1());
    hp_cmn_Print(getProfile_STEM_125_14_Z7020_v1_0());
    hp_cmn_Print(getProfile_STEM_125_14_Z7020_LN_v1_1());
    hp_cmn_Print(getProfile_STEM_125_14_Z7020_4IN_v1_0());
    hp_cmn_Print(getProfile_STEM_125_14_Z7020_4IN_v1_2());
    hp_cmn_Print(getProfile_STEM_125_14_Z7020_4IN_v1_3());
    hp_cmn_Print(getProfile_STEM_250_12_v1_0());
    hp_cmn_Print(getProfile_STEM_250_12_v1_1());
    hp_cmn_Print(getProfile_STEM_250_12_v1_2());
    hp_cmn_Print(getProfile_STEM_250_12_v1_2a());
    hp_cmn_Print(getProfile_STEM_250_12_v1_2b());
    hp_cmn_Print(getProfile_STEM_250_12_120());
    return RP_HP_OK;
}

int rp_HPGetModel(rp_HPeModels_t *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->boardModel;
        return RP_HP_OK;
    }
    return state;
}

int rp_HPGetModelName(char **value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = (char*)&p->boardName;
        return RP_HP_OK;
    }
    return state;
}

int rp_HPGetModelEEPROM(char **value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = (char*)&p->boardModelEEPROM;
        return RP_HP_OK;
    }
    return state;
}

int rp_HPGetModelETH_MAC_Address(char **value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = (char*)&p->boardETH_MAC;
        return RP_HP_OK;
    }
    return state;
}

int rp_HPGetZynqModel(rp_HPeZynqModels_t *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->zynqCPUModel;
        return RP_HP_OK;
    }
    return state;
}

rp_HPeZynqModels_t rp_HPGetZynqModelOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->zynqCPUModel;
}

int rp_HPGetBaseSpeedHz(uint32_t *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->oscillator_rate;
        return RP_HP_OK;
    }
    return state;
}

uint32_t rp_HPGetBaseSpeedHzOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->oscillator_rate;
}

int rp_HPGetBaseFastADCSpeedHz(uint32_t *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->fast_adc_rate;
        return RP_HP_OK;
    }
    return state;
}

float rp_HPGetHWADCFullScaleOrDefault(){
     profiles_t* p = getProfileDefualt();
    return p->fast_adc_full_scale;
}

int rp_HPGetHWADCFullScale(float *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->fast_adc_full_scale;
        return RP_HP_OK;
    }
    return state;
}

float rp_HPGetHWDACFullScaleOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->fast_dac_full_scale;
}

int rp_HPGetHWDACFullScale(float *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->fast_dac_full_scale;
        return RP_HP_OK;
    }
    return state;
}

uint32_t rp_HPGetBaseFastADCSpeedHzOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->fast_adc_rate;
}

int rp_HPGetSpectrumFastADCSpeedHz(uint32_t *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->fast_adc_spectrum_resolution;
        return RP_HP_OK;
    }
    return state;
}

uint32_t rp_HPGetSpectrumFastADCSpeedHzOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->fast_adc_spectrum_resolution;
}

int rp_HPGetFastADCChannelsCount(uint8_t *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->fast_adc_count_channels;
        return RP_HP_OK;
    }
    return state;
}

uint8_t rp_HPGetFastADCChannelsCountOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->fast_adc_count_channels;
}

int rp_HPGetFastADCIsSigned(bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->fast_adc_is_sign;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetFastADCIsSignedOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->fast_adc_is_sign;
}

int rp_HPGetFastADCBits(uint8_t *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->fast_adc_bits;
        return RP_HP_OK;
    }
    return state;
}

uint8_t rp_HPGetFastADCBitsOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->fast_adc_bits;
}

int rp_HPGetFastADCGain(uint8_t channel,rp_HPADCGainMode_t mode,float *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        if (p->fast_adc_count_channels < channel || channel >= MAX_CHANNELS){
            *value = 0;
            return RP_HP_ECI;
        }
        *value = p->fast_adc_gain[mode][channel];
        return RP_HP_OK;
    }
    return state;
}

float rp_HPGetFastADCGainOrDefault(uint8_t channel,rp_HPADCGainMode_t mode){
    profiles_t* p = getProfileDefualt();
    if (p->fast_adc_count_channels < channel || channel >= MAX_CHANNELS){
        return 0;
    }
    return p->fast_adc_gain[mode][channel];
}

int rp_HPIsFastDAC_Present(bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->is_dac_present;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPIsFastDAC_PresentOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->is_dac_present;
}

int rp_HPGetFastDACIsTempProtection(bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->is_fast_dac_temp_protection;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetFastDACIsTempProtectionOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->is_fast_dac_temp_protection;
}


int rp_HPGetBaseFastDACSpeedHz(uint32_t *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->fast_dac_rate;
        return RP_HP_OK;
    }
    return state;
}

uint32_t rp_HPGetBaseFastDACSpeedHzOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->fast_dac_rate;
}

int rp_HPGetFastDACChannelsCount(uint8_t *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->fast_dac_count_channels;
        return RP_HP_OK;
    }
    return state;
}

uint8_t rp_HPGetFastDACChannelsCountOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->fast_dac_count_channels;
}

int rp_HPGetFastDACIsSigned(bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->fast_dac_is_sign;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetFastDACIsSignedOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->fast_dac_is_sign;
}

int rp_HPGetFastDACBits(uint8_t *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->fast_dac_bits;
        return RP_HP_OK;
    }
    return state;
}

uint8_t rp_HPGetFastDACBitsOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->fast_dac_bits;
}

int rp_HPGetFastDACGain(uint8_t channel,float *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        if (p->fast_dac_count_channels < channel || channel >= MAX_CHANNELS){
            return RP_HP_ECI;
        }
        *value = p->fast_dac_gain[channel];
        return RP_HP_OK;
    }
    return state;
}

float rp_HPGetFastDACGainOrDefault(uint8_t channel){
    profiles_t* p = getProfileDefualt();
    if (p->fast_dac_count_channels < channel || channel >= MAX_CHANNELS){
        return 0;
    }
    return p->fast_dac_gain[channel];
}

int rp_HPGetFastADCIsLV_HV(bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->is_LV_HV_mode;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetFastADCIsLV_HVOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->is_LV_HV_mode;
}

int rp_HPGetFastADCIsAC_DC(bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->is_AC_DC_mode;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetFastADCIsAC_DCOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->is_AC_DC_mode;
}

int rp_HPGetFastADCIsFilterPresent(bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->is_fast_adc_filter_present;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetFastADCIsFilterPresentOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->is_fast_adc_filter_present;
}

int rp_HPGetSlowADCChannelsCount(uint8_t *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->slow_adc_count_channels;
        return RP_HP_OK;
    }
    return state;
}

uint8_t rp_HPGetSlowADCChannelsCountOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->slow_adc_count_channels;
}

int rp_HPGetSlowADCIsSigned(uint8_t channel,bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        if (p->slow_adc_count_channels < channel || channel > 3){
            return RP_HP_ECI;
        }
        *value = p->slow_adc[channel].is_signed;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetSlowADCIsSignedOrDefault(uint8_t channel){
    profiles_t* p = getProfileDefualt();
    if (p->slow_adc_count_channels < channel || channel > 3){
        return p->slow_adc[0].is_signed;
    }
    return p->slow_adc[channel].is_signed;
}

int rp_HPGetSlowADCBits(uint8_t channel,uint8_t *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        if (p->slow_adc_count_channels < channel || channel > 3){
            return RP_HP_ECI;
        }
        *value = p->slow_adc[channel].bits;
        return RP_HP_OK;
    }
    return state;
}

uint8_t rp_HPGetSlowADCBitsOrDefault(uint8_t channel){
    profiles_t* p = getProfileDefualt();
    if (p->slow_adc_count_channels < channel || channel > 3){
        return p->slow_adc[0].bits;
    }
    return p->slow_adc[channel].bits;
}

int rp_HPGetSlowADCFullScale(uint8_t channel,float *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        if (p->slow_adc_count_channels < channel || channel > 3){
            return RP_HP_ECI;
        }
        *value = p->slow_adc[channel].fullScale;
        return RP_HP_OK;
    }
    return state;
}

float rp_HPGetSlowADCFullScaleOrDefault(uint8_t channel){
    profiles_t* p = getProfileDefualt();
    if (p->slow_adc_count_channels < channel || channel > 3){
        return p->slow_adc[0].fullScale;
    }
    return p->slow_adc[channel].fullScale;
}

int rp_HPGetSlowDACChannelsCount(uint8_t *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->slow_dac_count_channels;
        return RP_HP_OK;
    }
    return state;
}

uint8_t rp_HPGetSlowDACChannelsCountOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->slow_dac_count_channels;
}

int rp_HPGetSlowDACIsSigned(uint8_t channel,bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        if (p->slow_dac_count_channels < channel || channel > 3){
            return RP_HP_ECI;
        }
        *value = p->slow_dac[channel].is_signed;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetSlowDACIsSignedOrDefault(uint8_t channel){
    profiles_t* p = getProfileDefualt();
    if (p->slow_dac_count_channels < channel || channel > 3){
        return p->slow_dac[0].is_signed;
    }
    return p->slow_dac[channel].is_signed;
}

int rp_HPGetSlowDACBits(uint8_t channel,uint8_t *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        if (p->slow_dac_count_channels < channel || channel > 3){
            return RP_HP_ECI;
        }
        *value = p->slow_dac[channel].bits;
        return RP_HP_OK;
    }
    return state;
}

uint8_t rp_HPGetSlowDACBitsOrDefault(uint8_t channel){
    profiles_t* p = getProfileDefualt();
    if (p->slow_dac_count_channels < channel || channel > 3){
        return p->slow_dac[0].bits;
    }
    return p->slow_dac[channel].bits;
}

int rp_HPGetSlowDACFullScale(uint8_t channel,float *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        if (p->slow_dac_count_channels < channel || channel > 3){
            return RP_HP_ECI;
        }
        *value = p->slow_dac[channel].fullScale;
        return RP_HP_OK;
    }
    return state;
}

float rp_HPGetSlowDACFullScaleOrDefault(uint8_t channel){
    profiles_t* p = getProfileDefualt();
    if (p->slow_dac_count_channels < channel || channel > 3){
        return p->slow_dac[0].fullScale;
    }
    return p->slow_dac[channel].fullScale;
}

int rp_HPGetIsGainDACx5(bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->is_DAC_gain_x5;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetIsGainDACx5OrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->is_DAC_gain_x5;
}

int rp_HPGetIsDAC50OhmMode(bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->is_DAC_50_Ohm_mode;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetIsDAC50OhmModeOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->is_DAC_50_Ohm_mode;
}

int rp_HPGetIsCalibrationLogicPresent(bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->is_fast_calibration;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetIsCalibrationLogicPresentOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->is_fast_calibration;
}

int rp_HPGetIsPLLControlEnable(bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->is_pll_control_present;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetIsPLLControlEnableOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->is_pll_control_present;
}

int rp_HPGetIsAttenuatorControllerPresent(bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->is_attenuator_controller_present;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetIsAttenuatorControllerPresentOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->is_attenuator_controller_present;
}

int rp_HPGetIsExternalTriggerLevelPresent(bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->is_ext_trigger_level_available;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetIsExternalTriggerLevelPresentOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->is_ext_trigger_level_available;
}

int rp_HPGetIsExternalTriggerFullScale(float *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->external_trigger_full_scale;
        return RP_HP_OK;
    }
    return state;
}

float rp_HPGetIsExternalTriggerFullScalePresentOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->external_trigger_full_scale;
}

int rp_HPGetIsExternalTriggerIsSigned(bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->is_ext_trigger_signed;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetIsExternalTriggerIsSignedOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->is_ext_trigger_signed;
}

int rp_HPGetIsDaisyChainClockAvailable(bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->is_daisy_chain_clock_sync;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetIsDaisyChainClockAvailableOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->is_daisy_chain_clock_sync;
}

int rp_HPGetIsDMAinv0_94(bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->is_dma_mode_v0_94;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetIsDMAinv0_94OrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->is_dma_mode_v0_94;
}

int rp_HPGetFastADCIsSplitTrigger(bool *value){
    int state;
    profiles_t* p = getProfile(&state);
    if (p){
        *value = p->is_split_osc_triggers;
        return RP_HP_OK;
    }
    return state;
}

bool rp_HPGetFastADCIsSplitTriggerOrDefault(){
    profiles_t* p = getProfileDefualt();
    return p->is_split_osc_triggers;
}