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


/** @name Error codes
 *  Various error codes returned by the API.
 */
///@{

/** Success */
#define RP_OK     0
/** Failed to Open Memory Device */
#define RP_EOMD   1
/** Failed to Close Memory Device*/
#define RP_ECMD   2
/** Failed to Map Memory Device */
#define RP_EMMD   3
/** Failed to Unmap Memory Device */
#define RP_EUMD   4
/** Value Out Of Range */
#define RP_EOOR   5
/** LED Input Direction is not valid */
#define RP_ELID   6
/** Modifying Read Only field */
#define RP_EMRO   7
/** Writing to Input Pin is not valid */
#define RP_EWIP   8
/** Invalid Pin number */
#define RP_EPN    9

///@}

/**
 * Type representing digital input output pins.
 */
typedef enum {
	RP_LED0,       //!< LED 0
	RP_LED1,       //!< LED 1
	RP_LED2,       //!< LED 2
	RP_LED3,       //!< LED 3
	RP_LED4,       //!< LED 4
	RP_LED5,       //!< LED 5
	RP_LED6,       //!< LED 6
	RP_LED7,       //!< LED 7
	RP_DIO0_P,     //!< DIO_P 0
	RP_DIO1_P,     //!< DIO_P 1
	RP_DIO2_P,     //!< DIO_P 2
	RP_DIO3_P,     //!< DIO_P 3
	RP_DIO4_P,     //!< DIO_P 4
	RP_DIO5_P,     //!< DIO_P 5
	RP_DIO6_P,     //!< DIO_P 6
	RP_DIO7_P,	   //!< DIO_P 7
	RP_DIO0_N,     //!< DIO_N 0
	RP_DIO1_N,     //!< DIO_N 1
	RP_DIO2_N,     //!< DIO_N 2
	RP_DIO3_N,     //!< DIO_N 3
	RP_DIO4_N,     //!< DIO_N 4
	RP_DIO5_N,     //!< DIO_N 5
	RP_DIO6_N,     //!< DIO_N 6
	RP_DIO7_N      //!< DIO_N 7
} rp_dpin_t;

/**
 * Type representing pin's high or low state (on/off).
 */
typedef enum {
	RP_LOW, //!< Low state
	RP_HIGH //!< High state
} rp_pinState_t;

/**
 * Type representing pin's input or output direction.
 */
typedef enum {
	RP_IN, //!< Input direction
	RP_OUT //!< Output direction
} rp_pinDirection_t;

/**
* Type representing analog input output pins.
*/
typedef enum {
	RP_AOUT0,      //!< Analog output 0
	RP_AOUT1,      //!< Analog output 1
	RP_AOUT2,      //!< Analog output 2
	RP_AOUT3,      //!< Analog output 3
	RP_AIN0,       //!< Analog input 0
	RP_AIN1,       //!< Analog input 1
	RP_AIN2,       //!< Analog input 2
	RP_AIN3        //!< Analog input 3
} rp_apin_t;

/**
* Type representing system health information.
*/
typedef enum {
	RP_TEMP_FPGA,  //!< FPGA temperature
	RP_VCC_PINT,   //!< VCC PINT
	RP_VCC_PAUX,   //!< VCC PAUX
	RP_VCC_BRAM,   //!< VCC BRAM
	RP_VCC_INT,    //!< VCC INT
	RP_VCC_AUX,    //!< VCC AUX
	RP_VCC_DDR     //!< VCC DDR
} rp_health_t;


/**
* Type representing acquire signal sampling rate.
*/
typedef enum {
	RP_SMP_125M,     //!< Sample rate 125Msps; Buffer time length 131us; Decimation 1
	RP_SMP_15_625M,  //!< Sample rate 15.625Msps; Buffer time length 1.048ms; Decimation 8
	RP_SMP_1_953M,   //!< Sample rate 1.953Msps; Buffer time length 8.388ms; Decimation 64
	RP_SMP_122_070K, //!< Sample rate 122.070ksps; Buffer time length 134.2ms; Decimation 1024
	RP_SMP_15_258K,  //!< Sample rate 15.258ksps; Buffer time length 1.073s; Decimation 8192
	RP_SMP_1_907K    //!< Sample rate 1.907ksps; Buffer time length 8.589s; Decimation 65536
} rp_acq_sampling_rate_t;


/**
* Type representing decimation used at acquiring signal.
*/
typedef enum {
	RP_DEC_1,     //!< Sample rate 125Msps; Buffer time length 131us; Decimation 1
	RP_DEC_8,     //!< Sample rate 15.625Msps; Buffer time length 1.048ms; Decimation 8
	RP_DEC_64,    //!< Sample rate 1.953Msps; Buffer time length 8.388ms; Decimation 64
	RP_DEC_1024,  //!< Sample rate 122.070ksps; Buffer time length 134.2ms; Decimation 1024
	RP_DEC_8192,  //!< Sample rate 15.258ksps; Buffer time length 1.073s; Decimation 8192
	RP_DEC_65536  //!< Sample rate 1.907ksps; Buffer time length 8.589s; Decimation 65536
} rp_acq_decimation_t;


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
 * Releases the library resources. It must be called last, after library is not used anymore. Typically before
 * application exits.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_Release();

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

/** @name Digital Input/Output
 */
///@{

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

/** @name Analog Input/Output
*/
///@{

/**
* Gets value from analog pin in volts.
* @param pin    Analog pin.
* @param value  Value on analog pin in volts
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_ApinGetValue(rp_apin_t pin, float* value);

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


/** @name Acquire
*/
///@{

/**
 * Sets the decimation used at acquiring signal. There is only a set of pre-defined decimation
 * values which can be specified. See the #rp_acq_decimation_t enum values.
 * @param decimation Specify one of pre-defined decimation values
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_SetDecimation(rp_acq_decimation_t decimation);

/**
 * Gets the decimation used at acquiring signal. There is only a set of pre-defined decimation
 * values which can be specified. See the #rp_acq_decimation_t enum values.
 * @param decimation Returns one of pre-defined decimation values which is currently set.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GetDecimation(rp_acq_decimation_t* decimation);

/**
 * Gets the decimation used at acquiring signal in a numerical form. Although this method returns an integer
 * value representing the current value of the decimation, there is only a set of pre-defined decimation
 * values which can be returned. See the #rp_acq_decimation_t enum values.
 * @param decimation Returns decimation value which is currently set.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GetDecimationNum(uint32_t* decimation);

/**
 * Sets the sampling rate for acquiring signal. There is only a set of pre-defined sampling rate
 * values which can be specified. See the #rp_acq_sampling_rate_t enum values.
 * @param sampling_rate Specify one of pre-defined sampling rate value
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetSamplingRate(rp_acq_sampling_rate_t sampling_rate);

/**
 * Gets the sampling rate for acquiring signal. There is only a set of pre-defined sampling rate
 * values which can be returned. See the #rp_acq_sampling_rate_t enum values.
 * @param sampling_rate Returns one of pre-defined sampling rate value which is currently set
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetSamplingRate(rp_acq_sampling_rate_t* sampling_rate);

/**
 * Gets the sampling rate for acquiring signal in a numerical form in Hz. Although this method returns a float
 * value representing the current value of the sampling rate, there is only a set of pre-defined sampling rate
 * values which can be returned. See the #rp_acq_sampling_rate_t enum values.
 * @param sampling_rate returns currently set sampling rate in Hz
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetSamplingRateHz(float* sampling_rate);

/**
 * Enables or disables averaging of data between samples.
 * Data between samples can be averaged by setting the averaging flag in the Data decimation register.
 * @param enabled When true, the averaging is enabled, otherwise it is disabled.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetAveraging(bool enabled);

/**
 * Returns information if averaging of data between samples is enabled or disabled.
 * Data between samples can be averaged by setting the averaging flag in the Data decimation register.
 * @param enabled Set to true when the averaging is enabled, otherwise is it set to false.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetAveraging(bool *enabled);

///@}


///@}

/** @name Health
*/
///@{


/**
* Gets data about system health like temperature
* @param sensor   From witch sensor the data is read
* @param value    The returned value
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_HealthGetValue(rp_health_t sensor, float* value);

///@}


#ifdef __cplusplus
}
#endif

#endif //__RP_H
