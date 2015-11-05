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

#include "redpitaya/rp.h"

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
#define GENERATE_BASE_ADDR      0x40200000
#define GENERATE_BASE_SIZE      0x00100000

// global definitions
#define RP_MNG      2

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
    uint32_t ctl_rst  :1;
    uint32_t ctl_trg  :1;
    // configuration
    uint32_t cfg_tsel :RP_GEN_TWS;
    uint32_t cfg_bena :1;
    uint32_t cfg_binf :1;
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
} generate_regset_t;

int generate_Init();
int generate_Release();

#endif //__GENERATE_H
