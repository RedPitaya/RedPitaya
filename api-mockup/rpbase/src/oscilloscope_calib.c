/**
 * $Id: $
 *
 * @brief Red Pitaya Oscilloscope Calibration Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include "oscilloscope_calib.h"

#include <stdio.h>

static const char eeprom_device[]="/sys/bus/i2c/devices/0-0050/eeprom";
static const int  eeprom_calib_off=0x0008;


/*----------------------------------------------------------------------------*/
/**
 * @brief Read calibration parameters from EEPROM device.
 *
 * Function reads calibration parameters from EEPROM device and stores them to the
 * specified buffer. Communication to the EEPROM device is taken place through
 * appropriate system driver accessed through the file system device
 * /sys/bus/i2c/devices/0-0050/eeprom.
 *
 * @param[out]   calib_params  Pointer to destination buffer.
 * @retval       0 Success
 * @retval       >0 Failure
 *
 */
int calib_ReadParams(calib_params_t *calib_params)
{
    FILE   *fp;
    size_t  size;

    /* sanity check */
    if(calib_params == NULL) {
        return RP_UIA;
    }

    /* open EEPROM device */
    fp = fopen(eeprom_device, "r");
    if(fp == NULL) {
       return RP_EOED;
    }

    /* ...and seek to the appropriate storage offset */
    if(fseek(fp, eeprom_calib_off, SEEK_SET) < 0) {
        fclose(fp);
        return RP_FCA;
    }

    /* read data from EEPROM component and store it to the specified buffer */
    size = fread(calib_params, sizeof(char), sizeof(calib_params_t), fp);
    if(size != sizeof(calib_params_t)) {
        fclose(fp);
        return RP_RCA;
    }
    fclose(fp);

    return 0;
}


/*----------------------------------------------------------------------------*/
/**
 * Initialize calibration parameters to default values.
 *
 * @param[out] calib_params  Pointer to target buffer to be initialized.
 * @retval 0 Success, could never fail.
 */

int calib_GetDefaultParams(calib_params_t *calib_params)
{
    calib_params->fe_ch1_fs_g_hi = 28101971; /* 0.6543 [V] */
    calib_params->fe_ch2_fs_g_hi = 28101971; /* 0.6543 [V] */
    calib_params->fe_ch1_fs_g_lo = 625682246; /* 14.56 [V] */
    calib_params->fe_ch2_fs_g_lo = 625682246; /* 14.56 [V] */
    calib_params->fe_ch1_dc_offs = 585;
    calib_params->fe_ch2_dc_offs = 585;
    calib_params->be_ch1_fs = 42949673; /* 1 [V] */
    calib_params->be_ch2_fs = 42949673; /* 1 [V] */
    calib_params->be_ch1_dc_offs = 0x3eac;
    calib_params->be_ch2_dc_offs = 0x3eac;

    return 0;
}
