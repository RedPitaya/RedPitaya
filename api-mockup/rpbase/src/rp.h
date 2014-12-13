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


/** @name General
 */
///@{


/**
 * Initializes the library. It must be called first, before any other library method.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate error.
 */
int rp_Init();

/**
 * Releases the library resources. It must be called last, after library is not used anymore. Typically before
 * application exits.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate error.
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
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate error.
 */
int rp_DpinSetState(rp_dpin_t pin, rp_pinState_t state);

/**
 * Gets digital input output pin state.
 * @param pin    Digital input output pin.
 * @param state  High/Low state that is set at the given pin.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate error.
 */
int rp_DpinGetState(rp_dpin_t pin, rp_pinState_t* state);

/**
 * Sets digital input output pin direction. LED pins are already automatically set to the output direction,
 * and they cannot be set to the input direction. DIOx_P and DIOx_N are must set either output or input direction
 * before they can be used. When set to input direction, it is not allowed to write into these pins.
 * @param pin        Digital input output pin.
 * @param direction  In/Out direction that will be set at the given pin.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate error.
 */
int rp_DpinSetDirection(rp_dpin_t pin, rp_pinDirection_t direction);

/**
 * Gets digital input output pin direction.
 * @param pin        Digital input output pin.
 * @param direction  In/Out direction that is set at the given pin.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate error.
 */
int rp_DpinGetDirection(rp_dpin_t pin, rp_pinDirection_t* direction);

///@}


#ifdef __cplusplus
}
#endif

#endif //__RP_H
