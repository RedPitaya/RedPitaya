/**
 * $Id: $
 *
 * @file rp_hw.h
 * @brief Red Pitaya library API for hardware calibration
 *
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef RP_HW_CALIB_H
#define RP_HW_CALIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "rp_hw-profiles.h"


/** @name Error codes
 *  Various error codes returned by the API.
 */
///@{

/** Success */
#define RP_HW_CALIB_OK     0
/** Error read from eeprom */
#define RP_HW_CALIB_ERE    1
/** Error write to eeprom */
#define RP_HW_CALIB_EWE    2
/** Error Calibration values are not initialized. */
#define RP_HW_CALIB_ENI    3
/** Board Model Detection Error. */
#define RP_HW_CALIB_EDM    4
/** Invalid channel parameter. */
#define RP_HW_CALIB_ECH    5
/** Invalid parameter. */
#define RP_HW_CALIB_EIP    6

///@}

#define RP_HW_PACK_ID_V1 1
#define RP_HW_PACK_ID_V2 2
#define RP_HW_PACK_ID_V3 3

/**
 * Type representing Input/Output channels.
 */
typedef enum {
    RP_CH_1_CALIB = 0,    //!< Channel A
    RP_CH_2_CALIB = 1,    //!< Channel B
    RP_CH_3_CALIB = 2,    //!< Channel C
    RP_CH_4_CALIB = 3     //!< Channel D
} rp_channel_calib_t;


typedef enum {
    RP_GAIN_CALIB_1X = 0,   //!< x1 mode for generator
    RP_GAIN_CALIB_5X = 1    //!< x5 mode for generator
} rp_gen_gain_calib_t;


typedef enum {
    RP_DC_CALIB = 0, // Normal mode for 125_14
    RP_AC_CALIB = 1  // AC mode
} rp_acq_ac_dc_mode_calib_t;


typedef struct{
    uint32_t gain;
    uint32_t base;
    uint8_t  precision;
    int32_t  offset;
} uint_gain_calib_t;

typedef struct{
    float       fullScale;
    uint32_t    calibValue;
    int32_t     offset;
    double      gainCalc;
} channel_calib_t;

typedef struct
{
    uint32_t aa;
    uint32_t bb;
    uint32_t pp;
    uint32_t kk;
} channel_filter_t;


typedef struct {
    char dataStructureId;
    char wpCheck;

    uint8_t fast_adc_count_1_1; // For 250-12 is DC mode
    channel_calib_t fast_adc_1_1[4];
    channel_filter_t fast_adc_filter_1_1[4];
    uint8_t fast_adc_count_1_20;
    channel_calib_t fast_adc_1_20[4]; // For 250-12 is DC mode
    channel_filter_t fast_adc_filter_1_20[4];

    uint8_t fast_adc_count_1_1_ac; // For 250-12
    channel_calib_t fast_adc_1_1_ac[4];
    uint8_t fast_adc_count_1_20_ac; // For 250-12
    channel_calib_t fast_adc_1_20_ac[4];

    uint8_t fast_dac_count_x1;
    channel_calib_t fast_dac_x1[2];

    uint8_t fast_dac_count_x5; // For 250-12
    channel_calib_t fast_dac_x5[2];
} rp_calib_params_t;

typedef struct {
    uint8_t  dataStructureId;
    uint8_t  wpCheck;
    uint8_t  reserved[6];
    int32_t  feCalPar[100];
} rp_eepromWpData_t;


/** @name General
 */
///@{

/**
 * Initializes a library card with calibration
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_CALIB_E* values that indicate an error.
 */
int rp_CalibInit();

/**
 * Initializes a library card with calibration for specific model
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_CALIB_E* values that indicate an error.
 */
int rp_CalibInitSpecific(rp_HPeModels_t model);


/**
 * Returns calibration settings.
 * These calibration settings are populated only once from EEPROM at rp_Init().
 * Each rp_GetCalibrationSettings call returns the same cached setting values.
 * @return Calibration settings
 */
rp_calib_params_t rp_GetCalibrationSettings();

/**
* Returns default calibration settings.
* These calibration settings are populated only once from EEPROM at rp_Init().
* Each rp_GetCalibrationSettings call returns the same cached setting values.
* @return Calibration settings
*/
rp_calib_params_t rp_GetDefaultCalibrationSettings();

/**
* Set default calibration values.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_CalibrationReset(bool use_factory_zone);

/**
* Copy factory calibration values into user eeprom.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_CalibrationFactoryReset();

/**
* Write calibration values.
* Calibration data is written to EEPROM and repopulated so that rp_GetCalibrationSettings works properly.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_CalibrationWriteParams(rp_calib_params_t calib_params,bool use_factory_zone);

/**
* Set calibration values in memory.
* Calibration values are written to temporary memory, but not permanently.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_CalibrationSetParams(rp_calib_params_t calib_params);

/**
* Returns the calibration as it is presented in the eeprom.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_CalibGetEEPROM(rp_eepromWpData_t *calib_params,bool use_factory_zone);

/**
* The function converts the data to a common format
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_CalibConvertEEPROM(rp_eepromWpData_t *calib_params,rp_calib_params_t *out);

/**
* Displays calibration information
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_CalibPrint(rp_calib_params_t *calib);

/**
* Gets the calibration settings for the filter
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_CalibGetFastADCFilter(rp_channel_calib_t channel,channel_filter_t *out);

/**
* Gets the calibration settings for the filter (HIGH mode 1:20)
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_CalibGetFastADCFilter_1_20(rp_channel_calib_t channel,channel_filter_t *out);

/**
* Gets the calibration settings for the filter (HIGH mode 1:20)
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_CalibGetFastADCFilter_1_20(rp_channel_calib_t channel,channel_filter_t *out);

/**
* Returns the calibration values for the selected channel and mode
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_CalibGetFastADCCalibValue(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, double *gain,int32_t *offset);
int rp_CalibGetFastADCCalibValueI(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, uint_gain_calib_t *calib);

/**
* Returns the calibration values for the selected channel and mode (HIGH mode 1:20)
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_CalibGetFastADCCalibValue_1_20(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, double *gain,int32_t *offset);
int rp_CalibGetFastADCCalibValue_1_20I(rp_channel_calib_t channel,rp_acq_ac_dc_mode_calib_t mode, uint_gain_calib_t *calib);

/**
* Returns the calibration values for the selected channel and mode
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_CalibGetFastDACCalibValue(rp_channel_calib_t channel,rp_gen_gain_calib_t mode, double *gain,int32_t *offset);
int rp_CalibGetFastDACCalibValueI(rp_channel_calib_t channel,rp_gen_gain_calib_t mode, uint_gain_calib_t *calib);


///@}

#ifdef __cplusplus
}
#endif

#endif //__RP_HW_CALIB_H
