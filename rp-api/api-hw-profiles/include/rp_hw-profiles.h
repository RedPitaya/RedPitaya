/**
 * $Id: $
 *
 * @file rp_hw_profiles.h
 * @brief Red Pitaya library API hardware profiles
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef RP_HW_PROFILES_H
#define RP_HW_PROFILES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/** @name Error codes
 *  Various error codes returned by the API.
 */
///@{

/** Success */
#define RP_HP_OK     0
/** Bad alloc */
#define RP_HP_EAL    1
/** Unknown error */
#define RP_HP_EU     2
/** Error read eeprom */
#define RP_HP_ERE    3
/** Error read model */
#define RP_HP_ERM    4
/** Error model undefined */
#define RP_HP_EMU    5
/** Error channel index */
#define RP_HP_ECI    6

///@}

/**
 * List of board models
 */
typedef enum {
    STEM_125_10_v1_0            = 0,
    STEM_125_14_v1_0            = 1,
    STEM_125_14_v1_1            = 2,
    STEM_122_16SDR_v1_0         = 3,
    STEM_122_16SDR_v1_1         = 4,
    STEM_125_14_LN_v1_1         = 5,
    STEM_125_14_Z7020_v1_0      = 6,
    STEM_125_14_Z7020_LN_v1_1   = 7,
    STEM_125_14_Z7020_4IN_v1_0  = 8,
    STEM_125_14_Z7020_4IN_v1_2  = 9,
    STEM_125_14_Z7020_4IN_v1_3  = 10,
    STEM_250_12_v1_0            = 11,
    STEM_250_12_v1_1            = 12,
    STEM_250_12_v1_2            = 13,
    STEM_250_12_120             = 14,
    STEM_250_12_v1_2a           = 15,
    STEM_250_12_v1_2b           = 16,
    STEM_125_14_LN_BO_v1_1      = 17,
    STEM_125_14_LN_CE1_v1_1     = 18,
    STEM_125_14_LN_CE2_v1_1     = 19,

    STEM_125_14_v2_0            = 20,
    STEM_125_14_Pro_v2_0        = 21,
    STEM_125_14_Z7020_Pro_v2_0  = 22,
    STEM_125_14_Z7020_Ind_v2_0  = 23
}  rp_HPeModels_t;

/**
 * List of CPU models
 */
typedef enum {
    Z7010 = 0,
    Z7020 = 1
}  rp_HPeZynqModels_t;

/**
 * List of gain mode
 */
typedef enum {
    RP_HP_ADC_GAIN_NORMAL  = 0, // LV mode, used by default
    RP_HP_ADC_GAIN_HIGH    = 1
}  rp_HPADCGainMode_t;

/** @name General
 */
///@{

/**
* Returns the model read from eeprom
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetModel(rp_HPeModels_t *_out_value);

/**
* Returns the model name. Return char array with zero terminate value.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetModelName(char **_no_free_value);

/**
* Returns the model for eeprom. Return char array with zero terminate value.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetModelEEPROM(char **_no_free_value);


/**
* Returns the mac address  fro ethernet stored in eeprom. Return char array with zero terminate value.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetModelETH_MAC_Address(char **_no_free_value);

/**
* Returns the zynq model
* Function rp_HPGetZynqModelOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetZynqModel(rp_HPeZynqModels_t *_out_value);
rp_HPeZynqModels_t rp_HPGetZynqModelOrDefault();

/**
* Returns the size of DDR memory on the chip in MB
* Function rp_HPGetDDRSizeOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetDDRSize(uint32_t *_out_value);
uint32_t rp_HPGetDDRSizeOrDefault();

/**
* Returns the clock frequency for FPGA
* Function rp_HPGetBaseSpeedHzOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetBaseSpeedHz(uint32_t *_out_value);
uint32_t rp_HPGetBaseSpeedHzOrDefault();


/**
* Returns Full Scale of ADC chip
* Function rp_HPGetHWADCFullScaleOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetHWADCFullScale(float *_out_value);
float rp_HPGetHWADCFullScaleOrDefault();

/**
* Returns Full Scale of DAC chip
* Function rp_HPGetHWDACFullScaleOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetHWDACFullScale(float *_out_value);
float rp_HPGetHWDACFullScaleOrDefault();

/**
* Returns the clock frequency for ADC
* Function rp_HPGetBaseFastADCSpeedHzOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetBaseFastADCSpeedHz(uint32_t *_out_value);
uint32_t rp_HPGetBaseFastADCSpeedHzOrDefault();

/**
* Returns the maximum value for the spectrum analyzer in hz
* Function rp_HPGetSpectrumFastADCSpeedHzOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetSpectrumFastADCSpeedHz(uint32_t *_out_value);
uint32_t rp_HPGetSpectrumFastADCSpeedHzOrDefault();

/**
* Returns the number of channels for ADC
* Function rp_HPGetFastADCChannelsCountOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetFastADCChannelsCount(uint8_t *_out_value);
uint8_t rp_HPGetFastADCChannelsCountOrDefault();

/**
* Returns whether the ADC has a signed value
* Function rp_HPGetFastADCIsSignedOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetFastADCIsSigned(bool *_out_value);
bool rp_HPGetFastADCIsSignedOrDefault();

/**
* Returns the bit depth for each channel
* Function rp_HPGetFastADCBitsOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetFastADCBits(uint8_t *_out_value);
uint8_t rp_HPGetFastADCBitsOrDefault();

/**
* Returns the gain for each channel
* Function rp_HPGetFastADCGainLV. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetFastADCGain(uint8_t _in_channel,rp_HPADCGainMode_t _in_mode,float *_out_value);
float rp_HPGetFastADCGainOrDefault(uint8_t channel,rp_HPADCGainMode_t mode);

/**
* Returns the generator presence flag
* Function rp_HPIdFastDACOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPIsFastDAC_Present(bool *_out_value);
bool rp_HPIsFastDAC_PresentOrDefault();

/**
* Returns the presence of FAST DAC overheating protection
* Function rp_HPGetFastDACIsTempProtectionOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetFastDACIsTempProtection(bool *_out_value);
bool rp_HPGetFastDACIsTempProtectionOrDefault();

/**
* Returns the clock frequency for DAC
* Function rp_HPGetBaseFastDACSpeedHzOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetBaseFastDACSpeedHz(uint32_t *_out_value);
uint32_t rp_HPGetBaseFastDACSpeedHzOrDefault();

/**
* Returns the number of channels for DAC
* Function rp_HPGetFastDACChannelsCountOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetFastDACChannelsCount(uint8_t *_out_value);
uint8_t rp_HPGetFastDACChannelsCountOrDefault();

/**
* Returns whether the DAC has a signed value
* Function rp_HPGetFastDACIsSignedOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetFastDACIsSigned(bool *_out_value);
bool rp_HPGetFastDACIsSignedOrDefault();

/**
* Returns the bit depth for each channel
* Function rp_HPGetFastDACBitsOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetFastDACBits(uint8_t *_out_value);
uint8_t rp_HPGetFastDACBitsOrDefault();

/**
* Returns the gain for each channel in Volt
* Function rp_HPGetFastDACGainOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetFastDACGain(uint8_t channel,float *_out_value);
float rp_HPGetFastDACGainOrDefault(uint8_t channel);


/**
* Returns AC and DC mode support for ADC inputs
* Function rp_HPGetFastADCIsLV_HVOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetFastADCIsLV_HV(bool *_out_value);
bool rp_HPGetFastADCIsLV_HVOrDefault();

/**
* Returns LV (1:1) and HV (1:20) mode support for ADC inputs
* Function rp_HPGetFastADCIsAC_DCOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetFastADCIsAC_DC(bool *_out_value);
bool rp_HPGetFastADCIsAC_DCOrDefault();

/**
* Checks for the presence of fast ADC filtering functionality
* Function rp_HPGetFastADCIsFilterPresentOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetFastADCIsFilterPresent(bool *_out_value);
bool rp_HPGetFastADCIsFilterPresentOrDefault();


/**
* Returns the number of channels for slow ADC
* Function rp_HPGetSlowADCChannelsCountOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetSlowADCChannelsCount(uint8_t *_out_value);
uint8_t rp_HPGetSlowADCChannelsCountOrDefault();

/**
* Returns whether the slow ADC has a signed value
* Function rp_HPGetSlowADCIsSignedOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetSlowADCIsSigned(uint8_t channel,bool *_out_value);
bool rp_HPGetSlowADCIsSignedOrDefault(uint8_t channel);

/**
* Returns the bit depth for each channel
* Function rp_HPGetSlowADCBitsOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetSlowADCBits(uint8_t channel,uint8_t *_out_value);
uint8_t rp_HPGetSlowADCBitsOrDefault(uint8_t channel);

/**
* Returns the full scale for each channel in Volt
* Function rp_HPGetSlowADCFullScaleOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetSlowADCFullScale(uint8_t channel,float *_out_value);
float rp_HPGetSlowADCFullScaleOrDefault(uint8_t channel);

/**
* Returns the number of channels for slow DAC
* Function rp_HPGetSlowDACChannelsCountOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetSlowDACChannelsCount(uint8_t *_out_value);
uint8_t rp_HPGetSlowDACChannelsCountOrDefault();

/**
* Returns whether the slow DAC has a signed value
* Function rp_HPGetSlowDACIsSignedOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetSlowDACIsSigned(uint8_t channel,bool *_out_value);
bool rp_HPGetSlowDACIsSignedOrDefault(uint8_t channel);

/**
* Returns the bit depth for each channel
* Function rp_HPGetSlowDACBitsOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetSlowDACBits(uint8_t channel,uint8_t *_out_value);
uint8_t rp_HPGetSlowDACBitsOrDefault(uint8_t channel);

/**
* Returns the full scale for each channel in Volt
* Function rp_HPGetSlowDACFullScaleOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetSlowDACFullScale(uint8_t channel,float *_out_value);
float rp_HPGetSlowDACFullScaleOrDefault(uint8_t channel);


/**
* Returns the presence of the generator amplifier at x5
* Function rp_HPGetIsGainDACx5OrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetIsGainDACx5(bool *_out_value);
bool rp_HPGetIsGainDACx5OrDefault();

/**
* Returns the presence of 50 ohm support for the oscillator.
* Function rp_HPGetIsDAC50OhmModeOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetIsDAC50OhmMode(bool *_out_value);
bool rp_HPGetIsDAC50OhmModeOrDefault();

/**
* Returns a sign of the presence of the calibration functionality for the board
* Function rp_HPGetIsCalibrationLogicPresentOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetIsCalibrationLogicPresent(bool *_out_value);
bool rp_HPGetIsCalibrationLogicPresentOrDefault();

/**
* Returns the presence of the PLL functionality. Present in boards 250-12.
* Function rp_HPGetIsPLLControlEnableOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetIsPLLControlEnable(bool *_out_value);
bool rp_HPGetIsPLLControlEnableOrDefault();

/**
* Returns the presence of a hardware attenuator switch. Present in boards 250-12.
* Function rp_HPGetIsAttenuatorControllerPresentOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetIsAttenuatorControllerPresent(bool *_out_value);
bool rp_HPGetIsAttenuatorControllerPresentOrDefault();

/**
* Returns whether it is possible to set the level for an external trigger. Present in boards 250-12.
* Function rp_HPGetIsExternalTriggerLevelPresentOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetIsExternalTriggerLevelPresent(bool *_out_value);
bool rp_HPGetIsExternalTriggerLevelPresentOrDefault();

/**
* Returns the full scale for external trigger
* Function rp_HPGetIsExternalTriggerFullScalePresentOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetIsExternalTriggerFullScale(float *_out_value);
float rp_HPGetIsExternalTriggerFullScalePresentOrDefault();

/**
* Returns whether the external trigger has a signed value
* Function rp_HPGetIsExternalTriggerIsSignedOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetIsExternalTriggerIsSigned(bool *_out_value);
bool rp_HPGetIsExternalTriggerIsSignedOrDefault();

/**
* Returns the availability clock synchronization through the daisy chain.
* Function rp_HPGetIsDaisyChainAvailableOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetIsDaisyChainClockAvailable(bool *_out_value);
bool rp_HPGetIsDaisyChainClockAvailableOrDefault();

/**
* Returns the availability of dma mode support for v0.94.
* Function rp_HPGetIsDMAinv0_94OrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetIsDMAinv0_94(bool *_out_value);
bool rp_HPGetIsDMAinv0_94OrDefault();

/**
* Returns the ability to separate trigger logic for fast ADC for v0.94.
* Function rp_HPGetFastADCIsSplitTriggerOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetFastADCIsSplitTrigger(bool *_out_value);
bool rp_HPGetFastADCIsSplitTriggerOrDefault();


/**
* Returns the number of available GPIO outputs N.
* Function rp_HPGetGPIO_N_CountOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetGPIO_N_Count(uint8_t *_out_value);
uint8_t rp_HPGetGPIO_N_CountOrDefault();


/**
* Returns the number of available GPIO outputs P.
* Function rp_HPGetGPIO_P_CountOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetGPIO_P_Count(uint8_t *_out_value);
uint8_t rp_HPGetGPIO_P_CountOrDefault();


/**
* Returns the presence of the E3 connector.
* Function rp_HPGetIsE3PresentOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetIsE3Present(bool *_out_value);
bool rp_HPGetIsE3PresentOrDefault();

/**
* Returns the presence of the E3 connector, as well as support for the high-speed GPIO connector.
* Function rp_HPGetIsE3HighSpeedGPIOOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetIsE3HighSpeedGPIO(bool *_out_value);
bool rp_HPGetIsE3HighSpeedGPIOOrDefault();

/**
* Returns the maximum frequency of the HS GPIO at the E3 output.
* Function rp_HPGetIsExtClockSelectorOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetIsE3HighSpeedGPIORate(uint32_t *_out_value);
uint32_t rp_HPGetIsE3HighSpeedGPIORateOrDefault();

/**
* Returns the presence of the E3 connector, as well as QSPI support for eMMC.
* Function rp_HPGetIsE3QSPIeMMCOrDefault. If it was not possible to determine the model, then the function returns a value for the model: STEMLab 125-10.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetIsE3QSPIeMMC(bool *_out_value);
bool rp_HPGetIsE3QSPIeMMCOrDefault();


/**
* Returns the name for the path to the FPGA files.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HP_E* values that indicate an error.
*/
int rp_HPGetFPGAVersion(char **_no_free_value);

/**
 * Print all parameters for current profile
 */
int rp_HPPrint();

/**
 * Print all parameters for all profiles
 */
int rp_HPPrintAll();

#ifdef __cplusplus
}
#endif

#endif // RP_HW_PROFILES_H
