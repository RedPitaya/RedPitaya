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
#define GENERATE_BASE_SIZE      0x00010000

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

// linear transformation
typedef struct {
    int32_t mul;  // multiplication (amplitude/gain)
    int32_t sum;  // summation (offset)
} linear_regset_t;

typedef struct {
    // control register
    uint32_t ctl_sys;
        // rst :1;
        // trg :1;
    uint32_t cfg_trg;
    uint32_t reserved_08;
    uint32_t reserved_0c;
    // configuration
    uint32_t cfg_siz;
    uint32_t cfg_off;
    uint32_t cfg_stp;
    uint32_t reserved_1c;
    // burst mode
    uint32_t cfg_bst;  // burst mode
        // ben :1;
        // inf :1;
    uint32_t cfg_bdl;  // burst data length
    uint32_t cfg_bil;  // burst idle length
    uint32_t cfg_bnm;  // burst repetition number
    // empty space
    uint32_t reserved_30 [(1<<RP_GEN_CWM)-0x30];
    // table
    int32_t  table [(1<<RP_GEN_CWM)];
} asg_regset_t;

typedef struct {
    // ASG registers
    asg_regset_t    asg;
    // linear transformation
    linear_regset_t lin;
} gen_regset_t;

int rp_GenInit(char *dev, rp_handle_uio_t *handle);
int rp_GenRelease(rp_handle_uio_t *handle);

/**
* Sets generate to default values.
*/
int rp_GenReset();

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
int rp_GenSetFreqPhase(rp_handle_uio_t *handle, double  frequency, double  phase);
int rp_GenGetFreqPhase(rp_handle_uio_t *handle, double *frequency, double *phase);

/**
* Sets channel signal waveform. This determines how the signal looks.
* @param channel Channel A or B for witch we want to set waveform type.
* @param form Wave form of the generated signal [SINE, SQUARE, TRIANGLE, SAWTOOTH, PWM, DC, ARBITRARY].
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetWaveform(rp_handle_uio_t *handle, int16_t *waveform, uint32_t  length);
int rp_GenGetWaveform(rp_handle_uio_t *handle, int16_t *waveform, uint32_t *length);

/**
* Sets generation mode.
* @param channel Channel A or B for witch we want to set generation mode.
* @param mode Type of signal generation (CONTINUOUS, BURST, STREAM).
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetMode(rp_handle_uio_t *handle, uint32_t  mode);
int rp_GenGetMode(rp_handle_uio_t *handle, uint32_t *mode);

/**
* Sets number of generated waveforms in a burst.
* @param num Number of generated waveforms. If -1 a continuous signal will be generated.
* @param repetitions Number of generated bursts. If -1, infinite bursts will be generated.
* @param period Time in micro seconds.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetBurst(rp_handle_uio_t *handle, uint32_t  count, uint32_t  repetitions, uint32_t  delay);
int rp_GenGetBurst(rp_handle_uio_t *handle, uint32_t *count, uint32_t *repetitions, uint32_t *delay);

/**
* Sets trigger source.
* @param channel Channel A or B for witch we want to set trigger source.
* @param src Trigger source (INTERNAL, EXTERNAL_PE, EXTERNAL_NE, GATED_BURST).
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetTriggerMask(rp_handle_uio_t *handle, uint32_t  mask);
int rp_GenGetTriggerMask(rp_handle_uio_t *handle, uint32_t *mask);

/**
* Sets Trigger for specified channel/channels.
* @param mask Mask determines channel: 1->ch1, 2->ch2, 3->ch1&ch2.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenTrigger(rp_handle_uio_t *handle);

#endif //__GENERATE_H
