/**
 * @file rp_hw-calib.h
 * @brief Red Pitaya Hardware Calibration Library API
 *
 * @copyright (c) Red Pitaya  http://www.redpitaya.com
 *
 * This library provides hardware calibration functionality for Red Pitaya devices.
 * It handles reading/writing calibration data from EEPROM and provides access to
 * calibration parameters for ADC, DAC, and filters.
 */

#ifndef RP_HW_CALIB_H
#define RP_HW_CALIB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include "rp_hw-profiles.h"

/**
  * @defgroup CalibError Error Codes
  * @brief Hardware calibration error codes
  * @{
  */

/**
  * @enum rp_calib_error
  * @brief Calibration operation status codes
  */
typedef enum {
    RP_HW_CALIB_OK = 0,   ///< Operation completed successfully
    RP_HW_CALIB_ERE = 1,  ///< Error reading from EEPROM
    RP_HW_CALIB_EWE = 2,  ///< Error writing to EEPROM
    RP_HW_CALIB_ENI = 3,  ///< Calibration values not initialized
    RP_HW_CALIB_EDM = 4,  ///< Board model detection error
    RP_HW_CALIB_ECH = 5,  ///< Invalid channel parameter
    RP_HW_CALIB_EIP = 6,  ///< Invalid parameter
    RP_HW_CALIB_EA = 7,   ///< Adjust error
    RP_HW_CALIB_UC = 8    ///< Unknown calibration
} rp_calib_error;

/** @} */  // end of CalibError group

/**
  * @defgroup CalibConstants Calibration Constants
  * @brief Hardware calibration constants and definitions
  * @{
  */

#define RP_HW_PACK_ID_V1 1  ///< 125-14 board version
#define RP_HW_PACK_ID_V2 2  ///< 250-12 board version
#define RP_HW_PACK_ID_V3 3  ///< 125-14 4Ch board version
#define RP_HW_PACK_ID_V4 4  ///< 122-16 board version
#define RP_HW_PACK_ID_V5 5  ///< Universal calibration
#define RP_HW_PACK_ID_V6 6  ///< Universal calibration. Calibration using FPGA.

#define MAX_UNIVERSAL_ITEMS_COUNT 512  ///< Maximum universal calibration items

/** @} */  // end of CalibConstants group

/**
  * @defgroup CalibTypes Calibration Types
  * @brief Hardware calibration data types
  * @{
  */

/**
  * @enum rp_channel_calib_t
  * @brief Input/Output channel identifiers
  */
typedef enum {
    RP_CH_1_CALIB = 0,  ///< Channel A
    RP_CH_2_CALIB = 1,  ///< Channel B
    RP_CH_3_CALIB = 2,  ///< Channel C
    RP_CH_4_CALIB = 3   ///< Channel D
} rp_channel_calib_t;

/**
  * @enum rp_gen_gain_calib_t
  * @brief Generator gain modes
  */
typedef enum {
    RP_GAIN_CALIB_1X = 0,  ///< x1 gain mode
    RP_GAIN_CALIB_5X = 1   ///< x5 gain mode
} rp_gen_gain_calib_t;

/**
  * @enum rp_acq_ac_dc_mode_calib_t
  * @brief Acquisition coupling modes
  */
typedef enum {
    RP_DC_CALIB = 0,  ///< DC coupling mode
    RP_AC_CALIB = 1   ///< AC coupling mode
} rp_acq_ac_dc_mode_calib_t;

/**
  * @struct uint_gain_calib_t
  * @brief Gain calibration parameters (integer version)
  */
typedef struct {
    uint32_t gain;      ///< Gain value
    uint32_t base;      ///< Base value
    uint8_t precision;  ///< Precision bits
    int32_t offset;     ///< Offset value
} uint_gain_calib_t;

/**
  * @struct channel_calib_t
  * @brief Channel calibration parameters
  */
typedef struct {
    float baseScale;      ///< Base scaling factor
    uint32_t calibValue;  ///< Calibration value
    int32_t offset;       ///< Offset value
    double gainCalc;      ///< Calculated gain
} channel_calib_t;

/**
  * @struct channel_filter_t
  * @brief Channel filter coefficients
  */
typedef struct {
    uint32_t aa;  ///< Filter coefficient AA
    uint32_t bb;  ///< Filter coefficient BB
    uint32_t pp;  ///< Filter coefficient PP
    uint32_t kk;  ///< Filter coefficient KK
} channel_filter_t;

/**
  * @struct rp_calib_params_t
  * @brief Complete calibration parameters structure
  */
typedef struct {
    char dataStructureId;  ///< Data structure identifier
    char wpCheck;          ///< Write protection check

    uint8_t fast_adc_count_1_1;               ///< ADC count for 1:1 ratio
    channel_calib_t fast_adc_1_1[4];          ///< ADC calibration (1:1)
    channel_filter_t fast_adc_filter_1_1[4];  ///< ADC filters (1:1)

    uint8_t fast_adc_count_1_20;               ///< ADC count for 1:20 ratio
    channel_calib_t fast_adc_1_20[4];          ///< ADC calibration (1:20)
    channel_filter_t fast_adc_filter_1_20[4];  ///< ADC filters (1:20)

    uint8_t fast_adc_count_1_1_ac;        ///< ADC count for AC mode (1:1)
    channel_calib_t fast_adc_1_1_ac[4];   ///< ADC calibration AC mode (1:1)
    uint8_t fast_adc_count_1_20_ac;       ///< ADC count for AC mode (1:20)
    channel_calib_t fast_adc_1_20_ac[4];  ///< ADC calibration AC mode (1:20)

    uint8_t fast_dac_count_x1;
    channel_calib_t fast_dac_x1[2];

    uint8_t fast_dac_count_x5;  // For 250-12
    channel_calib_t fast_dac_x5[2];
} rp_calib_params_t;

/**
  * @struct rp_eepromWpData_t
  * @brief Legacy EEPROM data structure
  */
typedef struct {
    uint8_t dataStructureId;  ///< Data structure ID
    uint8_t wpCheck;          ///< Write protection check
    uint8_t reserved[6];      ///< Reserved bytes
    int32_t feCalPar[100];    ///< Calibration parameters
} rp_eepromWpData_t;

/**
  * @struct rp_eepromUniData_item_t
  * @brief Universal calibration item
  */
typedef struct {
    uint16_t id;    ///< Parameter ID
    int32_t value;  ///< Parameter value
} __attribute__((__packed__)) rp_eepromUniData_item_t;

/**
  * @struct rp_eepromUniData_t
  * @brief Universal calibration data structure
  */
typedef struct {
    uint8_t dataStructureId;                                  ///< Data structure ID
    uint8_t wpCheck;                                          ///< Write protection check
    uint16_t count;                                           ///< Item count
    uint8_t reserved[4];                                      ///< Reserved bytes
    rp_eepromUniData_item_t item[MAX_UNIVERSAL_ITEMS_COUNT];  ///< Calibration items
} __attribute__((__packed__)) rp_eepromUniData_t;

typedef rp_eepromUniData_t rp_calib_params_universal_t;  ///< Universal calibration params alias

/** @} */  // end of CalibTypes group

/**
  * @defgroup CalibAPI Calibration API
  * @brief Hardware calibration functions
  * @{
  */

/**
  * @brief Initializes the calibration library
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibInit();

/**
  * @brief Initializes the calibration library for specific board model
  * @param model Board model to initialize for
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibInitSpecific(rp_HPeModels_t model);

/**
  * @brief Gets current calibration version
  * @param version Returns the version of the loaded calibration
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_GetCalibrationVersion(uint8_t* version);

/**
  * @brief Gets current calibration settings
  * @note Settings are cached after first read from EEPROM
  * @return Calibration parameters structure
  */
rp_calib_params_t rp_GetCalibrationSettings();

/**
  * @brief Gets default calibration settings
  * @note Settings are cached after first read from EEPROM
  * @return Default calibration parameters structure
  */
rp_calib_params_t rp_GetDefaultCalibrationSettings();

/**
  * @brief Gets default calibration settings in new format
  * @note Settings are cached after first read from EEPROM
  * @return Default calibration parameters structure
  */
rp_calib_params_t rp_GetDefaultUniCalibrationSettings();

/**
  * @brief Resets calibration to default values
  * @param use_factory_zone Use factory calibration zone
  * @param is_new_format Use new format calibration
  * @param setFilterZero Set filter coefficients to zero
  * @param version New format version. Versions RP_HW_PACK_ID_V5 and RP_HW_PACK_ID_V6 and higher are available
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibrationReset(bool use_factory_zone, bool is_new_format, bool setFilterZero, uint8_t version);

/**
  * @brief Copies factory calibration to user EEPROM
  * @param convert_to_new Convert to new format
  * @param version New format version. Versions RP_HW_PACK_ID_V5 and RP_HW_PACK_ID_V6 and higher are available
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibrationFactoryReset(bool convert_to_new, uint8_t version);

/**
  * @brief Writes calibration parameters to EEPROM
  * @param calib_params Calibration parameters to write
  * @param use_factory_zone Write to factory zone
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibrationWriteParams(rp_calib_params_t calib_params, bool use_factory_zone);

/**
  * @brief Writes calibration parameters without recalculation
  * @param calib_params Calibration parameters to write
  * @param use_factory_zone Write to factory zone
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibrationWriteParamsEx(rp_calib_params_t calib_params, bool use_factory_zone);

/**
  * @brief Sets calibration parameters in memory (without EEPROM write)
  * @param calib_params Calibration parameters to set
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibrationSetParams(rp_calib_params_t calib_params);

/**
  * @brief Gets raw EEPROM calibration data
  * @param[out] _out_data Pointer to receive data buffer
  * @param[out] _out_size Pointer to receive data size
  * @param use_factory_zone Read from factory zone
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibGetEEPROM(uint8_t** _out_data, uint16_t* _out_size, bool use_factory_zone);

/**
  * @brief Converts EEPROM data to calibration structure
  * @param data EEPROM data to convert
  * @param size Data size
  * @param[out] _out_calib Converted calibration structure
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibConvertEEPROM(uint8_t* data, uint16_t size, rp_calib_params_t* _out_calib);

/**
  * @brief Converts calibration to legacy format
  * @param[out] _out_calib Converted calibration structure
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibConvertToOld(rp_calib_params_t* _out_calib);

/**
  * @brief Gets name of universal calibration parameter
  * @param id Parameter ID
  * @param[out] _name Receive name string
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_GetNameOfUniversalId(uint16_t id, std::string* _name);

/**
  * @brief Prints calibration information
  * @param calib Calibration structure to print
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibPrint(rp_calib_params_t* calib);

/**
  * @brief Gets ADC filter calibration (1:1 ratio)
  * @param channel Channel to query
  * @param[out] _out_value Filter coefficients
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibGetFastADCFilter(rp_channel_calib_t channel, channel_filter_t* _out_value);

/**
  * @brief Gets ADC filter calibration (1:20 ratio)
  * @param channel Channel to query
  * @param[out] _out_value Filter coefficients
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibGetFastADCFilter_1_20(rp_channel_calib_t channel, channel_filter_t* _out_value);

/**
  * @brief Gets ADC calibration values (floating point)
  * @param channel Channel to query
  * @param mode Coupling mode
  * @param[out] _out_gain Gain value
  * @param[out] _out_offset Offset value
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibGetFastADCCalibValue(rp_channel_calib_t channel, rp_acq_ac_dc_mode_calib_t mode, double* _out_gain, int32_t* _out_offset);

/**
  * @brief Gets ADC calibration values (integer)
  * @param channel Channel to query
  * @param mode Coupling mode
  * @param[out] _out_calib Calibration structure
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibGetFastADCCalibValueI(rp_channel_calib_t channel, rp_acq_ac_dc_mode_calib_t mode, uint_gain_calib_t* _out_calib);

/**
  * @brief Gets ADC calibration values for 1:20 ratio (floating point)
  * @param channel Channel to query
  * @param mode Coupling mode
  * @param[out] _out_gain Gain value
  * @param[out] _out_offset Offset value
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibGetFastADCCalibValue_1_20(rp_channel_calib_t channel, rp_acq_ac_dc_mode_calib_t mode, double* _out_gain, int32_t* _out_offset);

/**
  * @brief Gets ADC calibration values for 1:20 ratio (integer)
  * @param channel Channel to query
  * @param mode Coupling mode
  * @param[out] _out_calib Calibration structure
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibGetFastADCCalibValue_1_20I(rp_channel_calib_t channel, rp_acq_ac_dc_mode_calib_t mode, uint_gain_calib_t* _out_calib);

/**
  * @brief Gets DAC calibration values (floating point)
  * @param channel Channel to query
  * @param gain_mode Gain mode
  * @param mode Impedance mode
  * @param[out] _out_gain Gain value
  * @param[out] _out_offset Offset value
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibGetFastDACCalibValue(rp_channel_calib_t channel, rp_gen_gain_calib_t gain_mode, double* _out_gain, int32_t* _out_offset);

/**
  * @brief Gets DAC calibration values (integer)
  * @param channel Channel to query
  * @param gain_mode Gain mode
  * @param mode Impedance mode
  * @param[out] _out_calib Calibration structure
  * @return Status code (RP_HW_CALIB_OK on success)
  */
rp_calib_error rp_CalibGetFastDACCalibValueI(rp_channel_calib_t channel, rp_gen_gain_calib_t gain_mode, uint_gain_calib_t* _out_calib);

/** @} */  // end of CalibAPI group

#endif  // RP_HW_CALIB_H
