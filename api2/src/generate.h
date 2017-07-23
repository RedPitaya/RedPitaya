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

#define BURST_REPETITIONS_MAX   0x0000ffff
#define BURST_REPETITIONS_MIN   1
#define RP_GEN_REP_INF          0

#define BURST_PERIOD_LEN_MAX    0xffffffff
#define BURST_PERIOD_LEN_MIN    1

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

/** Mode masks */
#define RP_GEN_CFG_BURST_MASK       (1<<0)  ///< if set generator will operate in burst mode, otherwise periodic mode
#define RP_GEN_CFG_BURST_INF_MASK   (1<<1)  ///< if set cfg_bnm will be inf regardless of cfg_bnm setting

typedef enum {
    RP_GEN_MODE_CONTINUOUS,
    RP_GEN_MODE_BURST,
} RP_GEN_MODE;

// Logic generator specific
/** Output enable */
#define RP_GEN_OUT_ALL_MASK  0xffff
#define RP_GEN_OUT_PORT0_MASK 0x00ff ///< lower  8 pins (RP hw 1.1 P_GPIO_PORT)
#define RP_GEN_OUT_PORT1_MASK 0xff00 ///< higher 8 pins (RP hw 1.1 N_GPIO_PORT)

// Analog generator specific
// linear transformation
typedef struct {
    int32_t mul;  // multiplication (amplitude/gain)
    int32_t sum;  // summation (offset)
} ag_regset_t;

typedef struct {
    uint32_t dig_out_en;
    uint32_t dig_openc;
} lg_spec_regset_t;

typedef struct {
    // control register
    uint32_t ctl; ///< control register
    uint32_t trig_mask; ///< global trigger registers
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
    uint32_t cfg_bln;  ///< burst period length (data length + idle length)
    uint32_t cfg_bnm;  ///< burst repetition number
    // status
    uint32_t sts_bln;  ///< burst period length (data length + idle length)
    uint32_t sts_bnm;  ///< burst repetition number
    // specific regs
    union {
        lg_spec_regset_t lg_spec;
        ag_regset_t      ag_spec;
    } gen_spec;
    // empty space
    uint32_t reserved_40[(1<<RP_GEN_CWM)-(0x40>>2)];
    // table
    uint32_t  table [RP_GEN_SIG_SAMPLES];
} asg_regset_t;

/**
* Common functions / controls
*/
int rp_GenOpen(const char *dev, rp_handle_uio_t *handle);
int rp_GenClose(rp_handle_uio_t *handle);

int rp_GenReset(rp_handle_uio_t *handle);
int rp_GenRun(rp_handle_uio_t *handle);
int rp_GenStop(rp_handle_uio_t *handle);
int rp_GenTrigger(rp_handle_uio_t *handle);
int rp_GenIsStopped(rp_handle_uio_t *handle, bool * status);
int rp_GenDefaultSettings();

int rp_GenGlobalTrigSet(rp_handle_uio_t *handle, uint32_t mask);

/**
* Sets channel signal peak to peak amplitude.
* @param amplitude Amplitude of the generated signal. From 0 to max value. Max amplitude is 1
* @param offset DC offset of the generated signal. Max offset is 2.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetLinear(rp_handle_uio_t *handle, float  amplitude, float  offset);
int rp_GenGetLinear(rp_handle_uio_t *handle, float *amplitude, float *offset);

/**
* Sets channel signal frequency.
* @param frequency Frequency of the generated signal in Hz.
* @param phase Phase in degrees of the generated signal. From 0 deg to 180 deg.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/

int rp_GenSetStepOffset(rp_handle_uio_t *handle, uint32_t  stp, uint32_t  off);
int rp_GenGetStepOffset(rp_handle_uio_t *handle, uint32_t *stp, uint32_t *off);

int rp_GenSetFreqPhase(rp_handle_uio_t *handle, double  frequency, double  phase);
int rp_GenGetFreqPhase(rp_handle_uio_t *handle, double *frequency, double *phase);

/** Sets mode */
int rp_GenSetMode(rp_handle_uio_t *handle, RP_GEN_MODE mode);

/**
 * Sets waveform sample rate
 *
 *  @param sample_rate Requested sample rate, on return actual sample rate
 */
int rp_GenSetWaveformSampleRate(rp_handle_uio_t *handle, double * sample_rate);

/*
 *  Sets/Gets waveform
 */
int rp_GenSetWaveform(rp_handle_uio_t *handle, uint16_t *waveform, uint32_t  size);
int rp_GenGetWaveform(rp_handle_uio_t *handle, uint16_t *waveform, uint32_t *size);
int rp_GenSetWaveformUpCountSeq(rp_handle_uio_t *handle, uint32_t size);

/*
 *  Burst mode settings
 */
int rp_GenSetBurstModeRepetitions(rp_handle_uio_t *handle, uint32_t val);
int rp_GenSetBurstModeDataLen(rp_handle_uio_t *handle, uint32_t length);
int rp_GenSetBurstModePeriodLen(rp_handle_uio_t *handle, uint32_t length); ///< TODO: to be fixed

/*
 *  Enables/disable physical logic outputs
 * @param mask Defines which outputs to enable bits [31-16] (PORT1), bits [15-0] (PORT0)
 */
int rp_GenOutputEnable(rp_handle_uio_t *handle, uint32_t  mask);
int rp_GenOutputDisable(rp_handle_uio_t *handle, uint32_t  mask);

/*
 *  Dumps fpga registers and waveform data
 * @param data_len Waveform data length to dump.
 */
int rp_GenFpgaRegDump(rp_handle_uio_t *handle, uint32_t data_len);

#endif //__GENERATE_H
