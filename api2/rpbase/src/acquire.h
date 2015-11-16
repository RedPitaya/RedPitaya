////////////////////////////////////////////////////////////////////////////////
// Red Pitaya library acquire module interface
// Author: Red Pitaya
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

#ifndef SRC_ACQIRE_H_
#define SRC_ACQIRE_H_

#include <stdint.h>
#include <stdbool.h>

#define RP_MNA 2

#define RP_ACQ_DWI 14

#define RP_ACQ_CTL_RST_MASK 0x1
#define RP_ACQ_CTL_TRG_MASK 0x2
#define RP_ACQ_CTL_ACQ_MASK 0x4

// Base acquire address
static const int ACQ_BASE_ADDR = 0x40600000;
static const int ACQ_BASE_SIZE = 0x00100000;

// acquire structure declaration
typedef struct {
    // control/status
    uint32_t ctl;
//    {
//       uint32_t rst :1;
//       uint32_t trg :1;
//       uint32_t acq :1;
//    }
    // configuration
    uint32_t cfg_rng;  // :1;
    // trigger
    uint32_t cfg_sel;  // TWG
    uint32_t cfg_dly;  // u32
    // edge detection
    int32_t  cfg_lvl;  // s14
    uint32_t cfg_hst;  // u14
    // filter/decimation
    uint32_t cfg_byp;  // :1;
    uint32_t cfg_avg;  // :1;
    uint32_t cfg_dec;  // :DWC;
    uint32_t cfg_shr;  // :DWS;
    int32_t  cfg_faa;
    int32_t  cfg_fbb;
    int32_t  cfg_fkk;
    int32_t  cfg_fpp;
} acq_regset_t;

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

int acq_Init();
int acq_Release();

#endif // SRC_ACQUIRE_H_
