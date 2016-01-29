/**
 * $Id: $
 *
 * @brief Red Pitaya library Generate module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */
#ifndef __GENERATE_H
#define __GENERATE_H

#include "common.h"

#define LEVEL_MAX               1.0         // V
#define AMPLITUDE_MAX           1.0         // V
#define OFFSET_MAX              2.0         // V
#define FREQUENCY_MIN           0           // Hz
#define FREQUENCY_MAX           62.5e6      // Hz
#define PHASE_MIN              -360         // deg
#define PHASE_MAX               360         // deg
#define BURST_COUNT_MIN        -1
#define BURST_COUNT_MAX         50000
#define BURST_REPETITIONS_MIN   1
#define BURST_REPETITIONS_MAX   50000
#define BURST_PERIOD_MIN        1           // us
#define BURST_PERIOD_MAX        500000000   // us

// Base Generate address
#define GENERATE_BASE_SIZE      0x00040000

// sampling rate
#define RP_GEN_SR   125000000
// number of trigger options
#define RP_GEN_TWS  RP_MNG+2
// ASG counter width (mantisa and fraction)
#define RP_GEN_CWM  14
#define RP_GEN_CWF  16
// linear
#define RP_GEN_DWO  14
#define RP_GEN_DWM  16
#define RP_GEN_DWS  14

#define RP_GEN_SIG_SAMPLES (1<<14) ///< 16384


typedef enum {
    RP_GEN_MODE_CONTINUOUS,
    RP_GEN_MODE_BURST,
} RP_GEN_MODE;

// linear transformation
typedef struct {
    int32_t mul;  // multiplication (amplitude/gain)
    int32_t sum;  // summation (offset)
} linear_regset_t;

/** Digital gen. registers */
typedef struct {
    uint32_t dig_out_en; ///< this enables digital outputs
} rp_dig_out_t;

#define RP_GEN_REP_INF 0

#define RP_GEN_CFG_BURST_MASK (1<<0)      ///< if set generator will operate in burst mode, otherwise periodic mode
#define RP_GEN_CFG_BURST_INF_MASK (1<<1)  ///< if set cfg_bnm will be inf regardless of cfg_bnm setting

typedef struct {
    // control register
	rp_ctl_regset_t ctl; ///< control register
    rp_global_trig_regset_t gtrg; ///< global trigger registers
    uint32_t reserved_08;
    uint32_t reserved_0c;
    // configuration that is (used only for periodic mode)
    uint32_t cfg_siz; ///< len = reg + 1
    uint32_t cfg_off; ///< defines offset from which generation will started
    uint32_t cfg_stp; ///< fixed point 16.16 value
    uint32_t reserved_1c;
    // burst mode
    uint32_t cfg_bst;  ///< enables & configures burst mode
    uint32_t cfg_bdl;  ///< burst data length
    uint32_t cfg_bil;  ///< burst idle length
    uint32_t cfg_bnm;  ///< burst repetition number

    rp_dig_out_t dig;  ///< output enable
    // empty space
    uint32_t reserved_30 [(1<<RP_GEN_CWM)-0x30];
    // table
    uint32_t  table [RP_GEN_SIG_SAMPLES];
} asg_regset_t;

// ??
typedef struct {
    // ASG registers
    asg_regset_t    asg;
    // linear transformation
    linear_regset_t lin;
} gen_regset_t;

/**
* Common functions / controls
*/
int rp_GenOpen(char *dev, rp_handle_uio_t *handle);
int rp_GenClose(rp_handle_uio_t *handle);

int rp_GenReset(rp_handle_uio_t *handle);
int rp_GenRun(rp_handle_uio_t *handle);
int rp_GenStop(rp_handle_uio_t *handle);
int rp_GenTrigger(rp_handle_uio_t *handle);
int rp_GenIsStopped(rp_handle_uio_t *handle, bool * status);
int rp_GenDefaultSettings();

int rp_GenGlobalTrigEnable(rp_handle_uio_t *handle, uint32_t a_mask);
int rp_GenGlobalTrigDisable(rp_handle_uio_t *handle, uint32_t a_mask);

/**
* Sets channel signal peak to peak amplitude.
* @param amplitude Amplitude of the generated signal. From 0 to max value. Max amplitude is 1
* @param offset DC offset of the generated signal. Max offset is 2.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
//int rp_GenSetLinear(rp_handle_uio_t *handle, float  amplitude, float  offset);
//int rp_GenGetLinear(rp_handle_uio_t *handle, float *amplitude, float *offset);

/**
* Sets channel signal frequency.
* @param frequency Frequency of the generated signal in Hz.
* @param phase Phase in degrees of the generated signal. From 0 deg to 180 deg.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetFreqPhase(rp_handle_uio_t *handle, double  frequency, double  phase);
int rp_GenGetFreqPhase(rp_handle_uio_t *handle, double *frequency, double *phase);

/**
* Sets channel signal waveform. This determines how the signal looks.
* @param channel Channel A or B for witch we want to set waveform type.
* @param form Wave form of the generated signal [SINE, SQUARE, TRIANGLE, SAWTOOTH, PWM, DC, ARBITRARY].
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/


/**
* Sets number of generated waveforms in a burst.
* @param num Number of generated waveforms. If -1 a continuous signal will be generated.
* @param repetitions Number of generated bursts. If -1, infinite bursts will be generated.
* @param period Time in micro seconds.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
//int rp_GenSetBurst(rp_handle_uio_t *handle, uint32_t  count, uint32_t  repetitions, uint32_t  delay);
//int rp_GenGetBurst(rp_handle_uio_t *handle, uint32_t *count, uint32_t *repetitions, uint32_t *delay);

/** Set mode */
int rp_GenSetMode(rp_handle_uio_t *handle, RP_GEN_MODE a_mode);

/** Set frequency */
int rp_GenSetSampleRate(rp_handle_uio_t *handle, double a_freq);

/** Set waveform */
int rp_GenSetWaveform(rp_handle_uio_t *handle, uint16_t *waveform, uint32_t  size);
int rp_GenGetWaveform(rp_handle_uio_t *handle, uint16_t *waveform, uint32_t *size);
int rp_GenSetWaveformUpCountSeq(rp_handle_uio_t *handle, uint32_t size);

/** Burst mode specific */
int rp_GenSetBurstModeRepetitions(rp_handle_uio_t *handle, uint32_t a_val);
int rp_GenSetBurstModeBurstDataLen(rp_handle_uio_t *handle, uint32_t length);
int rp_GenSetBurstModeIdle(rp_handle_uio_t *handle, uint32_t a_val);

/** Output enable */
#define RP_GEN_OUT_EN_ALL_MASK 	 0xffff
#define RP_GEN_OUT_EN_PORT0_MASK 0x00ff ///< lower  8 pins (RP hw 1.1 P_GPIO_PORT)
#define RP_GEN_OUT_EN_PORT1_MASK 0xff00 ///< higher 8 pins (RP hw 1.1 N_GPIO_PORT)

int rp_GenOutputEnable(rp_handle_uio_t *handle, uint32_t  mask);
int rp_GenOutputDisable(rp_handle_uio_t *handle, uint32_t  mask);

int rp_GenFpgaRegDump(rp_handle_uio_t *handle);

#endif //__GENERATE_H
