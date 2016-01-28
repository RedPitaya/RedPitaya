/**
 * @brief Red Pitaya Calibration Module.
 * @Author Red Pitaya
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __CALIB_H
#define __CALIB_H

#include "common.h"

// Base Housekeeping address
#define RP_CALIB_BASE_SIZE 0x00010000

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
} rp_calib_regset_t;

int rp_CalibOpen(char *dev, rp_handle_uio_t *handle);
int rp_CalibClose(rp_handle_uio_t *handle);

/**
 * Calibration parameters, structure stored in the EEPROM device
 */
typedef struct {
    float offset;  // in Volts
    float gain;    // correction ratio
} rp_calib_pair_t;

typedef struct {
    int unsigned chn;
    int range[2];
    rp_calib_pair_t acq [2] [2];
    rp_calib_pair_t gen [2];
} rp_calib_context_t;

/**
* Calibration reset.
* Default values are written into calibration regisers, EEPROM contents are unchanged.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_CalibReset(rp_handle_uio_t *handle);

/**
* Get calibration parameters.
* Hardware calibration registers are read and stored into calibration parameter structure.
* @return RP_OK
*/
int rp_CalibGetParams(rp_handle_uio_t *handle);

/**
* Set calibration parameters.
* Hardware calibration registers are writen based on calibration parameter structure.
* @return RP_OK
*/
int rp_CalibSetParams(rp_handle_uio_t *handle);

/**
* Read calibration parameters.
* The given calibration parameter structure is populated from EEPROM contents.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_CalibReadParams(rp_calib_context_t *context);

/**
* Write calibration parameters.
* The given calibration parameter structure is written into EEPROM.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_CalibWriteParams(rp_calib_context_t *context);
///@}


#endif //__CALIB_H
