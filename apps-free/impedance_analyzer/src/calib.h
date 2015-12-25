/** @file calib.h
 *
 * $Id: calib.h 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Oscilloscope Calibration Module (C Header).
 * @author Jure Menart <juremenart@gmail.com>
 * @copyright Red Pitaya  http://www.redpitaya.com
 */

#ifndef __CALIB_H
#define __CALIB_H

#include <stdint.h>

/** @defgroup calib_h Calibration
 * @{
 */

/** Calibration parameters, stored in eeprom device
 */
typedef struct rp_osc_calib_params_s {
    uint32_t fe_ch1_fs_g_hi;            /**< High gain front end full scale voltage, channel 1 */
    uint32_t fe_ch2_fs_g_hi;            /**< High gain front end full scale voltage, channel 2 */
    uint32_t fe_ch1_fs_g_lo;            /**< Low gain front end full scale voltage, channel 1  */
    uint32_t fe_ch2_fs_g_lo;            /**< Low gain front end full scale voltage, channel 2  */
    int32_t  fe_ch1_dc_offs;            /**< Front end DC offset, channel 1  */
    int32_t  fe_ch2_dc_offs;            /**< Front end DC offset, channel 2  */
    uint32_t be_ch1_fs;                 /**< Back end full scale voltage, channel 1  */
    uint32_t be_ch2_fs;                 /**< Back end full scale voltage, channel 2  */
    int32_t  be_ch1_dc_offs;            /**< Back end DC offset, channel 1 */
    int32_t  be_ch2_dc_offs;            /**< Back end DC offset, on channel 2 */
} rp_calib_params_t;

/** @} */

int rp_read_calib_params(rp_calib_params_t *calib_params);

int rp_default_calib_params(rp_calib_params_t *calib_params);

#endif //__CALIB_H
