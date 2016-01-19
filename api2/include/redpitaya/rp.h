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

#define ADC_BUFFER_SIZE             (16*1024)

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

#define SPECTR_OUT_SIG_LEN (2*1024)

///@}

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

typedef enum {
    RP_WAVEFORM_SINE,       //!< Wave form sine
    RP_WAVEFORM_SQUARE,     //!< Wave form square
    RP_WAVEFORM_TRIANGLE,   //!< Wave form triangle
    RP_WAVEFORM_RAMP_UP,    //!< Wave form sawtooth (/|)
    RP_WAVEFORM_RAMP_DOWN,  //!< Wave form reversed sawtooth (|\)
    RP_WAVEFORM_DC,         //!< Wave form dc
    RP_WAVEFORM_PWM,        //!< Wave form pwm
    RP_WAVEFORM_ARBITRARY   //!< Use defined wave form
} rp_waveform_t;

typedef enum {
    RP_GEN_MODE_CONTINUOUS, //!< Continuous signal generation
    RP_GEN_MODE_BURST,      //!< Signal is generated N times, wher N is defined with rp_GenBurstCount method
    RP_GEN_MODE_STREAM      //!< User can continuously write data to buffer
} rp_gen_mode_t;


typedef enum {
    RP_GEN_TRIG_SRC_INTERNAL = 1,   //!< Internal trigger source
    RP_GEN_TRIG_SRC_EXT_PE = 2,     //!< External trigger source positive edge
    RP_GEN_TRIG_SRC_EXT_NE = 3,     //!< External trigger source negative edge
    RP_GEN_TRIG_GATED_BURST     //!< External trigger gated burst
} rp_trig_src_t;

/**
 * Type representing Input/Output channels.
 */
typedef enum {
    RP_CH_1, //!< Channel A
    RP_CH_2  //!< Channel B
} rp_channel_t;

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


/**
 * Type representing different trigger sources used at acquiring signal.
 */
typedef enum {
    RP_TRIG_SRC_DISABLED, //!< Trigger is disabled
    RP_TRIG_SRC_NOW,      //!< Trigger triggered now (immediately)
    RP_TRIG_SRC_CHA_PE,   //!< Trigger set to Channel A threshold positive edge
    RP_TRIG_SRC_CHA_NE,   //!< Trigger set to Channel A threshold negative edge
    RP_TRIG_SRC_CHB_PE,   //!< Trigger set to Channel B threshold positive edge
    RP_TRIG_SRC_CHB_NE,   //!< Trigger set to Channel B threshold negative edge
    RP_TRIG_SRC_EXT_PE,   //!< Trigger set to external trigger positive edge (DIO0_P pin)
    RP_TRIG_SRC_EXT_NE,   //!< Trigger set to external trigger negative edge (DIO0_P pin)
    RP_TRIG_SRC_AWG_PE,   //!< Trigger set to arbitrary wave generator application positive edge
    RP_TRIG_SRC_AWG_NE    //!< Trigger set to arbitrary wave generator application negative edge
} rp_acq_trig_src_t;


/**
 * Type representing different trigger states.
 */
typedef enum {
    RP_TRIG_STATE_TRIGGERED, //!< Trigger is triggered/disabled
    RP_TRIG_STATE_WAITING,   //!< Trigger is set up and waiting (to be triggered)
} rp_acq_trig_state_t;


typedef struct wf_func_table_t {
    int (*rp_spectr_wf_init)();
    int (*rp_spectr_wf_clean)();
    int (*rp_spectr_wf_clean_map)();
    int (*rp_spectr_wf_calc)(double *cha_in, double *chb_in, float koeff, float koeff2);
    int (*rp_spectr_wf_save_jpeg)(const char *wf_file1, const char *wf_file2);
} wf_func_table_t;


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


/** @name Analog Inputs
 */
///@{

/**
 * Gets value from analog pin in volts.
 * @param pin    pin index
 * @param value  voltage
 * @return       RP_OK - successful, RP_E* - failure
 */
int rp_AIpinGetValue(int unsigned pin, float* value);

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
 * @return       RP_OK - successful, RP_E* - failure
 */
int rp_AOpinGetValue(int unsigned pin, float* value);

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


float rp_CmnCnvCntToV(uint32_t field_len, uint32_t cnts, float adc_max_v);

#ifdef __cplusplus
}
#endif

#endif //__RP_H
