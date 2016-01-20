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
#define ARBITRARY_MIN          -1.0         // V
#define ARBITRARY_MAX           1.0         // V
#define OFFSET_MAX              2.0         // V
#define FREQUENCY_MIN           0           // Hz
#define FREQUENCY_MAX           62.5e6      // Hz
#define PHASE_MIN              -360         // deg
#define PHASE_MAX               360         // deg
#define DUTY_CYCLE_MIN          0           // %
#define DUTY_CYCLE_MAX          100         // %
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

typedef struct {
    // control register
    uint32_t ctl_sys;
        // rst :1;
        // trg :1;
    // configuration
    uint32_t cfg_sys;
        // tsel :RP_GEN_TWS;
        // bena :1;
        // binf :1;
    uint32_t cfg_size;
    uint32_t cfg_offs;
    uint32_t cfg_step;
    // burst mode
    uint32_t cfg_bcyc;
    uint32_t cfg_bdly;
    uint32_t cfg_bnum;
    // linear transformation
    int32_t  cfg_lmul;
    int32_t  cfg_lsum;
    // empty space
    uint32_t reserved0[(1<<RP_GEN_CWM)-10];
    // table
    int32_t  table[(1<<RP_GEN_CWM)];
    // empty space
    uint32_t reserved1[GENERATE_BASE_SIZE - (2<<RP_GEN_CWM)];
} gen_regset_t;

int rp_GenInit(char *dev, rp_handle_uio_t *handle);
int rp_GenRelease(rp_handle_uio_t *handle);

/**
* Sets generate to default values.
*/
int rp_GenReset();

/**
* Sets channel signal peak to peak amplitude.
* @param channel Channel A or B for witch we want to set amplitude
* @param amplitude Amplitude of the generated signal. From 0 to max value. Max amplitude is 1
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetAmp(rp_handle_uio_t *handle, float amplitude);

/**
* Gets channel signal peak to peak amplitude.
* @param channel Channel A or B for witch we want to get amplitude.
* @param amplitude Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetAmp(rp_handle_uio_t *handle, float *amplitude);

/**
* Sets DC offset of the signal. signal = signal + DC_offset.
* @param channel Channel A or B for witch we want to set DC offset.
* @param offset DC offset of the generated signal. Max offset is 2.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetOffset(rp_handle_uio_t *handle, float offset);

/**
* Gets DC offset of the signal.
* @param channel Channel A or B for witch we want to get amplitude.
* @param offset Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetOffset(rp_handle_uio_t *handle, float *offset);

/**
* Sets channel signal frequency.
* @param channel Channel A or B for witch we want to set frequency.
* @param frequency Frequency of the generated signal in Hz.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetFreq(rp_handle_uio_t *handle, double frequency);

/**
* Gets channel signal frequency.
* @param channel Channel A or B for witch we want to get frequency.
* @param frequency Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetFreq(rp_handle_uio_t *handle, double *frequency);

/**
* Sets channel signal phase. This shifts the signal in time.
* @param channel Channel A or B for witch we want to set phase.
* @param phase Phase in degrees of the generated signal. From 0 deg to 180 deg.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetPhase(rp_handle_uio_t *handle, double phase);

/**
* Gets channel signal phase.
* @param channel Channel A or B for witch we want to get phase.
* @param phase Pointer where value will be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenGetPhase(rp_handle_uio_t *handle, double *phase);

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
* @param channel Channel A or B for witch we want to set number of generated waveforms in a burst.
* @param num Number of generated waveforms. If -1 a continuous signal will be generated.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetBurstCount(rp_handle_uio_t *handle, int  num);
int rp_GenGetBurstCount(rp_handle_uio_t *handle, int *num);

/**
* Sets number of burst repetitions. This determines how many bursts will be generated.
* @param channel Channel A or B for witch we want to set number of burst repetitions.
* @param repetitions Number of generated bursts. If -1, infinite bursts will be generated.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetBurstRepetitions(rp_handle_uio_t *handle, int  repetitions);
int rp_GenGetBurstRepetitions(rp_handle_uio_t *handle, int *repetitions);

/**
* Sets the time/period of one burst in micro seconds. Period must be equal or greater then the time of one burst.
* If it is greater than the difference will be the delay between two consequential bursts.
* @param channel Channel A or B for witch we want to set burst period.
* @param period Time in micro seconds.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetBurstDelay(rp_handle_uio_t *handle, uint32_t  delay);
int rp_GenGetBurstDelay(rp_handle_uio_t *handle, uint32_t *delay);

/**
* Sets trigger source.
* @param channel Channel A or B for witch we want to set trigger source.
* @param src Trigger source (INTERNAL, EXTERNAL_PE, EXTERNAL_NE, GATED_BURST).
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenSetTriggerSource(rp_handle_uio_t *handle, uint32_t  src);
int rp_GenGetTriggerSource(rp_handle_uio_t *handle, uint32_t *src);

/**
* Sets Trigger for specified channel/channels.
* @param mask Mask determines channel: 1->ch1, 2->ch2, 3->ch1&ch2.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GenTrigger(rp_handle_uio_t *handle);

#endif //__GENERATE_H
