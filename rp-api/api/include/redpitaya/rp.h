/**
 * $Id: $
 *
 * @file rp.h
 * @brief Red Pitaya library API interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __RP_H
#define __RP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "rp_enums.h"
#include "rp_acq.h"
#include "rp_acq_axi.h"
#include "rp_gen.h"

#define ADC_BUFFER_SIZE         (16 * 1024)
#define DAC_BUFFER_SIZE         (16 * 1024)

#define RISE_FALL_MIN_RATIO     0.0001      // ratio of rise/fall time to period
#define RISE_FALL_MAX_RATIO     0.1

/** @name Error codes
 *  Various error codes returned by the API.
 */
///@{

/** Success */
#define RP_OK     0
/** Failed to Open EEPROM Device */
#define RP_EOED   1
/** Failed to Open Memory Device */
#define RP_EOMD   2
/** Failed to Close Memory Device*/
#define RP_ECMD   3
/** Failed to Map Memory Device */
#define RP_EMMD   4
/** Failed to Unmap Memory Device */
#define RP_EUMD   5
/** Value Out Of Range */
#define RP_EOOR   6
/** LED Input Direction is not valid */
#define RP_ELID   7
/** Modifying Read Only field */
#define RP_EMRO   8
/** Writing to Input Pin is not valid */
#define RP_EWIP   9
/** Invalid Pin number */
#define RP_EPN    10
/** Uninitialized Input Argument */
#define RP_UIA    11
/** Failed to Find Calibration Parameters */
#define RP_FCA    12
/** Failed to Read Calibration Parameters */
#define RP_RCA    13
/** Buffer too small */
#define RP_BTS    14
/** Invalid parameter value */
#define RP_EIPV   15
/** Unsupported Feature */
#define RP_EUF    16
/** Data not normalized */
#define RP_ENN    17
/** Failed to open bus */
#define RP_EFOB   18
/** Failed to close bus */
#define RP_EFCB   19
/** Failed to acquire bus access */
#define RP_EABA   20
/** Failed to read from the bus */
#define RP_EFRB   21
/** Failed to write to the bus */
#define RP_EFWB   22
/** Extension module not connected */
#define RP_EMNC   23
/** Command not supported */
#define RP_NOTS   24

#define SPECTR_OUT_SIG_LEN (2*1024)

///@}



/** @name General
 */
///@{


/**
 * Initializes the library. It must be called first, before any other library method.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */

int rp_Init();

/**
 * Initializes the library. It must be called first, before any other library method.
 * @param reset Reset to default configuration on api
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */

int rp_InitReset(bool reset);

int rp_IsApiInit();

/**
 * Releases the library resources. It must be called last, after library is not used anymore. Typically before
 * application exits.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_Release();

/**
* Resets all modules. Typically calles after rp_Init()
* application exits.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_Reset();

/**
 * Retrieves the library version number
 * @return Library version
 */
const char* rp_GetVersion();

/**
 * Returns textual representation of error code.
 * @param errorCode Error code returned from API.
 * @return Textual representation of error given error code.
 */
const char* rp_GetError(int errorCode);


///@}
/** @name Digital loop
*/
///@{

/**
* Enable or disables digital loop. This internally connect output to input
* @param enable True if you want to enable this feature or false if you want to disable it
* Each rp_GetCalibrationSettings call returns the same cached setting values.
* @return Calibration settings
*/
int rp_EnableDigitalLoop(bool enable);


///@}


/** @name Identification
 */
///@{

/**
* Gets FPGA Synthesized ID
*/
int rp_IdGetID(uint32_t *id);

/**
* Gets FPGA Unique DNA
*/
int rp_IdGetDNA(uint64_t *dna);

///@}


/**
 * LED methods
 */

int rp_LEDSetState(uint32_t state);
int rp_LEDGetState(uint32_t *state);

/**
 * GPIO methods
 */

int rp_GPIOnSetDirection(uint32_t direction);
int rp_GPIOnGetDirection(uint32_t *direction);
int rp_GPIOnSetState(uint32_t state);
int rp_GPIOnGetState(uint32_t *state);
int rp_GPIOpSetDirection(uint32_t direction);
int rp_GPIOpGetDirection(uint32_t *direction);
int rp_GPIOpSetState(uint32_t state);
int rp_GPIOpGetState(uint32_t *state);

int rp_EnableDebugReg();

/** @name Digital Input/Output
 */
///@{

/**
* Sets digital pins to default values. Pins DIO1_P - DIO7_P, RP_DIO0_N - RP_DIO7_N are set all INPUT and to LOW. LEDs are set to LOW/OFF
*/
int rp_DpinReset();

/**
 * Sets digital input output pin state.
 * @param pin    Digital input output pin.
 * @param state  High/Low state that will be set at the given pin.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_DpinSetState(rp_dpin_t pin, rp_pinState_t state);

/**
 * Gets digital input output pin state.
 * @param pin    Digital input output pin.
 * @param state  High/Low state that is set at the given pin.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_DpinGetState(rp_dpin_t pin, rp_pinState_t* state);

/**
 * Sets digital input output pin direction. LED pins are already automatically set to the output direction,
 * and they cannot be set to the input direction. DIOx_P and DIOx_N are must set either output or input direction
 * before they can be used. When set to input direction, it is not allowed to write into these pins.
 * @param pin        Digital input output pin.
 * @param direction  In/Out direction that will be set at the given pin.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_DpinSetDirection(rp_dpin_t pin, rp_pinDirection_t direction);

/**
 * Gets digital input output pin direction.
 * @param pin        Digital input output pin.
 * @param direction  In/Out direction that is set at the given pin.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_DpinGetDirection(rp_dpin_t pin, rp_pinDirection_t* direction);

///@}


/** @name Daisy chain clocks and triggers
 */
///@{

/**
 * Enables trigger sync over SATA daisy chain connectors.
 * Once the primary board will be triggered, the trigger will be forwarded to the secondary board over
 * the SATA connector where the trigger can be detected using rp_GenTriggerSource with EXT_NE selector.
 * Noticed that the trigger that is received over SATA is ORed with the external trigger from GPIO.
 *
 * @param enable  Turns on the mode.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_SetEnableDaisyChainTrigSync(bool enable);

/**
 * Returns the current state of the SATA daisy chain mode.
 * @param status  Current state.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GetEnableDaisyChainTrigSync(bool *status);

/**
 * Function turns GPION_0 into trigger output for selected source - acquisition or generation
 *
 * @param enable  Turns on the mode.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_SetDpinEnableTrigOutput(bool enable);

/**
 * Returns the current mode state for GPION_0. If true, then the pin mode works as a source
 * @param status  Current state.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GetDpinEnableTrigOutput(bool *state);

/**
 * Sets the trigger source mode. ADC/DAC
 *
 * @param mode  Sets the mode.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_SetSourceTrigOutput(rp_outTiggerMode_t mode);

/**
 * Returns the trigger source mode. ADC/DAC
 * @param mode  Returns the current mode.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GetSourceTrigOutput(rp_outTiggerMode_t *mode);

/**
 * Enables clock sync over SATA daisy chain connectors. Primary board will start generating clock for secondary unit and so on.
 *
 * @param enable  Turns on the mode.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_SetEnableDiasyChainClockSync(bool enable);

/*
 * Returns the current state of the SATA daisy chain mode.
 * @param status  Current state.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GetEnableDiasyChainClockSync(bool *state);


///@}


/** @name Analog Inputs/Outputs
 */
///@{

/**
* Sets analog outputs to default values (0V).
*/
int rp_ApinReset();

/**
 * Gets value from analog pin in volts.
 * @param pin    Analog pin.
 * @param value  Value on analog pin in volts
 * @param raw    raw value
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_ApinGetValue(rp_apin_t pin, float* value, uint32_t* raw);

/**
 * Gets raw value from analog pin.
 * @param pin    Analog pin.
 * @param value  Raw value on analog pin
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_ApinGetValueRaw(rp_apin_t pin, uint32_t* value);

/**
 * Sets value in volts on analog output pin.
 * @param pin    Analog output pin.
 * @param value  Value in volts to be set on given output pin.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_ApinSetValue(rp_apin_t pin, float value);

/**
 * Sets raw value on analog output pin.
 * @param pin    Analog output pin.
 * @param value  Raw value to be set on given output pin.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_ApinSetValueRaw(rp_apin_t pin, uint32_t value);

/**
 * Gets range in volts on specific pin.
 * @param pin      Analog input output pin.
 * @param min_val  Minimum value in volts on given pin.
 * @param max_val  Maximum value in volts on given pin.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_ApinGetRange(rp_apin_t pin, float* min_val,  float* max_val);


/** @name Analog Inputs
 */
///@{

/**
 * Gets value from analog pin in volts.
 * @param pin    pin index
 * @param value  voltage
 * @param raw    raw value
 * @return       RP_OK - successful, RP_E* - failure
 */
int rp_AIpinGetValue(int unsigned pin, float* value, uint32_t* raw);

/**
 * Gets raw value from analog pin.
 * @param pin    pin index
 * @param value  raw 12 bit XADC value
 * @return       RP_OK - successful, RP_E* - failure
 */
int rp_AIpinGetValueRaw(int unsigned pin, uint32_t* value);


/** @name Analog Outputs
 */
///@{

/**
* Sets analog outputs to default values (0V).
*/
int rp_AOpinReset();

/**
 * Gets value from analog pin in volts.
 * @param pin    Analog output pin index.
 * @param value  Value on analog pin in volts
 * @param raw  Value on analog pin in raw
 * @return       RP_OK - successful, RP_E* - failure
 */
int rp_AOpinGetValue(int unsigned pin, float* value, uint32_t* raw);

/**
 * Gets raw value from analog pin.
 * @param pin    Analog output pin index.
 * @param value  Raw value on analog pin
 * @return       RP_OK - successful, RP_E* - failure
 */
int rp_AOpinGetValueRaw(int unsigned pin, uint32_t* value);

/**
 * Sets value in volts on analog output pin.
 * @param pin    Analog output pin index.
 * @param value  Value in volts to be set on given output pin.
 * @return       RP_OK - successful, RP_E* - failure
 */
int rp_AOpinSetValue(int unsigned pin, float value);

/**
 * Sets raw value on analog output pin.
 * @param pin    Analog output pin index.
 * @param value  Raw value to be set on given output pin.
 * @return       RP_OK - successful, RP_E* - failure
 */
int rp_AOpinSetValueRaw(int unsigned pin, uint32_t value);

/**
 * Gets range in volts on specific pin.
 * @param pin      Analog input output pin index.
 * @param min_val  Minimum value in volts on given pin.
 * @param max_val  Maximum value in volts on given pin.
 * @return       RP_OK - successful, RP_E* - failure
 */
int rp_AOpinGetRange(int unsigned pin, float* min_val,  float* max_val);


///@}

/** @name PLL Control for 250-12
*/
///@{

/**
* Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param enable return current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GetPllControlEnable(bool *enable);

/**
* Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param enable Flag enabling PLL control.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_SetPllControlEnable(bool enable);

/**
* Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param status Get current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GetPllControlLocked(bool *status);
///@}

#ifdef __cplusplus
}
#endif

#endif //__RP_H
