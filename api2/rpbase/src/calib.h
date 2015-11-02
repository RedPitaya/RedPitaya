/**
 * @brief Red Pitaya Calibration Module.
 * @Author Red Pitaya
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __CALIB_H
#define __CALIB_H

#include <stdint.h>

#include "redpitaya/rp.h"

// Base Housekeeping address
#define RP_CALIB_BASE_ADDR 0x40500000
#define RP_CALIB_BASE_SIZE 0x30

#define RP_CALIB_EEPROM_PATH "/sys/bus/i2c/devices/0-0050/eeprom"
#define RP_CALIB_EEPROM_ADDR 0x0008

#define RP_CALIB_DWM 16
#define RP_CALIB_DWS 14

// hardware - calibration gain offset pair
typedef struct {
    uint32_t mul;  // fixed point   signed  s1.14 (RP_CALIB_DWM bits)
    uint32_t sum;  // fixed point unsigned  u1.13 (RP_CALIB_DWS bits)
} regset_calib_pair_t;

// hardware - calibration register set structure
typedef struct {
    regset_calib_pair_t acq [2];
    regset_calib_pair_t gen [2];
} regset_calib_t;

int calib_Init();
int calib_Release();

#endif //__CALIB_H
