/**
 * $Id: $
 *
 * @file rp_enums.h
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

#ifndef __RP_ENUMS_H
#define __RP_ENUMS_H

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
    uint8_t channels;
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

#endif //__RP_ENUMS_H