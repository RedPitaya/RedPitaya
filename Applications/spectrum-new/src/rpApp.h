/**
* $Id: $
*
* @file rpApp.h
* @brief Red Pitaya application library API interface
*
* @Author Red Pitaya
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#ifndef __RP_APP_H
#define __RP_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rp.h"


/** @name Error codes
*  Various error codes returned by the API.
*/
///@{

/** Failed to Start a Thread */
#define RP_APP_EST  100
/** No signal found */
#define RP_APP_ENS  101
/** Failed to allocate array */
#define RP_EAA      102

///@}

/**
* Type representing input gain.
*/
typedef enum {
    RPAPP_OSC_IN_GAIN_LV,       //!< Input gain LV
    RPAPP_OSC_IN_GAIN_HV,       //!< Input gain HV
} rpApp_osc_in_gain_t;

/**
* Type representing trigger source.
*/
typedef enum {
    RPAPP_OSC_TRIG_SRC_CH1,     //!< Trigger source channel 1
    RPAPP_OSC_TRIG_SRC_CH2,     //!< Trigger source channel 2
    RPAPP_OSC_TRIG_SRC_EXTERNAL //!< Trigger source external
} rpApp_osc_trig_source_t;

/**
* Type representing trigger slope.
*/
typedef enum {
    RPAPP_OSC_TRIG_SLOPE_NE,    //!< Trigger source slope negative
    RPAPP_OSC_TRIG_SLOPE_PE     //!< Trigger source slope positive
} rpApp_osc_trig_slope_t;

/**
* Type representing trigger mode.
*/
typedef enum {
    RPAPP_OSC_TRIG_AUTO,        //!< Trigger sweep auto
    RPAPP_OSC_TRIG_NORMAL,      //!< Trigger sweep normal
    RPAPP_OSC_TRIG_SINGLE       //!< Trigger sweep single
} rpApp_osc_trig_sweep_t;

/**
* Type representing oscilloscope math and measure source
*/
typedef enum {
    RPAPP_OSC_SOUR_CH1,         //!< Trigger source ch1
    RPAPP_OSC_SOUR_CH2,         //!< Trigger source ch2
    RPAPP_OSC_SOUR_MATH         //!< Trigger source math
} rpApp_osc_source;

/**
* Type representing math operations.
*/
typedef enum {
    RPAPP_OSC_MATH_NONE,        //!< Math operation add
    RPAPP_OSC_MATH_ADD,         //!< Math operation add
    RPAPP_OSC_MATH_SUB,         //!< Math operation subtract
    RPAPP_OSC_MATH_MUL,         //!< Math operation mltiply
    RPAPP_OSC_MATH_DIV,         //!< Math operation divide
    RPAPP_OSC_MATH_ABS,         //!< Math operation absolute
    RPAPP_OSC_MATH_DER,         //!< Math operation derivative
    RPAPP_OSC_MATH_INT,         //!< Math operation integrate
} rpApp_osc_math_oper_t;

/** @name General
*/
///@{

/**
* Initializes the library. It must be called first, before any other library method.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_Init();

/**
* Releases the library resources. It must be called last, after library is not used anymore. Typically before
* application exits.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_Release();

/**
* Resets all modules. Typically called after rpApp_Init()
* application exits.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_Reset();

/**
* Retrieves the library version number
* @return Library version
*/
const char* rpApp_GetVersion();

/**
* Returns textual representation of error code.
* @param errorCode Error code returned from API.
* @return Textual representation of error given error code.
*/
const char* rpApp_GetError(int errorCode);

///@}

/** @name Oscilloscope
*/
///@{

/**
* Starts oscilloscope.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetViewSize(uint32_t *size);

///@}

// SPECTRUM

int rpApp_SpecRun();

int rpApp_SpecStop();


int rpApp_SpecGetViewData(int source, float *data, uint32_t size);

int rpApp_SpecGetParams();

int rpApp_SpecGetJpgIdx(int* jpg);

int rpApp_SpecGetPeakPower(int channel, float* power);

int rpApp_SpecGetPeakFreq(int channel, float* freq);

int rpApp_SpecSetUnit(int freq);


#ifdef __cplusplus
}
#endif

#endif //__RP_H
