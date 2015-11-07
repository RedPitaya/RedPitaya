////////////////////////////////////////////////////////////////////////////////
// Red Pitaya library oscilloscope module interface
// Author: Red Pitaya
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

#ifndef SRC_OSCILLOSCOPE_H_
#define SRC_OSCILLOSCOPE_H_

#include <stdint.h>
#include <stdbool.h>

// Base Oscilloscope address
static const int OSC_BASE_ADDR = 0x40600000;
static const int OSC_BASE_SIZE = 0x00100000;

// Oscilloscope structure declaration
typedef struct osc_control_s {
    // control/status
    uint32_t ctl;
    uint32_t sts;
//    {
//       uint32_t rst :1;
//       uint32_t trg :1;
//       uint32_t acq :1;
//    }
    // decimation
    uint32_t cfg_avg;  // 1
    uint32_t cfg_dec;  // DWC
    uint32_t cfg_hst;  // DWS
    // edge detection
    int32_t  cfg_lvl;  // s14
    uint32_t cfg_hst;  // u14
    // trigger
    uint32_t cfg_sel;  // TWG
    uint32_t cfg_dly;  // u32
    // filter
    int32_t  cfg_faa;
    int32_t  cfg_fbb;
    int32_t  cfg_fkk;
    int32_t  cfg_fpp;
} osc_regset_t;

static const uint32_t DATA_DEC_MASK         = 0x1FFFF;      // (17 bits)
static const uint32_t DATA_AVG_MASK         = 0x1;          // (1 bit)
static const uint32_t TRIG_SRC_MASK         = 0xF;          // (4 bits)
static const uint32_t START_DATA_WRITE_MASK = 0x1;          // (1 bit)
static const uint32_t THRESHOLD_MASK        = 0x3FFF;       // (14 bits)
static const uint32_t HYSTERESIS_MASK       = 0x3FFF;       // (14 bits)
static const uint32_t TRIG_DELAY_MASK       = 0xFFFFFFFF;   // (32 bits)
static const uint32_t WRITE_POINTER_MASK    = 0x3FFF;       // (14 bits)
static const uint32_t EQ_FILTER_AA          = 0x3FFFF;      // (18 bits)
static const uint32_t EQ_FILTER             = 0x1FFFFFF;    // (25 bits)
static const uint32_t RST_WR_ST_MCH_MASK    = 0x2;          // (1st bit)
static const uint32_t TRIG_ST_MCH_MASK      = 0x4;          // (2st bit)
static const uint32_t PRE_TRIGGER_COUNTER   = 0xFFFFFFFF;   // (32 bit)

/* @brief Default filter equalization coefficients */
static const uint32_t GAIN_LO_FILT_AA = 0x7D93;
static const uint32_t GAIN_LO_FILT_BB = 0x437C7;
static const uint32_t GAIN_LO_FILT_PP = 0x2666;
static const uint32_t GAIN_LO_FILT_KK = 0xd9999a;
static const uint32_t GAIN_HI_FILT_AA = 0x4C5F;
static const uint32_t GAIN_HI_FILT_BB = 0x2F38B;
static const uint32_t GAIN_HI_FILT_PP = 0x2666;
static const uint32_t GAIN_HI_FILT_KK = 0xd9999a;

int osc_Init();
int osc_Release();

#endif // SRC_OSCILLOSCOPE_H_
