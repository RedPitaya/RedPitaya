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

#define ADC_BUFFER_SIZE         (16 * 1024)
#define DAC_BUFFER_SIZE         (16 * 1024)

#define ADC_AXI_START           0x1000000
#define ADC_AXI_END             0x1180000

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
    RP_LOW, //!< Low state 1:1
    RP_HIGH //!< High state 1:20
} rp_pinState_t;

/**
 * Type representing pin's high or low state (on/off).
 */
typedef enum {
    OUT_TR_ADC = 0,//!< ADC trigger
    OUT_TR_DAC = 1 //!< DAC trigger
} rp_outTiggerMode_t;

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

typedef enum {
    RP_WAVEFORM_SINE,       //!< Wave form sine
    RP_WAVEFORM_SQUARE,     //!< Wave form square
    RP_WAVEFORM_TRIANGLE,   //!< Wave form triangle
    RP_WAVEFORM_RAMP_UP,    //!< Wave form sawtooth (/|)
    RP_WAVEFORM_RAMP_DOWN,  //!< Wave form reversed sawtooth (|\)
    RP_WAVEFORM_DC,         //!< Wave form dc
    RP_WAVEFORM_PWM,        //!< Wave form pwm
    RP_WAVEFORM_ARBITRARY,  //!< Use defined wave form
    RP_WAVEFORM_DC_NEG,     //!< Wave form negative dc
    RP_WAVEFORM_SWEEP       //!< Wave form sweep
} rp_waveform_t;

typedef enum {
    RP_GEN_MODE_CONTINUOUS, //!< Continuous signal generation
    RP_GEN_MODE_BURST,      //!< Signal is generated N times, wher N is defined with rp_GenBurstCount method
    RP_GEN_MODE_STREAM      //!< User can continuously write data to buffer
} rp_gen_mode_t;

typedef enum {
    RP_GEN_SWEEP_DIR_NORMAL,     //!< Generate sweep signal from start frequency to end frequency
    RP_GEN_SWEEP_DIR_UP_DOWN     //!< Generate sweep signal from start frequency to end frequency and back to start frequency
} rp_gen_sweep_dir_t;

typedef enum {
    RP_GEN_SWEEP_MODE_LINEAR,     //!< Generate sweep signal in linear mode
    RP_GEN_SWEEP_MODE_LOG         //!< Generate sweep signal in log mode
} rp_gen_sweep_mode_t;

typedef enum {
    RP_GEN_TRIG_SRC_INTERNAL = 1,   //!< Internal trigger source
    RP_GEN_TRIG_SRC_EXT_PE   = 2,   //!< External trigger source positive edge
    RP_GEN_TRIG_SRC_EXT_NE   = 3    //!< External trigger source negative edge
} rp_trig_src_t;
typedef enum {
    RP_GAIN_1X = 0,         //!< Set output gain in x1 mode
    RP_GAIN_5X = 1          //!< Set output gain in x5 mode
} rp_gen_gain_t;
/**
 * Type representing Input/Output channels.
 */
typedef enum {
    RP_CH_1 = 0,    //!< Channel A
    RP_CH_2 = 1,    //!< Channel B
    RP_CH_3 = 2,    //!< Channel C
    RP_CH_4 = 3     //!< Channel D
} rp_channel_t;


/**
 * Type representing Input/Output channels in trigger.
 */
typedef enum {
    RP_T_CH_1 = 0,    //!< Channel A
    RP_T_CH_2 = 1,    //!< Channel B
    RP_T_CH_3 = 2,    //!< Channel C
    RP_T_CH_4 = 3,    //!< Channel D
    RP_T_CH_EXT = 4
} rp_channel_trigger_t;

/**
 * The type represents the names of the coefficients in the filter.
 */
typedef enum {
    AA,    //!< AA
    BB,    //!< BB
    PP,    //!< PP
    KK     //!< KK
} rp_eq_filter_cof_t;

typedef struct
{
    uint32_t size;
    int16_t  *ch_i[4];
    double   *ch_d[4];
    float    *ch_f[4];
} buffers_t;


/**
 * Type representing decimation used at acquiring signal.
 */
typedef enum {
    RP_DEC_1     = 1,       //!< Decimation 1
    RP_DEC_2     = 2,       //!< Decimation 2
    RP_DEC_4     = 4,       //!< Decimation 4
    RP_DEC_8     = 8,       //!< Decimation 8
    RP_DEC_16    = 16,      //!< Decimation 16
    RP_DEC_32    = 32,      //!< Decimation 32
    RP_DEC_64    = 64,      //!< Decimation 64
    RP_DEC_128   = 128,     //!< Decimation 128
    RP_DEC_256   = 256,     //!< Decimation 256
    RP_DEC_512   = 512,     //!< Decimation 512
    RP_DEC_1024  = 1024,    //!< Decimation 1024
    RP_DEC_2048  = 2048,    //!< Decimation 2048
    RP_DEC_4096  = 4096,    //!< Decimation 4096
    RP_DEC_8192  = 8192,    //!< Decimation 8192
    RP_DEC_16384 = 16384,   //!< Decimation 16384
    RP_DEC_32768 = 32768,   //!< Decimation 32768
    RP_DEC_65536 = 65536    //!< Decimation 65536
} rp_acq_decimation_t;

typedef enum {
    RP_DC = 0,
    RP_AC = 1
} rp_acq_ac_dc_mode_t;
/**
 * Type representing different trigger sources used at acquiring signal.
 */
typedef enum {
    RP_TRIG_SRC_DISABLED = 0, //!< Trigger is disabled
    RP_TRIG_SRC_NOW      = 1, //!< Trigger triggered now (immediately)
    RP_TRIG_SRC_CHA_PE   = 2, //!< Trigger set to Channel A threshold positive edge
    RP_TRIG_SRC_CHA_NE   = 3, //!< Trigger set to Channel A threshold negative edge
    RP_TRIG_SRC_CHB_PE   = 4, //!< Trigger set to Channel B threshold positive edge
    RP_TRIG_SRC_CHB_NE   = 5, //!< Trigger set to Channel B threshold negative edge
    RP_TRIG_SRC_EXT_PE   = 6, //!< Trigger set to external trigger positive edge (DIO0_P pin)
    RP_TRIG_SRC_EXT_NE   = 7, //!< Trigger set to external trigger negative edge (DIO0_P pin)
    RP_TRIG_SRC_AWG_PE   = 8, //!< Trigger set to arbitrary wave generator application positive edge
    RP_TRIG_SRC_AWG_NE   = 9, //!< Trigger set to arbitrary wave generator application negative edge
    RP_TRIG_SRC_CHC_PE   = 10,//!< Trigger set to Channel B threshold positive edge
    RP_TRIG_SRC_CHC_NE   = 11,//!< Trigger set to Channel B threshold negative edge
    RP_TRIG_SRC_CHD_PE   = 12,//!< Trigger set to Channel B threshold positive edge
    RP_TRIG_SRC_CHD_NE   = 13 //!< Trigger set to Channel B threshold negative edge
} rp_acq_trig_src_t;


/**
 * Type representing different trigger states.
 */
typedef enum {
    RP_TRIG_STATE_TRIGGERED, //!< Trigger is triggered/disabled
    RP_TRIG_STATE_WAITING,   //!< Trigger is set up and waiting (to be triggered)
} rp_acq_trig_state_t;

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
* Sets digital pins to default values. Pins DIO1_P - DIO7_P, RP_DIO0_N - RP_DIO7_N are set al OUTPUT and to LOW. LEDs are set to LOW/OFF
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
 * Enables clock and trigger sync over SATA daisy chain connectors.
 * Once the primary board will be triggered, the trigger will be forwarded to the secondary board over
 * the SATA connector where the trigger can be detected using rp_GenTriggerSource with EXT_NE selector.
 * Noticed that the trigger that is received over SATA is ORed with the external trigger from GPIO.
 *
 * @param enable  Turns on the mode.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_SetEnableDaisyChainSync(bool enable);

/**
 * Returns the current state of the SATA daisy chain mode.
 * @param status  Current state.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GetEnableDaisyChainSync(bool *status);

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


///@}
/** @name Acquire
 */
///@{

/**
 * Enables continous acquirement even after trigger has happened.
 * @param enable True for enabling and false disabling
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetArmKeep(bool enable);

/**
 * Gets status of continous acquirement even after trigger has happened.
 * @param state Returns status
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetArmKeep(bool* state);

/**
 * Indicates whether the ADC buffer was full of data. The length of the buffer is determined by the delay. By default, the delay is half the buffer.
 * @param state Returns status
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetBufferFillState(bool* state);

/**
 * Indicates whether the ADC AXI buffer was full of data.
 * @param channel Channel index
 * @param state Returns status
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetBufferFillState(rp_channel_t channel, bool* state);

/**
 * Sets the decimation used at acquiring signal. There is only a set of pre-defined decimation
 * values which can be specified. See the #rp_acq_decimation_t enum values.
 * @param decimation Specify one of pre-defined decimation values
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetDecimation(rp_acq_decimation_t decimation);

/**
 * Gets the decimation used at acquiring signal. There is only a set of pre-defined decimation
 * values which can be specified. See the #rp_acq_decimation_t enum values.
 * @param decimation Returns one of pre-defined decimation values which is currently set.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDecimation(rp_acq_decimation_t* decimation);

/**
 * Convert factor to decimation used at acquiring signal. There is only a get of pre-defined decimation
 * values which can be specified. See the #rp_acq_decimation_t enum values.
 * @param factor Decimation factor.
 * @param decimation Returns one of pre-defined decimation values which is currently set.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqConvertFactorToDecimation(uint32_t factor,rp_acq_decimation_t* decimation);

/**
 * Sets the decimation used at acquiring signal.
 * You can specify values in the range (1,2,4,8,16-65536)
 * @param decimation Decimation values
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetDecimationFactor(uint32_t decimation);

/**
 * Sets the decimation used at acquiring signal for AXI.
 * You can specify values in the range (1,2,4,8,16-65536)
 * @param channel Channel index
 * @param decimation Decimation values
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiSetDecimationFactor(rp_channel_t channel, uint32_t decimation);

/**
 * Gets the decimation factor used at acquiring signal in a numerical form. Although this method returns an integer
 * value representing the current factor of the decimation, there is only a set of pre-defined decimation
 * factor values which can be returned. See the #rp_acq_decimation_t enum values.
 * @param decimation Returns decimation factor value which is currently set.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDecimationFactor(uint32_t* decimation);

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

/**
 * Sets the trigger source used at acquiring signal. When acquiring is started,
 * the FPGA waits for the trigger condition on the specified source and when the condition is met, it
 * starts writing the signal to the buffer.
 * @param source Trigger source.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerSrc(rp_acq_trig_src_t source);

/**
 * Gets the trigger source used at acquiring signal. When acquiring is started,
 * the FPGA waits for the trigger condition on the specified source and when the condition is met, it
 * starts writing the signal to the buffer.
 * @param source Currently set trigger source.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerSrc(rp_acq_trig_src_t* source);

/**
 * Returns the trigger state. Either it is waiting for a trigger to happen, or it has already been triggered.
 * By default it is in the triggered state, which is treated the same as disabled.
 * @param state Trigger state
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerState(rp_acq_trig_state_t* state);

/**
 * Sets the number of decimated data after trigger written into memory.
 * @param decimated_data_num Number of decimated data. It must not be higher than the ADC buffer size.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerDelay(int32_t decimated_data_num);

/**
 * Sets the number of decimated data after trigger written into memory.
 * @param channel Channel index
 * @param decimated_data_num Number of decimated data. It must not be higher than the ADC buffer size.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiSetTriggerDelay(rp_channel_t channel, int32_t decimated_data_num);

/**
 * Returns current number of decimated data after trigger written into memory.
 * @param decimated_data_num Number of decimated data.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerDelay(int32_t* decimated_data_num);

/**
 * Sets the amount of decimated data in nanoseconds after trigger written into memory.
 * @param time_ns Time in nanoseconds. Number of ADC samples within the specified
 * time must not be higher than the ADC buffer size.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerDelayNs(int64_t time_ns);

/**
 * Returns the current amount of decimated data in nanoseconds after trigger written into memory.
 * @param time_ns Time in nanoseconds.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerDelayNs(int64_t* time_ns);

/**
 * Returns the number of valid data ponts before trigger.
 * @param time_ns number of data points.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetPreTriggerCounter(uint32_t* value);

/**
 * Sets the trigger threshold value in volts. Makes the trigger when ADC value crosses this value.
 * @param voltage Threshold value for the channel
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerLevel(rp_channel_trigger_t channel, float voltage);

/**
 * Gets currently set trigger threshold value in volts
 * @param voltage Current threshold value for the channel
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerLevel(rp_channel_trigger_t channel, float* voltage);

/**
 * Sets the trigger threshold hysteresis value in volts.
 * Value must be outside to enable the trigger again.
 * @param voltage Threshold hysteresis value for the channel
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerHyst(float voltage);

/**
 * Gets currently set trigger threshold hysteresis value in volts
 * @param voltage Current threshold hysteresis value for the channel
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerHyst(float* voltage);

/**
 * Sets the acquire gain state. The gain should be set to the same value as it is set on the Red Pitaya
 * hardware by the LV/HV gain jumpers. LV = 1V; HV = 20V.
 * @param channel Channel A or B
 * @param state High or Low state
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetGain(rp_channel_t channel, rp_pinState_t state);

/**
 * Returns the currently set acquire gain state in the library. It may not be set to the same value as
 * it is set on the Red Pitaya hardware by the LV/HV gain jumpers. LV = 1V; HV = 20V.
 * @param channel Channel A or B
 * @param state Currently set High or Low state in the library.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetGain(rp_channel_t channel, rp_pinState_t* state);

/**
 * Returns the currently set acquire gain in the library. It may not be set to the same value as
 * it is set on the Red Pitaya hardware by the LV/HV gain jumpers. Returns value in Volts.
 * @param channel Channel A or B
 * @param voltage Currently set gain in the library. 1.0 or 20.0 Volts
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetGainV(rp_channel_t channel, float* voltage);

/**
 * Returns current position of ADC write pointer.
 * @param pos Write pointer position
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetWritePointer(uint32_t* pos);

/**
 * Returns current position of AXI ADC write pointer.
 * @param channel Channel index
 * @param pos Write pointer position
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetWritePointer(rp_channel_t channel, uint32_t* pos);

/**
 * Returns position of ADC write pointer at time when trigger arrived.
 * @param pos Write pointer position
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetWritePointerAtTrig(uint32_t* pos);

/**
 * Returns position of AXI ADC write pointer at time when trigger arrived.
 * @param channel Channel index
 * @param pos Write pointer position
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetWritePointerAtTrig(rp_channel_t channel, uint32_t* pos);

/**
 * Starts the acquire. Signals coming from the input channels are acquired and written into memory.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqStart();

/**
* Stops the acquire.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_AcqStop();

/**
 * Resets the acquire writing state machine and set by default all parameters.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqReset();


/**
 * Resets the acquire writing state machine.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqResetFpga();

/**
 * Normalizes the ADC buffer position. Returns the modulo operation of ADC buffer size...
 * @param pos position to be normalized
 * @return Normalized position (pos % ADC_BUFFER_SIZE)
 */
uint32_t rp_AcqGetNormalizedDataPos(uint32_t pos);

/**
 * Returns the ADC buffer in raw units from start to end position.
 *
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param start_pos Starting position of the ADC buffer to retrieve.
 * @param end_pos Ending position of the ADC buffer to retrieve.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @param buffer_size Length of input buffer. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataPosRaw(rp_channel_t channel, uint32_t start_pos, uint32_t end_pos, int16_t* buffer, uint32_t *buffer_size);

/**
 * Returns the ADC buffer in Volt units from start to end position.
 *
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param start_pos Starting position of the ADC buffer to retrieve.
 * @param end_pos Ending position of the ADC buffer to retrieve.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @param buffer_size Length of input buffer. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataPosV(rp_channel_t channel, uint32_t start_pos, uint32_t end_pos, float* buffer, uint32_t *buffer_size);

/**
 * Sets the AXI enable state.
 * @param channel Channel index
 * @param enable Enable state
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiEnable(rp_channel_t channel, bool enable);

/**
 * Returns the ADC buffer in raw units from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param pos Starting position of the ADC buffer to retrieve.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataRaw(rp_channel_t channel,  uint32_t pos, uint32_t* size, int16_t* buffer);

/**
 * Returns the AXI ADC buffer in raw units from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param pos Starting position of the ADC buffer to retrieve.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetDataRaw(rp_channel_t channel,  uint32_t pos, uint32_t* size, int16_t* buffer);

/**
 * Returns the ADC buffer in raw units from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * Data not calibrated
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param pos Starting position of the ADC buffer to retrieve.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataRawV2(uint32_t pos, buffers_t *out);

/**
 * Returns the ADC buffer in raw units from the oldest sample to the newest one.
 * Output buffer must be at least 'size' long.
 * CAUTION: Use this method only when write pointer has stopped (Trigger happened and writing stopped).
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetOldestDataRaw(rp_channel_t channel, uint32_t* size, int16_t* buffer);

/**
 * Returns the latest ADC buffer samples in raw units.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetLatestDataRaw(rp_channel_t channel, uint32_t* size, int16_t* buffer);

/**
 * Returns the ADC buffer in Volt units from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param pos Starting position of the ADC buffer to retrieve
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataV(rp_channel_t channel, uint32_t pos, uint32_t* size, float* buffer);

/**
 * Returns the AXI ADC buffer in Volt units from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param pos Starting position of the ADC buffer to retrieve
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetDataV(rp_channel_t channel, uint32_t pos, uint32_t* size, float* buffer);

/**
 * Returns the ADC buffer in Volt units from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * @param pos Starting position of the ADC buffer to retrieve
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer1 The output buffer gets filled with the selected part of the ADC buffer for channel 1.
 * @param buffer2 The output buffer gets filled with the selected part of the ADC buffer for channel 2.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataV2(uint32_t pos, buffers_t *out);


/**
 * Returns the ADC buffer in Volt units from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * @param pos Starting position of the ADC buffer to retrieve
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer1 The output buffer gets filled with the selected part of the ADC buffer for channel 1.
 * @param buffer2 The output buffer gets filled with the selected part of the ADC buffer for channel 2.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDataV2D(uint32_t pos, buffers_t *out);

/**
 * Returns the ADC buffer in Volt units from the oldest sample to the newest one.
 * Output buffer must be at least 'size' long.
 * CAUTION: Use this method only when write pointer has stopped (Trigger happened and writing stopped).
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetOldestDataV(rp_channel_t channel, uint32_t* size, float* buffer);

/**
 * Returns the latest ADC buffer samples in Volt units.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetLatestDataV(rp_channel_t channel, uint32_t* size, float* buffer);

/**
 * Returns the ADC buffer size in samples.
 *
 * @param size Size of the ADC buffer in samples.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetBufSize(uint32_t* size);

/**
 * Sets the AXI ADC buffer address and size in samples.
 *
 * @param channel Channel A or B for which we want to set the ADC buffer size.
 * @param address Address of the ADC buffer.
 * @param size Size of the ADC buffer in samples.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiSetBuffer(rp_channel_t channel, uint32_t address, uint32_t size);

/**
* Sets the current calibration values from temporary memory to the FPGA filter
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_AcqUpdateAcqFilter(rp_channel_t channel);

/**
* Sets the current calibration values from temporary memory to the FPGA filter
* @param channel Channel A or B for which we want to retrieve the ADC buffer.
* @param coef_aa Return AA coefficient.
* @param coef_bb Return BB coefficient.
* @param coef_kk Return KK coefficient.
* @param coef_pp Return PP coefficient.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_AcqGetFilterCalibValue(rp_channel_t channel,uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp);

/**
 * Sets ext. trigger debouncer for acquisition in Us (Value must be positive).
 * @param value Value in microseconds.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetExtTriggerDebouncerUs(double value);

/**
 * Gets ext. trigger debouncer for acquisition in Us
 * @param value Return value in microseconds.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetExtTriggerDebouncerUs(double *value);

/**
* Sets the AC / DC modes for input.
* Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B.
* @param mode Set current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_AcqSetAC_DC(rp_channel_t channel,rp_acq_ac_dc_mode_t mode);

/**
* Get the AC / DC modes for input.
* Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B.
* @param status Set current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_AcqGetAC_DC(rp_channel_t channel,rp_acq_ac_dc_mode_t *status);

///@}
/** @name Generate
*/
///@{


/**
* Sets generate to default values.
*/
int rp_GenReset();

/**
* Enables output
* @param channel Channel A or B which we want to enable
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenOutEnable(rp_channel_t channel);

/**
* Runs/Stop two channels synchronously
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenOutEnableSync(bool enable);

/**
* Disables output
* @param channel Channel A or B which we want to disable
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenOutDisable(rp_channel_t channel);

/**
* Gets value true if channel is enabled otherwise return false.
* @param channel Channel A or B.
* @param value Pointer where value will be returned
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenOutIsEnabled(rp_channel_t channel, bool *value);

/**
* Sets channel signal peak to peak amplitude.
* @param channel Channel A or B for witch we want to set amplitude
* @param amplitude Amplitude of the generated signal. From 0 to max value. Max amplitude is 1
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenAmp(rp_channel_t channel, float amplitude);

/**
* Gets channel signal peak to peak amplitude.
* @param channel Channel A or B for witch we want to get amplitude.
* @param amplitude Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetAmp(rp_channel_t channel, float *amplitude);

/**
* Sets DC offset of the signal. signal = signal + DC_offset.
* @param channel Channel A or B for witch we want to set DC offset.
* @param offset DC offset of the generated signal. Max offset is 2.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenOffset(rp_channel_t channel, float offset);

/**
* Gets DC offset of the signal.
* @param channel Channel A or B for witch we want to get amplitude.
* @param offset Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetOffset(rp_channel_t channel, float *offset);

/**
* Sets channel signal frequency.
* @param channel Channel A or B for witch we want to set frequency.
* @param frequency Frequency of the generated signal in Hz.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenFreq(rp_channel_t channel, float frequency);

/**
* Sets channel signal frequency in fpga without reset generator and rebuild signal.
* @param channel Channel A or B for witch we want to set frequency.
* @param frequency Frequency of the generated signal in Hz.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenFreqDirect(rp_channel_t channel, float frequency);

/**
* Gets channel signal frequency.
* @param channel Channel A or B for witch we want to get frequency.
* @param frequency Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetFreq(rp_channel_t channel, float *frequency);


/**
* Sets channel sweep signal start frequency.
* @param channel Channel A or B for witch we want to set frequency.
* @param frequency Frequency of the generated signal in Hz.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSweepStartFreq(rp_channel_t channel, float frequency);

/**
* Gets channel sweep signal start frequency.
* @param channel Channel A or B for witch we want to get frequency.
* @param frequency Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetSweepStartFreq(rp_channel_t channel, float *frequency);

/**
* Sets channel sweep signal end frequency.
* @param channel Channel A or B for witch we want to set frequency.
* @param frequency Frequency of the generated signal in Hz.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSweepEndFreq(rp_channel_t channel, float frequency);

/**
* Gets channel sweep signal end frequency.
* @param channel Channel A or B for witch we want to get frequency.
* @param frequency Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetSweepEndFreq(rp_channel_t channel, float *frequency);

/**
* Sets channel signal phase. This shifts the signal in time.
* @param channel Channel A or B for witch we want to set phase.
* @param phase Phase in degrees of the generated signal. From 0 deg to 180 deg.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenPhase(rp_channel_t channel, float phase);

/**
* Gets channel signal phase.
* @param channel Channel A or B for witch we want to get phase.
* @param phase Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetPhase(rp_channel_t channel, float *phase);

/**
* Sets channel signal waveform. This determines how the signal looks.
* @param channel Channel A or B for witch we want to set waveform type.
* @param form Wave form of the generated signal [SINE, SQUARE, TRIANGLE, SAWTOOTH, PWM, DC, ARBITRARY, SWEEP].
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenWaveform(rp_channel_t channel, rp_waveform_t type);

/**
* Gets channel signal waveform.
* @param channel Channel A or B for witch we want to get waveform.
* @param type Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetWaveform(rp_channel_t channel, rp_waveform_t *type);

/**
* Sets the generation mode for the sweep signal.
* @param channel Channel A or B for witch we want to set waveform type.
* @param mode Mode of the generated signal [RP_GEN_SWEEP_MODE_LINEAR, RP_GEN_SWEEP_MODE_LOG].
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSweepMode(rp_channel_t channel, rp_gen_sweep_mode_t mode);

/**
* Gets the generation mode for the sweep signal.
* @param channel Channel A or B for witch we want to get waveform.
* @param mode Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetSweepMode(rp_channel_t channel, rp_gen_sweep_mode_t *mode);


/**
* Sets the direction of frequency change for sweep.
* @param channel Channel A or B for witch we want to set waveform type.
* @param mode Wave form of the generated signal [RP_GEN_SWEEP_DIR_NORMAL, RP_GEN_SWEEP_DIR_UP_DOWN].
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSweepDir(rp_channel_t channel, rp_gen_sweep_dir_t mode);

/**
* Gets the direction of frequency change for sweep.
* @param channel Channel A or B for witch we want to get waveform.
* @param mode Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetSweepDir(rp_channel_t channel, rp_gen_sweep_dir_t *mode);

/**
* Sets user defined waveform.
* @param channel Channel A or B for witch we want to set waveform.
* @param waveform Use defined wave form, where min is -1V an max is 1V.
* @param length Length of waveform.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenArbWaveform(rp_channel_t channel, float *waveform, uint32_t length);

/**
* Gets user defined waveform.
* @param channel Channel A or B for witch we want to get waveform.
* @param waveform Pointer where waveform will be returned.
* @param length Pointer where waveform length will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetArbWaveform(rp_channel_t channel, float *waveform, uint32_t *length);

/**
* Sets duty cycle of PWM signal.
* @param channel Channel A or B for witch we want to set duty cycle.
* @param ratio Ratio betwen the time when signal in HIGH vs the time when signal is LOW.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenDutyCycle(rp_channel_t channel, float ratio);

/**
* Sets rise time of square signal.
* @param channel Channel A or B for witch we want to set rise time.
* @param time Rise time in microseconds.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/

int rp_GenRiseTime(rp_channel_t channel, float time);

/**
 * Sets fall time of square signal.
 * @param channel Channel A or B for witch we want to set fall time.
 * @param time Fall time in microseconds.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */

int rp_GenFallTime(rp_channel_t channel, float time);

/**
* Gets duty cycle of PWM signal.
* @param channel Channel A or B for witch we want to get duty cycle.
* @param ratio Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetDutyCycle(rp_channel_t channel, float *ratio);

/**
* Sets generation mode.
* @param channel Channel A or B for witch we want to set generation mode.
* @param mode Type of signal generation (CONTINUOUS, BURST, STREAM).
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenMode(rp_channel_t channel, rp_gen_mode_t mode);

/**
* Gets generation mode.
* @param channel Channel A or B for witch we want to get generation mode.
* @param mode Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetMode(rp_channel_t channel, rp_gen_mode_t *mode);

/**
* Sets number of generated waveforms in a burst.
* @param channel Channel A or B for witch we want to set number of generated waveforms in a burst.
* @param num Number of generated waveforms. If -1 a continuous signal will be generated.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenBurstCount(rp_channel_t channel, int num);

/**
* Gets number of generated waveforms in a burst.
* @param channel Channel A or B for witch we want to get number of generated waveforms in a burst.
* @param num Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetBurstCount(rp_channel_t channel, int *num);

/**
* Sets the value to be set at the end of the generated signal in burst mode.
* @param channel Channel A or B for witch we want to set number of generated waveforms in a burst.
* @param amplitude Amplitude level at the end of the signal (Volt).
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenBurstLastValue(rp_channel_t channel, float amlitude);

/**
* Gets the value to be set at the end of the generated signal in burst mode.
* @param channel Channel A or B for witch we want to get number of generated waveforms in a burst.
* @param amplitude Amplitude where value will be returned (Volt).
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetBurstLastValue(rp_channel_t channel, float *amlitude);

/**
* The level of which is set by the generator after the outputs are turned on before the signal is generated.
* @param channel Channel A or B for witch we want to set number of generated waveforms in a burst.
* @param amplitude Amplitude level at the end of the signal (Volt).
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetInitGenValue(rp_channel_t channel, float amlitude);

/**
* Gets the value of the initial signal level.
* @param channel Channel A or B for witch we want to get number of generated waveforms in a burst.
* @param amplitude Amplitude where value will be returned (Volt).
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetInitGenValue(rp_channel_t channel, float *amlitude);

/**
* Sets number of burst repetitions. This determines how many bursts will be generated.
* @param channel Channel A or B for witch we want to set number of burst repetitions.
* @param repetitions Number of generated bursts. If 0x10000, infinite bursts will be generated.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenBurstRepetitions(rp_channel_t channel, int repetitions);

/**
* Gets number of burst repetitions.
* @param channel Channel A or B for witch we want to get number of burst repetitions.
* @param repetitions Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetBurstRepetitions(rp_channel_t channel, int *repetitions);

/**
* Sets the time/period of one burst in micro seconds. Period must be equal or greater then the time of one burst.
* If it is greater than the difference will be the delay between two consequential bursts.
* @param channel Channel A or B for witch we want to set burst period.
* @param period Time in micro seconds.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenBurstPeriod(rp_channel_t channel, uint32_t period);

/**
* Gets the period of one burst in micro seconds.
* @param channel Channel A or B for witch we want to get burst period.
* @param period Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetBurstPeriod(rp_channel_t channel, uint32_t *period);

/**
* Sets trigger source.
* @param channel Channel A or B for witch we want to set trigger source.
* @param src Trigger source (INTERNAL, EXTERNAL_PE, EXTERNAL_NE, GATED_BURST).
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenTriggerSource(rp_channel_t channel, rp_trig_src_t src);

/**
* Gets trigger source.
* @param channel Channel A or B for witch we want to get burst period.
* @param src Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetTriggerSource(rp_channel_t channel, rp_trig_src_t *src);

/**
* The generator is reset on both channels.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSynchronise();

/**
* The generator is reset on channels.
* @param channel Channel A or B
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenResetTrigger(rp_channel_t channel);

/**
* Emit trigger for selected channel
* @param channel Channel A or B
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/

int rp_GenTriggerOnly(rp_channel_t channel);

/**
* Sets the DAC protection mode from overheating. Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B for witch we want to set protection.
* @param enable Flag enabling protection mode.total
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_SetEnableTempProtection(rp_channel_t channel, bool enable);

/**
* Get status of DAC protection mode from overheating. Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B for witch we want to set protection.
* @param enable Flag return current status.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GetEnableTempProtection(rp_channel_t channel, bool *enable);

/**
* Resets the flag indicating that the DAC is overheated. Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B.
* @param status  New status for latch trigger.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_SetLatchTempAlarm(rp_channel_t channel, bool status);

/**
* Returns the status that there was an overheat. Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B.
* @param status  State of latch trigger.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GetLatchTempAlarm(rp_channel_t channel, bool *status);

/**
* Returns the current DAC overheat status in real time. Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B.
* @param status  Get current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GetRuntimeTempAlarm(rp_channel_t channel, bool *status);

/**
* Sets the gain modes for output.
* Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B.
* @param mode Set current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetGainOut(rp_channel_t channel,rp_gen_gain_t mode);

/**
* Get the gain modes for output.
* Only works with Redpitaya 250-12 otherwise returns RP_NOTS
* @param channel Channel A or B.
* @param status Set current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetGainOut(rp_channel_t channel,rp_gen_gain_t *status);

/**
 * Sets ext. trigger debouncer for generation in Us (Value must be positive).
 * @param value Value in microseconds.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GenSetExtTriggerDebouncerUs(double value);

/**
 * Gets ext. trigger debouncer for generation in Us
 * @param value Return value in microseconds.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GenGetExtTriggerDebouncerUs(double *value);

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
